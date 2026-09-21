//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: Splat.f.glsl,v 1.3 2015/08/01 00:08:17 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform vec4							color;					// Color used to modulate texture.
uniform float							splatScale;
uniform sampler2D						tex;					// Texture.

in vec2									vertTexCoord;			// Incoming texture coordinate (xy)

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	// Calculate scaled texture coordinate such that texCoord 0,0 is centered in the texture at .5,.5.
	// At splatScale 1, texCoord -1,-1 becomes 0,0 in the actual texture lookup, which is the bottom left corner of the texture.
	// At splatScale 1, texCoord 1,1 becomes 1,1 in the actual texture lookup, which is the top right corner of the texture.
	// splatScale changes the size of the resulting texture coordinates such that splatScale .5 appears half as big as splatScale 1.
	vec2 tc = vertTexCoord / 2 / splatScale + vec2(.5, .5);

	// Prevent repetition of the texture pattern. We only want one, centered at texCoord 0,0.
	if(tc.x < 0)
		discard;
	if(tc.x > 1)
		discard;
	if(tc.y < 0)
		discard;
	if(tc.y > 1)
		discard;

	// Modulate texture color with current color.
	fragColor = texture(tex, tc) * color;
}
