//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: TexBar.f.glsl,v 1.3 2015/08/01 00:08:17 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform vec4							colorLeft;				// Color used to modulate texture at the left of the threshold.
uniform vec4							colorRight;				// Color used to modulate texture at the right of the threshold.
uniform sampler2D						tex;					// Texture.
uniform vec2							texOffset;				// Offset applied to incoming texture coordinate.
uniform float							threshold;				// Threshold texture coordinate(x) at which color changes.

in vec2									vertTexCoord;			// Incoming texture coordinate (xy)

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	fragColor = texture(tex, vertTexCoord + texOffset);
	if(vertTexCoord.x < threshold)
		fragColor *= colorLeft;
	else
		fragColor *= colorRight;
}
