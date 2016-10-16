/**
 * Copyright (c) 2016 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#ifndef EVENT_H
#define EVENT_H

enum {
  EVENT_NULL,
  EVENT_KEYBOARD_PRESSED,
  EVENT_KEYBOARD_RELEASED,
  EVENT_KEYBOARD_TEXTINPUT,
  EVENT_MOUSE_MOVED,
  EVENT_MOUSE_PRESSED,
  EVENT_MOUSE_RELEASED
};

typedef union {
  int type;

  struct {
    int type;
    int x, y;
    int dx, dy;
    int button;
  } mouse;

  struct {
    int type;
    const char *key;
    char text[64];
    int isrepeat;
  } keyboard;

} event_t;


const char* event_typestr(int type);
void event_push(event_t *e);
void event_pump(void);
int event_poll(event_t *e);

#endif
