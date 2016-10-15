/**
 * Copyright (c) 2016 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <stdio.h>
#include <string.h>
#include "keyboard.h"
#include "mouse.h"
#include "event.h"


const char* event_typestr(int type) {
  switch (type) {
    case EVENT_KEYBOARD_PRESSED   : return "keypressed";
    case EVENT_KEYBOARD_RELEASED  : return "keyreleased";
    case EVENT_KEYBOARD_TEXTINPUT : return "textinput";
    case EVENT_MOUSE_MOVED        : return "mousemoved";
    case EVENT_MOUSE_PRESSED      : return "mousepressed";
    case EVENT_MOUSE_RELEASED     : return "mousereleased";
  }
  return "?";
}


int event_poll(event_t *e) {

  /* Poll keyboard */
  keyboard_Event ke;
  if (keyboard_poll(&ke)) {
    if (ke.type == KEYBOARD_PRESSED || ke.type == KEYBOARD_RELEASED) {
      if (ke.type == KEYBOARD_PRESSED) {
        e->type = EVENT_KEYBOARD_PRESSED;
      } else {
        e->type = EVENT_KEYBOARD_RELEASED;
      }
      e->keyboard.key = ke.key;
      e->keyboard.isrepeat = ke.isrepeat;

    } else if (ke.type == KEYBOARD_TEXTINPUT) {
      e->type = EVENT_KEYBOARD_TEXTINPUT;
      strcpy(e->keyboard.text, ke.text);
    }

    return 1;
  }

  /* Poll mouse */
  mouse_Event me;
  if (mouse_poll(&me)) {
    if (me.type == MOUSE_PRESSED || me.type == MOUSE_RELEASED) {
      if (me.type == MOUSE_PRESSED) {
        e->type = EVENT_MOUSE_PRESSED;
      } else {
        e->type = EVENT_MOUSE_RELEASED;
      }
      e->mouse.button = me.button;
      e->mouse.x = me.x;
      e->mouse.y = me.y;
      e->mouse.dx = me.dx;
      e->mouse.dy = me.dy;

    } else if (me.type == MOUSE_MOVED) {
      e->type = EVENT_MOUSE_MOVED;
      e->mouse.x = me.x;
      e->mouse.y = me.y;
    }

    return 1;
  }

  /* No events */
  return 0;
}
