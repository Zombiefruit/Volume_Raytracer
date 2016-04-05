// vertex shader for texture

#version 300 es
precision mediump float;

uniform mat4 MVP;

layout (location = 0) in vec2 posIn;
layout (location = 1) in vec2 texCoordIn;

out vec2 texCoord;

void main()

{
  gl_Position = MVP * vec4( posIn, 0, 1 );
  texCoord = texCoordIn;
}
