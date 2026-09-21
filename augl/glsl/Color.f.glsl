//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: Color.f.glsl,v 1.3 2015/08/01 00:08:17 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Fragment shader assigns a single unmodified color to the fragment.
#version 150

in vec4									vertColor;				// Incoming fragment color interpolated from the vertex color.

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	fragColor = vertColor;
}
