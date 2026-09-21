//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: GalaxyCloud.f.glsl,v 1.5 2018/05/01 15:08:44 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Fragment shader for rendering a single texture modulated with the background galaxy color.
#version 150

uniform sampler2D						tex;					// Texture.
uniform vec3							color;					// Base color of the galaxy.

in vec3									vertTexCoordAlpha;		// Incoming texture coordinate(xy), alpha(z).

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	fragColor = texture(tex, vertTexCoordAlpha.xy) * vec4(color, vertTexCoordAlpha.z);
}
