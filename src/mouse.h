#ifndef MOUSE_H
#define MOUSE_H

enum {
  MOUSE_BUTTON_LEFT,
  MOUSE_BUTTON_RIGHT,
  MOUSE_BUTTON_MIDDLE,
  MOUSE_BUTTON_MAX,
};

typedef struct {
  int inited;
  int x, y;
  int lastX, lastY;
  int buttonsPressed[MOUSE_BUTTON_MAX];
  int buttonsReleased[MOUSE_BUTTON_MAX];
  int buttonsDown[MOUSE_BUTTON_MAX];
} mouse_State;

void mouse_init(void);
void mouse_update(void);
mouse_State* mouse_getState(void);

#endif
