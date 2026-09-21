//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: AnimalGlobe.f.glsl,v 1.1 2019/10/01 22:46:09 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform vec4							bottomSkinColor;			// Color to modulate bottom skin texture color.
uniform vec4							topSkinColor;			// Color to modulate top skin texture color.
uniform sampler2D						texSkinTop;				// Texture of top skin.
uniform sampler2D						texSkinBottom;			// Texture of bottom skin.
uniform sampler2D						texSkinDetail;			// Texture of decals on the skin.

in vec4									vertColor;				// Incoming fragment color interpolated from the vertex color.
in vec4									texCoordTopBottom;		// Incoming texture coordinate top (xy) and bottom (yz)
in vec4									texCoordDetailBlend;	// Detail tex coord (xy), top to bottom blend (z), decal blend (w)

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	// Get top and bottom skin colors from their textures.
	vec4 topColor = texture(texSkinTop, texCoordTopBottom.xy);
	vec4 bottomColor = texture(texSkinBottom, texCoordTopBottom.zw);

	// Modulate using skin color values.
	topColor *= topSkinColor;
	bottomColor *= bottomSkinColor;

	// Base skin color is a blend between the bottom texture and the top texture.
	vec4 skinColor = mix(bottomColor, topColor, texCoordDetailBlend.z);

	// Skin decals are blended onto the base skin color.
	vec4 decalColor = texture(texSkinDetail, texCoordDetailBlend.xy);
	skinColor = mix(skinColor, decalColor, texCoordDetailBlend.w * decalColor.a);
	skinColor.a = 1;

	// Final color is modulated with incoming vertex color, for lighting.
	fragColor = vertColor * skinColor;
}
