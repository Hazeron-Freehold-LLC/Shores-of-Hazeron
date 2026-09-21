//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: GalaxyStar.v.glsl,v 1.3 2020/01/28 16:13:58 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Vertex shader for rendering a texture.
#version 150

uniform mat4							mv;						// Model view matrix.
uniform mat4							p;						// Projection
uniform float							size;					// Base size of points.
uniform vec2							intenDist;				// Distance at which intensity is at the minimum(x) and max(y).
uniform vec4							intenRanges;			// intensity range(xy), transparencyRange(zw)

in vec3									position;				// Incoming vertex location.

out vec2								alphaInt;				// Outgoing texture alpha(x), intensity(y)

void main()
{
	gl_Position = mv * vec4(position,1.);		// Calc homogeneous vertex position using projection and model view matrices.
	float dist = length(position);				// Distance in sectors.
	float depthRatio = 1 - (dist - intenDist.y) / (intenDist.x - intenDist.y);	// Ratio of depth into galaxy. center = .5
	alphaInt.x = clamp(depthRatio, intenRanges.z, intenRanges.w);	// Get more transparent with distance across the galaxy.
	alphaInt.y = clamp(depthRatio, intenRanges.x, intenRanges.y);	// Get less intense with distance across the galaxy.
	gl_Position = p * gl_Position;
	gl_PointSize = clamp(size / (.25 * dist), 0, size);
}
