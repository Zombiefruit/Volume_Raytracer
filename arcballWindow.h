/* arcballWindow.h
 *
 * Adds view manipulation to the window class.
 *
 * Derived class must provide a draw() routine.
 */


#ifndef ARCBALL_WINDOW_H
#define ARCBALL_WINDOW_H

#include "headers.h"

#include "linalg.h"

#include <cstdio>
#include <string.h>

#include "window.h"


typedef enum { TRANSLATE, ROTATE } ModeType;
typedef enum { LEFT_BUTTON, MIDDLE_BUTTON, RIGHT_BUTTON, NO_BUTTON } ButtonType;
typedef float Matrix[4][4];


class arcballWindow : public window {

  vec3 mouse;			// last mouse position
  ModeType mode;		// translate or rotate mode
  ButtonType button;		// which button is currently down
  bool   mouseMoved;		// set true when mouse moves
  bool   showInfoAtMouse;

  void rotate( int x, int y );
  void tmmult(float a[4], Matrix m, float v[4]);
  void tmmmult(Matrix ans, Matrix a, Matrix b);
  void tm_matinv(Matrix a);
  void rot_mat2(float q[], Matrix m);
  void calc_quat(float q[], int mousex0, int mousey0, int mousex1, int mousey1);
  void calc_p(int mousex, int mousey, float p[]);
  void tm_copy(Matrix b, Matrix a);
  void displayMousePosition( int x, int y );

protected:

  GLint viewport[4];

public:

  arcballWindow( int x, int y, int width, int height, char *title )
    : window( x, y, width, height, title ) {
      
      mode = ROTATE;
      button = NO_BUTTON;
      setCursor();
      showInfoAtMouse = false;
    }

  virtual void draw() = 0;

  virtual char * mouseClickMessage( float objX, float objY ) {
    char buffer[1000];
    sprintf_s( buffer, "mouse at %g, %g", objX, objY );
    return _strdup( buffer );
  }

  virtual void userKeyAction( unsigned char key ) {}
  virtual void userSpecialAction( int key ) {}
  virtual bool userMouseAction( int b, int s, int x, int y ) { return false; }
  virtual bool userMouseMotion( int x, int y ) { return false; }

  void display();
  void mouseAction( int button, int state, int x, int y );
  void mouseMotion( int x, int y );
  void keyAction( unsigned char key, int x, int y );
  void specialAction( int key, int x, int y );
  void checkMouse( int x, int y );
  void setCursor();
  void mouseClick( int button, int x, int y );
  virtual void mouseClick( float x, float y, int xx, int yy );

  void OutputViewParams( char * filename );
  void ReadViewParams( char * filename );
};

#endif

