//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: LineFog.f.glsl,v 1.3 2015/08/01 00:08:17 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Fragment shader assigns a single unmodified color to the fragment.
#version 150

uniform vec4							color;					// Color of the fragment.
uniform vec4							fogColor;				// Color of the fog.

in float								fogDensity;				// Fog density interpolated between vertices.

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	// Calculate fog effect on fragment color that increases alpha with fog density.
	// Fog density value is 1 for full fog, 0 for no fog.
	fragColor = mix(color, fogColor, fogDensity);
}
