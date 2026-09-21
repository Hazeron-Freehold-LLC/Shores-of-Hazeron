//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: WaterRingworldNoBump.f.glsl,v 1.4 2019/05/03 00:40:24 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 140

uniform vec3							clipCenter;				// Clip center in ringworld section space.
uniform float							clipDist;				// Clip distance. Fragments within this distance are discarded.
uniform vec3							fogColor;				// Fog color
uniform float							opacity;				// Opacity of water
uniform sampler2D						tex;					// Texture.

in vec4									vertColor;				// Incoming fragment color interpolated from the vertex color.
in vec3									vertTexCoordFog;		// Incoming texture coordinate (xy) and fog density (z).
in vec4									vertTexCoord2;			// Incoming texture coordinate.
in vec3									vertPos;				// Vertex position in ringworld section space.

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	// Blend out pixels that are over the center area.
	float clipAlpha;
	if(clipDist > 0)
	{
		float l = length(vertPos - clipCenter);
		if(l < clipDist)
		{
			l /= clipDist;
			l /= clipDist;	// 0 at center, 1 at radius
			l = 1 - l;		// 1 at center, 0 at radius
			l *= 2;
			if(l >= 1)
				discard;
			clipAlpha = cos(l * 3.14159/2);
		}
		else
			clipAlpha = 1;
	}
	else
		clipAlpha = 1;

	fragColor = vec4((texture(tex, vertTexCoordFog.xy).rgb +
				texture(tex, vertTexCoord2.xy).rgb +
				texture(tex, vertTexCoord2.zw).rgb) / 6, opacity) * vertColor;

	// Calculate fog effect on fragment color that increases alpha with fog density.
	// Fog density value is 1 for full fog, 0 for no fog.
	fragColor = mix(fragColor, vec4(fogColor,1), vertTexCoordFog.z);
	fragColor.a *= clipAlpha;
}
