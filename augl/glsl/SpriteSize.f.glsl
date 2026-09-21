//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: SpriteSize.f.glsl,v 1.3 2015/08/01 00:08:17 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform vec4							color;					// Sprite color
uniform sampler2D						tex;					// Texture.

in float								fragAlpha;				// Incoming interpolated fragment alpha.

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	fragColor = texture(tex, gl_PointCoord) * vec4(color.rgb, color.a*fragAlpha);
	//if(fragColor.a < .1)
		//discard;
}
