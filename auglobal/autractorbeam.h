//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: autractorbeam.h,v 1.2 2022/02/19 18:31:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuTractorBeam_H
#define AuTractorBeam_H

#include <auid.h>
#include "auParticleSystem.h"
#include <aumatrix4x4.h>

class AUGLOBAL_DLL AuTractorBeam : public AuParticleSystem
{
private:
	AuId								m_idParent;				// Id of parent atom.
	float								m_SegScale;				// Scale of trail segments vs particle size returned from GetSize().
	AuMatrix4x4							m_tParent;				// Transformation matrix for the parent of this effect
	float								m_Length;				// my own length so I don't go past the target
	AuPoint								m_TargetLoc;			// for stopping the last particle at the target without overshooting
	bool								m_Switch;				// use this to alternate draw order
	int									m_Order;				// use this to spread the texture across multiple particles 
	float								m_TargetSize;
	GLuint								m_textureID3;			// Texture for glow at target end of beam.

public:
	AuTractorBeam();
	~AuTractorBeam();

	inline GLuint GetTextureID3()
		{ return m_textureID3; }

	inline void SetLength(float length)
	{
		m_Length = length;
	}

	// 'parentId' is the atom id of the parent atom of the atom that is generating this contrail.
	// 'parentT' is the atom transform of the atom generating this contrail relative to parentId.
	void SetParent(const AuId& parentId, const AuMatrix4x4& parentT);

	inline void SetSegmentScale(float scale)
	{
		m_SegScale = scale;
	}

	inline void SetTargetLoc(const AuPoint &targetLoc)
	{
		m_TargetLoc = targetLoc;
	}

	inline void SetTargetSize(float targetSize)
	{
		m_TargetSize = targetSize;
	}

	inline void SetTextureID3(GLuint textureId)	// Sets texture id used to render the particles
		{ m_textureID3 = textureId; }

	// Overrides
	virtual void InitParticle(auParticle* pParticle);
	virtual void MoveParticle(auParticle* pParticle);
	virtual void Render(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, float scale=1.f);
};

#endif	//AuTractorBeam_H
