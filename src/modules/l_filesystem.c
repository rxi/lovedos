/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include "filesystem.h"
#include "luaobj.h"


int l_filesystem_mount(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  int err = filesystem_mount(path);
  if (err) {
    lua_pushnil(L);
    lua_pushstring(L, filesystem_strerror(err));
    return 2;
  }
  lua_pushboolean(L, 1);
  return 1;
}


int l_filesystem_unmount(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  int err = filesystem_unmount(path);
  if (err) {
    lua_pushnil(L);
    lua_pushstring(L, filesystem_strerror(err));
    return 2;
  }
  lua_pushboolean(L, 1);
  return 1;
}


int l_filesystem_exists(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  lua_pushboolean( L, filesystem_exists(filename) );
  return 1;
}


int l_filesystem_isFile(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  lua_pushboolean( L, filesystem_isFile(filename) );
  return 1;
}


int l_filesystem_isDirectory(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  lua_pushboolean( L, filesystem_isDirectory(filename) );
  return 1;
}


int l_filesystem_read(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  int size;
  void *data = filesystem_read(filename, &size);
  if (!data) {
    luaL_error(L, "could not read file");
  }
  lua_pushlstring(L, data, size);
  filesystem_free(data);
  return 1;
}


int l_filesystem_setWriteDir(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  int err = filesystem_setWriteDir(path);
  if (err) {
    lua_pushnil(L);
    lua_pushstring(L, filesystem_strerror(err));
    return 2;
  }
  lua_pushboolean(L, 1);
  return 1;
}


int l_filesystem_write(lua_State *L) {
  size_t sz;
  const char *filename = luaL_checkstring(L, 1);
  const char *data = luaL_tolstring(L, 2, &sz);
  int err = filesystem_write(filename, data, sz);
  if (err) {
    luaL_error(L, "%s", filesystem_strerror(err));
  }
  lua_pushboolean(L, 1);
  return 1;
}


int luaopen_filesystem(lua_State *L) {
  luaL_Reg reg[] = {
    { "mount",        l_filesystem_mount        },
    { "unmount",      l_filesystem_unmount      },
    { "exists",       l_filesystem_exists       },
    { "isFile",       l_filesystem_isFile       },
    { "isDirectory",  l_filesystem_isDirectory  },
    { "read",         l_filesystem_read         },
    { "setWriteDir",  l_filesystem_setWriteDir  },
    { "write",        l_filesystem_write        },
    { 0, 0 },
  };
  luaL_newlib(L, reg);
  return 1;
}
