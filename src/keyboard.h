/**
 * Copyright (c) 2016 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#ifndef KEYBOARD_H
#define KEYBOARD_H

typedef struct {
  int type;
  unsigned code;
  const char *key;
  char text[64];
  int isrepeat;
} keyboard_Event;

enum {
  KEYBOARD_KEYPRESS,
  KEYBOARD_KEYRELEASE,
  KEYBOARD_TEXTINPUT
};

int keyboard_init(void);
void keyboard_deinit(void);
void keyboard_setKeyRepeat(int allow);
int keyboard_isDown(const char *key);
int keyboard_poll(keyboard_Event *e);

#endif
