//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: Shield.v.glsl,v 1.5 2016/09/09 00:15:22 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform mat4							pmv;					// Projection model view matrix.
uniform vec3							scale;					// Scale for unit sphere model.
uniform vec2							texOffset;				// Texture coordinate offset(xy)

in vec3									position;				// Incoming vertex location.
in vec2									texCoord;				// Incoming texture coordinate(xy)

out vec3								vertPos;
out vec2								vertTexCoord;			// Outgoing texture coordinate(xy)

void main()
{
	vertPos = position * scale;
	gl_Position = pmv * vec4(vertPos,1);	// Calculate homogeneous vertex position using projection and model view matrices.
	vertTexCoord = texCoord + texOffset;		// Pass offset texture coordinate to the fragment shader.
}
