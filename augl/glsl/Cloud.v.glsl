//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: Cloud.v.glsl,v 1.18 2016/09/09 00:09:22 daveh Exp $
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
	vec3								sunsetColor;         	// color of sunset light
	vec3								position;       		// position of light source, center of sun in object space
	float								radius;         		// radius of sun
};

struct LightSources
{
	float								ambient;
	SunLight							sun0;
	SunLight							sun1;
	SunLight							sun2;
};

void computeSunLight(SunLight sun,
			vec3 P,
			vec3 N,
		out vec3 oDiffuse,
		out vec3 oSunset)
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
	oDiffuse = sun.color * lighting * attenuation;

	// Compute sunset lighting.
	float sunset = 1 - min(abs(NdotL * 4), 1);
	oSunset = sun.sunsetColor * sunset * attenuation;
}

vec3 computeSolarLighting(LightSources light,
			vec3 P,
			vec3 N,
			float cloudSunsetFactor)
{
	// Get a diffuse contribution from each sun that is enabled.
	vec3 diffuseLight = vec3(0,0,0);
	vec3 diffuseSum = vec3(0,0,0);
	vec3 sunsetLight = vec3(0,0,0);
	vec3 sunsetSum = vec3(0,0,0);

	// Suns
	{
		if(light.sun0.enabled)
		{
			// This sun light is enabled.
			computeSunLight(light.sun0, P, N, diffuseLight, sunsetLight);
			diffuseSum += diffuseLight;
			sunsetSum += sunsetLight;
		}
		if(light.sun1.enabled)
		{
			// This sun light is enabled.
			computeSunLight(light.sun1, P, N, diffuseLight, sunsetLight);
			diffuseSum += diffuseLight;
			sunsetSum += sunsetLight;
		}
		if(light.sun2.enabled)
		{
			// This sun light is enabled.
			computeSunLight(light.sun2, P, N, diffuseLight, sunsetLight);
			diffuseSum += diffuseLight;
			sunsetSum += sunsetLight;
		}
	}

	// Calculate final color of lit vertex.
	vec3 color = vec3(light.ambient,light.ambient,light.ambient) + diffuseSum + sunsetSum * cloudSunsetFactor;

	// Clamp color values
	return clamp(color, vec3(0,0,0), vec3(1,1,1));
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

uniform vec4							cloudColor;				// Cloud color.
uniform float							cloudLayerRadius;		// Cloud layer radius.
uniform float							cloudLiftMax;			// Height added to cloudLayerRadius at max cloud density.
uniform float							cloudSunset;			// 1 on bottom layer to 0 on top layer
uniform mat4							pmv;					// Projection model view matrix.
uniform LightSources					light;					// Light sources.

uniform sampler2D						texWx;					// r=SaturationP, g=VaporP, b=instability
uniform vec2							offWx;					// horiz offset x=SaturationP, y=VaporP
uniform vec2							minWx;					// minimum x=SaturationP, y=VaporP
uniform vec2							maxWx;					// maximum x=SaturationP, y=VaporP

in vec3									position;				// Incoming vertex location.
in vec2									texCoord;				// Incoming texture coordinate.

out vec4								vertColor;				// Outgoing vertex color.
out vec2								vertTexCoord;			// Outgoing texture coordinate.

void main()
{
	// The model is a globe of unit radius centered at 0,0 so the vertexes are also the normals. cloudLayerRadius is the real size.
	vec3 posRad = position * cloudLayerRadius;

	// Lift vertex based on cloud density.
	if(cloudLiftMax != 0)
	{
		// Calculate saturation pressure without banding as if flow east is 0 and flow west is 1.
		vec2 tcoord;
		tcoord.y = texCoord.y;
		tcoord.x = fract(texCoord.x - offWx.x + 1);
		vec3 colW = texture(texWx, tcoord).rgb;
		float sp = minWx.x + colW.r * (maxWx.x - minWx.x);
		if(sp > 0)
		{
			// Calculate vapor pressure without banding.
			vec2 vpcoord;
			vpcoord.y = texCoord.y;
			vpcoord.x = fract(texCoord.x - offWx.y + 1);
			float vp = minWx.y + texture(texWx, vpcoord).g * (maxWx.y - minWx.y);
			vp *= 1 + (colW.b - .5) / 100.;
			if(vp > sp)
			{
				float lift = clamp((vp / sp)/3 + .1, 0, 1) * cloudLiftMax;
				posRad += position * lift;
			}
		}
	}

	// Calculate lighting using object space position and normal.
	vertColor = vec4(computeSolarLighting(light, posRad, position, cloudSunset), 1) * cloudColor;

	vertTexCoord = texCoord;			// Pass unmodified texture coordinate to the fragment shader.

	// Calculate homogeneous vertex position using projection and model view matrices.
	// Position is a unit vector from the center of the cloud sphere; it must be multiplied by the radius for true positioning.
	gl_Position = pmv * vec4(posRad, 1);
}
