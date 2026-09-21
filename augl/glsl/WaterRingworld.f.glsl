//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: WaterRingworld.f.glsl,v 1.11 2019/05/03 00:39:50 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform vec3							clipCenter;				// Clip center in ringworld section space.
uniform float							clipDist;				// Clip distance. Fragments within this distance are discarded.
uniform vec3							fogColor;				// Fog color
uniform float							opacity;				// Opacity of water
uniform sampler2D						tex;					// Texture.
uniform sampler2D						texBump;				// Texture.

in vec3									lightColor;				// Incoming fragment color interpolated from the light color.
in vec3									sunColor;				// Incoming fragment color interpolated from the sun color.
in vec3									vertTexCoordFog;		// Incoming texture coordinate (xy) and fog density (z).
in vec2									vertTexCoord2;			// Incoming texture coordinate.
in vec2									vertTexCoord3;			// Incoming texture coordinate.
in vec3									vertPos;				// Vertex position in ringworld section space.

in vec3									eyeDir;
in vec3									sunCol0;
in vec3									sunDir0;
in float								sunInt0;
in vec3									sunCol1;
in vec3									sunDir1;
in float								sunInt1;
in vec3									sunCol2;
in vec3									sunDir2;
in float								sunInt2;

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

	vec3 waterColor =
		(texture(tex, vertTexCoordFog.xy).rgb + texture(tex, vertTexCoord2).rgb + texture(tex, vertTexCoord3).rgb) / 6;

	// Bump mapping.
	{
		vec3 normDelta = texture(texBump, vertTexCoord2).rgb - vec3(.5,.5,.5);
		/*
		{
			// Procedurally generated bumps.
			float bumpDensity = 1;
			float bumpSize = .15;

			vec2 c = bumpDensity * vertTexCoordFog.xy;
			vec2 p = fract(c) - vec2(.5, .5);

			float d = dot(p,p);
			float f = inversesqrt(d + 1.);
			if(d >= bumpSize)
			{
				p = vec2(0.0);
				f = 1.0;
			}

			normDelta = vec3(p.x, p.y, 1.) * f;
		}
		*/
		normDelta = normalize(normDelta);

		// Specular highlighting.
		vec3 ed = normalize(eyeDir);
		float specularFactor = .5;
		float shininess = 100.0;

		// Modulate sunlight.
		float sunFact;
		vec3 specCol;
		if(sunInt0 > 0)
		{
			vec3 s0 = normalize(sunDir0);
			sunFact = max(dot(normDelta, s0), 0.0) * sunInt0;

			vec3 reflectDir = normalize(reflect(s0, normDelta));
			float spec0 = max(dot(ed, reflectDir), 0.0);
			spec0 = pow(spec0, shininess);
			spec0 *= specularFactor * sunInt0;
			specCol = sunCol0 * spec0;
		}
		else
		{
			sunFact = 0;
			specCol = vec3(0);
		}

		if(sunInt1 > 0)
		{
			vec3 s1 = normalize(sunDir1);
			sunFact += max(dot(normDelta, s1), 0.0) * sunInt1;

			vec3 reflectDir = normalize(reflect(s1, normDelta));
			float spec1 = max(dot(ed, reflectDir), 0.0);
			spec1 = pow(spec1, shininess);
			spec1 *= specularFactor * sunInt1;
			specCol += sunCol1 * spec1;
		}

		if(sunInt2 > 0)
		{
			vec3 s2 = normalize(sunDir2);
			sunFact += max(dot(normDelta, s2), 0.0) * sunInt2;

			vec3 reflectDir = normalize(reflect(s2, normDelta));
			float spec2 = max(dot(ed, reflectDir), 0.0);
			spec2 = pow(spec2, shininess);
			spec2 *= specularFactor * sunInt2;
			specCol += sunCol2 * spec2;
		}

		float ambient = dot(normDelta, vec3(0,0,1));

		fragColor.rgb = min(waterColor * (sunColor * min(sunFact, 1)) + specCol +
			(waterColor * lightColor * vec3(ambient,ambient,ambient)), vec3(1,1,1));
	}

	// Calculate fog effect on fragment color that increases alpha with fog density.
	// Fog density value is 1 for full fog, 0 for no fog.
	fragColor.rgb = mix(fragColor.rgb, fogColor, vertTexCoordFog.z);
	fragColor.a = opacity * clipAlpha;
}
