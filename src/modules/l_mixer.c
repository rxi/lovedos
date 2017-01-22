

#include "mixer.h"
#include "luaobj.h"


int l_mixer_mix(lua_State *L) {
  mixer_mix();
  return 0;
}


int luaopen_mixer(lua_State *L) {
  luaL_Reg reg[] = {
    { "mix", l_mixer_mix },
    { 0, 0 }
  };

  luaL_newlib(L, reg);
  return 1;
}
