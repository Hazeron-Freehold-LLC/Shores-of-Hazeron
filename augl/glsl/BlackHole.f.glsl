//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: BlackHole.f.glsl,v 1.4 2015/08/01 00:08:17 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Fragment shader for rendering a single unmodulated texture.
#version 150

uniform mat4							rotSky;					// Rotation matrix to align with the skybox.
uniform sampler2D						tex;					// Texture.
uniform samplerCube						texSky;					// Sky box texture.

in vec2									vertTexCoord;			// Incoming texture coordinate.

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	fragColor = texture(tex, vertTexCoord);
	if(fragColor.a == 0.)
		discard;

	if(fragColor.r != 0. || fragColor.g != 0. || fragColor.b != 0)
	{
		vec3 skyBoxCoord = (fragColor.xyz - .5) * 2;
		skyBoxCoord = (rotSky * vec4(skyBoxCoord,1)).xyz;
		fragColor.rgb = texture(texSky, skyBoxCoord).rgb;
	}
}
