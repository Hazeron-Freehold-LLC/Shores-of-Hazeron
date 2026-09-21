//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: NearStar.v.glsl,v 1.4 2016/09/09 00:13:37 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Vertex shader for rendering a texture.
#version 150

uniform mat4							pmv;					// Projection model view matrix.

in vec4									position;				// Incoming vertex location. w is the point size in pixels.

void main()
{
	gl_Position = pmv * vec4(position.xyz,1);	// Calc homogeneous vertex position using projection and model view matrices.
	gl_PointSize = position.w;			// Pass unmodified point size to OpenGL.
}
