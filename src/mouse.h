#ifndef MOUSE_H
#define MOUSE_H

enum {
  MOUSE_BUTTON_LEFT,
  MOUSE_BUTTON_RIGHT,
  MOUSE_BUTTON_MIDDLE,
  MOUSE_BUTTON_MAX
};

enum {
  MOUSE_PRESSED,
  MOUSE_RELEASED,
  MOUSE_MOVED
};

typedef struct {
  int type;
  int x, y;
  int dx, dy;
  int button;
} mouse_Event;


void mouse_init(void);
int mouse_poll(mouse_Event *e);
int mouse_isDown(int button);
int mouse_getX(void);
int mouse_getY(void);

#endif
