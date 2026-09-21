//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: Flora.v.glsl,v 1.20 2017/12/12 00:11:18 daveh Exp $
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
			vec3 P,
			vec3 N,
			vec3 globeP,				// position of globe
		out vec3 oDiffuse)
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

	// Compute final diffuse lighting result.
	float lighting = max(0.0, NdotL);
	oDiffuse = sun.color * lighting * attenuation;
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

vec3 computeLighting(LightSources light,
			vec3 P,
			vec3 N)
{
	// Get a diffuse contribution from each sun that is enabled.
	vec3 diffuseLight = vec3(0,0,0);
	vec3 diffuseSum = vec3(0,0,0);

	// Suns
	{
		if(light.sun0.enabled)
		{
			// This sun light is enabled.
			computeSunLight(light.sun0, P, N, light.globeP, diffuseLight);
			diffuseSum += diffuseLight;
		}
		if(light.sun1.enabled)
		{
			// This sun light is enabled.
			computeSunLight(light.sun1, P, N, light.globeP, diffuseLight);
			diffuseSum += diffuseLight;
		}
		if(light.sun2.enabled)
		{
			// This sun light is enabled.
			computeSunLight(light.sun2, P, N, light.globeP, diffuseLight);
			diffuseSum += diffuseLight;
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

	// Calculate spherical ambient lighting that drops off to zero.
	//vec3 ambient = light.ambient * (dot(N, light.ambientUp) + 1)/2;

	// Calculate spherical ambient lighting that drops off to 20% the ambient light value.
	vec3 ambient = mix(light.ambient * .15, light.ambient, (dot(N, light.ambientUp) + 1.)/2.);

	// Calculate final color of lit vertex.
	vec3 color = ambient + diffuseSum;

	// Clamp color values
	return clamp(color, vec3(0,0,0), vec3(1,1,1));
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

uniform float							fogDensity;				// Fog density for exponential fog calculation.
uniform vec4							glowAlpha;				// Glow color due to plant radioactivity (xyz), alpha (z)
uniform LightSources					light;					// Light sources.
uniform mat4							mv;						// Model view matrix of globe.
uniform mat4							p;						// Projection matrix.
uniform mat4							plantM;					// Plant placement matrix on globe.
uniform vec3							vForce;					// wind force vector, possibly combined with others such as blast

in vec3									normal;					// Incoming vertex normal.
in vec3									position;				// Incoming vertex location.
in vec3									texCoordMovement;		// Incoming texture coordinate(xy), move amount due to wind(z),

out vec4								vertColor;				// Outgoing vertex color.
out vec3								vertTexCoordFog;		// Outgoing texture coordinate(xy) and fog density (z)

void main()
{
	// Calculate vertex position in eye space.
	mat4 mvplant = mv * plantM;
	gl_Position = mvplant * vec4(position, 1);

	// Apply effect of eye space force vector.
	gl_Position.xyz += vForce * texCoordMovement.z;

	// Calculate lighting using eye space position and normal.
	vec4 eyeN = normalize(mvplant * vec4(normal, 1) - mvplant * vec4(0,0,0,1));	// Surface normal in eye space.
	vertColor = vec4(computeLighting(light, gl_Position.xyz, eyeN.xyz), glowAlpha.a);
	vertColor.rgb = min(vertColor.rgb+glowAlpha.rgb, vec3(1.5, 1.5, 1.5));

	gl_Position = p * gl_Position;

	vertTexCoordFog.xy = texCoordMovement.xy;					// Pass unmodified texture coordinate to the fragment shader.

	// Calculate fog density but save the effect for the fragment program.
	// Fog density value is 1 for full fog, 0 for no fog.
	float d = max(gl_Position.z, 0) * fogDensity;
	float f = 1 / exp(d*d);
	vertTexCoordFog.z = 1 - clamp(f, 0, 1);
}
