//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: RingworldFlora.f.glsl,v 1.3 2016/08/05 21:49:05 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform sampler2D						texGrass;				// Grass texture.
uniform sampler2D						texLookup;				// 1D lookup table of terrain tile indices by altitude
uniform sampler2D						texSnow;				// Snow texture.
uniform sampler2D						texTerrain[5];			// Texture palette for terrain.

in vec4									vertTexCoordAltVeg;		// Incoming texture coordinate(xy), altitude(z), veg density(w)
in float								vertSnow;				// Snow coverage

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	// The texLookup texture is only 1 pixel tall. It is created with GL_NEAREST so pixel values are not interpolated.
	// The texTerrain texture is a palette of tiles that is 16 rows by 4 columns.
	// The TERTEX_TYPE is the red value of the lookup texture, at the interpolated altitude.
	// Determine the starting terrain subtile texture coordinate based on the altitude of this fragment.
	float teralt = clamp(vertTexCoordAltVeg.z, 0., 1023./1024.);	// Terrain altitude.

	// Look up the tile index value from the lookup table. It is in the range 0..1 and must be converted to the tile coordinates.
	float terlu = texture(texLookup, vec2(teralt, 0)).r;		// Index value in the range 0..1
	int tileIndex = min(int(round(terlu * 5)), 4);				// Terrain texture index 0..4

	// Terrain. Incoming texture coordinate is for smallest pattern so we do not have to multiply texture coordinates to get there.
	// That loses precision. It is much better to divide at this point.
	vec2 terCoord1 = vertTexCoordAltVeg.xy/30.;			// Bigger pattern size.
	vec2 terCoord2 = vertTexCoordAltVeg.xy/(30.*30.);	// Biggest pattern size.
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

	// Vegetation
	// The .125 here matches the 1/8 in DaWorldGlobe::CreateGrass.
	if(vertTexCoordAltVeg.w >= .125f)
	{
		vec2 grassCoord1 = vertTexCoordAltVeg.xy/(17*30);
		vec2 grassCoord2 = vertTexCoordAltVeg.xy/30;
		vec4 grassColor1 = texture(texGrass, grassCoord1);
		vec4 grassColor2 = texture(texGrass, grassCoord2);
		vec4 grassColor = mix(grassColor1, grassColor2, .5);
		fragColor = vec4(mix(fragColor.rgb, grassColor.rgb, min(grassColor.a*1.5,1)), 1);
	}

	// Snow
	{
		vec4 snowColor = texture(texSnow, terCoord1);

		// Combine snow with terrain color.
		fragColor = mix(fragColor, snowColor, vertSnow);
	}
}
