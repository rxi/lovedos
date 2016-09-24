/**
 * Copyright (c) 2016 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include "luaobj.h"
#include "mouse.h"

int mouse_inited;
int mouse_x, mouse_y;
int mouse_lastX, mouse_lastY;
int mouse_buttonsPressed[MOUSE_BUTTON_MAX];
int mouse_buttonsReleased[MOUSE_BUTTON_MAX];
int mouse_buttonsDown[MOUSE_BUTTON_MAX];


void mouse_init(void) {
  union REGS regs = {};
  regs.x.ax = 0;
  int86(0x33, &regs, &regs);
  mouse_inited = regs.x.ax ? 1 : 0;
}


void mouse_update(void) {
  if (!mouse_inited) {
    return;
  }

  /* Update last mouse position */
  mouse_lastX = mouse_x;
  mouse_lastY = mouse_y;

  /* Update mouse position */
  union REGS regs = {};
  regs.x.ax = 3;
  int86(0x33, &regs, &regs);
  mouse_x = regs.x.cx >> 1;
  mouse_y = regs.x.dx;

  /* Update button states */
  int i;
  for (i = 0; i < MOUSE_BUTTON_MAX; i++) {
    /* Reset pressed/released */
    mouse_buttonsPressed[i] = 0;
    mouse_buttonsReleased[i] = 0;
    /* Handle mouse pressed */
    memset(&regs, 0, sizeof(regs));
    regs.x.ax = 5;
    regs.x.bx = i;
    int86(0x33, &regs, &regs);
    if (regs.x.bx) {
      mouse_buttonsPressed[i] = 1;
      mouse_buttonsDown[i] = 1;
    }
    /* Handle mouse released */
    memset(&regs, 0, sizeof(regs));
    regs.x.ax = 6;
    regs.x.bx = i;
    int86(0x33, &regs, &regs);
    if (regs.x.bx) {
      mouse_buttonsReleased[i] = 1;
      mouse_buttonsDown[i] = 0;
    }
  }
}


/**
 * Lua binds
 */


int l_mouse_getPosition(lua_State *L) {
  lua_pushinteger(L, mouse_x);
  lua_pushinteger(L, mouse_y);
  return 2;
}


int l_mouse_getX(lua_State *L) {
  lua_pushinteger(L, mouse_x);
  return 1;
}


int l_mouse_getY(lua_State *L) {
  lua_pushinteger(L, mouse_y);
  return 1;
}


int l_mouse_isDown(lua_State *L) {
  int n = lua_gettop(L);
  int res = 0;
  int i;
  for (i = 1; i <= n; i++) {
    int idx = luaL_checkinteger(L, i) - 1;
    if (idx >= 0 && idx < MOUSE_BUTTON_MAX) {
      res |= mouse_buttonsDown[idx];
    }
  }
  lua_pushboolean(L, res);
  return 1;
}


int l_mouse_update(lua_State *L) {
  mouse_update();
  return 0;
}


int l_mouse_getEvents(lua_State *L) {
  return 0;
}


int luaopen_mouse(lua_State *L) {
  luaL_Reg reg[] = {
    { "update",       l_mouse_update        },
    { "getEvents",    l_mouse_getEvents     },
    { "getPosition",  l_mouse_getPosition   },
    { "getX",         l_mouse_getX          },
    { "getY",         l_mouse_getY          },
    { "isDown",       l_mouse_isDown        },
    { 0, 0 },
  };
  luaL_newlib(L, reg);
  return 1;
}
