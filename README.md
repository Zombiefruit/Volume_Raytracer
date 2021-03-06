# Introduction
This assignment had us modify a fragment shader (trace.frag) to trace rays through a volume, incorporating refraction and internal reflection. The volumes were sourced from the URL given in PVMSource.txt, included in this repo.

# Overview
As a brief overview, the application:
- figures out the texture coordinate space (TCS) position and direction of the fragment in question, as well as finding that same fragment's position and direction in the world coordinate system (WCS)
- steps through the volume, incrementing a variable to track the accumlated transparency of the volume (which will modify the final fragment colour) *and* computing the changing direction of a ray as the index of refraction changes for each slice of the volume stepped through
- finally computing the exit position and direction of a ray which has travelled through the volume, which is then used to determine the fragment colour

# Images
This is a volume representing a hydrogen atom and its orbitals. Here, we are just visualizing the density of the volume (press '3', and hit the '+' key to increase the density factor enough to visualize this yourself).

![Hydrogen Density](/src/HydrogenDensity.png)

This is the same volume, now with refraction and reflection (the density factor is low enough to not attenuate the colour much - it is mostly the direction of the ray that changes).

![Hydrogen reflection and refraction](/src/Hydrogen.png)

And here are two images of a volume representing a lens. The banding is due to the fact that the volume is a series of slices. Decreasing the spacing value improves this effect, but cannot remove it entirely.

![Lens from the front](/src/Lens.png)

![Lens from the side](/src/Lens2.png)

# Running the application
All you have to do is download the source, run "make" in the source directory, and then run "./volren <*file*.pvm>", where *file* is one of the .pvm files included (i.e. "./volren Hydrogen.pvm" will run the application with the Hydrogen volume). Pressing the '+' and '-' keys increase and decrease the density factor. The '<' and '>' keys modify the slice-spacing. Pressing '4' will visualize reflection and refraction. 

# Disclaimer
Our task was to modify the refractionOrReflectionDirection(), lookupBackgroundTexture(), and main() functions in the trace.frag shader. We were supplied with the other code, which was written by Professor James Stewart, at Queen's University (http://research.cs.queensu.ca/~jstewart/).

