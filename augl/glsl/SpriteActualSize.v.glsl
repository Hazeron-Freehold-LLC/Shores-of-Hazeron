//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: SpriteActualSize.v.glsl,v 1.5 2016/09/09 00:16:01 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform mat4							pmv;					// Projection model view matrix.

in vec4									position;				// Incoming vertex location(xyz) and point size(w).

void main()
{
	gl_Position = pmv * vec4(position.xyz,1);				// Calculate position using projection and model view matrices.
	gl_PointSize = position.w;									// Set point size to passed in value for this vertex
}
