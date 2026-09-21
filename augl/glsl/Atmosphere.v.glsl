//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: Atmosphere.v.glsl,v 1.15 2016/09/09 14:59:15 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Lighting Structures and Functions

struct SunLight
{
	bool								enabled;        		// 1 if enabled
	float								attenuation;    		// light attenuation
	vec3								color;          		// color of light emitted
	vec3								position;       		// position of light source, center of sun in object space
	float								radius;         		// radius of sun
};

struct LightSources
{
	SunLight							sun0;
	SunLight							sun1;
	SunLight							sun2;
};

void computeSunLight(SunLight sun,
			vec3 P,
			vec3 N,
		out vec4 oDiffuse)
{
	// Compute the diffuse lighting from center of sun.
	vec3 toSun = sun.position - P;
	float distSun = length(toSun);
	vec3 L = toSun/distSun;
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

	// Compute final diffuse lighting result.
	float lighting = max(0.0, NdotL);
	oDiffuse = vec4(sun.color,1) * lighting * attenuation;
}

vec4 computeSolarLighting(LightSources light,
			vec3 P,
			vec3 N,
			vec4 materialColor,
			float materialAlpha)
{
	// Get a diffuse contribution from each sun that is enabled.
	vec4 diffuseLight = vec4(0,0,0,0);
	vec4 diffuseSum = vec4(0,0,0,0);

	// Suns
	{
		if(light.sun0.enabled)
		{
			// This sun light is enabled.
			computeSunLight(light.sun0, P, N, diffuseLight);
			diffuseSum += diffuseLight;
		}
		if(light.sun1.enabled)
		{
			// This sun light is enabled.
			computeSunLight(light.sun1, P, N, diffuseLight);
			diffuseSum += diffuseLight;
		}
		if(light.sun2.enabled)
		{
			// This sun light is enabled.
			computeSunLight(light.sun2, P, N, diffuseLight);
			diffuseSum += diffuseLight;
		}
	}

	// Modulate diffuse by material color.
	vec4 color = materialColor * diffuseSum;

	// Calculate final color of lit vertex.
	color.a += materialAlpha;

	// Clamp color values
	return clamp(color, vec4(0,0,0,0), vec4(1,1,1,1));
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

uniform float							atmAlpha;				// Atmosphere minimum alpha factor
uniform vec4							atmColor;				// Atmosphere color.
uniform float							atmRadius;				// Atmosphere radius.
uniform LightSources					light;					// Light sources.
uniform mat4							pmv;					// Projection model view matrix.

in vec3									position;				// Incoming vertex location.

out vec4								vertColor;				// Outgoing vertex color.

void main()
{
	// Calculate lighting using object space position and normal.
	// The object used with this shader is a globe of unit radius centered at 0,0. That means the incoming vertex locations are
	// also the normals at those vertexes. The atmosphereRadius enables us to calculate the actual size of this globe.
	vec3 posRad = position * atmRadius;
	vertColor = computeSolarLighting(light, posRad, position, atmColor, atmAlpha);

	// Calculate homogeneous vertex position using projection and model view matrices.
	gl_Position = pmv * vec4(posRad, 1);
}
