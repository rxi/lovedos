/** 
 * Copyright (c) 2015 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */


#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "wav.h"

typedef unsigned short Uint16;
typedef unsigned int   Uint32;


static const char *findSubChunk(
  const char *data, size_t len, const char *id, size_t *size
) {
  /* TODO : Error handling on malformed wav file */
  size_t idLen = strlen(id);
  const char *p = data + 12;
next:
  *size = *((Uint32*) (p + 4));
  if (memcmp(p, id, idLen)) {
    p += 8 + *size;
    if (p > data + len) return NULL;
    goto next;
  }
  return p + 8;
}


int wav_read(wav_t *w, const void *data, size_t len) {
  int bitdepth, channels, samplerate, format;
  size_t sz;
  const char *p = data;
  memset(w, 0, sizeof(*w));
  /* Check header */
  if (memcmp(p, "RIFF", 4) || memcmp(p + 8, "WAVE", 4)) {
    return WAV_EBADHEADER;
  }
  /* Find fmt subchunk */
  p = findSubChunk(data, len, "fmt", &sz);
  if (!p) return WAV_ENOFMT;
  /* Load fmt info */
  format      = *((Uint16*) (p));
  channels    = *((Uint16*) (p + 2));
  samplerate  = *((Uint32*) (p + 4));
  bitdepth    = *((Uint16*) (p + 14));
  if (format != 1) {
    return WAV_ENOSUPPORT;
  }
  if (channels == 0 || samplerate == 0 || bitdepth == 0) {
    return WAV_EBADFMT;
  }
  /* Find data subchunk */
  p = findSubChunk(data, len, "data", &sz);
  if (!p) return WAV_ENODATA;
  /* Init wav_t struct */
  w->data = p;
  w->samplerate = samplerate;
  w->channels = channels;
  w->length = (sz / (bitdepth / 8)) / channels;
  w->bitdepth = bitdepth;
  /* Done! */
  return WAV_ESUCCESS;
}


const char *wav_strerror(int err) {
  switch (err) {
    case WAV_ESUCCESS    : return "success";
    case WAV_EFAILURE    : return "failure";
    case WAV_EBADHEADER  : return "bad header data";
    case WAV_EBADFMT     : return "bad fmt data";
    case WAV_ENOFMT      : return "missing 'fmt' subchunk";
    case WAV_ENODATA     : return "missing 'data' subchunk";
    case WAV_ENOSUPPORT  : return "unsupported format; "
                                  "expected uncompressed PCM";
  }
  return "unknown error";
}
