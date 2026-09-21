//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: WarpTunnel.f.glsl,v 1.4 2015/08/01 00:08:17 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform vec4							color;					// Color used to modulate texture.
uniform samplerCube						texSky;					// Sky box texture.

in vec3									vertTexCoord;			// Incoming texture coordinate (xyz)
in vec4									vertColor;

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	fragColor = texture(texSky, vertTexCoord) * vertColor * color;
}
