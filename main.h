// main.h


#ifndef MAIN_H
#define MAIN_H

void glErrorReport( char *where );

#define BG_TEXTURE_UNIT       0
#define VOLUME_TEXTURE_UNIT   1
#define GRADIENT_TEXTURE_UNIT 2

#define VOLUME_WIN_WIDTH 800
#define VOLUME_WIN_HEIGHT 600

#define XFER_WIN_WIDTH 400
#define XFER_WIN_HEIGHT 440

#include "volWindow.h"

extern VolWindow *win;

#endif
