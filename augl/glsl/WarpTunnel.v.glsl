//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: WarpTunnel.v.glsl,v 1.5 2016/09/09 00:17:04 daveh Exp $
// Copyright (c) 2026 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#version 150

uniform mat4							pmv;					// Projection model view matrix.
uniform float							rotationAngle;			// Rotation angle applied to cube map texture coords.

in vec3									position;				// Incoming vertex location.
in vec2									texCoord;				// Incoming texture coordinate(x) is alpha value, (y) is blue shift

out vec3								vertTexCoord;			// Outgoing texture coordinate(xyz)
out vec4								vertColor;				// Color shift at the vertex, and alpha.

void main()
{
	gl_Position = pmv * vec4(position,1);	// Calculate homogeneous vertex position using projection and model view matrices.

	// Calculate texture coord.
	vec3 pv = normalize(position);
	vec3 n = normalize(cross(pv, vec3(0,0,1)));	// Rotation axis
	float c = cos(rotationAngle);					// Rotation angle
	float s = sin(rotationAngle);					// Rotation angle
	mat4 R;
	R[0] = vec4(n.xyz * n.x * (1.0 - c) + vec3(     c,  n.z*s, -n.y*s), 0.0);
	R[1] = vec4(n.xyz * n.y * (1.0 - c) + vec3(-n.z*s,      c,  n.x*s), 0.0);
	R[2] = vec4(n.xyz * n.z * (1.0 - c) + vec3( n.y*s, -n.x*s,      c), 0.0);
	R[3] = vec4(0.0, 0.0, 0.0, 1.0);
	vertTexCoord = (R * vec4(pv,1)).xyz;

	if(texCoord.y < 0)
		vertColor.rgb = mix(vec3(1,1,1), vec3(0,0,1), -texCoord.y);	// Blue shift.
	else
		vertColor.rgb = mix(vec3(1,1,1), vec3(1,0,0), texCoord.y);	// Red shift.
	vertColor.a = texCoord.x;
}
