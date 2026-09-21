//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ScRingworldFog.f.glsl,v 1.3 2019/02/13 17:16:04 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform float							effectApply;			// How to apply the effect texture: 1=lit by env, 2=unlit(glows)
uniform vec3							fogColor;				// Fog color
uniform vec4							glowDiscard;			// Glow(rgb), Alpha below which we discard fragments(a).
uniform sampler2D						texEffect;				// Texture for effects applied to the hull.
uniform sampler2D						texSurface[15];			// Textures for surfaces.

in vec4									vertColor;				// Incoming fragment color interpolated from the vertex color.
in vec3									vertEffCoord;			// Incoming effect texture coordinate
in vec4									vertPaint;				// Color applied to the surface. Alpha is blend amount.
in vec3									vertTexCoordFog;		// Incoming texture coordinate (xy) and fog density (z)
flat in vec2							vertTexIndex;			// Surface texture index(x), effect(y) 0=no, 1=yes

out vec4								fragColor;				// Outgoing fragment color.

void main()
{
	if(vertTexIndex.x == 0)
		fragColor = texture(texSurface[0], vertTexCoordFog.xy);
	else if(vertTexIndex.x == 1)
		fragColor = texture(texSurface[1], vertTexCoordFog.xy);
	else if(vertTexIndex.x == 2)
		fragColor = texture(texSurface[2], vertTexCoordFog.xy);
	else if(vertTexIndex.x == 3)
		fragColor = texture(texSurface[3], vertTexCoordFog.xy);
	else if(vertTexIndex.x == 4)
		fragColor = texture(texSurface[4], vertTexCoordFog.xy);
	else if(vertTexIndex.x == 5)
		fragColor = texture(texSurface[5], vertTexCoordFog.xy);
	else if(vertTexIndex.x == 6)
		fragColor = texture(texSurface[6], vertTexCoordFog.xy);
	else if(vertTexIndex.x == 7)
		fragColor = texture(texSurface[7], vertTexCoordFog.xy);
	else if(vertTexIndex.x == 8)
		fragColor = texture(texSurface[8], vertTexCoordFog.xy);
	else if(vertTexIndex.x == 9)
		fragColor = texture(texSurface[9], vertTexCoordFog.xy);
	else if(vertTexIndex.x == 10)
		fragColor = texture(texSurface[10], vertTexCoordFog.xy);
	else if(vertTexIndex.x == 11)
		fragColor = texture(texSurface[11], vertTexCoordFog.xy);
	else if(vertTexIndex.x == 12)
		fragColor = texture(texSurface[12], vertTexCoordFog.xy);
	else if(vertTexIndex.x == 13)
		fragColor = texture(texSurface[13], vertTexCoordFog.xy);
	else
		fragColor = texture(texSurface[14], vertTexCoordFog.xy);

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

	// Calculate fog effect on fragment color but do not modify the alpha using fog.
	// Fog density value is 1 for full fog, 0 for no fog.
	fragColor.rgb = mix(fragColor.rgb, fogColor, vertTexCoordFog.z);
}
