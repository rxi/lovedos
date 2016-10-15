/**
 * Copyright (c) 2016 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <dos.h>
#include <time.h>
#include "luaobj.h"
#include "image.h"
#include "vga.h"

long long timer_lastStep;
double timer_lastDt;

double  timer_avgLastDt;
double  timer_avgAcc = 1;
int     timer_avgCount;
double  timer_avgTimer;


int l_timer_step(lua_State *L) {
  /* Do delta */
  long long now;
  /* Sometimes a call to uclock() will return a slightly earlier time than the
   * previous call, resulting in a negative delta time. The below loop keeps
   * trying for a proper value if this occurs. */
  do {
    now = uclock();
    timer_lastDt = (now - timer_lastStep) / (double) UCLOCKS_PER_SEC;
  } while (timer_lastDt < 0);
  timer_lastStep = now;
  /* Do average */
  timer_avgAcc += timer_lastDt;
  timer_avgCount++;
  timer_avgTimer -= timer_lastDt;
  if (timer_avgTimer <= 0) {
    timer_avgLastDt = (timer_avgAcc / timer_avgCount);
    timer_avgTimer += 1;
    timer_avgAcc = 0;
    timer_avgCount = 0;
  }
  return 0;
}


int l_timer_sleep(lua_State *L) {
  delay(luaL_checknumber(L, 1) * 1000.);
  return 1;
}


int l_timer_getDelta(lua_State *L) {
  lua_pushnumber(L, timer_lastDt);
  return 1;
}


int l_timer_getAverageDelta(lua_State *L) {
  lua_pushnumber(L, timer_avgLastDt);
  return 1;
}


int l_timer_getFPS(lua_State *L) {
  lua_pushnumber(L, (int)(1. / timer_avgLastDt));
  return 1;
}


int l_timer_getTime(lua_State *L) {
  lua_pushnumber(L, uclock() / (double) UCLOCKS_PER_SEC);
  return 1;
}


int luaopen_timer(lua_State *L) {
  luaL_Reg reg[] = {
    { "step",             l_timer_step              },
    { "sleep",            l_timer_sleep             },
    { "getDelta",         l_timer_getDelta          },
    { "getAverageDelta",  l_timer_getAverageDelta   },
    { "getFPS",           l_timer_getFPS            },
    { "getTime",          l_timer_getTime           },
    { 0, 0 },
  };
  luaL_newlib(L, reg);
  return 1;
}
