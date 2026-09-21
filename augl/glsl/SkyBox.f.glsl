//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: SkyBox.f.glsl,v 1.3 2015/08/01 00:08:17 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Fragment shader for rendering a sky box that fades to black based on opacity.
#version 150

uniform float							opacity;				// Opacity of the sky dome.
uniform samplerCube						tex;					// Sky box texture.

in vec3									vertTexCoord;			// Incoming cube map texture coordinate.

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	fragColor = texture(tex, vertTexCoord) * vec4(opacity, opacity, opacity, 1);
}
