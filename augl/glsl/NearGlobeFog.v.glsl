//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: NearGlobeFog.v.glsl,v 1.15 2018/04/26 03:01:57 daveh Exp $
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
	float								ambient;
	vec3								eyeLocation;
	SunLight							sun0;
	SunLight							sun1;
	SunLight							sun2;
};

void computeSunLight(SunLight sun,
			vec3 eyeP,
			float shininess,
			vec3 P,
			vec3 N,
		out vec3 oDiffuse,
		out vec3 oSpecular)
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

	// Check for sun over the horizon.
	vec3 U = normalize(P);	// Up vector at vertex location, with respect to the globe.
	float LdotU = dot(L, U);
	float sunang2 = acos(LdotU);
	float rd3 = cos(sunang2 + diskang);
	float rd4 = cos(sunang2 - diskang);
	LdotU = max(rd3, LdotU);
	LdotU = max(rd4, LdotU);
	NdotL *= max(0, LdotU);

	vec3 V = normalize(eyeP - P);
	// Compute final diffuse and specular lighting results.
	vec3 H = normalize(L + V);	// Half-vector for specular lighting.

	// Compute final diffuse lighting result.
	float lighting = max(0.0, NdotL);
	float specular = (NdotL > 0. && shininess >= 1) ? pow(max(0., dot(N,H)), shininess) : 0.;
	oDiffuse = sun.color * lighting * attenuation;
	oSpecular = sun.color * specular * attenuation;
}

vec3 computeSolarLighting(LightSources light,
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
			computeSunLight(light.sun0, light.eyeLocation, shininess, P, N, diffuseLight, specularLight);
			diffuseSum += diffuseLight;
			specularSum += specularLight;
		}
		if(light.sun1.enabled)
		{
			// This sun light is enabled.
			computeSunLight(light.sun1, light.eyeLocation, shininess, P, N, diffuseLight, specularLight);
			diffuseSum += diffuseLight;
			specularSum += specularLight;
		}
		if(light.sun2.enabled)
		{
			// This sun light is enabled.
			computeSunLight(light.sun2, light.eyeLocation, shininess, P, N, diffuseLight, specularLight);
			diffuseSum += diffuseLight;
			specularSum += specularLight;
		}
	}

	// Calculate final color of lit vertex.
	vec3 color = vec3(light.ambient,light.ambient,light.ambient) + diffuseSum + specularSum;

	// Clamp color values
	return clamp(color, vec3(0,0,0), vec3(1,1,1));
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

uniform float							fogDensity;				// Fog density for exponential fog calculation.
uniform mat4							pmv;					// Projection model view matrix.
uniform LightSources					light;					// Light sources.

in vec3									position;				// Incoming vertex location.
in vec3									normal;					// Incoming vertex normal.
in vec3									texCoordShine;			// Incoming texture coordinate (xy) and shininess (z)

out vec4								vertColor;				// Outgoing vertex color.
out vec3								vertTexCoordFog;		// Outgoing texture coordinate (xy) and fog density (z).

void main()
{
	// Calculate lighting using object space position and normal.
	vertColor = vec4(computeSolarLighting(light, texCoordShine.z, position, normal), 1);

	vertTexCoordFog.xy = texCoordShine.xy;						// Pass unmodified texture coordinate to the fragment shader.
	gl_Position = pmv * vec4(position,1);					// Calculate position using projection and model view matrices.

	// Calculate fog density but save the effect for the fragment program.
	// Fog density value is 1 for full fog, 0 for no fog.
	float d = max(gl_Position.z, 0) * fogDensity;
	float f = 1 / exp(d*d);
	vertTexCoordFog.z = 1 - clamp(f, 0, 1);
}
