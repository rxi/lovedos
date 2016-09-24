#include <stdlib.h>
#include <pc.h>

#include "palette.h"

#define MAX_IDX   256
#define MAP_SIZE  1024
#define MAP_MASK  (MAP_SIZE - 1)

struct { unsigned color; int idx; } palette_map[MAP_SIZE];
int palette_nextIdx;
int palette_inited;


void palette_init(void) {
  if (palette_inited) return;
  palette_reset();
  palette_inited = 1;
}


void palette_reset(void) {
  /* Reset nextIdx -- start at idx 1 as 0 is used for transparency */
  palette_nextIdx = 1;
  /* Reset palette_map */
  int i;
  for (i = 0; i < MAP_SIZE; i++) {
    palette_map[i].idx = -1;
  }
}


static unsigned hash(const void *data, int size) {
  unsigned hash = 5381;
  const unsigned char *p = data;
  while (size--) {
    hash = ((hash << 5) + hash) ^ *p++;
  }
  return hash;
}


int palette_colorIdx(int r, int g, int b) {
  palette_init();

  /* Make 18bit rgb color (6bits per-channel) from 8bit channels  */
  unsigned color = ((b & 0xfc) << 10) | ((g & 0xfc) << 4) | ((r & 0xfc) >> 2);

  /* Hash color */
  unsigned h = hash(&color, sizeof(color));

  /* Find color in hashmap */
  int i = h & MAP_MASK;
  while (palette_map[i].idx != -1) {
    if (palette_map[i].color == color) {
      return palette_map[i].idx;
    }
    i = (i + 1) & MAP_MASK;
  }

  /* Color wasn't found in hashmap -- Add to system palette and map */
  if (palette_nextIdx >= MAX_IDX) {
    return -1;  /* Return -1 for error if we've exceeded the palette capacity */
  }
  int idx = palette_nextIdx++;

  /* Update system palette */
  outp(0x03c8, idx);
  outp(0x03c9, (color      ) & 0x3f); /* r */
  outp(0x03c9, (color >>  6) & 0x3f); /* g */
  outp(0x03c9, (color >> 12) & 0x3f); /* b */

  /* Add to hashmap and return idx */
  palette_map[i].color = color;
  palette_map[i].idx = idx;
  return idx;
}
