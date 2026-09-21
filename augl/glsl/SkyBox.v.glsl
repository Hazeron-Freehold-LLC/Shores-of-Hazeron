//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: SkyBox.v.glsl,v 1.3 2015/08/01 00:08:17 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Vertex shader for rendering a sky box.
#version 150

uniform mat4							mv;						// Model view matrix.
uniform mat4							p;						// Projection matrix.

in vec3									position;				// Incoming vertex location.

out vec3								vertTexCoord;			// Outgoing cube map texture coordinate.

void main()
{
	vertTexCoord = position;
	gl_Position = p * mv * vec4(position,1);	// Calculate homogeneous vertex position using projection and model view matrices.
}
