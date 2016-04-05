/* arcballWindow.C
 */

#include "headers.h"

#include <fstream>
#include <math.h>
#ifndef WIN32
  #include <string.h>
  #include <sstream>
#endif
#include <stdlib.h>
#include "arcballWindow.h"
#include "font.h"

using namespace std;


void arcballWindow::display()

{
  setupProjection();
  draw();
  if (showInfoAtMouse)
    displayMousePosition( (int) mouse.x, (int) mouse.y );
  glFlush();
  glutSwapBuffers();
}


// Output or Input the view parameters

void arcballWindow::OutputViewParams( char * filename )

{
  ofstream out( filename );

  if (!out) 
    cerr << "Error: Failed to open the output file\n";
  else {
    out << eye << endl;
    out << lookat << endl;
    out << updir << endl;
    out << fovy << endl;
    out << orthoScale << endl;
    out << left << ' ' << bottom << ' ' << nnear << ' ' << ffar << ' ' << zNear << ' ' << zFar << endl;
    if (projection == PERSPECTIVE)
      out << 'P' << endl;
    else
      out << 'O' << endl;
  }
}


void arcballWindow::ReadViewParams( char * filename )

{
  char c;

  ifstream in( filename );

  if (!in) 
    cerr << "Error: Failed to open the input file\n";
  else {
    in >> eye;
    in >> lookat;
    in >> updir;
    in >> fovy;
    in >> orthoScale;
    in >> left >> bottom >> nnear >> ffar >> zNear >> zFar;
    in >> c;
    if (c == 'P')
      projection = PERSPECTIVE;
    else
      projection = ORTHOGRAPHIC;
  }
}


// Switch translate/rotate mode when space is pressed

void arcballWindow::keyAction( unsigned char key, int x, int y )

{
  switch (key) {
  case ' ':
    if (mode == TRANSLATE)
      mode = ROTATE;
    else 
      mode = TRANSLATE;
    setCursor();
    break;
  case 'P':
    if (projection == PERSPECTIVE) {
      projection = ORTHOGRAPHIC;
      if (mode == ROTATE) {
	mode = TRANSLATE;
	setCursor();
      }
    } else
      projection = PERSPECTIVE;
    glutPostRedisplay();
    break;
  case 27:
    exit(0);
    break;
  default:
    userKeyAction( key );
  }
}


void arcballWindow::specialAction( int key, int x, int y )

{
  userSpecialAction( key );
}


// Set the cursor depending upon the translate/rotate mode

void arcballWindow::setCursor()

{
  if (mode == TRANSLATE)
    glutSetCursor( GLUT_CURSOR_CROSSHAIR );
  else
    glutSetCursor( GLUT_CURSOR_CYCLE );
}


// Update viewing params based on new mouse position

void arcballWindow::mouseMotion( int x, int y )

{
  if (userMouseMotion(x,y))
    return;

  vec3 xdir, ydir, zdir;

  ydir = updir.normalize();
  zdir = (eye - lookat).normalize();
  xdir = (ydir ^ zdir).normalize();

  if (mode == TRANSLATE) {

    if (button == LEFT_BUTTON)

      if (projection == PERSPECTIVE) {
	lookat = lookat + fovy/350.0 * ((mouse.x-x) * xdir + (y-mouse.y) * ydir);
	eye    = eye    + fovy/350.0 * ((mouse.x-x) * xdir + (y-mouse.y) * ydir);
      } else {
	lookat = lookat + orthoScale/(float)xdim * ((mouse.x-x) * xdir + (y-mouse.y) * ydir);
	eye    = eye    + orthoScale/(float)xdim * ((mouse.x-x) * xdir + (y-mouse.y) * ydir);
      }

    else if (button == RIGHT_BUTTON)

      if (projection == PERSPECTIVE) {
	lookat = lookat + (mouse.y-y)*0.2 * zdir;
	eye    = eye    + (mouse.y-y)*0.2 * zdir;
      } else {
	orthoScale *= 1.0 + 0.005 * (mouse.y-y);
      }

    else if (button == MIDDLE_BUTTON)

      if (projection == ORTHOGRAPHIC)
	showInfoAtMouse = true;

  } else if (projection == PERSPECTIVE) { /* and mode == ROTATE */

    if (button == RIGHT_BUTTON) {

      fovy *= 1.0 + 0.001*(mouse.y-y);
      if (fovy > 135)
	fovy = 135;
      if (fovy < 0.001)
	fovy = 0.001;

    } else if (button == LEFT_BUTTON) {

      rotate(x,y);
    }
  }

  mouse.x = x;
  mouse.y = y;
  mouseMoved = true;
  glutPostRedisplay();
}


// Record button state when mouse button is pressed or released

void arcballWindow::mouseAction( int b, int s, int x, int y )

{
  if (userMouseAction(b,s,x,y))
    return;

  mouse.x = x;
  mouse.y = y;

  if (s == GLUT_UP) {

    button = NO_BUTTON;
    if (!mouseMoved)
      mouseClick( b, x, y );
    if (showInfoAtMouse) {
      showInfoAtMouse = false;
      glutPostRedisplay();
    }
    setCursor();

  } else if (s == GLUT_DOWN) {

    if (b == GLUT_LEFT_BUTTON)
      button = LEFT_BUTTON;
    else if (b == GLUT_MIDDLE_BUTTON) {
      button = MIDDLE_BUTTON;
      glutSetCursor( GLUT_CURSOR_RIGHT_ARROW );
      glGetIntegerv( GL_VIEWPORT, viewport );
      showInfoAtMouse = true; // output mouse location to screen
      glutPostRedisplay();
    } else if (b == GLUT_RIGHT_BUTTON)
      button = RIGHT_BUTTON;
    mouseMoved = false;
  }
}


void arcballWindow::mouseClick( int button, int x, int y )

{
  return;

  // Still have to update this for OpenGL 3.3

#if 0
  GLdouble objX, objY, objZ;

  glGetDoublev( GL_MODELVIEW_MATRIX, modelMatrix );	// store for gluUnproject
  glGetDoublev( GL_PROJECTION_MATRIX, projMatrix );	// when locating mouse
  glGetIntegerv( GL_VIEWPORT, viewport );

  // You should really only use this with ORTHOGRAPHIC projection

  gluUnProject( (GLdouble) x + 3, (GLdouble) viewport[3] - (GLint) y - 1 + 3, 0,
                modelMatrix, projMatrix, viewport,
                &objX, &objY, &objZ );

  mouseClick( objX, objY, x, y );
#endif
}


// ================================================================
//                 From Michiel's arcball code
// ================================================================


void arcballWindow::calc_p(int mousex, int mousey, float p[])
{
  float r,s;

  p[0] = 2.0*(mousex - 0.5*xdim)/xdim;
  p[1] = -2.0*(mousey - 0.5*ydim)/ydim;;
  r = p[0]*p[0] + p[1]*p[1];
  if (r>1.0) {
    s = 1.0/sqrt(r);
    p[0] *= s;
    p[1] *= s;
    p[2] = 0.0;
  } else {
    p[2] = sqrt(1.0 - r);
  }
}


void arcballWindow::calc_quat(float q[], int mousex0, int mousey0, int mousex1, int mousey1)

{
  float p0[4], p1[4];

  calc_p(mousex0,mousey0,p0);
  calc_p(mousex1,mousey1,p1);
  q[1] = p0[1]*p1[2] - p1[1]*p0[2];
  q[2] = p1[0]*p0[2] - p0[0]*p1[2];
  q[3] = p0[0]*p1[1] - p1[0]*p0[1];
  q[0] = (p0[0]*p1[0] + p0[1]*p1[1] + p0[2]*p1[2]);
  if (q[0]<-1.0) q[0]= -1.0;
}


void arcballWindow::rot_mat2(float q[], Matrix m)

{
  float w,x,y,z;
  float xs,ys,zs,wx,wy,wz,xx,yy,zz,xy,yz,xz;
	
  w = q[0];
  x = q[1];
  y = q[2];
  z = q[3];

  /*
    if (w <= -0.999) {
    iden(m);
    return;
    }
    */
  xs = x + x;
  ys = y + y;
  zs = z + z;
  wx = w*xs;
  wy = w*ys;
  wz = w*zs;
  xx = x*xs;
  yy = y*ys;
  zz = z*zs;
  xy = x*ys;
  yz = y*zs;
  xz = x*zs;
  m[0][0] = 1.0 - (yy+zz);
  m[0][1] = xy-wz;
  m[0][2] = xz+wy;
  m[0][3] = 0.0;
  m[1][0] = xy + wz;
  m[1][1] = 1.0 - (xx+zz);
  m[1][2] = yz - wx;
  m[1][3] = 0.0;
  m[2][0] = xz-wy;
  m[2][1] = yz + wx;
  m[2][2] = 1.0 - (xx+yy);
  m[2][3] = 0.0;
  m[3][0] = 0.0;
  m[3][1] = 0.0;
  m[3][2] = 0.0;
  m[3][3] = 1.0;
}

void arcballWindow::tm_copy(Matrix b, Matrix a)

{
  int r,c;

  for (r=0; r<4; r++) {
    for (c=0; c<4; c++) {
      b[r][c] = a[r][c];
    }
  }
}

void arcballWindow::tm_matinv(Matrix a)

{
  int size=4;
  Matrix ai,aorig;
  int r,c,c1;
  float mf,pivot;

  tm_copy(aorig,a);
  for (r=0; r<size; r++) {         /* start with identity inverse matrix */
    for (c=0; c<size; c++) {
      ai[r][c] = (r==c);
    }
  }

  for (r=0; r<size; r++) {   /* for each r */
    for (c=0; c<r; c++) {        /* zero matrix entries before pivot */
      mf = a[r][c];
      for (c1=0; c1<size; c1++) {
	a[r][c1] -= mf*a[c][c1];
	ai[r][c1] -= mf*ai[c][c1]; 
      }
    }
    pivot = a[r][r];              /* get pivot */
    if (pivot==0) {
      cerr << "matinv: bad pivot\n";
      abort();
    }
    mf = 1.0/pivot;
    for (c=0; c<size; c++) {       /* divide r by pivot */
      a[r][c] *= mf;
      ai[r][c] *= mf;
    }
  }
  for (r=0; r<size; r++) {     /* eliminate upper-half */
    for (c=r+1; c<size; c++) {     /* each remaining entry */
      mf = a[r][c];
      for (c1=0; c1<size; c1++) {
	a[r][c1] -= mf*a[c][c1];
	ai[r][c1] -= mf*ai[c][c1] ;
      }
    }
  }
  for (r=0; r<size; r++)
    for (c=0; c<size; c++)
      a[r][c] = ai[r][c];
}

void arcballWindow::tmmult(float a[4], Matrix m, float v[4])

{
  int i,j;
  float s;

  for (i=0; i<4; i++) {
    s = 0.0;
    for (j=0; j<4; j++) 
      s += m[i][j]*v[j];
    a[i] = s;
  }
}

void arcballWindow::tmmmult(Matrix ans, Matrix a, Matrix b)

{
  int i,j,n;
  float s;

  for (i=0; i<4; i++) {
    for (j=0; j<4; j++) {
      s = 0.0;
      for (n=0; n<4; n++) {
    	s += a[i][n]*b[n][j];
      }
      ans[i][j] = s;
    }
  }
}


void arcballWindow::rotate( int x, int y )

{
  GLfloat modelviewMatrix[16];
  float quat[4], prev[4], next[4];
  Matrix qm, M, T;
  int k;

  // Get the rotation matrix
  
  calc_quat( quat, (int) mouse.x, (int) mouse.y, x, y );

  rot_mat2( quat, qm );
  tm_matinv( qm );

  // get current modelview matrix (rotation component only)

  glLoadIdentity();
  gluLookAt( eye.x, eye.y, eye.z, lookat.x, lookat.y, lookat.z, updir.x, updir.y, updir.z );
  glGetFloatv( GL_MODELVIEW_MATRIX, modelviewMatrix );

  k = 0;
  for (int i=0; i<4; i++)
    for (int j=0; j<4; j++)
      M[j][i] = modelviewMatrix[k++];
  M[0][3] = 0;
  M[1][3] = 0;
  M[2][3] = 0;
  M[3][3] = 1;

  // Make transform T = M^{-1} qm

  tm_matinv( M );
  tmmmult( T, M, qm );

  // Rotate the view direction and up direction

  prev[0] = 0;
  prev[1] = 0;
  prev[2] = (eye - lookat).length();
  prev[3] = 1;

  tmmult( next, T, prev );

  eye.x = next[0]/next[3] + lookat.x;
  eye.y = next[1]/next[3] + lookat.y;
  eye.z = next[2]/next[3] + lookat.z;

  prev[0] = 0;
  prev[1] = 1;
  prev[2] = 0;
  prev[3] = 1;

  tmmult( next, T, prev );

  updir.x = next[0]/next[3];
  updir.y = next[1]/next[3];
  updir.z = next[2]/next[3];
}



void arcballWindow::displayMousePosition( int x, int y )

{
#if 0
  GLdouble objX, objY, objZ;

  // You should really only use this with ORTHOGRAPHIC projection

  gluUnProject( (GLdouble) x + 3, (GLdouble) viewport[3] - (GLint) y - 1 + 3, 0,
                modelMatrix, projMatrix, viewport,
                &objX, &objY, &objZ );

  char * message;
  int xx, yy;

  message = mouseClickMessage( objX, objY );

  // Output the message

  glColor3f( 1, 0, .5 );
  xx = x - 16;
  yy = viewport[3] - 28 - y;
  printString( message, xx, yy, viewport[2], viewport[3] );

  free( message );

#endif
}

void arcballWindow::mouseClick( float objX, float objY, int x, int y )

{
  char * message;
  int xx, yy;

  message = mouseClickMessage( objX, objY );

  // Output the message

  glColor3f( 1, 0, .5 );
  xx = x - 16;
  yy = viewport[3] - 28 - y;
  printString( message, xx, yy, viewport[2], viewport[3] );

  free( message );
}


