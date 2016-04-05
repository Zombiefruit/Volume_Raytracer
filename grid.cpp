// grid.cpp


#include "headers.h"

#include "grid.h"
#include "linalg.h"


void Grid::renderGrid( GPUProgram *gpuProg )

{
  static bool firstTime = true;

  if (firstTime) {

    firstTime = false;

    // VAO

    glGenVertexArrays( 1, &VAO );
    glBindVertexArray( VAO );

    // positions & texCoords

    vec2 verts[] = {
      vec2(0,0), vec2(1,0), vec2(0,1),
      vec2(0,1), vec2(1,0), vec2(1,1)
    };

    // VBO

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), (GLfloat*) &verts[0], GL_STATIC_DRAW);

    // Position (attribute 0)

    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, 0 );

    // Colour (attribute 1)

    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, 0 );

    // Create a grid texture

    const int texDim = 1024;

    vec3 *texture = new vec3[ texDim * texDim ];

    vec3 bg(1,1,1);

    for (int i=0; i<texDim*texDim; i++)
      texture[i] = bg;

    const int numDivs = 30;

    vec3 fg(0,0,0);

    for (int i=0; i<numDivs; i++) {
      int offset = rint( i*texDim/(float)numDivs );
      for (int j=0; j<texDim; j++) {
	texture[j+offset*texDim] = fg;
	texture[offset+j*texDim] = fg;
      }
    }

    int offset = texDim-1;
    for (int j=0; j<texDim; j++) {
      texture[j+offset*texDim] = fg;
      texture[offset+j*texDim] = fg;
    }

    // Register it with OpenGL

    glGenTextures( 1, &textureID );
    glBindTexture( GL_TEXTURE_2D, textureID );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

    gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGB, texDim, texDim, GL_RGB, GL_FLOAT, texture );
  }

  // Draw

  glBindVertexArray( VAO );

  glEnable( GL_TEXTURE_2D );
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, textureID );
  gpuProg->setInt( "textureID", 0 );
  
  glDrawArrays( GL_TRIANGLES, 0, 6 );
  
  glDisable( GL_TEXTURE_2D );
}
