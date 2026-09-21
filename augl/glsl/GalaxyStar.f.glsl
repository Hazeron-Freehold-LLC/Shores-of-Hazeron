//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: GalaxyStar.f.glsl,v 1.1 2018/05/01 15:13:59 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Fragment shader for rendering a single unmodulated texture.
#version 150

uniform sampler2D						tex;					// Texture.

in vec2									alphaInt;				// Incoming alpha(x), intensity(y)

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	fragColor = texture(tex, gl_PointCoord) * vec4(alphaInt.y, alphaInt.y, alphaInt.y, alphaInt.x);
}
