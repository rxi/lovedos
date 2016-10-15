/**
 * Copyright (c) 2016 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

 #include "mouse.h"
 #include "luaobj.h"


int l_mouse_getPosition(lua_State *L) {
  mouse_State *state = mouse_getState();
  lua_pushinteger(L, state->x);
  lua_pushinteger(L, state->y);
  return 2;
}


int l_mouse_getX(lua_State *L) {
  mouse_State *state = mouse_getState();
  lua_pushinteger(L, state->x);
  return 1;
}


int l_mouse_getY(lua_State *L) {
  mouse_State *state = mouse_getState();
  lua_pushinteger(L, state->y);
  return 1;
}


int l_mouse_isDown(lua_State *L) {
  int n = lua_gettop(L);
  int res = 0;
  int i;
  mouse_State *state = mouse_getState();
  for (i = 1; i <= n; i++) {
    int idx = luaL_checknumber(L, i) - 1;
    if (idx >= 0 && idx < MOUSE_BUTTON_MAX) {
      res |= state->buttonsDown[idx];
    }
  }
  lua_pushboolean(L, res);
  return 1;
}


int l_mouse_poll(lua_State *L) {
  mouse_update();
  mouse_State *state = mouse_getState();

  int i;
  int n = 1;
  lua_newtable(L);

  /* Add motion event if mouse moved */
  if ( state->x != state->lastX || state->y != state->lastY ) {
    lua_newtable(L);
    lua_pushstring(L, "motion");
    lua_setfield(L, -2, "type");
    lua_pushinteger(L, state->x);
    lua_setfield(L, -2, "x");
    lua_pushinteger(L, state->y);
    lua_setfield(L, -2, "y");
    lua_pushinteger(L, state->x - state->lastX);
    lua_setfield(L, -2, "dx");
    lua_pushinteger(L, state->y - state->lastY);
    lua_setfield(L, -2, "dy");
    lua_rawseti(L, -2, n++);
  }

  /* Add `pressed` and `released` events */
  for (i = 0; i < MOUSE_BUTTON_MAX; i++) {
    if ( state->buttonsPressed[i] || state->buttonsReleased[i] ) {
      lua_newtable(L);
      lua_pushstring(L, state->buttonsPressed[i] ? "pressed" : "released");
      lua_setfield(L, -2, "type");
      lua_pushinteger(L, i + 1);
      lua_setfield(L, -2, "button");
      lua_pushinteger(L, state->x);
      lua_setfield(L, -2, "x");
      lua_pushinteger(L, state->y);
      lua_setfield(L, -2, "y");
      lua_rawseti(L, -2, n++);
    }
  }

  return 1;
}


int luaopen_mouse(lua_State *L) {
  luaL_Reg reg[] = {
    { "poll",         l_mouse_poll          },
    { "getPosition",  l_mouse_getPosition   },
    { "getX",         l_mouse_getX          },
    { "getY",         l_mouse_getY          },
    { "isDown",       l_mouse_isDown        },
    { 0, 0 },
  };
  luaL_newlib(L, reg);
  return 1;
}
