/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <string.h>
#include "lib/cmixer/cmixer.h"
#include "filesystem.h"
#include "luaobj.h"


#define CLASS_TYPE  LUAOBJ_TYPE_SOURCE
#define CLASS_NAME  "Source"


typedef struct {
  cm_Source *source;
  void *data;
} source_t;


int l_source_new(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  /* Create object */
  source_t *self = luaobj_newudata(L, sizeof(*self));
  luaobj_setclass(L, CLASS_TYPE, CLASS_NAME);
  memset(self, 0, sizeof(*self));
  /* Load file */
  int size;
  self->data = filesystem_read(filename, &size);
  if (!self->data) {
    luaL_error(L, "could not open file");
  }
  /* Init source */
  self->source = cm_new_source_from_mem(self->data, size);
  if (!self->source) {
    luaL_error(L, "%s", cm_get_error());
  }
  /* Return object */
  return 1;
}


int l_source_gc(lua_State *L) {
  source_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  if (self->source) cm_destroy_source(self->source);
  if (self->data) filesystem_free(self->data);
  return 0;
}


int l_source_setVolume(lua_State *L) {
  source_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  double n = luaL_checknumber(L, 2);
  cm_set_gain(self->source, n);
  return 0;
}


int l_source_setPitch(lua_State *L) {
  source_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  double n = luaL_checknumber(L, 2);
  cm_set_pitch(self->source, n);
  return 0;
}


int l_source_setLooping(lua_State *L) {
  source_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  int enable = lua_toboolean(L, 2);
  cm_set_loop(self->source, enable);
  return 0;
}


int l_source_getDuration(lua_State *L) {
  source_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  double n = cm_get_length(self->source);
  lua_pushnumber(L, n);
  return 1;
}


int l_source_isPlaying(lua_State *L) {
  source_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  lua_pushboolean(L, cm_get_state(self->source) == CM_STATE_PLAYING);
  return 1;
}


int l_source_isPaused(lua_State *L) {
  source_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  lua_pushboolean(L, cm_get_state(self->source) == CM_STATE_PAUSED);
  return 1;
}


int l_source_isStopped(lua_State *L) {
  source_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  lua_pushboolean(L, cm_get_state(self->source) == CM_STATE_STOPPED);
  return 1;
}


int l_source_tell(lua_State *L) {
  source_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  double n = cm_get_position(self->source);
  lua_pushnumber(L, n);
  return 1;
}


int l_source_play(lua_State *L) {
  source_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  cm_play(self->source);
  return 0;
}


int l_source_pause(lua_State *L) {
  source_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  cm_pause(self->source);
  return 0;
}


int l_source_stop(lua_State *L) {
  source_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  cm_stop(self->source);
  return 0;
}


int luaopen_source(lua_State *L) {
  luaL_Reg reg[] = {
    { "new",            l_source_new            },
    { "__gc",           l_source_gc             },
    { "setVolume",      l_source_setVolume      },
    { "setPitch",       l_source_setPitch       },
    { "setLooping",     l_source_setLooping     },
    { "getDuration",    l_source_getDuration    },
    { "isPlaying",      l_source_isPlaying      },
    { "isPaused",       l_source_isPaused       },
    { "isStopped",      l_source_isStopped      },
    { "tell",           l_source_tell           },
    { "play",           l_source_play           },
    { "pause",          l_source_pause          },
    { "stop",           l_source_stop           },
    { 0, 0 },
  };
  luaobj_newclass(L, CLASS_NAME, NULL, l_source_new, reg);
  return 1;
}
