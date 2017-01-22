/**
 * Copyright (c) 2017 rnlf
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <string.h>
#include "filesystem.h"
#include "wavefile.h"
#include "source.h"
#include "lib/dmt/dmt.h"

typedef struct {
  uint32_t chunkId;
  uint32_t chunkSize;
  uint32_t format;
  uint32_t subchunk1Id;
  uint32_t subchunk1Size;
  uint16_t audioFormat;
  uint16_t numChannels;
  uint32_t sampleRate;
  uint32_t byteRate;
  uint16_t blockAlign;
  uint16_t bitsPerSample;
  uint32_t subchunk2Id;
  uint32_t subchunk2Size;
} waveheader_t;

#define RIFF_CHUNK_ID     0x46464952
#define WAVE_FORMAT       0x45564157
#define FORMAT_CHUNK_ID   0x20746d66
#define DATA_CHUNK_ID     0x61746164
#define FORMAT_CHUNK_SIZE 16
#define AUDIO_FORMAT_PCM  1


char const* wavefile_load(source_t *source, char const *filename) {
  int fileSize;
  char const *err = 0;
  uint8_t *waveData = filesystem_read(filename, &fileSize);

  if(waveData == NULL) {
    err = "Could not read WAV file";
    goto fail;
  }

  waveheader_t const* header = (waveheader_t const*)waveData;

  if(header->chunkId != RIFF_CHUNK_ID
      || header->format != WAVE_FORMAT
      || header->subchunk1Id != FORMAT_CHUNK_ID
      || header->subchunk2Id != DATA_CHUNK_ID) {
    err = "Invalid WAV header";
    goto fail;
  }

  if(header->subchunk1Size != FORMAT_CHUNK_SIZE
      || header->audioFormat != AUDIO_FORMAT_PCM) {
    err = "Invalid Audio Format (only PCM supported)";
    goto fail;
  }
  
  if(header->numChannels != 1) {
    err = "Invalid Audio Format (only 1 channel supported)";
    goto fail;
  }

  if(header->sampleRate != 22050) {
    err = "Invalid Audio Format (only 22050Hz supported)";
    goto fail;
  }

  if(header->bitsPerSample != 16) {
    err = "Invalid Audio Format (only 16 Bit supported)";
    goto fail;
  }

  if(header->byteRate != header->sampleRate * header->numChannels * header->bitsPerSample / 8) {
    err = "Byte rate doesn't match header info";
    goto fail;
  }

  if(header->blockAlign != header->numChannels * header->bitsPerSample / 8) {
    err = "Block alignment doesn't match header info";
    goto fail;
  }

  if(fileSize < sizeof(waveheader_t) + header->subchunk2Size) {
    err = "Short read while reading samples";
    goto fail;
  }

  source->sampleCount = header->subchunk2Size / 2;
  source->samples = (int16_t*)dmt_malloc(header->subchunk2Size);

  memcpy(source->samples, ((uint8_t*)waveData) + sizeof(waveheader_t), header->subchunk2Size);

fail:
  filesystem_free(waveData);
  return err;
}
