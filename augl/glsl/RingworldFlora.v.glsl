//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: RingworldFlora.v.glsl,v 1.4 2016/09/09 00:14:30 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform mat4							pmv;					// Projection model view matrix.

in vec4									position;				// Incoming vertex location(xyz), snow coverage(w).
in vec4									texCoordAltVeg;			// Incoming texture coordinate(xy), altitude(z), veg density(w).

out vec4								vertTexCoordAltVeg;		// Outgoing texture coordinate(xy), altitude(z), veg density(w).
out float								vertSnow;				// Outgoing snow coverage.

void main()
{
	gl_Position = pmv * vec4(position.xyz,1);				// Calculate position using projection and model view matrices.

	vertTexCoordAltVeg = texCoordAltVeg;						// Pass unmodified texture coordinate to the fragment shader.
	vertSnow = position.w;
}
