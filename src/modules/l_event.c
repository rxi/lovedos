/**
 * Copyright (c) 2016 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include "luaobj.h"
#include "event.h"


int l_event_pump(lua_State *L) {
  event_pump();
  return 0;
}


int l_event_poll(lua_State *L) {
  event_t e;
  if (event_poll(&e)) {
    lua_pushstring(L, event_typestr(e.type));

    switch(e.type) {
      case EVENT_KEYBOARD_PRESSED:
      case EVENT_KEYBOARD_RELEASED:
        lua_pushstring(L, e.keyboard.key);
        lua_pushstring(L, e.keyboard.key);
        lua_pushboolean(L, e.keyboard.isrepeat);
        return 4;

      case EVENT_KEYBOARD_TEXTINPUT:
        lua_pushstring(L, e.keyboard.text);
        return 2;

      case EVENT_MOUSE_MOVED:
        lua_pushnumber(L, e.mouse.x);
        lua_pushnumber(L, e.mouse.y);
        lua_pushnumber(L, e.mouse.dx);
        lua_pushnumber(L, e.mouse.dy);
        return 5;

      case EVENT_MOUSE_PRESSED:
      case EVENT_MOUSE_RELEASED:
        lua_pushnumber(L, e.mouse.x);
        lua_pushnumber(L, e.mouse.y);
        lua_pushnumber(L, e.mouse.button);
        return 4;
    }
  }

  return 0;
}


int luaopen_event(lua_State *L) {
  luaL_Reg reg[] = {
    { "pump",  l_event_pump  },
    { "poll",  l_event_poll  },
    { 0, 0 },
  };
  luaL_newlib(L, reg);
  return 1;
}
