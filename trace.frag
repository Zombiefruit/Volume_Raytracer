// trace.frag
//
// Fragment shader for tracing through refractive volume

#version 330

uniform sampler3D texture_volume;     // tau values
uniform sampler3D texture_gradient;   // gradient values 
uniform sampler2D texture_background; // background 2D grid texture


uniform float stepSize;         // step size in texture coordinate system
uniform float densityFactor;    // factor by which to change optical densities

uniform int   flag;             // flag from C++ for any purpose.  Press digit to change flag.

uniform vec3  volumeOrigin;     // 000 corner of volume in WCS 
uniform vec3  volumeX;          // x axis of volume in WCS
uniform vec3  volumeY;          // y axis of volume in WCS
uniform vec3  volumeZ;          // z axis of volume in WCS

uniform vec3  bgOrigin;	        // 00 corner of background texture in WCS
uniform vec3  bgX;		//    - background texture is a grid in directions bgX and bgY
uniform vec3  bgY;		//    - background texture normal is in direction bgZ
uniform vec3  bgZ;              //    - bgX, bgY, bgZ are unit vectors in WCS
uniform float bgXdim;		//    - bgXdim, bgYdim are x and y dimensions in WCS
uniform float bgYdim;

uniform vec3  eyePosition;      // eye position in WCS

in vec3 texCoords;              // texCoords of this fragment on the face of the volume

out vec4 fragColour;		// output colour



// Determine the direction in which a ray refracts or reflects when
// cross a surface from index of refraction n1 to index of refraction
// n2.
//
// The incomingDir is the direction of the ray hitting the surface.
// The gradient is the volume gradient at the ray/surface point.  The
// gradient is not normalized and might be very small, in which case
// the direction doesn't change.  The gradient could point into or out
// of the surface.
//
// The ray might undergo total internal reflection if its angle of
// incidence is large enough.  That is also handled by this code.


vec3 refractionOrReflectionDirection( float n1, float n2, vec3 incomingDir, vec3 gradient )

{

  float n = n1 / n2;
  float cosI;
  float cosT;
  float sinT2; 
  vec3 outgoingDir;


  if (dot(incomingDir, gradient) > 0) {
    gradient = -gradient;
  }

  vec3 normal = normalize( gradient );
  vec3 incident = normalize( incomingDir );

  // For areas of no density change, the gradient will be very small.
  // Don't change direction in those areas.
  // The following code for this section has been sourced from this helpful
  // paper: http://graphics.stanford.edu/courses/cs148-10-summer/docs/2006--degreve--reflection_refraction.pdf
  // However, completely identical results are obtained with the default opengl functions reflect and refract.
  // This is no surprise, as they use identical equations.
  if (length(gradient) < 0.01 ) {
    outgoingDir = incomingDir;
  } else {
    cosI = -dot(normal, incident);
    sinT2 = n * n * (1.0 - cosI * cosI);
    // Incident angle is larger than the critical angle for this intersection, so
    // we have total internal reflection
    if (sinT2 > 1.0) {
      //outgoingDir = reflect(incident, normal);
      outgoingDir = incident + 2 * cosI * normal;
    } else {
      // Refraction takes place. This is just figuring out the 
      // direction of the outgoing ray, so we don't use Fresnel
      //outgoingDir = refract(incident, normal, n);
      cosT = sqrt(1.0 - sinT2);
      outgoingDir = n * incident + (n * cosI - cosT) * normal;
    }
  }
 
  // Return the noramlized new direction
  
  return normalize( outgoingDir ); 
}


// Determine the RGB value from the background texture as seen from a
// ray starting at 'start' in direction 'dir'.
//
// The texture has origin bgOrigin and local coordinate system
// <bgX,bgY,bgZ).  It has dimension bgXdim in the bgX direction and
// dimension bgYdim in the bgY direction.
//
// All vectors are in the WCS.


vec3 lookupBackgroundTexture( vec3 start, vec3 dir )

{
  // Find the intersection of "start + t dir" with the plane "n dot x
  // = k" of the texture.  Return white if the ray is close to being
  // parallel to the plane.
  
  float k = dot(-dir, normalize(bgZ));
  float t = dot(normalize(bgZ), start - bgOrigin)/k;

  vec3 intersection = start + t*dir;
  vec2 intersection2d = vec2(dot(intersection - bgOrigin, bgX)/bgXdim, dot(intersection - bgOrigin, bgY)/bgYdim);
  vec3 texValue;
   
  // If the intersection point is outside the bounds of the texture,
  // return white.  
  if (intersection2d.x > bgXdim || intersection2d.y > bgYdim || k < 0.001) {
     texValue = vec3(1, 1, 1);
  } else {
     texValue = (texture2D(texture_background, intersection2d)).xyz;
  }

  // Return the texture value at the intersection point
  return texValue; 
}



// We'll trace through the volume using the Texture Coordinate System
// (TCS) of the volume, which has its origin at the 000 corner of the
// volume and has x,y,z axes that span the corresponding edges of the
// volume.  The volume spans [0,1]x[0,1]x[0,1] in the TCS.
//
// In the WCS, the volume has origin volumeOrigin and local coordinate
// system <volumeX,volumeY,volumeZ).
//
// Note that the volumeX, volumeY, and volumeZ vectors are *not*
// unit-length; their length is that of the corresponding edge of the
// volume in the WCS.


void main()

{
  //Compute normalized versions of vectors volumeX, volumeY, and volumeZ, as they will be used repeatedly
  vec3 normVolumeX = normalize(volumeX);
  vec3 normVolumeY = normalize(volumeY);    
  vec3 normVolumeZ = normalize(volumeZ);

  // Get TCS position of fragment
  vec3 pos = texCoords;

  // Get WCS position and direction into the volume from the eye position
  vec3 fragPosition = volumeOrigin + (pos.x*volumeX + pos.y*volumeY + pos.z*volumeZ);

  // In WCS* mat3(bgX, bgY, bgZ)
  vec3 dirToFrag = normalize(fragPosition - eyePosition) ;

  // Convert WCS direction to normalized TCS direction
  // We need to get a normalized value (0 to 1) of the projection of dirToFrag onto each axis
  float dirX = dot(normVolumeX, dirToFrag);
  float dirY = dot(normVolumeY, dirToFrag);
  float dirZ = dot(normVolumeZ, dirToFrag);

  vec3 dir = vec3(dirX, dirY, dirZ);

  // Debugging: Press '1' to check that lookupBackgroundTexture() is
  // working.  It should appear as if there's no volume when this is
  // done, since the texture behind the volume is shown.  (You can
  // press 'b' to show the volume outline while doing this.)

  if (flag == 1) {
    fragColour = vec4( lookupBackgroundTexture( fragPosition, dirToFrag ), 1.0 );
    return;
  }
  
  // Maintain the total transparency of the ray travelled so far

  float totalTransparency = 1.0;

  // Store the opacity at the previous step

  float prevTau = 0.0;          // initially = value outside of volume

  // Limit the number of steps to that corresponding to 10 times the
  // distance across the volume.

  int maxNumSteps = int( 10.0 / stepSize );

  // Step through the volume

  int i;
  for (i=0; i<maxNumSteps; i++) {

    // Accumulate transparency

    float transFactor = 1.0 - (prevTau * stepSize * densityFactor);	
    totalTransparency *= transFactor;

    // Find the gradient halfway between previous and next positions

    vec3 gradient = (texture(texture_gradient, pos).rgb) / 2.0;

    // Get tau at the next position

    float tau = texture(texture_volume, pos).x;

    // We'll assume that the index of refraction is 1+tau*k, for some
    // 'densifying' factor k that globally varies the density.

    float n1 = 1.0 + prevTau * densityFactor;
    float n2 = 1.0 + tau * densityFactor;

    // Compute the new direction based on Snell's law.

    vec3 newDir = refractionOrReflectionDirection(n1, n2, dir, gradient);

    // Debugging: Press '2' or '3' to skip the refraction/reflection
    // step above.  The ray should go straight through the volume and
    // be coloured with the texture coordinates at the exit point (see
    // farther below).

    if (flag == 2 || flag == 3)
      newDir = dir; // no refraction/reflection

    // Move the position half a step forward in the original
    // direction, then half a step forward in the new direction.

    pos += 0.5 * (stepSize * dir) + 0.5 * (stepSize * newDir);

    // Stop if we have exited the volume (but only *after* the first
    // iteration so that we will have travelled at least one step into
    // the volume).
   if (i > 0 && (pos.x > 1.0 || pos.y > 1.0 || pos.z > 1.0 || pos.x < 0.0 || pos.y < 0.0 || pos.z < 0.0)) {
     break;
   }

    // Update and continue

    prevTau = tau;
    dir = newDir;
  }

  // If too many steps were taken, output white.
 
  if (i == maxNumSteps) {
    fragColour = vec4( 1, 1, 1, 1 );
    return;
  }

  // Debugging: If '2', show texture coordinates at exit point.  If
  // '3', show total transparency in straight line to exit point.
  //
  // Note: When using '3', increase the density factor (e.g. to 1.0 or
  // 2.0) to see the accumulated transparency.

  if (flag == 2) {
    fragColour = vec4( pos, 1 );
    return;
  } else if (flag == 3) {
    fragColour = vec4( totalTransparency, totalTransparency, totalTransparency, 1 );
    return;
  }

  // Find exiting position and direction in WCS
  vec3 exitPos = volumeOrigin + ((pos.x * volumeX) + (pos.y * volumeY) + (pos.z * volumeZ));
  vec3 exitDir = normalize(dir.x*normVolumeX + dir.y*normVolumeY + dir.z*normVolumeZ);

  // Given pos and dir in WCS, find the colour coming from that
  // direction and modulate it by the accumulated transparency

  fragColour = totalTransparency * vec4(lookupBackgroundTexture(exitPos, exitDir), 1);
}
