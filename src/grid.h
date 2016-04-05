// grid.h


#ifndef GRID_H
#define GRID_H

#include "gpuProgram.h"

class Grid {

 public:

  GLuint VAO;
  GLuint textureID;

  Grid() {}

  void renderGrid( GPUProgram *gpuProg );
};

#endif
