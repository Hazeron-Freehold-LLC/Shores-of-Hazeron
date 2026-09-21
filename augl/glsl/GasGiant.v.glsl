//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: GasGiant.v.glsl,v 1.11 2018/04/26 03:01:23 daveh Exp $
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
	vec3								position;       		// position of light source, center of sun in object space
	float								radius;         		// radius of sun
};

struct LightSources
{
	float								ambient;
	vec3								eyeLocation;
	PointLight							light[MAX_LIGHTS];
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

void computePointLight(PointLight light,
			vec3 P,
			vec3 N,
		out vec3 oDiffuse)
{
	// Compute the diffuse lighting from the point light source.
	vec3 toLight = light.position - P;
	float distLight = length(toLight);
	vec3 L = toLight/distLight;									// Unit vector from P pointing toward the light.
	float NdotL = dot(N, L);

	// Compute attenuation.
	float attenuation = (1 / (light.kL*distLight + light.kQ*pow(distLight,2)));

	// Compute final diffuse lighting results.
	float lighting = max(0.0, NdotL);
	oDiffuse = light.color * lighting * attenuation;
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

	// Point lights
	for(int l=0; l<MAX_LIGHTS; l++)
	{
		if(light.light[l].enabled)
		{
			// This other light is enabled.
			// Compute general lighting contribution.
			computePointLight(light.light[l], P, N, diffuseLight);

			// Compute spotlight effect.
			if(light.light[l].cosOuterCone > -1 || light.light[l].cosInnerCone > -1)
			{
				vec3 Vs = normalize(P - light.light[l].position);
				float cosDirection = dot(Vs, light.light[l].direction);
				float spotEffect = smoothstep(light.light[l].cosOuterCone, light.light[l].cosInnerCone, cosDirection);
				diffuseLight *= spotEffect;
			}

			diffuseSum += diffuseLight;
		}
	}

	// Calculate final color of lit vertex.
	vec3 color = vec3(light.ambient,light.ambient,light.ambient) + diffuseSum + specularSum;

	// Clamp color values
	return clamp(color, vec3(0,0,0), vec3(1,1,1));
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

uniform vec4							atmColor;				// Atmosphere color modulates texture color.
uniform mat4							pmv;					// Projection model view matrix.
uniform LightSources					light;					// Light sources.

in vec3									position;				// Incoming vertex location.
in vec3									normal;					// Incoming vertex normal.
in vec3									texCoordShine;			// Incoming texture coordinate (xy) and shininess (z)

out vec4								vertColor;				// Outgoing vertex color.
out vec2								vertTexCoord;			// Outgoing texture coordinate.

void main()
{
	// Calculate lighting using object space position and normal.
	vertColor = vec4(computeSolarLighting(light, texCoordShine.z, position, normal), 1) * atmColor;

	// Calculate homogeneous vertex position using projection and model view matrices.
	// Position is a unit vector from the center of the atmosphere sphere; it must be multiplied by the radius for true positioning.
	gl_Position = pmv * vec4(position,1);

	vertTexCoord = texCoordShine.xy;			// Pass unmodified texture coordinate to the fragment shader.
}
