//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: AnimalClothingRingworld.f.glsl,v 1.1 2019/10/01 22:46:27 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform vec3							clothColor[15];			// Modulates color of everything rendered.
uniform sampler2D						texClothingDecal;		// Texture of decals on the clothing.
uniform sampler2D						texClothing[15];		// Textures for clothing.

in vec4									vertColor;				// Incoming fragment color interpolated from the vertex color.
in vec3									dyeColor;				// Incoming fragment color interpolated from the vertex color.
in vec2									texCoord;				// Incoming texture coordinate (xy)
in vec3									texCoordDecalBlend;		// Decal tex coord (xy), decal blend (z)
flat in int								texClothingIndex;		// Index of clothing texture applied to this fragment.

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	// Clothing texture provides base color.
	vec4 clothingColor;
	if(texClothingIndex == 0)
	{
		clothingColor = texture(texClothing[0], texCoord.xy);
		clothingColor.rgb *= dyeColor;
		clothingColor.rgb *= clothColor[0];
	}
	else if(texClothingIndex == 1)
	{
		clothingColor = texture(texClothing[1], texCoord.xy);
		clothingColor.rgb *= dyeColor;
		clothingColor.rgb *= clothColor[1];
	}
	else if(texClothingIndex == 2)
	{
		clothingColor = texture(texClothing[2], texCoord.xy);
		clothingColor.rgb *= dyeColor;
		clothingColor.rgb *= clothColor[2];
	}
	else if(texClothingIndex == 3)
	{
		clothingColor = texture(texClothing[3], texCoord.xy);
		clothingColor.rgb *= dyeColor;
		clothingColor.rgb *= clothColor[3];
	}
	else if(texClothingIndex == 4)
	{
		clothingColor = texture(texClothing[4], texCoord.xy);
		clothingColor.rgb *= dyeColor;
		clothingColor.rgb *= clothColor[4];
	}
	else if(texClothingIndex == 5)
	{
		clothingColor = texture(texClothing[5], texCoord.xy);
		clothingColor.rgb *= dyeColor;
		clothingColor.rgb *= clothColor[5];
	}
	else if(texClothingIndex == 6)
	{
		clothingColor = texture(texClothing[6], texCoord.xy);
		clothingColor.rgb *= dyeColor;
		clothingColor.rgb *= clothColor[6];
	}
	else if(texClothingIndex == 7)
	{
		clothingColor = texture(texClothing[7], texCoord.xy);
		clothingColor.rgb *= dyeColor;
		clothingColor.rgb *= clothColor[7];
	}
	else if(texClothingIndex == 8)
	{
		clothingColor = texture(texClothing[8], texCoord.xy);
		clothingColor.rgb *= dyeColor;
		clothingColor.rgb *= clothColor[8];
	}
	else if(texClothingIndex == 9)
	{
		clothingColor = texture(texClothing[9], texCoord.xy);
		clothingColor.rgb *= dyeColor;
		clothingColor.rgb *= clothColor[9];
	}
	else if(texClothingIndex == 10)
	{
		clothingColor = texture(texClothing[10], texCoord.xy);
		clothingColor.rgb *= dyeColor;
		clothingColor.rgb *= clothColor[10];
	}
	else if(texClothingIndex == 11)
	{
		clothingColor = texture(texClothing[11], texCoord.xy);
		clothingColor.rgb *= dyeColor;
		clothingColor.rgb *= clothColor[11];
	}
	else if(texClothingIndex == 12)
	{
		clothingColor = texture(texClothing[12], texCoord.xy);
		clothingColor.rgb *= dyeColor;
		clothingColor.rgb *= clothColor[12];
	}
	else if(texClothingIndex == 13)
	{
		clothingColor = texture(texClothing[13], texCoord.xy);
		clothingColor.rgb *= dyeColor;
		clothingColor.rgb *= clothColor[13];
	}
	else
	{
		clothingColor = texture(texClothing[14], texCoord.xy);
		clothingColor.rgb *= dyeColor;
		clothingColor.rgb *= clothColor[14];
	}
	if(clothingColor.a < .1)
		discard;

	// Clothing decals are blended onto the base clothing color.
	vec4 decalColor = texture(texClothingDecal, texCoordDecalBlend.xy);
	clothingColor = mix(clothingColor, decalColor, texCoordDecalBlend.z * decalColor.a);
	clothingColor.a = 1;

	// Final color is modulated with incoming vertex color, for lighting.
	fragColor = vertColor * clothingColor;
}
