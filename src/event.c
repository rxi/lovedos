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

#define BUFFER_SIZE 256
#define BUFFER_MASK (BUFFER_SIZE - 1)

static struct {
  event_t buffer[BUFFER_SIZE];
  unsigned writei, readi;
} events;


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


void event_push(event_t *e) {
  events.buffer[events.writei++ & BUFFER_MASK] = *e;
}


void event_pump(void) {
  keyboard_update();
  mouse_update();
}


int event_poll(event_t *e) {
  if (events.readi != events.writei) {
    *e = events.buffer[events.readi++ & BUFFER_MASK];
    return 1;
  }
  return 0;
}
