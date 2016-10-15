/**
 * Copyright (c) 2016 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <stdlib.h>
#include <string.h>
#include <pc.h>
#include <conio.h>
#include <dpmi.h>
#include "luaobj.h"
#include "keyboard.h"

#define KEY_MAX 128
#define BUFFER_SIZE 32
#define BUFFER_MASK (BUFFER_SIZE - 1)

static const char *scancodeMap[] = {
  [  0] = "?",
  [  1] = "escape",
  [  2] = "1",
  [  3] = "2",
  [  4] = "3",
  [  5] = "4",
  [  6] = "5",
  [  7] = "6",
  [  8] = "7",
  [  9] = "8",
  [ 10] = "9",
  [ 11] = "0",
  [ 12] = "-",
  [ 13] = "=",
  [ 14] = "backspace",
  [ 15] = "tab",
  [ 16] = "q",
  [ 17] = "w",
  [ 18] = "e",
  [ 19] = "r",
  [ 20] = "t",
  [ 21] = "y",
  [ 22] = "u",
  [ 23] = "i",
  [ 24] = "o",
  [ 25] = "p",
  [ 26] = "[",
  [ 27] = "]",
  [ 28] = "return",
  [ 29] = "lctrl",
  [ 30] = "a",
  [ 31] = "s",
  [ 32] = "d",
  [ 33] = "f",
  [ 34] = "g",
  [ 35] = "h",
  [ 36] = "j",
  [ 37] = "k",
  [ 38] = "l",
  [ 39] = ";",
  [ 40] = "\"",
  [ 41] = "`",
  [ 42] = "lshift",
  [ 43] = "\\",
  [ 44] = "z",
  [ 45] = "x",
  [ 46] = "c",
  [ 47] = "v",
  [ 48] = "b",
  [ 49] = "n",
  [ 50] = "m",
  [ 51] = ",",
  [ 52] = ".",
  [ 53] = "/",
  [ 54] = "rshift",
  [ 55] = "*",
  [ 56] = "lalt",
  [ 57] = "space",
  [ 58] = "capslock",
  [ 59] = "f1",
  [ 60] = "f2",
  [ 61] = "f3",
  [ 62] = "f4",
  [ 63] = "f5",
  [ 64] = "f6",
  [ 65] = "f7",
  [ 66] = "f8",
  [ 67] = "f9",
  [ 68] = "f10",
  [ 69] = "numlock",
  [ 70] = "scrolllock",
  [ 71] = "home",
  [ 72] = "up",
  [ 73] = "pageup",
  [ 74] = "-",
  [ 75] = "left",
  [ 76] = "5",
  [ 77] = "right",
  [ 78] = "+",
  [ 79] = "end",
  [ 80] = "down",
  [ 81] = "pagedown",
  [ 82] = "insert",
  [ 83] = "delete",
  [ 84] = "?",
  [ 85] = "?",
  [ 86] = "?",
  [ 87] = "?",
  [ 88] = "f12",
  [ 89] = "?",
  [ 90] = "?",
  [ 91] = "?",
  [ 92] = "?",
  [ 93] = "?",
  [ 94] = "?",
  [ 95] = "?",
  [ 96] = "enter",
  [ 97] = "lctrl",
  [ 98] = "/",
  [ 99] = "f12",
  [100] = "rctrl",
  [101] = "pause",
  [102] = "home",
  [103] = "up",
  [104] = "pageup",
  [105] = "left",
  [106] = "right",
  [107] = "end",
  [108] = "down",
  [109] = "pagedown",
  [110] = "insert",
  [111] = "?",
  [112] = "?",
  [113] = "?",
  [114] = "?",
  [115] = "?",
  [116] = "?",
  [117] = "?",
  [118] = "?",
  [119] = "pause",
  [120] = "?",
  [121] = "?",
  [122] = "?",
  [123] = "?",
  [124] = "?",
  [125] = "?",
  [126] = "?",
  [127] = "?",
  [128] = NULL,
};


volatile int keyboard_allowKeyRepeat = 0;
volatile char keyboard_keyStates[KEY_MAX];

typedef struct { unsigned char type, code, isrepeat; } KeyEvent;

volatile struct {
  KeyEvent data[32];
  int readi, writei;
} keyboard_events;

_go32_dpmi_seginfo old_keyb_handler_seginfo, new_keyb_handler_seginfo;


void keyboard_handler() {
  static unsigned char code;
  code = inportb(0x60);

  if (code != 224) {
    volatile KeyEvent *e;
    /* Handle key up / down */
    if (code & (1 << 7)) {
      /* Key up */
      code &= ~(1 << 7);
      keyboard_keyStates[code] = 0;
      e = &keyboard_events.data[keyboard_events.writei & BUFFER_MASK];
      e->type = KEYBOARD_RELEASED;
      e->code = code;
      e->isrepeat = 0;
      keyboard_events.writei++;

    } else {
      /* Key down */
      int isrepeat = keyboard_keyStates[code];
      if (!isrepeat || keyboard_allowKeyRepeat) {
        keyboard_keyStates[code] = 1;
        e = &keyboard_events.data[keyboard_events.writei & BUFFER_MASK];
        e->type = KEYBOARD_PRESSED;
        e->code = code;
        e->isrepeat = isrepeat;
        keyboard_events.writei++;
      }
    }
  }

  outportb(0x20, 0x20);
}


void keyboard_handler_end() {}


int keyboard_init(void) {
  _go32_dpmi_lock_data((char*)&keyboard_keyStates, KEY_MAX);
  _go32_dpmi_lock_data((char*)&keyboard_events, sizeof(keyboard_events));
  _go32_dpmi_lock_code(keyboard_handler,(unsigned long)keyboard_handler_end -
                       (unsigned long)keyboard_handler);
  _go32_dpmi_get_protected_mode_interrupt_vector(9, &old_keyb_handler_seginfo);
  new_keyb_handler_seginfo.pm_offset = (int)keyboard_handler;
  _go32_dpmi_chain_protected_mode_interrupt_vector(9, &new_keyb_handler_seginfo);
  return 0;
}


void keyboard_deinit(void) {
  _go32_dpmi_set_protected_mode_interrupt_vector(9, &old_keyb_handler_seginfo);
}


void keyboard_setKeyRepeat(int allow) {
  keyboard_allowKeyRepeat = allow;
}


int keyboard_isDown(const char *key) {
  int i;
  for (i = 0; scancodeMap[i]; i++) {
    if (!strcmp(scancodeMap[i], key)) {
      return keyboard_keyStates[i];
    }
  }
  return 0;
}


int keyboard_poll(keyboard_Event *e) {

  /* Handle key press / release */
  if (keyboard_events.readi != keyboard_events.writei) {
    KeyEvent ke = keyboard_events.data[keyboard_events.readi & BUFFER_MASK];
    e->type = ke.type;
    e->code = ke.code;
    e->key = scancodeMap[ke.code];
    e->isrepeat = ke.isrepeat;
    keyboard_events.readi++;
    return 1;
  }

  /* Handle text input */
  char buf[64];
  int i = 0;
  while ( kbhit() ) {
    int chr = getch();
    if (chr == 0) { /* Discard "special" keys */
      getch();
    } else if (chr >= 32) {
      buf[i++] = chr;
    }
  }
  if (i > 0) {
    e->type = KEYBOARD_TEXTINPUT;
    memcpy(e->text, buf, i);
    e->text[i] = '\0';
    return 1;
  }

  return 0;
}
