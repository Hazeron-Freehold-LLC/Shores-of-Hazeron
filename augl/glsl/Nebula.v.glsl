//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: Nebula.v.glsl,v 1.4 2016/09/09 00:13:44 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Vertex shader for rendering a texture.
#version 150

uniform mat4							pmv;					// Projection model view matrix.

in vec3									color;					// Incoming vertex color.
in vec3									position;				// Incoming vertex location.
in vec2									texCoord;				// Incoming texture coordinate.

out vec3								vertColor;				// Outgoing vertex color.
out vec2								vertTexCoord;			// Outgoing texture coordinate.

void main()
{
	vertColor = color;
	vertTexCoord = texCoord;			// Pass unmodified texture coordinate to the fragment shader.
	gl_Position = pmv * vec4(position,1);	// Calculate homogeneous vertex position using projection and model view matrices.
}
