//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: TerrainLit.v.glsl,v 1.20 2017/12/12 00:13:06 daveh Exp $
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
	vec3								cityAmbient;
	PointLight							light[MAX_LIGHTS];
	SunLight							sun0;
	SunLight							sun1;
	SunLight							sun2;
};

void computeSunLight(SunLight sun,
			vec3 P,
			vec3 N,
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
	vec3 U = normalize(P);										// Up vector at vertex location, with respect to the globe.
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
			vec3 N,
			float city)
{
	// Get a diffuse contribution from each sun that is enabled.
	vec3 diffuseLight = vec3(0,0,0);
	vec3 diffuseSum = vec3(0,0,0);

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

	// Calculate spherical ambient lighting to enhance steep slopes.
	vec3 upVec = normalize(P);									// Up vector with respect to the globe.

	// Calculate spherical ambient lighting that drops off to zero.
	//float ambient = light.ambient * (dot(N, upVec) + 1)/2;

	// Calculate spherical ambient lighting that drops off to 20% the ambient light value.
	float ambient = mix(light.ambient * .15, light.ambient, (dot(N, upVec) + 1.)/2.);

	// Calculate ambient lighting due to city.
	vec3 cityAmbient = light.cityAmbient * city;

	// Calculate final color of lit vertex.
	vec3 color = vec3(ambient,ambient,ambient) + diffuseSum + cityAmbient;

	// Clamp color values
	return clamp(color, vec3(0,0,0), vec3(1,1,1));
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

uniform float							fogDensity;				// Fog density for exponential fog calculation.
uniform LightSources					light;					// Light sources.
uniform mat4							p;						// Projection matrix.
uniform mat4							mv;						// Model view matrix.

in vec4									normal;					// Incoming vertex normal(xyz), pavement type(w)
in vec4									position;				// Incoming vertex location(xyz), snow coverage(w)
in vec4									texCoordAltVeg;			// Incoming texture coordinate(xy), altitude(z), veg density(w)
in vec2									paveCity;				// Pavement coverage(x), city presence(y)

out vec4								vertColor;				// Outgoing vertex color.
out vec3								vertFogSnowPave;		// Outgoing fog density(x), snow cover(y), pavement cover(z)
out vec3								vertTexCoordAlt;		// Outgoing texture coordinate(xy), altitude(z)
flat out float							vertPaveType;			// Pavement type must not be interpolated.

void main()
{
	gl_Position = p * mv * vec4(position.xyz,1);				// Calculate position using projection and model view matrices.

	vertTexCoordAlt = texCoordAltVeg.xyz;						// Pass unmodified texture coordinate to the fragment shader.

	// Calculate lighting using object space position and normal.
	vertColor = vec4(computeLighting(light, position.xyz, normal.xyz, paveCity.y), 1);

	// Calculate fog density but save the effect for the fragment program.
	// Fog density value is 1 for full fog, 0 for no fog.
	float d = max(gl_Position.z, 0) * fogDensity;
	float f = 1 / exp(d*d);
	vertFogSnowPave.x = 1 - clamp(f, 0, 1);

	// Snow coverage is passed to the fragment shader unchanged.
	vertFogSnowPave.y = position.w;

	// Non-zero pavement type triggers 100% presence of pavement at this vertex that blends off to non-paved vertexes.
	// Pavement type is passed uninterpolated to the fragment shader.
	vertFogSnowPave.z = paveCity.x;
	vertPaveType = normal.w;
}
