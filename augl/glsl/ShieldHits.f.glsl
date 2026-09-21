//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ShieldHits.f.glsl,v 1.2 2018/07/10 22:38:54 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150
#define MaxShieldHitEffects				20

uniform vec4							color;					// Color used to modulate texture.
uniform sampler2D						tex;					// Texture.
uniform sampler2D						texScram;				// Texture.
uniform vec3							fbl;					// front, bottom, left - 0
uniform vec3							fblScram;				// front, bottom, left - 0
uniform vec3							btr;					// back, top, right    - 1
uniform vec3							btrScram;				// back, top, right    - 1
uniform float							hitDur[MaxShieldHitEffects];	// timer used for the hit effect
uniform vec4							hitPos[MaxShieldHitEffects];

in vec3									vertPos;
in vec2									vertTexCoord;			// Incoming texture coordinate (xy)

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	vec3 colorS = color.brg * 2;
	vec4 screenCol = texture(tex, vertTexCoord);
	vec4 scramCol = texture(texScram, vertTexCoord);

	// Normalize position and offset/scale into the range of 0..1.
	vec3 npos = normalize(vertPos);

	// Blend each side according to its intensity.
	vec3 c;
	if(npos.x < 0)
	{
		float f = -npos.x - .6;
		if(f <= 0)
			f = 0;
		else
			f /= (1-.6);
		if(fblScram.z > 0)
			c = mix(vec3(0,0,0), fbl.z * scramCol.rgb, f) * colorS;	// Blend left to center.
		else
			c = mix(vec3(0,0,0), fbl.z * screenCol.rgb, f) * color.rgb;	// Blend left to center.
	}
	else
	{
		float f = npos.x - .6;
		if(f <= 0)
			f = 0;
		else
			f /= (1-.6);
		if(btrScram.z > 0)
			c = mix(vec3(0,0,0), btr.z * scramCol.rgb, f) * colorS;	// Blend center to right.
		else
			c = mix(vec3(0,0,0), btr.z * screenCol.rgb, f) * color.rgb;	// Blend center to right.
	}
	if(npos.y < 0)
	{
		float f = -npos.y - .6;
		if(f <= 0)
			f = 0;
		else
			f /= (1-.6);
		if(btrScram.x > 0)
			c += mix(vec3(0,0,0), btr.x * scramCol.rgb, f) * colorS;	// Blend back to center.
		else
			c += mix(vec3(0,0,0), btr.x * screenCol.rgb, f) * color.rgb;	// Blend back to center.
	}
	else
	{
		float f = npos.y - .6;
		if(f <= 0)
			f = 0;
		else
			f /= (1-.6);
		if(fblScram.x > 0)
			c += mix(vec3(0,0,0), fbl.x * scramCol.rgb, f) * colorS;	// Blend center to front.
		else
			c += mix(vec3(0,0,0), fbl.x * screenCol.rgb, f) * color.rgb;	// Blend center to front.
	}
	if(npos.z < 0)
	{
		float f = -npos.z - .6;
		if(f <= 0)
			f = 0;
		else
			f /= (1-.6);
		if(fblScram.y > 0)
			c += mix(vec3(0,0,0), fbl.y * scramCol.rgb, f) * colorS;	// Blend bottom to center.
		else
			c += mix(vec3(0,0,0), fbl.y * screenCol.rgb, f) * color.rgb;	// Blend bottom to center.
	}
	else
	{
		float f = npos.z - .6;
		if(f <= 0)
			f = 0;
		else
			f /= (1-.6);
		if(btrScram.y > 0)
			c += mix(vec3(0,0,0), btr.y * scramCol.rgb, f) * colorS;	// Blend center to top.
		else
			c += mix(vec3(0,0,0), btr.y * screenCol.rgb, f) * color.rgb;	// Blend center to top.
	}
	fragColor.rgb = c;

	for(int i=0; i<MaxShieldHitEffects; i++)
	{
		float dist = distance(hitPos[i].xyz, vertPos);			// Get distance from hitPos.

		// Get all distances based off hitPos.w and put them in the range 2 - 0
		// hitPos.w is the gain associated with the hit.
		float colScale = ((hitPos[i].w - min(dist, hitPos[i].w))/hitPos[i].w)*2;
		if(colScale > 0)
		{
			// Amplify color based off distance.  Note that when the distance
			// is beyond the size set by hitPos.w, colScale becomes 0.
			fragColor.rgb += vec3(2*colScale, colScale, 0)*color.rgb*colScale*hitDur[i];
		}
	}

	fragColor.a = 1;
}
