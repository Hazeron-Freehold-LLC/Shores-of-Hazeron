//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: psweather.h,v 1.13 2022/02/19 18:32:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef PSWEATHER_H
#define PSWEATHER_H

#include "auParticleSystem.h"
#include <aumatrix4x4.h>

class AuGLEffects;
class AuGLVAO;
class GLFunctions;

class AUGLOBAL_DLL PsWeather : public AuParticleSystem
{
public:
	PsWeather();
	virtual ~PsWeather();

	// Different render methods.
	enum RENDER_METHOD
	{
		RENDER_METHOD_Snow,
		RENDER_METHOD_Rain,
		RENDER_METHOD_Snow_Ringworld,
	};

	// Inline functions
	void SetGravityNormal(const AuPoint &vGravity)	// Sets gravity normal
		{ m_gN = vGravity; }
	void SetWind(const AuPoint &vWind)				// Sets wind vector
		{ m_vWind = vWind; }
	void SetFixMatrix(const AuMatrix4x4& m)
		{ m_tFix = m; }
	void SetNewMatrix(const AuMatrix4x4& m)
		{ m_tNew = m; }
	void SetRenderMethod(RENDER_METHOD renderMethod)
		{ m_renderMethod =renderMethod; }
	RENDER_METHOD GetRenderMethod()
		{ return m_renderMethod; }
	void SetSphereConstraint(float fLength)	// Sets sphere constraint length for special particle systems
		{ m_sphereConstraintLength = fLength * fLength; }

	// Overrides
	virtual void MoveParticle(auParticle* pParticle);
	virtual void InitParticle(auParticle* pParticle);
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);

	void CreateSnowFlakeVAO(AuGLEffects& GLf, AuGLVAO& vao);

private:
	AuMatrix4x4	m_tFix;		// The projection matrix for updating particle due to viewer movement and rotation.
	AuMatrix4x4	m_tNew;		// Used to project the position of nu particles. Useful for moving them out ahead when the viewer is moving fast.
	AuPoint		m_vWind;	// Wind vector relative to the viewer in feet per second.
	AuPoint		m_gN;		// Gravity normal
	float		m_sphereConstraintLength;	// Radius squared of sphere
											// Used to push back particles inside a sphere constraints

	RENDER_METHOD	m_renderMethod;		// Render method
};

#endif	//PSWEATHER_H
