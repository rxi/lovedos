/**
 * Copyright (c) 2017 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <dos.h>

#include "lib/dmt/dmt.h"
#include "lib/cmixer/cmixer.h"
#include "vga.h"
#include "luaobj.h"
#include "keyboard.h"
#include "filesystem.h"
#include "mouse.h"
#include "image.h"
#include "palette.h"
#include "package.h"
#include "soundblaster.h"


static lua_State *L;

static void deinit(void) {
  /* Deinit and clear up everything. Called at exit */
  soundblaster_deinit();
  vga_deinit();
  keyboard_deinit();
  lua_close(L);
  filesystem_deinit();
  if ( dmt_usage() > 0 ) {
    dmt_dump(stdout);
  }
}


static int onLuaPanic(lua_State *L) {
  vga_deinit();
  const char *err = lua_tostring(L, -1);
  printf("lua panic: %s\n", err);
  return 0;
}


static short audioBuffer[SOUNDBLASTER_SAMPLES_PER_BUFFER * 2];

static const short* audioCallback(void) {
  /* For the moment the soundblaster code expects mono audio while the cmixer
  ** library outputs stereo -- we process to a stereo buffer, then copy the left
  ** channel to the start of the buffer */
  int i;
  int len = SOUNDBLASTER_SAMPLES_PER_BUFFER;
  cm_process(audioBuffer, len * 2);
  for (i = 0; i < len; i++) {
    audioBuffer[i] = audioBuffer[i * 2];
  }
  return audioBuffer;
}


int luaopen_love(lua_State *L);

int main(int argc, char **argv) {

  /* Handle package command */
  if ( package_run(argc, argv) == PACKAGE_ESUCCESS ) {
    exit(EXIT_SUCCESS);
  }

  /* Init everything */
  atexit(deinit);
  cm_init(soundblaster_getSampleRate());
  soundblaster_init(audioCallback);
  vga_init();
  palette_init();
  keyboard_init();
  mouse_init();

  /* Init lua */
  L = luaL_newstate();
  lua_atpanic(L, onLuaPanic);
  luaL_openlibs(L);
  luaL_requiref(L, "love", luaopen_love, 1);

  /* Create `love.argv` and fill with arguments */
  lua_getglobal(L, "love");
  if (!lua_isnil(L, -1)) {
    lua_newtable(L);
    int i;
    for (i = 0; i < argc; i++) {
      lua_pushstring(L, argv[i]);
      lua_rawseti(L, -2, i + 1);
    }
    lua_setfield(L, -2, "argv");
  }
  lua_pop(L, 1);

  /* Init embedded scripts */
  #include "nogame_lua.h"
  #include "boot_lua.h"
  struct {
    const char *name, *data; int size;
  } items[] = {
    { "nogame.lua",   nogame_lua,   sizeof(nogame_lua)  },
    { "boot.lua",     boot_lua,     sizeof(boot_lua)    },
    { NULL, NULL, 0 }
  };
  int i;
  for (i = 0; items[i].name; i++) {
    int err = luaL_loadbuffer(L, items[i].data, items[i].size, items[i].name);
    if (err || lua_pcall(L, 0, 0, 0) != 0) {
      const char *str = lua_tostring(L, -1);
      fprintf(stderr, "Error: %s\n", str);
      abort();
    }
  }

  return 0;
}
