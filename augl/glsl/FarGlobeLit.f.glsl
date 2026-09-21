//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: FarGlobeLit.f.glsl,v 1.7 2016/09/05 19:52:16 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform vec4							atmosphereColor;		// Color of the atmosphere. Alpha is the atmosphere density.
uniform sampler2D						texPlanet;				// Texture of the planet surface.

in vec4									vertColor;				// Incoming fragment color interpolated from the vertex color.
in vec2									vertTexCoord;			// Incoming texture coordinate.

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	// Combine the result.
	// Incoming vertex color provides modulation due to lighting.
	fragColor = vertColor * vec4(mix(texture(texPlanet, vertTexCoord + vec2(.5,0)).rgb, atmosphereColor.rgb, atmosphereColor.a), 1);
}
