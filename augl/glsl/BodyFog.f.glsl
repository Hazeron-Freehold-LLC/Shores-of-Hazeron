//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: BodyFog.f.glsl,v 1.4 2015/08/01 00:08:17 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform vec3							fogColor;				// Fog color
uniform sampler2D						texClothes;				// Texture.
uniform vec3							texClothesOffsetWear;	// x-uOffset, y-vOffset, z-wear
uniform sampler2D						texSkin;				// Texture.
uniform vec4							texSkinOffset;			// x-uTOffset, y-vTOffset, z-uBOffset, w-vBOffset

in vec4									vertColor;				// Incoming fragment color interpolated from the vertex color.
in vec4									vertTexCoordFogTopface;	// Incoming texture coordinate (xy) and fog density (z), top face(w)

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	// Do all texture lookups.
	vec4 tClothes =
		texture(texClothes, vec2(vertTexCoordFogTopface.x+texClothesOffsetWear.x, vertTexCoordFogTopface.y+texClothesOffsetWear.y));
	vec4 topColor = texture(texSkin, vec2(vertTexCoordFogTopface.x+texSkinOffset.x, vertTexCoordFogTopface.y+texSkinOffset.y));
	vec4 bottomColor = texture(texSkin, vec2(vertTexCoordFogTopface.x+texSkinOffset.z, vertTexCoordFogTopface.y+texSkinOffset.w));
	vec4 tSkin = mix(bottomColor, topColor, vertTexCoordFogTopface.w);

	// Decide how to blend the skin and clothes textures together to make the final color.
	// Set clothes=0 when wear value is 0, 1 when wear value is 1, tClothes.a for other wear values which combines skin and clothes.
	float clothes = texClothesOffsetWear.z <= 1 ? texClothesOffsetWear.z : tClothes.a;
	fragColor = mix(tSkin, tClothes, clothes);
	if(fragColor.a < .15)
		discard;
	fragColor *= vertColor;

	// Calculate fog effect on fragment color but do not modify the alpha using fog.
	// Fog density value is 1 for full fog, 0 for no fog.
	fragColor.rgb = mix(fragColor.rgb, fogColor, vertTexCoordFogTopface.z);
}
