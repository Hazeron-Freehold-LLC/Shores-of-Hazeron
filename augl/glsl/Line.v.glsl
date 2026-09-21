//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: Line.v.glsl,v 1.4 2016/09/09 00:12:11 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Vertex shader only projects a vertex location.
#version 150

uniform mat4							pmv;					// Projection model view matrix.

in vec4									position;				// Incoming vertex location.

void main()
{
	gl_Position = pmv * position;	// Calculate homogeneous vertex position using projection and model view matrices.
}
