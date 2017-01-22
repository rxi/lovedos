

#include "luaobj.h"
#include "sound.h"
#include "mixer.h"


#define CLASS_TYPE LUAOBJ_TYPE_SOUND
#define CLASS_NAME "Sound"


int l_sound_new(lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);

  sound_t *self = luaobj_newudata(L, sizeof(*self));
  luaobj_setclass(L, CLASS_TYPE, CLASS_NAME);
  const char *err = sound_init(self, filename);
  if (err) luaL_error(L, err);
  return 1;
}


int l_sound_gc(lua_State *L) {
  sound_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  sound_deinit(self);
  return 0;
}


int l_sound_play(lua_State *L) {
  sound_t *self = luaobj_checkudata(L, 1, CLASS_TYPE);
  mixer_play(self);
  return 0;
}


int luaopen_sound(lua_State *L) {
  luaL_Reg reg[] = {
    { "new",    l_sound_new  },
    { "__gc",   l_sound_gc   },
    { "play",   l_sound_play },
    { 0, 0 }
  };

  luaobj_newclass(L, CLASS_NAME, NULL, l_sound_new, reg);
  return 1;
}
