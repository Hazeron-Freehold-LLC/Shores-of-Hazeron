//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ScGlobe.f.glsl,v 1.3 2019/02/13 17:15:56 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform float							effectApply;			// How to apply the effect texture: 1=lit by env, 2=unlit(glows)
uniform vec4							glowDiscard;			// Glow(rgb), Alpha below which we discard fragments(a).
uniform sampler2D						texEffect;				// Texture for effects applied to the hull.
uniform sampler2D						texSurface[15];			// Textures for surfaces.

in vec4									vertColor;				// Incoming fragment color interpolated from the vertex color.
in vec3									vertEffCoord;			// Incoming effect texture coordinate
in vec4									vertPaint;				// Color applied to the surface. Alpha is blend amount.
in vec2									vertTexCoord;			// Incoming texture coordinate (xy)
flat in vec2							vertTexIndex;			// Surface texture index(x), effect(y) 0=no, 1=yes

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	if(vertTexIndex.x == 0)
		fragColor = texture(texSurface[0], vertTexCoord);
	else if(vertTexIndex.x == 1)
		fragColor = texture(texSurface[1], vertTexCoord);
	else if(vertTexIndex.x == 2)
		fragColor = texture(texSurface[2], vertTexCoord);
	else if(vertTexIndex.x == 3)
		fragColor = texture(texSurface[3], vertTexCoord);
	else if(vertTexIndex.x == 4)
		fragColor = texture(texSurface[4], vertTexCoord);
	else if(vertTexIndex.x == 5)
		fragColor = texture(texSurface[5], vertTexCoord);
	else if(vertTexIndex.x == 6)
		fragColor = texture(texSurface[6], vertTexCoord);
	else if(vertTexIndex.x == 7)
		fragColor = texture(texSurface[7], vertTexCoord);
	else if(vertTexIndex.x == 8)
		fragColor = texture(texSurface[8], vertTexCoord);
	else if(vertTexIndex.x == 9)
		fragColor = texture(texSurface[9], vertTexCoord);
	else if(vertTexIndex.x == 10)
		fragColor = texture(texSurface[10], vertTexCoord);
	else if(vertTexIndex.x == 11)
		fragColor = texture(texSurface[11], vertTexCoord);
	else if(vertTexIndex.x == 12)
		fragColor = texture(texSurface[12], vertTexCoord);
	else if(vertTexIndex.x == 13)
		fragColor = texture(texSurface[13], vertTexCoord);
	else
		fragColor = texture(texSurface[14], vertTexCoord);

	if(fragColor.a <= glowDiscard.a)
		discard;

	// Apply paint.
	fragColor.rgb = mix(fragColor.rgb, vertPaint.rgb, vertPaint.a);

	// Modulate with lighting and effects.
	float appl = vertTexIndex.y*effectApply;
	if(appl == 1)
	{
		// Blend in the "lit" effect texture color before modulating with light.
		vec4 litEffect = texture(texEffect, vertEffCoord.xy);	// Effect texture color.
		fragColor.rgb = mix(fragColor.rgb, litEffect.rgb, litEffect.a);

		// Modulate using light-modulated color.
		fragColor *= vertColor;

		// Glow
		fragColor.rgb = min(fragColor.rgb + glowDiscard.rgb, vec3(1., 1., 1.));
	}
	else if(appl == 2)
	{
		// Modulate using light-modulated color.
		fragColor *= vertColor;

		// Glow
		fragColor.rgb = min(fragColor.rgb + glowDiscard.rgb, vec3(1., 1., 1.));

		// Blend in the "unlit" effect texture color after light modulation applied.
		vec4 unlitEffect = texture(texEffect, vertEffCoord.xy);	// Effect texture color.
		unlitEffect *= vertEffCoord.z;							// Modulate intensity of glowing effect texture.
		fragColor.rgb = mix(fragColor.rgb, unlitEffect.rgb, unlitEffect.a);

		// Bring up the alpha to make the effect bright even on transparent surfaces.
		fragColor.a = max(unlitEffect.a, fragColor.a);
	}
	else
	{
		// Modulate using light-modulated color.
		fragColor *= vertColor;

		// Glow
		fragColor.rgb = min(fragColor.rgb + glowDiscard.rgb, vec3(1., 1., 1.));
	}
}
