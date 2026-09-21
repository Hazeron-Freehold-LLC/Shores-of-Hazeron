//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: FarGlobeLitLava.f.glsl,v 1.9 2016/09/05 19:52:49 daveh Exp $
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
	vec3 atmoColor = atmosphereColor.rgb * vertColor.rgb;

	// 1-alpha in the far globe texture is a glow factor.
	vec4 surfaceColor = texture(texPlanet, vertTexCoord + vec2(.5,0));
	float glow = 1-surfaceColor.a;								// 1-alpha in the near globe texture is a glow factor.
	vec3 glowColor = vertColor.rgb + vec3(glow,glow,glow);		// Brighten incoming vertex color.
	glowColor = min(glowColor, vec3(1,1,1));
	surfaceColor.rgb *= glowColor;

	fragColor = vec4(mix(surfaceColor.rgb, atmoColor, atmosphereColor.a), 1);
}
