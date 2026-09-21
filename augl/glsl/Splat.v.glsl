//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: Splat.v.glsl,v 1.4 2016/09/09 00:15:56 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform mat4							pmv;					// Projection model view matrix.

in vec3									position;				// Incoming vertex location.
in vec2									texCoord;				// Incoming texture coordinate(xy)

out vec2								vertTexCoord;			// Outgoing texture coordinate(xy)

void main()
{
	gl_Position = pmv * vec4(position,1);	// Calculate homogeneous vertex position using projection and model view matrices.
	vertTexCoord = texCoord;			// Pass unmodified texture coordinate to the fragment shader.
}
