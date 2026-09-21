//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: FarGlobe.v.glsl,v 1.5 2016/09/09 00:10:02 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform float							atmosphereRadius;		// Radius of the atmosphere.
uniform mat4							pmv;					// Projection model view matrix.

in vec3									position;				// Incoming vertex location.
in vec2									texCoord;				// Incoming texture coordinate.

out vec2								vertTexCoord;			// Outgoing texture coordinate.

void main()
{
	vertTexCoord = texCoord;			// Pass unmodified texture coordinate to the fragment shader.

	// Calculate homogeneous vertex position using projection and model view matrices.
	// Position is a unit vector from the center of the globe; it must be multiplied by the radius for true positioning.
	gl_Position = pmv * vec4(position*atmosphereRadius, 1);
}
