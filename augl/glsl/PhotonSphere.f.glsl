//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: PhotonSphere.f.glsl,v 1.5 2015/08/01 00:08:17 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

// Fragment shader for rendering a single unmodulated texture.
#version 150

uniform sampler2D						tex;					// Texture.

in vec2									vertTexCoord;			// Incoming texture coordinate.

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	fragColor = texture(tex, vertTexCoord);
	fragColor *= vec4(8./255., 0, 230./255., .1);				// Modulate to deep purple color to match intergalactic wormholes.
}
