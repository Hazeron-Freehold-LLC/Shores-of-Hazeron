//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auanimalpart.cpp,v 1.74 2022/05/09 16:12:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auanimalpart.h"
//
#include "auanimalia.h"
#include "aucombat.h"
#include "audice.h"
#include "aunoise.h"
#include "autime.h"

void AuAnimalArm::GenBones(const AuChromatid& dna, quint8& boneIndex)
{
	Q_ASSERT(!HasSkinMesh());
	quint8 boneCount = m_Chromosome->BoneCount(dna);
	boneIndex += boneCount;

	// Initialize my bones.
	m_Bones.clear();
	for(quint8 b=0; b<boneCount; b++)
		m_Bones.append(AuAnimalBone(m_Chromosome->BoneLoc(dna, b)));
}

void AuAnimalArm::GenClothedMesh(const AuChromatid& dna, GROWTHSTAGE stage)
{
	m_ClothedMesh.Clear();
	m_Chromosome->GetClothedMesh(m_ClothedMesh, dna, m_FirstBone, stage);
}

void AuAnimalArm::GenSkinMesh(const AuChromatid& dna, const AuHairStyle& hairStyle, const AuClothing& clothes, GROWTHSTAGE stage)
{
	m_SkinMesh.Clear();
	m_Chromosome->GetSkinMesh(m_SkinMesh, dna, m_FirstBone, hairStyle, clothes, stage);
}

bool AuAnimalArm::AnimatePose(const AuId& id, const AuElapsedTimer& when, const AuAnimalia& body)
{
	float shoulderX, shoulderY, shoulderZ, elbowX, elbowZ, wristX, wristY, wristZ, fingerY, fingerTipY;
	AnglesForPose(id, when, body, shoulderX, shoulderY, shoulderZ, elbowX, elbowZ, wristX, wristY, wristZ, fingerY, fingerTipY);

	AuMatrix4x4 baseM = body.NeckBaseMatrix();

	AuMatrix4x4 sm(baseM);
	sm.Translate(m_Bones[0].UnposedLoc());
	sm.RotateX(shoulderX);
	sm.RotateY(shoulderY);
	sm.RotateZ(shoulderZ);
	sm.Translate(-m_Bones[0].UnposedLoc());

	AuMatrix4x4 em(sm);
	em.Translate(m_Bones[1].UnposedLoc());
	em.RotateX(elbowX);
	em.RotateZ(elbowZ);
	em.Translate(-m_Bones[1].UnposedLoc());

	AuMatrix4x4 wm(em);
	wm.Translate(m_Bones[2].UnposedLoc());
	wm.RotateX(wristX);
	wm.RotateY(wristY);
	wm.RotateZ(wristZ);
	wm.Translate(-m_Bones[2].UnposedLoc());

	AuMatrix4x4 fm(wm);
	fm.Translate(m_Bones[3].UnposedLoc());
	fm.RotateY(fingerY);
	fm.Translate(-m_Bones[3].UnposedLoc());

	AuMatrix4x4 tm(fm);
	tm.Translate(m_Bones[4].UnposedLoc());
	tm.RotateY(fingerTipY);
	tm.Translate(-m_Bones[4].UnposedLoc());

	m_Bones[0].SetMatrix(sm);
	m_Bones[1].SetMatrix(em);
	m_Bones[2].SetMatrix(wm);
	m_Bones[3].SetMatrix(fm);
	m_Bones[4].SetMatrix(tm);

	return false;
}

void AuAnimalArm::AnglesForPose(const AuId& id, const AuElapsedTimer& when, const AuAnimalia& body,
		float& shoulderX, float& shoulderY, float& shoulderZ, float& elbowX, float& elbowZ, float& wristX, float& wristY,
		float& wristZ, float& fingerY, float& fingerTipY)
{
	bool bendDominant = body.BendDominantArm();
	if(bendDominant)
	{
		// Am I the dominant arm?
		AuChromosome::LATERALITY laterality = m_Chromosome->Laterality();
		HANDEDNESS handed = body.Handedness();
		bendDominant = (laterality == AuChromosome::Left && handed == LeftHanded) ||
			(laterality == AuChromosome::Right && handed == RightHanded);
		if(bendDominant)
			bendDominant = m_Chromosome->Which().Index() <= 1;	// Laterality is good. Dominant arm is index 0 or 1.
	}
	m_Chromosome->AnglesForPose(id, when, body.PoseFrequency(), body.ApparentSectionPose(PoseSection()),
			body.CurrentHeadTilt(), body.Handedness(), bendDominant,
			shoulderX, shoulderY, shoulderZ, elbowX, elbowZ, wristX, wristY, wristZ, fingerY, fingerTipY);
}

void AuAnimalArm::SightExtents(AuPoint& min, AuPoint& max) const
{
	m_Chromosome->SightExtents(min, max, m_Bones);
}

bool AuAnimalArm::SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const
{
	if(!m_Chromosome->SightTargetDetails(eyeLoc, sightedLoc, m_Bones))
		return false;
	sightedPart = (m_Chromosome->Which().Raw() << 8) | PoseSection();
	return true;
}

float AuAnimalArm::Volume() const
{
	return m_Chromosome->Volume();
}

bool AuAnimalArm::Weapon(const AuChromatid& dna, AuDNABase::WEAPONLOC loc, AuCombatWeapon& weapon) const
{
	Q_UNUSED(dna);
	if(loc != AuDNABase::WL_Hands)
		return false;

	weapon.m_Mode = WM_Contact;
	//if(m_DNA.HandType == DhDNA::HAND_Claw || m_DNA.HandType == DhDNA::HAND_Pincher)
		//weapon.m_w1.m_Effect = WE_Slash;
	//else
		weapon.m_w1.m_Effect = WE_Bludgeon;
	weapon.m_Range = m_Chromosome->LengthMM() * FeetPerMM();

	// Hand size determines base damage.
	float v = Volume();
	weapon.m_w1.m_Damage = (int)sqrtf(v*1000);
	if(!weapon.m_w1.m_Damage)
		weapon.m_w1.m_Damage = 1;
	return true;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuAnimalFin::GenBones(const AuChromatid& dna, quint8& boneIndex)
{
	Q_ASSERT(!HasSkinMesh());
	quint8 boneCount = m_Chromosome->BoneCount(dna);
	boneIndex += boneCount;

	// Initialize my bones.
	m_Bones.clear();
	for(quint8 b=0; b<boneCount; b++)
		m_Bones.append(AuAnimalBone(m_Chromosome->BoneLoc(dna, b)));
}

void AuAnimalFin::GenClothedMesh(const AuChromatid& dna, GROWTHSTAGE stage)
{
	m_ClothedMesh.Clear();
	//DHH
}

void AuAnimalFin::GenSkinMesh(const AuChromatid& dna, const AuHairStyle& hairStyle, const AuClothing& clothes, GROWTHSTAGE stage)
{
	m_SkinMesh.Clear();
	//DHH
}

bool AuAnimalFin::AnimatePose(const AuId& id, const AuElapsedTimer& when, const AuAnimalia& body)
{
	//DHH

	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuAnimalHead::GenBones(const AuChromatid& dna, quint8& boneIndex)
{
	Q_ASSERT(!HasSkinMesh());
	quint8 boneCount = m_Chromosome->BoneCount(dna);
	boneIndex += boneCount;

	// Initialize my bones.
	m_Bones.clear();
	for(quint8 b=0; b<boneCount; b++)
		m_Bones.append(AuAnimalBone(m_Chromosome->BoneLoc(dna, b)));
}

void AuAnimalHead::GenClothedMesh(const AuChromatid& dna, GROWTHSTAGE stage)
{
	m_ClothedMesh.Clear();
	m_Chromosome->GetClothedMesh(m_ClothedMesh, dna, m_FirstBone, stage);
}

void AuAnimalHead::GenSkinMesh(const AuChromatid& dna, const AuHairStyle& hairStyle, const AuClothing& clothes, GROWTHSTAGE stage)
{
	m_SkinMesh.Clear();
	m_Chromosome->GetSkinMesh(m_SkinMesh, dna, m_FirstBone, hairStyle, clothes, stage);
}

bool AuAnimalHead::AnimatePose(const AuId& id, const AuElapsedTimer& when, const AuAnimalia& body)
{
	float headX, headY, headZ, eyeLeftX, eyeLeftZ, eyeRightX, eyeRightZ, jawX, jawLeftY, jawRightY;
	AnglesForPose(id, when, body, headX, headY, headZ, eyeLeftX, eyeLeftZ, eyeRightX, eyeRightZ, jawX, jawLeftY, jawRightY);

	AuMatrix4x4 bm;
	if(body.HasNeck())
	{
		// Neck has already tilted it's final matrix to the current head tilt.
		bm = body.HeadBaseMatrix();
	}
	else
	{
		bm.Translate(m_Bones[0].UnposedLoc());
		bm.RotateX(headX);
		bm.RotateY(headY);
		bm.RotateZ(headZ);
		bm.Translate(-m_Bones[0].UnposedLoc());
		bm = body.NeckBaseMatrix() * bm;
	}

	// HeadOnly is rendered at the origin.
	if(body.CurrentGeneralPose().HeadOnly())
	{
		AuPoint headOffset = m_Bones[0].UnposedLoc();
		bm.Translate(-headOffset);
	}

	m_Bones[0].SetMatrix(bm);

	// Jaw
	AuMatrix4x4 mJaw;
	mJaw.Translate(m_Bones[1].UnposedLoc());
	mJaw.RotateX(jawX);
	mJaw.Translate(-m_Bones[1].UnposedLoc());
	m_Bones[1].SetMatrix(bm * mJaw);

	AuMatrix4x4 mJawLeft;
	mJawLeft.Translate(m_Bones[2].UnposedLoc());
	mJawLeft.RotateY(jawLeftY);
	mJawLeft.Translate(-m_Bones[2].UnposedLoc());
	m_Bones[2].SetMatrix(bm * mJawLeft);

	AuMatrix4x4 mJawRight;
	mJawRight.Translate(m_Bones[3].UnposedLoc());
	mJawRight.RotateY(jawRightY);
	mJawRight.Translate(-m_Bones[3].UnposedLoc());
	m_Bones[3].SetMatrix(bm * mJawRight);

	if(m_Bones.size() > 4)
	{
		// Left eye
		AuMatrix4x4 mEyeL;
		mEyeL.Translate(m_Bones[4].UnposedLoc());
		mEyeL.RotateX(eyeLeftX);
		mEyeL.RotateZ(eyeLeftZ);
		mEyeL.Translate(-m_Bones[4].UnposedLoc());
		m_Bones[4].SetMatrix(bm * mEyeL);
	}

	if(m_Bones.size() > 5)
	{
		// Right eye
		AuMatrix4x4 mEyeR;
		mEyeR.Translate(m_Bones[5].UnposedLoc());
		mEyeR.RotateX(eyeRightX);
		mEyeR.RotateZ(eyeRightZ);
		mEyeR.Translate(-m_Bones[5].UnposedLoc());
		m_Bones[5].SetMatrix(bm * mEyeR);
	}

	return false;
}

void AuAnimalHead::AnglesForPose(const AuId& id, const AuElapsedTimer& when, const AuAnimalia& body,
		float& headX, float& headY, float& headZ, float& eyeLeftX, float& eyeLeftZ, float& eyeRightX, float& eyeRightZ, float& jawX,
		float& jawLeftY, float& jawRightY)
{
	m_Chromosome->AnglesForPose(id, when, body.PoseFrequency(), body.ApparentSectionPose(PoseSection()).Pose(),
			body.ApparentSectionPose(PS_Mouth).Pose(), body.CurrentHeadTilt(), body.CurrentHeadTurn(), body.Handedness(),
			headX, headY, headZ, eyeLeftX, eyeLeftZ, eyeRightX, eyeRightZ, jawX, jawLeftY, jawRightY);
}

void AuAnimalHead::SightExtents(AuPoint& min, AuPoint& max) const
{
	m_Chromosome->SightExtents(min, max, m_Bones);
}

bool AuAnimalHead::SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const
{
	if(!m_Chromosome->SightTargetDetails(eyeLoc, sightedLoc, m_Bones))
		return false;
	sightedPart = PoseSection();
	return true;
}

float AuAnimalHead::Volume() const
{
	return m_Chromosome->Volume();
}

bool AuAnimalHead::Weapon(const AuChromatid& dna, AuDNABase::WEAPONLOC loc, AuCombatWeapon& weapon) const
{
	if(loc != AuDNABase::WL_Head)
		return false;

	const AuChromoMouth* mouth = dynamic_cast<const AuChromoMouth*>(dna.Chromosome(AuChromosome::Mouth));
	if(!mouth)
		return false;
	float v = mouth->Volume();	//DHH animalia Use HornVolume if bigger?
	if(v == 0)
		return false;

	// All physical head weapons seem to produce the same damage effect.
	// We don't head-butt so there's no bludgeoning effect.
	//DHH animalia horns?
	weapon.m_Mode = WM_Contact;
	weapon.m_Range = dna.BiteRange();
	switch(mouth->Teeth())
	{
	case AuChromoMouth::NoTeeth:
		weapon.m_w1.m_Effect = WE_Crush;
		weapon.m_w1.m_Damage = (int)(sqrtf(v*1000) * 2);
		break;
	case AuChromoMouth::Flat:
		weapon.m_w1.m_Effect = WE_Crush;
		weapon.m_w1.m_Damage = (int)(sqrtf(v*1000) * 3);
		break;
	case AuChromoMouth::Fangs:
		weapon.m_w1.m_Effect = WE_Pierce;
		weapon.m_w1.m_Damage = (int)(sqrtf(v*1000) * 4);
		break;
	}
	if(!weapon.m_w1.m_Damage)
		weapon.m_w1.m_Damage = 1;
	return true;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuAnimalLeg::GenBones(const AuChromatid& dna, quint8& boneIndex)
{
	Q_ASSERT(!HasSkinMesh());
	quint8 boneCount = m_Chromosome->BoneCount(dna);
	boneIndex += boneCount;

	// Initialize my bones.
	m_Bones.clear();
	for(quint8 b=0; b<boneCount; b++)
		m_Bones.append(AuAnimalBone(m_Chromosome->BoneLoc(dna, b)));
}

void AuAnimalLeg::GenClothedMesh(const AuChromatid& dna, GROWTHSTAGE stage)
{
	m_ClothedMesh.Clear();
	m_Chromosome->GetClothedMesh(m_ClothedMesh, dna, m_FirstBone, stage);
}

void AuAnimalLeg::GenSkinMesh(const AuChromatid& dna, const AuHairStyle& hairStyle, const AuClothing& clothes, GROWTHSTAGE stage)
{
	m_SkinMesh.Clear();
	m_Chromosome->GetSkinMesh(m_SkinMesh, dna, m_FirstBone, hairStyle, clothes, stage);
}

bool AuAnimalLeg::AnimatePose(const AuId& id, const AuElapsedTimer& when, const AuAnimalia& body)
{
	float hipX, hipY, hipZ, kneeX, ankleX, toeX;
	AnglesForPose(id, when, body, hipX, hipY, hipZ, kneeX, ankleX, toeX);

	AuMatrix4x4 baseM = body.TorsoBaseMatrix();

	AuMatrix4x4 hm(baseM);
	hm.Translate(m_Bones[0].UnposedLoc());
	hm.RotateX(hipX);
	hm.RotateY(hipY);
	hm.RotateZ(hipZ);
	hm.Translate(-m_Bones[0].UnposedLoc());

	AuMatrix4x4 km(hm);
	km.Translate(m_Bones[1].UnposedLoc());
	km.RotateX(kneeX);
	km.Translate(-m_Bones[1].UnposedLoc());

	AuMatrix4x4 am(km);
	am.Translate(m_Bones[2].UnposedLoc());
	am.RotateX(ankleX);
	am.Translate(-m_Bones[2].UnposedLoc());

	AuMatrix4x4 tm(am);
	tm.Translate(m_Bones[3].UnposedLoc());
	tm.RotateX(toeX);
	tm.Translate(-m_Bones[3].UnposedLoc());

	m_Bones[0].SetMatrix(hm);
	m_Bones[1].SetMatrix(km);
	m_Bones[2].SetMatrix(am);
	m_Bones[3].SetMatrix(tm);

	return false;
}

void AuAnimalLeg::AnglesForPose(const AuId& id, const AuElapsedTimer& when, const AuAnimalia& body,
		float& hipX, float& hipY, float& hipZ, float& kneeX, float& ankleX, float& toeX)
{
	m_Chromosome->AnglesForPose(id, when, body.PoseFrequency(), body.ApparentSectionPose(PoseSection()), body.Handedness(),
			body.HasMaleGenitals(), body.TorsoThickness(),
			hipX, hipY, hipZ, kneeX, ankleX, toeX);
}

float AuAnimalLeg::HipAltitudePosed(const AuChromatid& ctd) const
{
	if(m_Bones.size() < 4)
		return 0;
	AuPoint ballLoc = m_Bones[3].Matrix().Project(m_Chromosome->BoneLoc(ctd, 3));				// Bone 3 is the ball of my foot.
	return -ballLoc.z;
}

void AuAnimalLeg::SightExtents(AuPoint& min, AuPoint& max) const
{
	m_Chromosome->SightExtents(min, max, m_Bones);
}

bool AuAnimalLeg::SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const
{
	if(!m_Chromosome->SightTargetDetails(eyeLoc, sightedLoc, m_Bones))
		return false;
	sightedPart = (m_Chromosome->Which().Raw() << 8) | PoseSection();
	return true;
}

float AuAnimalLeg::Volume() const
{
	return m_Chromosome->Volume();
}

bool AuAnimalLeg::Weapon(const AuChromatid& dna, AuDNABase::WEAPONLOC loc, AuCombatWeapon& weapon) const
{
	Q_UNUSED(dna);
	if(loc != AuDNABase::WL_Feet)
		return false;

	weapon.m_Mode = WM_Contact;
	//if(m_DNA.FootType == DhDNA::FOOT_Claw || m_DNA.FootType == DhDNA::FOOT_Pincher)
		//weapon.m_w1.m_Effect = WE_Slash;
	//else
		weapon.m_w1.m_Effect = WE_Bludgeon;
	weapon.m_Range = (m_Chromosome->LengthMM() + m_Chromosome->FootLengthMM()) * FeetPerMM();

	// Hand size determines base damage.
	float v = Volume();
	weapon.m_w1.m_Damage = (int)sqrtf(v*1000);
	if(!weapon.m_w1.m_Damage)
		weapon.m_w1.m_Damage = 1;
	return true;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuAnimalTentacle::GenBones(const AuChromatid& dna, quint8& boneIndex)
{
	Q_ASSERT(!HasSkinMesh());
	quint8 boneCount = m_Chromosome->BoneCount(dna);
	boneIndex += boneCount;

	// Initialize my bones.
	m_Bones.clear();
	for(quint8 b=0; b<boneCount; b++)
		m_Bones.append(AuAnimalBone(m_Chromosome->BoneLoc(dna, b)));
}

void AuAnimalTentacle::GenClothedMesh(const AuChromatid& dna, GROWTHSTAGE stage)
{
	m_ClothedMesh.Clear();
	//DHH
}

void AuAnimalTentacle::GenSkinMesh(const AuChromatid& dna, const AuHairStyle& hairStyle, const AuClothing& clothes,
		GROWTHSTAGE stage)
{
	m_SkinMesh.Clear();
	//DHH
}

bool AuAnimalTentacle::AnimatePose(const AuId& id, const AuElapsedTimer& when, const AuAnimalia& body)
{
	//DHH

	return false;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuAnimalTorso::GenBones(const AuChromatid& dna, quint8& boneIndex)
{
	Q_ASSERT(!HasSkinMesh());
	quint8 boneCount = m_Chromosome->BoneCount(dna);
	boneIndex += boneCount;

	// Initialize my bones.
	m_Bones.clear();
	for(quint8 b=0; b<boneCount; b++)
		m_Bones.append(AuAnimalBone(m_Chromosome->BoneLoc(dna, b)));
}

void AuAnimalTorso::GenClothedMesh(const AuChromatid& dna, GROWTHSTAGE stage)
{
	m_ClothedMesh.Clear();
	m_Chromosome->GetClothedMesh(m_ClothedMesh, dna, m_FirstBone, stage);
}

void AuAnimalTorso::GenSkinMesh(const AuChromatid& dna, const AuHairStyle& hairStyle, const AuClothing& clothes, GROWTHSTAGE stage)
{
	m_SkinMesh.Clear();
	m_Chromosome->GetSkinMesh(m_SkinMesh, dna, m_FirstBone, hairStyle, clothes, stage);
}

bool AuAnimalTorso::AnimatePose(const AuId& id, const AuElapsedTimer& when, const AuAnimalia& body)
{
	float buttX, buttY, buttZ, chestX, chestY, chestZ, neckX, neckY, neckZ, tailX, tailY;
	AnglesForPose(id, when, body, buttX, buttY, buttZ, chestX, chestY, chestZ, neckX, neckY, neckZ, tailX, tailY);

	AuMatrix4x4 bm;

	AuPose pose(body.CurrentGeneralPose());
	if(pose.Pose() == AuPose::MateOnTop)
	{
		float cycleOffset = AuDice(id.GetID()).Roll(1,360,-1)*DEG2RADf;
		float persin = pose.Rhythm(body.PoseFrequency(), cycleOffset, when);
		bm.Translate(0, -body.TorsoThickness() * .75f, persin*body.TorsoThickness()/4);
	}

	bm.Translate(m_Bones[0].UnposedLoc());
	bm.RotateX(buttX);
	bm.RotateY(buttY);
	bm.RotateZ(buttZ);
	bm.Translate(-m_Bones[0].UnposedLoc());

	AuMatrix4x4 cm(bm);
	cm.Translate(m_Bones[1].UnposedLoc());
	cm.RotateX(chestX);
	cm.RotateY(chestY);
	cm.RotateZ(chestZ);
	cm.Translate(-m_Bones[1].UnposedLoc());

	m_Bones[0].SetMatrix(bm);
	m_Bones[1].SetMatrix(cm);

	quint8 neckBones = m_Chromosome->NeckBoneCount();
	float tiltPerBoneX = neckX / neckBones;
	float tiltPerBoneY = neckY / neckBones;
	float tiltPerBoneZ = neckZ / neckBones;
	for(int b=0; b<neckBones; b++)
	{
		int bone = b + 2;	// First two bones are the back bone.
		// Build matrix for this neck segment.
		AuMatrix4x4 nm;
		nm.Translate(m_Bones[bone].UnposedLoc());
		nm.RotateX(tiltPerBoneX);
		nm.RotateY(tiltPerBoneY);
		nm.RotateZ(tiltPerBoneZ);
		nm.Translate(-m_Bones[bone].UnposedLoc());

		// Propagate it by the previous segment's matrix.
		nm = cm * nm;
		m_Bones[bone].SetMatrix(nm);

		// Move up to the previous matrix.
		cm = nm;
	}

	quint8 tailBones = m_Chromosome->TailBoneCount();
	tiltPerBoneX = tailX / tailBones;
	tiltPerBoneY = tailY / tailBones;
	for(int b=0; b<tailBones; b++)
	{
		int bone = b + 2 + neckBones;	// First two bones are the back bone, then the neck bones.
		// Build matrix for this neck segment.
		AuMatrix4x4 nm;
		nm.Translate(m_Bones[bone].UnposedLoc());
		nm.RotateX(tiltPerBoneX);
		nm.RotateY(tiltPerBoneY);
		nm.Translate(-m_Bones[bone].UnposedLoc());

		// Propagate it by the previous segment's matrix.
		nm = bm * nm;
		m_Bones[bone].SetMatrix(nm);

		// Move up to the previous matrix.
		bm = nm;
	}

	return false;
}

void AuAnimalTorso::AnglesForPose(const AuId& id, const AuElapsedTimer& when, const AuAnimalia& body,
		float& buttX, float& buttY, float& buttZ, float& chestX, float& chestY, float& chestZ, float& neckX, float& neckY,
		float& neckZ, float& tailX, float& tailY)
{
	m_Chromosome->AnglesForPose(id, when, body.PoseFrequency(), body.ApparentSectionPose(PoseSection()), body.CurrentHeadTilt(),
			body.CurrentHeadTurn(), body.Handedness(), body.HasMaleGenitals(),
			buttX, buttY, buttZ, chestX, chestY, chestZ, neckX, neckY, neckZ, tailX, tailY);
}

const AuMatrix4x4& AuAnimalTorso::HeadBaseMatrix() const
{
	const QList<AuAnimalBone>& bones = Bones();
	quint8 neckBones = m_Chromosome->NeckBoneCount();
	if(neckBones)
		return bones[neckBones+1].Matrix();
	return NeckBaseMatrix();
}

const AuMatrix4x4& AuAnimalTorso::BaseMatrix() const
{
	const QList<AuAnimalBone>& bones = Bones();
	return bones[0].Matrix();
}

const AuMatrix4x4& AuAnimalTorso::NeckBaseMatrix() const
{
	const QList<AuAnimalBone>& bones = Bones();
	return bones[1].Matrix();
}

bool AuAnimalTorso::HasNeck() const
{
	return m_Chromosome->NeckLengthMM() > 0;
}

bool AuAnimalTorso::HasTail() const
{
	return m_Chromosome->TailLengthMM() > 0;
}

float AuAnimalTorso::BottomThickness() const
{
	return m_Chromosome->BottomThickness();
}

void AuAnimalTorso::SightExtents(AuPoint& min, AuPoint& max) const
{
	m_Chromosome->SightExtents(min, max, m_Bones);
}

bool AuAnimalTorso::SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const
{
	if(!m_Chromosome->SightTargetDetails(eyeLoc, sightedLoc, m_Bones))
		return false;
	sightedPart = PoseSection();
	return true;
}

float AuAnimalTorso::Width() const
{
	quint16 twid;
	if(m_Chromosome->LowerLengthMM() && m_Chromosome->UpperLengthMM())
	{
		quint16 lwid = m_Chromosome->LowerWidthMM();
		quint16 uwid = m_Chromosome->UpperWidthMM();
		if(lwid > uwid)
			twid = lwid;
		else
			twid = uwid;
	}
	else if(m_Chromosome->LowerLengthMM())
		twid = m_Chromosome->LowerWidthMM();
	else
		twid = m_Chromosome->UpperWidthMM();
	return (float)twid * FeetPerMM();
}

float AuAnimalTorso::Volume() const
{
	return m_Chromosome->Volume();
}

bool AuAnimalTorso::Weapon(const AuChromatid& dna, AuDNABase::WEAPONLOC loc, AuCombatWeapon& weapon) const
{
	if(loc != AuDNABase::WL_Tail)
		return false;

	float v = m_Chromosome->TailVolume();
	if(v == 0)
		return false;

	weapon.m_Mode = WM_Contact;
	weapon.m_w1.m_Effect = WE_Bludgeon;
	weapon.m_Range = (m_Chromosome->LowerLengthMM() + m_Chromosome->TailLengthMM()) * FeetPerMM();

	weapon.m_w1.m_Damage = (int)sqrtf(v*1000);
	if(!weapon.m_w1.m_Damage)
		weapon.m_w1.m_Damage = 1;
	return true;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuAnimalWing::GenBones(const AuChromatid& dna, quint8& boneIndex)
{
	Q_ASSERT(!HasSkinMesh());
	quint8 boneCount = m_Chromosome->BoneCount(dna);
	boneIndex += boneCount;

	// Initialize my bones.
	m_Bones.clear();
	for(quint8 b=0; b<boneCount; b++)
		m_Bones.append(AuAnimalBone(m_Chromosome->BoneLoc(dna, b)));
}

void AuAnimalWing::GenClothedMesh(const AuChromatid& dna, GROWTHSTAGE stage)
{
	m_ClothedMesh.Clear();
	//DHH
}

void AuAnimalWing::GenSkinMesh(const AuChromatid& dna, const AuHairStyle& hairStyle, const AuClothing& clothes, GROWTHSTAGE stage)
{
	m_SkinMesh.Clear();
	//DHH
}

bool AuAnimalWing::AnimatePose(const AuId& id, const AuElapsedTimer& when, const AuAnimalia& body)
{
	//DHH

	return false;
}

bool AuAnimalWing::CanFly() const
{
	return m_Chromosome->CanFly();
}


//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
