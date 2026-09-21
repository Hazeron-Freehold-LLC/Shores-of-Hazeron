//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: NearGlobeLava.f.glsl,v 1.4 2015/08/01 00:08:17 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Fragment shader for rendering a single texture modulated using a color.
#version 150

uniform sampler2D						tex;					// Texture.

in vec4									vertColor;				// Incoming fragment color interpolated from the vertex color.
in vec2									vertTexCoord;			// Incoming texture coordinate.

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	fragColor = texture(tex, vertTexCoord);
	float glow = 1-fragColor.a;									// 1-alpha in the near globe texture is a glow factor.
	vec3 glowColor = vertColor.rgb + vec3(glow,glow,glow);		// Brighten incoming vertex color.
	glowColor = min(glowColor, vec3(1,1,1));
	fragColor.rgb *= glowColor;
	fragColor.a = 1;
}
