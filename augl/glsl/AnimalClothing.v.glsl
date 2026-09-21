//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: AnimalClothing.v.glsl,v 1.10 2019/10/02 14:54:10 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

#SunLight								// Suns, point light sources.

uniform vec4							glowAlpha;				// Glow color added to lit color(rgb). Alpha of vertColor(a).
uniform LightSources					light;					// Light sources.
uniform mat4							mv;						// Model view matrix.
uniform mat4							p;						// Projection matrix.
uniform mat4							bM[MAX_ANIMAL_BONES];	// 4x4 bone matrices

in vec4									positionShine;			// Incoming vertex location (xyz) and shininess (w).
in vec4									normalReflect;			// Incoming vertex normal (xyz) and reflectiveness (w).
in vec4									boneIndex;
in vec4									boneWeight;

in vec3									texCoordClothing;		// Incoming texture coordinate (xy), texture index (z)
in vec3									texCoordDecalBlendIn;	// Decal tex coord (xy), decal blend (z)
in vec4									dyeColorAlpha;			// Dye color modulates color of this vertex (rgb) and (a) is alpha.

out vec4								vertColor;				// Outgoing vertex color.
out vec3								dyeColor;
out vec2								texCoord;				// Outgoing texture coordinate (xy)
out vec3								texCoordDecalBlend;		// Decal tex coord (xy), decal blend (w)
flat out int							texClothingIndex;		// Index of clothing texture applied to this fragment.

void main()
{
	// Project this vertex through the applicable bone matrices.
	vec4 netPosition = vec4(0,0,0,0);
	vec3 netNormal = vec3(0,0,0);
	for(int ib=0; ib<4; ib++)
	{
		if(boneWeight[ib] != 0)
		{
			// Bone weight is not zero.
			// We allow for zero weight because most vertices are affected by only one bone.
			// NOTE: The vertex must reference at least one bone.
			// NOTE: The sum of the bone weights at this vertex must add up to 1.
			int index = int(boneIndex[ib]);
			vec4 bonePosition = bM[index] * vec4(positionShine.xyz,1);

			// Assumes no scaling in matrix, just rotation and translation.
			mat3 rotate = mat3(bM[index]);
			vec3 boneNormal = rotate * normalReflect.xyz;

			// net values are weighted according to bone weights. This calculation
			// assumes all boneWeights at this vertex add up to 1.
			netPosition += boneWeight[ib] * bonePosition;
			netNormal += boneWeight[ib] * boneNormal;
		}
	}

	// Normal is likely to need normalizing after bone projections.
	netNormal = normalize(netNormal);

	// Calculate vertex position in eye space.
	gl_Position = mv * netPosition;

	// Calculate lighting using eye space position and normal.
	vec4 eyeN = normalize(mv * vec4(netNormal, 1) - mv * vec4(0,0,0,1));	// Surface normal in eye space.
	vertColor = vec4(computeLighting(light, positionShine.w, normalReflect.w, gl_Position.xyz, eyeN.xyz),
			glowAlpha.a * dyeColorAlpha.a);

	// Pass the unmodified dye color to the fragment shader.
	dyeColor = dyeColorAlpha.rgb;

	// Make it glow and clamp the final result.
	vertColor.rgb = min(vertColor.rgb + glowAlpha.rgb, vec3(1.5, 1.5, 1.5));

	gl_Position = p * gl_Position;

	// Pass unmodified texture coordinates and clothing texture index to fragment shader.
	texCoord = texCoordClothing.xy;	
	texCoordDecalBlend = texCoordDecalBlendIn;
	texClothingIndex = int(texCoordClothing.z);
}
