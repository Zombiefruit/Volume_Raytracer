/* volume.h
 */


#ifndef VOLUME_H
#define VOLUME_H

#include "headers.h"

#include "main.h"
#include "grid.h"
#include "gpuProgram.h"



class Volume {

  char *name;		      // name of this volume (= filename)
  GPUProgram *traceProg;      // plain shader

  static float kernel[];	// gradient-computation kernal

  // Volume data

  unsigned char *volumeTexMap;	    // 3D volume
  int            bytesPerVoxel;	    // 1 or 2
  GLuint         volumeTextureID;   // texture ID

  // Gradient data

  float  *gradientTexMap;
  GLuint gradientTextureID;
  
 public:

  float        maxDim;
  vec3         dim;		    // dimensions of the 3D volume
  vec3         scale;		    // scale factor

  // Misc parameters

  float      xferFactor;       // factor by which to mulitply to get denser surfaces
  bool       drawBB;	       // draw bounding box
  int        flag;	       // flag for passing in to the shader.
  vec3       backgroundColour; // background
  float      sliceSpacing;     // delta s 


  Volume( char *filename ) {

    // Read and store the volume data

    readVolumeData( filename );
    registerVolumeData();

    // Compute and store the gradient

    buildGradientData();
    registerGradientData();

    traceProg = new GPUProgram( "trace.vert", "trace.frag" );

    // Set misc parameters

    flag = 0;
    backgroundColour = vec3(1,1,1);
    sliceSpacing = 0.01;
    xferFactor = 0.01;

    drawBB = false;
  }

  void draw( mat4 &M, mat4 &V, mat4 &P, GPUProgram *plainProg, Grid *grid, mat4 &gridM );

  void readVolumeData( char *filename );
  void registerVolumeData();
  void buildGradientData();
  void registerGradientData();

  void buildTransferHistogram();
};


#endif
