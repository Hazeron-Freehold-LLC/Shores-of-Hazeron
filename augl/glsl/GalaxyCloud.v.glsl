//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: GalaxyCloud.v.glsl,v 1.5 2018/05/01 15:10:50 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Vertex shader for rendering a texture.
#version 150

uniform mat4							p;						// Projection matrix.
uniform mat4							mv;						// Model view matrix.

in vec3									position;				// Incoming vertex location.
in vec2									texCoord;				// Incoming texture coordinate.

out vec3								vertTexCoordAlpha;		// Outgoing texture coordinate(xy) and alpha(z).

void main()
{
	gl_Position = mv * vec4(position,1);	// Calculate homogeneous vertex position using projection and model view matrices.
	vertTexCoordAlpha.xy = texCoord;		// Pass unmodified texture coordinate to the fragment shader.
	float dist = length(position);			// Distance in sectors.
	vertTexCoordAlpha.z = 1 - clamp(dist / 10, 0, .875);	// Get more transparent with distance.
	gl_Position = p * gl_Position;
}
