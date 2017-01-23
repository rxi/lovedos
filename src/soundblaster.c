/**
 * Copyright (c) 2017 Florian Kesseler
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <dos.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/nearptr.h>
#include "soundblaster.h"

#define BYTE(val, byte) (((val) >> ((byte) * 8)) & 0xFF)

#define SAMPLE_BUFFER_SIZE (SOUNDBLASTER_SAMPLES_PER_BUFFER * sizeof(uint16_t) * 2)

// SB16
#define BLASTER_RESET_PORT                   0x6
#define BLASTER_READ_PORT                    0xA
#define BLASTER_WRITE_PORT                   0xC
#define BLASTER_READ_BUFFER_STATUS_PORT      0xE
#define BLASTER_INTERRUPT_ACKNOWLEDGE_8BIT   0xE
#define BLASTER_INTERRUPT_ACKNOWLEDGE_16BIT  0xF
#define BLASTER_MIXER_OUT_PORT               0x4
#define BLASTER_MIXER_IN_PORT                0x5
#define BLASTER_MIXER_INTERRUPT_STATUS       0x82
#define BLASTER_16BIT_INTERRUPT              0x02
#define BLASTER_READ_BUFFER_STATUS_AVAIL     0x80
#define BLASTER_WRITE_BUFFER_STATUS_UNAVAIL  0x80
#define BLASTER_READY_BYTE                   0xAA
#define BLASTER_SET_OUTPUT_SAMPLING_RATE     0x41
#define BLASTER_PROGRAM_16BIT_IO_CMD         0xB0
#define BLASTER_PROGRAM_16BIT_FLAG_FIFO      0x02
#define BLASTER_PROGRAM_16BIT_FLAG_AUTO_INIT 0x04
#define BLASTER_PROGRAM_16BIT_FLAG_INPUT     0x08
#define BLASTER_PROGRAM_STEREO               0x20
#define BLASTER_PROGRAM_SIGNED               0x10
#define BLASTER_SPEAKER_ON_CMD               0xD1
#define BLASTER_SPEAKER_OFF_CMD              0xD3
#define BLASTER_EXIT_AUTO_DMA                0xD9


// PIC
#define PIC1_COMMAND   0x20
#define PIC2_COMMAND   0xA0
#define PIC1_DATA      0x21
#define PIC2_DATA      0xA1
#define PIC_EOI        0x20
#define PIC_IRQ07_MAP  0x08
#define PIC_IRQ8F_MAP  0x70


// DMA
#define DMA_DIRECTION_READ_FROM_MEMORY 0x04
#define DMA_TRANSFER_MODE_BLOCK        0x80


static const struct  {
  uint8_t startAddressRegister;
  uint8_t countRegister;
  uint8_t singleChannelMaskRegister;
  uint8_t modeRegister;
  uint8_t flipFlopResetRegister;
  uint8_t pageRegister;
} dmaRegisters[] = {
  { 0x00, 0x01, 0x0A, 0x0B, 0x0C, 0x87 },
  { 0x02, 0x03, 0x0A, 0x0B, 0x0C, 0x83 },
  { 0x04, 0x05, 0x0A, 0x0B, 0x0C, 0x81 },
  { 0x06, 0x07, 0x0A, 0x0B, 0x0C, 0x82 },
  { 0xC0, 0xC2, 0xD4, 0xD6, 0xD8, 0x8F },
  { 0xC4, 0xC6, 0xD4, 0xD6, 0xD8, 0x8B },
  { 0xC8, 0xCA, 0xD4, 0xD6, 0xD8, 0x89 },
  { 0xCC, 0xCE, 0xD4, 0xD6, 0xD8, 0x8A }
};

static volatile int  stopDma = 0;
static uint16_t     *sampleBuffer;
static int           sampleBufferSelector;
static uint16_t      baseAddress;
static uint16_t      irq;
static uint16_t      dmaChannel;
static bool          isrInstalled = false;
static int           writePage = 0;
static bool          blasterInitialized = false;
static _go32_dpmi_seginfo oldBlasterHandler, newBlasterHandler;
static soundblaster_getSampleProc getSamples;


static void writeDSP(uint8_t value) {
  while((inportb(baseAddress + BLASTER_WRITE_PORT) & 
          BLASTER_WRITE_BUFFER_STATUS_UNAVAIL) != 0) {}

  outportb(baseAddress + BLASTER_WRITE_PORT, value);
}


static uint8_t readDSP() {
  uint8_t status;
  while(((status = inportb(baseAddress + BLASTER_READ_BUFFER_STATUS_PORT)) 
        & BLASTER_READ_BUFFER_STATUS_AVAIL) == 0) {
  }

  return inportb(baseAddress + BLASTER_READ_PORT);
}


static inline void delay3us() {
  uint64_t waited = 0;
  uclock_t lastTime = uclock();
  while(waited < (3*UCLOCKS_PER_SEC) / 1000000) {
    uclock_t nowTime = uclock();

    // Just ignore timer wraps. In the worst case we get a slightly
    // longer delay, but who cares?
    if(nowTime > lastTime) {
      waited += nowTime - lastTime;
    }

    lastTime = nowTime;
  }
}


static int resetBlaster(void) {
  for(int j = 0; j < 1000; ++j) {
    outportb(baseAddress + BLASTER_RESET_PORT, 1);
    delay3us();
    outportb(baseAddress + BLASTER_RESET_PORT, 0);

    if(readDSP() == BLASTER_READY_BYTE) {
      return 0;
    }
  }
  return SOUNDBLASTER_RESET_ERROR;
}


static void soundblasterISR(void) {
  outportb(baseAddress + BLASTER_MIXER_OUT_PORT, 
           BLASTER_MIXER_INTERRUPT_STATUS);
  uint8_t status = inportb(baseAddress + BLASTER_MIXER_IN_PORT);

  if(status & BLASTER_16BIT_INTERRUPT) {
    if(stopDma == 1) {
      writeDSP(BLASTER_EXIT_AUTO_DMA);
      stopDma = 2;
    } else {
      uint8_t* dst = (uint8_t*)(sampleBuffer)
        + writePage * SAMPLE_BUFFER_SIZE / 2;

      memcpy(dst, getSamples(), SAMPLE_BUFFER_SIZE / 2);

      writePage = 1 - writePage;
      inportb(baseAddress + BLASTER_INTERRUPT_ACKNOWLEDGE_16BIT);
    }
  }

  if(irq >= 8) {
    outportb(PIC2_COMMAND, PIC_EOI);
  }
  outportb(PIC1_COMMAND, PIC_EOI);
}


static void setBlasterISR(void) {
  // Map IRQ to interrupt number on the CPU
  uint16_t interruptVector = irq + irq + (irq < 8)
    ? PIC_IRQ07_MAP
    : PIC_IRQ8F_MAP;

  _go32_dpmi_get_protected_mode_interrupt_vector(interruptVector, 
                                                 &oldBlasterHandler);

  newBlasterHandler.pm_offset = (int)soundblasterISR;
  newBlasterHandler.pm_selector = _go32_my_cs();
  _go32_dpmi_chain_protected_mode_interrupt_vector(interruptVector, &newBlasterHandler);

  // PIC: unmask SB IRQ
  if(irq < 8) {
    uint8_t irqmask = inportb(PIC1_DATA);
    outportb(PIC1_DATA, irqmask & ~(1<<irq));
  } else {
    uint8_t irqmask = inportb(PIC2_DATA);
    outportb(PIC2_DATA, irqmask & ~(1<<(irq-8)));
  }

  isrInstalled = true;
}


static int parseBlasterSettings(void) {
  char const* blasterEnv = getenv("BLASTER");
  if(blasterEnv == 0) {
    return SOUNDBLASTER_ENV_NOT_SET;
  }

  int res = sscanf(blasterEnv, "A%hx I%hu", &baseAddress, &irq);
  if(res < 2) {
    return SOUNDBLASTER_ENV_INVALID;
  }

  // "H" field may be preceeded by any number of other fields, so let's just search it
  char const *dmaLoc = strchr(blasterEnv, 'H');
  if(dmaLoc == NULL) {
    return SOUNDBLASTER_ENV_INVALID;
  }

  dmaChannel = atoi(dmaLoc+1);

  return 0;
}


static int allocSampleBuffer(void) {
  static int maxRetries = 10;
  int selectors[maxRetries];
  int current;

  for(current = 0; current < maxRetries; ++current) {
    int segment = __dpmi_allocate_dos_memory((SAMPLE_BUFFER_SIZE+15)>>4, &selectors[current]);
    if(segment == -1) {
      break;
    }

    uint32_t bufferPhys = __djgpp_conventional_base + segment * 16;

    // The DMA buffer must not cross a 64k boundary
    if(bufferPhys % 0x10000 < 0x10000 - SAMPLE_BUFFER_SIZE) {
      sampleBuffer = (uint16_t*)bufferPhys;
      memset(sampleBuffer, 0, SAMPLE_BUFFER_SIZE);
      sampleBufferSelector = selectors[current];
      --current;
      break;
    } 
  }

  // Free misaligned buffers
  for(; current > 0; --current) {
    __dpmi_free_dos_memory(selectors[current]);
  }

  if(sampleBuffer == NULL) {
    return SOUNDBLASTER_ALLOC_ERROR;
  }

  return 0;
}


static void turnSpeakerOn(void) {
  writeDSP(BLASTER_SPEAKER_ON_CMD);
}


static void turnSpeakerOff(void) {
  writeDSP(BLASTER_SPEAKER_OFF_CMD);
}


static void dmaSetupTransfer(int      channel,
                             uint8_t  direction,
                             bool     autoReload,
                             bool     down,
                             uint8_t  mode,
                             uint32_t startAddress,
                             uint32_t count) {

  uint8_t modeByte = direction
                   | mode
                   | ((uint8_t)autoReload << 4)
                   | ((uint8_t)down       << 5)
                   | (channel & 0x03);

  uint8_t maskEnable = (channel & 0x03) | 0x04;
    
  uint32_t offset = startAddress;

  // Special handling of 16 bit DMA channels:
  // The DMA controller needs offset and count to be half the actual value and
  // internally doubles it again
  if(channel > 3) {
    offset >>= 1;
    count >>= 1;
  }

  uint8_t page = BYTE(startAddress, 2);

  outportb(dmaRegisters[channel].singleChannelMaskRegister, maskEnable);
  outportb(dmaRegisters[channel].flipFlopResetRegister,     0x00);
  outportb(dmaRegisters[channel].modeRegister,              modeByte);
  outportb(dmaRegisters[channel].startAddressRegister,      BYTE(offset, 0));
  outportb(dmaRegisters[channel].startAddressRegister,      BYTE(offset, 1));
  outportb(dmaRegisters[channel].countRegister,             BYTE(count-1, 0));
  outportb(dmaRegisters[channel].countRegister,             BYTE(count-1, 1));
  outportb(dmaRegisters[channel].pageRegister,              page);
  outportb(dmaRegisters[channel].singleChannelMaskRegister, maskEnable & 0x03);
}


static void startDMAOutput(void) {
  uint32_t offset = ((uint32_t)sampleBuffer) - __djgpp_conventional_base;

  uint32_t samples = SAMPLE_BUFFER_SIZE / sizeof(int16_t);

  dmaSetupTransfer(dmaChannel, DMA_DIRECTION_READ_FROM_MEMORY, true, false,
                   DMA_TRANSFER_MODE_BLOCK, offset, SAMPLE_BUFFER_SIZE);

  // SB16 setup
  writeDSP(BLASTER_SET_OUTPUT_SAMPLING_RATE);
  writeDSP(BYTE(22050, 1));
  writeDSP(BYTE(22050, 0));
  writeDSP(BLASTER_PROGRAM_16BIT_IO_CMD
            | BLASTER_PROGRAM_16BIT_FLAG_AUTO_INIT
            | BLASTER_PROGRAM_16BIT_FLAG_FIFO);
  writeDSP(BLASTER_PROGRAM_SIGNED);
  writeDSP(BYTE(samples/2-1, 0));
  writeDSP(BYTE(samples/2-1, 1));
}


int soundblaster_init(soundblaster_getSampleProc getsamplesproc) {
  if(!__djgpp_nearptr_enable()) {
    return SOUNDBLASTER_DOS_ERROR;
  }

  int err = parseBlasterSettings();
  if(err != 0) {
    fprintf(stderr, "BLASTER environment variable not set or invalid\n");
    return err;
  }

  err = resetBlaster();
  if(err != 0) {
    fprintf(stderr, "Could not reset Soundblaster\n");
    return err;
  }

  err = allocSampleBuffer();
  if(err != 0) {
    fprintf(stderr, "Could not allocate sample buffer in conventional memory\n");
    return err;
  }

  getSamples = getsamplesproc;

  setBlasterISR();
  turnSpeakerOn();
  startDMAOutput();

  blasterInitialized = true;

  return 0;
}


static void deallocSampleBuffer(void) {
  __dpmi_free_dos_memory(sampleBufferSelector);
}


static void resetBlasterISR(void) {
  if(isrInstalled) {
    uint16_t interruptVector = irq + irq + (irq < 8)
      ? PIC_IRQ07_MAP
      : PIC_IRQ8F_MAP;

    _go32_dpmi_set_protected_mode_interrupt_vector(interruptVector, &oldBlasterHandler);
    isrInstalled = false;
  }
}


static void stopDMAOutput(void) {
  stopDma = 1;
  while(stopDma == 1) {}
}


void soundblaster_deinit(void) {
  if(blasterInitialized) {
    turnSpeakerOff();
    stopDMAOutput();
    resetBlaster();
    resetBlasterISR();
    deallocSampleBuffer();
    blasterInitialized = false;
  }
}
