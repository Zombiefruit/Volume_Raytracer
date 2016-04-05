// fragment shader for texture

#version 300 es
precision mediump float;

uniform sampler2D textureID;

in  vec2 texCoord;

out vec4 outputColour;

void main()

{
  outputColour = vec4( texture( textureID, texCoord ).rgb, 1);
}	
