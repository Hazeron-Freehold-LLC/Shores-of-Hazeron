//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: WaterRingworldNoHole.v.glsl,v 1.3 2020/09/14 18:56:21 daveh Exp $
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
	float								shadowAngle;			// angle of rotation of shadow panel
	float								shadowCount;			// shadow panels in circle
	vec3								shadowXVec;       		// X vector of sun plane for shadow panel calcs
	vec3								shadowYVec;       		// Y vector of sun plane for shadow panel calcs
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
		out float oSunlight)
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

	// Compute final diffuse lighting results.
	float lighting = max(0.0, NdotL);
	oDiffuse = sun.color * lighting * attenuation;

	// Modulate result by shadow square effect.
	if(sun.shadowCount > 0.)
	{
		vec3 Ps = P - sun.position;								// Position relative to the sun.
		float sunx = dot(Ps, sun.shadowXVec);
		float suny = dot(Ps, sun.shadowYVec);
		float ang = atan(suny, sunx) - sun.shadowAngle;			// Angle between P and shadow square in plane of sun
		oSunlight = sin(ang * sun.shadowCount) * 2.;
		oSunlight = clamp(oSunlight, 0, 1);						// Prevent negative or excess sunlight.
		oDiffuse *= oSunlight;
	}
	else
		oSunlight = 0;
}

void computePointLight(PointLight light,
			vec3 eyeP,
			float shininess,
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

vec3 computeSunLighting(LightSources light,
			float shininess,
			vec3 P,
			vec3 N,
			out float oSunlight0,
			out float oSunlight1,
			out float oSunlight2)
{
	// Get a diffuse contribution from each sun that is enabled.
	vec3 diffuseLight = vec3(0,0,0);
	vec3 diffuseSum = vec3(0,0,0);

	// Suns
	{
		if(light.sun0.enabled)
		{
			// This sun light is enabled.
			computeSunLight(light.sun0, light.eyeLocation, shininess, P, N, diffuseLight, oSunlight0);
			diffuseSum += diffuseLight;
		}
		else
			oSunlight0 = 0;
		if(light.sun1.enabled)
		{
			// This sun light is enabled.
			computeSunLight(light.sun1, light.eyeLocation, shininess, P, N, diffuseLight, oSunlight1);
			diffuseSum += diffuseLight;
		}
		else
			oSunlight1 = 0;
		if(light.sun2.enabled)
		{
			// This sun light is enabled.
			computeSunLight(light.sun2, light.eyeLocation, shininess, P, N, diffuseLight, oSunlight2);
			diffuseSum += diffuseLight;
		}
		else
			oSunlight2 = 0;
	}

	// Clamp color values
	return clamp(diffuseSum, vec3(0,0,0), vec3(1,1,1));
}

vec3 computeLighting(LightSources light,
			float shininess,
			vec3 P,
			vec3 N)
{
	// Get a diffuse contribution from each sun that is enabled.
	vec3 diffuseLight = vec3(0,0,0);
	vec3 diffuseSum = vec3(0,0,0);

	// Point lights
	for(int l=0; l<MAX_LIGHTS; l++)
	{
		if(light.light[l].enabled)
		{
			// This other light is enabled.
			// Compute general lighting contribution.
			computePointLight(light.light[l], light.eyeLocation, shininess, P, N, diffuseLight);

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
	vec3 color = vec3(light.ambient,light.ambient,light.ambient) + diffuseSum;

	// Clamp color values
	return clamp(color, vec3(0,0,0), vec3(1,1,1));
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

uniform float							fogDensity;				// Fog density for exponential fog calculation.
uniform LightSources					light;					// Light sources.
uniform mat4							p;						// Projection matrix.
uniform mat4							mv;						// Model view matrix.
uniform vec2							texOffset;				// Texture offset

in vec3									position;				// Incoming vertex location.
in vec3									normal;					// Incoming vertex normal.
in vec2									texCoord;				// Incoming texture coordinate.
in vec3									tangent;				// Incoming tangent vector.

out vec3								lightColor;				// Outgoing light color at the vertex.
out vec3								sunColor;				// Outgoing sun color at the vertex.
out vec3								vertTexCoordFog;		// Outgoing texture coordinate (xy) and fog density (z).
out vec2								vertTexCoord2;			// Outgoing texture coordinate.
out vec2								vertTexCoord3;			// Outgoing texture coordinate.
out vec3								vertPos;				// Vertex position in ringworld section space.
out vec3								eyeDir;
out vec3								sunCol0;
out vec3								sunDir0;
out float								sunInt0;
out vec3								sunCol1;
out vec3								sunDir1;
out float								sunInt1;
out vec3								sunCol2;
out vec3								sunDir2;
out float								sunInt2;


void main()
{
	// Calculate lighting using object space position and normal.
	sunColor = computeSunLighting(light, 100., position, normal, sunInt0, sunInt1, sunInt2);
	lightColor = computeLighting(light, 100., position, normal);

	gl_Position = p * mv * vec4(position,1);	// Calculate homogeneous vertex position using projection and model view matrices.

	vertTexCoordFog.xy = texCoord + texOffset;					// Offset texture coordinate for the fragment shader.
	vertTexCoord2 = texCoord/5 + texOffset;						// Lower frequency sample.
	vertTexCoord3 = texCoord/7 + texOffset;						// Lowest frequency sample.

	// Calculate fog density but save the effect for the fragment program.
	// Fog density value is 1 for full fog, 0 for no fog.
	float d = max(gl_Position.z, 0) * fogDensity;
	float f = 1 / exp(d*d);
	vertTexCoordFog.z = 1 - clamp(f, 0, 1);

	// Vertex location relative to the ringworld section is used to clip away the fragments near the viewer.
	vertPos = position;

	// Bump mapping.
	{
		// Binormal
		vec3 bN = normalize(cross(normal, tangent));

		// Eye direction in tangent space.
		eyeDir = position - light.eyeLocation;
		vec3 v;
		v.x = dot(eyeDir, tangent);
		v.y = dot(eyeDir, bN);
		v.z = dot(eyeDir, normal);
		eyeDir = normalize(v);

		// Sun 0 in tangent space.
		if(light.sun0.enabled)
		{
			sunCol0 = light.sun0.color;
			vec3 sunLoc = light.sun0.position - position;
			v.x = dot(sunLoc, tangent);
			v.y = dot(sunLoc, bN);
			v.z = dot(sunLoc, normal);
			sunDir0 = normalize(v);
		}
		else
		{
			sunCol0 = vec3(0);
			sunDir0 = vec3(0);
		}

		// Sun 1 in tangent space.
		if(light.sun1.enabled)
		{
			sunCol1 = light.sun1.color;
			vec3 sunLoc = light.sun1.position - position;
			v.x = dot(sunLoc, tangent);
			v.y = dot(sunLoc, bN);
			v.z = dot(sunLoc, normal);
			sunDir1 = normalize(v);
		}
		else
		{
			sunCol1 = vec3(0);
			sunDir1 = vec3(0);
		}

		// Sun 2 in tangent space.
		if(light.sun2.enabled)
		{
			sunCol2 = light.sun2.color;
			vec3 sunLoc = light.sun2.position - position;
			v.x = dot(sunLoc, tangent);
			v.y = dot(sunLoc, bN);
			v.z = dot(sunLoc, normal);
			sunDir2 = normalize(v);
		}
		else
		{
			sunCol2 = vec3(0);
			sunDir2 = vec3(0);
		}
	}
}
