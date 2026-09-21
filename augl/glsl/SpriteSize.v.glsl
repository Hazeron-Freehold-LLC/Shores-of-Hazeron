//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: SpriteSize.v.glsl,v 1.7 2016/09/09 00:03:11 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform mat4							pmv;					// Projectio model view matrix.
uniform float							size;

in vec4									position;				// Incoming vertex location(xyz) and point alpha(w).

out float								fragAlpha;				// Outgoing fragment alpha.
void main()
{
	gl_Position = pmv * vec4(position.xyz,1);					// Calculate position using projection model view matrix.
	gl_PointSize = clamp(size/(.2*(gl_Position.z+3)), 0, size);
	fragAlpha = position.w;
}
