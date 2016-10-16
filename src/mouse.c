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
#include "event.h"

int mouse_inited;
int mouse_x, mouse_y;
int mouse_lastX, mouse_lastY;
int mouse_buttonStates[MOUSE_BUTTON_MAX];


void mouse_init(void) {
  union REGS regs = {};
  int86(0x33, &regs, &regs);
  mouse_inited = regs.x.ax ? 1 : 0;
}


void mouse_update(void) {
  if (!mouse_inited) {
    return;
  }

  /* Update mouse position */
  union REGS regs = {};
  regs.x.ax = 3;
  int86(0x33, &regs, &regs);
  mouse_x = regs.x.cx >> 1;
  mouse_y = regs.x.dx;

  /* Do moved event if mouse moved */
  if (mouse_x != mouse_lastX || mouse_y != mouse_lastY) {
    event_t e;
    e.type = EVENT_MOUSE_MOVED;
    e.mouse.x = mouse_x;
    e.mouse.y = mouse_y;
    e.mouse.dx = mouse_x - mouse_lastX;
    e.mouse.dy = mouse_y - mouse_lastY;
    event_push(&e);
  }

  /* Update last position */
  mouse_lastX = mouse_x;
  mouse_lastY = mouse_y;

  /* Update button states and push pressed/released events */
  int i, t;
  for (i = 0; i < MOUSE_BUTTON_MAX; i++) {
    for (t = 0; t < 2; t++) {
      memset(&regs, 0, sizeof(regs));
      regs.x.ax = 5 + t;
      regs.x.bx = i;
      int86(0x33, &regs, &regs);
      if (regs.x.bx) {
        /* Push event */
        event_t e;
        e.type = t == 0 ? EVENT_MOUSE_PRESSED : EVENT_MOUSE_RELEASED;
        e.mouse.button = i;
        e.mouse.x = mouse_x;
        e.mouse.y = mouse_y;
        event_push(&e);
        /* Set state */
        mouse_buttonStates[i] = t == 0 ? 1 : 0;
      }
    }
  }
}



int mouse_isDown(int button) {
  return mouse_buttonStates[button];
}


int mouse_getX(void) {
  return mouse_x;
}


int mouse_getY(void) {
  return mouse_y;
}
