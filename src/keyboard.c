/**
 * Copyright (c) 2016 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <string.h>
#include <pc.h>
#include <dpmi.h>
#include "luaobj.h"
#include "keyboard.h"

#define KEYBOARD_KEY_MAX 256
volatile char keyboard_keyStates[KEYBOARD_KEY_MAX];

volatile struct {
  struct { unsigned char type; unsigned char code; } data[32];
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
    /* Handle key up / down */
    if (code & (1 << 7)) {
      /* Key up */
      code &= ~(1 << 7);
      code |= extended;
      keyboard_keyStates[code] = 0;
      keyboard_events.data[keyboard_events.writei & 31].code = code;
      keyboard_events.data[keyboard_events.writei & 31].type = 0;
      keyboard_events.writei++;

    } else {
      /* Key down */
      code |= extended;
      if (!keyboard_keyStates[code]) { /* Ignore key repeat */
        keyboard_keyStates[code] = 1;
        keyboard_events.data[keyboard_events.writei & 31].code = code;
        keyboard_events.data[keyboard_events.writei & 31].type = 1;
        keyboard_events.writei++;
      }
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
  if (_go32_dpmi_allocate_iret_wrapper(&new_keyb_handler_seginfo) != 0) {
    return 1;
  }
  if (_go32_dpmi_set_protected_mode_interrupt_vector(
        9, &new_keyb_handler_seginfo) != 0
  ) {
    _go32_dpmi_free_iret_wrapper(&new_keyb_handler_seginfo);
    return 1;
  }
  return 0;
}


void keyboard_deinit(void) {
  _go32_dpmi_set_protected_mode_interrupt_vector(9, &old_keyb_handler_seginfo);
  _go32_dpmi_free_iret_wrapper(&new_keyb_handler_seginfo);
}


/**
 * Lua binds
 */


int l_keyboard_isDown(lua_State *L) {
  int code = luaL_checknumber(L, 1);
  if (code < 0 || code >= KEYBOARD_KEY_MAX) return 0;
  lua_pushboolean(L, keyboard_keyStates[code]);
  return 1;
}


int l_keyboard_getEvents(lua_State *L) {
  lua_newtable(L);
  int idx = 1;
  while (keyboard_events.readi != keyboard_events.writei) {
    lua_newtable(L);
    lua_pushstring(L, keyboard_events.data[keyboard_events.readi & 31].type
                      ? "down" : "up");
    lua_setfield(L, -2, "type");
    lua_pushnumber(L, keyboard_events.data[keyboard_events.readi & 31].code);
    lua_setfield(L, -2, "code");
    lua_rawseti(L, -2, idx);
    keyboard_events.readi++;
    idx++;
  }
  return 1;
}


int luaopen_keyboard(lua_State *L) {
  luaL_Reg reg[] = {
    { "getEvents",  l_keyboard_getEvents  },
    { "isDown",     l_keyboard_isDown     },
    { 0, 0 },
  };
  luaL_newlib(L, reg);
  return 1;
}
