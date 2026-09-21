//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: SnowRingworld.f.glsl,v 1.3 2015/08/01 00:08:17 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Fragment shader for rendering a single texture modulated using a color.
#version 150

uniform vec3							fogColor;				// Fog color
uniform sampler2D						tex;					// Texture.

in vec4									vertColor;				// Incoming fragment color interpolated from the vertex color.
in vec3									vertTexCoordFog;		// Incoming tex coord (xy) and fog density (z).

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	fragColor = texture(tex, vertTexCoordFog.xy) * vertColor;
	if(fragColor.a < .1)
		discard;

	// Calculate fog effect on fragment color that increases alpha with fog density.
	// Fog density value is 1 for full fog, 0 for no fog.
	fragColor = mix(fragColor, vec4(fogColor,1), vertTexCoordFog.z);
}
