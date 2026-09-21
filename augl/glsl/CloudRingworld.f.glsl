//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: CloudRingworld.f.glsl,v 1.6 2016/08/31 16:17:49 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform sampler2D						texWx;					// r=SaturationP, g=VaporP, b=instability
uniform vec2							offWx;					// horiz offset x=SaturationP, y=VaporP
uniform vec2							minWx;					// minimum x=SaturationP, y=VaporP
uniform vec2							maxWx;					// maximum x=SaturationP, y=VaporP

in vec4									vertColor;				// Incoming fragment color interpolated from the vertex color.
in vec2									vertTexCoord;			// Incoming texture coordinate.

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	fragColor = vertColor;

	// Calculate saturation pressure without banding as if flow east is 0 and flow west is 1.
	vec2 tcoord;
	tcoord.y = vertTexCoord.y;
	tcoord.x = fract(vertTexCoord.x - offWx.x + 1);
	vec3 colW = texture(texWx, tcoord).rgb;
	float sp = minWx.x + colW.r * (maxWx.x - minWx.x);
	if(sp <= 0)
		discard;

	// Calculate vapor pressure without banding.
	vec2 vpcoord;
	vpcoord.y = vertTexCoord.y;
	vpcoord.x = fract(vertTexCoord.x - offWx.y + 1);
	float vp = minWx.y + texture(texWx, vpcoord).g * (maxWx.y - minWx.y);
	vp *= 1 + (colW.b - .5) / 100.;
	if(vp <= sp)
		discard;

	fragColor.a = vp / sp - 1;
}
