/** 
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#ifndef LUAOBJ_H
#define LUAOBJ_H

#include <stdint.h>

#include "lib/lua/lua.h"
#include "lib/lua/lualib.h"
#include "lib/lua/lauxlib.h"


typedef struct {
  uint32_t type;
} luaobj_head_t;


/* Each mask should consist of its unique bit and the unique bit of all its
 * super classes */
#define LUAOBJ_TYPE_IMAGE  (1 << 0)
#define LUAOBJ_TYPE_QUAD   (1 << 1)
#define LUAOBJ_TYPE_FONT   (1 << 2)
#define LUAOBJ_TYPE_SOURCE (1 << 3)


int luaobj_newclass(lua_State *L, const char *name, const char *extends, 
                    int (*constructor)(lua_State*), luaL_Reg* reg);
void luaobj_setclass(lua_State *L, uint32_t type, char *name);
void *luaobj_newudata(lua_State *L, int size);
void *luaobj_checkudata(lua_State *L, int index, uint32_t type);


#endif
