//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: Color.v.glsl,v 1.5 2016/09/09 00:09:32 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Vertex shader passes a single unmodified color to the fragment shader.
#version 150

uniform mat4							pmv;					// Projection model view matrix.

in vec4									position;				// Incoming vertex location.
in vec4									color;					// Incoming vertex color.

out vec4								vertColor;				// Outgoing vertex color.

void main()
{
	vertColor = color;					// Pass unmodified vertex color to the fragment shader.
	gl_Position = pmv * position;		// Calculate homogeneous vertex position using projection and model view matrices.
}
