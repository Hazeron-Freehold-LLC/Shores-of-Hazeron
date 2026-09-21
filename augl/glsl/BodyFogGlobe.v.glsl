//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: BodyFogGlobe.v.glsl,v 1.14 2018/04/26 03:00:52 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Lighting Structures and Functions

// Defines a point light that shines in all direction and spot lights that are
// confined to a direction and cone of dispersion.
struct PointLight
{
	bool								enabled;			// 1 if enabled
	float								kL;					// Linear light attenuation
	float								kQ;					// Quadratic light attenuation
	vec3								color;				// color of light emitted
	vec3								position;			// position of light source

	// Spot light values define a cone of dispersion for the light. If not a spot light, the light is emitted in all directions.
	float								cosOuterCone;		// outer light cone for spot lights, -1 = full, no cone
	float								cosInnerCone;		// inner light cone for spot lights
	vec3								direction;			// direction to point spot light
};

struct SunLight
{
	bool								enabled;        		// 1 if enabled
	float								attenuation;    		// light attenuation
	vec3								color;          		// color of light emitted
	vec3								position;       		// position of light source, center of sun in eye space
	float								radius;         		// radius of sun
};

struct LightSources
{
	vec3								ambient;				// Color/intensity of ambient lighting.
	vec3								ambientUp;				// Up vector in eye space for spherical ambient lighting calc.
	vec3								globeP;					// Globe position in eye space for sun over-the-horizon calc.
	PointLight							light[MAX_LIGHTS];
	SunLight							sun0;
	SunLight							sun1;
	SunLight							sun2;
};

void computeSunLight(SunLight sun,
			float shininess,
			vec3 P,
			vec3 N,
			vec3 globeP,				// position of globe
		out vec3 oDiffuse,
		out vec3 oSpecular)
{
	// Compute the diffuse lighting from center of sun.
	vec3 toSun = sun.position - P;
	float distSun = length(toSun);
	vec3 L = toSun/distSun;										// Unit vector from P pointing toward the light.
	float NdotL = dot(N, L);

	// Compute attenuation.
	float attenuation = 1 / (sun.attenuation * pow(distSun,2));

	// Account for radius of solar disk.
	float diskang = atan(sun.radius, distSun);
	float sunang = acos(NdotL);
	float rd1 = cos(sunang + diskang);
	float rd2 = cos(sunang - diskang);
	NdotL = max(rd1, NdotL);
	NdotL = max(rd2, NdotL);

	// Check for sun over the horizon.
	vec3 U = normalize(P - globeP);								// Up vector at vertex location, with respect to the globe.
	float LdotU = dot(L, U);
	float sunang2 = acos(LdotU);
	float rd3 = cos(sunang2 + diskang);
	float rd4 = cos(sunang2 - diskang);
	LdotU = max(rd3, LdotU);
	LdotU = max(rd4, LdotU);
	NdotL *= max(0, LdotU);

	// Compute specular lighting terms.
	vec3 V = normalize(-P);										// Vector to the eye from P. We are lighting in eye-space.
	vec3 H = normalize(L + V);									// Half-vector for specular lighting.

	// Compute final diffuse lighting result.
	float lighting = max(0.0, NdotL);
	float specular = (NdotL > 0. && shininess >= 1) ? pow(max(0., dot(N,H)), shininess) : 0.;
	oDiffuse = sun.color * lighting * attenuation;
	oSpecular = sun.color * specular * attenuation;
}

void computePointLight(PointLight light,
			float shininess,
			vec3 P,
			vec3 N,
		out vec3 oDiffuse,
		out vec3 oSpecular)
{
	// Compute the diffuse lighting from the point light source.
	vec3 toLight = light.position - P;
	float distLight = length(toLight);
	vec3 L = toLight/distLight;									// Unit vector from P pointing toward the light.
	float NdotL = dot(N, L);

	// Compute attenuation.
	float attenuation = (1 / (light.kL*distLight + light.kQ*pow(distLight,2)));

	// Compute specular lighting terms.
	vec3 V = normalize(-P);										// Vector to the eye from P. We are lighting in eye-space.
	vec3 H = normalize(L + V);									// Half-vector for specular lighting.

	// Compute final diffuse lighting results.
	float lighting = max(0.0, NdotL);
	float specular = (NdotL > 0. && shininess >= 1) ? pow(max(0., dot(N,H)), shininess) : 0.;
	oDiffuse = light.color * lighting * attenuation;
	oSpecular = light.color * specular * attenuation;
}

vec3 computeLighting(LightSources light,
			float shininess,
			vec3 P,
			vec3 N)
{
	// Get a diffuse contribution from each sun that is enabled.
	vec3 diffuseLight = vec3(0,0,0);
	vec3 diffuseSum = vec3(0,0,0);
	vec3 specularLight = vec3(0,0,0);
	vec3 specularSum = vec3(0,0,0);

	// Suns
	{
		if(light.sun0.enabled)
		{
			// This sun light is enabled.
			computeSunLight(light.sun0, shininess, P, N, light.globeP, diffuseLight, specularLight);
			diffuseSum += diffuseLight;
			specularSum += specularLight;
		}
		if(light.sun1.enabled)
		{
			// This sun light is enabled.
			computeSunLight(light.sun1, shininess, P, N, light.globeP, diffuseLight, specularLight);
			diffuseSum += diffuseLight;
			specularSum += specularLight;
		}
		if(light.sun2.enabled)
		{
			// This sun light is enabled.
			computeSunLight(light.sun2, shininess, P, N, light.globeP, diffuseLight, specularLight);
			diffuseSum += diffuseLight;
			specularSum += specularLight;
		}
	}

	// Point lights
	for(int l=0; l<MAX_LIGHTS; l++)
	{
		if(light.light[l].enabled)
		{
			// This other light is enabled.
			// Compute general lighting contribution.
			computePointLight(light.light[l], shininess, P, N, diffuseLight, specularLight);

			// Compute spotlight effect.
			if(light.light[l].cosOuterCone > -1 || light.light[l].cosInnerCone > -1)
			{
				vec3 Vs = normalize(P - light.light[l].position);
				float cosDirection = dot(Vs, light.light[l].direction);
				float spotEffect = smoothstep(light.light[l].cosOuterCone, light.light[l].cosInnerCone, cosDirection);
				diffuseLight *= spotEffect;
				specularLight *= spotEffect;
			}

			diffuseSum += diffuseLight;
			specularSum += specularLight;
		}
	}

	// Calculate spherical ambient lighting that drops off to zero.
	//vec3 ambient = light.ambient * (dot(N, light.ambientUp) + 1)/2;

	// Calculate spherical ambient lighting that drops off to 20% the ambient light value.
	vec3 ambient = mix(light.ambient * .15, light.ambient, (dot(N, light.ambientUp) + 1.)/2.);

	// Calculate final color of lit vertex.
	vec3 color = ambient + diffuseSum + specularSum;

	// Clamp color values
	return clamp(color, vec3(0,0,0), vec3(1,1,1));
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#define MaxBones 61	// NOTE: Too many bone matrices and nothing dislays on the screen.

uniform float							fogDensity;				// Fog density for exponential fog calculation.
uniform vec4							glowAlpha;				// Glow color added to lit color(rgb). Alpha of vertColor(a).
uniform LightSources					light;					// Light sources.
uniform mat4							mv;						// Model view matrix.
uniform mat4							p;						// Projection matrix.
uniform mat4							bM[MaxBones];			// 4x4 bone matrices

in vec4									boneIndex;
in vec4									boneWeight;
in vec3									normal;					// Incoming vertex normal.
in vec3									position;				// Incoming vertex location.
in vec4									texCoordShineTopface;	// Incoming texture coordinate(xy), shininess(z), top face(w)

out vec4								vertColor;				// Outgoing vertex color.
out vec4								vertTexCoordFogTopface;	// Outgoing texture coordinate(xy) and fog density (z), top face(w)

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
			vec4 bonePosition = bM[index] * vec4(position,1);

			// Assumes no scaling in matrix, just rotation and translation.
			mat3 rotate = mat3(bM[index]);
			vec3 boneNormal = rotate * normal;

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
	vertColor = vec4(computeLighting(light, texCoordShineTopface.z, gl_Position.xyz, eyeN.xyz), glowAlpha.a);
	vertColor.rgb = min(vertColor.rgb + glowAlpha.rgb, vec3(1.5, 1.5, 1.5));

	gl_Position = p * gl_Position;

	vertTexCoordFogTopface.xy = texCoordShineTopface.xy;		// Pass unmodified texture coordinate to the fragment shader.

	// Calculate fog density but save the effect for the fragment program.
	// Fog density value is 1 for full fog, 0 for no fog.
	float d = max(gl_Position.z, 0) * fogDensity;
	float f = 1 / exp(d*d);
	vertTexCoordFogTopface.z = 1 - clamp(f, 0, 1);

	vertTexCoordFogTopface.w = texCoordShineTopface.w;			// Pass unmodified top face value to fragment shader.
}
