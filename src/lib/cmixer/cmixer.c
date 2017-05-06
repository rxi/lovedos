/*
** Copyright (c) 2017 rxi
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to
** deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
** sell copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
** IN THE SOFTWARE.
**/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmixer.h"

#define UNUSED(x)         ((void) (x))
#define CLAMP(x, a, b)    ((x) < (a) ? (a) : (x) > (b) ? (b) : (x))
#define MIN(a, b)         ((a) < (b) ? (a) : (b))
#define MAX(a, b)         ((a) > (b) ? (a) : (b))

#define FX_BITS           (12)
#define FX_UNIT           (1 << FX_BITS)
#define FX_MASK           (FX_UNIT - 1)
#define FX_FROM_FLOAT(f)  ((f) * FX_UNIT)
#define FX_LERP(a, b, p)  ((a) + ((((b) - (a)) * (p)) >> FX_BITS))

#define BUFFER_SIZE       (512)
#define BUFFER_MASK       (BUFFER_SIZE - 1)


struct cm_Source {
  cm_Source *next;              /* Next source in list */
  cm_Int16 buffer[BUFFER_SIZE]; /* Internal buffer with raw stereo PCM */
  cm_EventHandler handler;      /* Event handler */
  void *udata;          /* Stream's udata (from cm_SourceInfo) */
  int samplerate;       /* Stream's native samplerate */
  int length;           /* Stream's length in frames */
  int end;              /* End index for the current play-through */
  int state;            /* Current state (playing|paused|stopped) */
  cm_Int64 position;    /* Current playhead position (fixed point) */
  int lgain, rgain;     /* Left and right gain (fixed point) */
  int rate;             /* Playback rate (fixed point) */
  int nextfill;         /* Next frame idx where the buffer needs to be filled */
  int loop;             /* Whether the source will loop when `end` is reached */
  int rewind;           /* Whether the source will rewind before playing */
  int active;           /* Whether the source is part of `sources` list */
  double gain;          /* Gain set by `cm_set_gain()` */
  double pan;           /* Pan set by `cm_set_pan()` */
};


static struct {
  const char *lasterror;        /* Last error message */
  cm_EventHandler lock;         /* Event handler for lock/unlock events */
  cm_Source *sources;           /* Linked list of active (playing) sources */
  cm_Int32 buffer[BUFFER_SIZE]; /* Internal master buffer */
  int samplerate;               /* Master samplerate */
  int gain;                     /* Master gain (fixed point) */
} cmixer;


static void dummy_handler(cm_Event *e) {
  UNUSED(e);
}


static void lock(void) {
  cm_Event e;
  e.type = CM_EVENT_LOCK;
  cmixer.lock(&e);
}


static void unlock(void) {
  cm_Event e;
  e.type = CM_EVENT_UNLOCK;
  cmixer.lock(&e);
}


const char* cm_get_error(void) {
  const char *res = cmixer.lasterror;
  cmixer.lasterror = NULL;
  return res;
}


static const char* error(const char *msg) {
  cmixer.lasterror = msg;
  return msg;
}


void cm_init(int samplerate) {
  cmixer.samplerate = samplerate;
  cmixer.lock = dummy_handler;
  cmixer.sources = NULL;
  cmixer.gain = FX_UNIT;
}


void cm_set_lock(cm_EventHandler lock) {
  cmixer.lock = lock;
}


void cm_set_master_gain(double gain) {
  cmixer.gain = FX_FROM_FLOAT(gain);
}


static void rewind_source(cm_Source *src) {
  cm_Event e;
  e.type = CM_EVENT_REWIND;
  e.udata = src->udata;
  src->handler(&e);
  src->position = 0;
  src->rewind = 0;
  src->end = src->length;
  src->nextfill = 0;
}


static void fill_source_buffer(cm_Source *src, int offset, int length) {
  cm_Event e;
  e.type = CM_EVENT_SAMPLES;
  e.udata = src->udata;
  e.buffer = src->buffer + offset;
  e.length = length;
  src->handler(&e);
}


static void process_source(cm_Source *src, int len) {
  int i, n, a, b, p;
  int frame, count;
  cm_Int32 *dst = cmixer.buffer;

  /* Do rewind if flag is set */
  if (src->rewind) {
    rewind_source(src);
  }

  /* Process audio */
  while (len > 0) {
    /* Get current position frame */
    frame = src->position >> FX_BITS;

    /* Fill buffer if required */
    if (frame + 3 >= src->nextfill) {
      fill_source_buffer(src, (src->nextfill*2) & BUFFER_MASK, BUFFER_SIZE/2);
      src->nextfill += BUFFER_SIZE / 4;
    }

    /* Handle reaching the end of the playthrough */
    if (frame >= src->end) {
      /* As streams continiously fill the raw buffer in a loop we simply
      ** increment the end idx by one length and continue reading from it for
      ** another play-through */
      src->end = frame + src->length;
      /* Set state and stop processing if we're not set to loop */
      if (!src->loop) {
        src->state = CM_STATE_STOPPED;
        break;
      }
    }

    /* Work out how many frames we should process in the loop */
    n = MIN(src->nextfill - 2, src->end) - frame;
    count = (n << FX_BITS) / src->rate;
    count = MAX(count, 1);
    count = MIN(count, len / 2);
    len -= count * 2;

    /* Add audio to master buffer */
    if (src->rate == FX_UNIT) {
      /* Add audio to buffer -- basic */
      n = frame * 2;
      for (i = 0; i < count; i++) {
        dst[0] += (src->buffer[(n    ) & BUFFER_MASK] * src->lgain) >> FX_BITS;
        dst[1] += (src->buffer[(n + 1) & BUFFER_MASK] * src->rgain) >> FX_BITS;
        n += 2;
        dst += 2;
      }
      src->position += count * FX_UNIT;

    } else {
      /* Add audio to buffer -- interpolated */
      for (i = 0; i < count; i++) {
        n = (src->position >> FX_BITS) * 2;
        p = src->position & FX_MASK;
        a = src->buffer[(n    ) & BUFFER_MASK];
        b = src->buffer[(n + 2) & BUFFER_MASK];
        dst[0] += (FX_LERP(a, b, p) * src->lgain) >> FX_BITS;
        n++;
        a = src->buffer[(n    ) & BUFFER_MASK];
        b = src->buffer[(n + 2) & BUFFER_MASK];
        dst[1] += (FX_LERP(a, b, p) * src->rgain) >> FX_BITS;
        src->position += src->rate;
        dst += 2;
      }
    }

  }
}


void cm_process(cm_Int16 *dst, int len) {
  int i;
  cm_Source **s;

  /* Process in chunks of BUFFER_SIZE if `len` is larger than BUFFER_SIZE */
  while (len > BUFFER_SIZE) {
    cm_process(dst, BUFFER_SIZE);
    dst += BUFFER_SIZE;
    len -= BUFFER_SIZE;
  }

  /* Zeroset internal buffer */
  memset(cmixer.buffer, 0, len * sizeof(cmixer.buffer[0]));

  /* Process active sources */
  lock();
  s = &cmixer.sources;
  while (*s) {
    process_source(*s, len);
    /* Remove source from list if it is no longer playing */
    if ((*s)->state != CM_STATE_PLAYING) {
      (*s)->active = 0;
      *s = (*s)->next;
    } else {
      s = &(*s)->next;
    }
  }
  unlock();

  /* Copy internal buffer to destination and clip */
  for (i = 0; i < len; i++) {
    int x = (cmixer.buffer[i] * cmixer.gain) >> FX_BITS;
    dst[i] = CLAMP(x, -32768, 32767);
  }
}


cm_Source* cm_new_source(const cm_SourceInfo *info) {
  cm_Source *src = calloc(1, sizeof(*src));
  if (!src) {
    error("allocation failed");
    return NULL;
  }
  src->handler = info->handler;
  src->length = info->length;
  src->samplerate = info->samplerate;
  src->udata = info->udata;
  cm_set_gain(src, 1);
  cm_set_pan(src, 0);
  cm_set_pitch(src, 1);
  cm_set_loop(src, 0);
  cm_stop(src);
  return src;
}


static const char* wav_init(cm_SourceInfo *info, void *data, int len, int ownsdata);

#ifdef CM_USE_STB_VORBIS
static const char* ogg_init(cm_SourceInfo *info, void *data, int len, int ownsdata);
#endif


static int check_header(void *data, int size, char *str, int offset) {
  int len = strlen(str);
  return (size >= offset + len) && !memcmp((char*) data + offset, str, len);
}


static cm_Source* new_source_from_mem(void *data, int size, int ownsdata) {
  const char *err;
  cm_SourceInfo info;

  if (check_header(data, size, "WAVE", 8)) {
    err = wav_init(&info, data, size, ownsdata);
    if (err) {
      return NULL;
    }
    return cm_new_source(&info);
  }

#ifdef CM_USE_STB_VORBIS
  if (check_header(data, size, "OggS", 0)) {
    err = ogg_init(&info, data, size, ownsdata);
    if (err) {
      return NULL;
    }
    return cm_new_source(&info);
  }
#endif

  error("unknown format or invalid data");
  return NULL;
}


static void* load_file(const char *filename, int *size) {
  FILE *fp;
  void *data;
  int n;

  fp = fopen(filename, "rb");
  if (!fp) {
    return NULL;
  }

  /* Get size */
  fseek(fp, 0, SEEK_END);
  *size = ftell(fp);
  rewind(fp);

  /* Malloc, read and return data */
  data = malloc(*size);
  if (!data) {
    fclose(fp);
    return NULL;
  }
  n = fread(data, 1, *size, fp);
  fclose(fp);
  if (n != *size) {
    free(data);
    return NULL;
  }

  return data;
}


cm_Source* cm_new_source_from_file(const char *filename) {
  int size;
  cm_Source *src;
  void *data;

  /* Load file into memory */
  data = load_file(filename, &size);
  if (!data) {
    error("could not load file");
    return NULL;
  }

  /* Try to load and return */
  src = new_source_from_mem(data, size, 1);
  if (!src) {
    free(data);
    return NULL;
  }

  return src;
}


cm_Source* cm_new_source_from_mem(void *data, int size) {
  return new_source_from_mem(data, size, 0);
}


void cm_destroy_source(cm_Source *src) {
  cm_Event e;
  lock();
  if (src->active) {
    cm_Source **s = &cmixer.sources;
    while (*s) {
      if (*s == src) {
        *s = src->next;
        break;
      }
    }
  }
  unlock();
  e.type = CM_EVENT_DESTROY;
  e.udata = src->udata;
  src->handler(&e);
  free(src);
}


double cm_get_length(cm_Source *src) {
  return src->length / (double) src->samplerate;
}


double cm_get_position(cm_Source *src) {
  return ((src->position >> FX_BITS) % src->length) / (double) src->samplerate;
}


int cm_get_state(cm_Source *src) {
  return src->state;
}


static void recalc_source_gains(cm_Source *src) {
  double l, r;
  double pan = src->pan;
  l = src->gain * (pan <= 0. ? 1. : 1. - pan);
  r = src->gain * (pan >= 0. ? 1. : 1. + pan);
  src->lgain = FX_FROM_FLOAT(l);
  src->rgain = FX_FROM_FLOAT(r);
}


void cm_set_gain(cm_Source *src, double gain) {
  src->gain = gain;
  recalc_source_gains(src);
}


void cm_set_pan(cm_Source *src, double pan) {
  src->pan = pan;
  recalc_source_gains(src);
}


void cm_set_pitch(cm_Source *src, double pitch) {
  double rate = src->samplerate / (double) cmixer.samplerate * pitch;
  src->rate = FX_FROM_FLOAT(rate);
}


void cm_set_loop(cm_Source *src, int loop) {
  src->loop = loop;
}


void cm_play(cm_Source *src) {
  lock();
  src->state = CM_STATE_PLAYING;
  if (!src->active) {
    src->active = 1;
    src->next = cmixer.sources;
    cmixer.sources = src;
  }
  unlock();
}


void cm_pause(cm_Source *src) {
  src->state = CM_STATE_PAUSED;
}


void cm_stop(cm_Source *src) {
  src->state = CM_STATE_STOPPED;
  src->rewind = 1;
}


/*============================================================================
** Wav stream
**============================================================================*/

typedef struct {
  void *data;
  int bitdepth;
  int samplerate;
  int channels;
  int length;
} Wav;

typedef struct {
  Wav wav;
  void *data;
  int idx;
} WavStream;


static char* find_subchunk(char *data, int len, char *id, int *size) {
  /* TODO : Error handling on malformed wav file */
  int idlen = strlen(id);
  char *p = data + 12;
next:
  *size = *((cm_UInt32*) (p + 4));
  if (memcmp(p, id, idlen)) {
    p += 8 + *size;
    if (p > data + len) return NULL;
    goto next;
  }
  return p + 8;
}


static const char* read_wav(Wav *w, void *data, int len) {
  int bitdepth, channels, samplerate, format;
  int sz;
  char *p = data;
  memset(w, 0, sizeof(*w));

  /* Check header */
  if (memcmp(p, "RIFF", 4) || memcmp(p + 8, "WAVE", 4)) {
    return error("bad wav header");
  }
  /* Find fmt subchunk */
  p = find_subchunk(data, len, "fmt", &sz);
  if (!p) {
    return error("no fmt subchunk");
  }

  /* Load fmt info */
  format      = *((cm_UInt16*) (p));
  channels    = *((cm_UInt16*) (p + 2));
  samplerate  = *((cm_UInt32*) (p + 4));
  bitdepth    = *((cm_UInt16*) (p + 14));
  if (format != 1) {
    return error("unsupported format");
  }
  if (channels == 0 || samplerate == 0 || bitdepth == 0) {
    return error("bad format");
  }

  /* Find data subchunk */
  p = find_subchunk(data, len, "data", &sz);
  if (!p) {
    return error("no data subchunk");
  }

  /* Init struct */
  w->data = (void*) p;
  w->samplerate = samplerate;
  w->channels = channels;
  w->length = (sz / (bitdepth / 8)) / channels;
  w->bitdepth = bitdepth;
  /* Done */
  return NULL;
}


#define WAV_PROCESS_LOOP(X) \
  while (n--) {             \
    X                       \
    dst += 2;               \
    s->idx++;               \
  }

static void wav_handler(cm_Event *e) {
  int x, n;
  cm_Int16 *dst;
  WavStream *s = e->udata;
  int len;

  switch (e->type) {

    case CM_EVENT_DESTROY:
      free(s->data);
      free(s);
      break;

    case CM_EVENT_SAMPLES:
      dst = e->buffer;
      len = e->length / 2;
fill:
      n = MIN(len, s->wav.length - s->idx);
      len -= n;
      if (s->wav.bitdepth == 16 && s->wav.channels == 1) {
        WAV_PROCESS_LOOP({
          dst[0] = dst[1] = ((cm_Int16*) s->wav.data)[s->idx];
        });
      } else if (s->wav.bitdepth == 16 && s->wav.channels == 2) {
        WAV_PROCESS_LOOP({
          x = s->idx * 2;
          dst[0] = ((cm_Int16*) s->wav.data)[x    ];
          dst[1] = ((cm_Int16*) s->wav.data)[x + 1];
        });
      } else if (s->wav.bitdepth == 8 && s->wav.channels == 1) {
        WAV_PROCESS_LOOP({
          dst[0] = dst[1] = (((cm_UInt8*) s->wav.data)[s->idx] - 128) << 8;
        });
      } else if (s->wav.bitdepth == 8 && s->wav.channels == 2) {
        WAV_PROCESS_LOOP({
          x = s->idx * 2;
          dst[0] = (((cm_UInt8*) s->wav.data)[x    ] - 128) << 8;
          dst[1] = (((cm_UInt8*) s->wav.data)[x + 1] - 128) << 8;
        });
      }
      /* Loop back and continue filling buffer if we didn't fill the buffer */
      if (len > 0) {
        s->idx = 0;
        goto fill;
      }
      break;

    case CM_EVENT_REWIND:
      s->idx = 0;
      break;
  }
}


static const char* wav_init(cm_SourceInfo *info, void *data, int len, int ownsdata) {
  WavStream *stream;
  Wav wav;

  const char *err = read_wav(&wav, data, len);
  if (err != NULL) {
    return err;
  }

  if (wav.channels > 2 || (wav.bitdepth != 16 && wav.bitdepth != 8)) {
    return error("unsupported wav format");
  }

  stream = calloc(1, sizeof(*stream));
  if (!stream) {
    return error("allocation failed");
  }
  stream->wav = wav;

  if (ownsdata) {
    stream->data = data;
  }
  stream->idx = 0;

  info->udata = stream;
  info->handler = wav_handler;
  info->samplerate = wav.samplerate;
  info->length = wav.length;

  /* Return NULL (no error) for success */
  return NULL;
}


/*============================================================================
** Ogg stream
**============================================================================*/

#ifdef CM_USE_STB_VORBIS

#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

typedef struct {
  stb_vorbis *ogg;
  void *data;
} OggStream;


static void ogg_handler(cm_Event *e) {
  int n, len;
  OggStream *s = e->udata;
  cm_Int16 *buf;

  switch (e->type) {

    case CM_EVENT_DESTROY:
      stb_vorbis_close(s->ogg);
      free(s->data);
      free(s);
      break;

    case CM_EVENT_SAMPLES:
      len = e->length;
      buf = e->buffer;
fill:
      n = stb_vorbis_get_samples_short_interleaved(s->ogg, 2, buf, len);
      n *= 2;
      /* rewind and fill remaining buffer if we reached the end of the ogg
      ** before filling it */
      if (len != n) {
        stb_vorbis_seek_start(s->ogg);
        buf += n;
        len -= n;
        goto fill;
      }
      break;

    case CM_EVENT_REWIND:
      stb_vorbis_seek_start(s->ogg);
      break;
  }
}


static const char* ogg_init(cm_SourceInfo *info, void *data, int len, int ownsdata) {
  OggStream *stream;
  stb_vorbis *ogg;
  stb_vorbis_info ogginfo;
  int err;

  ogg = stb_vorbis_open_memory(data, len, &err, NULL);
  if (!ogg) {
    return error("invalid ogg data");
  }

  stream = calloc(1, sizeof(*stream));
  if (!stream) {
    stb_vorbis_close(ogg);
    return error("allocation failed");
  }

  stream->ogg = ogg;
  if (ownsdata) {
    stream->data = data;
  }

  ogginfo = stb_vorbis_get_info(ogg);

  info->udata = stream;
  info->handler = ogg_handler;
  info->samplerate = ogginfo.sample_rate;
  info->length = stb_vorbis_stream_length_in_samples(ogg);

  /* Return NULL (no error) for success */
  return NULL;
}


#endif
