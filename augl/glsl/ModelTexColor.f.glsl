//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ModelTexColor.f.glsl,v 1.5 2016/08/05 19:57:35 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform sampler2D						texPaletteHigh;			// Palette of textures for various combinations of materials.
uniform sampler2D						texPaletteLow;			// Palette of textures for various combinations of materials.

in vec4									vertColor;				// Incoming fragment color interpolated from the vertex color.
in vec2									vertTexCoord;			// Incoming texture coordinate (xy)
flat in float							texTileIndex;			// Texture tile index in the palette. Must not be interpolated.

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	// Palette is 16 rows x 8 columns. Total tiles is 16*8 = 128
	vec2 tile;
	tile.x = floor(texTileIndex/16);							// Column index 0..4
	tile.y = floor(texTileIndex - (tile.x * 16));				// Row index 0..15
	tile /= vec2(8, 16);

	vec4 highColor;
	{
		// Use high resolution textures.
		vec2 tertex = tile + vec2(.5/2048., .5/4096.);			// Add 1/2 pixel to avoid seam.

		// Calculate tile texture coords.
		// Limit to 255 pixels of texture to avoid seam.
		vec2 vertTex = min(fract(vertTexCoord.xy) / vec2(8,16), vec2(255./2048., 255./4096.));

		// Convert incoming texture coordinate to a subtile coordinate in the texPaletteHigh texture.
		vec2 terCoord = tertex + vertTex;
		highColor = texture(texPaletteHigh, terCoord);			// Calculate base palette color.
	}

	vec4 lowColor;
	{
		// Use low resolution textures.
		vec2 tertex = tile + vec2(.5/128., .5/256.);			// Add 1/2 pixel to avoid seam.

		// Calculate tile texture coords for multi sampling.
		// Limit to 15 pixels of texture to avoid seam.
		vec2 vertTex = min(fract(vertTexCoord.xy) / vec2(8,16), vec2(15./128., 15./256.));

		// Convert incoming texture coordinate to a subtile coordinate in the texPaletteLow texture.
		vec2 terCoord = tertex + vertTex;
		lowColor = texture(texPaletteLow, terCoord);
	}

	// Combine the low res texture lookup with the high res texture lookup based on depth in the scene.
	fragColor = mix(highColor, lowColor, smoothstep(.9, 1.01, gl_FragCoord.z));

	// Modulate with vertColor for lighting effect.
	fragColor *= vertColor;

	// Calculate fog effect on fragment color but do not modify the alpha using fog.
	// Fog density value is 1 for full fog, 0 for no fog.
	//fragColor.rgb = mix(fragColor.rgb, fogColor, vertFogSnowPave.x);
}
