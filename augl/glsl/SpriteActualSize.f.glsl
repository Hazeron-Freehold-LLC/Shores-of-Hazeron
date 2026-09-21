//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: SpriteActualSize.f.glsl,v 1.5 2015/10/27 16:18:04 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform vec4							color;					// Sprite color
uniform sampler2D						tex;					// Texture.

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	fragColor = texture(tex, gl_PointCoord) * color;
	if(fragColor.a < .1)
		discard;
}
