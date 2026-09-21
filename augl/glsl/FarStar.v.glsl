//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: FarStar.v.glsl,v 1.7 2018/05/01 15:03:39 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Vertex shader for rendering a texture.
#version 150

uniform mat4							pmv;					// Projection model view matrix.
uniform float							size;					// Base size of points.

in vec3									position;				// Incoming vertex location. w is the point size in pixels.

void main()
{
	gl_Position = pmv * vec4(position,1.);	// Calc homogeneous vertex position using projection and model view matrices.
	float dist = length(position);			// Distance in sectors.
	gl_PointSize = clamp(size / (.15 * dist), 0, size);
}
