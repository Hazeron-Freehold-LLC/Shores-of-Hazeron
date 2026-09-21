//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: LineFog.v.glsl,v 1.8 2016/09/09 00:12:06 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Vertex shader only projects a vertex location.
#version 150

uniform vec2							fogParm;				// Screen depth start and dropoff depth values for linear fog.
uniform mat4							pmv;					// Projection model view matrix.

in vec4									position;				// Incoming vertex location.

out float								fogDensity;				// Outgoing fog density

void main()
{
	gl_Position = pmv * position;	// Calculate homogeneous vertex position using projection and model view matrices.

	// Calculate fog density but save the effect for the fragment program.
	// Fog density value is 1 for full fog, 0 for no fog.
	fogDensity = clamp((max(gl_Position.z,1) - fogParm.x) / fogParm.y, 0, 1);
}
