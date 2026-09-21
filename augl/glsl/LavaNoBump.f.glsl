//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: LavaNoBump.f.glsl,v 1.1 2016/01/18 22:54:36 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 140

uniform vec3							fogColor;				// Fog color
uniform sampler2D						tex;					// Texture.

in vec4									vertColor;				// Incoming fragment color interpolated from the vertex color.
in vec3									vertTexCoordFog;		// Incoming texture coordinate (xy) and fog density (z).
in vec4									vertTexCoord2;			// Incoming texture coordinate.

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	// Lava is not transparent.
	fragColor = vec4((texture(tex, vertTexCoordFog.xy).rgb +
				texture(tex, vertTexCoord2.xy).rgb +
				texture(tex, vertTexCoord2.zw).rgb) / 6, 1) * vertColor;

	// Calculate fog effect on fragment color that increases alpha with fog density.
	// Fog density value is 1 for full fog, 0 for no fog.
	fragColor = mix(fragColor, vec4(fogColor,1), vertTexCoordFog.z);
}
