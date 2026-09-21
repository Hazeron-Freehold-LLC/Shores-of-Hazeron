//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: SpriteSizeColor.v.glsl,v 1.1 2020/01/17 23:57:18 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform mat4							pmv;					// Projection model view matrix.

in vec4									position;				// Incoming vertex location(xyz) and point size(w).
in vec4									color;					// Incoming vertex color(rgba)

out vec4								spriteColor;

void main()
{
	gl_Position = pmv * vec4(position.xyz,1);				// Calculate position using projection and model view matrices.
	gl_PointSize = clamp(position.w/(.2*(gl_Position.z+3)), 0, position.w);
	spriteColor = color;
}
