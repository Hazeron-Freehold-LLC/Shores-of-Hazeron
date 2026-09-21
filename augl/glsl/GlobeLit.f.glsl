//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: GlobeLit.f.glsl,v 1.2 2016/08/05 20:09:24 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform float							altNoiseFactor;			// Multiplier for altitude noise.
uniform float							clipCos;				// Clip area cosine.
uniform vec3							clipUp;					// Clip area 'up' vector in globe space.
uniform vec3							fogColor;				// Fog color
uniform sampler2D						texLookup;				// 1D lookup table of terrain tile indices by altitude
uniform sampler2D						texNoise;				// Noise texture.
uniform sampler2D						texSnow;				// Snow texture.
uniform sampler2D						texTerrain[5];			// Texture palette for terrain.

in vec4									vertColor;				// Incoming fragment color interpolated from the vertex color.
in vec2									vertFogSnow;			// Incoming fog density(x), snow coverage(y).
in vec3									vertTexCoordAlt;		// Incoming texture coordinate(xy), altitude(z)
in vec3									vertUp;					// Vertex 'up' vector in globe space.

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	// Discard fragments that are too close to the clip area.
	float VdotC = dot(normalize(vertUp), clipUp);
	if(VdotC > clipCos)
		discard;

	// Noise helps to create irregular boundaries between textures, instead of perfectly straight lines.
	vec4 noiseColor = texture(texNoise, vertTexCoordAlt.xy/(2*30));
	float noise = (noiseColor.r + noiseColor.g + noiseColor.b)/3.;	// Range is 0..1 so altitude only goes up, not down.

	// The texLookup texture is only 1 pixel tall. It is created with GL_NEAREST so pixel values are not interpolated.
	// The texTerrain texture is a palette of tiles that is 16 rows by 4 columns.
	// The TERTEX_TYPE is the red value of the lookup texture, at the interpolated altitude.
	// Determine the starting terrain subtile texture coordinate based on the altitude of this fragment.
	float teralt = clamp(vertTexCoordAlt.z + noise*altNoiseFactor, 0., 1023./1024.);	// Terrain altitude with noise.

	// Look up the tile index value from the lookup table. It is in the range 0..1 and must be converted to the tile coordinates.
	float terlu = texture(texLookup, vec2(teralt, 0)).r;		// Index value in the range 0..1
	int tileIndex = min(int(round(terlu * 5)), 4);				// Terrain texture index 0..4

	// Terrain. Incoming texture coordinate is for smallest pattern so we do not have to multiply texture coordinates to get there.
	// That loses precision. It is much better to divide at this point.
	vec2 terCoord1 = vertTexCoordAlt.xy/30.;		// Bigger pattern size.
	vec2 terCoord2 = vertTexCoordAlt.xy/(30.*30.);	// Biggest pattern size.
	{
		// Samples are taken at different frequencies.
		vec4 terColor1;
		vec4 terColor2;
		if(tileIndex == 0)
		{
			terColor1 = texture(texTerrain[0], terCoord1);
			terColor2 = texture(texTerrain[0], terCoord2);
		}
		else if(tileIndex == 1)
		{
			terColor1 = texture(texTerrain[1], terCoord1);
			terColor2 = texture(texTerrain[1], terCoord2);
		}
		else if(tileIndex == 2)
		{
			terColor1 = texture(texTerrain[2], terCoord1);
			terColor2 = texture(texTerrain[2], terCoord2);
		}
		else if(tileIndex == 3)
		{
			terColor1 = texture(texTerrain[3], terCoord1);
			terColor2 = texture(texTerrain[3], terCoord2);
		}
		else
		{
			terColor1 = texture(texTerrain[4], terCoord1);
			terColor2 = texture(texTerrain[4], terCoord2);
		}
		fragColor = mix(terColor1, terColor2, .5);
	}

	// Snow
	{
		vec4 snowColor = texture(texSnow, terCoord1);

		// Combine snow with terrain color.
		fragColor = mix(fragColor, snowColor, vertFogSnow.y);
	}

	// Modulate with vertColor for lighting effect.
	fragColor *= vertColor;

	// Calculate fog effect on fragment color but do not modify the alpha using fog.
	// Fog density value is 1 for full fog, 0 for no fog.
	fragColor.rgb = mix(fragColor.rgb, fogColor, vertFogSnow.x);
}
