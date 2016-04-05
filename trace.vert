// trace.vert
//
// Vertex shader for tracing through refractive volume

#version 330

uniform mat4 MVP;		// OCS-to-CCS

layout (location = 0) in vec3 vertPosition;
layout (location = 1) in vec3 vertColour;

out vec3 texCoords;		// texCoords on face of volume

void main()

{
  gl_Position  = MVP * vec4( vertPosition, 1 );

  // Pass texture coordinates to the fragment shader

  texCoords = vertColour;
}
