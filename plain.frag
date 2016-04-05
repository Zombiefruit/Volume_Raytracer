// Plain fragment shader

#version 300 es
precision mediump float;

in  vec3 colour;
out vec4 outputColour;

void main()

{
  outputColour = vec4( colour, 1 );
}	
