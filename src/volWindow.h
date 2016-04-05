// volWindow.h


#ifndef VOLWINDOW_H
#define VOLWINDOW_H

#include "headers.h"

class VolWindow;

#include "arcballWindow.h"
#include "volume.h"
#include "main.h"
#include "gpuProgram.h"
#include "grid.h"

using namespace std;


class VolWindow : public arcballWindow {

  GPUProgram *plainProg;
  GPUProgram *textureProg;
  bool drawTexture;

 public:

  Volume *volume;
  Grid *grid;

  VolWindow()
    : arcballWindow( 0, 0, VOLUME_WIN_WIDTH, VOLUME_WIN_HEIGHT, "Volume" ) {

    volume = NULL;
    eye    = vec3( 0, 0, 5 );
    lookat = vec3( 0, 0, 0 );
    updir  = vec3( 0, 1, 0 );

    plainProg = new GPUProgram( "plain.vert", "plain.frag" );
    textureProg = new GPUProgram( "tex2D.vert", "tex2D.frag" );
    grid = new Grid();
    drawTexture = true;
  }

  void draw() {

    glClearColor( 1, 1, 1, 1 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    mat4 gridM = translate( 0, 0, -10 ) * scale(10,10,10) * translate( -0.5, -0.5, -0.5 );

    if (drawTexture) {
      mat4 MVP = P * gridM;		      // don't include V, so grid remains static in VCS
      textureProg->activate();
      textureProg->setMat4( "MVP", MVP );
      grid->renderGrid( plainProg );
      textureProg->deactivate();
    }

    if (volume != NULL) {

      vec3 s( volume->scale.x * volume->dim.x / volume->maxDim,
	      volume->scale.y * volume->dim.y / volume->maxDim,
	      volume->scale.z * volume->dim.z / volume->maxDim );

      mat4 M = scale(s) * translate( -0.5, -0.5, -0.5 );

      volume->draw( M, V, P, plainProg, grid, gridM );
    }

  }

  void userSpecialAction( int specialKey );
  void userKeyAction( unsigned char key );
};


#endif
