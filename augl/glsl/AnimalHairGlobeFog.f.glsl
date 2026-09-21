//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: AnimalHairGlobeFog.f.glsl,v 1.1 2019/10/01 22:46:09 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform vec3							fogColor;				// Fog color
uniform sampler2D						texHair;				// Texture of hair.

in float								fog;
in vec4									hairColor;				// Color of the hair.
in vec2									texCoord;				// Incoming texture coordinate (xy).
in vec4									vertColor;				// Incoming fragment color interpolated from the vertex color.

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	vec4 texCol = texture(texHair, texCoord);
	//if(texCol.a < .1)
		//discard;
	fragColor = texCol * hairColor * vertColor;

	// Calculate fog effect on fragment color but do not modify the alpha using fog.
	// Fog density value is 1 for full fog, 0 for no fog.
	fragColor.rgb = mix(fragColor.rgb, fogColor, fog);
}
