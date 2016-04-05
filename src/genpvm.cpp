// genpvm.cpp
//
// Generate a cubic PVM volume


#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include "linalg.h"
 
using namespace std;


// Voxels within the volume

#define VOXEL_TYPE  unsigned short int
int maxVoxelValue;
#define toVoxel(x) ((int)(x * maxVoxelValue)) // convert float [0,1] to integer in volume

// Types of volume that we can generate

typedef enum { SOLID_GLASS, LENS, NUM_TYPES } VolumeType;
char *typeNames[] = { "solid glass", "lens" };

// Indices of refraction

#define CROWN_GLASS_INDEX 0.52	// index = 1.52, but we subtract 1 from all indices for storage


int main( int argc, char **argv )

{
  // Get command-line arguments

  if (argc < 3) {
    cerr << "Usage: " << argv[0] << " dim type filename" << endl
	 << endl
	 << "The 'type' is one of these:" << endl;

    for (int i=0; i<NUM_TYPES; i++)
      cerr << "  " << i+1 << " - " << typeNames[i] << endl;
    
    exit(1);
  }

  int dim = atoi( argv[1] );
  VolumeType volType = (VolumeType) (atoi( argv[2] ) - 1);
  char *filename = argv[3];

  if (volType < 0 or volType >= NUM_TYPES) {
    cerr << "Volume type must be between 1 and " << NUM_TYPES << endl;
    exit(1);
  }

  // Create volume and fill with zeroes

  VOXEL_TYPE * vol = (VOXEL_TYPE *) malloc( dim*dim*dim*sizeof(VOXEL_TYPE) );

  VOXEL_TYPE *p = vol;
  for (int i=0; i<dim*dim*dim; i++)
    *(p++) = 0;

  maxVoxelValue = (1 << 8*sizeof(VOXEL_TYPE)) - 1;

  // Fill volume with something interesting.  Leave 2 slabs of voxels
  // = 0 on the outside so that normals will later be calculated
  // correctly.

  switch( volType ) {

  case SOLID_GLASS:

    for (int x=0; x<dim; x++)
      for (int y=0; y<dim; y++)
	for (int z=0; z<dim; z++)
	  vol[x+dim*(y+dim*z)]= toVoxel(CROWN_GLASS_INDEX);
    break;

  case LENS:

    // Create a lens from the intersection of two spheres of equal radius

    const float lensHeight    = 0.90; // spans 90% of the volume in xy
    const float lensThickness = 0.40; // 40% along z axis
    
    float sphereRadius    = 0.25 * (lensThickness*lensThickness + lensHeight*lensHeight) / lensThickness;
    float sphereCentreOnZ = sphereRadius - 0.5 * lensThickness;

    vec3 sphereCentre1( 0, 0, -sphereCentreOnZ );
    vec3 sphereCentre2( 0, 0, +sphereCentreOnZ );

    float sphereRadiusSquared = sphereRadius*sphereRadius;

    for (int x=0; x<dim; x++)
      for (int y=0; y<dim; y++)
	for (int z=0; z<dim; z++) {

	  // Get position in [-0.5,+0.5]^3

	  vec3 pos = vec3( x/(float)(dim-1)-0.5, y/(float)(dim-1)-0.5, z/(float)(dim-1)-0.5 );

	  // If this position is inside of both spheres, fill with glass

	  if ((pos-sphereCentre1).squaredLength() < sphereRadiusSquared && (pos-sphereCentre2).squaredLength() < sphereRadiusSquared)
	    vol[x+dim*(y+dim*z)]= toVoxel(CROWN_GLASS_INDEX);
	}

    break;
  }

  // Write volume

  ofstream out( filename, ios::out | ios::binary );
  if (!out) {
    cerr << "Couldn't open volume file '" << filename << "'." << endl;
    exit(1);
  }

  // Write the header.  The format is
  // 
  // PVM3
  // 128 128 128    dimensions in x,y,z
  // 1 1 1          scale in x,y,z
  // 2              bytes per voxel

  out << "PVM3" << endl
      << dim << " " << dim << " " << dim << endl
      << "1 1 1" << endl
      << sizeof( VOXEL_TYPE ) << endl;

  out.write( (char *) vol, dim*dim*dim*sizeof(VOXEL_TYPE) );

  return 0;
}
