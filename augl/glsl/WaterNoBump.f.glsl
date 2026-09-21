//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: WaterNoBump.f.glsl,v 1.4 2019/05/03 00:34:51 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 140

uniform float							clipCos;				// Clip area cosine.
uniform vec3							clipUp;					// Clip area 'up' vector in globe space.
uniform vec3							fogColor;				// Fog color
uniform float							opacity;				// Opacity of water
uniform sampler2D						tex;					// Texture.

in vec4									vertColor;				// Incoming fragment color interpolated from the vertex color.
in vec3									vertTexCoordFog;		// Incoming texture coordinate (xy) and fog density (z).
in vec4									vertTexCoord2;			// Incoming texture coordinate.
in vec3									vertUp;					// Vertex 'up' vector in globe space.

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	// Blend out pixels that are over the center area.
	float clipAlpha;
	if(clipCos < 1)
	{
		float VdotC = dot(normalize(vertUp), clipUp);
		if(VdotC > clipCos)
		{
			float l = (VdotC-clipCos) / (1-clipCos);	// 1 at center, 0 at radius
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
