//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: TexOffset.f.glsl,v 1.4 2015/08/01 00:08:17 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform vec4							color;					// Color used to modulate texture.
uniform float							discardAlpha;			// Alpha below which we discard fragments.
uniform sampler2D						tex;					// Texture.

in vec2									vertTexCoord;			// Incoming texture coordinate (xy)

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	fragColor = texture(tex, vertTexCoord) * color;
	if(fragColor.a < discardAlpha)
		discard;
}
