//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: WaterNoBumpNoHole.f.glsl,v 1.1 2016/10/17 21:05:26 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 140

uniform vec3							fogColor;				// Fog color
uniform float							opacity;				// Opacity of water
uniform sampler2D						tex;					// Texture.

in vec4									vertColor;				// Incoming fragment color interpolated from the vertex color.
in vec3									vertTexCoordFog;		// Incoming texture coordinate (xy) and fog density (z).
in vec4									vertTexCoord2;			// Incoming texture coordinate.
in vec3									vertUp;					// Vertex 'up' vector in globe space.

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	fragColor = vec4((texture(tex, vertTexCoordFog.xy).rgb +
				texture(tex, vertTexCoord2.xy).rgb +
				texture(tex, vertTexCoord2.zw).rgb) / 6, opacity) * vertColor;

	// Calculate fog effect on fragment color that increases alpha with fog density.
	// Fog density value is 1 for full fog, 0 for no fog.
	fragColor = mix(fragColor, vec4(fogColor,1), vertTexCoordFog.z);
}
