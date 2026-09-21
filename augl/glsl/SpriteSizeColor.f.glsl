//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: SpriteSizeColor.f.glsl,v 1.1 2020/01/17 23:57:18 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform sampler2D						tex;					// Texture.

in vec4									spriteColor;

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	fragColor = texture(tex, gl_PointCoord) * spriteColor;
	//if(fragColor.a < .1)
		//discard;
}
