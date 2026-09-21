//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: NearStar.f.glsl,v 1.3 2015/08/01 00:08:17 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Fragment shader for rendering a single unmodulated texture.
#version 150

uniform sampler2D						tex;					// Texture.
uniform float							opacity;				// Opacity of this fragment.

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	fragColor = texture(tex, gl_PointCoord);
	fragColor.a *= opacity;
	if(fragColor.a == 0.)
		discard;
}
