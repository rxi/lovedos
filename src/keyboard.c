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

#define KEYBOARD_KEY_MAX 256
#define BUFFER_SIZE 32
#define BUFFER_MASK (BUFFER_SIZE - 1)

enum { KEY_UP, KEY_DOWN };

typedef struct { unsigned char type, code, isrepeat; } KeyEvent;

volatile char keyboard_keyStates[KEYBOARD_KEY_MAX];

volatile struct {
  KeyEvent data[32];
  int readi, writei;
} keyboard_events;

_go32_dpmi_seginfo old_keyb_handler_seginfo, new_keyb_handler_seginfo;


void keyboard_handler() {
  static unsigned char code, extended;
  code = inportb(0x60);

  if (code == 224) {
    /* Handle extended code */
    extended = 1 << 7;

  } else {
    volatile KeyEvent *e;
    /* Handle key up / down */
    if (code & (1 << 7)) {
      /* Key up */
      code &= ~(1 << 7);
      code |= extended;
      e = &keyboard_events.data[keyboard_events.writei & BUFFER_MASK];
      e->code = code;
      e->type = KEY_UP;
      e->isrepeat = 0;
      keyboard_events.writei++;
      keyboard_keyStates[code] = 0;

    } else {
      /* Key down */
      code |= extended;
      e = &keyboard_events.data[keyboard_events.writei & BUFFER_MASK];
      e->code = code;
      e->type = KEY_DOWN;
      e->isrepeat = keyboard_keyStates[code];
      keyboard_events.writei++;
      keyboard_keyStates[code] = 1;
    }
    extended = 0x0;
  }

  outportb(0x20, 0x20);
}


void keyboard_handler_end() {}


int keyboard_init(void) {
  _go32_dpmi_lock_data((char*)&keyboard_keyStates, KEYBOARD_KEY_MAX);
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


/**
 * Lua binds
 */

const char *scancodeMap[] = {
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
  [ 79] = "end",
  [ 78] = "+",
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


int l_keyboard_isDown(lua_State *L) {
  int n = lua_gettop(L);
  int res = 0;
  int i;
  for (i = 1; i <= n; i++) {
    int code = luaL_checkoption(L, 1, NULL, scancodeMap);
    res |= keyboard_keyStates[code];
  }
  lua_pushboolean(L, res);
  return 1;
}


int l_keyboard_poll(lua_State *L) {
  lua_newtable(L);
  int idx = 1;

  /* Handle key presses / releases */
  while (keyboard_events.readi != keyboard_events.writei) {
    lua_newtable(L);

    KeyEvent e = keyboard_events.data[keyboard_events.readi & BUFFER_MASK];
    e.code &= 127;

    lua_pushstring(L, e.type == KEY_DOWN ? "down" : "up");
    lua_setfield(L, -2, "type");
    lua_pushnumber(L, e.code);
    lua_setfield(L, -2, "code");
    lua_pushstring(L, scancodeMap[e.code]);
    lua_setfield(L, -2, "key");
    if (e.type == KEY_DOWN) {
      lua_pushboolean(L, e.isrepeat);
      lua_setfield(L, -2, "isrepeat");
    }

    lua_rawseti(L, -2, idx++);
    keyboard_events.readi++;
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
    lua_newtable(L);
    lua_pushstring(L, "text");
    lua_setfield(L, -2, "type");
    lua_pushlstring(L, buf, i);
    lua_setfield(L, -2, "text");
    lua_rawseti(L, -2, idx++);
  }

  return 1;
}


int luaopen_keyboard(lua_State *L) {
  luaL_Reg reg[] = {
    { "poll",       l_keyboard_poll       },
    { "isDown",     l_keyboard_isDown     },
    { 0, 0 },
  };
  luaL_newlib(L, reg);
  return 1;
}
