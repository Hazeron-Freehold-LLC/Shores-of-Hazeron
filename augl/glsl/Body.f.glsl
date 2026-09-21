//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: Body.f.glsl,v 1.4 2015/08/01 00:08:17 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform sampler2D						texClothes;				// Texture.
uniform vec3							texClothesOffsetWear;	// x-uOffset, y-vOffset, z-wear
uniform sampler2D						texSkin;				// Texture.
uniform vec4							texSkinOffset;			// x-uTOffset, y-vTOffset, z-uBOffset, w-vBOffset

in vec4									vertColor;				// Incoming fragment color interpolated from the vertex color.
in vec3									vertTexCoordTopface;	// Incoming texture coordinate (xy) and top face(z)

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	// Do all texture lookups.
	vec4 tClothes =
		texture(texClothes, vec2(vertTexCoordTopface.x+texClothesOffsetWear.x, vertTexCoordTopface.y+texClothesOffsetWear.y));
	vec4 topColor = texture(texSkin, vec2(vertTexCoordTopface.x+texSkinOffset.x, vertTexCoordTopface.y+texSkinOffset.y));
	vec4 bottomColor = texture(texSkin, vec2(vertTexCoordTopface.x+texSkinOffset.z, vertTexCoordTopface.y+texSkinOffset.w));
	vec4 tSkin = mix(bottomColor, topColor, vertTexCoordTopface.z);

	// Decide how to blend the skin and clothes textures together to make the final color.
	// Set clothes=0 when wear value is 0, 1 when wear value is 1, tClothes.a for other wear values which combines skin and clothes.
	float clothes = texClothesOffsetWear.z <= 1 ? texClothesOffsetWear.z : tClothes.a;
	fragColor = mix(tSkin, tClothes, clothes);
	if(fragColor.a < .15)
		discard;
	fragColor *= vertColor;
}
