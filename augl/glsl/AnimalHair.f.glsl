//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: AnimalHair.f.glsl,v 1.3 2019/07/27 00:04:42 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform sampler2D						texHair;				// Texture of hair.

in vec4									vertColor;				// Incoming fragment color interpolated from the vertex color.
in vec2									texCoord;				// Incoming texture coordinate (xy).
in vec4									hairColor;				// Color of the hair.

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	vec4 texCol = texture(texHair, texCoord);
	//if(texCol.a < .1)
		//discard;
	fragColor = texCol * hairColor * vertColor;
}
