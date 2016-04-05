// volWindow.cpp

#include "volWindow.h"

using namespace std;



void VolWindow::userSpecialAction( int specialKey )

{
  switch (specialKey) {
  }
    
  glutPostRedisplay();
}

  
void VolWindow::userKeyAction( unsigned char key )

{
  switch (key) {

  case '?':
    cout << " b  - toggle volume outline" << endl
         << " t  - show backgroundtexture" << endl 
	 << " <  - decrease slice spacing" << endl
	 << " >  - increase slice spacing" << endl
	 << " -  - decrease density factor" << endl
	 << " +  - increase density factor" << endl
	 << "0-9 - set 'flag' to value (for use in shader)"
	 << endl;
    break;
      
  case '+':
  case '=':
    volume->xferFactor *= 1.1;
    break;
      
  case '-':
  case '_':
    volume->xferFactor /= 1.1;
    if (volume->xferFactor < 0.00001)
      volume->xferFactor = 0.00001;
    break;

  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    volume->flag = key - '0';
    break;

  case '<':
  case ',':
    volume->sliceSpacing *= 0.7;
    break;

  case '>':
  case '.':
    volume->sliceSpacing /= 0.7;
    if (volume->sliceSpacing > 1)
      volume->sliceSpacing = 1;
    break;

  case 'b':
    volume->drawBB = !volume->drawBB;
    break;

  case 't':
    drawTexture = !drawTexture;
    break;

  case 'W':
    OutputViewParams( "viewParameters.data" );
    cout << "Success: Writing view parameters to file." << endl;
    break;

  case 'R':
    ReadViewParams( "viewParameters.data" );
    cout << "Success: Reading view parameters from file." << endl;
    glutPostRedisplay();
    break;
  }
      
  glutPostRedisplay();
}
