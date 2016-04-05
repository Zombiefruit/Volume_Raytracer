// volume.cpp


#include "headers.h"

#include <fstream>
#include <cstdio>
#include <cstring>
#include <math.h>

#include "volWindow.h"
#include "volume.h"
#include "main.h"
#include "font.h"
#include "cube.h"

using namespace std;



#if 0
float Volume::kernel[] = { // Central difference
   0,0,0, 0,-1,0, 0,0,0,
   0,0,0, 0, 0,0, 0,0,0,
   0,0,0, 0, 1,0, 0,0,0
};
#else
float Volume::kernel[] = { // Sobel
  -1,-3,-1, -3,-6,-3, -1,-3,-1,
   0, 0, 0,  0, 0, 0,  0, 0, 0,
   1, 3, 1,  3, 6, 3,  1, 3, 1
};
#endif


void Volume::draw( mat4 &M, mat4 &V, mat4 &P, GPUProgram *plainProg, Grid *grid, mat4 &gridM )

{
  mat4 MV = V * M;
  mat4 MVP = P * MV;
  
  glDisable( GL_LIGHTING );
  glDisable( GL_BLEND );
  glDisable( GL_POINT_SMOOTH );
  glDisable( GL_DEPTH_TEST );
  glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

  int winWidth = glutGet( GLUT_WINDOW_WIDTH );
  int winHeight = glutGet( GLUT_WINDOW_HEIGHT );

  // Determine the volume coordinate system (origin + axes) in the WCS

  vec4 v;
  vec3 volumeOrigin, volumeX, volumeY, volumeZ;

  v = M * vec4(0,0,0,1);
  volumeOrigin = vec3( v.x/v.w, v.y/v.w, v.z/v.w );

  v = M * vec4(1,0,0,0);
  volumeX = vec3( v.x, v.y, v.z );

  v = M * vec4(0,1,0,0);
  volumeY = vec3( v.x, v.y, v.z );

  v = M * vec4(0,0,1,0);
  volumeZ = vec3( v.x, v.y, v.z );

  // Determine the background texture coordinate system (origin + axes) in the WCS

  vec3 bgOrigin, bgX, bgY, bgZ;
  mat4 gridToWCS = V.inverse() * gridM;

  v = gridToWCS * vec4(0,0,0,1);
  bgOrigin = vec3( v.x/v.w, v.y/v.w, v.z/v.w );

  v = gridToWCS * vec4(1,0,0,0);
  bgX = vec3( v.x, v.y, v.z );

  float bgXdim = bgX.length();
  bgX = bgX.normalize();

  v = gridToWCS * vec4(0,1,0,0);
  bgY = vec3( v.x, v.y, v.z );

  float bgYdim = bgY.length();
  bgY = bgY.normalize();

  v = gridToWCS * vec4(0,0,1,0);
  bgZ = vec3( v.x, v.y, v.z ).normalize();

  // Send uniforms to shaders

  traceProg->activate();

  traceProg->setMat4( "M",   M );
  traceProg->setMat4( "MV",  MV );
  traceProg->setMat4( "MVP", MVP );

  traceProg->setFloat( "stepSize",      sliceSpacing );
  traceProg->setFloat( "densityFactor", xferFactor );
  traceProg->setInt(   "flag",          flag );

  traceProg->setInt( "texture_volume",   VOLUME_TEXTURE_UNIT );
  traceProg->setInt( "texture_gradient", GRADIENT_TEXTURE_UNIT );
  traceProg->setInt( "texture_background", BG_TEXTURE_UNIT );

  traceProg->setVec3( "volumeOrigin", volumeOrigin );
  traceProg->setVec3( "volumeX",      volumeX );
  traceProg->setVec3( "volumeY",      volumeY );
  traceProg->setVec3( "volumeZ",      volumeZ );

  traceProg->setVec3( "bgOrigin", bgOrigin );
  traceProg->setVec3( "bgX",      bgX );
  traceProg->setVec3( "bgY",      bgY );
  traceProg->setVec3( "bgZ",      bgZ );
  traceProg->setFloat( "bgXdim",   bgXdim );
  traceProg->setFloat( "bgYdim",   bgYdim );

  traceProg->setVec3( "eyePosition",  win->eye );

  glActiveTexture( GL_TEXTURE0 + VOLUME_TEXTURE_UNIT );
  glBindTexture( GL_TEXTURE_3D, volumeTextureID );

  glActiveTexture( GL_TEXTURE0 + GRADIENT_TEXTURE_UNIT );
  glBindTexture( GL_TEXTURE_3D, gradientTextureID );

  glActiveTexture( GL_TEXTURE0 + BG_TEXTURE_UNIT );
  glBindTexture( GL_TEXTURE_2D, grid->textureID );

  // Render BBox front face with GPU raytracing fragment shader

  glEnable( GL_CULL_FACE );
  glCullFace( GL_BACK );

  renderCubeWithRGBCoords();

  glDisable( GL_CULL_FACE );

  // Deactivate shader and textures

  glActiveTexture( GL_TEXTURE0 + VOLUME_TEXTURE_UNIT );
  glBindTexture( GL_TEXTURE_3D, 0 );

  glActiveTexture( GL_TEXTURE0 + GRADIENT_TEXTURE_UNIT );
  glBindTexture( GL_TEXTURE_3D, 0 );

  traceProg->deactivate();

  // Draw the volume cube outline

  if (drawBB) {
    plainProg->activate();
    plainProg->setMat4( "MVP", MVP );
    glDepthFunc( GL_LEQUAL );
    renderCubeOutline();
    glDepthFunc( GL_LESS );
    plainProg->deactivate();
  }

  glErrorReport( "after Volume::renderGPURT" );

  glColor3f(0,0,0);
  char buff[100];
  sprintf( buff, "spacing %.4f   factor %.5g", sliceSpacing, xferFactor );
  printString( buff, 10,  10, winWidth, winHeight );
}



// Register the volume texture with OpenGL


void Volume::registerVolumeData()

{
  glErrorReport( "before Volume::registerVolumeData" );

  glGenTextures( 1, &volumeTextureID );

  glBindTexture( GL_TEXTURE_3D, volumeTextureID );

  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

  if (bytesPerVoxel == 1)
    glTexImage3D( GL_TEXTURE_3D, 0, GL_R8,  dim.x, dim.y, dim.z, 0, GL_RED, GL_UNSIGNED_BYTE,  volumeTexMap );
  else
    glTexImage3D( GL_TEXTURE_3D, 0, GL_R16, dim.x, dim.y, dim.z, 0, GL_RED, GL_UNSIGNED_SHORT, volumeTexMap );

  glBindTexture( GL_TEXTURE_3D, 0 );

  glErrorReport( "after Volume::registerVolumeData" );
}



// Build the gradient volume


void Volume::buildGradientData()

{
  // Create texture map for the gradient

  if (bytesPerVoxel > 2) {
    cerr << "Can have at most two bytes per voxel, but this volume has " << bytesPerVoxel << "." << endl;
    exit(1);
  }

  gradientTexMap = new float [ (int) (dim.x * dim.y * dim.z * 3) ]; // 3 component RGB

  vec3 *p = (vec3*) gradientTexMap; // destination

  // Find maximum possible gradient magnitude

  const float maxGradientMagnitude = sqrt(3);

  int xinc = bytesPerVoxel;
  int yinc = bytesPerVoxel * dim.x;
  int zinc = bytesPerVoxel * dim.x * dim.y;

  float maxSqLen = 0;

  for (int z=0; z<dim.z; z++) {
    for (int y=0; y<dim.y; y++)
      for (int x=0; x<dim.x; x++) {

	// Compute gradient

#define TEXEL(i,j,k)   (* ((unsigned char *) (volumeTexMap + (i)*xinc + (j)*yinc + (k)*zinc)))
#define TEXEL2(i,j,k)  (* ((unsigned short int *) (volumeTexMap + (i)*xinc + (j)*yinc + (k)*zinc)))

	vec3 gradient;
	
	if (x == 0 || x == dim.x-1 || y == 0 || y == dim.y-1 || z == 0 || z == dim.z-1)

	  gradient = vec3(0,0,0);

	else {

	  float sumX = 0;
	  float *weight = &kernel[0];
	  for (int i=-1; i<2; i++)
	    for (int j=-1; j<2; j++)
	      for (int k=-1; k<2; k++) {
		if (*weight != 0) {
		  if (bytesPerVoxel == 1)
		    sumX += (*weight) * TEXEL(x+i,y+j,z+k);
		  else
		    sumX += (*weight) * TEXEL2(x+i,y+j,z+k);
		}
		weight++;
	      }

	  float sumY = 0;
	  weight = &kernel[0];
	  for (int j=-1; j<2; j++)
	    for (int i=-1; i<2; i++)
	      for (int k=-1; k<2; k++) {
		if (*weight != 0) {
		  if (bytesPerVoxel == 1)
		    sumY += (*weight) * TEXEL(x+i,y+j,z+k);
		  else
		    sumY += (*weight) * TEXEL2(x+i,y+j,z+k);
		}
		weight++;
	      }

	  float sumZ = 0;
	  weight = &kernel[0];
	  for (int k=-1; k<2; k++)
	    for (int i=-1; i<2; i++)
	      for (int j=-1; j<2; j++) {
		if (*weight != 0) {
		  if (bytesPerVoxel == 1)
		    sumZ += (*weight) * TEXEL(x+i,y+j,z+k);
		  else
		    sumZ += (*weight) * TEXEL2(x+i,y+j,z+k);
		}
		weight++;
	      }

	  gradient = vec3( sumX/scale.x, sumY/scale.y, sumZ/scale.z );
	}

	// Store gradient as a short (signed) int

	*p++ = gradient;

	float len = gradient.squaredLength();
	if (len > maxSqLen)
	  maxSqLen = len;
      }
    
    cout << "\r" << dim.z-z << "  "; cout.flush();
  }

  cout << "\r   \r"; cout.flush();

  // Scale the gradients so that the maximum length is 1. 

  p = (vec3*) gradientTexMap;

  float factor = -1 / sqrt(maxSqLen); // negative to point out of surface

  cout << "\rnormalizing gradients by " << factor << "  \r"; cout.flush();
 
  for (int z=0; z<dim.z; z++)
    for (int y=0; y<dim.y; y++)
      for (int x=0; x<dim.x; x++)
	*p++ = factor * (*p);

  cout << "\r                                                 \r"; cout.flush();
}


void Volume::registerGradientData()

{
  glErrorReport( "before Texture3D::registerWithOpenGL" );

  glGenTextures( 1, &gradientTextureID );

  glBindTexture( GL_TEXTURE_3D, gradientTextureID );

  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

  glTexImage3D( GL_TEXTURE_3D, 0, GL_RGB16_SNORM, dim.x, dim.y, dim.z, 0, GL_RGB, GL_FLOAT, gradientTexMap );

  glBindTexture( GL_TEXTURE_3D, 0 );

  glErrorReport( "after Texture3D::registerWithOpenGL" );
}



// Read a 3D texture from a PVM file.


void Volume::readVolumeData( char *filename )

{
  name = _strdup( filename );

  // Open volume file

  ifstream vol( filename, ios::in | ios::binary );
  if (!vol) {
    cerr << "Couldn't open volume file '" << filename << "'." << endl;
    exit(1);
  }

  // Read the header

  string line;

  getline( vol, line );
  if (line != "PVM2" && line != "PVM3") {
    cerr << "Unrecognized format: " << line << ".  Can only read PVM2 and PVM3 files." << endl;
    exit(1);
  }

  int xdim, ydim, zdim;

  getline( vol, line );
  sscanf_s( line.c_str(), "%d %d %d", &xdim, &ydim, &zdim );

  float xscale, yscale, zscale;

  getline( vol, line );
  sscanf_s( line.c_str(), "%f %f %f", &xscale, &yscale, &zscale );

  getline( vol, line );
  sscanf_s( line.c_str(), "%d", &bytesPerVoxel );

  // Normalize the x,y,z scales so that the maximum is 1.0

  if (xscale >= yscale && xscale >= zscale) {
    yscale /= xscale;
    zscale /= xscale;
    xscale = 1;
  } else if  (yscale >= xscale && yscale >= zscale) {
    xscale /= yscale;
    zscale /= yscale;
    yscale = 1;
  } else {
    xscale /= zscale;
    yscale /= zscale;
    zscale = 1;
  } 

  // Read the volume

  dim = vec3( xdim, ydim, zdim );
  scale = vec3( xscale, yscale, zscale );

  if (xdim >= ydim && xdim >= zdim)
    maxDim = xdim;
  else if (ydim >= xdim && ydim >= zdim)
    maxDim = ydim;
  else
    maxDim = zdim;

  volumeTexMap = (unsigned char *) malloc( xdim * ydim * zdim * bytesPerVoxel );

  vol.read( (char *) volumeTexMap, xdim * ydim * zdim * bytesPerVoxel );
}

