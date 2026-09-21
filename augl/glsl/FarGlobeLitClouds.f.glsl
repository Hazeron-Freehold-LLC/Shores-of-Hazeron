//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: FarGlobeLitClouds.f.glsl,v 1.1 2016/09/05 22:06:16 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform vec4							atmosphereColor;		// Color of the atmosphere. Alpha is the atmosphere density.
uniform vec2							lowMaxWx;				// low layer maximum x=SaturationP, y=VaporP
uniform vec2							lowMinWx;				// low layer minimum x=SaturationP, y=VaporP
uniform sampler2D						lowTexWx;				// Low layer wx texture r=SaturationP, g=VaporP, b=instability
uniform vec2							offWx;					// horiz offset x=SaturationP, y=VaporP
uniform sampler2D						texPlanet;				// Texture of the planet surface.

in vec4									vertColor;				// Incoming fragment color interpolated from the vertex color.
in vec2									vertTexCoord;			// Incoming texture coordinate.

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	// Calculate alpha value due to low cloud layer.
	float lowClouds;
	{
		// Calculate saturation pressure without banding as if flow east is 0 and flow west is 1.
		vec2 tcoord;
		tcoord.y = vertTexCoord.y;
		tcoord.x = fract(vertTexCoord.x - offWx.x + 1);
		vec3 colW = texture(lowTexWx, tcoord).rgb;
		float sp = lowMinWx.x + colW.r * (lowMaxWx.x - lowMinWx.x);
		if(sp > 0)
		{
			// Calculate vapor pressure without banding.
			vec2 vpcoord;
			vpcoord.y = vertTexCoord.y;
			vpcoord.x = fract(vertTexCoord.x - offWx.y + 1);
			float vp = lowMinWx.y + texture(lowTexWx, vpcoord).g * (lowMaxWx.y - lowMinWx.y);
			vp *= 1 + (colW.b - .5) / 100;
			if(vp > sp)
				lowClouds = vp / sp - 1;
			else
				lowClouds = 0;
		}
		else
			lowClouds = 0;
	}

	// Atmosphere alpha is the combined density of the atmosphere and both cloud layers, maxing out at 1.
	float atmoAlpha = min(atmosphereColor.a + lowClouds, 1);

	// Combine the result.
	// Incoming vertex color provides modulation due to lighting.
	fragColor = vertColor * vec4(mix(texture(texPlanet, vertTexCoord + vec2(.5,0)).rgb, atmosphereColor.rgb, atmoAlpha), 1);
}
