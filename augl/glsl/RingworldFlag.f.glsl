//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: RingworldFlag.f.glsl,v 1.3 2015/08/01 00:08:17 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform vec3							fogColor;				// Fog color
uniform sampler2D						texFlag;				// Texture of the flag.

in vec4									vertColor;				// Incoming fragment color interpolated from the vertex color.
in float								vertFog;				// Incoming fog density
in vec2									vertTexCoord;			// Incoming surface texture coordinate(xy)

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	fragColor = texture(texFlag, vertTexCoord);
	if(fragColor.a < .15)
		discard;

	// Modulate with vertColor for lighting effect.
	fragColor *= vertColor;

	// Calculate fog effect on fragment color but do not modify the alpha using fog.
	// Fog density value is 1 for full fog, 0 for no fog.
	fragColor.rgb = mix(fragColor.rgb, fogColor, vertFog);
	fragColor.a = 1;
}
