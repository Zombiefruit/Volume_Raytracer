// main.cpp
//
// Volume renderer


#include "headers.h"

#include <cstdlib>
#include <ctype.h>
#include <fstream>
#include "volWindow.h"
#include "volume.h"

using namespace std;


VolWindow *win;


int main( int argc, char **argv )

{
  if (argc < 2) {
    cerr << "Usage: " << argv[0] << " filename.pvm" << endl;
    exit(1);
  }
  
  glutInit( &argc, argv );
  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );

  win = new VolWindow();

  win->volume = new Volume( argv[1] );
  
  glutMainLoop();

  return 0;
}



void glErrorReport( char *where )

{
  GLuint errnum;
  const char *errstr;

  while ((errnum = glGetError())) {
    errstr = reinterpret_cast<const char *>(gluErrorString(errnum));
    printf("%s: %s\n", where, errstr);
  }
}




// A bug in some GL* library in Ubuntu 14.04 requires that libpthread
// be forced to load.  The code below accomplishes this (from MikeMx7f
// http://stackoverflow.com/questions/31579243/segmentation-fault-before-main-when-using-glut-and-stdstring).

#include <pthread.h>

void* simpleFunc(void*) { return NULL; }

void forcePThreadLink() { pthread_t t1; pthread_create(&t1, NULL, &simpleFunc, NULL); }
