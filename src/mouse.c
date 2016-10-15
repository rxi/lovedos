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

mouse_State mouse_state;


void mouse_init(void) {
  union REGS regs = {};
  int86(0x33, &regs, &regs);
  mouse_state.inited = regs.x.ax ? 1 : 0;
}


void mouse_update(void) {
  if (!mouse_state.inited) {
    return;
  }

  /* Update last mouse position */
  mouse_state.lastX = mouse_state.x;
  mouse_state.lastY = mouse_state.y;

  /* Update mouse position */
  union REGS regs = {};
  regs.x.ax = 3;
  int86(0x33, &regs, &regs);
  mouse_state.x = regs.x.cx >> 1;
  mouse_state.y = regs.x.dx;

  /* Update button states */
  int i;
  for (i = 0; i < MOUSE_BUTTON_MAX; i++) {
    /* Reset pressed/released */
    mouse_state.buttonsPressed[i] = 0;
    mouse_state.buttonsReleased[i] = 0;
    /* Handle mouse pressed */
    memset(&regs, 0, sizeof(regs));
    regs.x.ax = 5;
    regs.x.bx = i;
    int86(0x33, &regs, &regs);
    if (regs.x.bx) {
      mouse_state.buttonsPressed[i] = 1;
      mouse_state.buttonsDown[i] = 1;
    }
    /* Handle mouse released */
    memset(&regs, 0, sizeof(regs));
    regs.x.ax = 6;
    regs.x.bx = i;
    int86(0x33, &regs, &regs);
    if (regs.x.bx) {
      mouse_state.buttonsReleased[i] = 1;
      mouse_state.buttonsDown[i] = 0;
    }
  }
}


mouse_State* mouse_getState(void) {
  return &mouse_state;
}
