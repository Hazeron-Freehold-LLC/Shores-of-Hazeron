//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: Globe.v.glsl,v 1.6 2016/09/09 00:11:36 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform mat4							pmv;					// Projection model view matrix.

in vec4									position;				// Incoming vertex location(xyz), snow coverage(w).
in vec3									texCoordAlt;			// Incoming texture coordinate(xy), altitude(z).

out vec4								vertTexCoordAltSnow;	// Outgoing texture coordinate(xy), altitude(z), snow(w)

void main()
{
	gl_Position = pmv * vec4(position.xyz,1);				// Calculate position using projection and model view matrices.

	vertTexCoordAltSnow.xyz = texCoordAlt;						// Pass unmodified texture coordinate to the fragment shader.
	vertTexCoordAltSnow.w = position.w;
}
