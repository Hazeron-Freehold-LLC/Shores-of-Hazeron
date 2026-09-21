//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpbody.cpp,v 1.315 2022/05/09 16:00:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpbody.h"
//
#include "au3dsglsl.h"
#include "aucombat.h"
#include "audice.h"
#include "augleffects.h"
#include "auglobal.h"
#include "auglmodel.h"
#include "austation.h"
#include <QMap>

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class SkinVertex
{
public:
	GLint	idx[4];	// indices of bones 0=prev, 1=this, 2=next
	AuPoint	loc;	// vertex location
	AuPoint	norm;	// normal
	GLfloat	wt[4];	// weight of bones affecting this vertex, must add up to 1.

	SkinVertex(const AuPoint& v,
			GLfloat wt0, GLfloat wt1, GLfloat wt2, GLfloat wt3,
			GLint idx0, GLint idx1, GLint idx2, GLint idx3)
	{
		loc = v;
		CalcNormal();
		idx[0] = idx0;
		idx[1] = idx1;
		idx[2] = idx2;
		idx[3] = idx3;
		wt[0] = wt0;
		wt[1] = wt1;
		wt[2] = wt2;
		wt[3] = wt3;
	}
	SkinVertex()
	{
		norm = AuPoint(0,0,1);
		idx[0] = 0;
		idx[1] = 0;
		idx[2] = 0;
		idx[3] = 0;
		wt[0] = 1;
		wt[1] = 0;
		wt[2] = 0;
		wt[3] = 0;
	}
	void CalcNormal()
	{
		norm = loc;
		norm.Unit();
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

DpBody::DpBody()
{
	InitBoneMatrices();
	m_BendArm = false;
	m_BiteRange = 0;
	for(int iSkin=0; iSkin<SkinTexLim; iSkin++)
	{
		m_glTexSkin[iSkin] = 0;
		m_glTexClothes[iSkin] = 0;
	}
	m_glTexWings = 0;
	for(int iArmor=0; iArmor<ArmorTexLim; iArmor++)
		m_glTexArmor[iArmor] = 0;
	for(int iHelmet=0; iHelmet<HelmetTexLim; iHelmet++)
	{
		for(int iType=0; iType<ArmorTexLim; iType++)
			m_glTexHelmet[iHelmet][iType] = 0;
	}
	m_eBodyClothesType = CID_Invalid;
	m_eHelmetClothesType = CID_Invalid;
	m_HeadTilt = 0;
	m_HeadTurn = 0;
	m_HitPoints = 0;
	m_IsLefty = false;
	for(int ps=0; ps<DpBody::PS_Count; ps++)
		m_Pose[ps] = AuPose::Init;
	m_Sex = 0;
	m_Volume = 0;
	for(int i=0; i<2; i++)
	{
		m_TopColor[i] = QColor(255,255,255,0);
		m_BottomColor[i] = QColor(255,255,255,0);
		m_TopShininess[i] = 0;
		m_BottomShininess[i] = 0;
		m_Glow[i] = 0;
		m_Glowdim[i] = 0;
	}
	m_uTOffset = 0;
	m_vTOffset = 0;
	m_uBOffset = 0;
	m_vBOffset = 0;
	m_CalfLength = 0;
	m_CalfThickness = 0;
	m_ForeArmLength = 0;
	m_ForeArmThickness = 0;
	m_HeadLength = 0;
	m_HeadThickness = 0;
	m_HeadWidth = 0;
	m_ThighLength = 0;
	m_ThighThickness = 0;
	m_TorsoLength = 0;
	m_TorsoSegmentLength = 0;
	m_TorsoThickness = 0;
	m_TorsoWidth = 0;
	m_UpperArmLength = 0;
	m_UpperArmThickness = 0;
}

DpBody::DpBody(char sex, bool isLefty)
{
	InitBoneMatrices();
	m_BendArm = false;
	m_BiteRange = 0;
	for(int iSkin=0; iSkin<SkinTexLim; iSkin++)
	{
		m_glTexSkin[iSkin] = 0;
		m_glTexClothes[iSkin] = 0;
	}
	m_glTexWings = 0;
	for(int iArmor=0; iArmor<ArmorTexLim; iArmor++)
		m_glTexArmor[iArmor] = 0;
	for(int iHelmet=0; iHelmet<HelmetTexLim; iHelmet++)
	{
		for(int iType=0; iType<ArmorTexLim; iType++)
			m_glTexHelmet[iHelmet][iType] = 0;
	}
	m_eBodyClothesType = CID_Invalid;
	m_eHelmetClothesType = CID_Invalid;
	m_HeadTilt = 0;
	m_HeadTurn = 0;
	m_HitPoints = 0;
	m_IsLefty = isLefty;
	for(int ps=0; ps<DpBody::PS_Count; ps++)
		m_Pose[ps] = AuPose::Init;
	m_Sex = sex;
	m_Volume = 0;
	for(int i=0; i<2; i++)
	{
		m_TopColor[i] = QColor(255,255,255,0);
		m_BottomColor[i] = QColor(255,255,255,0);
		m_TopShininess[i] = 0;
		m_BottomShininess[i] = 0;
		m_Glow[i] = 0;
		m_Glowdim[i] = 0;
	}
	m_uTOffset = 0;
	m_vTOffset = 0;
	m_uBOffset = 0;
	m_vBOffset = 0;
	m_CalfLength = 0;
	m_CalfThickness = 0;
	m_ForeArmLength = 0;
	m_ForeArmThickness = 0;
	m_HeadLength = 0;
	m_HeadThickness = 0;
	m_HeadWidth = 0;
	m_ThighLength = 0;
	m_ThighThickness = 0;
	m_TorsoLength = 0;
	m_TorsoSegmentLength = 0;
	m_TorsoThickness = 0;
	m_TorsoWidth = 0;
	m_UpperArmLength = 0;
	m_UpperArmThickness = 0;
}

DpBody::DpBody(char sex, bool isLefty, const DhDNA& dna)
{
	InitBoneMatrices();
	m_BendArm = false;
	m_BiteRange = 0;
	m_DNA = dna;
	for(int iSkin=0; iSkin<SkinTexLim; iSkin++)
	{
		m_glTexSkin[iSkin] = 0;
		m_glTexClothes[iSkin] = 0;
	}
	m_glTexWings = 0;
	for(int iArmor=0; iArmor<ArmorTexLim; iArmor++)
		m_glTexArmor[iArmor] = 0;
	for(int iHelmet=0; iHelmet<HelmetTexLim; iHelmet++)
	{
		for(int iType=0; iType<ArmorTexLim; iType++)
			m_glTexHelmet[iHelmet][iType] = 0;
	}
	m_eBodyClothesType = CID_Invalid;
	m_eHelmetClothesType = CID_Invalid;
	m_HeadTilt = 0;
	m_HeadTurn = 0;
	m_HitPoints = 0;
	m_IsLefty = isLefty;
	for(int ps=0; ps<DpBody::PS_Count; ps++)
		m_Pose[ps] = AuPose::Init;
	m_Sex = sex;
	m_Volume = 0;
	for(int i=0; i<2; i++)
	{
		m_TopColor[i] = QColor(255,255,255,0);
		m_BottomColor[i] = QColor(255,255,255,0);
		m_TopShininess[i] = 0;
		m_BottomShininess[i] = 0;
		m_Glow[i] = 0;
		m_Glowdim[i] = 0;
	}
	m_uTOffset = 0;
	m_vTOffset = 0;
	m_uBOffset = 0;
	m_vBOffset = 0;
	m_CalfLength = 0;
	m_CalfThickness = 0;
	m_ForeArmLength = 0;
	m_ForeArmThickness = 0;
	m_HeadLength = 0;
	m_HeadThickness = 0;
	m_HeadWidth = 0;
	m_ThighLength = 0;
	m_ThighThickness = 0;
	m_TorsoLength = 0;
	m_TorsoSegmentLength = 0;
	m_TorsoThickness = 0;
	m_TorsoWidth = 0;
	m_UpperArmLength = 0;
	m_UpperArmThickness = 0;

	Init();
}

// Base class QVector is explicity initialized on the copy constructor line to eliminate a compiler warning.
DpBody::DpBody(const DpBody& c) : QVector<DpBodyPart*>(), m_Models(c.m_Models)
{
	// Clone the body parts.
	for(const_iterator bpi = c.constBegin(); bpi != c.constEnd(); bpi++)
		append((*bpi)->Clone(this));

	m_BendArm = c.m_BendArm;
	m_BiteRange = c.m_BiteRange;
	for(int ib=0; ib<DhDNA::BONE_Max; ib++)
		m_Bones[ib] = c.m_Bones[ib];
	m_DNA = c.m_DNA;
	for(int iSkin=0; iSkin<SkinTexLim; iSkin++)
	{
		m_glTexSkin[iSkin] = c.m_glTexSkin[iSkin];
		m_glTexClothes[iSkin] = c.m_glTexClothes[iSkin];
	}
	m_glTexWings = c.m_glTexWings;
	for(int iArmor=0; iArmor<ArmorTexLim; iArmor++)
		m_glTexArmor[iArmor] = c.m_glTexArmor[iArmor];
	for(int iHelmet=0; iHelmet<HelmetTexLim; iHelmet++)
	{
		for(int iType=0; iType<ArmorTexLim; iType++)
			m_glTexHelmet[iHelmet][iType] = c.m_glTexHelmet[iHelmet][iType];
	}
	m_bDisplayHelmet = c.m_bDisplayHelmet;
	m_eBodyClothesType = c.m_eBodyClothesType;
	m_eHelmetClothesType = c.m_eHelmetClothesType;
	m_HeadTilt = c.m_HeadTilt;
	m_HeadTurn = c.m_HeadTurn;
	m_HitPoints = c.m_HitPoints;
	m_IsLefty = c.m_IsLefty;
	m_LastSpecUse = c.m_LastSpecUse;
	m_Max = c.m_Max;
	m_Min = c.m_Min;
	for(int ps=0; ps<DpBody::PS_Count; ps++)
		m_Pose[ps] = c.m_Pose[ps];
	m_Sex = c.m_Sex;
	m_Volume = c.m_Volume;
	for(int i=0; i<2; i++)
	{
		m_TopColor[i] = c.m_TopColor[i];
		m_BottomColor[i] = c.m_BottomColor[i];
		m_TopShininess[i] = c.m_TopShininess[i];
		m_BottomShininess[i] = c.m_TopShininess[i];
		m_Glow[i] = c.m_Glow[i];
		m_Glowdim[i] = c.m_Glowdim[i];
	}
	m_uTOffset = c.m_uTOffset;
	m_vTOffset = c.m_vTOffset;
	m_uBOffset = c.m_uBOffset;
	m_vBOffset = c.m_vBOffset;
	m_eyePos = c.m_eyePos;
	m_CalfLength = c.m_CalfLength;
	m_CalfThickness = c.m_CalfThickness;
	m_ForeArmLength = c.m_ForeArmLength;
	m_ForeArmThickness = c.m_ForeArmThickness;
	m_HeadLength = c.m_HeadLength;
	m_HeadThickness = c.m_HeadThickness;
	m_HeadWidth = c.m_HeadWidth;
	m_ThighLength = c.m_ThighLength;
	m_ThighThickness = c.m_ThighThickness;
	m_TorsoLength = c.m_TorsoLength;
	m_TorsoSegmentLength = c.m_TorsoSegmentLength;
	m_TorsoThickness = c.m_TorsoThickness;
	m_TorsoWidth = c.m_TorsoWidth;
	m_UpperArmLength = c.m_UpperArmLength;
	m_UpperArmThickness = c.m_UpperArmThickness;
}

DpBody::~DpBody()
{
	DeleteBodyParts();
}

void DpBody::DeleteBodyParts()
{
	if(!isEmpty())
	{
		for(QVector<DpBodyPart*>::iterator it = begin(); it != end(); it++)
			delete(*it);
		clear();
	}
}

const DpBody& DpBody::operator=(const DpBody& c)
{
	// Delete my body parts. They'll get recreated when I am posed.
	DeleteBodyParts();

	m_BendArm = c.m_BendArm;
	m_BiteRange = c.m_BiteRange;
	for(int ib=0; ib<DhDNA::BONE_Max; ib++)
		m_Bones[ib] = c.m_Bones[ib];
	m_DNA = c.m_DNA;
	for(int iSkin=0; iSkin<SkinTexLim; iSkin++)
	{
		m_glTexSkin[iSkin] = c.m_glTexSkin[iSkin];
		m_glTexClothes[iSkin] = c.m_glTexClothes[iSkin];
	}
	m_glTexWings = c.m_glTexWings;
	for(int iArmor=0; iArmor<ArmorTexLim; iArmor++)
		m_glTexArmor[iArmor] = c.m_glTexArmor[iArmor];
	for(int iHelmet=0; iHelmet<HelmetTexLim; iHelmet++)
	{
		for(int iType=0; iType<ArmorTexLim; iType++)
			m_glTexHelmet[iHelmet][iType] = c.m_glTexHelmet[iHelmet][iType];
	}
	m_bDisplayHelmet = c.m_bDisplayHelmet;
	m_eBodyClothesType = c.m_eBodyClothesType;
	m_eHelmetClothesType = c.m_eHelmetClothesType;
	m_HeadTilt = c.m_HeadTilt;
	m_HeadTurn = c.m_HeadTurn;
	m_HitPoints = c.m_HitPoints;
	m_IsLefty = c.m_IsLefty;
	m_LastSpecUse = c.m_LastSpecUse;
	m_Max = c.m_Max;
	m_Min = c.m_Min;
	for(int ps=0; ps<DpBody::PS_Count; ps++)
		m_Pose[ps] = c.m_Pose[ps];
	m_Sex = c.m_Sex;
	m_Volume = c.m_Volume;
	m_Models = c.m_Models;
	for(int i=0; i<2; i++)
	{
		m_TopColor[i] = c.m_TopColor[i];
		m_BottomColor[i] = c.m_BottomColor[i];
		m_TopShininess[i] = c.m_TopShininess[i];
		m_BottomShininess[i] = c.m_TopShininess[i];
		m_Glow[i] = c.m_Glow[i];
		m_Glowdim[i] = c.m_Glowdim[i];
	}
	m_uTOffset = c.m_uTOffset;
	m_vTOffset = c.m_vTOffset;
	m_uBOffset = c.m_uBOffset;
	m_vBOffset = c.m_vBOffset;
	m_eyePos = c.m_eyePos;
	m_CalfLength = c.m_CalfLength;
	m_CalfThickness = c.m_CalfThickness;
	m_ForeArmLength = c.m_ForeArmLength;
	m_ForeArmThickness = c.m_ForeArmThickness;
	m_HeadLength = c.m_HeadLength;
	m_HeadThickness = c.m_HeadThickness;
	m_HeadWidth = c.m_HeadWidth;
	m_ThighLength = c.m_ThighLength;
	m_ThighThickness = c.m_ThighThickness;
	m_TorsoLength = c.m_TorsoLength;
	m_TorsoSegmentLength = c.m_TorsoSegmentLength;
	m_TorsoThickness = c.m_TorsoThickness;
	m_TorsoWidth = c.m_TorsoWidth;
	m_UpperArmLength = c.m_UpperArmLength;
	m_UpperArmThickness = c.m_UpperArmThickness;
	/*
#if !defined(QT_NO_DEBUG)
	for(int ib=0; ib<DhDNA::BONE_Max; ib++)
	{
		const AuMatrix4x4& bmat = m_Bones[ib];
		if(bmat[3] != 0 || bmat[7] != 0 || bmat[11] != 0 || bmat[15] != 1)
		{
			qDebug("DpBody::operator= Copied Bad Bone Matrix %d in %s", ib,
					(const char*)m_DNA.Description().toAscii());
			bmat.Dump(0);
			Q_ASSERT(false);
		}
	}
#endif
*/

	return(*this);
}

bool DpBody::CanHover() const
{
	// Only certain types of wings can hover.
	if(DNA().ArmType != DhDNA::ARM_WingDragonFly &&
		DNA().ArmType != DhDNA::ARM_WingFairy)
		return false;

	return DNA().CanFly();
}

AuPoint DpBody::CameraLocation() const
{
	if(CurrentGeneralPose() == AuPose::EyesOnly)
		return AuPoint(0,0,0);									// Head only so camera is not offset from my location.

	/*
	// Camera location at turret gets modified in DaCreature
	else if(CurrentGeneralPose() == AuPose::ConsoleTurret0 ||
			CurrentGeneralPose() == AuPose::ConsoleTurret45 ||
			CurrentGeneralPose() == AuPose::ConsoleTurret90 ||
			CurrentGeneralPose() == AuPose::ConsoleTurret135 ||
			CurrentGeneralPose() == AuPose::ConsoleTurret180 ||
			CurrentGeneralPose() == AuPose::ConsoleTurret225 ||
			CurrentGeneralPose() == AuPose::ConsoleTurret270 ||
			CurrentGeneralPose() == AuPose::ConsoleTurret315)
		return AuPoint(0,0,4);		// Close enough.
	else if(CurrentGeneralPose() == AuPose::ConsoleTurretBottom)
		return AuPoint(0,0,-1.5);	// Close enough.
	else if(CurrentGeneralPose() == AuPose::ConsoleTurretTop)
		return AuPoint(0,0,1.5);		// Close enough.
	else if(IsEatingPose())
	{
		// Creatures need this adjustment due to bobbing heads while munching. If their origin is at their eye, their
		// body is constantly in motion, adjusting up and down to the terrain. Munching critters must cause no
		// server load. This offset produces a stationary location while they eat.
		// This induces an offset when changing to/from eating pose that is noticeable, as the body origin changes from its eye to
		// its pivot point and adjusts to the nu terrain altitude there. Directly above/below the eye location minimizes
		// this offset, but the body undulates back and forth wierdly as the creatures head bobs up and down. This looks really
		// strange if I am riding the creature when it starts eating.
		// I suppose the cg offset when eating/not eating is tolerable; it doesn't happen that often. The creatures kind of look
		// like they are pouncing on their food. :P
		// A better solution might be to actually change the origin of the creature by this offset when it changes to|from eating.
		return AuPoint(0,0,0);
	}
	*/


	// Can skip first body part because it's always the torso.
	if(count() > 1)
	{
		if(DNA().Heads && IsRightHanded())
		{
			// Two-headed right-handed creature sees with the second head, which is the right head when there are two.
			bool isLeft = true;	// first head is left
			for(QVector<DpBodyPart*>::const_iterator it = constBegin() + 1; it != constEnd(); it++)
				if((*it)->IsHeadAndNeck())
				{
					if(isLeft)
						isLeft = false;
					else
					{
						DpHeadAndNeck* head = (DpHeadAndNeck*)(*it);
						return head->CurrentCameraLoc();
					}
				}
		}
		else
		{
			// One-headed creature or left-handed creature sees with the first head, which is the left head when there are two.
			for(QVector<DpBodyPart*>::const_iterator it = constBegin() + 1; it != constEnd(); it++)
				if((*it)->IsHeadAndNeck())
				{
					DpHeadAndNeck* head = (DpHeadAndNeck*)(*it);
					return head->CurrentCameraLoc();
				}
		}
	}

	return AuPoint(0,0,0);	// Body pivot point location.
}

AuPoint DpBody::HeadLocation() const
{
	if(CurrentGeneralPose() == AuPose::EyesOnly)
		return AuPoint(0,0,0);									// Head only so camera is not offset from my location.

	/*
	// Camera location at turret gets modified in DaCreature
	else if(CurrentGeneralPose() == AuPose::ConsoleTurret0 ||
			CurrentGeneralPose() == AuPose::ConsoleTurret45 ||
			CurrentGeneralPose() == AuPose::ConsoleTurret90 ||
			CurrentGeneralPose() == AuPose::ConsoleTurret135 ||
			CurrentGeneralPose() == AuPose::ConsoleTurret180 ||
			CurrentGeneralPose() == AuPose::ConsoleTurret225 ||
			CurrentGeneralPose() == AuPose::ConsoleTurret270 ||
			CurrentGeneralPose() == AuPose::ConsoleTurret315)
		return AuPoint(0,0,4);		// Close enough.
	else if(CurrentGeneralPose() == AuPose::ConsoleTurretBottom)
		return AuPoint(0,0,-1.5);	// Close enough.
	else if(CurrentGeneralPose() == AuPose::ConsoleTurretTop)
		return AuPoint(0,0,1.5);		// Close enough.
	else if(IsEatingPose())
	{
		// Creatures need this adjustment due to bobbing heads while munching. If their origin is at their eye, their
		// body is constantly in motion, adjusting up and down to the terrain. Munching critters must cause no
		// server load. This offset produces a stationary location while they eat.
		// This induces an offset when changing to/from eating pose that is noticeable, as the body origin changes from its eye to
		// its pivot point and adjusts to the nu terrain altitude there. Directly above/below the eye location minimizes
		// this offset, but the body undulates back and forth wierdly as the creatures head bobs up and down. This looks really
		// strange if I am riding the creature when it starts eating.
		// I suppose the cg offset when eating/not eating is tolerable; it doesn't happen that often. The creatures kind of look
		// like they are pouncing on their food. :P
		// A better solution might be to actually change the origin of the creature by this offset when it changes to|from eating.
		return AuPoint(0,0,0);
	}
	*/


	// Can skip first body part because it's always the torso.
	if(count() > 1)
	{
		if(DNA().Heads && IsRightHanded())
		{
			// Two-headed right-handed creature sees with the second head, which is the right head when there are two.
			bool isLeft = true;	// first head is left
			for(QVector<DpBodyPart*>::const_iterator it = constBegin() + 1; it != constEnd(); it++)
				if((*it)->IsHeadAndNeck())
				{
					if(isLeft)
						isLeft = false;
					else
					{
						DpHeadAndNeck* head = (DpHeadAndNeck*)(*it);
						return head->CurrentHeadLoc();
					}
				}
		}
		else
		{
			// One-headed creature or left-handed creature sees with the first head, which is the left head when there are two.
			for(QVector<DpBodyPart*>::const_iterator it = constBegin() + 1; it != constEnd(); it++)
				if((*it)->IsHeadAndNeck())
				{
					DpHeadAndNeck* head = (DpHeadAndNeck*)(*it);
					return head->CurrentHeadLoc();
				}
		}
	}

	return AuPoint(0,0,0);	// Body pivot point location.
}

AuPoint DpBody::TargetLocation() const
{
	return HeadLocation();										// Head shots for everyone.
	/*
	if(CurrentGeneralPose() == AuPose::EyesOnly)
		return AuPoint(0,0,0);									// Head only so aim for center of head.

	// Target the head when there is no body|torso and 50% of the time otherwise.
	// This causes the AI to move their point of aim about the target, improving their chance to "see" body where they are looking.
	if(!m_DNA.TorsoVertebrae)// || g_Au.Dice().Roll(1,100) <= 50)
		return HeadLocation();									// No torso or 50% chance to aim at head.

	// Body sometimes bends at the waist to create an upper torso section.
	unsigned int waist = m_DNA.WaistVertex();
	if(waist)
		return m_Bones[DhDNA::BONE_Torso0].Project(AuPoint(0,0)) / 2;	// Aim for middle of torso upper body mass.

	// There is no upper torso above the waist.
	// Shoot at the lower torso section behind the waist.
	return m_Bones[m_DNA.TorsoVertebrae-1].Project(AuPoint(0,0)) / 2;	// Aim for middle of lower body mass.
	*/
}

AuPoint DpBody::CurrentLeftHandLoc() const
{
	// Can skip first body part because it's always the torso.
	if(count() > 1 && m_DNA.HasArms())
		for(QVector<DpBodyPart*>::const_iterator it = constBegin() + 1; it != constEnd(); it++)
			if((*it)->IsArmAndHand())
			{
				DpArmAndHand* arm = (DpArmAndHand*)(*it);
				if(arm->IsLeft() && !arm->IsWing())
					return arm->CurrentHandLoc();
			}

	return AuPoint(-TorsoWidth(),0,0);	// Body location.
}

AuPoint DpBody::CurrentRightHandLoc() const
{
	// Can skip first body part because it's always the torso.
	if(count() > 1 && m_DNA.HasArms())
		for(QVector<DpBodyPart*>::const_iterator it = constBegin() + 1; it != constEnd(); it++)
			if((*it)->IsArmAndHand())
			{
				DpArmAndHand* arm = (DpArmAndHand*)(*it);
				if(!arm->IsLeft() && !arm->IsWing())
					return arm->CurrentHandLoc();
			}

	return AuPoint(TorsoWidth(),0,0);	// Body location.
}

float DpBody::PhysicalHeight() const
{
	const float torsoThickness = TorsoThickness();				// Torso thickness from chest to back.
	const float cgHt = CGHeight(true);							// Height at waist/pivot point, based on my current pose.
	const float skullHt = HeadLocation().z + HeadThickness()/2;	// Height of the top of my head, relative to my CG.
	if(skullHt > torsoThickness)
	{
		// Top of skull is above meat of body at CG.
		return cgHt + skullHt;
	}
	else
	{
		// Top of skull is below meat of body at CG.
		return cgHt + torsoThickness;
	}
}

// Current height from floor to center of gravity based on how the front legs are posed.
float DpBody::CGHeight(bool toBottomOfFoot) const
{
	// Calculate preferred standing CG height.
	float ht = 0;	// When 0, final height will be body radius.

	// Adjust preferred height with pose height factor.
	switch(CurrentGeneralPose().Pose())
	{
	case AuPose::EyesOnly:
		return 0;
	case AuPose::Init:
		break;
	case AuPose::Beckon:
	case AuPose::Bite:
	case AuPose::Block:
	case AuPose::Eat:
	case AuPose::Hug:
	case AuPose::Kick:
	case AuPose::Lash:
	case AuPose::Point:
	case AuPose::Psionic:
	case AuPose::Stomp:
	case AuPose::Strike:
	case AuPose::Throw:
	case AuPose::Upright:
	case AuPose::Wag:
	case AuPose::Wave:
	case AuPose::MouthPursed:
	case AuPose::MouthClosed:
	case AuPose::MouthOpen:
	case AuPose::MouthAgape:
	case AuPose::MouthOpenWide:
	case AuPose::MouthYawn:
	case AuPose::MouthBite:
	case AuPose::MouthLaugh:
	case AuPose::MouthYap:
	case AuPose::MouthWhisper:
	case AuPose::MouthTalk:
	case AuPose::MouthJabber:
	case AuPose::MouthYell:
	case AuPose::MouthSmile:
	case AuPose::MouthGrin:
	case AuPose::MouthSad:
	case AuPose::MouthFrown:
	case AuPose::MouthCry:
	case AuPose::MouthPerplexed:
	case AuPose::MouthGrim:
	case AuPose::MouthSardonic:
		Q_ASSERT(false);	// Invalid lower torso pose.
		break;

	case AuPose::BerthBottomFaceDown:
	case AuPose::BerthBottomFaceLeft:
	case AuPose::BerthBottomFaceRight:
	case AuPose::BerthBottomFaceUp:
		return 1.5;

	case AuPose::BerthTopFaceDown:
	case AuPose::BerthTopFaceLeft:
	case AuPose::BerthTopFaceRight:
	case AuPose::BerthTopFaceUp:
		return 5;

	case AuPose::GameTableSeat0_0:
	case AuPose::GameTableSeat1_0:
	case AuPose::GameTableSeat2_0:
	case AuPose::GameTableSeat3_0:
	case AuPose::GameTableSeat0_45:
	case AuPose::GameTableSeat1_45:
	case AuPose::GameTableSeat2_45:
	case AuPose::GameTableSeat3_45:
	case AuPose::ConsoleCommand:
	case AuPose::ConsoleNav:
	case AuPose::ConsoleEngine:
	case AuPose::ConsoleFireControl:
	case AuPose::ConsoleSurgery:
	case AuPose::ConsolePilot:
	case AuPose::ConsolePower:
	case AuPose::ConsoleSensor:
	case AuPose::ConsoleShield:
	case AuPose::ConsoleTransporter:
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::ConsoleTurretBottom:
	case AuPose::ConsoleTurretTop:
	case AuPose::Command_Sit:
	case AuPose::Command_SitStationary:
	case AuPose::Engine_Sit:
	case AuPose::Engine_SitStationary:
	case AuPose::FireControl_Sit:
	case AuPose::FireControl_SitStationary:
	case AuPose::Helm_Sit:
	case AuPose::Helm_SitStationary:
	case AuPose::Medic_Sit:
	case AuPose::Medic_SitStationary:
	case AuPose::Nav_Sit:
	case AuPose::Nav_SitStationary:
	case AuPose::Patient_Sit:
	case AuPose::Patient_SitStationary:
	case AuPose::Post_Sit:
	case AuPose::Power_Sit:
	case AuPose::Power_SitStationary:
	case AuPose::Sensor_Sit:
	case AuPose::Sensor_SitStationary:
	case AuPose::Shield_Sit:
	case AuPose::Shield_SitStationary:
	case AuPose::Transporter_Sit:
	case AuPose::Transporter_SitStationary:
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
		return AuStation::SeatHeight();
	case AuPose::Crouch:
	case AuPose::CrouchCreep:
		if(DNA().BodyCarriage == 0 || DNA().BodyCarriage == 1)
			ht = 3.f/2.f * CGHeightStanding(toBottomOfFoot);
		else
			ht = 2.f/3.f * CGHeightStanding(toBottomOfFoot);
		break;
	case AuPose::DeadFaceDown:
	case AuPose::DeadFaceUp:
	case AuPose::Fly:
	case AuPose::FlyFast:
	case AuPose::Glide:
	case AuPose::Hover:
	case AuPose::Prone:
	case AuPose::ProneCreep:
	case AuPose::SitOnGround:
	case AuPose::SitOnGroundCreep:
	case AuPose::Swim:
	case AuPose::SwimFast:
	case AuPose::WritheFaceDown:
	case AuPose::WritheFaceUp:
	case AuPose::ReceptiveMate:
	case AuPose::MateOnBottom:
	case AuPose::Patient_Prone:
		ht = 0;
		break;
	case AuPose::Straddle:
	case AuPose::MateOnTop:
		if(TorsoThickness() > HeadThickness())
			ht = TorsoThickness()/2;
		else
			ht = HeadThickness()/2;
		break;
	case AuPose::DeadFaceLeft:
	case AuPose::DeadFaceRight:
	case AuPose::WritheFaceLeft:
	case AuPose::WritheFaceRight:
		ht = TorsoWidth()/2.f;
		return ht;
	case AuPose::ConsoleSurgeryUnit:
	case AuPose::Dig:
	case AuPose::Ladder:
	case AuPose::LadderClimb:
	case AuPose::Post_Stand:
	case AuPose::Spawn:
	case AuPose::Stand:
	case AuPose::StandRun:
	case AuPose::StandWalk:
	case AuPose::TreadWater:
	case AuPose::Command_Stand:
	case AuPose::Command_StandStationary:
	case AuPose::Engine_Stand:
	case AuPose::Engine_StandStationary:
	case AuPose::FireControl_Stand:
	case AuPose::FireControl_StandStationary:
	case AuPose::Helm_Stand:
	case AuPose::Helm_StandStationary:
	case AuPose::Medic_Stand:
	case AuPose::Medic_StandStationary:
	case AuPose::Nav_Stand:
	case AuPose::Nav_StandStationary:
	case AuPose::Patient_Stand:
	case AuPose::Patient_StandStationary:
	case AuPose::Power_Stand:
	case AuPose::Power_StandStationary:
	case AuPose::Sensor_Stand:
	case AuPose::Sensor_StandStationary:
	case AuPose::Shield_Stand:
	case AuPose::Shield_StandStationary:
	case AuPose::Transporter_Stand:
	case AuPose::Transporter_StandStationary:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
		ht = CGHeightStanding(toBottomOfFoot);
		break;
	case AuPose::Jump:
		ht = CGHeightStanding(toBottomOfFoot) / 2.f;
		break;
	}

	// Must be at least a body radius above the ground.
	float bodyrad = TorsoThickness()/2.f;
	if(ht < bodyrad)
		ht = bodyrad;

	//qDebug("CGHeight %g", ht);
	return ht;
}

// Height to center of gravity based on legs and preferred body carriage.
float DpBody::CGHeightStanding(bool toBottomOfFoot) const
{
	// Calculate preferred standing CG height.
	float ht = 0;	// When 0, final height will be body radius.
	if(m_DNA.LegPairs)
	{
		// Legs and body carriage determine desired CG height.
		const float thighlen = ThighLength();
		const float calflen = CalfLength();
		const float footlen = FootLength();
		//qDebug("CGHeightStanding thigh %g calf %g foot %g carr %d bodyrad %g",
				//thighlen, calflen, footlen, m_DNA.BodyCarriage, TorsoThickness());
		switch(m_DNA.BodyCarriage)
		{
		case 0:
			if(m_DNA.HasAnkleBend())
			{
				ht = (thighlen + calflen) * 0.10f + footlen;
				if(toBottomOfFoot)
					ht += FootHeight();	// Add the thickness of the foot.
			}
			else
				ht = (thighlen + calflen + footlen) * 0.10f;
			break;
		case 1:
			if(m_DNA.HasAnkleBend())
			{
				ht = (thighlen + calflen) * 0.40f + footlen;
				if(toBottomOfFoot)
					ht += FootHeight();	// Add the thickness of the foot.
			}
			else
				ht = (thighlen + calflen + footlen) * 0.40f;
			break;
		case 2:
			if(m_DNA.HasAnkleBend())
			{
				ht = (thighlen + calflen) * 0.60f;
				if(toBottomOfFoot)
					ht += FootHeight();	// Add the thickness of the foot.
			}
			else
				ht = (thighlen + calflen + footlen) * 0.60f;
			break;
		case 3:
			if(m_DNA.HasAnkleBend())
			{
				ht = (thighlen + calflen) * 0.99f;
				if(toBottomOfFoot)
					ht += FootHeight();	// Add the thickness of the foot.
			}
			else
				ht = (thighlen + calflen + footlen) * 0.90f;
			break;
		}

		// Must be at least a body radius above the ground.
		float bodyrad = TorsoThickness()/2.f;
		if(m_DNA.HasAnkleBend() && toBottomOfFoot)
			bodyrad -= FootHeight();	// Subtract the thickness of the foot for proper result.
		if(ht < bodyrad)
			ht = bodyrad;
	}
	else
	{
		// Must be at least a body radius above the ground.
		float bodyrad = TorsoThickness()/2.f;
		if(ht < bodyrad)
			ht = bodyrad;
	}

	return ht;
}

void DpBody::SelectTextures(AuGLEffects &re)
{
	// Select skin/clothes texture
	QString sSkinType = "";
	if(m_DNA.SkinType == DhDNA::SKIN_DrySkin || m_DNA.SkinType == DhDNA::SKIN_WetSkin)
	{
		sSkinType = "Skin";
		if(!m_glTexSkin[0])
			m_glTexSkin[0] = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, QString("DpBody::%1").arg(sSkinType), QString("tex/%1.png").arg(sSkinType));
		if(!m_glTexClothes[0])
			m_glTexClothes[0] = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, QString("DpBody::%1Clothes").arg(sSkinType), QString("tex/%1Clothes.png").arg(sSkinType));
	}
	else if(m_DNA.SkinType == DhDNA::SKIN_DryExoskeleton || m_DNA.SkinType == DhDNA::SKIN_WetExoskeleton)
	{
		sSkinType = "Exo";
		if(!m_glTexSkin[1])
			m_glTexSkin[1] = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, QString("DpBody::%1").arg(sSkinType), QString("tex/%1.png").arg(sSkinType));
		if(!m_glTexClothes[1])
			m_glTexClothes[1] = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, QString("DpBody::%1Clothes").arg(sSkinType), QString("tex/%1Clothes.png").arg(sSkinType));
	}
	else if(m_DNA.SkinType == DhDNA::SKIN_DryScales || m_DNA.SkinType == DhDNA::SKIN_WetScales)
	{
		sSkinType = "Scales";
		if(!m_glTexSkin[2])
			m_glTexSkin[2] = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, QString("DpBody::%1").arg(sSkinType), QString("tex/%1.png").arg(sSkinType));
		if(!m_glTexClothes[2])
			m_glTexClothes[2] = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, QString("DpBody::%1Clothes").arg(sSkinType), QString("tex/%1Clothes.png").arg(sSkinType));
	}
	else if(m_DNA.SkinType == DhDNA::SKIN_Fur || m_DNA.SkinType == DhDNA::SKIN_Feathers)
	{
		sSkinType = "FurFeathers";
		if(!m_glTexSkin[3])
			m_glTexSkin[3] = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, QString("DpBody::%1").arg(sSkinType), QString("tex/%1.png").arg(sSkinType));
		if(!m_glTexClothes[3])
			m_glTexClothes[3] = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, QString("DpBody::%1Clothes").arg(sSkinType), QString("tex/%1Clothes.png").arg(sSkinType));
	}

	// Select wing texture
	if(!m_glTexWings)
		m_glTexWings = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "DpBody::Wings", "tex/Wings.png");

	// Select armor textures
	if(!m_glTexArmor[0])
		m_glTexArmor[0] = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, QString("DpBody::VacSuit"), QString("tex/VacSuit.png"));
	if(!m_glTexArmor[1])
		m_glTexArmor[1] = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, QString("DpBody::PAMetal"), QString("tex/PAMetal.png"));
	if(!m_glTexArmor[2])
		m_glTexArmor[2] = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, QString("DpBody::PAMagmium"), QString("tex/PAMagmium.png"));
	if(!m_glTexArmor[3])
		m_glTexArmor[3] = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, QString("DpBody::PAVulcium"), QString("tex/PAVulcium.png"));
	
	// Select helmet texture
	int iType = 0;
	if(m_eHelmetClothesType == CID_MetalPowerArmorHelmet)
		iType = 0;
	else if(m_eHelmetClothesType == CID_MagmiumPowerArmorHelmet)
		iType = 1;
	else if(m_eHelmetClothesType == CID_VulciumPowerArmorHelmet)
		iType = 2;
	else if(m_eHelmetClothesType == CID_VacSuitHelmet)
		iType = 3;
	if(!m_glTexHelmet[m_DNA.SkinType][iType] && HasHelmetPowerArmorOn())
	{
		if(iType == 3)	// Vac Suit Helmet
			m_glTexHelmet[m_DNA.SkinType][iType] = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID,
				QString("Vac%1Helmet.png").arg(m_DNA.SkinType),
				QString("tex/Vac%1Helmet.png").arg(m_DNA.SkinType));
		else
			m_glTexHelmet[m_DNA.SkinType][iType] = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID,
				QString("PA%1Helmet%2.png").arg(m_DNA.SkinType).arg(iType),
				QString("tex/PA%1Helmet%2.png").arg(m_DNA.SkinType).arg(iType));
	}
}

void DpBody::CreateDeviceObjects(AuGLEffects& re, AuGLModelCache& mc, COMMODITYID clothing, COMMODITYID helmet, float speedEnc)
{
	// Clear my model cache if my model changes.
	if(m_eBodyClothesType != clothing || m_eHelmetClothesType != helmet)
		m_Models.clear();

	// Create my model cache when it is empty.
	if(m_Models.isEmpty())
	{
		// Configure my clothing and helmet.
		//qDebug("DpBody::CreateDeviceObjects %d %s", id.GetID(), (const char*)m_DNA.Description().toAscii());
		SetClothesType(clothing);
		SetHelmetType(helmet);
		DeleteBodyParts();
		Init();
		AnimatePose(false, speedEnc);		// Pose nu body parts to current pose.
		CreateSkin(re, mc);								// Create the skin model in this thread.
		//qDebug("DpBody::CreateDeviceObjects %d %s done %d model pieces", id.GetID(), (const char*)m_DNA.Description().toAscii(),
				//m_Models.count());
	}
}

void DpBody::DeleteDeviceObjects(AuGLEffects& /*re*/)
{
// 	if(m_glModelBody)
// 	{
// 		//qDebug("DpBody::DeleteDeviceObjects - %d %s",
// 				//id.GetID(), (const char*)DNA().SpotDescription().toAscii());
// 		// This user no longer needs the cached display list model.
// 		// Remove them from the list
// 		Au3dsGLSLDNA dna;
// 		m_DNA.CreateModelDNA(dna);
// 
// 		// Need to come back to this functionality at some point because right now
// 		// its not deleting the users from the model cache.
// 		// The function below is suppose to do the following:
// 		// 1 - Remove user from model cache and delete the model if its the last user
// 		// 2 - Remove user from DNA configuration and delete the key if its the last one
// 		// 3 - Delete the display list if this is the last user using this DNA configuration.
// 		re.RemoveUserFromModelDNA(id.GetID(), dna);
// 		m_glModelBody = 0;
// 	}

	// Delete textures
// 	if(bModels)
// 	{
// 		re.DeleteTextures(id.GetID());
// 		for(int iSkin=0; iSkin<SkinTexLim; iSkin++)
// 		{
// 			m_glTexSkin[iSkin] = 0;
// 			m_glTexClothes[iSkin] = 0;
// 		}
// 		m_glTexWings = 0;
// 		for(int iArmor=0; iArmor<ArmorTexLim; iArmor++)
// 			m_glTexArmor[iArmor] = 0;
// 		for(int iHelmet=0; iHelmet<HelmetTexLim; iHelmet++)
// 		{
// 			for(int iType=0; iType<ArmorTexLim; iType++)
// 				m_glTexHelmet[iHelmet][iType] = 0;
// 		}
// 	}

	// Only the DNA configuration similar to this DpBody that first created
	// the model cache structure will have a list.  We only want to clear the list
	// if they are the one being deleted here, because others may still be using the
	// models.
	if(!m_Models.isEmpty())
	{
		//if(bModels)
		//{
		//	QVector<Au3dsGLSLModelInstance*>::iterator itModels;
		//	for(itModels = m_Models.begin(); itModels != m_Models.end(); itModels++)
		//	{
		//		mc.DeleteModel((*itModels));
		//		(*itModels) = 0;
		//	}
		//}
		m_Models.clear();
	}
}

void DpBody::SetupSkinMaterial()
{
	if(HasBodyPowerArmorOn())
	{
		m_TopColor[1] = QColor(255, 255, 255, 0);
		m_BottomColor[1] = QColor(255, 255 ,255, 0);
		m_TopShininess[1] = 0.;
		m_BottomShininess[1] = 0.;
		m_Glow[1] = 0.;
	}
	else
	{
		// Calculate color intensity based on skin thickness.
		int dim;
		dim = 255 - 255*m_DNA.TopSkinThickness/14;
		m_TopColor[0] = QColor(dim, dim, dim, 0);
		dim = 255 - 255*m_DNA.BottomSkinThickness/14;
		m_BottomColor[0] = QColor(dim, dim ,dim, 0);
		// Calculate shininess based on skin type.
		m_TopShininess[0] = 0.;
		m_BottomShininess[0] = 0.;
		switch(m_DNA.SkinType)
		{
		case DhDNA::SKIN_DryExoskeleton:	// Ant.
			m_TopShininess[0] = 0;
			m_BottomShininess[0] = 0;
			break;
		case DhDNA::SKIN_DryScales:		// Lizard.
			m_TopShininess[0] = 1;
			m_BottomShininess[0] = 1;
			break;
		case DhDNA::SKIN_DrySkin:		// Pig, human.
			m_TopShininess[0] = 300;
			m_BottomShininess[0] = 300;
			break;
		case DhDNA::SKIN_Feathers:		// Sparrow, ostrich, chicken.
			m_TopShininess[0] = 4;
			m_BottomShininess[0] = 4;
			break;
		case DhDNA::SKIN_Fur:			// Covered with fur. Beaver, horse.
			m_TopShininess[0] = 5;
			m_BottomShininess[0] = 5;
			break;
		case DhDNA::SKIN_WetExoskeleton:	// Black widow, millipede.
			m_TopShininess[0] = 200;
			m_BottomShininess[0] = 200;
			break;
		case DhDNA::SKIN_WetScales:		// Fish.
			m_TopShininess[0] = 40;
			m_BottomShininess[0] = 40;
			break;
		case DhDNA::SKIN_WetSkin:		// Salamander, kardassian
			m_TopShininess[0] = 20;
			m_BottomShininess[0] = 20;
			break;
		}
		// Animals with a radioactive attack will glow green.
		m_Glow[0] = 0;
		if(m_DNA.WeaponType == DhDNA::WT_Radiation)
			m_Glow[0] = ((float)m_DNA.WeaponSize + 1) * .1;
	}
	m_Glowdim[0] = m_Glow[0] * .05f/.3f;

	// Texture coordinates offset
	float textureSize = 4096.0;
	m_vTOffset = 0;
	m_vBOffset = 0;
	if(m_DNA.TopSkinSubtype <= 3)
		m_uTOffset = m_DNA.TopSkinSubtype*(textureSize/4.0);
	else
	{
		m_uTOffset = (m_DNA.TopSkinSubtype-4)*(textureSize/4.0);
		m_vTOffset = textureSize/4.0;
	}
	if(m_DNA.BottomSkinSubtype <= 3)
		m_uBOffset = m_DNA.BottomSkinSubtype*(textureSize/4.0);
	else
	{
		m_uBOffset = (m_DNA.BottomSkinSubtype-4)*(textureSize/4.0);
		m_vBOffset = textureSize/4.0;
	}
	m_uTOffset = m_uTOffset/textureSize;
	m_uBOffset = m_uBOffset/textureSize;
	if(m_vTOffset > 0)
		m_vTOffset = 1-m_vTOffset/textureSize;
	if(m_vBOffset > 0)
		m_vBOffset = 1-m_vBOffset/textureSize;
	m_skinOffset[0] = m_uTOffset;
	m_skinOffset[1] = m_vTOffset;
	m_skinOffset[2] = m_uBOffset;
	m_skinOffset[3] = m_vBOffset;
}

QString DpBody::GetBodyModelPrefix() const
{
	if(HasBodyPowerArmorOn())
		return ":data/PA";
	
	return QString(":data/%1").arg(m_DNA.SkinType);
}

QString DpBody::GetHelmetModelPrefix() const
{
	if(HasHelmetPowerArmorOn())
		return ":data/PA";

	return QString(":data/%1").arg(m_DNA.SkinType);
}

bool DpBody::HasBodyPowerArmorOn() const
{
	return DbCommodity::IsEnvironmentSuit(m_eBodyClothesType);
}

bool DpBody::HasHelmetPowerArmorOn() const
{
	return DbCommodity::IsEnvironmentHelmet(m_eHelmetClothesType);
}

float DpBody::FootLength(bool bHp) const
{
	Au3dsGLSLDNA dna;
	m_DNA.CreateModelDNA(dna);
	if(HasBodyPowerArmorOn() && !bHp)
		dna.ModelType = 1;
	AuPoint footLength = g_Au.FootOffset(dna);
	if(footLength != AuPoint(0, 0, 0))
		return footLength.y;

	// Get values that must come from the 3ds model.
	AuPoint pMin, pMax, pConnect, pFootScalePt;
	QString sPrefix = GetBodyModelPrefix();
	QString footModel;
	if(HasBodyPowerArmorOn())
		footModel = QString("%1FootRight%2.3dsN").arg(sPrefix).arg(m_DNA.HasAnkleBend());
	else
		footModel = QString("%1FootRight%2.3dsN").arg(sPrefix).arg(m_DNA.FootType);
	AuGLModelCache& models = g_Au.LockGLModelsForWrite();
	const Au3dsGLSLModelInstance* modelInstance = dynamic_cast<const Au3dsGLSLModelInstance*>(models.CreateGeometry(footModel));
	if(modelInstance)
	{
		modelInstance->GetExtents(pMin,pMax);
		pConnect = modelInstance->GetConnectPos();
		pFootScalePt = modelInstance->GetFootScalePos();
	}
	g_Au.UnlockGLModels();

	AuPoint pOffset;
	if(modelInstance)
	{
		float fOverScale = OverallScale();
		pConnect *= fOverScale;
		pMin *= fOverScale;
		pMax *= fOverScale;
		float lLength = m_DNA.Length(m_DNA.CalfLength);
		float lThick = m_DNA.Thickness(m_DNA.CalfThickness);
		lThick = lThick/.5*(lLength/1.5);
		// Change lLength to foot length
		lLength = m_DNA.Length(m_DNA.FootLength)/4;
		lLength /= .5;
		// Overall scaling
		if(lThick != 1)		// This is calf thickness
		{
			pMin = pMin-pFootScalePt;	// Move to scale location
			pMin = AuPoint(pMin.x*lThick, pMin.y*lThick,
				pMin.z*lThick) + pFootScalePt;
			pMax = pMax-pFootScalePt;	// Move to scale location
			pMax = AuPoint(pMax.x*lThick, pMax.y*lThick,
				pMax.z*lThick) + pFootScalePt;

			pConnect = pConnect-pFootScalePt;	// Move to scale location
			pConnect = AuPoint(pConnect.x*lThick, pConnect.y*lThick,
				pConnect.z*lThick) + pFootScalePt;
		}

		// Y - scaling
		if(lLength != 1)			// This is foot length
		{
			pMin = pMin-pFootScalePt;	// Move to scale location
			pMin = AuPoint(pMin.x, pMin.y*lLength, pMin.z) + pFootScalePt;
			pMax = pMax-pFootScalePt;	// Move to scale location
			pMax = AuPoint(pMax.x, pMax.y*lLength, pMax.z) + pFootScalePt;

			pConnect = pConnect-pFootScalePt;	// Move to scale location
			pConnect = AuPoint(pConnect.x, pConnect.y*lLength, pConnect.z) + pFootScalePt;
		}

		pOffset = AuPoint(0, pMax.y - pConnect.y, pConnect.z - pMin.z);
		g_Au.SetFootOffset(dna, pOffset);
	}
	return pOffset.y;
}

float DpBody::FootHeight() const
{
	Au3dsGLSLDNA dna;
	m_DNA.CreateModelDNA(dna);
	if(HasBodyPowerArmorOn())
		dna.ModelType = 1;
	AuPoint footHeight = g_Au.FootOffset(dna);
	if(footHeight != AuPoint(0, 0, 0))
		return footHeight.z;

	// Get values that must come from the 3ds model.
	AuPoint pMin, pMax, pConnect, pFootScalePt;
	QString sPrefix = GetBodyModelPrefix();
	QString footModel;
	if(HasBodyPowerArmorOn())
		footModel = QString("%1FootRight%2.3dsN").arg(sPrefix).arg(m_DNA.HasAnkleBend());
	else
		footModel = QString("%1FootRight%2.3dsN").arg(sPrefix).arg(m_DNA.FootType);
	AuGLModelCache& models = g_Au.LockGLModelsForWrite();
	const Au3dsGLSLModelInstance* modelInstance = dynamic_cast<const Au3dsGLSLModelInstance*>(models.CreateGeometry(footModel));
	if(modelInstance)
	{
		modelInstance->GetExtents(pMin,pMax);
		pConnect = modelInstance->GetConnectPos();
		pFootScalePt = modelInstance->GetFootScalePos();
	}
	g_Au.UnlockGLModels();

	AuPoint pOffset;
	if(modelInstance)
	{
		float fOverScale = OverallScale();
		pConnect *= fOverScale;
		pMin *= fOverScale;
		pMax *= fOverScale;
		float lLength = m_DNA.Length(m_DNA.CalfLength);
		float lThick = m_DNA.Thickness(m_DNA.CalfThickness);
		lThick = lThick/.5*(lLength/1.5);
		// Change lLength to foot length
		lLength = m_DNA.Length(m_DNA.FootLength)/4;
		lLength /= .5;
		// Overall scaling
		if(lThick != 1)		// This is calf thickness
		{
			pMin = pMin-pFootScalePt;	// Move to scale location
			pMin = AuPoint(pMin.x*lThick, pMin.y*lThick,
				pMin.z*lThick) + pFootScalePt;
			pMax = pMax-pFootScalePt;	// Move to scale location
			pMax = AuPoint(pMax.x*lThick, pMax.y*lThick,
				pMax.z*lThick) + pFootScalePt;

			pConnect = pConnect-pFootScalePt;	// Move to scale location
			pConnect = AuPoint(pConnect.x*lThick, pConnect.y*lThick,
				pConnect.z*lThick) + pFootScalePt;
		}

		// Y - scaling
		if(lLength != 1)			// This is foot length
		{
			pMin = pMin-pFootScalePt;	// Move to scale location
			pMin = AuPoint(pMin.x, pMin.y*lLength, pMin.z) + pFootScalePt;
			pMax = pMax-pFootScalePt;	// Move to scale location
			pMax = AuPoint(pMax.x, pMax.y*lLength, pMax.z) + pFootScalePt;

			pConnect = pConnect-pFootScalePt;	// Move to scale location
			pConnect = AuPoint(pConnect.x, pConnect.y*lLength, pConnect.z) + pFootScalePt;
		}

		pOffset = AuPoint(0, pMax.y - pConnect.y, pConnect.z - pMin.z);
		g_Au.SetFootOffset(dna, pOffset);
	}
	return pOffset.z;
}

float DpBody::HandLength(bool bHp) const
{
	Au3dsGLSLDNA dna;
	m_DNA.CreateModelDNA(dna);
	if(HasBodyPowerArmorOn() && !bHp)
		dna.ModelType = 1;
	AuPoint handLength = g_Au.HandOffset(dna);
	if(handLength != AuPoint(0, 0, 0))
		return handLength.y;

	// Get values that must come from the 3ds model.
	AuPoint pMin, pMax, pConnect, pHandScalePt;
	QString sPrefix = GetBodyModelPrefix();
	QString handModel;
	if(HasBodyPowerArmorOn())
		handModel = QString("%1HandRight.3dsN").arg(sPrefix);
	else
		handModel = QString("%1HandRight%2.3dsN").arg(sPrefix).arg(m_DNA.HandType);
	AuGLModelCache& models = g_Au.LockGLModelsForWrite();
	const Au3dsGLSLModelInstance* modelInstance = dynamic_cast<const Au3dsGLSLModelInstance*>(models.CreateGeometry(handModel));
	if(modelInstance)
	{
		modelInstance->GetExtents(pMin,pMax);
		pConnect = modelInstance->GetConnectPos();
		pHandScalePt = modelInstance->GetHandScalePos();
	}
	g_Au.UnlockGLModels();

	AuPoint pOffset;
	if(modelInstance)
	{
		float fOverScale = OverallScale();
		pConnect *= fOverScale;
		pMin *= fOverScale;
		pMax *= fOverScale;
		float lLength = m_DNA.Length(m_DNA.ForeArmLength);
		float lThick = m_DNA.Thickness(m_DNA.ForeArmThickness);
		lThick = lThick/.5*(lLength/1.5);
		// Change lLength to hand length
		lLength = m_DNA.Length(m_DNA.HandLength)/4;
		lLength /= .5;
		// Overall scaling
		if(lThick != 1)		// This is forearm thickness
		{
			pMin = pMin-pHandScalePt;	// Move to scale location
			pMin = AuPoint(pMin.x*lThick, pMin.y*lThick,
				pMin.z*lThick) + pHandScalePt;
			pMax = pMax-pHandScalePt;	// Move to scale location
			pMax = AuPoint(pMax.x*lThick, pMax.y*lThick,
				pMax.z*lThick) + pHandScalePt;

			pConnect = pConnect-pHandScalePt;	// Move to scale location
			pConnect = AuPoint(pConnect.x*lThick, pConnect.y*lThick,
				pConnect.z*lThick) + pHandScalePt;
		}

		// Y - scaling
		if(lLength != 1)			// This is hand length
		{
			pMin = pMin-pHandScalePt;	// Move to scale location
			pMin = AuPoint(pMin.x, pMin.y*lLength, pMin.z) + pHandScalePt;
			pMax = pMax-pHandScalePt;	// Move to scale location
			pMax = AuPoint(pMax.x, pMax.y*lLength, pMax.z) + pHandScalePt;

			pConnect = pConnect-pHandScalePt;	// Move to scale location
			pConnect = AuPoint(pConnect.x, pConnect.y*lLength, pConnect.z) + pHandScalePt;
		}

		pOffset = AuPoint(0, pMax.y - pConnect.y, pConnect.z-pMin.z);
		g_Au.SetHandOffset(dna, pOffset);
	}
	return pOffset.y;
}

// Front to Back
float DpBody::HeadLength() const
{
	float hret = TorsoThickness();
	float hwid = TorsoWidth();

	// Use LARGER of torso thickness or width.
	if(hret < hwid)
		hret = hwid;

	// Head shrinks with length of neck.
	float frac[5] = {1.0f, 0.8f, 0.7f, 0.6f, 0.5f};
	hret *= frac[m_DNA.NeckVertebraeCount()];
	hret *= 1.25f;

	return hret;
}

// Top to Bottom
float DpBody::HeadThickness() const
{
	float hret = TorsoThickness();
	if(!m_DNA.LivesInWater && !m_DNA.LivesInAir && !m_DNA.LivesUnderGround)
	{
		// Land dweller.
		// Use SMALLER of torso thickness or width.
		float hwid = TorsoWidth();
		if(hret > hwid)
			hret = hwid;
	}

	// Head shrinks with length of neck.
	float frac[5] = {1.0f, 0.9f, 0.8f, 0.7f, 0.6f};
	hret *= frac[m_DNA.NeckVertebraeCount()];

	return hret;
}

// Left to right
float DpBody::HeadWidth() const
{
	float hret = TorsoWidth();
	if(!m_DNA.LivesInWater && !m_DNA.LivesInAir && !m_DNA.LivesUnderGround)
	{
		// Land dweller.
		// Use SMALLER of torso thickness or width.
		float hthk = TorsoThickness();
		if(hret > hthk)
			hret = hthk;
	}

	// Head shrinks with length of neck.
	float frac[5] = {1.0f, 0.9f, 0.8f, 0.7f, 0.6f};
	hret *= frac[m_DNA.NeckVertebraeCount()];

	return hret;
}

void DpBody::Init()
{
	Q_ASSERT(isEmpty());

	// Create the torso first.
	// There is always a torso, even if it has no vertebrae.
	append(new DpTorso(this));

	// Create the tail.
	if(m_DNA.TailType != DhDNA::TAIL_None)
		append(new DpTail(this));

	// Neck/head combinations, one for each head, up to two.
	for(unsigned int ih=0; ih<=m_DNA.Heads; ih++)
		append(new DpHeadAndNeck(this, ih));

	// Append arm pairs.
	if(m_DNA.ArmPairs)
	{
		for(unsigned int ia=0; ia<m_DNA.ArmPairs; ia++)
		{
			DpLeftArmAndHand* lah = new DpLeftArmAndHand(this, ia);
			DpRightArmAndHand* rah = new DpRightArmAndHand(this, ia);

			// When a creature has multiple sets of wings, turn all but the last
			// set into arms.
			// This will result in only ever having one set of wings and the
			// rest will be arms.
			// The arms will be distributed nicely among the available vertices.
			// Wings are always placed at the first arm vertex, sharing the
			// vertex with any arms that are already there.
			if(m_DNA.ArmPairs > 1 && m_DNA.HasWings() &&
					ia < (unsigned int)(m_DNA.ArmPairs-1))
			{
				lah->ChangeArmType(DhDNA::ARM_Limb);
				rah->ChangeArmType(DhDNA::ARM_Limb);
			}

			append(lah);
			append(rah);
		}
	}

	// Append leg pairs.
	if(m_DNA.LegPairs)
	{
		for(unsigned int ia=0; ia<m_DNA.LegPairs; ia++)
		{
			append(new DpLeftLegAndFoot(this, ia));
			append(new DpRightLegAndFoot(this, ia));
		}
	}

	// Initialize all bone matrices to identity, whether or not we're using
	// them.
	InitAllBoneMatrices();

	// Setup variables that only need to be calculated once.
	OneTimeCalcs();
	for(QVector<DpBodyPart*>::iterator it = begin(); it != end(); it++)
		(*it)->OneTimeCalcs();
}

// Initialize to identity only the bone matrices that this body uses.
// This places the body flat on the ground facing the ground with arms and legs
// extending up the +Y axis from their attachment joint. Tail is straight down
// the -Y axis and necks are straight up the +Y axis with heads at the ends.
void DpBody::InitBoneMatrices()
{
	// Torso
	for(unsigned int ib=0; ib<m_DNA.TorsoVertebrae; ib++)
		m_Bones[DhDNA::BONE_Torso0 + ib].LoadIdentity();

	// Head(s) and neck(s)
	const int nvc = m_DNA.NeckVertebraeCount();
	m_Bones[DhDNA::BONE_Head0].LoadIdentity();
	m_Bones[DhDNA::BONE_Jaw0].LoadIdentity();
	for(int ib=0; ib<nvc; ib++)
		m_Bones[DhDNA::BONE_Neck00 + ib].LoadIdentity();
	if(m_DNA.Heads)
	{
		m_Bones[DhDNA::BONE_Head1].LoadIdentity();
		m_Bones[DhDNA::BONE_Jaw1].LoadIdentity();
		for(int ib=0; ib<nvc; ib++)
			m_Bones[DhDNA::BONE_Neck10 + ib].LoadIdentity();
	}

	// Tail
	int tailverts = (unsigned int)m_DNA.TailVertebrae*2;	// Tail
	if(tailverts)
		tailverts--;
	for(int ib=0; ib<=tailverts; ib++)
		m_Bones[DhDNA::BONE_Tail0 + ib].LoadIdentity();

	// Arms
	if(m_DNA.ArmPairs)
	{
		m_Bones[DhDNA::BONE_Hand0L].LoadIdentity();
		m_Bones[DhDNA::BONE_Hand0R].LoadIdentity();
		m_Bones[DhDNA::BONE_ForeArm0L].LoadIdentity();
		m_Bones[DhDNA::BONE_ForeArm0R].LoadIdentity();
		m_Bones[DhDNA::BONE_UpperArm0L].LoadIdentity();
		m_Bones[DhDNA::BONE_UpperArm0R].LoadIdentity();
	}
	if(m_DNA.ArmPairs >= 2)
	{
		m_Bones[DhDNA::BONE_Hand1L].LoadIdentity();
		m_Bones[DhDNA::BONE_Hand1R].LoadIdentity();
		m_Bones[DhDNA::BONE_ForeArm1L].LoadIdentity();
		m_Bones[DhDNA::BONE_ForeArm1R].LoadIdentity();
		m_Bones[DhDNA::BONE_UpperArm1L].LoadIdentity();
		m_Bones[DhDNA::BONE_UpperArm1R].LoadIdentity();
	}
	if(m_DNA.ArmPairs == 3)
	{
		m_Bones[DhDNA::BONE_Hand2L].LoadIdentity();
		m_Bones[DhDNA::BONE_Hand2R].LoadIdentity();
		m_Bones[DhDNA::BONE_ForeArm2L].LoadIdentity();
		m_Bones[DhDNA::BONE_ForeArm2R].LoadIdentity();
		m_Bones[DhDNA::BONE_UpperArm2L].LoadIdentity();
		m_Bones[DhDNA::BONE_UpperArm2R].LoadIdentity();
	}

	// Legs
	if(m_DNA.LegPairs)
	{
		m_Bones[DhDNA::BONE_Foot0L].LoadIdentity();
		m_Bones[DhDNA::BONE_Foot0R].LoadIdentity();
		m_Bones[DhDNA::BONE_Calf0L].LoadIdentity();
		m_Bones[DhDNA::BONE_Calf0R].LoadIdentity();
		m_Bones[DhDNA::BONE_Thigh0L].LoadIdentity();
		m_Bones[DhDNA::BONE_Thigh0R].LoadIdentity();
	}
	if(m_DNA.LegPairs >= 2)
	{
		m_Bones[DhDNA::BONE_Foot1L].LoadIdentity();
		m_Bones[DhDNA::BONE_Foot1R].LoadIdentity();
		m_Bones[DhDNA::BONE_Calf1L].LoadIdentity();
		m_Bones[DhDNA::BONE_Calf1R].LoadIdentity();
		m_Bones[DhDNA::BONE_Thigh1L].LoadIdentity();
		m_Bones[DhDNA::BONE_Thigh1R].LoadIdentity();
	}
	if(m_DNA.LegPairs == 3)
	{
		m_Bones[DhDNA::BONE_Foot2L].LoadIdentity();
		m_Bones[DhDNA::BONE_Foot2R].LoadIdentity();
		m_Bones[DhDNA::BONE_Calf2L].LoadIdentity();
		m_Bones[DhDNA::BONE_Calf2R].LoadIdentity();
		m_Bones[DhDNA::BONE_Thigh2L].LoadIdentity();
		m_Bones[DhDNA::BONE_Thigh2R].LoadIdentity();
	}
}

// Init all bone matrices to identity.
void DpBody::InitAllBoneMatrices()
{
	for(int ib=0; ib<DhDNA::BONE_Max; ib++)
		m_Bones[ib].LoadIdentity();

	/*
#if !defined(QT_NO_DEBUG)
	for(int ib=0; ib<DhDNA::BONE_Max; ib++)
	{
		const AuMatrix4x4& bmat = m_Bones[ib];
		if(bmat[3] != 0 || bmat[7] != 0 || bmat[11] != 0 || bmat[15] != 1)
		{
			qDebug("DpBody::DpBody - Copied Bad Bone Matrix %d in %s", ib,
					(const char*)m_DNA.Description().toAscii());
			bmat.Dump(0);
			Q_ASSERT(false);
		}
	}
#endif
*/
}

void DpBody::OneTimeCalcs()
{
	// Calf Length
	float val = m_DNA.Length(m_DNA.CalfLength);
	val *= SizeFactor();				// Scale by overall size factor.
	m_CalfLength = val * 1.3;

	// Calf Thickness
	float len = CalfLength()*0.5f;		// Basis of thickness is limb length.
	val = m_DNA.Thickness(m_DNA.CalfThickness) * len;
	m_CalfThickness = val;

	// Forearm Length
	val = m_DNA.Length(m_DNA.ForeArmLength);
	val *= SizeFactor();				// Scale by overall size factor.
	m_ForeArmLength = val;

	// Forearm Thickness
	len = ForeArmLength()*0.5f;	// Basis of thickness is limb length.
	val = m_DNA.Thickness(m_DNA.ForeArmThickness) * len;
	m_ForeArmThickness = val;

	// Torso Segment Length
	val = m_DNA.Length(m_DNA.TorsoSegmentLength);
	val *= SizeFactor();				// Scale by overall size factor.
	m_TorsoSegmentLength = val;

	// Torso Length
	float vertlen = TorsoSegmentLength();
	unsigned int waist = m_DNA.WaistVertex();
	m_TorsoLength = (float)waist * vertlen;

	// Torso Thickness
	len = TorsoSegmentLength();			// Basis of thickness is torso length.
	val = m_DNA.TorsoThicknessCalc(len);
	// Squash body based on environment to which it is adapted.
	if(m_DNA.LivesOnLand && m_DNA.LegPairs < 2)
	{
		// Land dweller bodies are squat.
		val *= 0.75f;
	}
	m_TorsoThickness = val;

	// Torso Width
	len = TorsoSegmentLength();	// Basis of thickness is torso length.
	val = m_DNA.TorsoThicknessCalc(len);
	// Squash body based on environment to which it is adapted.
	if(m_DNA.LivesInWater)
	{
		// Water dweller bodies are narrow.
		val *= 0.75f;
	}
	m_TorsoWidth = val;

	// Head Length
	float hret = TorsoThickness();
	float hwid = TorsoWidth();
	if(!m_DNA.LivesInWater && !m_DNA.LivesInAir && !m_DNA.LivesUnderGround)
	{
		// Land dweller.
		// Use SMALLER of torso thickness or width.
		if(hret > hwid)
			hret = hwid;
	}
	else
	{
		// Swimmer, flier, or digger.
		// Use LARGER of torso thickness or width.
		if(hret < hwid)
			hret = hwid;
	}
	// Head shrinks with length of neck.
	float frac[5] = {1.0f, 0.8f, 0.7f, 0.6f, 0.5f};
	hret *= frac[m_DNA.NeckVertebraeCount()];
	m_HeadLength = hret;

	// Head Thickness
	hret = TorsoThickness();
	if(!m_DNA.LivesInWater && !m_DNA.LivesInAir && !m_DNA.LivesUnderGround)
	{
		// Land dweller.
		// Use SMALLER of torso thickness or width.
		float hwid = TorsoWidth();
		if(hret > hwid)
			hret = hwid;
	}
	// Head shrinks with length of neck.
	float frac2[5] = {1.0f, 0.9f, 0.8f, 0.7f, 0.6f};
	hret *= frac2[m_DNA.NeckVertebraeCount()];
	m_HeadThickness = hret;

	// Head Width
	hret = TorsoWidth();
	if(!m_DNA.LivesInWater && !m_DNA.LivesInAir && !m_DNA.LivesUnderGround)
	{
		// Land dweller.
		// Use SMALLER of torso thickness or width.
		float hthk = TorsoThickness();
		if(hret > hthk)
			hret = hthk;
	}
	// Head shrinks with length of neck.
	float frac3[5] = {1.0f, 0.9f, 0.8f, 0.7f, 0.6f};
	hret *= frac3[m_DNA.NeckVertebraeCount()];
	m_HeadWidth = hret;

	// Thigh Length
	val = m_DNA.Length(m_DNA.ThighLength);
	val *= SizeFactor();				// Scale by overall size factor.
	m_ThighLength = val * 1.3;

	// Thigh Thickness
	len = ThighLength()*0.5f;		// Basis of thickness is limb length.
	val = m_DNA.Thickness(m_DNA.ThighThickness) * len;
	m_ThighThickness = val;

	// Upper Arm Length
	val = m_DNA.Length(m_DNA.UpperArmLength);
	val *= SizeFactor();				// Scale by overall size factor.
	m_UpperArmLength = val;

	// Upper Arm Thickness
	len = UpperArmLength()*0.5f;	// Basis of thickness is limb length.
	val = m_DNA.Thickness(m_DNA.UpperArmThickness) * len;
	m_UpperArmThickness = val;
}

// This function is called by the model maker thread when it is in use. This function must do all that it needs to make the
// entire body model.
void DpBody::CreateSkin(AuGLEffects& re, AuGLModelCache& mc)
{
	SelectTextures(re);
	SetupSkinMaterial();
	for(QVector<DpBodyPart*>::const_iterator it = constBegin(); it != constEnd(); it++)
		(*it)->CreateSkin(re, mc);

	/*
	// Make a simple box of quads.
	re.glColor4f(.9,1.0,.9,1.0);
	const float rht = 5.75;
	const float rht2 = rht/2.0;
	re.glBegin(GL_QUADS);
		re.glNormal3f(0.0, 1.0, 0.0);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,0,1,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(0.0, 10.5, -rht2);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,1,0,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(0.0, 10.5, rht2);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,1,0,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(-2.0, 10.5, rht2);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,1,0,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(-2.0, 10.5, -rht2);
	re.glEnd();

	re.glBegin(GL_QUADS);
		re.glNormal3f(0.0, -1.0, 0.0);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,0,1,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(-2.0, -0.5, -rht2);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,1,0,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(-2.0, -0.5, rht2);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,1,0,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(0.0, -0.5, rht2);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,1,0,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(0.0, -0.5, -rht2);

		re.glNormal3f(0.0, 0.0, 1.0);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,0,1,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(-2.0, -0.5, rht2);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,1,0,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(-2.0, 10.5, rht2);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,1,0,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(0.0, 10.5, rht2);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,1,0,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(0.0, -0.5, rht2);

		re.glNormal3f(0.0, 0.0, -1.0);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,0,1,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(-2.0, 10.5, -rht2);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,1,0,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(-2.0, -0.5, -rht2);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,1,0,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(0.0, -0.5, -rht2);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,1,0,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(0.0, 10.5, -rht2);

		re.glNormal3f(1.0, 0.0, 0.0);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,0,1,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(0.0, -0.5, -rht2);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,1,0,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(0.0, -0.5, rht2);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,1,0,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(0.0, 10.5, rht2);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,1,0,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(0.0, 10.5, -rht2);

		re.glNormal3f(-1.0, 0.0, 0.0);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,0,1,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(-2.0, 10.5, -rht2);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,1,0,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(-2.0, 10.5, rht2);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,1,0,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(-2.0, -0.5, rht2);
		GLf.glMultiTexCoord4f(GL_TEXTURE5,1,0,0,0);
		GLf.glMultiTexCoord4f(GL_TEXTURE6,0,0,0,0);
		re.glVertex3f(-2.0, -0.5, -rht2);
	re.glEnd();
	*/
}

// One of many RenderSkin variations. If you change this one, be sure to apply those changes to the others.
void DpBody::RenderSkin(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, const AuPoint& ambientLightUp,
		bool headless)
{
	if(m_Models.isEmpty())
		return;

	bool bHeadOnly = false;
	if(CurrentGeneralPose().HeadOnly())
	{
		if(headless)
			return;
		// Load Identity matrix into bone
		Bone(DhDNA::BONE_Head0).LoadIdentity();
		if(m_DNA.Heads)
			Bone(DhDNA::BONE_Head1).LoadIdentity();

		// Translate bone to current camera location
		AuPoint fTrans;
		if(DNA().Heads && IsRightHanded())
		{
			// Two-headed right-handed creature sees with the second head, which is the right head when there are two.
			bool isLeft = true;	// first head is left
			for(QVector<DpBodyPart*>::const_iterator it = constBegin(); it != constEnd(); it++)
				if((*it)->IsHeadAndNeck())
				{
					if(isLeft)
						isLeft = false;
					else
					{
						fTrans = ((DpHeadAndNeck*)(*it))->CurrentCameraLoc();
						break;
					}
				}
		}
		else
		{
			// One-headed creature or left-handed creature sees with the first head, which is the left head when there are two.
			for(QVector<DpBodyPart*>::const_iterator it = constBegin(); it != constEnd(); it++)
				if((*it)->IsHeadAndNeck())
				{
					fTrans = ((DpHeadAndNeck*)(*it))->CurrentCameraLoc();
					break;
				}
		}
		AuMatrix4x4 headBone;
		headBone.Translate(-fTrans);
		bHeadOnly = true;
		Bone(DhDNA::BONE_Head0) = headBone;
		if(m_DNA.Heads)
			Bone(DhDNA::BONE_Head1) = headBone;
	}

	// Select skin, clothing and armor textures.
	int iCurSkin = 0;
	int iCurClothes = 0;
	int iCurArmor = 0;
	int iCurHelmet = 0;
	{
		// Get current skin texture
		if(m_DNA.SkinType == DhDNA::SKIN_DrySkin || m_DNA.SkinType == DhDNA::SKIN_WetSkin)
		{
			iCurSkin = m_glTexSkin[0];
			iCurClothes = m_glTexClothes[0];
		}
		else if(m_DNA.SkinType == DhDNA::SKIN_DryExoskeleton || m_DNA.SkinType == DhDNA::SKIN_WetExoskeleton)
		{
			iCurSkin = m_glTexSkin[1];
			iCurClothes = m_glTexClothes[1];
		}
		else if(m_DNA.SkinType == DhDNA::SKIN_DryScales || m_DNA.SkinType == DhDNA::SKIN_WetScales)
		{
			iCurSkin = m_glTexSkin[2];
			iCurClothes = m_glTexClothes[2];
		}
		else// if(m_DNA.SkinType == DhDNA::SKIN_Fur || m_DNA.SkinType == DhDNA::SKIN_Feathers)
		{
			iCurSkin = m_glTexSkin[3];
			iCurClothes = m_glTexClothes[3];
		}

		// Get current armor texture
		if(m_eBodyClothesType == CID_VacSuit)
			iCurArmor = m_glTexArmor[0];
		else if(m_eBodyClothesType == CID_MetalPowerArmor)
			iCurArmor = m_glTexArmor[1];
		else if(m_eBodyClothesType == CID_MagmiumPowerArmor)
			iCurArmor = m_glTexArmor[2];
		else if(m_eBodyClothesType == CID_VulciumPowerArmor)
			iCurArmor = m_glTexArmor[3];

		// Get current helmet texture
		int iType = 0;
		if(m_eHelmetClothesType == CID_MetalPowerArmorHelmet)
			iType = 0;
		else if(m_eHelmetClothesType == CID_MagmiumPowerArmorHelmet)
			iType = 1;
		else if(m_eHelmetClothesType == CID_VulciumPowerArmorHelmet)
			iType = 2;
		else if(m_eHelmetClothesType == CID_VacSuitHelmet)
			iType = 3;
		iCurHelmet = m_glTexHelmet[m_DNA.SkinType][iType];
	}

	// Configure shaders.
	float ar, ag, ab, aa;
	re.CurrentAmbientLight(ar, ag, ab, aa);

	re.GLSL().BodyShader().Select(projection, re);
	re.GLSL().BodyShader().SetModelView(model);

	// Ambient light
	re.GLSL().BodyShader().SetAmbientLight(ar, ag, ab);
	re.GLSL().BodyShader().SetAmbientLightUp(ambientLightUp.x, ambientLightUp.y, ambientLightUp.z);

	// Bone matrices.
	{
		// The shader parameter space is limited in size; when exceeded the
		// shader stops working and nothing appears on the screen.
		// For speed, we only send the bone matrices that we actually use.
		// Torso
		for(unsigned int ib=0; ib<m_DNA.TorsoVertebrae; ib++)
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Torso0 + ib, m_Bones[DhDNA::BONE_Torso0 + ib]);

		// Head(s) and neck(s)
		const int nvc = m_DNA.NeckVertebraeCount();
		re.GLSL().BodyShader().SetBone(DhDNA::BONE_Head0, m_Bones[DhDNA::BONE_Head0]);
		//RenderBoneMatrix(DhDNA::BONE_Jaw0, vp, pname1, pname2, pname3);
		for(int ib=0; ib<nvc; ib++)
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Neck00 + ib, m_Bones[DhDNA::BONE_Neck00 + ib]);
		if(m_DNA.Heads)
		{
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Head1, m_Bones[DhDNA::BONE_Head1]);
			//RenderBoneMatrix(DhDNA::BONE_Jaw1, vp, pname1, pname2, pname3);
			for(int ib=0; ib<nvc; ib++)
				re.GLSL().BodyShader().SetBone(DhDNA::BONE_Neck10 + ib, m_Bones[DhDNA::BONE_Neck10 + ib]);
		}

		// Tail
		int tailverts = (unsigned int)m_DNA.TailVertebrae*2;	// Tail
		if(tailverts)
			tailverts--;
		for(int ib=0; ib<=tailverts; ib++)
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Tail0 + ib, m_Bones[DhDNA::BONE_Tail0 + ib]);

		// Arms
		if(m_DNA.ArmPairs)
		{
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Hand0L, m_Bones[DhDNA::BONE_Hand0L]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Hand0R, m_Bones[DhDNA::BONE_Hand0R]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_ForeArm0L, m_Bones[DhDNA::BONE_ForeArm0L]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_ForeArm0R, m_Bones[DhDNA::BONE_ForeArm0R]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_UpperArm0L, m_Bones[DhDNA::BONE_UpperArm0L]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_UpperArm0R, m_Bones[DhDNA::BONE_UpperArm0R]);
		}
		if(m_DNA.ArmPairs >= 2)
		{
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Hand1L, m_Bones[DhDNA::BONE_Hand1L]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Hand1R, m_Bones[DhDNA::BONE_Hand1R]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_ForeArm1L, m_Bones[DhDNA::BONE_ForeArm1L]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_ForeArm1R, m_Bones[DhDNA::BONE_ForeArm1R]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_UpperArm1L, m_Bones[DhDNA::BONE_UpperArm1L]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_UpperArm1R, m_Bones[DhDNA::BONE_UpperArm1R]);
		}
		if(m_DNA.ArmPairs == 3)
		{
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Hand2L, m_Bones[DhDNA::BONE_Hand2L]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Hand2R, m_Bones[DhDNA::BONE_Hand2R]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_ForeArm2L, m_Bones[DhDNA::BONE_ForeArm2L]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_ForeArm2R, m_Bones[DhDNA::BONE_ForeArm2R]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_UpperArm2L, m_Bones[DhDNA::BONE_UpperArm2L]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_UpperArm2R, m_Bones[DhDNA::BONE_UpperArm2R]);
		}

		// Legs
		if(m_DNA.LegPairs)
		{
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Foot0L, m_Bones[DhDNA::BONE_Foot0L]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Foot0R, m_Bones[DhDNA::BONE_Foot0R]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Calf0L, m_Bones[DhDNA::BONE_Calf0L]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Calf0R, m_Bones[DhDNA::BONE_Calf0R]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Thigh0L, m_Bones[DhDNA::BONE_Thigh0L]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Thigh0R, m_Bones[DhDNA::BONE_Thigh0R]);
		}
		if(m_DNA.LegPairs >= 2)
		{
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Foot1L, m_Bones[DhDNA::BONE_Foot1L]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Foot1R, m_Bones[DhDNA::BONE_Foot1R]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Calf1L, m_Bones[DhDNA::BONE_Calf1L]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Calf1R, m_Bones[DhDNA::BONE_Calf1R]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Thigh1L, m_Bones[DhDNA::BONE_Thigh1L]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Thigh1R, m_Bones[DhDNA::BONE_Thigh1R]);
		}
		if(m_DNA.LegPairs == 3)
		{
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Foot2L, m_Bones[DhDNA::BONE_Foot2L]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Foot2R, m_Bones[DhDNA::BONE_Foot2R]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Calf2L, m_Bones[DhDNA::BONE_Calf2L]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Calf2R, m_Bones[DhDNA::BONE_Calf2R]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Thigh2L, m_Bones[DhDNA::BONE_Thigh2L]);
			re.GLSL().BodyShader().SetBone(DhDNA::BONE_Thigh2R, m_Bones[DhDNA::BONE_Thigh2R]);
		}
	}

	re.GLSL().BodyShader().SetSkinTextureOffset(m_skinOffset[0], m_skinOffset[1], m_skinOffset[2], m_skinOffset[3]);

	// Call the OpenGL display lists of the vertex skinned model.

	// Prepare to draw head or helmet. There is always a head; it might be the only thing we draw.
	if(HasHelmetPowerArmorOn())
	{
		re.GLSL().BodyShader().SetClothesTextureOffsetWear(0, 0, 1);	// Clothes only
		re.GLSL().BodyShader().SetClothesTexture(iCurHelmet);
		re.GLSL().BodyShader().SetSkinTexture(0);
	}
	else
	{
		re.GLSL().BodyShader().SetClothesTextureOffsetWear(0, 0, 0);	// Skin only
		re.GLSL().BodyShader().SetClothesTexture(0);
		re.GLSL().BodyShader().SetSkinTexture(iCurSkin);
	}

	if(bHeadOnly)
	{
		// Display head pieces only.
		for(QVector<Au3dsGLSLModelInstance*>::iterator mii = m_Models.begin(); mii != m_Models.end(); mii++)
		{
			Au3dsGLSLModelInstance* mi = *mii;
			if(mi->FileName().contains("Head"))
				mi->Render(re, false);
		}
	}
	else
	{
		// Display head pieces and collect info about other pieces we will draw.
		Au3dsGLSLModelInstance* mNeck = 0;
		Au3dsGLSLModelInstance* mTail = 0;
		Au3dsGLSLModelInstance* mTip = 0;
		Au3dsGLSLModelInstance* mWingL = 0;
		Au3dsGLSLModelInstance* mWingR = 0;
		QList<Au3dsGLSLModelInstance*> mBody;	// All parts except heads, necks, wings, tails and tips.
		for(QVector<Au3dsGLSLModelInstance*>::iterator mii = m_Models.begin(); mii != m_Models.end(); mii++)
		{
			Au3dsGLSLModelInstance* mi = *mii;
			if(mi->FileName().contains("Head"))
			{
				if(!headless)
					mi->Render(re, false);
			}
			else if(!bHeadOnly)
			{
				if(!mi->FileName().contains("Wing") && !mi->FileName().contains("Head") &&
					!mi->FileName().contains("Neck") && !mi->FileName().contains("Tail") &&
					!mi->FileName().contains("Tip"))
					mBody.append(mi);
				else if(mi->FileName().contains("WingL"))
					mWingL = mi;
				else if(mi->FileName().contains("WingR"))
					mWingR = mi;
				else if(mi->FileName().contains("Neck"))
					mNeck = mi;
				else if(mi->FileName().contains("Tail"))
					mTail = mi;
				else if(mi->FileName().contains("Tip"))
					mTip = mi;
			}
		}

		// Display neck.
		if(mNeck)
		{
			// Shader is currently configured for skin texture or helmet texture.
			if(HasHelmetPowerArmorOn())
			{
				// Select armor texture for neck, to match the helmet.
				QString sArmorType;
				if(iCurArmor == 0)
				{
					int iType = 0;
					if(m_eHelmetClothesType == CID_MetalPowerArmorHelmet)
						iType = 0;
					else if(m_eHelmetClothesType == CID_MagmiumPowerArmorHelmet)
						iType = 1;
					else if(m_eHelmetClothesType == CID_VulciumPowerArmorHelmet)
						iType = 2;
					else if(m_eHelmetClothesType == CID_VacSuitHelmet)
						iType = 3;
					iCurArmor = m_glTexArmor[iType];
				}
				re.GLSL().BodyShader().SetClothesTexture(iCurArmor);
			}
			mNeck->Render(re, false);
		}

		// Display wings and tail. They never have armor.
		if(mTail || mTip || mWingL || mWingR)
		{
			// Tail is always skin, not armored.
			re.GLSL().BodyShader().SetSkinTexture(iCurSkin);
			re.GLSL().BodyShader().SetClothesTexture(0);
			re.GLSL().BodyShader().SetClothesTextureOffsetWear(0, 0, 0);	// Skin only
			if(mTail)
				mTail->Render(re, false);
			if(mTip)
				mTip->Render(re, false);

			// Wings use their own texture.
			if(mWingL || mWingR)
			{
				float uOffset = 0;
				float vOffset = (((m_DNA.WingSubType) * 128))/1024.0;
				if(vOffset > 0)
					vOffset = 1-vOffset;

				re.GLSL().BodyShader().SetSkinTexture(m_glTexWings);
				re.GLSL().BodyShader().SetSkinTextureOffset(uOffset, vOffset, uOffset, vOffset);

				if(mWingL)
					mWingL->Render(re, false);
				if(mWingR)
					mWingR->Render(re, false);

				// Restore skin texture offsets for the body.
				re.GLSL().BodyShader().SetSkinTextureOffset(m_skinOffset[0], m_skinOffset[1], m_skinOffset[2], m_skinOffset[3]);
			}
		}

		if(!mBody.isEmpty())
		{
			// Select appropriate texture(s) for body parts.
			if(HasBodyPowerArmorOn())
			{
				// Body is wearing power armor.
				re.GLSL().BodyShader().SetSkinTexture(0);
				re.GLSL().BodyShader().SetClothesTexture(iCurArmor);
				re.GLSL().BodyShader().SetClothesTextureOffsetWear(0, 0, 1);	// Clothes only
			}
			else if(iCurClothes && m_eBodyClothesType != CID_Invalid)
			{
				// Body is wearing clothing.
				// Skin offset.
				re.GLSL().BodyShader().SetSkinTexture(iCurSkin);

				// Clothes offset
				float clothesOffX;
				float clothesOffY;
				switch(m_eBodyClothesType)
				{
				default:
					clothesOffX = 0;
					clothesOffY = 0;
					break;
				case CID_Underwear:
				case CID_LeatherClothes:
				case CID_JeansAndShirt:
				case CID_BlackUniform:
				case CID_BlueUniform:
				case CID_BrownUniform:
				case CID_GoldUniform:
				case CID_GreenUniform:
				case CID_GreyUniform:
				case CID_OrangeUniform:
				case CID_PinkUniform:
				case CID_RedUniform:
				case CID_TealUniform:
				case CID_WhiteUniform:
				case CID_ParcXUniform:
				case CID_GreyBDU:
				case CID_KhakiBDU:
				case CID_OliveDrabBDU:
					clothesOffX = 0;
					clothesOffY = 1;
					break;
				}
				re.GLSL().BodyShader().SetClothesTexture(iCurClothes);
				re.GLSL().BodyShader().SetClothesTextureOffsetWear(clothesOffX, clothesOffY, 2);	// Skin and clothes
			}
			else
			{
				// Body is naked.
				// Skin offset.
				re.GLSL().BodyShader().SetSkinTexture(iCurSkin);

				// Clothes offset
				re.GLSL().BodyShader().SetClothesTexture(0);
				re.GLSL().BodyShader().SetClothesTextureOffsetWear(0, 0, 0);	// Skin only
			}

			// Display body pieces
			for(QList<Au3dsGLSLModelInstance*>::iterator mii = mBody.begin(); mii != mBody.end(); mii++)
				(*mii)->Render(re, false);
		}
	}

	re.GLSL().BodyShader().Deselect();
}

// One of many RenderSkin variations. If you change this one, be sure to apply those changes to the others.
void DpBody::RenderSkinGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		const AuPoint& ambientLightUp, const AuPoint& globeP, bool headless)
{
	Q_UNUSED(projection);
	if(m_Models.isEmpty())
		return;

	bool bHeadOnly = false;
	if(CurrentGeneralPose().HeadOnly())
	{
		if(headless)
			return;
		// Load Identity matrix into bone
		Bone(DhDNA::BONE_Head0).LoadIdentity();
		if(m_DNA.Heads)
			Bone(DhDNA::BONE_Head1).LoadIdentity();

		// Translate bone to current camera location
		AuPoint fTrans;
		if(DNA().Heads && IsRightHanded())
		{
			// Two-headed right-handed creature sees with the second head, which is the right head when there are two.
			bool isLeft = true;	// first head is left
			for(QVector<DpBodyPart*>::const_iterator it = constBegin(); it != constEnd(); it++)
				if((*it)->IsHeadAndNeck())
				{
					if(isLeft)
						isLeft = false;
					else
					{
						fTrans = ((DpHeadAndNeck*)(*it))->CurrentCameraLoc();
						break;
					}
				}
		}
		else
		{
			// One-headed creature or left-handed creature sees with the first head, which is the left head when there are two.
			for(QVector<DpBodyPart*>::const_iterator it = constBegin(); it != constEnd(); it++)
				if((*it)->IsHeadAndNeck())
				{
					fTrans = ((DpHeadAndNeck*)(*it))->CurrentCameraLoc();
					break;
				}
		}
		AuMatrix4x4 headBone;
		headBone.Translate(-fTrans);
		bHeadOnly = true;
		Bone(DhDNA::BONE_Head0) = headBone;
		if(m_DNA.Heads)
			Bone(DhDNA::BONE_Head1) = headBone;
	}

	// Select skin, clothing and armor textures.
	int iCurSkin = 0;
	int iCurClothes = 0;
	int iCurArmor = 0;
	int iCurHelmet = 0;
	{
		// Get current skin texture
		if(m_DNA.SkinType == DhDNA::SKIN_DrySkin || m_DNA.SkinType == DhDNA::SKIN_WetSkin)
		{
			iCurSkin = m_glTexSkin[0];
			iCurClothes = m_glTexClothes[0];
		}
		else if(m_DNA.SkinType == DhDNA::SKIN_DryExoskeleton || m_DNA.SkinType == DhDNA::SKIN_WetExoskeleton)
		{
			iCurSkin = m_glTexSkin[1];
			iCurClothes = m_glTexClothes[1];
		}
		else if(m_DNA.SkinType == DhDNA::SKIN_DryScales || m_DNA.SkinType == DhDNA::SKIN_WetScales)
		{
			iCurSkin = m_glTexSkin[2];
			iCurClothes = m_glTexClothes[2];
		}
		else// if(m_DNA.SkinType == DhDNA::SKIN_Fur || m_DNA.SkinType == DhDNA::SKIN_Feathers)
		{
			iCurSkin = m_glTexSkin[3];
			iCurClothes = m_glTexClothes[3];
		}

		// Get current armor texture
		if(m_eBodyClothesType == CID_VacSuit)
			iCurArmor = m_glTexArmor[0];
		else if(m_eBodyClothesType == CID_MetalPowerArmor)
			iCurArmor = m_glTexArmor[1];
		else if(m_eBodyClothesType == CID_MagmiumPowerArmor)
			iCurArmor = m_glTexArmor[2];
		else if(m_eBodyClothesType == CID_VulciumPowerArmor)
			iCurArmor = m_glTexArmor[3];

		// Get current helmet texture
		int iType = 0;
		if(m_eHelmetClothesType == CID_MetalPowerArmorHelmet)
			iType = 0;
		else if(m_eHelmetClothesType == CID_MagmiumPowerArmorHelmet)
			iType = 1;
		else if(m_eHelmetClothesType == CID_VulciumPowerArmorHelmet)
			iType = 2;
		else if(m_eHelmetClothesType == CID_VacSuitHelmet)
			iType = 3;
		iCurHelmet = m_glTexHelmet[m_DNA.SkinType][iType];
	}

	// Configure shaders.
	float ar, ag, ab, aa;
	re.CurrentAmbientLight(ar, ag, ab, aa);

	// No fog, not a ringworld.
	re.GLSL().BodyGlobeShader().Select(projection, re);
	re.GLSL().BodyGlobeShader().SetModelView(model);

	// Ambient light
	re.GLSL().BodyGlobeShader().SetAmbientLight(ar, ag, ab);
	re.GLSL().BodyGlobeShader().SetAmbientLightUp(ambientLightUp.x, ambientLightUp.y, ambientLightUp.z);
	re.GLSL().BodyGlobeShader().SetGlobeLocation(globeP.x, globeP.y, globeP.z);

	// Bone matrices.
	{
		// The shader parameter space is limited in size; when exceeded the
		// shader stops working and nothing appears on the screen.
		// For speed, we only send the bone matrices that we actually use.
		// Torso
		for(unsigned int ib=0; ib<m_DNA.TorsoVertebrae; ib++)
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Torso0 + ib, m_Bones[DhDNA::BONE_Torso0 + ib]);

		// Head(s) and neck(s)
		const int nvc = m_DNA.NeckVertebraeCount();
		re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Head0, m_Bones[DhDNA::BONE_Head0]);
		//RenderBoneMatrix(DhDNA::BONE_Jaw0, vp, pname1, pname2, pname3);
		for(int ib=0; ib<nvc; ib++)
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Neck00 + ib, m_Bones[DhDNA::BONE_Neck00 + ib]);
		if(m_DNA.Heads)
		{
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Head1, m_Bones[DhDNA::BONE_Head1]);
			//RenderBoneMatrix(DhDNA::BONE_Jaw1, vp, pname1, pname2, pname3);
			for(int ib=0; ib<nvc; ib++)
				re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Neck10 + ib, m_Bones[DhDNA::BONE_Neck10 + ib]);
		}

		// Tail
		int tailverts = (unsigned int)m_DNA.TailVertebrae*2;	// Tail
		if(tailverts)
			tailverts--;
		for(int ib=0; ib<=tailverts; ib++)
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Tail0 + ib, m_Bones[DhDNA::BONE_Tail0 + ib]);

		// Arms
		if(m_DNA.ArmPairs)
		{
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Hand0L, m_Bones[DhDNA::BONE_Hand0L]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Hand0R, m_Bones[DhDNA::BONE_Hand0R]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_ForeArm0L, m_Bones[DhDNA::BONE_ForeArm0L]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_ForeArm0R, m_Bones[DhDNA::BONE_ForeArm0R]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_UpperArm0L, m_Bones[DhDNA::BONE_UpperArm0L]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_UpperArm0R, m_Bones[DhDNA::BONE_UpperArm0R]);
		}
		if(m_DNA.ArmPairs >= 2)
		{
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Hand1L, m_Bones[DhDNA::BONE_Hand1L]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Hand1R, m_Bones[DhDNA::BONE_Hand1R]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_ForeArm1L, m_Bones[DhDNA::BONE_ForeArm1L]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_ForeArm1R, m_Bones[DhDNA::BONE_ForeArm1R]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_UpperArm1L, m_Bones[DhDNA::BONE_UpperArm1L]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_UpperArm1R, m_Bones[DhDNA::BONE_UpperArm1R]);
		}
		if(m_DNA.ArmPairs == 3)
		{
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Hand2L, m_Bones[DhDNA::BONE_Hand2L]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Hand2R, m_Bones[DhDNA::BONE_Hand2R]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_ForeArm2L, m_Bones[DhDNA::BONE_ForeArm2L]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_ForeArm2R, m_Bones[DhDNA::BONE_ForeArm2R]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_UpperArm2L, m_Bones[DhDNA::BONE_UpperArm2L]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_UpperArm2R, m_Bones[DhDNA::BONE_UpperArm2R]);
		}

		// Legs
		if(m_DNA.LegPairs)
		{
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Foot0L, m_Bones[DhDNA::BONE_Foot0L]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Foot0R, m_Bones[DhDNA::BONE_Foot0R]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Calf0L, m_Bones[DhDNA::BONE_Calf0L]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Calf0R, m_Bones[DhDNA::BONE_Calf0R]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Thigh0L, m_Bones[DhDNA::BONE_Thigh0L]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Thigh0R, m_Bones[DhDNA::BONE_Thigh0R]);
		}
		if(m_DNA.LegPairs >= 2)
		{
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Foot1L, m_Bones[DhDNA::BONE_Foot1L]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Foot1R, m_Bones[DhDNA::BONE_Foot1R]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Calf1L, m_Bones[DhDNA::BONE_Calf1L]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Calf1R, m_Bones[DhDNA::BONE_Calf1R]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Thigh1L, m_Bones[DhDNA::BONE_Thigh1L]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Thigh1R, m_Bones[DhDNA::BONE_Thigh1R]);
		}
		if(m_DNA.LegPairs == 3)
		{
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Foot2L, m_Bones[DhDNA::BONE_Foot2L]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Foot2R, m_Bones[DhDNA::BONE_Foot2R]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Calf2L, m_Bones[DhDNA::BONE_Calf2L]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Calf2R, m_Bones[DhDNA::BONE_Calf2R]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Thigh2L, m_Bones[DhDNA::BONE_Thigh2L]);
			re.GLSL().BodyGlobeShader().SetBone(DhDNA::BONE_Thigh2R, m_Bones[DhDNA::BONE_Thigh2R]);
		}
	}

	re.GLSL().BodyGlobeShader().SetSkinTextureOffset(m_skinOffset[0], m_skinOffset[1], m_skinOffset[2], m_skinOffset[3]);

	// Call the OpenGL display lists of the vertex skinned model.

	// Prepare to draw head or helmet. There is always a head; it might be the only thing we draw.
	if(HasHelmetPowerArmorOn())
	{
		re.GLSL().BodyGlobeShader().SetClothesTextureOffsetWear(0, 0, 1);	// Clothes only
		re.GLSL().BodyGlobeShader().SetClothesTexture(iCurHelmet);
		re.GLSL().BodyGlobeShader().SetSkinTexture(0);
	}
	else
	{
		re.GLSL().BodyGlobeShader().SetClothesTextureOffsetWear(0, 0, 0);	// Skin only
		re.GLSL().BodyGlobeShader().SetClothesTexture(0);
		re.GLSL().BodyGlobeShader().SetSkinTexture(iCurSkin);
	}

	if(bHeadOnly)
	{
		// Display head pieces only.
		for(QVector<Au3dsGLSLModelInstance*>::iterator mii = m_Models.begin(); mii != m_Models.end(); mii++)
		{
			Au3dsGLSLModelInstance* mi = *mii;
			if(mi->FileName().contains("Head"))
				mi->Render(re, false);
		}
	}
	else
	{
		// Display head pieces and collect info about other pieces we will draw.
		Au3dsGLSLModelInstance* mNeck = 0;
		Au3dsGLSLModelInstance* mTail = 0;
		Au3dsGLSLModelInstance* mTip = 0;
		Au3dsGLSLModelInstance* mWingL = 0;
		Au3dsGLSLModelInstance* mWingR = 0;
		QList<Au3dsGLSLModelInstance*> mBody;	// All parts except heads, necks, wings, tails and tips.
		for(QVector<Au3dsGLSLModelInstance*>::iterator mii = m_Models.begin(); mii != m_Models.end(); mii++)
		{
			Au3dsGLSLModelInstance* mi = *mii;
			if(mi->FileName().contains("Head"))
			{
				if(!headless)
					mi->Render(re, false);
			}
			else if(!bHeadOnly)
			{
				if(!mi->FileName().contains("Wing") && !mi->FileName().contains("Head") &&
					!mi->FileName().contains("Neck") && !mi->FileName().contains("Tail") &&
					!mi->FileName().contains("Tip"))
					mBody.append(mi);
				else if(mi->FileName().contains("WingL"))
					mWingL = mi;
				else if(mi->FileName().contains("WingR"))
					mWingR = mi;
				else if(mi->FileName().contains("Neck"))
					mNeck = mi;
				else if(mi->FileName().contains("Tail"))
					mTail = mi;
				else if(mi->FileName().contains("Tip"))
					mTip = mi;
			}
		}

		// Display neck.
		if(mNeck)
		{
			// Shader is currently configured for skin texture or helmet texture.
			if(HasHelmetPowerArmorOn())
			{
				// Select armor texture for neck, to match the helmet.
				QString sArmorType;
				if(iCurArmor == 0)
				{
					int iType = 0;
					if(m_eHelmetClothesType == CID_MetalPowerArmorHelmet)
						iType = 0;
					else if(m_eHelmetClothesType == CID_MagmiumPowerArmorHelmet)
						iType = 1;
					else if(m_eHelmetClothesType == CID_VulciumPowerArmorHelmet)
						iType = 2;
					else if(m_eHelmetClothesType == CID_VacSuitHelmet)
						iType = 3;
					iCurArmor = m_glTexArmor[iType];
				}
				re.GLSL().BodyGlobeShader().SetClothesTexture(iCurArmor);
			}
			mNeck->Render(re, false);
		}

		// Display wings and tail. They never have armor.
		if(mTail || mTip || mWingL || mWingR)
		{
			// Tail is always skin, not armored.
			re.GLSL().BodyGlobeShader().SetSkinTexture(iCurSkin);
			re.GLSL().BodyGlobeShader().SetClothesTexture(0);
			re.GLSL().BodyGlobeShader().SetClothesTextureOffsetWear(0, 0, 0);	// Skin only
			if(mTail)
				mTail->Render(re, false);
			if(mTip)
				mTip->Render(re, false);

			// Wings use their own texture.
			if(mWingL || mWingR)
			{
				float uOffset = 0;
				float vOffset = (((m_DNA.WingSubType) * 128))/1024.0;
				if(vOffset > 0)
					vOffset = 1-vOffset;

				re.GLSL().BodyGlobeShader().SetSkinTexture(m_glTexWings);
				re.GLSL().BodyGlobeShader().SetSkinTextureOffset(uOffset, vOffset, uOffset, vOffset);

				if(mWingL)
					mWingL->Render(re, false);
				if(mWingR)
					mWingR->Render(re, false);

				// Restore skin texture offsets for the body.
				re.GLSL().BodyGlobeShader().SetSkinTextureOffset(m_skinOffset[0], m_skinOffset[1], m_skinOffset[2],
						m_skinOffset[3]);
			}
		}

		if(!mBody.isEmpty())
		{
			// Select appropriate texture(s) for body parts.
			if(HasBodyPowerArmorOn())
			{
				// Body is wearing power armor.
				re.GLSL().BodyGlobeShader().SetSkinTexture(0);
				re.GLSL().BodyGlobeShader().SetClothesTexture(iCurArmor);
				re.GLSL().BodyGlobeShader().SetClothesTextureOffsetWear(0, 0, 1);	// Clothes only
			}
			else if(iCurClothes && m_eBodyClothesType != CID_Invalid)
			{
				// Body is wearing clothing.
				// Skin offset.
				re.GLSL().BodyGlobeShader().SetSkinTexture(iCurSkin);

				// Clothes offset
				float clothesOffX;
				float clothesOffY;
				switch(m_eBodyClothesType)
				{
				default:
					clothesOffX = 0;
					clothesOffY = 0;
					break;
				case CID_Underwear:
				case CID_LeatherClothes:
				case CID_JeansAndShirt:
				case CID_BlackUniform:
				case CID_BlueUniform:
				case CID_BrownUniform:
				case CID_GoldUniform:
				case CID_GreenUniform:
				case CID_GreyUniform:
				case CID_OrangeUniform:
				case CID_PinkUniform:
				case CID_RedUniform:
				case CID_TealUniform:
				case CID_WhiteUniform:
				case CID_ParcXUniform:
				case CID_GreyBDU:
				case CID_KhakiBDU:
				case CID_OliveDrabBDU:
					clothesOffX = 0;
					clothesOffY = 1;
					break;
				}
				re.GLSL().BodyGlobeShader().SetClothesTexture(iCurClothes);
				re.GLSL().BodyGlobeShader().SetClothesTextureOffsetWear(clothesOffX, clothesOffY, 2);	// Skin and clothes
			}
			else
			{
				// Body is naked.
				// Skin offset.
				re.GLSL().BodyGlobeShader().SetSkinTexture(iCurSkin);

				// Clothes offset
				re.GLSL().BodyGlobeShader().SetClothesTexture(0);
				re.GLSL().BodyGlobeShader().SetClothesTextureOffsetWear(0, 0, 0);	// Skin only
			}

			// Display body pieces
			for(QList<Au3dsGLSLModelInstance*>::iterator mii = mBody.begin(); mii != mBody.end(); mii++)
				(*mii)->Render(re, false);
		}
	}

	re.GLSL().BodyGlobeShader().Deselect();
}

// One of many RenderSkin variations. If you change this one, be sure to apply those changes to the others.
void DpBody::RenderSkinRingworld(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		const AuPoint& ambientLightUp, float shadowPanelSunAngles[MAX_SUNS], bool headless)
{
	Q_UNUSED(projection);
	if(m_Models.isEmpty())
		return;

	bool bHeadOnly = false;
	if(CurrentGeneralPose().HeadOnly())
	{
		if(headless)
			return;
		// Load Identity matrix into bone
		Bone(DhDNA::BONE_Head0).LoadIdentity();
		if(m_DNA.Heads)
			Bone(DhDNA::BONE_Head1).LoadIdentity();

		// Translate bone to current camera location
		AuPoint fTrans;
		if(DNA().Heads && IsRightHanded())
		{
			// Two-headed right-handed creature sees with the second head, which is the right head when there are two.
			bool isLeft = true;	// first head is left
			for(QVector<DpBodyPart*>::const_iterator it = constBegin(); it != constEnd(); it++)
				if((*it)->IsHeadAndNeck())
				{
					if(isLeft)
						isLeft = false;
					else
					{
						fTrans = ((DpHeadAndNeck*)(*it))->CurrentCameraLoc();
						break;
					}
				}
		}
		else
		{
			// One-headed creature or left-handed creature sees with the first head, which is the left head when there are two.
			for(QVector<DpBodyPart*>::const_iterator it = constBegin(); it != constEnd(); it++)
				if((*it)->IsHeadAndNeck())
				{
					fTrans = ((DpHeadAndNeck*)(*it))->CurrentCameraLoc();
					break;
				}
		}
		AuMatrix4x4 headBone;
		headBone.Translate(-fTrans);
		bHeadOnly = true;
		Bone(DhDNA::BONE_Head0) = headBone;
		if(m_DNA.Heads)
			Bone(DhDNA::BONE_Head1) = headBone;
	}

	// Select skin, clothing and armor textures.
	int iCurSkin = 0;
	int iCurClothes = 0;
	int iCurArmor = 0;
	int iCurHelmet = 0;
	{
		// Get current skin texture
		if(m_DNA.SkinType == DhDNA::SKIN_DrySkin || m_DNA.SkinType == DhDNA::SKIN_WetSkin)
		{
			iCurSkin = m_glTexSkin[0];
			iCurClothes = m_glTexClothes[0];
		}
		else if(m_DNA.SkinType == DhDNA::SKIN_DryExoskeleton || m_DNA.SkinType == DhDNA::SKIN_WetExoskeleton)
		{
			iCurSkin = m_glTexSkin[1];
			iCurClothes = m_glTexClothes[1];
		}
		else if(m_DNA.SkinType == DhDNA::SKIN_DryScales || m_DNA.SkinType == DhDNA::SKIN_WetScales)
		{
			iCurSkin = m_glTexSkin[2];
			iCurClothes = m_glTexClothes[2];
		}
		else// if(m_DNA.SkinType == DhDNA::SKIN_Fur || m_DNA.SkinType == DhDNA::SKIN_Feathers)
		{
			iCurSkin = m_glTexSkin[3];
			iCurClothes = m_glTexClothes[3];
		}

		// Get current armor texture
		if(m_eBodyClothesType == CID_VacSuit)
			iCurArmor = m_glTexArmor[0];
		else if(m_eBodyClothesType == CID_MetalPowerArmor)
			iCurArmor = m_glTexArmor[1];
		else if(m_eBodyClothesType == CID_MagmiumPowerArmor)
			iCurArmor = m_glTexArmor[2];
		else if(m_eBodyClothesType == CID_VulciumPowerArmor)
			iCurArmor = m_glTexArmor[3];

		// Get current helmet texture
		int iType = 0;
		if(m_eHelmetClothesType == CID_MetalPowerArmorHelmet)
			iType = 0;
		else if(m_eHelmetClothesType == CID_MagmiumPowerArmorHelmet)
			iType = 1;
		else if(m_eHelmetClothesType == CID_VulciumPowerArmorHelmet)
			iType = 2;
		else if(m_eHelmetClothesType == CID_VacSuitHelmet)
			iType = 3;
		iCurHelmet = m_glTexHelmet[m_DNA.SkinType][iType];
	}

	// Configure shaders.
	float ar, ag, ab, aa;
	re.CurrentAmbientLight(ar, ag, ab, aa);

	// No fog, on a ringworld.
	re.GLSL().BodyRingworldShader().Select(projection, re);
	re.GLSL().BodyRingworldShader().SetModelView(model);

	// Ambient light
	re.GLSL().BodyRingworldShader().SetAmbientLight(ar, ag, ab);
	re.GLSL().BodyRingworldShader().SetAmbientLightUp(ambientLightUp.x, ambientLightUp.y, ambientLightUp.z);

	// Suns
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		re.GLSL().BodyRingworldShader().SetSunShadowAngle(sunIndex, shadowPanelSunAngles[sunIndex]);

	// Bone matrices.
	{
		// The shader parameter space is limited in size; when exceeded the
		// shader stops working and nothing appears on the screen.
		// For speed, we only send the bone matrices that we actually use.
		// Torso
		for(unsigned int ib=0; ib<m_DNA.TorsoVertebrae; ib++)
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Torso0 + ib, m_Bones[DhDNA::BONE_Torso0 + ib]);

		// Head(s) and neck(s)
		const int nvc = m_DNA.NeckVertebraeCount();
		re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Head0, m_Bones[DhDNA::BONE_Head0]);
		//RenderBoneMatrix(DhDNA::BONE_Jaw0, vp, pname1, pname2, pname3);
		for(int ib=0; ib<nvc; ib++)
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Neck00 + ib, m_Bones[DhDNA::BONE_Neck00 + ib]);
		if(m_DNA.Heads)
		{
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Head1, m_Bones[DhDNA::BONE_Head1]);
			//RenderBoneMatrix(DhDNA::BONE_Jaw1, vp, pname1, pname2, pname3);
			for(int ib=0; ib<nvc; ib++)
				re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Neck10 + ib, m_Bones[DhDNA::BONE_Neck10 + ib]);
		}

		// Tail
		int tailverts = (unsigned int)m_DNA.TailVertebrae*2;	// Tail
		if(tailverts)
			tailverts--;
		for(int ib=0; ib<=tailverts; ib++)
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Tail0 + ib, m_Bones[DhDNA::BONE_Tail0 + ib]);

		// Arms
		if(m_DNA.ArmPairs)
		{
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Hand0L, m_Bones[DhDNA::BONE_Hand0L]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Hand0R, m_Bones[DhDNA::BONE_Hand0R]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_ForeArm0L, m_Bones[DhDNA::BONE_ForeArm0L]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_ForeArm0R, m_Bones[DhDNA::BONE_ForeArm0R]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_UpperArm0L, m_Bones[DhDNA::BONE_UpperArm0L]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_UpperArm0R, m_Bones[DhDNA::BONE_UpperArm0R]);
		}
		if(m_DNA.ArmPairs >= 2)
		{
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Hand1L, m_Bones[DhDNA::BONE_Hand1L]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Hand1R, m_Bones[DhDNA::BONE_Hand1R]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_ForeArm1L, m_Bones[DhDNA::BONE_ForeArm1L]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_ForeArm1R, m_Bones[DhDNA::BONE_ForeArm1R]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_UpperArm1L, m_Bones[DhDNA::BONE_UpperArm1L]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_UpperArm1R, m_Bones[DhDNA::BONE_UpperArm1R]);
		}
		if(m_DNA.ArmPairs == 3)
		{
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Hand2L, m_Bones[DhDNA::BONE_Hand2L]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Hand2R, m_Bones[DhDNA::BONE_Hand2R]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_ForeArm2L, m_Bones[DhDNA::BONE_ForeArm2L]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_ForeArm2R, m_Bones[DhDNA::BONE_ForeArm2R]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_UpperArm2L, m_Bones[DhDNA::BONE_UpperArm2L]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_UpperArm2R, m_Bones[DhDNA::BONE_UpperArm2R]);
		}

		// Legs
		if(m_DNA.LegPairs)
		{
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Foot0L, m_Bones[DhDNA::BONE_Foot0L]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Foot0R, m_Bones[DhDNA::BONE_Foot0R]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Calf0L, m_Bones[DhDNA::BONE_Calf0L]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Calf0R, m_Bones[DhDNA::BONE_Calf0R]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Thigh0L, m_Bones[DhDNA::BONE_Thigh0L]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Thigh0R, m_Bones[DhDNA::BONE_Thigh0R]);
		}
		if(m_DNA.LegPairs >= 2)
		{
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Foot1L, m_Bones[DhDNA::BONE_Foot1L]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Foot1R, m_Bones[DhDNA::BONE_Foot1R]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Calf1L, m_Bones[DhDNA::BONE_Calf1L]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Calf1R, m_Bones[DhDNA::BONE_Calf1R]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Thigh1L, m_Bones[DhDNA::BONE_Thigh1L]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Thigh1R, m_Bones[DhDNA::BONE_Thigh1R]);
		}
		if(m_DNA.LegPairs == 3)
		{
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Foot2L, m_Bones[DhDNA::BONE_Foot2L]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Foot2R, m_Bones[DhDNA::BONE_Foot2R]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Calf2L, m_Bones[DhDNA::BONE_Calf2L]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Calf2R, m_Bones[DhDNA::BONE_Calf2R]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Thigh2L, m_Bones[DhDNA::BONE_Thigh2L]);
			re.GLSL().BodyRingworldShader().SetBone(DhDNA::BONE_Thigh2R, m_Bones[DhDNA::BONE_Thigh2R]);
		}
	}

	re.GLSL().BodyRingworldShader().SetSkinTextureOffset(m_skinOffset[0], m_skinOffset[1], m_skinOffset[2],
			m_skinOffset[3]);

	// Call the OpenGL display lists of the vertex skinned model.

	// Prepare to draw head or helmet. There is always a head; it might be the only thing we draw.
	if(HasHelmetPowerArmorOn())
	{
		re.GLSL().BodyRingworldShader().SetClothesTextureOffsetWear(0, 0, 1);	// Clothes only
		re.GLSL().BodyRingworldShader().SetClothesTexture(iCurHelmet);
		re.GLSL().BodyRingworldShader().SetSkinTexture(0);
	}
	else
	{
		re.GLSL().BodyRingworldShader().SetClothesTextureOffsetWear(0, 0, 0);	// Skin only
		re.GLSL().BodyRingworldShader().SetClothesTexture(0);
		re.GLSL().BodyRingworldShader().SetSkinTexture(iCurSkin);
	}

	if(bHeadOnly)
	{
		// Display head pieces only.
		for(QVector<Au3dsGLSLModelInstance*>::iterator mii = m_Models.begin(); mii != m_Models.end(); mii++)
		{
			Au3dsGLSLModelInstance* mi = *mii;
			if(mi->FileName().contains("Head"))
				mi->Render(re, false);
		}
	}
	else
	{
		// Display head pieces and collect info about other pieces we will draw.
		Au3dsGLSLModelInstance* mNeck = 0;
		Au3dsGLSLModelInstance* mTail = 0;
		Au3dsGLSLModelInstance* mTip = 0;
		Au3dsGLSLModelInstance* mWingL = 0;
		Au3dsGLSLModelInstance* mWingR = 0;
		QList<Au3dsGLSLModelInstance*> mBody;	// All parts except heads, necks, wings, tails and tips.
		for(QVector<Au3dsGLSLModelInstance*>::iterator mii = m_Models.begin(); mii != m_Models.end(); mii++)
		{
			Au3dsGLSLModelInstance* mi = *mii;
			if(mi->FileName().contains("Head"))
			{
				if(!headless)
					mi->Render(re, false);
			}
			else if(!bHeadOnly)
			{
				if(!mi->FileName().contains("Wing") && !mi->FileName().contains("Head") &&
					!mi->FileName().contains("Neck") && !mi->FileName().contains("Tail") &&
					!mi->FileName().contains("Tip"))
					mBody.append(mi);
				else if(mi->FileName().contains("WingL"))
					mWingL = mi;
				else if(mi->FileName().contains("WingR"))
					mWingR = mi;
				else if(mi->FileName().contains("Neck"))
					mNeck = mi;
				else if(mi->FileName().contains("Tail"))
					mTail = mi;
				else if(mi->FileName().contains("Tip"))
					mTip = mi;
			}
		}

		// Display neck.
		if(mNeck)
		{
			// Shader is currently configured for skin texture or helmet texture.
			if(HasHelmetPowerArmorOn())
			{
				// Select armor texture for neck, to match the helmet.
				QString sArmorType;
				if(iCurArmor == 0)
				{
					int iType = 0;
					if(m_eHelmetClothesType == CID_MetalPowerArmorHelmet)
						iType = 0;
					else if(m_eHelmetClothesType == CID_MagmiumPowerArmorHelmet)
						iType = 1;
					else if(m_eHelmetClothesType == CID_VulciumPowerArmorHelmet)
						iType = 2;
					else if(m_eHelmetClothesType == CID_VacSuitHelmet)
						iType = 3;
					iCurArmor = m_glTexArmor[iType];
				}
				re.GLSL().BodyRingworldShader().SetClothesTexture(iCurArmor);
			}
			mNeck->Render(re, false);
		}

		// Display wings and tail. They never have armor.
		if(mTail || mTip || mWingL || mWingR)
		{
			// Tail is always skin, not armored.
			re.GLSL().BodyRingworldShader().SetSkinTexture(iCurSkin);
			re.GLSL().BodyRingworldShader().SetClothesTexture(0);
			re.GLSL().BodyRingworldShader().SetClothesTextureOffsetWear(0, 0, 0);	// Skin only
			if(mTail)
				mTail->Render(re, false);
			if(mTip)
				mTip->Render(re, false);

			// Wings use their own texture.
			if(mWingL || mWingR)
			{
				float uOffset = 0;
				float vOffset = (((m_DNA.WingSubType) * 128))/1024.0;
				if(vOffset > 0)
					vOffset = 1-vOffset;

				re.GLSL().BodyRingworldShader().SetSkinTexture(m_glTexWings);
				re.GLSL().BodyRingworldShader().SetSkinTextureOffset(uOffset, vOffset, uOffset, vOffset);

				if(mWingL)
					mWingL->Render(re, false);
				if(mWingR)
					mWingR->Render(re, false);

				// Restore skin texture offsets for the body.
				re.GLSL().BodyRingworldShader().SetSkinTextureOffset(m_skinOffset[0], m_skinOffset[1], m_skinOffset[2],
						m_skinOffset[3]);
			}
		}

		if(!mBody.isEmpty())
		{
			// Select appropriate texture(s) for body parts.
			if(HasBodyPowerArmorOn())
			{
				// Body is wearing power armor.
				re.GLSL().BodyRingworldShader().SetSkinTexture(0);
				re.GLSL().BodyRingworldShader().SetClothesTexture(iCurArmor);
				re.GLSL().BodyRingworldShader().SetClothesTextureOffsetWear(0, 0, 1);	// Clothes only
			}
			else if(iCurClothes && m_eBodyClothesType != CID_Invalid)
			{
				// Body is wearing clothing.
				// Skin offset.
				re.GLSL().BodyRingworldShader().SetSkinTexture(iCurSkin);

				// Clothes offset
				float clothesOffX;
				float clothesOffY;
				switch(m_eBodyClothesType)
				{
				default:
					clothesOffX = 0;
					clothesOffY = 0;
					break;
				case CID_Underwear:
				case CID_LeatherClothes:
				case CID_JeansAndShirt:
				case CID_BlackUniform:
				case CID_BlueUniform:
				case CID_BrownUniform:
				case CID_GoldUniform:
				case CID_GreenUniform:
				case CID_GreyUniform:
				case CID_OrangeUniform:
				case CID_PinkUniform:
				case CID_RedUniform:
				case CID_TealUniform:
				case CID_WhiteUniform:
				case CID_ParcXUniform:
				case CID_GreyBDU:
				case CID_KhakiBDU:
				case CID_OliveDrabBDU:
					clothesOffX = 0;
					clothesOffY = 1;
					break;
				}
				re.GLSL().BodyRingworldShader().SetClothesTexture(iCurClothes);
				re.GLSL().BodyRingworldShader().SetClothesTextureOffsetWear(clothesOffX, clothesOffY, 2);	// Skin and clothes
			}
			else
			{
				// Body is naked.
				// Skin offset.
				re.GLSL().BodyRingworldShader().SetSkinTexture(iCurSkin);

				// Clothes offset
				re.GLSL().BodyRingworldShader().SetClothesTexture(0);
				re.GLSL().BodyRingworldShader().SetClothesTextureOffsetWear(0, 0, 0);	// Skin only
			}

			// Display body pieces
			for(QList<Au3dsGLSLModelInstance*>::iterator mii = mBody.begin(); mii != mBody.end(); mii++)
				(*mii)->Render(re, false);
		}
	}

	re.GLSL().BodyRingworldShader().Deselect();
}

// One of many RenderSkin variations. If you change this one, be sure to apply those changes to the others.
void DpBody::RenderSkinFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, const AuPoint& ambientLightUp,
		bool headless)
{
	Q_UNUSED(projection);
	if(m_Models.isEmpty())
		return;

	bool bHeadOnly = false;
	if(CurrentGeneralPose().HeadOnly())
	{
		if(headless)
			return;
		// Load Identity matrix into bone
		Bone(DhDNA::BONE_Head0).LoadIdentity();
		if(m_DNA.Heads)
			Bone(DhDNA::BONE_Head1).LoadIdentity();

		// Translate bone to current camera location
		AuPoint fTrans;
		if(DNA().Heads && IsRightHanded())
		{
			// Two-headed right-handed creature sees with the second head, which is the right head when there are two.
			bool isLeft = true;	// first head is left
			for(QVector<DpBodyPart*>::const_iterator it = constBegin(); it != constEnd(); it++)
				if((*it)->IsHeadAndNeck())
				{
					if(isLeft)
						isLeft = false;
					else
					{
						fTrans = ((DpHeadAndNeck*)(*it))->CurrentCameraLoc();
						break;
					}
				}
		}
		else
		{
			// One-headed creature or left-handed creature sees with the first head, which is the left head when there are two.
			for(QVector<DpBodyPart*>::const_iterator it = constBegin(); it != constEnd(); it++)
				if((*it)->IsHeadAndNeck())
				{
					fTrans = ((DpHeadAndNeck*)(*it))->CurrentCameraLoc();
					break;
				}
		}
		AuMatrix4x4 headBone;
		headBone.Translate(-fTrans);
		bHeadOnly = true;
		Bone(DhDNA::BONE_Head0) = headBone;
		if(m_DNA.Heads)
			Bone(DhDNA::BONE_Head1) = headBone;
	}

	// Select skin, clothing and armor textures.
	int iCurSkin = 0;
	int iCurClothes = 0;
	int iCurArmor = 0;
	int iCurHelmet = 0;
	{
		// Get current skin texture
		if(m_DNA.SkinType == DhDNA::SKIN_DrySkin || m_DNA.SkinType == DhDNA::SKIN_WetSkin)
		{
			iCurSkin = m_glTexSkin[0];
			iCurClothes = m_glTexClothes[0];
		}
		else if(m_DNA.SkinType == DhDNA::SKIN_DryExoskeleton || m_DNA.SkinType == DhDNA::SKIN_WetExoskeleton)
		{
			iCurSkin = m_glTexSkin[1];
			iCurClothes = m_glTexClothes[1];
		}
		else if(m_DNA.SkinType == DhDNA::SKIN_DryScales || m_DNA.SkinType == DhDNA::SKIN_WetScales)
		{
			iCurSkin = m_glTexSkin[2];
			iCurClothes = m_glTexClothes[2];
		}
		else// if(m_DNA.SkinType == DhDNA::SKIN_Fur || m_DNA.SkinType == DhDNA::SKIN_Feathers)
		{
			iCurSkin = m_glTexSkin[3];
			iCurClothes = m_glTexClothes[3];
		}

		// Get current armor texture
		if(m_eBodyClothesType == CID_VacSuit)
			iCurArmor = m_glTexArmor[0];
		else if(m_eBodyClothesType == CID_MetalPowerArmor)
			iCurArmor = m_glTexArmor[1];
		else if(m_eBodyClothesType == CID_MagmiumPowerArmor)
			iCurArmor = m_glTexArmor[2];
		else if(m_eBodyClothesType == CID_VulciumPowerArmor)
			iCurArmor = m_glTexArmor[3];

		// Get current helmet texture
		int iType = 0;
		if(m_eHelmetClothesType == CID_MetalPowerArmorHelmet)
			iType = 0;
		else if(m_eHelmetClothesType == CID_MagmiumPowerArmorHelmet)
			iType = 1;
		else if(m_eHelmetClothesType == CID_VulciumPowerArmorHelmet)
			iType = 2;
		else if(m_eHelmetClothesType == CID_VacSuitHelmet)
			iType = 3;
		iCurHelmet = m_glTexHelmet[m_DNA.SkinType][iType];
	}

	// Configure shaders.
	float ar, ag, ab, aa;
	re.CurrentAmbientLight(ar, ag, ab, aa);

	float fogR, fogG, fogB, fogDensity;
	re.CurrentFog(fogR, fogG, fogB, fogDensity);

	// Fog, not a ringworld.
	re.GLSL().BodyFogShader().Select(projection, re);
	re.GLSL().BodyFogShader().SetModelView(model);
	re.GLSL().BodyFogShader().SetFog(fogDensity, fogR, fogG, fogB);

	// Ambient light
	re.GLSL().BodyFogShader().SetAmbientLight(ar, ag, ab);
	re.GLSL().BodyFogShader().SetAmbientLightUp(ambientLightUp.x, ambientLightUp.y, ambientLightUp.z);

	// Bone matrices.
	{
		// The shader parameter space is limited in size; when exceeded the
		// shader stops working and nothing appears on the screen.
		// For speed, we only send the bone matrices that we actually use.
		// Torso
		for(unsigned int ib=0; ib<m_DNA.TorsoVertebrae; ib++)
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Torso0 + ib, m_Bones[DhDNA::BONE_Torso0 + ib]);

		// Head(s) and neck(s)
		const int nvc = m_DNA.NeckVertebraeCount();
		re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Head0, m_Bones[DhDNA::BONE_Head0]);
		//RenderBoneMatrix(DhDNA::BONE_Jaw0, vp, pname1, pname2, pname3);
		for(int ib=0; ib<nvc; ib++)
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Neck00 + ib, m_Bones[DhDNA::BONE_Neck00 + ib]);
		if(m_DNA.Heads)
		{
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Head1, m_Bones[DhDNA::BONE_Head1]);
			//RenderBoneMatrix(DhDNA::BONE_Jaw1, vp, pname1, pname2, pname3);
			for(int ib=0; ib<nvc; ib++)
				re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Neck10 + ib, m_Bones[DhDNA::BONE_Neck10 + ib]);
		}

		// Tail
		int tailverts = (unsigned int)m_DNA.TailVertebrae*2;	// Tail
		if(tailverts)
			tailverts--;
		for(int ib=0; ib<=tailverts; ib++)
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Tail0 + ib, m_Bones[DhDNA::BONE_Tail0 + ib]);

		// Arms
		if(m_DNA.ArmPairs)
		{
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Hand0L, m_Bones[DhDNA::BONE_Hand0L]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Hand0R, m_Bones[DhDNA::BONE_Hand0R]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_ForeArm0L, m_Bones[DhDNA::BONE_ForeArm0L]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_ForeArm0R, m_Bones[DhDNA::BONE_ForeArm0R]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_UpperArm0L, m_Bones[DhDNA::BONE_UpperArm0L]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_UpperArm0R, m_Bones[DhDNA::BONE_UpperArm0R]);
		}
		if(m_DNA.ArmPairs >= 2)
		{
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Hand1L, m_Bones[DhDNA::BONE_Hand1L]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Hand1R, m_Bones[DhDNA::BONE_Hand1R]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_ForeArm1L, m_Bones[DhDNA::BONE_ForeArm1L]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_ForeArm1R, m_Bones[DhDNA::BONE_ForeArm1R]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_UpperArm1L, m_Bones[DhDNA::BONE_UpperArm1L]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_UpperArm1R, m_Bones[DhDNA::BONE_UpperArm1R]);
		}
		if(m_DNA.ArmPairs == 3)
		{
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Hand2L, m_Bones[DhDNA::BONE_Hand2L]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Hand2R, m_Bones[DhDNA::BONE_Hand2R]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_ForeArm2L, m_Bones[DhDNA::BONE_ForeArm2L]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_ForeArm2R, m_Bones[DhDNA::BONE_ForeArm2R]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_UpperArm2L, m_Bones[DhDNA::BONE_UpperArm2L]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_UpperArm2R, m_Bones[DhDNA::BONE_UpperArm2R]);
		}

		// Legs
		if(m_DNA.LegPairs)
		{
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Foot0L, m_Bones[DhDNA::BONE_Foot0L]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Foot0R, m_Bones[DhDNA::BONE_Foot0R]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Calf0L, m_Bones[DhDNA::BONE_Calf0L]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Calf0R, m_Bones[DhDNA::BONE_Calf0R]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Thigh0L, m_Bones[DhDNA::BONE_Thigh0L]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Thigh0R, m_Bones[DhDNA::BONE_Thigh0R]);
		}
		if(m_DNA.LegPairs >= 2)
		{
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Foot1L, m_Bones[DhDNA::BONE_Foot1L]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Foot1R, m_Bones[DhDNA::BONE_Foot1R]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Calf1L, m_Bones[DhDNA::BONE_Calf1L]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Calf1R, m_Bones[DhDNA::BONE_Calf1R]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Thigh1L, m_Bones[DhDNA::BONE_Thigh1L]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Thigh1R, m_Bones[DhDNA::BONE_Thigh1R]);
		}
		if(m_DNA.LegPairs == 3)
		{
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Foot2L, m_Bones[DhDNA::BONE_Foot2L]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Foot2R, m_Bones[DhDNA::BONE_Foot2R]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Calf2L, m_Bones[DhDNA::BONE_Calf2L]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Calf2R, m_Bones[DhDNA::BONE_Calf2R]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Thigh2L, m_Bones[DhDNA::BONE_Thigh2L]);
			re.GLSL().BodyFogShader().SetBone(DhDNA::BONE_Thigh2R, m_Bones[DhDNA::BONE_Thigh2R]);
		}
	}

	re.GLSL().BodyFogShader().SetSkinTextureOffset(m_skinOffset[0], m_skinOffset[1], m_skinOffset[2], m_skinOffset[3]);

	// Call the OpenGL display lists of the vertex skinned model.

	// Prepare to draw head or helmet. There is always a head; it might be the only thing we draw.
	if(HasHelmetPowerArmorOn())
	{
		re.GLSL().BodyFogShader().SetClothesTextureOffsetWear(0, 0, 1);	// Clothes only
		re.GLSL().BodyFogShader().SetClothesTexture(iCurHelmet);
		re.GLSL().BodyFogShader().SetSkinTexture(0);
	}
	else
	{
		re.GLSL().BodyFogShader().SetClothesTextureOffsetWear(0, 0, 0);	// Skin only
		re.GLSL().BodyFogShader().SetClothesTexture(0);
		re.GLSL().BodyFogShader().SetSkinTexture(iCurSkin);
	}

	if(bHeadOnly)
	{
		// Display head pieces only.
		for(QVector<Au3dsGLSLModelInstance*>::iterator mii = m_Models.begin(); mii != m_Models.end(); mii++)
		{
			Au3dsGLSLModelInstance* mi = *mii;
			if(mi->FileName().contains("Head"))
				mi->Render(re, false);
		}
	}
	else
	{
		// Display head pieces and collect info about other pieces we will draw.
		Au3dsGLSLModelInstance* mNeck = 0;
		Au3dsGLSLModelInstance* mTail = 0;
		Au3dsGLSLModelInstance* mTip = 0;
		Au3dsGLSLModelInstance* mWingL = 0;
		Au3dsGLSLModelInstance* mWingR = 0;
		QList<Au3dsGLSLModelInstance*> mBody;	// All parts except heads, necks, wings, tails and tips.
		for(QVector<Au3dsGLSLModelInstance*>::iterator mii = m_Models.begin(); mii != m_Models.end(); mii++)
		{
			Au3dsGLSLModelInstance* mi = *mii;
			if(mi->FileName().contains("Head"))
			{
				if(!headless)
					mi->Render(re, false);
			}
			else if(!bHeadOnly)
			{
				if(!mi->FileName().contains("Wing") && !mi->FileName().contains("Head") &&
					!mi->FileName().contains("Neck") && !mi->FileName().contains("Tail") &&
					!mi->FileName().contains("Tip"))
					mBody.append(mi);
				else if(mi->FileName().contains("WingL"))
					mWingL = mi;
				else if(mi->FileName().contains("WingR"))
					mWingR = mi;
				else if(mi->FileName().contains("Neck"))
					mNeck = mi;
				else if(mi->FileName().contains("Tail"))
					mTail = mi;
				else if(mi->FileName().contains("Tip"))
					mTip = mi;
			}
		}

		// Display neck.
		if(mNeck)
		{
			// Shader is currently configured for skin texture or helmet texture.
			if(HasHelmetPowerArmorOn())
			{
				// Select armor texture for neck, to match the helmet.
				QString sArmorType;
				if(iCurArmor == 0)
				{
					int iType = 0;
					if(m_eHelmetClothesType == CID_MetalPowerArmorHelmet)
						iType = 0;
					else if(m_eHelmetClothesType == CID_MagmiumPowerArmorHelmet)
						iType = 1;
					else if(m_eHelmetClothesType == CID_VulciumPowerArmorHelmet)
						iType = 2;
					else if(m_eHelmetClothesType == CID_VacSuitHelmet)
						iType = 3;
					iCurArmor = m_glTexArmor[iType];
				}
				re.GLSL().BodyFogShader().SetClothesTexture(iCurArmor);
			}
			mNeck->Render(re, false);
		}

		// Display wings and tail. They never have armor.
		if(mTail || mTip || mWingL || mWingR)
		{
			// Tail is always skin, not armored.
			re.GLSL().BodyFogShader().SetSkinTexture(iCurSkin);
			re.GLSL().BodyFogShader().SetClothesTexture(0);
			re.GLSL().BodyFogShader().SetClothesTextureOffsetWear(0, 0, 0);	// Skin only
			if(mTail)
				mTail->Render(re, false);
			if(mTip)
				mTip->Render(re, false);

			// Wings use their own texture.
			if(mWingL || mWingR)
			{
				float uOffset = 0;
				float vOffset = (((m_DNA.WingSubType) * 128))/1024.0;
				if(vOffset > 0)
					vOffset = 1-vOffset;

				re.GLSL().BodyFogShader().SetSkinTexture(m_glTexWings);
				re.GLSL().BodyFogShader().SetSkinTextureOffset(uOffset, vOffset, uOffset, vOffset);

				if(mWingL)
					mWingL->Render(re, false);
				if(mWingR)
					mWingR->Render(re, false);

				// Restore skin texture offsets for the body.
				re.GLSL().BodyFogShader().SetSkinTextureOffset(m_skinOffset[0], m_skinOffset[1], m_skinOffset[2], m_skinOffset[3]);
			}
		}

		if(!mBody.isEmpty())
		{
			// Select appropriate texture(s) for body parts.
			if(HasBodyPowerArmorOn())
			{
				// Body is wearing power armor.
				re.GLSL().BodyFogShader().SetSkinTexture(0);
				re.GLSL().BodyFogShader().SetClothesTexture(iCurArmor);
				re.GLSL().BodyFogShader().SetClothesTextureOffsetWear(0, 0, 1);	// Clothes only
			}
			else if(iCurClothes && m_eBodyClothesType != CID_Invalid)
			{
				// Body is wearing clothing.
				// Skin offset.
				re.GLSL().BodyFogShader().SetSkinTexture(iCurSkin);

				// Clothes offset
				float clothesOffX;
				float clothesOffY;
				switch(m_eBodyClothesType)
				{
				default:
					clothesOffX = 0;
					clothesOffY = 0;
					break;
				case CID_Underwear:
				case CID_LeatherClothes:
				case CID_JeansAndShirt:
				case CID_BlackUniform:
				case CID_BlueUniform:
				case CID_BrownUniform:
				case CID_GoldUniform:
				case CID_GreenUniform:
				case CID_GreyUniform:
				case CID_OrangeUniform:
				case CID_PinkUniform:
				case CID_RedUniform:
				case CID_TealUniform:
				case CID_WhiteUniform:
				case CID_ParcXUniform:
				case CID_GreyBDU:
				case CID_KhakiBDU:
				case CID_OliveDrabBDU:
					clothesOffX = 0;
					clothesOffY = 1;
					break;
				}
				re.GLSL().BodyFogShader().SetClothesTexture(iCurClothes);
				re.GLSL().BodyFogShader().SetClothesTextureOffsetWear(clothesOffX, clothesOffY, 2);	// Skin and clothes
			}
			else
			{
				// Body is naked.
				// Skin offset.
				re.GLSL().BodyFogShader().SetSkinTexture(iCurSkin);

				// Clothes offset
				re.GLSL().BodyFogShader().SetClothesTexture(0);
				re.GLSL().BodyFogShader().SetClothesTextureOffsetWear(0, 0, 0);	// Skin only
			}

			// Display body pieces
			for(QList<Au3dsGLSLModelInstance*>::iterator mii = mBody.begin(); mii != mBody.end(); mii++)
				(*mii)->Render(re, false);
		}
	}

	re.GLSL().BodyFogShader().Deselect();
}

// One of many RenderSkin variations. If you change this one, be sure to apply those changes to the others.
void DpBody::RenderSkinFogGlobe(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		const AuPoint& ambientLightUp, const AuPoint& globeP, bool headless)
{
	Q_UNUSED(projection);
	if(m_Models.isEmpty())
		return;

	bool bHeadOnly = false;
	if(CurrentGeneralPose().HeadOnly())
	{
		if(headless)
			return;
		// Load Identity matrix into bone
		Bone(DhDNA::BONE_Head0).LoadIdentity();
		if(m_DNA.Heads)
			Bone(DhDNA::BONE_Head1).LoadIdentity();

		// Translate bone to current camera location
		AuPoint fTrans;
		if(DNA().Heads && IsRightHanded())
		{
			// Two-headed right-handed creature sees with the second head, which is the right head when there are two.
			bool isLeft = true;	// first head is left
			for(QVector<DpBodyPart*>::const_iterator it = constBegin(); it != constEnd(); it++)
				if((*it)->IsHeadAndNeck())
				{
					if(isLeft)
						isLeft = false;
					else
					{
						fTrans = ((DpHeadAndNeck*)(*it))->CurrentCameraLoc();
						break;
					}
				}
		}
		else
		{
			// One-headed creature or left-handed creature sees with the first head, which is the left head when there are two.
			for(QVector<DpBodyPart*>::const_iterator it = constBegin(); it != constEnd(); it++)
				if((*it)->IsHeadAndNeck())
				{
					fTrans = ((DpHeadAndNeck*)(*it))->CurrentCameraLoc();
					break;
				}
		}
		AuMatrix4x4 headBone;
		headBone.Translate(-fTrans);
		bHeadOnly = true;
		Bone(DhDNA::BONE_Head0) = headBone;
		if(m_DNA.Heads)
			Bone(DhDNA::BONE_Head1) = headBone;
	}

	// Select skin, clothing and armor textures.
	int iCurSkin = 0;
	int iCurClothes = 0;
	int iCurArmor = 0;
	int iCurHelmet = 0;
	{
		// Get current skin texture
		if(m_DNA.SkinType == DhDNA::SKIN_DrySkin || m_DNA.SkinType == DhDNA::SKIN_WetSkin)
		{
			iCurSkin = m_glTexSkin[0];
			iCurClothes = m_glTexClothes[0];
		}
		else if(m_DNA.SkinType == DhDNA::SKIN_DryExoskeleton || m_DNA.SkinType == DhDNA::SKIN_WetExoskeleton)
		{
			iCurSkin = m_glTexSkin[1];
			iCurClothes = m_glTexClothes[1];
		}
		else if(m_DNA.SkinType == DhDNA::SKIN_DryScales || m_DNA.SkinType == DhDNA::SKIN_WetScales)
		{
			iCurSkin = m_glTexSkin[2];
			iCurClothes = m_glTexClothes[2];
		}
		else// if(m_DNA.SkinType == DhDNA::SKIN_Fur || m_DNA.SkinType == DhDNA::SKIN_Feathers)
		{
			iCurSkin = m_glTexSkin[3];
			iCurClothes = m_glTexClothes[3];
		}

		// Get current armor texture
		if(m_eBodyClothesType == CID_VacSuit)
			iCurArmor = m_glTexArmor[0];
		else if(m_eBodyClothesType == CID_MetalPowerArmor)
			iCurArmor = m_glTexArmor[1];
		else if(m_eBodyClothesType == CID_MagmiumPowerArmor)
			iCurArmor = m_glTexArmor[2];
		else if(m_eBodyClothesType == CID_VulciumPowerArmor)
			iCurArmor = m_glTexArmor[3];

		// Get current helmet texture
		int iType = 0;
		if(m_eHelmetClothesType == CID_MetalPowerArmorHelmet)
			iType = 0;
		else if(m_eHelmetClothesType == CID_MagmiumPowerArmorHelmet)
			iType = 1;
		else if(m_eHelmetClothesType == CID_VulciumPowerArmorHelmet)
			iType = 2;
		else if(m_eHelmetClothesType == CID_VacSuitHelmet)
			iType = 3;
		iCurHelmet = m_glTexHelmet[m_DNA.SkinType][iType];
	}

	// Configure shaders.
	float ar, ag, ab, aa;
	re.CurrentAmbientLight(ar, ag, ab, aa);

	float fogR, fogG, fogB, fogDensity;
	re.CurrentFog(fogR, fogG, fogB, fogDensity);

	// Fog, not a ringworld.
	re.GLSL().BodyFogGlobeShader().Select(projection, re);
	re.GLSL().BodyFogGlobeShader().SetModelView(model);
	re.GLSL().BodyFogGlobeShader().SetFog(fogDensity, fogR, fogG, fogB);

	// Ambient light
	re.GLSL().BodyFogGlobeShader().SetAmbientLight(ar, ag, ab);
	re.GLSL().BodyFogGlobeShader().SetAmbientLightUp(ambientLightUp.x, ambientLightUp.y, ambientLightUp.z);
	re.GLSL().BodyFogGlobeShader().SetGlobeLocation(globeP.x, globeP.y, globeP.z);

	// Bone matrices.
	{
		// The shader parameter space is limited in size; when exceeded the
		// shader stops working and nothing appears on the screen.
		// For speed, we only send the bone matrices that we actually use.
		// Torso
		for(unsigned int ib=0; ib<m_DNA.TorsoVertebrae; ib++)
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Torso0 + ib, m_Bones[DhDNA::BONE_Torso0 + ib]);

		// Head(s) and neck(s)
		const int nvc = m_DNA.NeckVertebraeCount();
		re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Head0, m_Bones[DhDNA::BONE_Head0]);
		//RenderBoneMatrix(DhDNA::BONE_Jaw0, vp, pname1, pname2, pname3);
		for(int ib=0; ib<nvc; ib++)
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Neck00 + ib, m_Bones[DhDNA::BONE_Neck00 + ib]);
		if(m_DNA.Heads)
		{
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Head1, m_Bones[DhDNA::BONE_Head1]);
			//RenderBoneMatrix(DhDNA::BONE_Jaw1, vp, pname1, pname2, pname3);
			for(int ib=0; ib<nvc; ib++)
				re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Neck10 + ib, m_Bones[DhDNA::BONE_Neck10 + ib]);
		}

		// Tail
		int tailverts = (unsigned int)m_DNA.TailVertebrae*2;	// Tail
		if(tailverts)
			tailverts--;
		for(int ib=0; ib<=tailverts; ib++)
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Tail0 + ib, m_Bones[DhDNA::BONE_Tail0 + ib]);

		// Arms
		if(m_DNA.ArmPairs)
		{
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Hand0L, m_Bones[DhDNA::BONE_Hand0L]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Hand0R, m_Bones[DhDNA::BONE_Hand0R]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_ForeArm0L, m_Bones[DhDNA::BONE_ForeArm0L]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_ForeArm0R, m_Bones[DhDNA::BONE_ForeArm0R]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_UpperArm0L, m_Bones[DhDNA::BONE_UpperArm0L]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_UpperArm0R, m_Bones[DhDNA::BONE_UpperArm0R]);
		}
		if(m_DNA.ArmPairs >= 2)
		{
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Hand1L, m_Bones[DhDNA::BONE_Hand1L]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Hand1R, m_Bones[DhDNA::BONE_Hand1R]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_ForeArm1L, m_Bones[DhDNA::BONE_ForeArm1L]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_ForeArm1R, m_Bones[DhDNA::BONE_ForeArm1R]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_UpperArm1L, m_Bones[DhDNA::BONE_UpperArm1L]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_UpperArm1R, m_Bones[DhDNA::BONE_UpperArm1R]);
		}
		if(m_DNA.ArmPairs == 3)
		{
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Hand2L, m_Bones[DhDNA::BONE_Hand2L]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Hand2R, m_Bones[DhDNA::BONE_Hand2R]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_ForeArm2L, m_Bones[DhDNA::BONE_ForeArm2L]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_ForeArm2R, m_Bones[DhDNA::BONE_ForeArm2R]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_UpperArm2L, m_Bones[DhDNA::BONE_UpperArm2L]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_UpperArm2R, m_Bones[DhDNA::BONE_UpperArm2R]);
		}

		// Legs
		if(m_DNA.LegPairs)
		{
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Foot0L, m_Bones[DhDNA::BONE_Foot0L]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Foot0R, m_Bones[DhDNA::BONE_Foot0R]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Calf0L, m_Bones[DhDNA::BONE_Calf0L]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Calf0R, m_Bones[DhDNA::BONE_Calf0R]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Thigh0L, m_Bones[DhDNA::BONE_Thigh0L]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Thigh0R, m_Bones[DhDNA::BONE_Thigh0R]);
		}
		if(m_DNA.LegPairs >= 2)
		{
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Foot1L, m_Bones[DhDNA::BONE_Foot1L]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Foot1R, m_Bones[DhDNA::BONE_Foot1R]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Calf1L, m_Bones[DhDNA::BONE_Calf1L]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Calf1R, m_Bones[DhDNA::BONE_Calf1R]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Thigh1L, m_Bones[DhDNA::BONE_Thigh1L]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Thigh1R, m_Bones[DhDNA::BONE_Thigh1R]);
		}
		if(m_DNA.LegPairs == 3)
		{
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Foot2L, m_Bones[DhDNA::BONE_Foot2L]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Foot2R, m_Bones[DhDNA::BONE_Foot2R]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Calf2L, m_Bones[DhDNA::BONE_Calf2L]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Calf2R, m_Bones[DhDNA::BONE_Calf2R]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Thigh2L, m_Bones[DhDNA::BONE_Thigh2L]);
			re.GLSL().BodyFogGlobeShader().SetBone(DhDNA::BONE_Thigh2R, m_Bones[DhDNA::BONE_Thigh2R]);
		}
	}

	re.GLSL().BodyFogGlobeShader().SetSkinTextureOffset(m_skinOffset[0], m_skinOffset[1], m_skinOffset[2], m_skinOffset[3]);

	// Call the OpenGL display lists of the vertex skinned model.

	// Prepare to draw head or helmet. There is always a head; it might be the only thing we draw.
	if(HasHelmetPowerArmorOn())
	{
		re.GLSL().BodyFogGlobeShader().SetClothesTextureOffsetWear(0, 0, 1);	// Clothes only
		re.GLSL().BodyFogGlobeShader().SetClothesTexture(iCurHelmet);
		re.GLSL().BodyFogGlobeShader().SetSkinTexture(0);
	}
	else
	{
		re.GLSL().BodyFogGlobeShader().SetClothesTextureOffsetWear(0, 0, 0);	// Skin only
		re.GLSL().BodyFogGlobeShader().SetClothesTexture(0);
		re.GLSL().BodyFogGlobeShader().SetSkinTexture(iCurSkin);
	}

	if(bHeadOnly)
	{
		// Display head pieces only.
		for(QVector<Au3dsGLSLModelInstance*>::iterator mii = m_Models.begin(); mii != m_Models.end(); mii++)
		{
			Au3dsGLSLModelInstance* mi = *mii;
			if(mi->FileName().contains("Head"))
				mi->Render(re, false);
		}
	}
	else
	{
		// Display head pieces and collect info about other pieces we will draw.
		Au3dsGLSLModelInstance* mNeck = 0;
		Au3dsGLSLModelInstance* mTail = 0;
		Au3dsGLSLModelInstance* mTip = 0;
		Au3dsGLSLModelInstance* mWingL = 0;
		Au3dsGLSLModelInstance* mWingR = 0;
		QList<Au3dsGLSLModelInstance*> mBody;	// All parts except heads, necks, wings, tails and tips.
		for(QVector<Au3dsGLSLModelInstance*>::iterator mii = m_Models.begin(); mii != m_Models.end(); mii++)
		{
			Au3dsGLSLModelInstance* mi = *mii;
			if(mi->FileName().contains("Head"))
			{
				if(!headless)
					mi->Render(re, false);
			}
			else if(!bHeadOnly)
			{
				if(!mi->FileName().contains("Wing") && !mi->FileName().contains("Head") &&
					!mi->FileName().contains("Neck") && !mi->FileName().contains("Tail") &&
					!mi->FileName().contains("Tip"))
					mBody.append(mi);
				else if(mi->FileName().contains("WingL"))
					mWingL = mi;
				else if(mi->FileName().contains("WingR"))
					mWingR = mi;
				else if(mi->FileName().contains("Neck"))
					mNeck = mi;
				else if(mi->FileName().contains("Tail"))
					mTail = mi;
				else if(mi->FileName().contains("Tip"))
					mTip = mi;
			}
		}

		// Display neck.
		if(mNeck)
		{
			// Shader is currently configured for skin texture or helmet texture.
			if(HasHelmetPowerArmorOn())
			{
				// Select armor texture for neck, to match the helmet.
				QString sArmorType;
				if(iCurArmor == 0)
				{
					int iType = 0;
					if(m_eHelmetClothesType == CID_MetalPowerArmorHelmet)
						iType = 0;
					else if(m_eHelmetClothesType == CID_MagmiumPowerArmorHelmet)
						iType = 1;
					else if(m_eHelmetClothesType == CID_VulciumPowerArmorHelmet)
						iType = 2;
					else if(m_eHelmetClothesType == CID_VacSuitHelmet)
						iType = 3;
					iCurArmor = m_glTexArmor[iType];
				}
				re.GLSL().BodyFogGlobeShader().SetClothesTexture(iCurArmor);
			}
			mNeck->Render(re, false);
		}

		// Display wings and tail. They never have armor.
		if(mTail || mTip || mWingL || mWingR)
		{
			// Tail is always skin, not armored.
			re.GLSL().BodyFogGlobeShader().SetSkinTexture(iCurSkin);
			re.GLSL().BodyFogGlobeShader().SetClothesTexture(0);
			re.GLSL().BodyFogGlobeShader().SetClothesTextureOffsetWear(0, 0, 0);	// Skin only
			if(mTail)
				mTail->Render(re, false);
			if(mTip)
				mTip->Render(re, false);

			// Wings use their own texture.
			if(mWingL || mWingR)
			{
				float uOffset = 0;
				float vOffset = (((m_DNA.WingSubType) * 128))/1024.0;
				if(vOffset > 0)
					vOffset = 1-vOffset;

				re.GLSL().BodyFogGlobeShader().SetSkinTexture(m_glTexWings);
				re.GLSL().BodyFogGlobeShader().SetSkinTextureOffset(uOffset, vOffset, uOffset, vOffset);

				if(mWingL)
					mWingL->Render(re, false);
				if(mWingR)
					mWingR->Render(re, false);

				// Restore skin texture offsets for the body.
				re.GLSL().BodyFogGlobeShader().SetSkinTextureOffset(m_skinOffset[0], m_skinOffset[1], m_skinOffset[2],
						m_skinOffset[3]);
			}
		}

		if(!mBody.isEmpty())
		{
			// Select appropriate texture(s) for body parts.
			if(HasBodyPowerArmorOn())
			{
				// Body is wearing power armor.
				re.GLSL().BodyFogGlobeShader().SetSkinTexture(0);
				re.GLSL().BodyFogGlobeShader().SetClothesTexture(iCurArmor);
				re.GLSL().BodyFogGlobeShader().SetClothesTextureOffsetWear(0, 0, 1);	// Clothes only
			}
			else if(iCurClothes && m_eBodyClothesType != CID_Invalid)
			{
				// Body is wearing clothing.
				// Skin offset.
				re.GLSL().BodyFogGlobeShader().SetSkinTexture(iCurSkin);

				// Clothes offset
				float clothesOffX;
				float clothesOffY;
				switch(m_eBodyClothesType)
				{
				default:
					clothesOffX = 0;
					clothesOffY = 0;
					break;
				case CID_Underwear:
				case CID_LeatherClothes:
				case CID_JeansAndShirt:
				case CID_BlackUniform:
				case CID_BlueUniform:
				case CID_BrownUniform:
				case CID_GoldUniform:
				case CID_GreenUniform:
				case CID_GreyUniform:
				case CID_OrangeUniform:
				case CID_PinkUniform:
				case CID_RedUniform:
				case CID_TealUniform:
				case CID_WhiteUniform:
				case CID_ParcXUniform:
				case CID_GreyBDU:
				case CID_KhakiBDU:
				case CID_OliveDrabBDU:
					clothesOffX = 0;
					clothesOffY = 1;
					break;
				}
				re.GLSL().BodyFogGlobeShader().SetClothesTexture(iCurClothes);
				re.GLSL().BodyFogGlobeShader().SetClothesTextureOffsetWear(clothesOffX, clothesOffY, 2);	// Skin and clothes
			}
			else
			{
				// Body is naked.
				// Skin offset.
				re.GLSL().BodyFogGlobeShader().SetSkinTexture(iCurSkin);

				// Clothes offset
				re.GLSL().BodyFogGlobeShader().SetClothesTexture(0);
				re.GLSL().BodyFogGlobeShader().SetClothesTextureOffsetWear(0, 0, 0);	// Skin only
			}

			// Display body pieces
			for(QList<Au3dsGLSLModelInstance*>::iterator mii = mBody.begin(); mii != mBody.end(); mii++)
				(*mii)->Render(re, false);
		}
	}

	re.GLSL().BodyFogGlobeShader().Deselect();
}

// One of many RenderSkin variations. If you change this one, be sure to apply those changes to the others.
void DpBody::RenderSkinFogRingworld(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		const AuPoint& ambientLightUp, float shadowPanelSunAngles[MAX_SUNS], bool headless)
{
	Q_UNUSED(projection);
	if(m_Models.isEmpty())
		return;

	bool bHeadOnly = false;
	if(CurrentGeneralPose().HeadOnly())
	{
		if(headless)
			return;
		// Load Identity matrix into bone
		Bone(DhDNA::BONE_Head0).LoadIdentity();
		if(m_DNA.Heads)
			Bone(DhDNA::BONE_Head1).LoadIdentity();

		// Translate bone to current camera location
		AuPoint fTrans;
		if(DNA().Heads && IsRightHanded())
		{
			// Two-headed right-handed creature sees with the second head, which is the right head when there are two.
			bool isLeft = true;	// first head is left
			for(QVector<DpBodyPart*>::const_iterator it = constBegin(); it != constEnd(); it++)
				if((*it)->IsHeadAndNeck())
				{
					if(isLeft)
						isLeft = false;
					else
					{
						fTrans = ((DpHeadAndNeck*)(*it))->CurrentCameraLoc();
						break;
					}
				}
		}
		else
		{
			// One-headed creature or left-handed creature sees with the first head, which is the left head when there are two.
			for(QVector<DpBodyPart*>::const_iterator it = constBegin(); it != constEnd(); it++)
				if((*it)->IsHeadAndNeck())
				{
					fTrans = ((DpHeadAndNeck*)(*it))->CurrentCameraLoc();
					break;
				}
		}
		AuMatrix4x4 headBone;
		headBone.Translate(-fTrans);
		bHeadOnly = true;
		Bone(DhDNA::BONE_Head0) = headBone;
		if(m_DNA.Heads)
			Bone(DhDNA::BONE_Head1) = headBone;
	}

	// Select skin, clothing and armor textures.
	int iCurSkin = 0;
	int iCurClothes = 0;
	int iCurArmor = 0;
	int iCurHelmet = 0;
	{
		// Get current skin texture
		if(m_DNA.SkinType == DhDNA::SKIN_DrySkin || m_DNA.SkinType == DhDNA::SKIN_WetSkin)
		{
			iCurSkin = m_glTexSkin[0];
			iCurClothes = m_glTexClothes[0];
		}
		else if(m_DNA.SkinType == DhDNA::SKIN_DryExoskeleton || m_DNA.SkinType == DhDNA::SKIN_WetExoskeleton)
		{
			iCurSkin = m_glTexSkin[1];
			iCurClothes = m_glTexClothes[1];
		}
		else if(m_DNA.SkinType == DhDNA::SKIN_DryScales || m_DNA.SkinType == DhDNA::SKIN_WetScales)
		{
			iCurSkin = m_glTexSkin[2];
			iCurClothes = m_glTexClothes[2];
		}
		else// if(m_DNA.SkinType == DhDNA::SKIN_Fur || m_DNA.SkinType == DhDNA::SKIN_Feathers)
		{
			iCurSkin = m_glTexSkin[3];
			iCurClothes = m_glTexClothes[3];
		}

		// Get current armor texture
		if(m_eBodyClothesType == CID_VacSuit)
			iCurArmor = m_glTexArmor[0];
		else if(m_eBodyClothesType == CID_MetalPowerArmor)
			iCurArmor = m_glTexArmor[1];
		else if(m_eBodyClothesType == CID_MagmiumPowerArmor)
			iCurArmor = m_glTexArmor[2];
		else if(m_eBodyClothesType == CID_VulciumPowerArmor)
			iCurArmor = m_glTexArmor[3];

		// Get current helmet texture
		int iType = 0;
		if(m_eHelmetClothesType == CID_MetalPowerArmorHelmet)
			iType = 0;
		else if(m_eHelmetClothesType == CID_MagmiumPowerArmorHelmet)
			iType = 1;
		else if(m_eHelmetClothesType == CID_VulciumPowerArmorHelmet)
			iType = 2;
		else if(m_eHelmetClothesType == CID_VacSuitHelmet)
			iType = 3;
		iCurHelmet = m_glTexHelmet[m_DNA.SkinType][iType];
	}

	// Configure shaders.
	float ar, ag, ab, aa;
	re.CurrentAmbientLight(ar, ag, ab, aa);

	float fogR, fogG, fogB, fogDensity;
	re.CurrentFog(fogR, fogG, fogB, fogDensity);

	// Fog, on a ringworld.
	re.GLSL().BodyFogRingworldShader().Select(projection, re);
	re.GLSL().BodyFogRingworldShader().SetModelView(model);
	re.GLSL().BodyFogRingworldShader().SetFog(fogDensity, fogR, fogG, fogB);

	// Ambient light
	re.GLSL().BodyFogRingworldShader().SetAmbientLight(ar, ag, ab);
	re.GLSL().BodyFogRingworldShader().SetAmbientLightUp(ambientLightUp.x, ambientLightUp.y, ambientLightUp.z);

	// Suns
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		re.GLSL().BodyFogRingworldShader().SetSunShadowAngle(sunIndex, shadowPanelSunAngles[sunIndex]);

	// Bone matrices.
	{
		// The shader parameter space is limited in size; when exceeded the
		// shader stops working and nothing appears on the screen.
		// For speed, we only send the bone matrices that we actually use.
		// Torso
		for(unsigned int ib=0; ib<m_DNA.TorsoVertebrae; ib++)
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Torso0 + ib, m_Bones[DhDNA::BONE_Torso0 + ib]);

		// Head(s) and neck(s)
		const int nvc = m_DNA.NeckVertebraeCount();
		re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Head0, m_Bones[DhDNA::BONE_Head0]);
		//RenderBoneMatrix(DhDNA::BONE_Jaw0, vp, pname1, pname2, pname3);
		for(int ib=0; ib<nvc; ib++)
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Neck00 + ib, m_Bones[DhDNA::BONE_Neck00 + ib]);
		if(m_DNA.Heads)
		{
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Head1, m_Bones[DhDNA::BONE_Head1]);
			//RenderBoneMatrix(DhDNA::BONE_Jaw1, vp, pname1, pname2, pname3);
			for(int ib=0; ib<nvc; ib++)
				re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Neck10 + ib, m_Bones[DhDNA::BONE_Neck10 + ib]);
		}

		// Tail
		int tailverts = (unsigned int)m_DNA.TailVertebrae*2;	// Tail
		if(tailverts)
			tailverts--;
		for(int ib=0; ib<=tailverts; ib++)
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Tail0 + ib, m_Bones[DhDNA::BONE_Tail0 + ib]);

		// Arms
		if(m_DNA.ArmPairs)
		{
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Hand0L, m_Bones[DhDNA::BONE_Hand0L]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Hand0R, m_Bones[DhDNA::BONE_Hand0R]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_ForeArm0L, m_Bones[DhDNA::BONE_ForeArm0L]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_ForeArm0R, m_Bones[DhDNA::BONE_ForeArm0R]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_UpperArm0L, m_Bones[DhDNA::BONE_UpperArm0L]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_UpperArm0R, m_Bones[DhDNA::BONE_UpperArm0R]);
		}
		if(m_DNA.ArmPairs >= 2)
		{
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Hand1L, m_Bones[DhDNA::BONE_Hand1L]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Hand1R, m_Bones[DhDNA::BONE_Hand1R]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_ForeArm1L, m_Bones[DhDNA::BONE_ForeArm1L]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_ForeArm1R, m_Bones[DhDNA::BONE_ForeArm1R]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_UpperArm1L, m_Bones[DhDNA::BONE_UpperArm1L]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_UpperArm1R, m_Bones[DhDNA::BONE_UpperArm1R]);
		}
		if(m_DNA.ArmPairs == 3)
		{
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Hand2L, m_Bones[DhDNA::BONE_Hand2L]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Hand2R, m_Bones[DhDNA::BONE_Hand2R]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_ForeArm2L, m_Bones[DhDNA::BONE_ForeArm2L]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_ForeArm2R, m_Bones[DhDNA::BONE_ForeArm2R]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_UpperArm2L, m_Bones[DhDNA::BONE_UpperArm2L]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_UpperArm2R, m_Bones[DhDNA::BONE_UpperArm2R]);
		}

		// Legs
		if(m_DNA.LegPairs)
		{
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Foot0L, m_Bones[DhDNA::BONE_Foot0L]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Foot0R, m_Bones[DhDNA::BONE_Foot0R]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Calf0L, m_Bones[DhDNA::BONE_Calf0L]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Calf0R, m_Bones[DhDNA::BONE_Calf0R]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Thigh0L, m_Bones[DhDNA::BONE_Thigh0L]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Thigh0R, m_Bones[DhDNA::BONE_Thigh0R]);
		}
		if(m_DNA.LegPairs >= 2)
		{
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Foot1L, m_Bones[DhDNA::BONE_Foot1L]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Foot1R, m_Bones[DhDNA::BONE_Foot1R]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Calf1L, m_Bones[DhDNA::BONE_Calf1L]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Calf1R, m_Bones[DhDNA::BONE_Calf1R]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Thigh1L, m_Bones[DhDNA::BONE_Thigh1L]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Thigh1R, m_Bones[DhDNA::BONE_Thigh1R]);
		}
		if(m_DNA.LegPairs == 3)
		{
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Foot2L, m_Bones[DhDNA::BONE_Foot2L]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Foot2R, m_Bones[DhDNA::BONE_Foot2R]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Calf2L, m_Bones[DhDNA::BONE_Calf2L]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Calf2R, m_Bones[DhDNA::BONE_Calf2R]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Thigh2L, m_Bones[DhDNA::BONE_Thigh2L]);
			re.GLSL().BodyFogRingworldShader().SetBone(DhDNA::BONE_Thigh2R, m_Bones[DhDNA::BONE_Thigh2R]);
		}
	}

	re.GLSL().BodyFogRingworldShader().SetSkinTextureOffset(m_skinOffset[0], m_skinOffset[1], m_skinOffset[2],
			m_skinOffset[3]);

	// Call the OpenGL display lists of the vertex skinned model.

	// Prepare to draw head or helmet. There is always a head; it might be the only thing we draw.
	if(HasHelmetPowerArmorOn())
	{
		re.GLSL().BodyFogRingworldShader().SetClothesTextureOffsetWear(0, 0, 1);	// Clothes only
		re.GLSL().BodyFogRingworldShader().SetClothesTexture(iCurHelmet);
		re.GLSL().BodyFogRingworldShader().SetSkinTexture(0);
	}
	else
	{
		re.GLSL().BodyFogRingworldShader().SetClothesTextureOffsetWear(0, 0, 0);	// Skin only
		re.GLSL().BodyFogRingworldShader().SetClothesTexture(0);
		re.GLSL().BodyFogRingworldShader().SetSkinTexture(iCurSkin);
	}

	if(bHeadOnly)
	{
		// Display head pieces only.
		for(QVector<Au3dsGLSLModelInstance*>::iterator mii = m_Models.begin(); mii != m_Models.end(); mii++)
		{
			Au3dsGLSLModelInstance* mi = *mii;
			if(mi->FileName().contains("Head"))
				mi->Render(re, false);
		}
	}
	else
	{
		// Display head pieces and collect info about other pieces we will draw.
		Au3dsGLSLModelInstance* mNeck = 0;
		Au3dsGLSLModelInstance* mTail = 0;
		Au3dsGLSLModelInstance* mTip = 0;
		Au3dsGLSLModelInstance* mWingL = 0;
		Au3dsGLSLModelInstance* mWingR = 0;
		QList<Au3dsGLSLModelInstance*> mBody;	// All parts except heads, necks, wings, tails and tips.
		for(QVector<Au3dsGLSLModelInstance*>::iterator mii = m_Models.begin(); mii != m_Models.end(); mii++)
		{
			Au3dsGLSLModelInstance* mi = *mii;
			if(mi->FileName().contains("Head"))
			{
				if(!headless)
					mi->Render(re, false);
			}
			else if(!bHeadOnly)
			{
				if(!mi->FileName().contains("Wing") && !mi->FileName().contains("Head") &&
					!mi->FileName().contains("Neck") && !mi->FileName().contains("Tail") &&
					!mi->FileName().contains("Tip"))
					mBody.append(mi);
				else if(mi->FileName().contains("WingL"))
					mWingL = mi;
				else if(mi->FileName().contains("WingR"))
					mWingR = mi;
				else if(mi->FileName().contains("Neck"))
					mNeck = mi;
				else if(mi->FileName().contains("Tail"))
					mTail = mi;
				else if(mi->FileName().contains("Tip"))
					mTip = mi;
			}
		}

		// Display neck.
		if(mNeck)
		{
			// Shader is currently configured for skin texture or helmet texture.
			if(HasHelmetPowerArmorOn())
			{
				// Select armor texture for neck, to match the helmet.
				QString sArmorType;
				if(iCurArmor == 0)
				{
					int iType = 0;
					if(m_eHelmetClothesType == CID_MetalPowerArmorHelmet)
						iType = 0;
					else if(m_eHelmetClothesType == CID_MagmiumPowerArmorHelmet)
						iType = 1;
					else if(m_eHelmetClothesType == CID_VulciumPowerArmorHelmet)
						iType = 2;
					else if(m_eHelmetClothesType == CID_VacSuitHelmet)
						iType = 3;
					iCurArmor = m_glTexArmor[iType];
				}
				re.GLSL().BodyFogRingworldShader().SetClothesTexture(iCurArmor);
			}
			mNeck->Render(re, false);
		}

		// Display wings and tail. They never have armor.
		if(mTail || mTip || mWingL || mWingR)
		{
			// Tail is always skin, not armored.
			re.GLSL().BodyFogRingworldShader().SetSkinTexture(iCurSkin);
			re.GLSL().BodyFogRingworldShader().SetClothesTexture(0);
			re.GLSL().BodyFogRingworldShader().SetClothesTextureOffsetWear(0, 0, 0);	// Skin only
			if(mTail)
				mTail->Render(re, false);
			if(mTip)
				mTip->Render(re, false);

			// Wings use their own texture.
			if(mWingL || mWingR)
			{
				float uOffset = 0;
				float vOffset = (((m_DNA.WingSubType) * 128))/1024.0;
				if(vOffset > 0)
					vOffset = 1-vOffset;

				re.GLSL().BodyFogRingworldShader().SetSkinTexture(m_glTexWings);
				re.GLSL().BodyFogRingworldShader().SetSkinTextureOffset(uOffset, vOffset, uOffset, vOffset);

				if(mWingL)
					mWingL->Render(re, false);
				if(mWingR)
					mWingR->Render(re, false);

				// Restore skin texture offsets for the body.
				re.GLSL().BodyFogRingworldShader().SetSkinTextureOffset(m_skinOffset[0], m_skinOffset[1], m_skinOffset[2],
						m_skinOffset[3]);
			}
		}

		if(!mBody.isEmpty())
		{
			// Select appropriate texture(s) for body parts.
			if(HasBodyPowerArmorOn())
			{
				// Body is wearing power armor.
				re.GLSL().BodyFogRingworldShader().SetSkinTexture(0);
				re.GLSL().BodyFogRingworldShader().SetClothesTexture(iCurArmor);
				re.GLSL().BodyFogRingworldShader().SetClothesTextureOffsetWear(0, 0, 1);	// Clothes only
			}
			else if(iCurClothes && m_eBodyClothesType != CID_Invalid)
			{
				// Body is wearing clothing.
				// Skin offset.
				re.GLSL().BodyFogRingworldShader().SetSkinTexture(iCurSkin);

				// Clothes offset
				float clothesOffX;
				float clothesOffY;
				switch(m_eBodyClothesType)
				{
				default:
					clothesOffX = 0;
					clothesOffY = 0;
					break;
				case CID_Underwear:
				case CID_LeatherClothes:
				case CID_JeansAndShirt:
				case CID_BlackUniform:
				case CID_BlueUniform:
				case CID_BrownUniform:
				case CID_GoldUniform:
				case CID_GreenUniform:
				case CID_GreyUniform:
				case CID_OrangeUniform:
				case CID_PinkUniform:
				case CID_RedUniform:
				case CID_TealUniform:
				case CID_WhiteUniform:
				case CID_ParcXUniform:
				case CID_GreyBDU:
				case CID_KhakiBDU:
				case CID_OliveDrabBDU:
					clothesOffX = 0;
					clothesOffY = 1;
					break;
				}
				re.GLSL().BodyFogRingworldShader().SetClothesTexture(iCurClothes);
				re.GLSL().BodyFogRingworldShader().SetClothesTextureOffsetWear(clothesOffX, clothesOffY, 2);	// Skin and clothes
			}
			else
			{
				// Body is naked.
				// Skin offset.
				re.GLSL().BodyFogRingworldShader().SetSkinTexture(iCurSkin);

				// Clothes offset
				re.GLSL().BodyFogRingworldShader().SetClothesTexture(0);
				re.GLSL().BodyFogRingworldShader().SetClothesTextureOffsetWear(0, 0, 0);	// Skin only
			}

			// Display body pieces
			for(QList<Au3dsGLSLModelInstance*>::iterator mii = mBody.begin(); mii != mBody.end(); mii++)
				(*mii)->Render(re, false);
		}
	}

	re.GLSL().BodyFogRingworldShader().Deselect();
}

void DpBody::SetDNA(const DhDNA& dna, float speedEnc)
{
	// If the DNA is not changed, do nothing.
	if(dna == m_DNA)
		return;

	// Delete any existing gl models and body parts that were created using my old dna.
	m_Models.clear();
	DeleteBodyParts();

	// Create nu body parts based on my nu dna.
	m_DNA = dna;

	// Forget values that were cached based on previous body/dna.
	ResetCache();

	// Do not init the body on the scene master. There is no model cache on the master so the Init would cause a crash.
	// The limbo rescue code ends up here but it doesn't need the body geometry anyway.
	if(!g_Au.IsSceneMaster())
	{
		Init();

		// Pose nu body parts to current pose.
		AnimatePose(false, speedEnc);
	}
}

bool DpBody::Pose(const AuPose pose[PS_Count], bool bendDominantArm, float speedEnc)
{
	bool anychange = false;
	for(int ps=0; ps<DpBody::PS_Count; ps++)
	{
		if(m_Pose[ps] != pose[ps])
		{
			m_Pose[ps] = pose[ps];
			anychange = true;
		}
	}

	if(bendDominantArm != m_BendArm)
	{
		m_BendArm = bendDominantArm;
		anychange = true;
	}

	AnimatePose(bendDominantArm, speedEnc);
	return anychange;
}

bool DpBody::PoseAttackFlying()
{
	// Weapon info.
	AuDNABase::WEAPONLOC weaponLoc;
	AuCombatWeapon weapon(WM_Not);
	BestWeaponAtRange(0, weaponLoc, weapon);

	// Configure pose for each body section.
	AuPose pose[PS_Count];
	InitPoses(AuPose::FlyFast, pose);

	switch(weaponLoc)
	{
	case DhDNA::WL_None:
		break;
	case DhDNA::WL_Eyes:
	case DhDNA::WL_Head:
		pose[PS_LeftHeadNeck] = AuPose::Bite;
		pose[PS_RightHeadNeck] = AuPose::Bite;
		pose[PS_UpperTorso] = AuPose::Strike;
		break;
	case DhDNA::WL_Hands:
		{
			AuPose::POSE attackpose;
			if(weapon.m_Mode == WM_Contact)
				attackpose = AuPose::Strike;
			else
				attackpose = AuPose::Psionic;
			if(IsRightHanded())
			{
				pose[PS_LeftArm] = AuPose::Block;
				pose[PS_RightArm] = attackpose;
			}
			else
			{
				pose[PS_LeftArm] = attackpose;
				pose[PS_RightArm] = AuPose::Block;
			}
		}
		break;
	case DhDNA::WL_Feet:
		pose[PS_FrontLegs] = AuPose::Stomp;
		pose[PS_RearLegs] = AuPose::Kick;
		break;
	case DhDNA::WL_Tail:
	case DhDNA::WL_Anus:
		pose[PS_Tail] = AuPose::Strike;
		break;
	case DhDNA::WL_Torso:
		pose[PS_UpperTorso] = AuPose::Strike;
		break;
	}

	return Pose(pose, false, 1);
}

bool DpBody::PoseAttackHovering()
{
	// Weapon info.
	AuDNABase::WEAPONLOC weaponLoc;
	AuCombatWeapon weapon(WM_Not);
	BestWeaponAtRange(0, weaponLoc, weapon);

	// Configure pose for each body section.
	AuPose pose[PS_Count];
	InitPoses(AuPose::Hover, pose);

	switch(weaponLoc)
	{
	case DhDNA::WL_None:
		break;
	case DhDNA::WL_Eyes:
	case DhDNA::WL_Head:
		pose[PS_LeftHeadNeck] = AuPose::Bite;
		pose[PS_RightHeadNeck] = AuPose::Bite;
		pose[PS_UpperTorso] = AuPose::Strike;
		break;
	case DhDNA::WL_Hands:
		{
			AuPose::POSE attackpose;
			if(weapon.m_Mode == WM_Contact)
				attackpose = AuPose::Strike;
			else
				attackpose = AuPose::Psionic;
			if(IsRightHanded())
			{
				pose[PS_LeftArm] = AuPose::Block;
				pose[PS_RightArm] = attackpose;
			}
			else
			{
				pose[PS_LeftArm] = attackpose;
				pose[PS_RightArm] = AuPose::Block;
			}
		}
		break;
	case DhDNA::WL_Feet:
		pose[PS_FrontLegs] = AuPose::Stomp;
		pose[PS_RearLegs] = AuPose::Kick;
		break;
	case DhDNA::WL_Tail:
	case DhDNA::WL_Anus:
		pose[PS_Tail] = AuPose::Strike;
		break;
	case DhDNA::WL_Torso:
		pose[PS_UpperTorso] = AuPose::Strike;
		break;
	}

	return Pose(pose, false, 1);
}

bool DpBody::PoseAttackStanding()
{
	// Weapon info.
	AuDNABase::WEAPONLOC weaponLoc;
	AuCombatWeapon weapon(WM_Not);
	BestWeaponAtRange(0, weaponLoc, weapon);

	// Configure pose for each body section.
	AuPose pose[PS_Count];
	InitPoses(AuPose::Stand, pose);
	pose[PS_UpperTorso] = AuPose::Upright;		// rear up when attacking
	pose[PS_Tail] = AuPose::Lash;

	switch(weaponLoc)
	{
	case DhDNA::WL_None:
		break;
	case DhDNA::WL_Eyes:
	case DhDNA::WL_Head:
		pose[PS_LeftHeadNeck] = AuPose::Bite;
		pose[PS_RightHeadNeck] = AuPose::Bite;
		pose[PS_UpperTorso] = AuPose::Strike;
		break;
	case DhDNA::WL_Hands:
		{
			AuPose::POSE attackpose;
			if(weapon.m_Mode == WM_Contact)
				attackpose = AuPose::Strike;
			else
				attackpose = AuPose::Psionic;
			if(IsRightHanded())
			{
				pose[PS_LeftArm] = AuPose::Block;
				pose[PS_RightArm] = attackpose;
			}
			else
			{
				pose[PS_LeftArm] = attackpose;
				pose[PS_RightArm] = AuPose::Block;
			}
		}
		break;
	case DhDNA::WL_Feet:
		pose[PS_FrontLegs] = AuPose::Stomp;
		break;
	case DhDNA::WL_Tail:
	case DhDNA::WL_Anus:
		pose[PS_Tail] = AuPose::Strike;
		break;
	case DhDNA::WL_Torso:
		pose[PS_UpperTorso] = AuPose::Strike;
		break;
	}

	return Pose(pose, false, 1);
}

bool DpBody::PoseAttackUnderGround()
{
	return PoseAttackStanding();
}

bool DpBody::PoseAttackUnderWater()
{
	// Weapon info.
	AuDNABase::WEAPONLOC weaponLoc;
	AuCombatWeapon weapon(WM_Not);
	BestWeaponAtRange(0, weaponLoc, weapon);

	// Configure pose for each body section.
	AuPose pose[PS_Count];
	InitPoses(AuPose::TreadWater, pose);
	pose[PS_UpperTorso] = AuPose::Upright;		// rear up when attacking

	switch(weaponLoc)
	{
	case DhDNA::WL_None:
		break;
	case DhDNA::WL_Eyes:
	case DhDNA::WL_Head:
		pose[PS_LeftHeadNeck] = AuPose::Bite;
		pose[PS_RightHeadNeck] = AuPose::Bite;
		pose[PS_UpperTorso] = AuPose::Strike;
		break;
	case DhDNA::WL_Hands:
		{
			AuPose::POSE attackpose;
			if(weapon.m_Mode == WM_Contact)
				attackpose = AuPose::Strike;
			else
				attackpose = AuPose::Psionic;
			if(IsRightHanded())
			{
				pose[PS_LeftArm] = AuPose::Block;
				pose[PS_RightArm] = attackpose;
			}
			else
			{
				pose[PS_LeftArm] = attackpose;
				pose[PS_RightArm] = AuPose::Block;
			}
		}
		break;
	case DhDNA::WL_Feet:
		pose[PS_FrontLegs] = AuPose::Kick;
		pose[PS_RearLegs] = AuPose::Kick;
		break;
	case DhDNA::WL_Tail:
	case DhDNA::WL_Anus:
		pose[PS_Tail] = AuPose::Strike;
		break;
	case DhDNA::WL_Torso:
		pose[PS_UpperTorso] = AuPose::Strike;
		break;
	}

	return Pose(pose, false, 1);
}

bool DpBody::PoseBurrow(float speedEnc)
{
	AuPose pose[PS_Count];
	InitPoses(AuPose::Dig, pose);

	if(m_DNA.HasArms())
		pose[PS_FrontLegs] = AuPose::StandWalk;	// Front legs don't dig if I
												// have arms.
	pose[PS_RearLegs] = AuPose::StandWalk;			// Rear legs are never used for
												// digging.

	return Pose(pose, false, speedEnc);
}

bool DpBody::PoseEatOffGround()
{
	AuPose pose[PS_Count];
	if(m_DNA.BodyCarriage == 0 || m_DNA.BodyCarriage == 1)
		InitPoses(AuPose::Stand, pose);	// Belly crawler can "stand" to eat.
	else
		InitPoses(AuPose::Crouch, pose);	// Everybody else crouches to eat.

	pose[PS_LeftHeadNeck] = AuPose::Eat;
	pose[PS_RightHeadNeck] = AuPose::Eat;
	pose[PS_UpperTorso] = AuPose::Eat;
	pose[PS_LeftArm] = AuPose::Eat;
	pose[PS_RightArm] = AuPose::Eat;
	pose[PS_Tail] = AuPose::Eat;

	return Pose(pose, false, 1);
}

// Position body based on poses of each section.
void DpBody::AnimatePose(bool bendDominantArm, float speedEnc)
{
	if(isEmpty())
		return;
	m_BendArm = bendDominantArm;

	// Determine waist vertex height established by legs and body carriage.
	float hipHt = CGHeight(false);

	// Init all bone matrices to identity, whether or not we're using them.
	InitBoneMatrices();

	// Position body parts.
	// Legs and tail are positioned before torso. If lower torso moves, it will need to move them as well.
	// NOTE: We don't currently move the lower torso.
	if(m_DNA.LegPairs)
		PositionLegs(hipHt, speedEnc);
	if(m_DNA.TailType != DhDNA::TAIL_None)
		PositionTail(hipHt, speedEnc);
	float waistrot = PositionTorso(hipHt, speedEnc);	// Bends torso and moves arms to connect locs.
	PositionHeads();								// Heads depend on upper torso position.

	/*
	//DHH More work is needed to stand at console instead of sit.
	if(IsConsolePose() && CGHeight(true) + HeadLocation().z > 7)
	{
		// Too tall to sit at a console. Stand instead. This will recurse.
		qDebug() << "Too tall to sit at a console. Stand instead.";
		AuPose::POSE savePose[PS_Count];	// Current position of each body section
		for(int ps=0; ps<DpBody::PS_Count; ps++)
			savePose[ps] = m_Pose[ps];
		Pose(AuPose::Stand, bendDominantArm, tNow);
		for(int ps=0; ps<DpBody::PS_Count; ps++)
			m_Pose[ps] = savePose[ps];
	}
	else
	*/
	{
		if(m_DNA.HasArms())
			PositionArms(hipHt, speedEnc);						// Arms depend on upper torso position.
		if(m_DNA.HasWings())
			PositionWings(waistrot);					// Wings depend on upper torso position.
	}
}

void DpBody::PositionArms(float hipHt, float speedEnc)
{
	// Left arms.
	switch(m_Pose[PS_LeftArm].Pose())
	{
	case AuPose::Init:
		break;
	case AuPose::Beckon:
		BeckonArms(0);
		break;
	case AuPose::Block:
		BlockArms(0);
		break;
	case AuPose::GameTableSeat0_0:
	case AuPose::GameTableSeat1_0:
	case AuPose::GameTableSeat2_0:
	case AuPose::GameTableSeat3_0:
	case AuPose::GameTableSeat0_45:
	case AuPose::GameTableSeat1_45:
	case AuPose::GameTableSeat2_45:
	case AuPose::GameTableSeat3_45:
	case AuPose::ConsoleCommand:
	case AuPose::ConsoleNav:
	case AuPose::ConsoleEngine:
	case AuPose::ConsoleFireControl:
	case AuPose::ConsoleSurgery:
	case AuPose::ConsolePilot:
	case AuPose::ConsolePower:
	case AuPose::ConsoleSensor:
	case AuPose::ConsoleShield:
	case AuPose::ConsoleSurgeryUnit:
	case AuPose::ConsoleTransporter:
	case AuPose::Crouch:
	case AuPose::Post_Sit:
	case AuPose::Post_Stand:
	case AuPose::SitOnGround:
	case AuPose::SitOnGroundCreep:
	case AuPose::Spawn:
	case AuPose::Stand:
	case AuPose::Command_Sit:
	case AuPose::Command_SitStationary:
	case AuPose::Command_Stand:
	case AuPose::Command_StandStationary:
	case AuPose::Engine_Sit:
	case AuPose::Engine_SitStationary:
	case AuPose::Engine_Stand:
	case AuPose::Engine_StandStationary:
	case AuPose::FireControl_Sit:
	case AuPose::FireControl_SitStationary:
	case AuPose::FireControl_Stand:
	case AuPose::FireControl_StandStationary:
	case AuPose::Helm_Sit:
	case AuPose::Helm_SitStationary:
	case AuPose::Helm_Stand:
	case AuPose::Helm_StandStationary:
	case AuPose::Hug:
	case AuPose::Medic_Sit:
	case AuPose::Medic_SitStationary:
	case AuPose::Medic_Stand:
	case AuPose::Medic_StandStationary:
	case AuPose::MouthPursed:
	case AuPose::MouthClosed:
	case AuPose::MouthOpen:
	case AuPose::MouthAgape:
	case AuPose::MouthOpenWide:
	case AuPose::MouthYawn:
	case AuPose::MouthBite:
	case AuPose::MouthLaugh:
	case AuPose::MouthYap:
	case AuPose::MouthWhisper:
	case AuPose::MouthTalk:
	case AuPose::MouthJabber:
	case AuPose::MouthYell:
	case AuPose::MouthSmile:
	case AuPose::MouthGrin:
	case AuPose::MouthSad:
	case AuPose::MouthFrown:
	case AuPose::MouthCry:
	case AuPose::MouthPerplexed:
	case AuPose::MouthGrim:
	case AuPose::MouthSardonic:
	case AuPose::Nav_Sit:
	case AuPose::Nav_SitStationary:
	case AuPose::Nav_Stand:
	case AuPose::Nav_StandStationary:
	case AuPose::Patient_Sit:
	case AuPose::Patient_SitStationary:
	case AuPose::Patient_Stand:
	case AuPose::Patient_StandStationary:
	case AuPose::Power_Sit:
	case AuPose::Power_SitStationary:
	case AuPose::Power_Stand:
	case AuPose::Power_StandStationary:
	case AuPose::Sensor_Sit:
	case AuPose::Sensor_SitStationary:
	case AuPose::Sensor_Stand:
	case AuPose::Sensor_StandStationary:
	case AuPose::Shield_Sit:
	case AuPose::Shield_SitStationary:
	case AuPose::Shield_Stand:
	case AuPose::Shield_StandStationary:
	case AuPose::Transporter_Sit:
	case AuPose::Transporter_SitStationary:
	case AuPose::Transporter_Stand:
	case AuPose::Transporter_StandStationary:
		StandArms(hipHt, 0, (m_BendArm && IsLeftHanded())?true:false);
		break;
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::ConsoleTurretBottom:
	case AuPose::ConsoleTurretTop:
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
		StandArms(hipHt, 0, true);	// Turrets need both hands.
		break;
	case AuPose::CrouchCreep:
	case AuPose::StandWalk:
		WalkArms(hipHt, 0, (m_BendArm && IsLeftHanded())?true:false, speedEnc);
		break;
	case AuPose::DeadFaceDown:
	case AuPose::BerthBottomFaceDown:
	case AuPose::BerthTopFaceDown:
		DeadArmsFacingDown(0);
		break;
	case AuPose::DeadFaceLeft:
	case AuPose::BerthBottomFaceLeft:
	case AuPose::BerthTopFaceLeft:
		DeadArmsFacingLeft(0);
		break;
	case AuPose::DeadFaceRight:
	case AuPose::BerthBottomFaceRight:
	case AuPose::BerthTopFaceRight:
		DeadArmsFacingRight(0);
		break;
	case AuPose::DeadFaceUp:
	case AuPose::BerthBottomFaceUp:
	case AuPose::BerthTopFaceUp:
		DeadArmsFacingUp(0);
		break;
	case AuPose::Dig:
		DigArms(0);
		break;
	case AuPose::Eat:
		EatWithArms(hipHt, 0);
		break;
	case AuPose::EyesOnly:
		break;
	case AuPose::Fly:
	case AuPose::FlyFast:
	case AuPose::Glide:
	case AuPose::Hover:
		FoldArms(0, (m_BendArm && IsLeftHanded())?true:false);
		break;
	case AuPose::Ladder:
		LadderArms(0, (m_BendArm && IsLeftHanded())?true:false);
		break;
	case AuPose::LadderClimb:
		ClimbArms(0, (m_BendArm && IsLeftHanded())?true:false, speedEnc);
		break;
	case AuPose::Bite:
	case AuPose::Kick:
	case AuPose::Lash:
	case AuPose::Stomp:
	case AuPose::Upright:
	case AuPose::Wag:
		Q_ASSERT(false);	// Invalid pose for arms.
		break;
	case AuPose::Jump:
		JumpArms(hipHt, 0, (m_BendArm && IsLeftHanded())?true:false, speedEnc);
		break;
	case AuPose::Point:
		PointArms(0);
		break;
	case AuPose::Prone:
		ProneArms(0, (m_BendArm && IsLeftHanded())?true:false);
		break;
	case AuPose::ProneCreep:
		ProneCreepArms(hipHt, 0, (m_BendArm && IsLeftHanded())?true:false, speedEnc);
		break;
	case AuPose::Psionic:
		PsionicArms(0);
		break;
	case AuPose::StandRun:
		RunArms(hipHt, 0, (m_BendArm && IsLeftHanded())?true:false, speedEnc);
		break;
	case AuPose::Straddle:
	case AuPose::MateOnTop:
		StraddleArms(hipHt, 0, (m_BendArm && IsLeftHanded())?true:false);
		break;
	case AuPose::Strike:
		StrikeArms(0);
		break;
	case AuPose::Swim:
	case AuPose::SwimFast:
		SwimArms(0, (m_BendArm && IsLeftHanded())?true:false);
		break;
	case AuPose::Throw:
		ThrowArms(0);
		break;
	case AuPose::TreadWater:
		TreadWaterArms(0, (m_BendArm && IsLeftHanded())?true:false);
		break;
	case AuPose::Wave:
		WaveArms(0);
		break;
	case AuPose::WritheFaceDown:
		WritheArmsFacingDown(0);
		break;
	case AuPose::WritheFaceLeft:
		WritheArmsFacingLeft(0);
		break;
	case AuPose::WritheFaceRight:
		WritheArmsFacingRight(0);
		break;
	case AuPose::WritheFaceUp:
	case AuPose::Patient_Prone:
		WritheArmsFacingUp(0);
		break;
	case AuPose::ReceptiveMate:
	case AuPose::MateOnBottom:
		ReceptiveArmsFacingUp(0);
		break;
	}

	// Right arms.
	switch(m_Pose[PS_RightArm].Pose())
	{
	case AuPose::Init:
		break;
	case AuPose::Beckon:
		BeckonArms(1);
		break;
	case AuPose::Block:
		BlockArms(1);
		break;
	case AuPose::GameTableSeat0_0:
	case AuPose::GameTableSeat1_0:
	case AuPose::GameTableSeat2_0:
	case AuPose::GameTableSeat3_0:
	case AuPose::GameTableSeat0_45:
	case AuPose::GameTableSeat1_45:
	case AuPose::GameTableSeat2_45:
	case AuPose::GameTableSeat3_45:
	case AuPose::ConsoleCommand:
	case AuPose::ConsoleNav:
	case AuPose::ConsoleEngine:
	case AuPose::ConsoleFireControl:
	case AuPose::ConsoleSurgery:
	case AuPose::ConsolePilot:
	case AuPose::ConsolePower:
	case AuPose::ConsoleSensor:
	case AuPose::ConsoleShield:
	case AuPose::ConsoleSurgeryUnit:
	case AuPose::ConsoleTransporter:
	case AuPose::Crouch:
	case AuPose::Post_Sit:
	case AuPose::Post_Stand:
	case AuPose::SitOnGround:
	case AuPose::SitOnGroundCreep:
	case AuPose::Spawn:
	case AuPose::Stand:
	case AuPose::Command_Sit:
	case AuPose::Command_SitStationary:
	case AuPose::Command_Stand:
	case AuPose::Command_StandStationary:
	case AuPose::Engine_Sit:
	case AuPose::Engine_SitStationary:
	case AuPose::Engine_Stand:
	case AuPose::Engine_StandStationary:
	case AuPose::FireControl_Sit:
	case AuPose::FireControl_SitStationary:
	case AuPose::FireControl_Stand:
	case AuPose::FireControl_StandStationary:
	case AuPose::Helm_Sit:
	case AuPose::Helm_SitStationary:
	case AuPose::Helm_Stand:
	case AuPose::Helm_StandStationary:
	case AuPose::Hug:
	case AuPose::Medic_Sit:
	case AuPose::Medic_SitStationary:
	case AuPose::Medic_Stand:
	case AuPose::Medic_StandStationary:
	case AuPose::MouthPursed:
	case AuPose::MouthClosed:
	case AuPose::MouthOpen:
	case AuPose::MouthAgape:
	case AuPose::MouthOpenWide:
	case AuPose::MouthYawn:
	case AuPose::MouthBite:
	case AuPose::MouthLaugh:
	case AuPose::MouthYap:
	case AuPose::MouthWhisper:
	case AuPose::MouthTalk:
	case AuPose::MouthJabber:
	case AuPose::MouthYell:
	case AuPose::MouthSmile:
	case AuPose::MouthGrin:
	case AuPose::MouthSad:
	case AuPose::MouthFrown:
	case AuPose::MouthCry:
	case AuPose::MouthPerplexed:
	case AuPose::MouthGrim:
	case AuPose::MouthSardonic:
	case AuPose::Nav_Sit:
	case AuPose::Nav_SitStationary:
	case AuPose::Nav_Stand:
	case AuPose::Nav_StandStationary:
	case AuPose::Patient_Sit:
	case AuPose::Patient_SitStationary:
	case AuPose::Patient_Stand:
	case AuPose::Patient_StandStationary:
	case AuPose::Power_Sit:
	case AuPose::Power_SitStationary:
	case AuPose::Power_Stand:
	case AuPose::Power_StandStationary:
	case AuPose::Sensor_Sit:
	case AuPose::Sensor_SitStationary:
	case AuPose::Sensor_Stand:
	case AuPose::Sensor_StandStationary:
	case AuPose::Shield_Sit:
	case AuPose::Shield_SitStationary:
	case AuPose::Shield_Stand:
	case AuPose::Shield_StandStationary:
	case AuPose::Transporter_Sit:
	case AuPose::Transporter_SitStationary:
	case AuPose::Transporter_Stand:
	case AuPose::Transporter_StandStationary:
		StandArms(hipHt, 1, (m_BendArm && IsRightHanded())?true:false);
		break;
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::ConsoleTurretBottom:
	case AuPose::ConsoleTurretTop:
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
		StandArms(hipHt, 1, true);	// Turrets need both hands.
		break;
	case AuPose::CrouchCreep:
	case AuPose::StandWalk:
		WalkArms(hipHt, 1, (m_BendArm && IsRightHanded())?true:false, speedEnc);
		break;
	case AuPose::DeadFaceDown:
	case AuPose::BerthBottomFaceDown:
	case AuPose::BerthTopFaceDown:
		DeadArmsFacingDown(1);
		break;
	case AuPose::DeadFaceLeft:
	case AuPose::BerthBottomFaceLeft:
	case AuPose::BerthTopFaceLeft:
		DeadArmsFacingLeft(1);
		break;
	case AuPose::DeadFaceRight:
	case AuPose::BerthBottomFaceRight:
	case AuPose::BerthTopFaceRight:
		DeadArmsFacingRight(1);
		break;
	case AuPose::DeadFaceUp:
	case AuPose::BerthBottomFaceUp:
	case AuPose::BerthTopFaceUp:
		DeadArmsFacingUp(1);
		break;
	case AuPose::Dig:
		DigArms(1);
		break;
	case AuPose::Eat:
		EatWithArms(hipHt, 1);
		break;
	case AuPose::EyesOnly:
		break;
	case AuPose::Fly:
	case AuPose::FlyFast:
	case AuPose::Glide:
	case AuPose::Hover:
		FoldArms(1, (m_BendArm && IsRightHanded())?true:false);
		break;
	case AuPose::Ladder:
		LadderArms(1, (m_BendArm && IsRightHanded())?true:false);
		break;
	case AuPose::LadderClimb:
		ClimbArms(1, (m_BendArm && IsRightHanded())?true:false, speedEnc);
		break;
	case AuPose::Bite:
	case AuPose::Kick:
	case AuPose::Lash:
	case AuPose::Stomp:
	case AuPose::Upright:
	case AuPose::Wag:
		Q_ASSERT(false);	// Invalid pose for arms.
		break;
	case AuPose::Jump:
		JumpArms(hipHt, 1, (m_BendArm && IsRightHanded())?true:false, speedEnc);
		break;
	case AuPose::Point:
		PointArms(1);
		break;
	case AuPose::Prone:
		ProneArms(1, (m_BendArm && IsRightHanded())?true:false);
		break;
	case AuPose::ProneCreep:
		ProneCreepArms(hipHt, 1, (m_BendArm && IsRightHanded())?true:false, speedEnc);
		break;
	case AuPose::Psionic:
		PsionicArms(1);
		break;
	case AuPose::StandRun:
		RunArms(hipHt, 1, (m_BendArm && IsRightHanded())?true:false, speedEnc);
		break;
	case AuPose::Straddle:
	case AuPose::MateOnTop:
		StraddleArms(hipHt, 1, (m_BendArm && IsRightHanded())?true:false);
		break;
	case AuPose::Strike:
		StrikeArms(1);
		break;
	case AuPose::Swim:
	case AuPose::SwimFast:
		SwimArms(1, (m_BendArm && IsRightHanded())?true:false);
		break;
	case AuPose::Throw:
		ThrowArms(1);
		break;
	case AuPose::TreadWater:
		TreadWaterArms(1, (m_BendArm && IsRightHanded())?true:false);
		break;
	case AuPose::Wave:
		WaveArms(1);
		break;
	case AuPose::WritheFaceDown:
		WritheArmsFacingDown(1);
		break;
	case AuPose::WritheFaceLeft:
		WritheArmsFacingLeft(1);
		break;
	case AuPose::WritheFaceRight:
		WritheArmsFacingRight(1);
		break;
	case AuPose::WritheFaceUp:
	case AuPose::Patient_Prone:
		WritheArmsFacingUp(1);
		break;
	case AuPose::ReceptiveMate:
	case AuPose::MateOnBottom:
		ReceptiveArmsFacingUp(1);
		break;
	}
}

void DpBody::PositionHeads()
{
	float leftHeadTilt = m_HeadTilt;
	switch(m_Pose[PS_LeftHeadNeck].Pose())
	{
	case AuPose::Init:
	case AuPose::Beckon:
	case AuPose::Block:
	case AuPose::GameTableSeat0_0:
	case AuPose::GameTableSeat1_0:
	case AuPose::GameTableSeat2_0:
	case AuPose::GameTableSeat3_0:
	case AuPose::GameTableSeat0_45:
	case AuPose::GameTableSeat1_45:
	case AuPose::GameTableSeat2_45:
	case AuPose::GameTableSeat3_45:
	case AuPose::ConsoleCommand:
	case AuPose::ConsoleNav:
	case AuPose::ConsoleEngine:
	case AuPose::ConsoleFireControl:
	case AuPose::ConsoleSurgery:
	case AuPose::ConsolePilot:
	case AuPose::ConsolePower:
	case AuPose::ConsoleSensor:
	case AuPose::ConsoleShield:
	case AuPose::ConsoleSurgeryUnit:
	case AuPose::ConsoleTransporter:
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::ConsoleTurretBottom:
	case AuPose::ConsoleTurretTop:
	case AuPose::Crouch:
	case AuPose::CrouchCreep:
	case AuPose::Dig:
	case AuPose::EyesOnly:
	case AuPose::Fly:
	case AuPose::FlyFast:
	case AuPose::Glide:
	case AuPose::Hover:
	case AuPose::Hug:
	case AuPose::Jump:
	case AuPose::Kick:
	case AuPose::Ladder:
	case AuPose::LadderClimb:
	case AuPose::Lash:
	case AuPose::MouthPursed:
	case AuPose::MouthClosed:
	case AuPose::MouthOpen:
	case AuPose::MouthAgape:
	case AuPose::MouthOpenWide:
	case AuPose::MouthYawn:
	case AuPose::MouthBite:
	case AuPose::MouthLaugh:
	case AuPose::MouthYap:
	case AuPose::MouthWhisper:
	case AuPose::MouthTalk:
	case AuPose::MouthJabber:
	case AuPose::MouthYell:
	case AuPose::MouthSmile:
	case AuPose::MouthGrin:
	case AuPose::MouthSad:
	case AuPose::MouthFrown:
	case AuPose::MouthCry:
	case AuPose::MouthPerplexed:
	case AuPose::MouthGrim:
	case AuPose::MouthSardonic:
	case AuPose::Point:
	case AuPose::Post_Sit:
	case AuPose::Post_Stand:
	case AuPose::Prone:
	case AuPose::ProneCreep:
	case AuPose::Psionic:
	case AuPose::SitOnGround:
	case AuPose::SitOnGroundCreep:
	case AuPose::Spawn:
	case AuPose::Stand:
	case AuPose::StandRun:
	case AuPose::StandWalk:
	case AuPose::Stomp:
	case AuPose::Straddle:
	case AuPose::MateOnTop:
	case AuPose::Strike:
	case AuPose::Swim:
	case AuPose::SwimFast:
	case AuPose::Throw:
	case AuPose::TreadWater:
	case AuPose::Upright:
	case AuPose::Wag:
	case AuPose::Wave:
	case AuPose::Command_Sit:
	case AuPose::Command_SitStationary:
	case AuPose::Command_Stand:
	case AuPose::Command_StandStationary:
	case AuPose::Engine_Sit:
	case AuPose::Engine_SitStationary:
	case AuPose::Engine_Stand:
	case AuPose::Engine_StandStationary:
	case AuPose::FireControl_Sit:
	case AuPose::FireControl_SitStationary:
	case AuPose::FireControl_Stand:
	case AuPose::FireControl_StandStationary:
	case AuPose::Helm_Sit:
	case AuPose::Helm_SitStationary:
	case AuPose::Helm_Stand:
	case AuPose::Helm_StandStationary:
	case AuPose::Medic_Sit:
	case AuPose::Medic_SitStationary:
	case AuPose::Medic_Stand:
	case AuPose::Medic_StandStationary:
	case AuPose::Nav_Sit:
	case AuPose::Nav_SitStationary:
	case AuPose::Nav_Stand:
	case AuPose::Nav_StandStationary:
	case AuPose::Patient_Sit:
	case AuPose::Patient_SitStationary:
	case AuPose::Patient_Stand:
	case AuPose::Patient_StandStationary:
	case AuPose::Power_Sit:
	case AuPose::Power_SitStationary:
	case AuPose::Power_Stand:
	case AuPose::Power_StandStationary:
	case AuPose::Sensor_Sit:
	case AuPose::Sensor_SitStationary:
	case AuPose::Sensor_Stand:
	case AuPose::Sensor_StandStationary:
	case AuPose::Shield_Sit:
	case AuPose::Shield_SitStationary:
	case AuPose::Shield_Stand:
	case AuPose::Shield_StandStationary:
	case AuPose::Transporter_Sit:
	case AuPose::Transporter_SitStationary:
	case AuPose::Transporter_Stand:
	case AuPose::Transporter_StandStationary:
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
		break;
	case AuPose::DeadFaceDown:
	case AuPose::DeadFaceLeft:
	case AuPose::DeadFaceRight:
	case AuPose::DeadFaceUp:
	case AuPose::BerthBottomFaceDown:
	case AuPose::BerthBottomFaceLeft:
	case AuPose::BerthBottomFaceRight:
	case AuPose::BerthBottomFaceUp:
	case AuPose::BerthTopFaceDown:
	case AuPose::BerthTopFaceLeft:
	case AuPose::BerthTopFaceRight:
	case AuPose::BerthTopFaceUp:
		leftHeadTilt = -PIf/2.f;
		break;
	case AuPose::WritheFaceDown:
	case AuPose::WritheFaceLeft:
	case AuPose::WritheFaceRight:
	case AuPose::WritheFaceUp:
	case AuPose::Patient_Prone:
		leftHeadTilt = 1.5f*DEG2RADf * m_DNA.Rhythm(m_Pose[PS_LeftHeadNeck], 0, 1) - PIf/2.f;
		break;
	case AuPose::Bite:
		leftHeadTilt += (10.f * m_DNA.Rhythm(AuPose::Bite, 0, 1) - 10.f)*DEG2RADf;
		break;
	case AuPose::Eat:
		leftHeadTilt += (10.f * m_DNA.Rhythm(AuPose::Eat, 0, 1) - 10.f)*DEG2RADf;
		break;
	case AuPose::ReceptiveMate:
	case AuPose::MateOnBottom:
		leftHeadTilt += -PIf/2.f;
		break;
	}
	float rightHeadTilt = m_HeadTilt;
	switch(m_Pose[PS_RightHeadNeck].Pose())
	{
	case AuPose::Init:
	case AuPose::Beckon:
	case AuPose::Block:
	case AuPose::GameTableSeat0_0:
	case AuPose::GameTableSeat1_0:
	case AuPose::GameTableSeat2_0:
	case AuPose::GameTableSeat3_0:
	case AuPose::GameTableSeat0_45:
	case AuPose::GameTableSeat1_45:
	case AuPose::GameTableSeat2_45:
	case AuPose::GameTableSeat3_45:
	case AuPose::ConsoleCommand:
	case AuPose::ConsoleNav:
	case AuPose::ConsoleEngine:
	case AuPose::ConsoleFireControl:
	case AuPose::ConsoleSurgery:
	case AuPose::ConsolePilot:
	case AuPose::ConsolePower:
	case AuPose::ConsoleSensor:
	case AuPose::ConsoleShield:
	case AuPose::ConsoleSurgeryUnit:
	case AuPose::ConsoleTransporter:
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::ConsoleTurretBottom:
	case AuPose::ConsoleTurretTop:
	case AuPose::Crouch:
	case AuPose::CrouchCreep:
	case AuPose::Dig:
	case AuPose::EyesOnly:
	case AuPose::Fly:
	case AuPose::FlyFast:
	case AuPose::Glide:
	case AuPose::Hover:
	case AuPose::Hug:
	case AuPose::Jump:
	case AuPose::Kick:
	case AuPose::Ladder:
	case AuPose::LadderClimb:
	case AuPose::Lash:
	case AuPose::MouthPursed:
	case AuPose::MouthClosed:
	case AuPose::MouthOpen:
	case AuPose::MouthAgape:
	case AuPose::MouthOpenWide:
	case AuPose::MouthYawn:
	case AuPose::MouthBite:
	case AuPose::MouthLaugh:
	case AuPose::MouthYap:
	case AuPose::MouthWhisper:
	case AuPose::MouthTalk:
	case AuPose::MouthJabber:
	case AuPose::MouthYell:
	case AuPose::MouthSmile:
	case AuPose::MouthGrin:
	case AuPose::MouthSad:
	case AuPose::MouthFrown:
	case AuPose::MouthCry:
	case AuPose::MouthPerplexed:
	case AuPose::MouthGrim:
	case AuPose::MouthSardonic:
	case AuPose::Point:
	case AuPose::Post_Sit:
	case AuPose::Post_Stand:
	case AuPose::Prone:
	case AuPose::ProneCreep:
	case AuPose::Psionic:
	case AuPose::SitOnGround:
	case AuPose::SitOnGroundCreep:
	case AuPose::Spawn:
	case AuPose::Stand:
	case AuPose::StandRun:
	case AuPose::StandWalk:
	case AuPose::Stomp:
	case AuPose::Straddle:
	case AuPose::MateOnTop:
	case AuPose::Strike:
	case AuPose::Swim:
	case AuPose::SwimFast:
	case AuPose::Throw:
	case AuPose::TreadWater:
	case AuPose::Upright:
	case AuPose::Wag:
	case AuPose::Wave:
	case AuPose::Command_Sit:
	case AuPose::Command_SitStationary:
	case AuPose::Command_Stand:
	case AuPose::Command_StandStationary:
	case AuPose::Engine_Sit:
	case AuPose::Engine_SitStationary:
	case AuPose::Engine_Stand:
	case AuPose::Engine_StandStationary:
	case AuPose::FireControl_Sit:
	case AuPose::FireControl_SitStationary:
	case AuPose::FireControl_Stand:
	case AuPose::FireControl_StandStationary:
	case AuPose::Helm_Sit:
	case AuPose::Helm_SitStationary:
	case AuPose::Helm_Stand:
	case AuPose::Helm_StandStationary:
	case AuPose::Medic_Sit:
	case AuPose::Medic_SitStationary:
	case AuPose::Medic_Stand:
	case AuPose::Medic_StandStationary:
	case AuPose::Nav_Sit:
	case AuPose::Nav_SitStationary:
	case AuPose::Nav_Stand:
	case AuPose::Nav_StandStationary:
	case AuPose::Patient_Sit:
	case AuPose::Patient_SitStationary:
	case AuPose::Patient_Stand:
	case AuPose::Patient_StandStationary:
	case AuPose::Power_Sit:
	case AuPose::Power_SitStationary:
	case AuPose::Power_Stand:
	case AuPose::Power_StandStationary:
	case AuPose::Sensor_Sit:
	case AuPose::Sensor_SitStationary:
	case AuPose::Sensor_Stand:
	case AuPose::Sensor_StandStationary:
	case AuPose::Shield_Sit:
	case AuPose::Shield_SitStationary:
	case AuPose::Shield_Stand:
	case AuPose::Shield_StandStationary:
	case AuPose::Transporter_Sit:
	case AuPose::Transporter_SitStationary:
	case AuPose::Transporter_Stand:
	case AuPose::Transporter_StandStationary:
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
		break;
	case AuPose::DeadFaceDown:
	case AuPose::DeadFaceLeft:
	case AuPose::DeadFaceRight:
	case AuPose::DeadFaceUp:
	case AuPose::BerthBottomFaceDown:
	case AuPose::BerthBottomFaceLeft:
	case AuPose::BerthBottomFaceRight:
	case AuPose::BerthBottomFaceUp:
	case AuPose::BerthTopFaceDown:
	case AuPose::BerthTopFaceLeft:
	case AuPose::BerthTopFaceRight:
	case AuPose::BerthTopFaceUp:
		rightHeadTilt = -PIf/2.f;
		break;
	case AuPose::WritheFaceDown:
	case AuPose::WritheFaceLeft:
	case AuPose::WritheFaceRight:
	case AuPose::WritheFaceUp:
	case AuPose::Patient_Prone:
		// Right head writhes slightly out of phase with left head.
		rightHeadTilt = 1.5f*DEG2RADf * m_DNA.Rhythm(m_Pose[PS_RightHeadNeck], 15.f*DEG2RADf, 1);
		break;
	case AuPose::Bite:
		// Heads take turns biting.
		rightHeadTilt += (10.f * m_DNA.Rhythm(AuPose::Bite, PIf, 1) - 10.f)*DEG2RADf - PIf/2.f;
		break;
	case AuPose::Eat:
		// Heads take turns eating.
		rightHeadTilt += (10.f * m_DNA.Rhythm(AuPose::Eat, PIf, 1) - 10.f)*DEG2RADf - PIf/2.f;
		break;
	case AuPose::ReceptiveMate:
	case AuPose::MateOnBottom:
		rightHeadTilt += -PIf/2.f;
		break;
	}

	// Multiple necks will end up on top of each other so we'll fan them out here.
	if(m_DNA.Heads)
	{
		// This critter has more than one head.
		bool isLeft = true;	// first head is left
		for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		{
			if(!(*it)->IsHeadAndNeck())
				continue;
			DpHeadAndNeck* head = (DpHeadAndNeck*)(*it);
			if(isLeft)
			{
				head->TiltHeadTo(leftHeadTilt, m_HeadTurn);
				isLeft = false;	// next head is right
			}
			else
			{
				head->TiltHeadTo(rightHeadTilt, m_HeadTurn);
				break;
			}
		}

		// We don't allow zero heads.
		unsigned int heads = (unsigned int)m_DNA.Heads + 1;
		float necksplay = 40.f * DEG2RADf;		// Angle between necks.
		float neckang = -(necksplay * (float)(heads - 1))/2.f;	// First neck leans to the left.
		isLeft = true;	// first head is left
		for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
			if((*it)->IsHeadAndNeck())
			{
				DpHeadAndNeck* head = (DpHeadAndNeck*)(*it);
				head->LeanNeck(neckang);
				neckang += necksplay;
				if(isLeft)
					isLeft = false;
				else
					break;
			}
	}
	else
	{
		// This critter has one head.
		for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		{
			if(!(*it)->IsHeadAndNeck())
				continue;
			DpHeadAndNeck* head = (DpHeadAndNeck*)(*it);
			head->TiltHeadTo(leftHeadTilt, m_HeadTurn);
			break;
		}
	}
}

void DpBody::PositionLegs(float hipHt, float speedEnc)
{
	// Rear legs.
	switch(m_Pose[PS_RearLegs].Pose())
	{
	case AuPose::Init:
		break;
	case AuPose::Beckon:
	case AuPose::Bite:
	case AuPose::Block:
	case AuPose::Eat:
	case AuPose::Lash:
	case AuPose::Point:
	case AuPose::Psionic:
	case AuPose::Strike:
	case AuPose::Throw:
	case AuPose::Upright:
	case AuPose::Wag:
	case AuPose::Wave:
		qWarning("DpBody::PositionLegs Invalid pose %d for rear legs", m_Pose[PS_RearLegs].Pose());
		Q_ASSERT(false);	// Invalid pose for rear legs.
		break;
	case AuPose::GameTableSeat0_0:
	case AuPose::GameTableSeat1_0:
	case AuPose::GameTableSeat2_0:
	case AuPose::GameTableSeat3_0:
	case AuPose::GameTableSeat0_45:
	case AuPose::GameTableSeat1_45:
	case AuPose::GameTableSeat2_45:
	case AuPose::GameTableSeat3_45:
	case AuPose::ConsoleCommand:
	case AuPose::ConsoleNav:
	case AuPose::ConsoleEngine:
	case AuPose::ConsoleFireControl:
	case AuPose::ConsoleSurgery:
	case AuPose::ConsolePilot:
	case AuPose::ConsolePower:
	case AuPose::ConsoleSensor:
	case AuPose::ConsoleShield:
	case AuPose::ConsoleTransporter:
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::ConsoleTurretBottom:
	case AuPose::ConsoleTurretTop:
	case AuPose::Command_Sit:
	case AuPose::Command_SitStationary:
	case AuPose::Engine_Sit:
	case AuPose::Engine_SitStationary:
	case AuPose::FireControl_Sit:
	case AuPose::FireControl_SitStationary:
	case AuPose::Helm_Sit:
	case AuPose::Helm_SitStationary:
	case AuPose::Hug:
	case AuPose::MouthPursed:
	case AuPose::MouthClosed:
	case AuPose::MouthOpen:
	case AuPose::MouthAgape:
	case AuPose::MouthOpenWide:
	case AuPose::MouthYawn:
	case AuPose::MouthBite:
	case AuPose::MouthLaugh:
	case AuPose::MouthYap:
	case AuPose::MouthWhisper:
	case AuPose::MouthTalk:
	case AuPose::MouthJabber:
	case AuPose::MouthYell:
	case AuPose::MouthSmile:
	case AuPose::MouthGrin:
	case AuPose::MouthSad:
	case AuPose::MouthFrown:
	case AuPose::MouthCry:
	case AuPose::MouthPerplexed:
	case AuPose::MouthGrim:
	case AuPose::MouthSardonic:
	case AuPose::Medic_Sit:
	case AuPose::Medic_SitStationary:
	case AuPose::Nav_Sit:
	case AuPose::Nav_SitStationary:
	case AuPose::Patient_Sit:
	case AuPose::Patient_SitStationary:
	case AuPose::Post_Sit:
	case AuPose::Power_Sit:
	case AuPose::Power_SitStationary:
	case AuPose::Sensor_Sit:
	case AuPose::Sensor_SitStationary:
	case AuPose::Shield_Sit:
	case AuPose::Shield_SitStationary:
	case AuPose::Transporter_Sit:
	case AuPose::Transporter_SitStationary:
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
		StandLegs(hipHt, 2);
		return;	// All legs are affected by console pose so we're done.
	case AuPose::ConsoleSurgeryUnit:
	case AuPose::Crouch:
	case AuPose::Ladder:
	case AuPose::Post_Stand:
	case AuPose::SitOnGround:
	case AuPose::SitOnGroundCreep:
	case AuPose::Spawn:
	case AuPose::Stand:
	case AuPose::Command_Stand:
	case AuPose::Command_StandStationary:
	case AuPose::Engine_Stand:
	case AuPose::Engine_StandStationary:
	case AuPose::FireControl_Stand:
	case AuPose::FireControl_StandStationary:
	case AuPose::Helm_Stand:
	case AuPose::Helm_StandStationary:
	case AuPose::Medic_Stand:
	case AuPose::Medic_StandStationary:
	case AuPose::Nav_Stand:
	case AuPose::Nav_StandStationary:
	case AuPose::Patient_Stand:
	case AuPose::Patient_StandStationary:
	case AuPose::Power_Stand:
	case AuPose::Power_StandStationary:
	case AuPose::Sensor_Stand:
	case AuPose::Sensor_StandStationary:
	case AuPose::Shield_Stand:
	case AuPose::Shield_StandStationary:
	case AuPose::Transporter_Stand:
	case AuPose::Transporter_StandStationary:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
		StandLegs(hipHt, 0);
		break;
	case AuPose::CrouchCreep:
	case AuPose::Dig:
	case AuPose::LadderClimb:
	case AuPose::StandWalk:
		WalkLegs(hipHt, 0, speedEnc);
		break;
	case AuPose::DeadFaceDown:
	case AuPose::BerthBottomFaceDown:
	case AuPose::BerthTopFaceDown:
		DeadLegsFacingDown(0);
		break;
	case AuPose::DeadFaceLeft:
	case AuPose::BerthBottomFaceLeft:
	case AuPose::BerthTopFaceLeft:
		DeadLegsFacingLeft(0);
		break;
	case AuPose::DeadFaceRight:
	case AuPose::BerthBottomFaceRight:
	case AuPose::BerthTopFaceRight:
		DeadLegsFacingRight(0);
		break;
	case AuPose::DeadFaceUp:
	case AuPose::BerthBottomFaceUp:
	case AuPose::BerthTopFaceUp:
		DeadLegsFacingUp(0);
		break;
	case AuPose::EyesOnly:
		break;
	case AuPose::Fly:
	case AuPose::FlyFast:
	case AuPose::Glide:
	case AuPose::Hover:
		FoldLegs(0);
		break;
	case AuPose::Jump:
		JumpLegs(hipHt, 0, speedEnc);
		break;
	case AuPose::Kick:
		KickLegs(hipHt, 0);
		break;
	case AuPose::Prone:
		ProneLegs(0);
		break;
	case AuPose::ProneCreep:
		ProneCreepLegs(0, speedEnc);
		break;
	case AuPose::StandRun:
		RunLegs(hipHt, 0, speedEnc);
		break;
	case AuPose::Stomp:
		StompLegs(hipHt, 0);
		break;
	case AuPose::Straddle:
	case AuPose::MateOnTop:
		StraddleLegs();
		return;	// All legs are affected by straddle pose so we're done.
	case AuPose::Swim:
	case AuPose::SwimFast:
		SwimLegs(0);
		break;
	case AuPose::TreadWater:
		TreadWaterLegs(hipHt, 0);
		break;
	case AuPose::WritheFaceDown:
		WritheLegsFacingDown(0);
		break;
	case AuPose::WritheFaceLeft:
		WritheLegsFacingLeft(0);
		break;
	case AuPose::WritheFaceRight:
		WritheLegsFacingRight(0);
		break;
	case AuPose::WritheFaceUp:
	case AuPose::Patient_Prone:
		WritheLegsFacingUp(0);
		break;
	case AuPose::ReceptiveMate:
	case AuPose::MateOnBottom:
		ReceptiveLegsFacingUp(0);
		break;
	}

	// Front legs.
	switch(m_Pose[PS_FrontLegs].Pose())
	{
	case AuPose::Init:
		break;
	case AuPose::Beckon:
	case AuPose::Bite:
	case AuPose::Block:
	case AuPose::Eat:
	case AuPose::Lash:
	case AuPose::Point:
	case AuPose::Psionic:
	case AuPose::Strike:
	case AuPose::Throw:
	case AuPose::Upright:
	case AuPose::Wag:
	case AuPose::Wave:
		Q_ASSERT(false);	// Invalid pose for front legs.
		break;
	case AuPose::GameTableSeat0_0:
	case AuPose::GameTableSeat1_0:
	case AuPose::GameTableSeat2_0:
	case AuPose::GameTableSeat3_0:
	case AuPose::GameTableSeat0_45:
	case AuPose::GameTableSeat1_45:
	case AuPose::GameTableSeat2_45:
	case AuPose::GameTableSeat3_45:
	case AuPose::ConsoleCommand:
	case AuPose::ConsoleNav:
	case AuPose::ConsoleEngine:
	case AuPose::ConsoleFireControl:
	case AuPose::ConsoleSurgery:
	case AuPose::ConsolePilot:
	case AuPose::ConsolePower:
	case AuPose::ConsoleSensor:
	case AuPose::ConsoleShield:
	case AuPose::ConsoleTransporter:
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::ConsoleTurretBottom:
	case AuPose::ConsoleTurretTop:
	case AuPose::Straddle:
	case AuPose::MateOnTop:
	case AuPose::Command_Sit:
	case AuPose::Command_SitStationary:
	case AuPose::Engine_Sit:
	case AuPose::Engine_SitStationary:
	case AuPose::FireControl_Sit:
	case AuPose::FireControl_SitStationary:
	case AuPose::Helm_Sit:
	case AuPose::Helm_SitStationary:
	case AuPose::Hug:
	case AuPose::MouthPursed:
	case AuPose::MouthClosed:
	case AuPose::MouthOpen:
	case AuPose::MouthAgape:
	case AuPose::MouthOpenWide:
	case AuPose::MouthYawn:
	case AuPose::MouthBite:
	case AuPose::MouthLaugh:
	case AuPose::MouthYap:
	case AuPose::MouthWhisper:
	case AuPose::MouthTalk:
	case AuPose::MouthJabber:
	case AuPose::MouthYell:
	case AuPose::MouthSmile:
	case AuPose::MouthGrin:
	case AuPose::MouthSad:
	case AuPose::MouthFrown:
	case AuPose::MouthCry:
	case AuPose::MouthPerplexed:
	case AuPose::MouthGrim:
	case AuPose::MouthSardonic:
	case AuPose::Medic_Sit:
	case AuPose::Medic_SitStationary:
	case AuPose::Nav_Sit:
	case AuPose::Nav_SitStationary:
	case AuPose::Patient_Sit:
	case AuPose::Patient_SitStationary:
	case AuPose::Post_Sit:
	case AuPose::Power_Sit:
	case AuPose::Power_SitStationary:
	case AuPose::Sensor_Sit:
	case AuPose::Sensor_SitStationary:
	case AuPose::Shield_Sit:
	case AuPose::Shield_SitStationary:
	case AuPose::Transporter_Sit:
	case AuPose::Transporter_SitStationary:
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
		Q_ASSERT(false);	// Error when front legs in this pose but not back legs.
		break;
	case AuPose::ConsoleSurgeryUnit:
	case AuPose::Crouch:
	case AuPose::Ladder:
	case AuPose::Post_Stand:
	case AuPose::SitOnGround:
	case AuPose::SitOnGroundCreep:
	case AuPose::Spawn:
	case AuPose::Stand:
	case AuPose::Command_Stand:
	case AuPose::Command_StandStationary:
	case AuPose::Engine_Stand:
	case AuPose::Engine_StandStationary:
	case AuPose::FireControl_Stand:
	case AuPose::FireControl_StandStationary:
	case AuPose::Helm_Stand:
	case AuPose::Helm_StandStationary:
	case AuPose::Medic_Stand:
	case AuPose::Medic_StandStationary:
	case AuPose::Nav_Stand:
	case AuPose::Nav_StandStationary:
	case AuPose::Patient_Stand:
	case AuPose::Patient_StandStationary:
	case AuPose::Power_Stand:
	case AuPose::Power_StandStationary:
	case AuPose::Sensor_Stand:
	case AuPose::Sensor_StandStationary:
	case AuPose::Shield_Stand:
	case AuPose::Shield_StandStationary:
	case AuPose::Transporter_Stand:
	case AuPose::Transporter_StandStationary:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
		StandLegs(hipHt, 1);
		break;
	case AuPose::CrouchCreep:
	case AuPose::StandWalk:
		WalkLegs(hipHt, 1, speedEnc);
		break;
	case AuPose::LadderClimb:
		ClimbLegs(hipHt, speedEnc);
		break;
	case AuPose::DeadFaceDown:
	case AuPose::BerthBottomFaceDown:
	case AuPose::BerthTopFaceDown:
		DeadLegsFacingDown(1);
		break;
	case AuPose::DeadFaceLeft:
	case AuPose::BerthBottomFaceLeft:
	case AuPose::BerthTopFaceLeft:
		DeadLegsFacingLeft(1);
		break;
	case AuPose::DeadFaceRight:
	case AuPose::BerthBottomFaceRight:
	case AuPose::BerthTopFaceRight:
		DeadLegsFacingRight(1);
		break;
	case AuPose::DeadFaceUp:
	case AuPose::BerthBottomFaceUp:
	case AuPose::BerthTopFaceUp:
		DeadLegsFacingUp(1);
		break;
	case AuPose::Dig:
		DigLegs(hipHt, speedEnc);
		break;
	case AuPose::EyesOnly:
		break;
	case AuPose::Fly:
	case AuPose::FlyFast:
	case AuPose::Glide:
	case AuPose::Hover:
		FoldLegs(1);
		break;
	case AuPose::Jump:
		JumpLegs(hipHt, 1, speedEnc);
		break;
	case AuPose::Kick:
		KickLegs(hipHt, 1);
		break;
	case AuPose::Prone:
		ProneLegs(1);
		break;
	case AuPose::ProneCreep:
		ProneCreepLegs(1, speedEnc);
		break;
	case AuPose::StandRun:
		RunLegs(hipHt, 1, speedEnc);
		break;
	case AuPose::Stomp:
		StompLegs(hipHt, 1);
		break;
	case AuPose::Swim:
	case AuPose::SwimFast:
		SwimLegs(1);
		break;
	case AuPose::TreadWater:
		TreadWaterLegs(hipHt, 1);
		break;
	case AuPose::WritheFaceDown:
		WritheLegsFacingDown(1);
		break;
	case AuPose::WritheFaceLeft:
		WritheLegsFacingLeft(1);
		break;
	case AuPose::WritheFaceRight:
		WritheLegsFacingRight(1);
		break;
	case AuPose::WritheFaceUp:
	case AuPose::Patient_Prone:
		WritheLegsFacingUp(1);
		break;
	case AuPose::ReceptiveMate:
	case AuPose::MateOnBottom:
		ReceptiveLegsFacingUp(1);
		break;
	}
}

void DpBody::PositionTail(float hipHt, float speedEnc)
{
	switch(m_Pose[PS_Tail].Pose())
	{
	case AuPose::Init:
	case AuPose::EyesOnly:
		// Left unchanged, the tail sticks out straight behind.
		break;
	case AuPose::Beckon:
	case AuPose::Bite:
	case AuPose::Block:
	case AuPose::Point:
	case AuPose::Psionic:
	case AuPose::Stomp:
	case AuPose::Throw:
	case AuPose::Upright:
	case AuPose::Wave:
		Q_ASSERT(false);	// Invalid pose for tail.
		break;
	case AuPose::GameTableSeat0_0:
	case AuPose::GameTableSeat1_0:
	case AuPose::GameTableSeat2_0:
	case AuPose::GameTableSeat3_0:
	case AuPose::GameTableSeat0_45:
	case AuPose::GameTableSeat1_45:
	case AuPose::GameTableSeat2_45:
	case AuPose::GameTableSeat3_45:
	case AuPose::ConsoleCommand:
	case AuPose::ConsoleNav:
	case AuPose::ConsoleEngine:
	case AuPose::ConsoleFireControl:
	case AuPose::ConsoleSurgery:
	case AuPose::ConsolePilot:
	case AuPose::ConsolePower:
	case AuPose::ConsoleSensor:
	case AuPose::ConsoleShield:
	case AuPose::ConsoleSurgeryUnit:
	case AuPose::ConsoleTransporter:
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::ConsoleTurretBottom:
	case AuPose::ConsoleTurretTop:
	case AuPose::Crouch:
	case AuPose::Ladder:
	case AuPose::Post_Sit:
	case AuPose::Post_Stand:
	case AuPose::Prone:
	case AuPose::Spawn:
	case AuPose::Stand:
	case AuPose::Command_Sit:
	case AuPose::Command_SitStationary:
	case AuPose::Command_Stand:
	case AuPose::Command_StandStationary:
	case AuPose::Engine_Sit:
	case AuPose::Engine_SitStationary:
	case AuPose::Engine_Stand:
	case AuPose::Engine_StandStationary:
	case AuPose::FireControl_Sit:
	case AuPose::FireControl_SitStationary:
	case AuPose::FireControl_Stand:
	case AuPose::FireControl_StandStationary:
	case AuPose::Helm_Sit:
	case AuPose::Helm_SitStationary:
	case AuPose::Helm_Stand:
	case AuPose::Helm_StandStationary:
	case AuPose::Hug:
	case AuPose::MouthPursed:
	case AuPose::MouthClosed:
	case AuPose::MouthOpen:
	case AuPose::MouthAgape:
	case AuPose::MouthOpenWide:
	case AuPose::MouthYawn:
	case AuPose::MouthBite:
	case AuPose::MouthLaugh:
	case AuPose::MouthYap:
	case AuPose::MouthWhisper:
	case AuPose::MouthTalk:
	case AuPose::MouthJabber:
	case AuPose::MouthYell:
	case AuPose::MouthSmile:
	case AuPose::MouthGrin:
	case AuPose::MouthSad:
	case AuPose::MouthFrown:
	case AuPose::MouthCry:
	case AuPose::MouthPerplexed:
	case AuPose::MouthGrim:
	case AuPose::MouthSardonic:
	case AuPose::Medic_Sit:
	case AuPose::Medic_SitStationary:
	case AuPose::Medic_Stand:
	case AuPose::Medic_StandStationary:
	case AuPose::Nav_Sit:
	case AuPose::Nav_SitStationary:
	case AuPose::Nav_Stand:
	case AuPose::Nav_StandStationary:
	case AuPose::Patient_Sit:
	case AuPose::Patient_SitStationary:
	case AuPose::Patient_Stand:
	case AuPose::Patient_StandStationary:
	case AuPose::Power_Sit:
	case AuPose::Power_SitStationary:
	case AuPose::Power_Stand:
	case AuPose::Power_StandStationary:
	case AuPose::Sensor_Sit:
	case AuPose::Sensor_SitStationary:
	case AuPose::Sensor_Stand:
	case AuPose::Sensor_StandStationary:
	case AuPose::Shield_Sit:
	case AuPose::Shield_SitStationary:
	case AuPose::Shield_Stand:
	case AuPose::Shield_StandStationary:
	case AuPose::Transporter_Sit:
	case AuPose::Transporter_SitStationary:
	case AuPose::Transporter_Stand:
	case AuPose::Transporter_StandStationary:
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
		if(m_DNA.LegPairs)
			RelaxTail(hipHt);
		else
			SlitherTail(m_Pose[PS_Tail], 1);
		break;
	case AuPose::CrouchCreep:
	case AuPose::Dig:
	case AuPose::LadderClimb:
	case AuPose::Jump:
	case AuPose::ProneCreep:
	case AuPose::StandWalk:
		if(m_DNA.LegPairs)
			WaveTail(hipHt, speedEnc);
		else
			SlitherTail(m_Pose[PS_Tail], speedEnc);
		break;

	case AuPose::Fly:
	case AuPose::FlyFast:
	case AuPose::Glide:
	case AuPose::Hover:
		if(m_DNA.LegPairs)
			WaveTail(hipHt, speedEnc);
		else
			SlitherTail(m_Pose[PS_Tail], 1);
		break;
	case AuPose::DeadFaceDown:
	case AuPose::BerthBottomFaceDown:
	case AuPose::BerthTopFaceDown:
		DeadTailFacingDown();
		break;
	case AuPose::DeadFaceLeft:
	case AuPose::BerthBottomFaceLeft:
	case AuPose::BerthTopFaceLeft:
		DeadTailFacingLeft();
		break;
	case AuPose::DeadFaceRight:
	case AuPose::BerthBottomFaceRight:
	case AuPose::BerthTopFaceRight:
		DeadTailFacingRight();
		break;
	case AuPose::DeadFaceUp:
	case AuPose::BerthBottomFaceUp:
	case AuPose::BerthTopFaceUp:
		DeadTailFacingUp();
		break;
	case AuPose::Eat:
	case AuPose::Wag:
		if(IsSwimmingPose())
			WagTail(-1);
		else
			WagTail(hipHt);
		break;
	case AuPose::StandRun:
		if(m_DNA.LegPairs)
			LashTail(hipHt, speedEnc);
		else
			SlitherTail(m_Pose[PS_Tail], speedEnc);
		break;
	case AuPose::Kick:
	case AuPose::Lash:
		if(IsSwimmingPose())
			LashTail(-1, 1);
		else
			LashTail(hipHt, 1);
		break;
	case AuPose::SitOnGround:
	case AuPose::SitOnGroundCreep:
	case AuPose::Straddle:
	case AuPose::MateOnTop:
		FoldTail();
		break;
	case AuPose::Strike:
		if(IsSwimmingPose())
			StrikeTail(-1);
		else
			StrikeTail(hipHt);
		break;
	case AuPose::Swim:
	case AuPose::TreadWater:
	case AuPose::SwimFast:
		SwimTail();
		break;
	case AuPose::WritheFaceDown:
		WritheTailFacingDown();
		break;
	case AuPose::WritheFaceLeft:
		WritheTailFacingLeft();
		break;
	case AuPose::WritheFaceRight:
		WritheTailFacingRight();
		break;
	case AuPose::WritheFaceUp:
	case AuPose::Patient_Prone:
		WritheTailFacingUp();
		break;
	case AuPose::ReceptiveMate:
	case AuPose::MateOnBottom:
		ReceptiveTailFacingUp();
		break;
	}
}

// Moves the torso into position.
// NOTE: We don't currently move or bend the lower torso. Legs and tail are
// positioned before torso. If lower torso moves, this code should be changed
// to move them also.
float DpBody::PositionTorso(float hipHt, float speedEnc)
{
	// Determine waist rotation angle.
	float waistrot = 0;

	// Some poses are affected by the angle needed for the arms to touch the ground.
	const float standrot = StandingArmWaistRotation(hipHt);	// 0 if arms don't stand
	float sway = 0;	// sway angle, entire sway sweep is sway*2
	float encSpeed = 1;
	switch(m_Pose[PS_UpperTorso].Pose())
	{
	case AuPose::Init:
	case AuPose::DeadFaceDown:
	case AuPose::DeadFaceUp:
	case AuPose::BerthBottomFaceDown:
	case AuPose::BerthBottomFaceUp:
	case AuPose::BerthTopFaceDown:
	case AuPose::BerthTopFaceUp:
	case AuPose::EyesOnly:
	case AuPose::ReceptiveMate:
	case AuPose::MateOnBottom:
		break;
	case AuPose::Prone:
	case AuPose::ProneCreep:
		encSpeed = speedEnc;	// Body movement speed reduced by encumberance in this pose.
		break;
	case AuPose::Beckon:
	case AuPose::Bite:
	case AuPose::Block:
	case AuPose::Lash:
	case AuPose::Point:
	case AuPose::Psionic:
	case AuPose::Throw:
	case AuPose::Wag:
	case AuPose::Wave:
		Q_ASSERT(false);	// Invalid pose for upper torso.
		break;
	case AuPose::GameTableSeat0_0:
	case AuPose::GameTableSeat1_0:
	case AuPose::GameTableSeat2_0:
	case AuPose::GameTableSeat3_0:
	case AuPose::GameTableSeat0_45:
	case AuPose::GameTableSeat1_45:
	case AuPose::GameTableSeat2_45:
	case AuPose::GameTableSeat3_45:
	case AuPose::ConsoleCommand:
	case AuPose::ConsoleNav:
	case AuPose::ConsoleEngine:
	case AuPose::ConsoleFireControl:
	case AuPose::ConsoleSurgery:
	case AuPose::ConsolePilot:
	case AuPose::ConsolePower:
	case AuPose::ConsoleSensor:
	case AuPose::ConsoleShield:
	case AuPose::ConsoleTransporter:
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::ConsoleTurretBottom:
	case AuPose::ConsoleTurretTop:
	case AuPose::SitOnGround:
	case AuPose::SitOnGroundCreep:
	case AuPose::Command_Sit:
	case AuPose::Command_SitStationary:
	case AuPose::Engine_Sit:
	case AuPose::Engine_SitStationary:
	case AuPose::FireControl_Sit:
	case AuPose::FireControl_SitStationary:
	case AuPose::Helm_Sit:
	case AuPose::Helm_SitStationary:
	case AuPose::Hug:
	case AuPose::MouthPursed:
	case AuPose::MouthClosed:
	case AuPose::MouthOpen:
	case AuPose::MouthAgape:
	case AuPose::MouthOpenWide:
	case AuPose::MouthYawn:
	case AuPose::MouthBite:
	case AuPose::MouthLaugh:
	case AuPose::MouthYap:
	case AuPose::MouthWhisper:
	case AuPose::MouthTalk:
	case AuPose::MouthJabber:
	case AuPose::MouthYell:
	case AuPose::MouthSmile:
	case AuPose::MouthGrin:
	case AuPose::MouthSad:
	case AuPose::MouthFrown:
	case AuPose::MouthCry:
	case AuPose::MouthPerplexed:
	case AuPose::MouthGrim:
	case AuPose::MouthSardonic:
	case AuPose::Medic_Sit:
	case AuPose::Medic_SitStationary:
	case AuPose::Nav_Sit:
	case AuPose::Nav_SitStationary:
	case AuPose::Patient_Sit:
	case AuPose::Patient_SitStationary:
	case AuPose::Post_Sit:
	case AuPose::Power_Sit:
	case AuPose::Power_SitStationary:
	case AuPose::Sensor_Sit:
	case AuPose::Sensor_SitStationary:
	case AuPose::Shield_Sit:
	case AuPose::Shield_SitStationary:
	case AuPose::Transporter_Sit:
	case AuPose::Transporter_SitStationary:
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
		waistrot = PIf/2.f - 3.f*DEG2RADf;	// Upright leaning forward slightly.
		break;
	case AuPose::ConsoleSurgeryUnit:
	case AuPose::Crouch:
	case AuPose::Post_Stand:
	case AuPose::Spawn:
	case AuPose::Stand:
	case AuPose::Command_Stand:
	case AuPose::Command_StandStationary:
	case AuPose::Engine_Stand:
	case AuPose::Engine_StandStationary:
	case AuPose::FireControl_Stand:
	case AuPose::FireControl_StandStationary:
	case AuPose::Helm_Stand:
	case AuPose::Helm_StandStationary:
	case AuPose::Medic_Stand:
	case AuPose::Medic_StandStationary:
	case AuPose::Nav_Stand:
	case AuPose::Nav_StandStationary:
	case AuPose::Patient_Stand:
	case AuPose::Patient_StandStationary:
	case AuPose::Power_Stand:
	case AuPose::Power_StandStationary:
	case AuPose::Sensor_Stand:
	case AuPose::Sensor_StandStationary:
	case AuPose::Shield_Stand:
	case AuPose::Shield_StandStationary:
	case AuPose::Transporter_Stand:
	case AuPose::Transporter_StandStationary:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
		if(standrot != 0)
			waistrot = standrot;
		else
			waistrot = PIf/2.f - 5.f*DEG2RADf;	// Upright leaning forward.
		break;
	case AuPose::CrouchCreep:
		if(standrot != 0)
			waistrot = standrot;
		else
			waistrot = PIf/2.f - 5.f*DEG2RADf;	// Upright leaning forward.
		sway = 1.5f*DEG2RADf;
		encSpeed = speedEnc;	// Body movement speed reduced by encumberance in this pose.
		break;
	case AuPose::DeadFaceLeft:
	case AuPose::DeadFaceRight:
	case AuPose::BerthBottomFaceLeft:
	case AuPose::BerthBottomFaceRight:
	case AuPose::BerthTopFaceLeft:
	case AuPose::BerthTopFaceRight:
		if(standrot != 0)
			waistrot = standrot;
		else
			waistrot = -10.f*DEG2RADf;	// Leaning forward.
		break;
	case AuPose::Dig:
		if(standrot != 0)
			waistrot = standrot;
		else
			waistrot = PIf/2.f - 5.f*DEG2RADf;	// Upright leaning forward.
		sway = 1.5f*DEG2RADf;
		encSpeed = speedEnc;	// Body movement speed reduced by encumberance in this pose.
		break;
	case AuPose::Eat:
		if(standrot != 0)
			waistrot = standrot;
		else
			waistrot = 15.f*DEG2RADf;		// Upright leaning forward a lot.
		sway = 2.f*DEG2RADf;
		break;
	case AuPose::Fly:
		if(standrot != 0)
			waistrot = 15.f*DEG2RADf;			// Creature stands on its hands.
		else
			waistrot = 8.f*DEG2RADf;
		sway = 2.5f*DEG2RADf;
		break;
	case AuPose::FlyFast:
		if(standrot != 0)
			waistrot = 5.f*DEG2RADf;			// Creature stands on its hands.
		else
			waistrot = 1.f*DEG2RADf;
		sway = 3.5f*DEG2RADf;
		break;
	case AuPose::Glide:
	case AuPose::Hover:
		if(standrot != 0)
			waistrot = 25.f*DEG2RADf;			// Creature stands on its hands.
		else
			waistrot = 15.f*DEG2RADf;
		sway = 1.5f*DEG2RADf;
		break;
	case AuPose::Jump:
		if(standrot != 0)
			waistrot = standrot/2.f;			// Creature stands on its hands.
		else
			waistrot = PIf/2.f - 45.f*DEG2RADf;
		sway = 3.f*DEG2RADf;
		encSpeed = speedEnc;	// Body movement speed reduced by encumberance in this pose.
		break;
	case AuPose::Kick:
	case AuPose::Stomp:
		waistrot = PIf/2.f + 1.f*DEG2RADf;
		sway = 1.5f*DEG2RADf;
		break;
	case AuPose::Ladder:
	case AuPose::LadderClimb:
		waistrot = PIf/2.f;						// Straight upright spine parallel to ladder.
		encSpeed = speedEnc;	// Body movement speed reduced by encumberance in this pose.
		break;
	case AuPose::StandRun:
		if(standrot != 0)
			waistrot = standrot;				// Creature stands on its hands.
		else
			waistrot = PIf/2.f - 15.f*DEG2RADf;
		sway = 2.5f*DEG2RADf;
		encSpeed = speedEnc;	// Body movement speed reduced by encumberance in this pose.
		break;
	case AuPose::StandWalk:
		if(standrot != 0)
			waistrot = standrot;
		else
			waistrot = PIf/2.f - 5.f*DEG2RADf;	// Upright leaning forward.
		sway = 1.5f*DEG2RADf;
		encSpeed = speedEnc;	// Body movement speed reduced by encumberance in this pose.
		break;
	case AuPose::Straddle:
	case AuPose::MateOnTop:
		waistrot = PIf/2.f - 10.f*DEG2RADf;
		break;
	case AuPose::Strike:
		if(IsFlyingPose())
			waistrot = standrot - 10.f*DEG2RADf;
		else if(IsSwimmingPose())
			waistrot = 0;
		else
			waistrot = PIf/4.f;
		sway = 25.f*DEG2RADf;
		break;
	case AuPose::Swim:
		waistrot = 0;
		sway = 1.5f*DEG2RADf;
		break;
	case AuPose::SwimFast:
		waistrot = 0;
		sway = 2.5f*DEG2RADf;
		break;
	case AuPose::TreadWater:
		if(standrot != 0)
			waistrot = 0;
		else
			waistrot = PIf/2.f - 3.f*DEG2RADf;
		sway = .5f*DEG2RADf;
		break;
	case AuPose::Upright:
		waistrot = PIf/2.f - 2.f*DEG2RADf;
		sway = 1.f*DEG2RADf;
		break;
	case AuPose::WritheFaceDown:
	case AuPose::WritheFaceUp:
	case AuPose::Patient_Prone:
		sway = .5f*DEG2RADf;
		break;
	case AuPose::WritheFaceLeft:
	case AuPose::WritheFaceRight:
		if(standrot != 0)
			waistrot = standrot;
		else
			waistrot = -10.f*DEG2RADf;	// Leaning forward.
		sway = .5f*DEG2RADf;
		break;
	}

	if(waistrot != 0 || sway != 0)
	{
		// Rotate the upper torso at the waist to the calculated waist angle.
		if(sway != 0)
			waistrot += sway * m_DNA.Rhythm(m_Pose[PS_UpperTorso], 0, encSpeed);
		if(!RotateWaist(waistrot))
			waistrot = 0;	// There is no upper torso to rotate.
	}

	return waistrot;
}

void DpBody::PositionWings(float waistRot)
{
	switch(m_Pose[PS_Wings].Pose())
	{
	case AuPose::Init:
	case AuPose::EyesOnly:
		break;
	case AuPose::Beckon:
	case AuPose::Bite:
	case AuPose::Block:
	case AuPose::Eat:
	case AuPose::Kick:
	case AuPose::Lash:
	case AuPose::Point:
	case AuPose::Psionic:
	case AuPose::Stomp:
	case AuPose::Strike:
	case AuPose::Throw:
	case AuPose::Upright:
	case AuPose::Wag:
	case AuPose::Wave:
		Q_ASSERT(false);	// Invalid pose for wings.
		break;
	case AuPose::GameTableSeat0_0:
	case AuPose::GameTableSeat1_0:
	case AuPose::GameTableSeat2_0:
	case AuPose::GameTableSeat3_0:
	case AuPose::GameTableSeat0_45:
	case AuPose::GameTableSeat1_45:
	case AuPose::GameTableSeat2_45:
	case AuPose::GameTableSeat3_45:
	case AuPose::ConsoleCommand:
	case AuPose::ConsoleNav:
	case AuPose::ConsoleEngine:
	case AuPose::ConsoleFireControl:
	case AuPose::ConsoleSurgery:
	case AuPose::ConsolePilot:
	case AuPose::ConsolePower:
	case AuPose::ConsoleSensor:
	case AuPose::ConsoleShield:
	case AuPose::ConsoleSurgeryUnit:
	case AuPose::ConsoleTransporter:
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::ConsoleTurretBottom:
	case AuPose::ConsoleTurretTop:
	case AuPose::Crouch:
	case AuPose::CrouchCreep:
	case AuPose::Dig:
	case AuPose::Jump:
	case AuPose::Ladder:
	case AuPose::LadderClimb:
	case AuPose::Post_Sit:
	case AuPose::Post_Stand:
	case AuPose::Prone:
	case AuPose::ProneCreep:
	case AuPose::SitOnGround:
	case AuPose::SitOnGroundCreep:
	case AuPose::Spawn:
	case AuPose::Stand:
	case AuPose::StandRun:
	case AuPose::StandWalk:
	case AuPose::Straddle:
	case AuPose::MateOnTop:
	case AuPose::TreadWater:
	case AuPose::Command_Sit:
	case AuPose::Command_SitStationary:
	case AuPose::Command_Stand:
	case AuPose::Command_StandStationary:
	case AuPose::Engine_Sit:
	case AuPose::Engine_SitStationary:
	case AuPose::Engine_Stand:
	case AuPose::Engine_StandStationary:
	case AuPose::FireControl_Sit:
	case AuPose::FireControl_SitStationary:
	case AuPose::FireControl_Stand:
	case AuPose::FireControl_StandStationary:
	case AuPose::Helm_Sit:
	case AuPose::Helm_SitStationary:
	case AuPose::Helm_Stand:
	case AuPose::Helm_StandStationary:
	case AuPose::Hug:
	case AuPose::MouthPursed:
	case AuPose::MouthClosed:
	case AuPose::MouthOpen:
	case AuPose::MouthAgape:
	case AuPose::MouthOpenWide:
	case AuPose::MouthYawn:
	case AuPose::MouthBite:
	case AuPose::MouthLaugh:
	case AuPose::MouthYap:
	case AuPose::MouthWhisper:
	case AuPose::MouthTalk:
	case AuPose::MouthJabber:
	case AuPose::MouthYell:
	case AuPose::MouthSmile:
	case AuPose::MouthGrin:
	case AuPose::MouthSad:
	case AuPose::MouthFrown:
	case AuPose::MouthCry:
	case AuPose::MouthPerplexed:
	case AuPose::MouthGrim:
	case AuPose::MouthSardonic:
	case AuPose::Medic_Sit:
	case AuPose::Medic_SitStationary:
	case AuPose::Medic_Stand:
	case AuPose::Medic_StandStationary:
	case AuPose::Nav_Sit:
	case AuPose::Nav_SitStationary:
	case AuPose::Nav_Stand:
	case AuPose::Nav_StandStationary:
	case AuPose::Patient_Sit:
	case AuPose::Patient_SitStationary:
	case AuPose::Patient_Stand:
	case AuPose::Patient_StandStationary:
	case AuPose::Power_Sit:
	case AuPose::Power_SitStationary:
	case AuPose::Power_Stand:
	case AuPose::Power_StandStationary:
	case AuPose::Sensor_Sit:
	case AuPose::Sensor_SitStationary:
	case AuPose::Sensor_Stand:
	case AuPose::Sensor_StandStationary:
	case AuPose::Shield_Sit:
	case AuPose::Shield_SitStationary:
	case AuPose::Shield_Stand:
	case AuPose::Shield_StandStationary:
	case AuPose::Transporter_Sit:
	case AuPose::Transporter_SitStationary:
	case AuPose::Transporter_Stand:
	case AuPose::Transporter_StandStationary:
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
		StandWings(waistRot);
		break;
	case AuPose::DeadFaceDown:
	case AuPose::BerthBottomFaceDown:
	case AuPose::BerthTopFaceDown:
		DeadWingsFacingDown(waistRot);
		break;
	case AuPose::DeadFaceLeft:
	case AuPose::BerthBottomFaceLeft:
	case AuPose::BerthTopFaceLeft:
		DeadWingsFacingLeft(waistRot);
		break;
	case AuPose::DeadFaceRight:
	case AuPose::BerthBottomFaceRight:
	case AuPose::BerthTopFaceRight:
		DeadWingsFacingRight(waistRot);
		break;
	case AuPose::DeadFaceUp:
	case AuPose::BerthBottomFaceUp:
	case AuPose::BerthTopFaceUp:
		DeadWingsFacingUp(waistRot);
		break;
	case AuPose::Fly:
	case AuPose::SwimFast:
		FlyWings(waistRot, false);	// Cyclic flying position.
		break;
	case AuPose::FlyFast:
	case AuPose::Hover:
		FlyWings(waistRot, true);		// Cyclic flying position.
		break;
	case AuPose::Glide:
	case AuPose::Swim:
		GlideWings(waistRot);	// Cyclic flying position.
		break;
	case AuPose::WritheFaceDown:
		WritheWingsFacingDown(waistRot);
		break;
	case AuPose::WritheFaceLeft:
		WritheWingsFacingLeft(waistRot);
		break;
	case AuPose::WritheFaceRight:
		WritheWingsFacingRight(waistRot);
		break;
	case AuPose::WritheFaceUp:
	case AuPose::ReceptiveMate:
	case AuPose::MateOnBottom:
	case AuPose::Patient_Prone:
		WritheWingsFacingUp(waistRot);
		break;
	}
}

// Calculate the upper torso rotation angle required to stand arms naturally.
// Return 0 indicates arms are not used when standing.
float DpBody::StandingArmWaistRotation(float hipHt) const
{
	float waistrot = 0;	// Non-zero indicates ability to stand with hands on
						// ground.
	if(m_DNA.WalksOnHands())
	{
		// Hands that are used for feet will touch the ground when standing.
		// This will likely require a bend at the waist. Such arms are held at
		// near maximum extension, or at least the farthest the body will allow.
		if(!m_DNA.TorsoVertebrae || !m_DNA.WaistVertex())
		{
			// There is no upper torso. A vertical waist rotation hangs the
			// arms in a sensible way even though they will emanate from the
			// front of the lower torso.
			waistrot = PIf/2.f;
		}
		else
		{
			// Has a back and a waist, which means a torso above the waist.
			// This is the most complex case. We must lean the body forward so
			// the first set of arms reaches the ground. Other pairs of arms
			// must then be extended to touch the ground in a sensible way.
			float torso = TorsoLength();
			Q_ASSERT(torso > 0);
			if(torso != 0)
			{
				// Calculate height of shoulder at optimum arm extension.
				float ht = 0;
				const float upperarmlen = UpperArmLength();
				const float forearmlen = ForeArmLength();
				if(m_DNA.HasWristBend())
				{
					// These hand types have a wrist bend.
					// Arms bend differently than legs.
					// The calc is just like no wrist bend except that the
					// point of contact is the end of the forearm instead of
					// the forearm and the hand.
					ht = upperarmlen + forearmlen;
					ht *= 0.95f;	// Arms are mostly extended when standing.
				}
				else
				{
					// These hand types do not have a wrist bend.
					float handlen = HandLength();
					ht = upperarmlen + forearmlen + handlen;
					ht *= 0.95f;	// Arms are mostly extended when standing.
				}

				// Calculate height above ground if torso was pointing straight
				// downward from waist, that is the height of the hip minus the
				// length of the torso.
				float tht = hipHt - torso;	// Closest torso can get to ground.
				float nht = ht - hipHt;		// Height change from waist to neck.
				if(tht > ht || fabs(nht) >= torso)
				{
					// The closest height to the torso is greater than the
					// shoulder height of the arms. The arms can't reach the
					// ground so they can't really be used for walking.
					//  or
					// Shoulder height is higher than torso length?
					waistrot = 0;
				}
				else
				{
					// Hand type can be used as feet and they reach the ground.
					waistrot = asinf(nht/torso);
				}
			}
		}
	}
	return waistrot;
}

// Heads tilt at base of each head to a specified angle. 0 = straight and level.
void DpBody::TiltHeadsTo(float tiltAngle, float turnAngle)
{
	//qDebug("DpBody::TiltHeadsTo %g %s", angle*RAD2DEG,
			//(const char*)m_DNA.Description());
	float tilt = tiltAngle;
	if(tilt > PIf/2.f)
		tilt = PIf/2.f;
	else if(tilt < -PIf/2.f)
		tilt = -PIf/2.f;
	m_HeadTilt = tilt;
	m_HeadTurn = turnAngle;
}

// Rotates body head and arms around X at waist.
bool DpBody::RotateWaist(float angle)
{
	if(angle == 0.0)
		return false;

	unsigned int waist = m_DNA.WaistVertex();
	if(!waist)
		return false;

	AuPoint waistloc = CurrentWaistLoc();
	unsigned int waistBone = DhDNA::BONE_Torso0;
	if(waist)
		waistBone += (waist - 1);		// Last bone above waist vertex.

	/*
#if !defined(QT_NO_DEBUG)
	for(int ib=0; ib<DhDNA::BONE_Max; ib++)
	{
		const AuMatrix4x4& bmat = m_Bones[ib];
		if(bmat[3] != 0 || bmat[7] != 0 || bmat[11] != 0 || bmat[15] != 1)
		{
			qDebug("DpBody::RotateWaist - Bad Bone Matrix %d in %s", ib,
					(const char*)m_DNA.Description().toAscii());
			bmat.Dump("DpBody::RotateWaist Begin");
			DNA().Dump();
			Q_ASSERT(false);
		}
	}
#endif
*/
	AuMatrix4x4 t;
	t.Translate(waistloc);
	t.RotateX(angle);
	t.Translate(-waistloc);

	for(QVector<DpBodyPart*>::iterator it = begin(); it != end(); it++)
	{
		if((*it)->IsTorso())
		{
			// Rotate torso bones above the waist at the waist.
			DpTorso* torso = (DpTorso*)(*it);
			torso->RotateWaist(angle);
		}
		else if((*it)->IsHeadAndNeck())
		{
			// Move necks their nu locations without rotating them.
			AuPoint oldCon = (*it)->CurrentConnectLoc();
			AuPoint nuCon = t.Project(oldCon);
			AuPoint vmove = nuCon - oldCon;
			(*it)->MoveBy(vmove);

			// Rotate necks to match waist angle.
			// Move necks and arms to their nu locations without rotating them.
			DpHeadAndNeck* head = (DpHeadAndNeck*)(*it);
			head->RotateNeck(angle);
		}
		else if((*it)->IsArmAndHand())
		{
			// Move arms to their nu locations without rotating them.
			AuPoint oldCon = (*it)->CurrentConnectLoc();
			AuPoint nuCon = t.Project(oldCon);
			AuPoint vmove = nuCon - oldCon;
			(*it)->MoveBy(vmove);
		}
	}
	/*
#if !defined(QT_NO_DEBUG)
	for(int ib=0; ib<DhDNA::BONE_Max; ib++)
	{
		const AuMatrix4x4& bmat = m_Bones[ib];
		if(bmat[3] != 0 || bmat[7] != 0 || bmat[11] != 0 || bmat[15] != 1)
		{
			qDebug("DpBody::RotateWaist - Done. Bad Bone Matrix %d in %s", ib,
					(const char*)m_DNA.Description().toAscii());
			bmat.Dump("RotateWaist Done");
			DNA().Dump();
			//Q_ASSERT(false);
		}
	}
#endif
	*/
	return true;
}

AuPoint DpBody::CurrentWaistLoc() const
{
	if(isEmpty())
		return AuPoint(0,0);

	// First body part is always a torso.
	DpBodyPart* part = first();
	if(!part || !part->IsTorso())
	{
		Q_ASSERT(false);
		return AuPoint(0,0);
	}
	DpTorso* torso = (DpTorso*)part;
	return torso->CurrentConnectLoc();
}

// which 0=left, 1=right, 2=both
void DpBody::BeckonArms(int which)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = -5.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle = 20.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
				arm->Beckon(m_HeadTilt);
		}
}

// Bends all necks in XZ plane at each vertex.
void DpBody::BendNecks(float angle)
{
	// Can skip first bone because it's always a torso.
	if(count() < 2)
		return;
	if(DNA().Heads)
	{
		// Two heads.
		bool isLeft = true;	// first head is left
		for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
			if((*it)->IsHeadAndNeck())
			{
				DpHeadAndNeck* head = (DpHeadAndNeck*)(*it);
				head->BendNeck(angle);
				if(isLeft)
					isLeft = false;
				else
					break;
			}
	}
	else
	{
		// Only one head.
		for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
			if((*it)->IsHeadAndNeck())
			{
				DpHeadAndNeck* head = (DpHeadAndNeck*)(*it);
				head->BendNeck(angle);
				break;
			}
	}
}

// which 0=left, 1=right, 2=both
void DpBody::BlockArms(int which)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = -40.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle = 40.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
				arm->Block(m_HeadTilt);
		}
}

void DpBody::DigArms(int which)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = -10.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle = 15.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
				arm->Dig();
		}
}

// Only climb with front legs.
void DpBody::ClimbLegs(float hipHt, float speedEnc)
{
	// Most critters carry their legs close to their bodies, with their feet
	// under them. If they won't work in this position, SpreadLegs will evenly
	// distribute them like a spider or crab, ignoring the angle we pass here.
	float hipSplay = 15.f*DEG2RADf;	// Rotation in XY plane, i.e. critter left/right
	SpreadLegs(hipSplay, 1);	// Move knees away from body, rotating each leg
							// at the hips so they are not on top of each other.

	// Position legs.
	// Legs are always the last body parts. Iterate from the end and stop when
	// we get to the first non-leg.
	for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin(); it--)
		if((*it)->IsLegAndFoot())
		{
			DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
			if(leg->IsFront())
				leg->Climb(hipHt, speedEnc);
		}
		else
			break;	// Stop at first non-leg body part.
}

// Only dig with front legs.
void DpBody::DigLegs(float hipHt, float speedEnc)
{
	// Most critters carry their legs close to their bodies, with their feet
	// under them. If they won't work in this position, SpreadLegs will evenly
	// distribute them like a spider or crab, ignoring the angle we pass here.
	float hipSplay = 15.f*DEG2RADf;	// Rotation in XY plane, i.e. critter left/right
	SpreadLegs(hipSplay, 1);	// Move knees away from body, rotating each leg
							// at the hips so they are not on top of each other.

	// Position legs.
	// Legs are always the last body parts. Iterate from the end and stop when
	// we get to the first non-leg.
	for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin(); it--)
		if((*it)->IsLegAndFoot())
		{
			DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
			if(leg->IsFront())
				leg->Dig(hipHt, speedEnc);
		}
		else
			break;	// Stop at first non-leg body part.
}

// Feed myself.
// which 0=left, 1=right, 2=both
void DpBody::EatWithArms(float hipHt, int which)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = 20.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle = 4.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
				arm->Eat(hipHt);
		}
}

// Position wings for flying.
// Does not affect arms.
void DpBody::FlyWings(float waistRot, bool fast)
{
	// Position wings for flying.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(!arm->IsWing())
				continue;
			arm->FlyWing(fast+1);

			float rotwing;
			if(m_DNA.TorsoVertebrae)
				rotwing = waistRot - PIf/2.0;
			else
				rotwing = -PIf/2.;	// No vertebrae so rotate 90 degrees
			arm->RotateShoulder(rotwing);
			if(arm->IsWingLobed())
				arm->RotateElbow(rotwing);	// Rotate lower wing lobe also.
		}
}

// Fold legs close to body.
void DpBody::FoldArms(int which, bool bendFirstArm)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = 10.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle = 10.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
			{
				arm->Fold(bendFirstArm);
				bendFirstArm = false;
			}
		}
}

void DpBody::FoldLegs(int which)
{
	// Most critters carry their legs close to their bodies, with their feet
	// under them. If they won't work in this position, SpreadLegs will evenly
	// distribute them like a spider or crab, ignoring the angle we pass here.
	float hipSplay = 20.f*DEG2RADf;	// Rotation in XY plane, i.e. critter left/right
	SpreadLegs(hipSplay, which);	// Move knees away from body, rotating each
							// leg at the hips so they are not on top of each
							// other.

	// Position legs.
	// Legs are always the last body parts. Iterate from the end and stop when
	// we get to the first non-leg.
	for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin(); it--)
		if((*it)->IsLegAndFoot())
		{
			DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
			if(which == 2 ||
					(which == 0 && !leg->IsFront()) ||
					(which == 1 && leg->IsFront()))
				leg->Fold();
		}
		else
			break;	// Stop at first non-leg body part.
}

// Fold tail so this thing can sit on its butt.
void DpBody::FoldTail()
{
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsTail())
		{
			DpTail* tail = (DpTail*)(*it);
			tail->Fold();
			break;	// There is only one tail.
		}
}

// Position wings for gliding.
// Does not affect arms.
void DpBody::GlideWings(float waistRot)
{
	// Position wings for flying.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(!arm->IsWing())
				continue;
			arm->FlyWing(0);

			float rotwing;
			if(m_DNA.TorsoVertebrae)
				rotwing = waistRot - PIf/2.0;
			else
				rotwing = -PIf/2.;	// No vertebrae so rotate 90 degrees
			arm->RotateShoulder(rotwing);
			if(arm->IsWingLobed())
				arm->RotateElbow(rotwing);	// Rotate lower wing lobe also.
		}
}

// which 0=left, 1=right, 2=both
void DpBody::JumpArms(float hipHt, int which, bool bendFirstArm, float speedEnc)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = -2.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle = 5.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
			{
				arm->Jump(hipHt, bendFirstArm, speedEnc);
				bendFirstArm = false;
			}
		}
}

// which 0=back, 1=front, 2=both
void DpBody::JumpLegs(float hipHt, int which, float speedEnc)
{
	// Most critters carry their legs close to their bodies, with their feet
	// under them. If they won't work in this position, SpreadLegs will evenly
	// distribute them like a spider or crab, ignoring the angle we pass here.
	float hipSplay = 10.f*DEG2RADf;	// Rotation in XY plane, i.e. critter left/right
	SpreadLegs(hipSplay, which);	// Move knees away from body, rotating each
							// leg at the hips so they are not on top of each
							// other.

	// Position legs.
	// Legs are always the last body parts. Iterate from the end and stop when
	// we get to the first non-leg.
	for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin(); it--)
		if((*it)->IsLegAndFoot())
		{
			DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
			if(which == 2 ||
					(which == 0 && !leg->IsFront()) ||
					(which == 1 && leg->IsFront()))
				leg->Jump(hipHt, speedEnc);
		}
		else
			break;	// Stop at first non-leg body part.
}

// which 0=back, 1=front, 2=both
void DpBody::KickLegs(float hipHt, int which)
{
	// Most critters carry their legs close to their bodies, with their feet
	// under them. If they won't work in this position, SpreadLegs will evenly
	// distribute them like a spider or crab, ignoring the angle we pass here.
	float hipSplay = 10.f*DEG2RADf;	// Rotation in XY plane, i.e. critter left/right
	SpreadLegs(hipSplay, which);	// Move knees away from body, rotating each
							// leg at the hips so they are not on top of each
							// other.

	// Position legs.
	// Legs are always the last body parts. Iterate from the end and stop when
	// we get to the first non-leg.
	for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin(); it--)
		if((*it)->IsLegAndFoot())
		{
			DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
			if(which == 2 ||
					(which == 0 && !leg->IsFront()) ||
					(which == 1 && leg->IsFront()))
			{
				// When kicking front legs, only kick with the dominant leg.
				if(leg->IsFront())
				{
					// This is a front leg.
					if(IsRightHanded())
					{
						// I am right handed.
						if(leg->IsLeft())
						{
							leg->Stand(hipHt);
							continue;	// Don't kick with left leg.
						}
					}
					else
					{
						// I am left handed.
						if(!leg->IsLeft())
						{
							leg->Stand(hipHt);
							continue;	// Don't kick with right leg.
						}
					}
				}
				leg->Kick(hipHt);
			}
		}
		else
			break;	// Stop at first non-leg body part.
}

void DpBody::LashTail(float hipHt, float speedEnc)
{
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsTail())
		{
			DpTail* tail = (DpTail*)(*it);
			tail->Lash(hipHt, speedEnc);
			break;	// There is only one tail.
		}
}

void DpBody::SlitherTail(const AuPose& tailPose, float speedEnc)
{
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsTail())
		{
			DpTail* tail = (DpTail*)(*it);
			tail->Slither(tailPose, speedEnc);
			break;	// There is only one tail.
		}
}

// which 0=left, 1=right, 2=both
void DpBody::PointArms(int which)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = -1.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle = 5.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
				arm->Point(m_HeadTilt);
		}
}

// which 0=left, 1=right, 2=both
void DpBody::PsionicArms(int which)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = -30.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle = 40.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
				arm->PsionicWave(m_HeadTilt);
		}
}

void DpBody::DeadArmsFacingDown(int which)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = 25.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle = 0;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
				arm->DeadFacingDown();
		}
}

void DpBody::DeadArmsFacingLeft(int which)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = 0;	// Rotation in XY plane.
	float liftangle = 35.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
				arm->DeadFacingLeft();
		}
}

void DpBody::DeadArmsFacingRight(int which)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = 0;	// Rotation in XY plane.
	float liftangle = 35.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
				arm->DeadFacingRight();
		}
}

void DpBody::DeadArmsFacingUp(int which)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = 25.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle = 0;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
				arm->DeadFacingUp();
		}
}

void DpBody::DeadLegsFacingDown(int which)
{
	// Most critters carry their legs close to their bodies, with their feet
	// under them. If they won't work in this position, SpreadLegs will evenly
	// distribute them like a spider or crab, ignoring the angle we pass here.
	float hipSplay = -1.f*DEG2RADf;	// Rotation in XY plane, i.e. critter left/right
	SpreadLegs(hipSplay, which);	// Move knees away from body, rotating each
							// leg at the hips so they are not on top of each
							// other.

	// Position legs.
	// Legs are always the last body parts. Iterate from the end and stop when
	// we get to the first non-leg.
	for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin(); it--)
		if((*it)->IsLegAndFoot())
		{
			DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
			if(which == 2 ||
					(which == 0 && !leg->IsFront()) ||
					(which == 1 && leg->IsFront()))
				leg->DeadFacingDown();
		}
		else
			break;	// Stop at first non-leg body part.
}

void DpBody::DeadLegsFacingLeft(int which)
{
	// Most critters carry their legs close to their bodies, with their feet
	// under them. If they won't work in this position, SpreadLegs will evenly
	// distribute them like a spider or crab, ignoring the angle we pass here.
	float hipSplay = -2.f*DEG2RADf;	// Rotation in XY plane, i.e. critter
									// left/right
	SpreadLegs(hipSplay, which);	// Move knees away from body, rotating each
						// leg at the hips so they are not on top of each other.

	// Position legs.
	// Legs are always the last body parts. Iterate from the end and stop when
	// we get to the first non-leg.
	for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin(); it--)
		if((*it)->IsLegAndFoot())
		{
			DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
			if(which == 2 ||
					(which == 0 && !leg->IsFront()) ||
					(which == 1 && leg->IsFront()))
				leg->DeadFacingLeft();
		}
		else
			break;	// Stop at first non-leg body part.
}

void DpBody::DeadLegsFacingRight(int which)
{
	// Most critters carry their legs close to their bodies, with their feet
	// under them. If they won't work in this position, SpreadLegs will evenly
	// distribute them like a spider or crab, ignoring the angle we pass here.
	float hipSplay = -2.f*DEG2RADf;	// Rotation in XY plane, i.e. critter
									// left/right
	SpreadLegs(hipSplay, which);	// Move knees away from body, rotating each
						// leg at the hips so they are not on top of each other.

	// Position legs.
	// Legs are always the last body parts. Iterate from the end and stop when
	// we get to the first non-leg.
	for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin(); it--)
		if((*it)->IsLegAndFoot())
		{
			DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
			if(which == 2 ||
					(which == 0 && !leg->IsFront()) ||
					(which == 1 && leg->IsFront()))
				leg->DeadFacingRight();
		}
		else
			break;	// Stop at first non-leg body part.
}

void DpBody::DeadLegsFacingUp(int which)
{
	// Most critters carry their legs close to their bodies, with their feet
	// under them. If they won't work in this position, SpreadLegs will evenly
	// distribute them like a spider or crab, ignoring the angle we pass here.
	float hipSplay = -1.f*DEG2RADf;	// Rotation in XY plane, i.e. critter left/right
	SpreadLegs(hipSplay, which);	// Move knees away from body, rotating each
						// leg at the hips so they are not on top of each other.

	// Position legs.
	// Legs are always the last body parts. Iterate from the end and stop when
	// we get to the first non-leg.
	for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin(); it--)
		if((*it)->IsLegAndFoot())
		{
			DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
			if(which == 2 ||
					(which == 0 && !leg->IsFront()) ||
					(which == 1 && leg->IsFront()))
				leg->DeadFacingUp();
		}
		else
			break;	// Stop at first non-leg body part.
}

void DpBody::DeadTailFacingDown()
{
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsTail())
		{
			DpTail* tail = (DpTail*)(*it);
			tail->DeadFacingDown();
			break;	// There is only one tail.
		}
}

void DpBody::DeadTailFacingLeft()
{
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsTail())
		{
			DpTail* tail = (DpTail*)(*it);
			tail->DeadFacingLeft();
			break;	// There is only one tail.
		}
}

void DpBody::DeadTailFacingRight()
{
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsTail())
		{
			DpTail* tail = (DpTail*)(*it);
			tail->DeadFacingRight();
			break;	// There is only one tail.
		}
}

void DpBody::DeadTailFacingUp()
{
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsTail())
		{
			DpTail* tail = (DpTail*)(*it);
			tail->DeadFacingUp();
			break;	// There is only one tail.
		}
}

void DpBody::RelaxTail(float hipHt)
{
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsTail())
		{
			DpTail* tail = (DpTail*)(*it);
			tail->Relax(hipHt);
			break;	// There is only one tail.
		}
}

void DpBody::DeadWingsFacingDown(float waistRot)
{
	// Position wings for flying.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(!arm->IsWing())
				continue;
			arm->DeadWingFacingDown();

			float rotwing;
			if(m_DNA.TorsoVertebrae)
				rotwing = waistRot - PIf/2.0;
			else
				rotwing = -PIf/2.;	// No vertebrae so rotate 90 degrees
			arm->RotateShoulder(rotwing);
			if(arm->IsWingLobed())
				arm->RotateElbow(rotwing);	// Rotate lower wing lobe also.
		}
}

void DpBody::DeadWingsFacingLeft(float waistRot)
{
	// Position wings for flying.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(!arm->IsWing())
				continue;
			arm->DeadWingFacingLeft();

			float rotwing;
			if(m_DNA.TorsoVertebrae)
				rotwing = waistRot - PIf/2.0;
			else
				rotwing = -PIf/2.;	// No vertebrae so rotate 90 degrees
			arm->RotateShoulder(rotwing);
			if(arm->IsWingLobed())
				arm->RotateElbow(rotwing);	// Rotate lower wing lobe also.
		}
}

void DpBody::DeadWingsFacingRight(float waistRot)
{
	// Position wings for flying.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(!arm->IsWing())
				continue;
			arm->DeadWingFacingRight();

			float rotwing;
			if(m_DNA.TorsoVertebrae)
				rotwing = waistRot - PIf/2.0;
			else
				rotwing = -PIf/2.;	// No vertebrae so rotate 90 degrees
			arm->RotateShoulder(rotwing);
			if(arm->IsWingLobed())
				arm->RotateElbow(rotwing);	// Rotate lower wing lobe also.
		}
}

void DpBody::DeadWingsFacingUp(float waistRot)
{
	// Position wings for flying.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(!arm->IsWing())
				continue;
			arm->DeadWingFacingUp();

			float rotwing;
			if(m_DNA.TorsoVertebrae)
				rotwing = waistRot - PIf/2.0;
			else
				rotwing = -PIf/2.;	// No vertebrae so rotate 90 degrees
			arm->RotateShoulder(rotwing);
			if(arm->IsWingLobed())
				arm->RotateElbow(rotwing);	// Rotate lower wing lobe also.
		}
}

// which 0=left, 1=right, 2=both
void DpBody::RunArms(float hipHt, int which, bool bendFirstArm, float speedEnc)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = 5.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle;
	if(m_DNA.WalksOnHands())
		liftangle = 1.f*DEG2RADf;
	else
		liftangle = 10.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
			{
				arm->Run(hipHt, bendFirstArm, speedEnc);
				bendFirstArm = false;
			}
		}
}

// which 0=back, 1=front, 2=both
void DpBody::RunLegs(float hipHt, int which, float speedEnc)
{
	// Most critters carry their legs close to their bodies, with their feet
	// under them. If they won't work in this position, SpreadLegs will evenly
	// distribute them like a spider or crab, ignoring the angle we pass here.
	float hipSplay = 1.f*DEG2RADf;	// Rotation in XY plane, i.e. critter left/right
	SpreadLegs(hipSplay, which);	// Move knees away from body, rotating each
						// leg at the hips so they are not on top of each other.

	// Position legs.
	// Legs are always the last body parts. Iterate from the end and stop when
	// we get to the first non-leg.
	for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin(); it--)
		if((*it)->IsLegAndFoot())
		{
			DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
			if(which == 2 ||
					(which == 0 && !leg->IsFront()) ||
					(which == 1 && leg->IsFront()))
				leg->Run(hipHt, speedEnc);
		}
		else
			break;	// Stop at first non-leg body part.
}

// Sets spread angles on arms.
// Handles multiple limbs at node.
// Does not affect wings.
void DpBody::SpreadArms(float splayAng, float liftAng, int which)
{
	QMap<AuPoint, int> distmap;	// Map of number of pairs of limbs at each loc.
	for(QVector<DpBodyPart*>::const_iterator it = constBegin() + 1;
			it != constEnd(); it++)
	{
		if((*it)->IsArmAndHand())
		{
			const DpArmAndHand* arm = (const DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			AuPoint shoulderLoc = arm->ConnectLoc();
			int ex = distmap.value(shoulderLoc, 0);
			ex++;
			distmap.insert(shoulderLoc, ex);
		}
	}

	// Now we know the number of pairs of limbs at each joint. Spread them accordingly.
	for(QMap<AuPoint,int>::const_iterator mi = distmap.constBegin();
			mi != distmap.constEnd(); mi++)
	{
		const AuPoint& jointLoc = mi.key();	// Joint base location.
		float splay = 0;				// Angle of first limb.
		float lift = liftAng * 3.f/4.f;
		// Can skip first bone because it's always a BONE_Body.
		for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		{
			if((*it)->IsArmAndHand())
			{
				DpArmAndHand* arm = (DpArmAndHand*)(*it);
				if(arm->IsWing())
					continue;
				AuPoint shoulderLoc = arm->ConnectLoc();
				if(shoulderLoc == jointLoc)
				{
					splay += splayAng;		// Increment angle for this limb.
					lift += liftAng/4.f;
					if(which == 2 ||
							(which == 0 && arm->IsLeft()) ||
							(which == 1 && !arm->IsLeft()))
						arm->SetSpread(lift, splay);
				}
			}
		}
	}
}

// Sets spread angles on legs.
// Handles multiple limbs at node.
void DpBody::SpreadLegs(float splayAng, int which)
{
	QMap<AuPoint, int> distmap;	// Map of number of pairs of limbs at each loc.
	// Legs are always the last body parts. Iterate from the end and
	// stop when we get to the first non-leg.
	for(QVector<DpBodyPart*>::const_iterator it = constEnd() - 1;
			it != constBegin(); it--)
	{
		if((*it)->IsLegAndFoot())
		{
			const DpLegAndFoot* leg = (const DpLegAndFoot*)(*it);
			AuPoint hipLoc = leg->ConnectLoc();
			int ex = distmap.value(hipLoc, 0);
			ex++;
			distmap.insert(hipLoc, ex);
		}
		else
			break;	// Stop at first non-leg body part.
	}

	// Now we know the number of pairs of limbs at each hip.
	// Splay them accordingly.
	float hipHt = CGHeightStanding(false);
	const float thighlen = ThighLength();
	const float calflen = CalfLength();
	const float footlen = FootLength();
	float angleinc = splayAng;
	for(QMap<AuPoint,int>::const_iterator mi = distmap.constBegin();
			mi != distmap.constEnd(); mi++)
	{
		const AuPoint& jointLoc = mi.key();	// Joint base location.
		int limbs = mi.value();				// Limbs emanating from the hip.
		if(splayAng == 0.0)
			angleinc = PIf/(limbs+1);		// Evenly space limbs at hip.
		float splayang = 0.0;				// Angle of first limb.

		// Legs are always the last body parts. Iterate from the end and stop
		// when we get to the first non-leg.
		for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin(); it--)
		{
			if((*it)->IsLegAndFoot())
			{
				DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
				AuPoint hipLoc = leg->ConnectLoc();
				if(hipLoc == jointLoc)
				{
					// Increment angle for this limb.
					if(which == 2 ||
							(which == 0 && !leg->IsFront()) ||
							(which == 1 && leg->IsFront()))
					{
						if(leg->DoesTouchGroundBelowJoint(hipHt, thighlen,
									calflen, footlen))
							splayang += angleinc;	
						else
							splayang += PIf/(limbs+1);
						leg->SetSpread(splayang);
					}
				}
			}
			else
				break;	// Stop at first non-leg body part.
		}
	}
}

// Position for standing at hip height.
// Does not affect wings.
// which 0=left, 1=right, 2=both
void DpBody::StandArms(float hipHt, int which, bool bendFirstArm)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = 20.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle;
	if(m_DNA.WalksOnHands())
		liftangle = 1.f*DEG2RADf;
	else
		liftangle = 12.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
			{
				arm->Relax(hipHt, bendFirstArm);
				bendFirstArm = false;
			}
		}
}

// which 0=left, 1=right, 2=both
void DpBody::ClimbArms(int which, bool bendFirstArm, float speedEnc)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = 5.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle = 10.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
			{
				arm->Climb(bendFirstArm, speedEnc);
				bendFirstArm = false;
			}
		}
}

void DpBody::LadderArms(int which, bool bendFirstArm)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = -5.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle = 10.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
			{
				arm->Ladder(bendFirstArm);
				bendFirstArm = false;
			}
		}
}

// Position for straddle.
// Does not affect wings.
// which 0=left, 1=right, 2=both
void DpBody::StraddleArms(float hipHt, int which, bool bendFirstArm)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = 1.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle;
	if(m_DNA.WalksOnHands())
		liftangle = 1.f*DEG2RADf;
	else
		liftangle = 14.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
			{
				arm->Straddle(hipHt, bendFirstArm);
				bendFirstArm = false;
			}
		}
}

// Stand legs for desired hip height.
void DpBody::StandLegs(float hipHt, int which)
{
	// Most critters carry their legs close to their bodies, with their feet
	// under them. If they won't work in this position, SpreadLegs will evenly
	// distribute them like a spider or crab, ignoring the angle we pass here.
	float hipSplay = 20.f*DEG2RADf;	// Rotation in XY plane, i.e. critter
									// left/right
	SpreadLegs(hipSplay, which);	// Move knees away from body, rotating each
						// leg at the hips so they are not on top of each other.

	// Position legs.
	// Legs are always the last body parts. Iterate from the end and stop when
	// we get to the first non-leg.
	for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin() + 1; it--)
		if((*it)->IsLegAndFoot())
		{
			DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
			if(which == 2 ||
					(which == 0 && !leg->IsFront()) ||
					(which == 1 && leg->IsFront()))
				leg->Stand(hipHt);
		}
		else
			break;	// Stop at first non-leg body part.
}

// Position for standing at hip height.
// Does not affect arms.
void DpBody::StandWings(float waistRot)
{
	// Position wings for standing on ground.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(!arm->IsWing())
				continue;
			arm->StandWing();

			float rotwing;
			if(m_DNA.TorsoVertebrae)
				rotwing = waistRot - PIf/2.0;
			else
				rotwing = -PIf/2.;	// No vertebrae so rotate 90 degrees
			arm->RotateShoulder(rotwing);
			if(arm->IsWingLobed())
				arm->RotateElbow(rotwing);	// Rotate lower wing lobe also.
		}
}

// which 0=back, 1=front, 2=both
void DpBody::StompLegs(float hipHt, int which)
{
	// Most critters carry their legs close to their bodies, with their feet
	// under them. If they won't work in this position, SpreadLegs will evenly
	// distribute them like a spider or crab, ignoring the angle we pass here.
	float hipSplay = 15.f*DEG2RADf;	// Rotation in XY plane, i.e. critter left/right
	SpreadLegs(hipSplay, which);	// Move knees away from body, rotating each
						// leg at the hips so they are not on top of each other.

	// Position legs.
	// Legs are always the last body parts. Iterate from the end and stop when
	// we get to the first non-leg.
	for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin(); it--)
		if((*it)->IsLegAndFoot())
		{
			DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
			if(which == 2 ||
					(which == 0 && !leg->IsFront()) ||
					(which == 1 && leg->IsFront()))
			{
				// When stomping front legs, only stomp with the dominant leg.
				if(leg->IsFront())
				{
					// This is a front leg.
					if(IsRightHanded())
					{
						// I am right handed.
						if(leg->IsLeft())
						{
							leg->Stand(hipHt);
							continue;	// Don't stomp with left leg.
						}
					}
					else
					{
						// I am left handed.
						if(!leg->IsLeft())
						{
							leg->Stand(hipHt);
							continue;	// Don't stomp with right leg.
						}
					}
				}
				leg->Stomp(hipHt);
			}
		}
		else
			break;	// Stop at first non-leg body part.
}

// All legs only.
void DpBody::StraddleLegs()
{
	// Most critters carry their legs close to their bodies, with their feet
	// under them. If they won't work in this position, SpreadLegs will evenly
	// distribute them like a spider or crab, ignoring the angle we pass here.
	float hipSplay = 15.f*DEG2RADf;	// Rot in XY plane, i.e. critter left/right
	SpreadLegs(hipSplay, 2);	// Move knees away from body, rotating each leg
							// at the hips so they are not on top of each other.

	// Position legs.
	// Legs are always the last body parts. Iterate from the end and stop when
	// we get to the first non-leg.
	for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin(); it--)
		if((*it)->IsLegAndFoot())
		{
			DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
			leg->Straddle();
		}
		else
			break;	// Stop at first non-leg body part.
}

// which 0=left, 1=right, 2=both
void DpBody::StrikeArms(int which)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = 5.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle = 10.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
				arm->Pummel(m_HeadTilt);
		}
}

void DpBody::StrikeTail(float hipHt)
{
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsTail())
		{
			DpTail* tail = (DpTail*)(*it);
			tail->Strike(hipHt);
			break;	// There is only one tail.
		}
}

// which 0=left, 1=right, 2=both
void DpBody::SwimArms(int which, bool bendFirstArm)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = -15.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle = 85.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
			{
				arm->Swim(CurrentGeneralPose(), bendFirstArm);
				bendFirstArm = false;
			}
		}
}

// which 0=back, 1=front, 2=both
void DpBody::SwimLegs(int which)
{
	// Most critters carry their legs close to their bodies, with their feet
	// under them. If they won't work in this position, SpreadLegs will evenly
	// distribute them like a spider or crab, ignoring the angle we pass here.
	float hipSplay = 3.f*DEG2RADf;	// Rotation in XY plane, i.e. critter left/right
	SpreadLegs(hipSplay, which);	// Move knees away from body, rotating each
						// leg at the hips so they are not on top of each other.

	// Position legs.
	// Legs are always the last body parts. Iterate from the end and stop when
	// we get to the first non-leg.
	for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin(); it--)
		if((*it)->IsLegAndFoot())
		{
			DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
			if(which == 2 ||
					(which == 0 && !leg->IsFront()) ||
					(which == 1 && leg->IsFront()))
				leg->Swim(CurrentGeneralPose());
		}
		else
			break;	// Stop at first non-leg body part.
}

void DpBody::SwimTail()
{
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsTail())
		{
			DpTail* tail = (DpTail*)(*it);
			tail->Swim(CurrentGeneralPose());
			break;	// There is only one tail.
		}
}

// Throw with arms at enemy.
void DpBody::ThrowArms(int which)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = -15.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle = 5.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
				arm->Throw(m_HeadTilt);
		}
}

// which 0=left, 1=right, 2=both
void DpBody::TreadWaterArms(int which, bool bendFirstArm)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = -20.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle = 80.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
			{
				arm->TreadWater(bendFirstArm);
				bendFirstArm = false;
			}
		}
}

// which 0=back, 1=front, 2=both
void DpBody::TreadWaterLegs(float hipHt, int which)
{
	// Most critters carry their legs close to their bodies, with their feet
	// under them. If they won't work in this position, SpreadLegs will evenly
	// distribute them like a spider or crab, ignoring the angle we pass here.
	float hipSplay = 10.f*DEG2RADf;	// Rotation in XY plane, i.e. critter left/right
	SpreadLegs(hipSplay, which);	// Move knees away from body, rotating each
						// leg at the hips so they are not on top of each other.

	// Position legs.
	// Legs are always the last body parts. Iterate from the end and stop when
	// we get to the first non-leg.
	for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin(); it--)
		if((*it)->IsLegAndFoot())
		{
			DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
			if(which == 2 ||
					(which == 0 && !leg->IsFront()) ||
					(which == 1 && leg->IsFront()))
				leg->TreadWater(hipHt);
		}
		else
			break;	// Stop at first non-leg body part.
}

void DpBody::WagTail(float hipHt)
{
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsTail())
		{
			DpTail* tail = (DpTail*)(*it);
			tail->Wag(hipHt);
			break;	// There is only one tail.
		}
}

void DpBody::WaveTail(float hipHt, float speedEnc)
{
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsTail())
		{
			DpTail* tail = (DpTail*)(*it);
			tail->Wave(hipHt, speedEnc);
			break;	// There is only one tail.
		}
}

// which 0=left, 1=right, 2=both
void DpBody::WalkArms(float hipHt, int which, bool bendFirstArm, float speedEnc)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = 10.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle;
	if(m_DNA.WalksOnHands())
		liftangle = 2.f*DEG2RADf;
	else
		liftangle = 10.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
			{
				arm->Walk(hipHt, bendFirstArm, speedEnc);
				bendFirstArm = false;
			}
		}
}

// which 0=left, 1=right, 2=both
void DpBody::ProneArms(int which, bool bendFirstArm)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = 10.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle = 80.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
			{
				arm->Fold(bendFirstArm);
				bendFirstArm = false;
			}
		}
}

// which 0=left, 1=right, 2=both
void DpBody::ProneCreepArms(float hipHt, int which, bool bendFirstArm, float speedEnc)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = 10.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle = 80.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
			{
				arm->ProneCreep(hipHt, bendFirstArm, speedEnc);
				bendFirstArm = false;
			}
		}
}

// which 0=back, 1=front, 2=both
void DpBody::ProneCreepLegs(int which, float speedEnc)
{
	// Most critters carry their legs close to their bodies, with their feet
	// under them. If they won't work in this position, SpreadLegs will evenly
	// distribute them like a spider or crab, ignoring the angle we pass here.
	float hipSplay = -5.f*DEG2RADf;	// Rotation in XY plane, i.e. critter left/right
	SpreadLegs(hipSplay, which);	// Move knees away from body, rotating each
						// leg at the hips so they are not on top of each other.

	// Position legs.
	// Legs are always the last body parts. Iterate from the end and stop when
	// we get to the first non-leg.
	for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin(); it--)
		if((*it)->IsLegAndFoot())
		{
			DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
			if(which == 2 ||
					(which == 0 && !leg->IsFront()) ||
					(which == 1 && leg->IsFront()))
				leg->ProneCreep(speedEnc);
		}
		else
			break;	// Stop at first non-leg body part.
}

// which 0=back, 1=front, 2=both
void DpBody::ProneLegs(int which)
{
	// Most critters carry their legs close to their bodies, with their feet
	// under them. If they won't work in this position, SpreadLegs will evenly
	// distribute them like a spider or crab, ignoring the angle we pass here.
	float hipSplay = -1.f*DEG2RADf;	// Rotation in XY plane, i.e. critter left/right
	SpreadLegs(hipSplay, which);	// Move knees away from body, rotating each
						// leg at the hips so they are not on top of each other.

	// Position legs.
	// Legs are always the last body parts. Iterate from the end and stop when
	// we get to the first non-leg.
	for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin(); it--)
		if((*it)->IsLegAndFoot())
		{
			DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
			if(which == 2 ||
					(which == 0 && !leg->IsFront()) ||
					(which == 1 && leg->IsFront()))
				leg->Prone();
		}
		else
			break;	// Stop at first non-leg body part.
}

// which 0=back, 1=front, 2=both
void DpBody::WalkLegs(float hipHt, int which, float speedEnc)
{
	// Most critters carry their legs close to their bodies, with their feet
	// under them. If they won't work in this position, SpreadLegs will evenly
	// distribute them like a spider or crab, ignoring the angle we pass here.
	float hipSplay = 5.f*DEG2RADf;	// Rotation in XY plane, i.e. critter left/right
	SpreadLegs(hipSplay, which);	// Move knees away from body, rotating each
						// leg at the hips so they are not on top of each other.

	// Position legs.
	// Legs are always the last body parts. Iterate from the end and stop when
	// we get to the first non-leg.
	for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin(); it--)
		if((*it)->IsLegAndFoot())
		{
			DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
			if(which == 2 ||
					(which == 0 && !leg->IsFront()) ||
					(which == 1 && leg->IsFront()))
				leg->Walk(hipHt, speedEnc);
		}
		else
			break;	// Stop at first non-leg body part.
}

// which 0=left, 1=right, 2=both
void DpBody::WaveArms(int which)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = -85.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle = -15.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
				arm->Wave(m_HeadTilt);
		}
}

void DpBody::ReceptiveArmsFacingUp(int which)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = 25.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle = 0;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
				arm->ReceptiveFacingUp();
		}
}

// which 0=back, 1=front, 2=both
void DpBody::ReceptiveLegsFacingUp(int which)
{
	// Most critters carry their legs close to their bodies, with their feet
	// under them. If they won't work in this position, SpreadLegs will evenly
	// distribute them like a spider or crab, ignoring the angle we pass here.
	float hipSplay = -8.f * DEG2RADf;	// Rotation in XY plane, i.e. critter left/right
	SpreadLegs(hipSplay, which);	// Move knees away from body, rotating each
						// leg at the hips so they are not on top of each other.

	// Position legs.
	// Legs are always the last body parts. Iterate from the end and stop when
	// we get to the first non-leg.
	for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin(); it--)
		if((*it)->IsLegAndFoot())
		{
			DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
			if(which == 2 ||
					(which == 0 && !leg->IsFront()) ||
					(which == 1 && leg->IsFront()))
				leg->ReceptiveFacingUp();
		}
		else
			break;	// Stop at first non-leg body part.
}

void DpBody::ReceptiveTailFacingUp()
{
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsTail())
		{
			DpTail* tail = (DpTail*)(*it);
			tail->ReceptiveFacingUp();
			break;	// There is only one tail.
		}
}

// which 0=left, 1=right, 2=both
void DpBody::WritheArmsFacingDown(int which)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = 25.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle = 0;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
				arm->WritheFacingDown();
		}
}

// which 0=left, 1=right, 2=both
void DpBody::WritheArmsFacingLeft(int which)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = 0;	// Rotation in XY plane.
	float liftangle = 35.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
				arm->WritheFacingLeft();
		}
}

// which 0=left, 1=right, 2=both
void DpBody::WritheArmsFacingRight(int which)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = 0;	// Rotation in XY plane.
	float liftangle = 35.f*DEG2RADf;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
				arm->WritheFacingRight();
		}
}

void DpBody::WritheArmsFacingUp(int which)
{
	// Fan elbows away from body. Multiple arms get incrementally more angle.
	float shouldersplay = 25.f*DEG2RADf;	// Rotation in XY plane.
	float liftangle = 0;
	SpreadArms(shouldersplay, liftangle, which);

	// Position arms.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(arm->IsWing())
				continue;
			if(which == 2 ||
					(which == 0 && arm->IsLeft()) ||
					(which == 1 && !arm->IsLeft()))
				arm->WritheFacingUp();
		}
}

// which 0=back, 1=front, 2=both
void DpBody::WritheLegsFacingDown(int which)
{
	// Most critters carry their legs close to their bodies, with their feet
	// under them. If they won't work in this position, SpreadLegs will evenly
	// distribute them like a spider or crab, ignoring the angle we pass here.
	float hipSplay = -1.f*DEG2RADf;	// Rotation in XY plane, i.e. critter left/right
	SpreadLegs(hipSplay, which);	// Move knees away from body, rotating each
						// leg at the hips so they are not on top of each other.

	// Position legs.
	// Legs are always the last body parts. Iterate from the end and stop when
	// we get to the first non-leg.
	for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin(); it--)
		if((*it)->IsLegAndFoot())
		{
			DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
			if(which == 2 ||
					(which == 0 && !leg->IsFront()) ||
					(which == 1 && leg->IsFront()))
				leg->WritheFacingDown();
		}
		else
			break;	// Stop at first non-leg body part.
}

// which 0=back, 1=front, 2=both
void DpBody::WritheLegsFacingLeft(int which)
{
	// Most critters carry their legs close to their bodies, with their feet
	// under them. If they won't work in this position, SpreadLegs will evenly
	// distribute them like a spider or crab, ignoring the angle we pass here.
	float hipSplay = -2.f*DEG2RADf;	// Rotation in XY plane, i.e. critter left/right
	SpreadLegs(hipSplay, which);	// Move knees away from body, rotating each
						// leg at the hips so they are not on top of each other.

	// Position legs.
	// Legs are always the last body parts. Iterate from the end and stop when
	// we get to the first non-leg.
	for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin(); it--)
		if((*it)->IsLegAndFoot())
		{
			DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
			if(which == 2 ||
					(which == 0 && !leg->IsFront()) ||
					(which == 1 && leg->IsFront()))
				leg->WritheFacingLeft();
		}
		else
			break;	// Stop at first non-leg body part.
}

// which 0=back, 1=front, 2=both
void DpBody::WritheLegsFacingRight(int which)
{
	// Most critters carry their legs close to their bodies, with their feet
	// under them. If they won't work in this position, SpreadLegs will evenly
	// distribute them like a spider or crab, ignoring the angle we pass here.
	float hipSplay = -2.f*DEG2RADf;	// Rotation in XY plane, i.e. critter left/right
	SpreadLegs(hipSplay, which);	// Move knees away from body, rotating each
						// leg at the hips so they are not on top of each other.

	// Position legs.
	// Legs are always the last body parts. Iterate from the end and stop when
	// we get to the first non-leg.
	for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin(); it--)
		if((*it)->IsLegAndFoot())
		{
			DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
			if(which == 2 ||
					(which == 0 && !leg->IsFront()) ||
					(which == 1 && leg->IsFront()))
				leg->WritheFacingRight();
		}
		else
			break;	// Stop at first non-leg body part.
}

// which 0=back, 1=front, 2=both
void DpBody::WritheLegsFacingUp(int which)
{
	// Most critters carry their legs close to their bodies, with their feet
	// under them. If they won't work in this position, SpreadLegs will evenly
	// distribute them like a spider or crab, ignoring the angle we pass here.
	float hipSplay = -1.f*DEG2RADf;	// Rotation in XY plane, i.e. critter left/right
	SpreadLegs(hipSplay, which);	// Move knees away from body, rotating each
						// leg at the hips so they are not on top of each other.

	// Position legs.
	// Legs are always the last body parts. Iterate from the end and stop when
	// we get to the first non-leg.
	for(QVector<DpBodyPart*>::iterator it = end() - 1; it != begin(); it--)
		if((*it)->IsLegAndFoot())
		{
			DpLegAndFoot* leg = (DpLegAndFoot*)(*it);
			if(which == 2 ||
					(which == 0 && !leg->IsFront()) ||
					(which == 1 && leg->IsFront()))
				leg->WritheFacingUp();
		}
		else
			break;	// Stop at first non-leg body part.
}

void DpBody::WritheTailFacingDown()
{
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsTail())
		{
			DpTail* tail = (DpTail*)(*it);
			tail->WritheFacingDown();
			break;	// There is only one tail.
		}
}

void DpBody::WritheTailFacingLeft()
{
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsTail())
		{
			DpTail* tail = (DpTail*)(*it);
			tail->WritheFacingLeft();
			break;	// There is only one tail.
		}
}

void DpBody::WritheTailFacingRight()
{
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsTail())
		{
			DpTail* tail = (DpTail*)(*it);
			tail->WritheFacingRight();
			break;	// There is only one tail.
		}
}

void DpBody::WritheTailFacingUp()
{
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsTail())
		{
			DpTail* tail = (DpTail*)(*it);
			tail->WritheFacingUp();
			break;	// There is only one tail.
		}
}

void DpBody::WritheWingsFacingDown(float waistRot)
{
	// Position wings for writhing.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(!arm->IsWing())
				continue;
			arm->WritheWingFacingDown();

			float rotwing;
			if(m_DNA.TorsoVertebrae)
				rotwing = waistRot - PIf/2.0;
			else
				rotwing = -PIf/2.;	// No vertebrae so rotate 90 degrees
			arm->RotateShoulder(rotwing);
			if(arm->IsWingLobed())
				arm->RotateElbow(rotwing);	// Rotate lower wing lobe also.
		}
}

void DpBody::WritheWingsFacingLeft(float waistRot)
{
	// Position wings for writhing.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(!arm->IsWing())
				continue;
			arm->WritheWingFacingLeft();

			float rotwing;
			if(m_DNA.TorsoVertebrae)
				rotwing = waistRot - PIf/2.0;
			else
				rotwing = -PIf/2.;	// No vertebrae so rotate 90 degrees
			arm->RotateShoulder(rotwing);
			if(arm->IsWingLobed())
				arm->RotateElbow(rotwing);	// Rotate lower wing lobe also.
		}
}

void DpBody::WritheWingsFacingRight(float waistRot)
{
	// Position wings for writhing.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(!arm->IsWing())
				continue;
			arm->WritheWingFacingRight();

			float rotwing;
			if(m_DNA.TorsoVertebrae)
				rotwing = waistRot - PIf/2.0;
			else
				rotwing = -PIf/2.;	// No vertebrae so rotate 90 degrees
			arm->RotateShoulder(rotwing);
			if(arm->IsWingLobed())
				arm->RotateElbow(rotwing);	// Rotate lower wing lobe also.
		}
}

void DpBody::WritheWingsFacingUp(float waistRot)
{
	// Position wings for writhing.
	for(QVector<DpBodyPart*>::iterator it = begin() + 1; it != end(); it++)
		if((*it)->IsArmAndHand())
		{
			DpArmAndHand* arm = (DpArmAndHand*)(*it);
			if(!arm->IsWing())
				continue;
			arm->WritheWingFacingUp();

			float rotwing;
			if(m_DNA.TorsoVertebrae)
				rotwing = waistRot - PIf/2.0;
			else
				rotwing = -PIf/2.;	// No vertebrae so rotate 90 degrees
			arm->RotateShoulder(rotwing);
			if(arm->IsWingLobed())
				arm->RotateElbow(rotwing);	// Rotate lower wing lobe also.
		}
}

void DpBody::RenderSightRay(AuGLEffects& re)
{
	Q_UNUSED(re);
	/*
	CGprogram vp = re.SelectVSimple();
	re.SetModelViewProjection(vp);
	re.SelectFPassthrough();

	AuPoint seg0 = HeadLocation();
	AuPoint seg1 = seg0 + AuPoint(0,1000,0).RotateX(CurrentHeadTilt());

	// Draw lines.
	re.glBegin(GL_LINES);
		re.glColor4f(1,0,0,1);
		re.glVertex3d(seg0.x, seg0.y, seg0.z);
		re.glVertex3d(seg1.x, seg1.y, seg1.z);
	re.glEnd();

	re.DeselectCg();
	*/
}

void DpBody::RenderHitBox(AuGLEffects& re)
{
	Q_UNUSED(re);
	/*
	CGprogram vp = re.SelectVSimple();
	re.SetModelViewProjection(vp);
	re.SelectFPassthrough();

	if(CurrentGeneralPose().HeadOnly())
	{
		// Only consider the first head.
		for(QVector<DpBodyPart*>::const_iterator it = constBegin()+1; it != constEnd(); it++)
			if((*it)->IsHeadAndNeck())
			{
				(*it)->RenderHitBoxHeadOnly(re);
				break;
			}
	}
	else
	{
		for(QVector<DpBodyPart*>::const_iterator it = constBegin(); it != constEnd(); it++)
			(*it)->RenderHitBox(re);
	}

	re.DeselectCg();
	*/
}

void DpBody::RoughExtents(AuPoint& min, AuPoint& max) const
{
	if(!isEmpty())
	{
		// Get an array of the bone joint locations.
		AuPointArray bodyLocs;
		if(CurrentGeneralPose().HeadOnly())
		{
			// Only consider the first head.
			for(QVector<DpBodyPart*>::const_iterator it = constBegin()+1; it != constEnd(); it++)
				if((*it)->IsHeadAndNeck())
				{
					(*it)->RoughExtentLocs(bodyLocs);
					break;
				}
		}
		else
		{
			for(QVector<DpBodyPart*>::const_iterator it = constBegin(); it != constEnd(); it++)
				(*it)->RoughExtentLocs(bodyLocs);
		}
		bodyLocs.Extents(min, max);

		// Body part rough extent locs just give me the bone locations.
		// Puff up the extents a little if this thing is too small, otherwise you can't target|shoot it.
		if(max.x - min.x < 1)
		{
			max.x += .5;
			min.x -= .5;
		}
		if(max.y - min.y < 1)
		{
			max.y += .5;
			min.y -= .5;
		}
		if(max.z - min.z < 1)
			max.z += 1;											// Only add height so it doesn't hang down through the floor.
	}
}

bool DpBody::SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const
{
	bool hitMe = false;

	// This loop will trim the target segment to the closest hit to the eye.
	if(CurrentGeneralPose().HeadOnly())
	{
		// Only target the first head.
		for(QVector<DpBodyPart*>::const_iterator it = constBegin()+1; it != constEnd(); it++)
			if((*it)->IsHeadAndNeck())
			{
				if((*it)->SightTargetDetailsHead(eyeLoc, sightedLoc, sightedPart))
					hitMe = true;	// Hit at least one body part.
				break;
			}
	}
	else
	{
		for(QVector<DpBodyPart*>::const_iterator it = constBegin(); it != constEnd(); it++)
			if((*it)->SightTargetDetails(eyeLoc, sightedLoc, sightedPart))
				hitMe = true;	// Hit at least one body part.
	}

	return hitMe;
}

void DpBody::RoughExtentChunk(AuPointArray& locs, DhDNA::BONES bone, const AuPoint& loc, float len) const
{
	AuPoint seg0 = m_Bones[bone].Project(loc);
	AuPoint seg1 = m_Bones[bone].Project(loc + AuPoint(0,len,0));
	locs.append(seg0);
	locs.append(seg1);
}

void DpBody::RenderHitBoxChunk(AuGLEffects& re, DhDNA::BONES bone, const AuPoint& loc, float len, float avgrad) const
{
	Q_UNUSED(re);
	Q_UNUSED(bone);
	Q_UNUSED(loc);
	Q_UNUSED(len);
	Q_UNUSED(avgrad);
	/*DHH glsl
	AuPoint seg0 = m_Bones[bone].Project(loc);
	AuPoint seg1 = m_Bones[bone].Project(loc + AuPoint(0,len,0));
	AuPoint vx = m_Bones[bone].ProjectVector(AuPoint(avgrad,0));
	AuPoint vy = m_Bones[bone].ProjectVector(AuPoint(0,0,avgrad));

	// Calculate corner points of flesh chunk.
	AuPoint ll0 = seg0 - vx - vy;
	AuPoint ul0 = seg0 - vx + vy;
	AuPoint ur0 = seg0 + vx + vy;
	AuPoint lr0 = seg0 + vx - vy;

	AuPoint ll1 = seg1 - vx - vy;
	AuPoint ul1 = seg1 - vx + vy;
	AuPoint ur1 = seg1 + vx + vy;
	AuPoint lr1 = seg1 + vx - vy;

	// Draw lines.
	re.glBegin(GL_LINES);
		// Bone is blue
		re.glColor4f(0,0,1,1);
		re.glVertex3d(seg0.x, seg0.y, seg0.z);
		re.glVertex3d(seg1.x, seg1.y, seg1.z);

		// Three edge lines that won't be drawn in the line strip.
		if(bone == DhDNA::BONE_Head0 || bone == DhDNA::BONE_Head1)
			re.glColor4f(1,0,0,1);				// Red head(s)
		else
			re.glColor4f(1,.5,.5,1);			// Pink body
		re.glVertex3d(ul0.x, ul0.y, ul0.z);
		re.glVertex3d(ul1.x, ul1.y, ul1.z);
		re.glVertex3d(ur0.x, ur0.y, ur0.z);
		re.glVertex3d(ur1.x, ur1.y, ur1.z);
		re.glVertex3d(lr0.x, lr0.y, lr0.z);
		re.glVertex3d(lr1.x, lr1.y, lr1.z);
	re.glEnd();

	re.glBegin(GL_LINE_STRIP);
		if(bone == DhDNA::BONE_Head0 || bone == DhDNA::BONE_Head1)
			re.glColor4f(1,0,0,1);				// Red head(s)
		else
			re.glColor4f(1,.5,.5,1);			// Pink body

		// Frame end 0.
		re.glVertex3d(ll0.x, ll0.y, ll0.z);
		re.glVertex3d(ul0.x, ul0.y, ul0.z);
		re.glVertex3d(ur0.x, ur0.y, ur0.z);
		re.glVertex3d(lr0.x, lr0.y, lr0.z);
		re.glVertex3d(ll0.x, ll0.y, ll0.z);

		// Edge line up to end 1 and frame end 1.
		re.glVertex3d(ll1.x, ll1.y, ll1.z);
		re.glVertex3d(ul1.x, ul1.y, ul1.z);
		re.glVertex3d(ur1.x, ur1.y, ur1.z);
		re.glVertex3d(lr1.x, lr1.y, lr1.z);
		re.glVertex3d(ll1.x, ll1.y, ll1.z);
	re.glEnd();
	*/
}

bool DpBody::SightChunk(DhDNA::BONES bone, const AuPoint& start, const AuPoint& end, float avgrad,
		const AuPoint& eyeLoc, AuPoint& sightedLoc) const
{
	AuPoint seg0 = m_Bones[bone].Project(start);
	AuPoint seg1 = m_Bones[bone].Project(end);
	AuPoint pc0, pc1;
	if(eyeLoc.PerpConn(sightedLoc, seg0, seg1, pc0, pc1))
	{
		/*
		qDebug() << "DpBody::SightChunk" << DhDNA::BoneName(bone) << "PerpConn'd" << pc0.Distance(pc1)
			<< pc0.Between(eyeLoc, sightedLoc)
			<< pc0.DistToSegment(eyeLoc, sightedLoc)
			<< pc0.DistToSegment(seg0, seg1)
			<< pc1.Between(seg0, seg1)
			<< pc1.DistToSegment(eyeLoc, sightedLoc)
			<< pc1.DistToSegment(seg0, seg1)
			<< eyeLoc << sightedLoc << seg0 << seg1;
		*/
		if(pc0.Between(eyeLoc, sightedLoc) && pc1.Between(seg0, seg1))
		{
			float missdist = pc0.Distance(pc1);
			//qDebug() << "DpBody::SightChunk" << DhDNA::BoneName(bone) << "miss distance" << missdist;
			if(missdist <= avgrad)
			{
				// This is close enough to call it a hit.
				//qDebug() << "DpBody::SightChunk" << DhDNA::BoneName(bone) << "hit";
				sightedLoc = pc0;
				return true;
			}
		}
	}
	return false;
}

// Set parameters about the personal weapon used by this body, whether it be a
// special weapon such as psionics or a physical weapon such as claws or teeth.
// Weapons that cannot reach 'targetRange' are ignored so the best weapon for the
// given range is returned. If the targeRange is 0, all weapons are available, with a
// preference to a special weapon.
// 'targetRange' is distance to the target from my body pivot point.
// 'range' returned is range of body weapon relative to my body pivot point.
// Returns 'true' if the special weapon is selected.
bool DpBody::BestWeaponAtRange(float targetRange, AuDNABase::WEAPONLOC& loc, AuCombatWeapon& weapon,
		bool okToUseSpecialWeapon) const
{
	// Check to see if I can use my special weapon.
	if(okToUseSpecialWeapon && SpecialWeaponReady())
		loc = m_DNA.WeaponLocation();	// Special weapon location, if any.
	else
		loc = DhDNA::WL_None;			// Ignore special weapon until its ready to use.
	if(loc != DhDNA::WL_None)
	{
		// Determine range that I can strike with my special weapon.
		WeaponAtPos(loc, true, weapon);
		if(targetRange <= weapon.m_Range)
			return true;
	}

	// Determine preferred physical body weapon.
	if(m_DNA.HandLength/8 && m_DNA.HasArms())
		loc = DhDNA::WL_Hands;
	else if(m_DNA.FootLength/8 && m_DNA.LegPairs)
		loc = DhDNA::WL_Feet;
	else if(m_DNA.TailVertebrae > 1)
		loc = DhDNA::WL_Tail;				// flexible tail
	else
		loc = DhDNA::WL_Head;	// Only physical attack option; eat it.

	// We might choose to bite or butt if we can't reach with our hands or feet.
	if(m_DNA.MouthSize > 1 || m_DNA.HornSize > 1)
	{
		// Large enough mouth or horns to be significant.
		if(loc == DhDNA::WL_Hands)
		{
			// Determine range to strike with physical weapon, i.e. hands, feet, or head.
			float physAttackRange = UpperArmLength() + ForeArmLength();
			if(targetRange > physAttackRange)
			{
				// Target is out of hand range so use head if its range is greater.
				if(BiteRange() > physAttackRange)
					loc = DhDNA::WL_Head;
			}
		}
		else if(loc == DhDNA::WL_Feet)
		{
			// Determine range to strike with physical weapon, i.e. hands, feet, or head.
			float physAttackRange = ThighLength() + CalfLength();
			if(targetRange > physAttackRange)
			{
				// Target is out of foot range so use head if its range is greater.
				if(BiteRange() > physAttackRange)
					loc = DhDNA::WL_Head;
			}
		}
	}

	WeaponAtPos(loc, false, weapon);
	if(targetRange > weapon.m_Range)
	{
		// Target is beyond physical attack range.
		weapon.m_w1.m_Effect = WE_None;
		weapon.m_w2.m_Effect = WE_None;
	}
	return false;
}

void DpBody::WeaponAtPos(AuDNABase::WEAPONLOC loc, bool useSpecialWeapon, AuCombatWeapon& weapon) const
{
	if(loc == DhDNA::WL_None)
	{
		// Return null values.
		weapon = AuCombatWeapon(WM_Not);
	}
	else
	{
		// No body weapons currently have armor piercing or strike through properties.
		weapon.m_w1.m_AP = 0;
		weapon.m_w1.m_ST = 0;
		weapon.m_w2.m_AP = 0;
		weapon.m_w2.m_ST = 0;

		// Calculate a size factor for weapons based on the size of the creature, not its DNA OverallSize, which doesn't reflect
		// its final size. Human max hit points = 50. Human OverallSize = 8.
		if(useSpecialWeapon)
		{
			// Special Weapon
			float wpnSizeFactor = MaxHitPoints() / 50.f;	// Special wpnSizeFactor = 1 for default human.
			// Verify that the location is correct when selecting the special weapon, which only operates from that location.
			if(loc == m_DNA.WeaponLocation())
			{
				// That is my special weapon location.
				// Default size of weapon 1 is based on size of delivery organ, may change.
				float wpnSize1 = 0;
				switch(loc)
				{
				case DhDNA::WL_None:
					break;
				case DhDNA::WL_Anus:
				case DhDNA::WL_Torso:
					if(m_DNA.TorsoThickness > m_DNA.TorsoSegmentLength)
						wpnSize1 = m_DNA.TorsoThickness;
					else
						wpnSize1 = m_DNA.TorsoSegmentLength;
					break;
				case DhDNA::WL_Eyes:
					wpnSize1 = m_DNA.EyeSize*8;
					break;
				case DhDNA::WL_Head:
					if(m_DNA.MouthSize > m_DNA.HornSize)
						wpnSize1 = m_DNA.MouthSize*8;
					else
						wpnSize1 = m_DNA.HornSize*8;
					break;
				case DhDNA::WL_Feet:
					wpnSize1 = m_DNA.FootLength;
					break;
				case DhDNA::WL_Hands:
					wpnSize1 = m_DNA.HandLength;
					break;
				case DhDNA::WL_Tail:
					wpnSize1 = m_DNA.TailVertebrae*8;
					break;
				}

				// Default size of weapon 2 is based on size of special weapon.
				float wpnSize2 = m_DNA.WeaponSize*4;
				weapon.m_w1.m_Radius = 0;
				weapon.m_w2.m_Radius = 0;
				switch(m_DNA.WeaponMode)
				{
				case DhDNA::DWM_Projectile:
					// Projectile does piercing damage in addition to special damage.
					weapon.m_Mode = WM_Projectile;
					weapon.m_Range = m_DNA.SightRange()/3.f;
					weapon.m_w1.m_Effect = WE_Pierce;
					weapon.m_w2.m_Effect = (WEAPONEFFECT)m_DNA.WeaponType;
					break;
				case DhDNA::DWM_Psionic:
					// Psionic weapon only does special weapon damage.
					weapon.m_Mode = WM_Psionic;
					weapon.m_Range = m_DNA.SightRange()/4.f;
					weapon.m_w1.m_Effect = (WEAPONEFFECT)m_DNA.WeaponType;
					wpnSize1 = wpnSize2;
					weapon.m_w2.m_Effect = WE_None;
					break;
				case DhDNA::DWM_Contact:
					weapon.m_Mode = WM_Contact;
					switch(loc)
					{
					case DhDNA::WL_Head:
						// Head weapon does piercing damage in addition to special damage.
						weapon.m_Range = BiteRange();
						weapon.m_w1.m_Effect = WE_Pierce;
						weapon.m_w2.m_Effect = (WEAPONEFFECT)m_DNA.WeaponType;
						break;
					case DhDNA::WL_Eyes:
						// Eye weapon only does special weapon damage.
						weapon.m_Range = BiteRange();
						weapon.m_w1.m_Effect = (WEAPONEFFECT)m_DNA.WeaponType;
						wpnSize1 = wpnSize2;
						weapon.m_w2.m_Effect = WE_None;
						break;
					case DhDNA::WL_Hands:
						weapon.m_Range = UpperArmLength() + ForeArmLength();
						if(m_DNA.HandType == DhDNA::HAND_Claw)
						{
							// Claw does slashing damage in addition to special damage.
							weapon.m_w1.m_Effect = WE_Slash;
							weapon.m_w2.m_Effect = (WEAPONEFFECT)m_DNA.WeaponType;
						}
						else if(m_DNA.HandType == DhDNA::HAND_Pincher)
						{
							// Pincher does piercing damage in addition to special damage.
							weapon.m_w1.m_Effect = WE_Pierce;
							weapon.m_w2.m_Effect = (WEAPONEFFECT)m_DNA.WeaponType;
						}
						else
						{
							// Other hand type does bludgeon damage in addition to
							// special damage.
							weapon.m_w1.m_Effect = WE_Bludgeon;
							weapon.m_w2.m_Effect = (WEAPONEFFECT)m_DNA.WeaponType;
						}
						break;
					case DhDNA::WL_Feet:
						weapon.m_Range = ThighLength() + CalfLength();
						if(m_DNA.FootType == DhDNA::FOOT_Claw)
						{
							// Claw does slashing damage in addition to special damage.
							weapon.m_w1.m_Effect = WE_Slash;
							weapon.m_w2.m_Effect = (WEAPONEFFECT)m_DNA.WeaponType;
						}
						else if(m_DNA.FootType == DhDNA::FOOT_Pincher)
						{
							// Pincher does piercing damage in addition to special damage.
							weapon.m_w1.m_Effect = WE_Pierce;
							weapon.m_w2.m_Effect = (WEAPONEFFECT)m_DNA.WeaponType;
						}
						else
						{
							// Other hand type does bludgeon damage in addition to
							// special damage.
							weapon.m_w1.m_Effect = WE_Bludgeon;
							weapon.m_w2.m_Effect = (WEAPONEFFECT)m_DNA.WeaponType;
						}
						break;
					case DhDNA::WL_Tail:
						{
							// Tail with special weapon does pierce damage in addition to
							// special damage.
							float vertlen = TorsoSegmentLength() * 2.f/3.f;	// Tail vert len
							weapon.m_Range = vertlen * (float)m_DNA.TailVertebrae / 2.f;	// 1/2 tail
							weapon.m_w1.m_Effect = WE_Pierce;
							weapon.m_w2.m_Effect = (WEAPONEFFECT)m_DNA.WeaponType;
						}
						break;
					case DhDNA::WL_Anus:
						// Anus weapon only does special weapon damage.
						weapon.m_Range = TorsoThickness()/2.f;
						weapon.m_w1.m_Effect = (WEAPONEFFECT)m_DNA.WeaponType;
						wpnSize1 = wpnSize2;
						weapon.m_w2.m_Effect = WE_None;
						break;
					case DhDNA::WL_Torso:
						// Torso does bludgeon damage in addition to special damage.
						weapon.m_Range = TorsoLength();
						weapon.m_w1.m_Effect = WE_Bludgeon;
						weapon.m_w2.m_Effect = (WEAPONEFFECT)m_DNA.WeaponType;
						break;
					case DhDNA::WL_None:
						// Not really a valid possibility.
						weapon.m_w1.m_Effect = (WEAPONEFFECT)m_DNA.WeaponType;
						wpnSize1 = wpnSize2;
						weapon.m_w2.m_Effect = WE_None;
						break;
					}
					weapon.m_Range += CGHeight(true);	// Add CG height to insure reach to ground.
					weapon.m_Range += 4.f;		// For a reach margin. Helps small creatures.
					break;
				case DhDNA::DWM_Area:
					// Area affected is based on overall size of critter.
					weapon.m_Mode = WM_Area;
					weapon.m_Range = 0;
					weapon.m_w1.m_Radius = wpnSizeFactor * 20.f;
					weapon.m_w1.m_Effect = (WEAPONEFFECT)m_DNA.WeaponType;
					wpnSize1 = wpnSize2;
					weapon.m_w2.m_Effect = WE_None;
					break;
				}

				// Calculate damage based on weapon size.
				weapon.m_w1.m_Damage = (int)((wpnSize1 + 1)*wpnSizeFactor) + 1;
				weapon.m_w2.m_Damage = (int)((wpnSize2 + 1)*wpnSizeFactor) + 1;
				weapon.m_w1.m_DamageMod = (int)wpnSizeFactor;					// Size affects damage.
				weapon.m_w2.m_DamageMod = (int)wpnSizeFactor;
			}
			else
			{
				// No special weapon at that body location.
				// Return null values.
				weapon = AuCombatWeapon(WM_Not);
			}
		}
		else
		{
			// Physical Body Weapon
			float wpnSizeFactor = MaxHitPoints() / 100.f;	// Physical wpnSizeFactor = .5 for default human.
			switch(loc)
			{
			case DhDNA::WL_Head:
				// All physical head weapons seem to produce the same damage effect.
				// We don't head-butt so there's no bludgeoning effect.
				weapon.m_Mode = WM_Contact;
				weapon.m_w1.m_Effect = WE_Pierce;	// horns or mouth
				weapon.m_Range = BiteRange();

				// Use larger of mouth vs horns for base damage.
				if(m_DNA.MouthSize > m_DNA.HornSize)
					weapon.m_w1.m_Damage = (int)((m_DNA.MouthSize*8+1)*wpnSizeFactor) + 1;
				else
					weapon.m_w1.m_Damage = (int)((m_DNA.HornSize*8+1)*wpnSizeFactor) + 1;
				break;
			case DhDNA::WL_Hands:
				// Hand type determines damage effect.
				if(m_DNA.HasArms())
				{
					weapon.m_Mode = WM_Contact;
					if(m_DNA.HandType == DhDNA::HAND_Claw || m_DNA.HandType == DhDNA::HAND_Pincher)
						weapon.m_w1.m_Effect = WE_Slash;
					else
						weapon.m_w1.m_Effect = WE_Bludgeon;
					weapon.m_Range = UpperArmLength() + ForeArmLength();

					// Hand size determines base damage.
					weapon.m_w1.m_Damage = (int)((m_DNA.HandLength+1)*wpnSizeFactor) + 1;
				}
				else
				{
					weapon.m_Mode = WM_Not;
					weapon.m_w1.m_Effect = WE_None;
					weapon.m_Range = 0;
					weapon.m_w1.m_Damage = 0;
				}
				break;
			case DhDNA::WL_Feet:
				// Foot type determines damage effect.
				if(m_DNA.LegPairs)
				{
					weapon.m_Mode = WM_Contact;
					if(m_DNA.FootType == DhDNA::FOOT_Claw || m_DNA.FootType == DhDNA::FOOT_Pincher)
						weapon.m_w1.m_Effect = WE_Slash;
					else
						weapon.m_w1.m_Effect = WE_Bludgeon;
					weapon.m_Range = ThighLength() + CalfLength();

					// Foot size determines base damage.
					weapon.m_w1.m_Damage = (int)((m_DNA.FootLength+1)*wpnSizeFactor) + 1;
				}
				else
				{
					weapon.m_Mode = WM_Not;
					weapon.m_w1.m_Effect = WE_None;
					weapon.m_Range = 0;
					weapon.m_w1.m_Damage = 0;
				}
				break;
			case DhDNA::WL_Tail:
				if(m_DNA.TailVertebrae > 1)
				{
					weapon.m_Mode = WM_Contact;
					weapon.m_w1.m_Effect = WE_Bludgeon;
					float vertlen = TorsoSegmentLength() * 2.f/3.f;	// Tail vert len
					weapon.m_Range = vertlen * (float)m_DNA.TailVertebrae / 2.f;	// 1/2 tail len

					// Tail length determines base damage.
					weapon.m_w1.m_Damage = (int)((m_DNA.TailVertebrae*8+1)*wpnSizeFactor) + 1;
				}
				else
				{
					weapon.m_Mode = WM_Not;
					weapon.m_w1.m_Effect = WE_None;
					weapon.m_Range = 0;
					weapon.m_w1.m_Damage = 0;
				}
				break;
			case DhDNA::WL_Anus:
			case DhDNA::WL_Eyes:
			case DhDNA::WL_Torso:
				// Invalid physical weapon location.
			case DhDNA::WL_None:
				weapon.m_Mode = WM_Not;
				weapon.m_w1.m_Effect = WE_None;
				weapon.m_w1.m_Damage = 0;
				weapon.m_Range = 0;
				break;
			}
			if(weapon.m_Range > 0)
			{
				weapon.m_Range += CGHeight(true);	// Add CG height to insure reach to the ground.
				weapon.m_Range += 4.f;				// For a reach margin. Helps small creatures.
			}

			weapon.m_w1.m_DamageMod = (int)wpnSizeFactor;	// Size affects damage.
			weapon.m_w1.m_Radius = 0;					// physical body has no area affect

			weapon.m_w2.m_Effect = WE_None;
			weapon.m_w2.m_Damage = 0;
			weapon.m_w2.m_DamageMod = 0;
			weapon.m_w2.m_Radius = 0;
		}
	}
}

// Distance from loc to food for eating.
float DpBody::BiteRange() const
{
	if(m_BiteRange == 0)
	{
		// Determine range to strike physically with head.
		unsigned int nvc = m_DNA.NeckVertebraeCount();
		float vertlen = TorsoSegmentLength() / 2.5f;
		float neckLength = vertlen * nvc;
		m_BiteRange = TorsoLength() + neckLength + HeadLength();
	}
	return m_BiteRange;
}

// Maximum turn rate.
// Radians per real world second.
float DpBody::TurnRate() const
{
	// Calculate turn rate based on Speed dna.
	float turn = ((float)m_DNA.Speed+1)/6.f * PIf;
	//if(CurrentGeneralPose() == AuPose::Glide)
		//turn /= 2.f;
	//else if(CurrentGeneralPose() == AuPose::Fly)
		//turn /= 2.5f;
	//else if(CurrentGeneralPose() == AuPose::FlyFast)
		//turn /= 3.f;
	return turn;
}

// Approximate volume of body in cubic feet.
float DpBody::Volume() const
{
	if(m_Volume == 0)
	{
		//qDebug() << "DpBody::Volume" << DNA().Description();
		for(QVector<DpBodyPart*>::const_iterator it = constBegin(); it != constEnd(); it++)
		{
			//qDebug() << " " << (*it)->Volume() << "cubic feet" << ((*it)->Volume() * 28.3168466) << "liters";
			m_Volume += (*it)->Volume();
		}
		//qDebug() << "DpBody::Volume total" << m_Volume << "cubic feet" << (m_Volume * 28.3168466) << "liters";
	}
	return m_Volume;
}

void DpBody::ResetCache()
{
	m_Volume = 0;
	m_BiteRange = 0;
}

// Returns true if the eye height is valid for a person.
bool DpBody::IsCameraHeightValidForPerson() const
{
	if(CurrentGeneralPose() != AuPose::ConsoleCommand)
	{
		if(DNA().BodyCarriage == 0 || DNA().BodyCarriage == 1)
			Q_ASSERT(CurrentGeneralPose() == AuPose::Crouch);
		else
			Q_ASSERT(CurrentGeneralPose() == AuPose::Stand);
	}
	float cameraHeight = CGHeight(true) + CameraLocation().z;
	return cameraHeight >= 0.5f && cameraHeight <= 7.5f;
}

// Returns true if the CG height is valid for a person.
bool DpBody::IsCgHeightValidForPerson() const
{
	if(DNA().BodyCarriage == 0 || DNA().BodyCarriage == 1)
		Q_ASSERT(CurrentGeneralPose() == AuPose::Crouch);
	else
		Q_ASSERT(CurrentGeneralPose() == AuPose::Stand);
	float cgHeight = CGHeight(true);
	return cgHeight >= 0.1245f && cgHeight <= 7.0f;
}

// Returns true if the head height is valid for a person.
bool DpBody::IsHeadHeightValidForPerson() const
{
	if(CurrentGeneralPose() != AuPose::ConsoleCommand)
	{
		if(DNA().BodyCarriage == 0 || DNA().BodyCarriage == 1)
			Q_ASSERT(CurrentGeneralPose() == AuPose::Crouch);
		else
			Q_ASSERT(CurrentGeneralPose() == AuPose::Stand);
	}
	float headHeight = CGHeight(true) + HeadLocation().z;
	return headHeight >= 0.5f && headHeight <= 7.0f;
}

bool DpBody::IsAbilityPointsValidForPerson() const
{
	return AbilityPointsUsed() <= 60;
}

int DpBody::AbilityPointsUsed() const
{
	int iPointsUsed = DNA().OverallSize/3;
	iPointsUsed += DNA().Speed;
	iPointsUsed += DNA().Strength;
	iPointsUsed += DNA().Agility;
	iPointsUsed += DNA().Endurance;
	iPointsUsed += DNA().TopSkinThickness;
	iPointsUsed += DNA().BottomSkinThickness;
	if(DNA().WeaponType != DhDNA::WT_None)
	{
		iPointsUsed += 2;				// Having a weapon on the body requires two points
		switch(DNA().WeaponMode)
		{
		case DhDNA::DWM_Area:
			iPointsUsed += DNA().WeaponSize * 4;
			break;
		case DhDNA::DWM_Contact:
			iPointsUsed += DNA().WeaponSize;
			break;
		case DhDNA::DWM_Projectile:
			iPointsUsed += DNA().WeaponSize * 3;
			break;
		case DhDNA::DWM_Psionic:
			iPointsUsed += DNA().WeaponSize * 2;
			break;
		}
	}

	// Physical weapon
	AuDNABase::WEAPONLOC wLoc;
	AuCombatWeapon weapon(WM_Not);
	BestWeaponAtRange(0, wLoc, weapon, false);
	iPointsUsed += weapon.m_w1.m_Damage/2;						// 2 points of physical weapon damage = 1 ability point
	return iPointsUsed;
}

// Returns true if the maximum hit points of this body are valid for a player character.
bool DpBody::IsHitPointsValidForPerson() const
{
	unsigned short int maxHits = MaxHitPoints();
	if(DNA().CanFly())
		return maxHits >= 12 && maxHits <= 40;
	else if(DNA().CanJump())
		return maxHits >= 16 && maxHits <= 53;
	else
		return maxHits >= 25 && maxHits <= 80;
}

unsigned short int DpBody::MaxHitPoints() const
{
	// This formula causes a human-sized creature to have exactly 50 hit points.
	float vol = Volume();
	unsigned short int hp = (unsigned short int)(sqrtf(vol) * 29.565878244f);
	//qDebug("DpBody::MaxHitPoints vol %g hp %d", vol, hp);
	if(hp > 0x7fff)
	{
		qWarning() << "DpBody::MaxHitPoints vol" << vol << "exceeds hit points maximum";
		hp = 0x7fff;
	}

	if(DNA().CanFly())
		hp = hp / 2;					// Fliers have 1/2 the hit points of non-flyers.
	else if(DNA().CanJump())
		hp = hp * 2 / 3;				// Leapers have 2/3 the hit points of non-leapers.

	// The body must have at least 1 hit point.
	if(hp <= 0)
		hp = 1;

	return hp;
}

AuPose DpBody::CurrentBonePose(DhDNA::BONES bone) const
{
	switch(bone)
	{
	case DhDNA::BONE_Max:
	case DhDNA::BONE_Torso0:
	case DhDNA::BONE_Torso1:
	case DhDNA::BONE_Torso2:
	case DhDNA::BONE_Torso3:
	case DhDNA::BONE_Torso4:
	case DhDNA::BONE_Torso5:
	case DhDNA::BONE_Torso6:
		if(bone <= (int)m_DNA.WaistVertex())
			return m_Pose[PS_UpperTorso];
		else
			return m_Pose[PS_LowerTorso];
	case DhDNA::BONE_Head0:
	case DhDNA::BONE_Neck00:
	case DhDNA::BONE_Neck01:
	case DhDNA::BONE_Neck02:
	case DhDNA::BONE_Neck03:
	case DhDNA::BONE_Jaw0:
		return m_Pose[PS_LeftHeadNeck];
	case DhDNA::BONE_Head1:
	case DhDNA::BONE_Neck10:
	case DhDNA::BONE_Neck11:
	case DhDNA::BONE_Neck12:
	case DhDNA::BONE_Neck13:
	case DhDNA::BONE_Jaw1:
		return m_Pose[PS_RightHeadNeck];
	case DhDNA::BONE_Tail0:
	case DhDNA::BONE_Tail1:
	case DhDNA::BONE_Tail2:
	case DhDNA::BONE_Tail3:
	case DhDNA::BONE_Tail4:
	case DhDNA::BONE_Tail5:
		return m_Pose[PS_Tail];
	case DhDNA::BONE_Hand0L:
	case DhDNA::BONE_ForeArm0L:
	case DhDNA::BONE_UpperArm0L:
	case DhDNA::BONE_Hand1L:
	case DhDNA::BONE_ForeArm1L:
	case DhDNA::BONE_UpperArm1L:
	case DhDNA::BONE_Hand2L:
	case DhDNA::BONE_ForeArm2L:
	case DhDNA::BONE_UpperArm2L:
		return m_Pose[PS_LeftArm];
	case DhDNA::BONE_Hand0R:
	case DhDNA::BONE_ForeArm0R:
	case DhDNA::BONE_UpperArm0R:
	case DhDNA::BONE_Hand1R:
	case DhDNA::BONE_ForeArm1R:
	case DhDNA::BONE_UpperArm1R:
	case DhDNA::BONE_Hand2R:
	case DhDNA::BONE_ForeArm2R:
	case DhDNA::BONE_UpperArm2R:
		return m_Pose[PS_RightArm];
	case DhDNA::BONE_Foot0L:
	case DhDNA::BONE_Calf0L:
	case DhDNA::BONE_Thigh0L:
	case DhDNA::BONE_Foot0R:
	case DhDNA::BONE_Calf0R:
	case DhDNA::BONE_Thigh0R:
		return m_Pose[PS_FrontLegs];
	case DhDNA::BONE_Foot1L:
	case DhDNA::BONE_Calf1L:
	case DhDNA::BONE_Thigh1L:
	case DhDNA::BONE_Foot1R:
	case DhDNA::BONE_Calf1R:
	case DhDNA::BONE_Thigh1R:
	case DhDNA::BONE_Foot2L:
	case DhDNA::BONE_Calf2L:
	case DhDNA::BONE_Thigh2L:
	case DhDNA::BONE_Foot2R:
	case DhDNA::BONE_Calf2R:
	case DhDNA::BONE_Thigh2R:
		return m_Pose[PS_RearLegs];
	}
	return m_Pose[PS_LowerTorso];
}

DpBody::TORSO_SEGMENT DpBody::TorsoSegment(unsigned int segmentIndex) const
{
	// When the index is beyond the range of torso segments, the answer is none.
	// When this creature has no torso vertebrae, the answer is none.
	if(segmentIndex >= m_DNA.TorsoVertebrae)
		return TS_None;

	// Determine how many vertices are used by arms and legs.
	unsigned int waist = m_DNA.WaistVertex();
	unsigned int armverts = m_DNA.ArmVertices();
	unsigned int legverts;
	if(m_DNA.LegPairs)
	{
		unsigned int verts = (unsigned int)m_DNA.TorsoVertebrae + 1;	// Use each end of spine segments.
		legverts = verts - armverts;
		if(legverts < 1)
			legverts = 1;				// Need at least one vertex for legs.
		else if(legverts > m_DNA.LegPairs)
			legverts = m_DNA.LegPairs;	// Don't take more vertices than needed.
	}
	else
		legverts = 0;
	if(m_DNA.HasWings() && armverts)
		armverts--;		// One of its arm pairs is a set of wings, which don't go at a torso segment vertex.

	// What is at the head end of the segment.
	// node0 : 0=neck, 1=arms & neck, 2=arms & torso, 3=torso, 4=legs & neck, 5=legs & torso
	unsigned int node0 = 0;
	if(segmentIndex < armverts)
	{
		// There is a pair of arms at the vertex.
		if(segmentIndex == 0)
			node0 = 1;	// arms & neck
		else
			node0 = 2;	// arms & torso
	}
	else if(segmentIndex < waist)
	{
		// There are no arms or legs at the vertex and its not the last vertex in the torso.
		if(segmentIndex == 0)
			node0 = 0;	// neck
		else
			node0 = 3;	// torso
	}
	else if(legverts)
	{
		// This critter has legs.
		if(segmentIndex == 0)
			node0 = 4;	// legs & neck
		else if(segmentIndex == waist)
			node0 = 5;	// legs & torso
		else if(legverts == 3)
		{
			// There is an intermediate leg pair between the waist and the butt.
			unsigned int iv = ((unsigned int)m_DNA.TorsoVertebrae - waist)/2 + waist;
			if(segmentIndex == iv)
				node0 = 5;	// legs & torso
			else
				node0 = 3;	// torso
		}
		else
			node0 = 3;	// torso
	}
	else
	{
		// There are no arms or legs at the vertex.
		if(segmentIndex == 0)
			node0 = 0;	// neck
		else
			node0 = 3;	// torso
	}

	// What is at the tail end of the segment.
	// node1 : 0=butt, 1=arms & tail, 2=legs & butt, 3=arms & torso, 4=torso, 5=legs & tail, 6=legs & torso, 7=tail, 8=arms & butt
	unsigned int node1 = 0;
	unsigned int node1idx = segmentIndex + 1;
	if(node1idx < armverts)
	{
		// There is a pair of arms at the vertex.
		if(node1idx == m_DNA.TorsoVertebrae)
		{
			// This is the last vertex in the torso.
			if(m_DNA.TailVertebrae)
				node1 = 1;	// arms & tail
			else
				node1 = 8;	// arms & butt
		}
		else
			node1 = 3;	// arms & torso
	}
	else if(node1idx < waist)
	{
		// There are no arms or legs at the vertex and its not the last vertex in the torso.
		node1 = 4;	// torso
	}
	else if(legverts)
	{
		// This critter has legs.
		if(node1idx == m_DNA.TorsoVertebrae)
		{
			// This is the last vertex in the torso.
			if(legverts > 1 || node1idx == waist)
			{
				// There are legs at the node.
				if(m_DNA.TailVertebrae)
					node1 = 5;	// legs & tail
				else
					node1 = 2;	// legs & butt
			}
			else
			{
				// There are no legs at the last vertex of the torso.
				if(m_DNA.TailVertebrae)
					node1 = 7;	// tail
				else
					node1 = 0;	// butt
			}
		}
		else if(node1idx == waist)
			node1 = 6;	// legs & torso
		else if(legverts == 3)
		{
			// There is an intermediate leg pair between the waist and the butt.
			unsigned int iv = ((unsigned int)m_DNA.TorsoVertebrae - waist)/2 + waist;
			if(node1idx == iv)
				node1 = 6;	// legs & torso
			else
				node1 = 4;	// torso
		}
		else
			node1 = 4;	// torso
	}
	else
	{
		// There are no arms or legs at the vertex.
		if(node1idx == m_DNA.TorsoVertebrae)
		{
			// This is the last vertex in the torso.
			if(m_DNA.TailVertebrae)
				node1 = 7;	// tail
			else
				node1 = 0;	// butt
		}
		else
			node1 = 4;	// torso
	}

	// This could be done with a table lookup. I'm too lazy right now. ;)
	// node0 : 0=neck, 1=arms & neck, 2=arms & torso, 3=torso, 4=legs & neck, 5=legs & torso
	// node1 : 0=butt, 1=arms & tail, 2=legs & butt, 3=arms & torso, 4=torso, 5=legs & tail, 6=legs & torso, 7=tail, 8=arms & butt
	const TORSO_SEGMENT torsoSegs[6][9] =
	{
		// node0 = neck
		{TS_NeckToButt,
		TS_NeckToShouldersAndTail,
		TS_NeckToHipsAndButt,
		TS_NeckToShouldersAndTorso,
		TS_NeckToTorso,
		TS_NeckToHipsAndTail,
		TS_NeckToHipsAndTorso,
		TS_NeckToTail,
		TS_NeckToShouldersAndButt},
		// node0 = arms & neck
		{TS_ShouldersAndNeckToButt,
		TS_ShouldersAndNeckToShouldersAndTail,
		TS_ShouldersAndNeckToHipsAndButt,
		TS_ShouldersAndNeckToShouldersAndTorso,
		TS_ShouldersAndNeckToTorso,
		TS_ShouldersAndNeckToHipsAndTail,
		TS_ShouldersAndNeckToHipsAndTorso,
		TS_ShouldersAndNeckToTail,
		TS_ShouldersAndNeckToShouldersAndButt},
		// node0 = arms & torso
		{TS_ShouldersAndTorsoToButt,
		TS_ShouldersAndTorsoToShouldersAndTail,
		TS_ShouldersAndTorsoToHipsAndButt,
		TS_ShouldersAndTorsoToShouldersAndTorso,
		TS_ShouldersAndTorsoToTorso,
		TS_ShouldersAndTorsoToHipsAndTail,
		TS_ShouldersAndTorsoToHipsAndTorso,
		TS_ShouldersAndTorsoToTail,
		TS_ShouldersAndTorsoToShouldersAndButt},
		// node0 = torso
		{TS_TorsoToButt,
		TS_TorsoToShouldersAndTail,
		TS_TorsoToHipsAndButt,
		TS_TorsoToShouldersAndTorso,
		TS_TorsoToTorso,
		TS_TorsoToHipsAndTail,
		TS_TorsoToHipsAndTorso,
		TS_TorsoToTail,
		TS_TorsoToShouldersAndButt},
		// node0 = legs & neck
		{TS_HipsAndNeckToButt,
		TS_HipsAndNeckToShouldersAndTail,
		TS_HipsAndNeckToHipsAndButt,
		TS_HipsAndNeckToShouldersAndTorso,
		TS_HipsAndNeckToTorso,
		TS_HipsAndNeckToHipsAndTail,
		TS_HipsAndNeckToHipsAndTorso,
		TS_HipsAndNeckToTail,
		TS_HipsAndNeckToShouldersAndButt},
		// node0 = legs & torso
		{TS_HipsAndTorsoToButt,
		TS_HipsAndTorsoToShouldersAndTail,
		TS_HipsAndTorsoToHipsAndButt,
		TS_HipsAndTorsoToShouldersAndTorso,
		TS_HipsAndTorsoToTorso,
		TS_HipsAndTorsoToHipsAndTail,
		TS_HipsAndTorsoToHipsAndTorso,
		TS_HipsAndTorsoToTail,
		TS_HipsAndTorsoToShouldersAndButt},
	};
	//qDebug("DpBody::TorsoSegment segment %d node0 %d node1 %d result %d", segmentIndex, node0, node1, torsoSegs[node0][node1]);
	return torsoSegs[node0][node1];
}

void DpBody::operator<<(QDataStream& in)
{
	in >> m_HitPoints;
	in >> m_IsLefty;
	quint8 sex;
	in >> sex;
	m_Sex = (char)sex;

	QByteArray badna;
	in >> badna;
	DhDNA_ dna(badna);
	SetDNA(dna, 1);

	AuPose curPose;
	in >> curPose;
	Pose(curPose, false, 1);
}

void DpBody::operator>>(QDataStream& out) const
{
	out << m_HitPoints;
	out << m_IsLefty;
	out << (quint8)m_Sex;

	QByteArray badna;
	badna.resize(sizeof(DhDNA_));
	DhDNA_ dna3(DNA());
	memcpy(badna.data(), &dna3, sizeof(DhDNA_));
	out << badna;

	out << CurrentGeneralPose();
}

float DpBody::OverallScale() const
{
	// The .84 factor here seems to be dependent on the 3ds model data or perhaps the 3ds body model loader.
	//return SizeFactor()/.68688;							// Size factor of nominal size 8 human = tanf(0) + .68688 = .68688
	return SizeFactor()/.84f;							// Size factor of nominal size 8 human = tanf(0) + .84 = .84
}

float DpBody::SizeFactor() const
{
	/*
	// Tangent curve is nice here. When centered on human size, it drops off/increases more with size difference away from human,
	// both smaller and larger. In the range 0..31, human size is arbitrarly chosen to be 8.
	qDebug("DpBody::SizeFactor sizeFactors generation code");
	for(int osize=0; osize<32; osize++)
	{
 		float x = (float)osize - 8;				// Range of x is -8 to 23. x = 0 at size 8 for nominal human.

 		//float a = (x/24.f) * (PIf/2.f);		// Range of a is -30 deg to 86.25 deg.
 		//float a = (x/25.f) * (PIf/2.f);			// Range of a is -28.8 deg to 82.8 deg.
 		//float a = (x/36.95f) * (PIf/2.f);			// Range of a is -19.5 deg to 82.8 deg.
 		float a = (x/35.056f) * (PIf/2.f);			// Range of a is -20.54 deg to 82.8 deg.

 		float sizeFactor = tanf(a) + .68688;	// Calculate scaling so size 8 = .68688, putting human sz 8 camera height at 5'6".
 		//float sizeFactor = tanf(a) + .84;		// Calculate scaling so size 8 = .84, putting human sz 6 camera height about 5'6".
		qDebug("%gf,", sizeFactor);				// Formatted for easy copy/paste of output into the table below.
	}
	*/

	// There are only 32 possible values.
	// A table is faster than doing a bunch of math.
	float sizeFactors[32] =
	{
		0.312228f,
		0.362515f,
		0.41136f,
		0.459014f,
		0.505703f,
		0.55164f,
		0.597023f,
		0.642042f,
		0.68688f,
		0.731718f,
		0.776737f,
		0.82212f,
		0.868057f,
		0.914746f,
		0.9624f,
		1.01124f,
		1.06153f,
		1.11354f,
		1.16757f,
		1.22399f,
		1.28319f,
		1.34564f,
		1.41189f,
		1.4826f,
		1.55853f,
		1.64065f,
		1.7301f,
		1.82833f,
		1.93716f,
		2.05891f,
		2.19663f,
		2.35435f,
		/*
		0.290245f,
		0.369436f,
		0.444072f,
		0.51508f,
		0.583244f,
		0.64924f,
		0.713671f,
		0.777085f,
		0.84f,
		0.902915f,
		0.966329f,
		1.03076f,
		1.09676f,
		1.16492f,
		1.23593f,
		1.31056f,
		1.38975f,
		1.47462f,
		1.56654f,
		1.66727f,
		1.77906f,
		1.90489f,
		2.04879f,
		2.21638f,
		2.41575f,
		2.65899f,
		2.96511f,
		3.36571f,
		3.91768f,
		4.73474f,
		6.08219f,
		8.75582f,
		*/
	};
	return sizeFactors[m_DNA.OverallSize];
}

quint64 DpBody::MemoryUsage() const
{
	quint64 mem = sizeof(DpBody);
	for(QVector<DpBodyPart*>::const_iterator it = constBegin(); it != constEnd(); it++)
		mem += (*it)->MemoryUsage();
	return mem;
}

//static
bool DpBody::CanRestorePositionPose(const AuPose& pose)
{
	switch(pose.Pose())
	{
	case AuPose::Crouch:
	case AuPose::CrouchCreep:
	case AuPose::Fly:
	case AuPose::FlyFast:
	case AuPose::Glide:
	case AuPose::Hover:
	case AuPose::Prone:
	case AuPose::ProneCreep:
	case AuPose::SitOnGround:
	case AuPose::SitOnGroundCreep:
	case AuPose::Stand:
	case AuPose::StandRun:
	case AuPose::StandWalk:
	case AuPose::Swim:
	case AuPose::SwimFast:
	case AuPose::TreadWater:
		return true;
	case AuPose::Beckon:
	case AuPose::BerthBottomFaceDown:
	case AuPose::BerthBottomFaceLeft:
	case AuPose::BerthBottomFaceRight:
	case AuPose::BerthBottomFaceUp:
	case AuPose::BerthTopFaceDown:
	case AuPose::BerthTopFaceLeft:
	case AuPose::BerthTopFaceRight:
	case AuPose::BerthTopFaceUp:
	case AuPose::Bite:
	case AuPose::Block:
	case AuPose::Command_Sit:
	case AuPose::Command_SitStationary:
	case AuPose::Command_Stand:
	case AuPose::Command_StandStationary:
	case AuPose::ConsoleCommand:
	case AuPose::ConsoleEngine:
	case AuPose::ConsoleFireControl:
	case AuPose::ConsoleNav:
	case AuPose::ConsolePilot:
	case AuPose::ConsolePower:
	case AuPose::ConsoleSensor:
	case AuPose::ConsoleShield:
	case AuPose::ConsoleSurgery:
	case AuPose::ConsoleSurgeryUnit:
	case AuPose::ConsoleTransporter:
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurretBottom:
	case AuPose::ConsoleTurretTop:
	case AuPose::DeadFaceDown:
	case AuPose::DeadFaceLeft:
	case AuPose::DeadFaceRight:
	case AuPose::DeadFaceUp:
	case AuPose::Dig:
	case AuPose::Eat:
	case AuPose::Engine_Sit:
	case AuPose::Engine_SitStationary:
	case AuPose::Engine_Stand:
	case AuPose::Engine_StandStationary:
	case AuPose::EyesOnly:
	case AuPose::FireControl_Sit:
	case AuPose::FireControl_SitStationary:
	case AuPose::FireControl_Stand:
	case AuPose::FireControl_StandStationary:
	case AuPose::GameTableSeat0_0:
	case AuPose::GameTableSeat0_45:
	case AuPose::GameTableSeat1_0:
	case AuPose::GameTableSeat1_45:
	case AuPose::GameTableSeat2_0:
	case AuPose::GameTableSeat2_45:
	case AuPose::GameTableSeat3_0:
	case AuPose::GameTableSeat3_45:
	case AuPose::Helm_Sit:
	case AuPose::Helm_SitStationary:
	case AuPose::Helm_Stand:
	case AuPose::Helm_StandStationary:
	case AuPose::Hug:
	case AuPose::MouthPursed:
	case AuPose::MouthClosed:
	case AuPose::MouthOpen:
	case AuPose::MouthAgape:
	case AuPose::MouthOpenWide:
	case AuPose::MouthYawn:
	case AuPose::MouthBite:
	case AuPose::MouthLaugh:
	case AuPose::MouthYap:
	case AuPose::MouthWhisper:
	case AuPose::MouthTalk:
	case AuPose::MouthJabber:
	case AuPose::MouthYell:
	case AuPose::MouthSmile:
	case AuPose::MouthGrin:
	case AuPose::MouthSad:
	case AuPose::MouthFrown:
	case AuPose::MouthCry:
	case AuPose::MouthPerplexed:
	case AuPose::MouthGrim:
	case AuPose::MouthSardonic:
	case AuPose::Init:
	case AuPose::Jump:
	case AuPose::Kick:
	case AuPose::Ladder:
	case AuPose::LadderClimb:
	case AuPose::Lash:
	case AuPose::Medic_Sit:
	case AuPose::Medic_SitStationary:
	case AuPose::Medic_Stand:
	case AuPose::Medic_StandStationary:
	case AuPose::Nav_Sit:
	case AuPose::Nav_SitStationary:
	case AuPose::Nav_Stand:
	case AuPose::Nav_StandStationary:
	case AuPose::Patient_Prone:
	case AuPose::Patient_Sit:
	case AuPose::Patient_SitStationary:
	case AuPose::Patient_Stand:
	case AuPose::Patient_StandStationary:
	case AuPose::Point:
	case AuPose::Post_Sit:
	case AuPose::Post_Stand:
	case AuPose::Power_Sit:
	case AuPose::Power_SitStationary:
	case AuPose::Power_Stand:
	case AuPose::Power_StandStationary:
	case AuPose::Psionic:
	case AuPose::ReceptiveMate:
	case AuPose::Sensor_Sit:
	case AuPose::Sensor_SitStationary:
	case AuPose::Sensor_Stand:
	case AuPose::Sensor_StandStationary:
	case AuPose::Shield_Sit:
	case AuPose::Shield_SitStationary:
	case AuPose::Shield_Stand:
	case AuPose::Shield_StandStationary:
	case AuPose::Spawn:
	case AuPose::Stomp:
	case AuPose::Straddle:
	case AuPose::MateOnBottom:
	case AuPose::MateOnTop:
	case AuPose::Strike:
	case AuPose::Throw:
	case AuPose::Transporter_Sit:
	case AuPose::Transporter_SitStationary:
	case AuPose::Transporter_Stand:
	case AuPose::Transporter_StandStationary:
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
	case AuPose::Upright:
	case AuPose::Wag:
	case AuPose::Wave:
	case AuPose::WritheFaceDown:
	case AuPose::WritheFaceLeft:
	case AuPose::WritheFaceRight:
	case AuPose::WritheFaceUp:
		break;
	}
	return false;
}

//static
AuPose DpBody::StationaryPose(const AuPose& pose, const DhDNA& dna)
{
	switch(pose.Pose())
	{
	case AuPose::Init:
	case AuPose::Beckon:
	case AuPose::Bite:
	case AuPose::Block:
	case AuPose::BerthBottomFaceDown:
	case AuPose::BerthBottomFaceLeft:
	case AuPose::BerthBottomFaceRight:
	case AuPose::BerthBottomFaceUp:
	case AuPose::BerthTopFaceDown:
	case AuPose::BerthTopFaceLeft:
	case AuPose::BerthTopFaceRight:
	case AuPose::BerthTopFaceUp:
	case AuPose::GameTableSeat0_0:
	case AuPose::GameTableSeat1_0:
	case AuPose::GameTableSeat2_0:
	case AuPose::GameTableSeat3_0:
	case AuPose::GameTableSeat0_45:
	case AuPose::GameTableSeat1_45:
	case AuPose::GameTableSeat2_45:
	case AuPose::GameTableSeat3_45:
	case AuPose::ConsoleCommand:
	case AuPose::ConsoleNav:
	case AuPose::ConsoleEngine:
	case AuPose::ConsoleFireControl:
	case AuPose::ConsoleSurgery:
	case AuPose::ConsolePilot:
	case AuPose::ConsolePower:
	case AuPose::ConsoleSensor:
	case AuPose::ConsoleShield:
	case AuPose::ConsoleTransporter:
	case AuPose::ConsoleSurgeryUnit:
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::ConsoleTurretBottom:
	case AuPose::ConsoleTurretTop:
	case AuPose::Crouch:
	case AuPose::DeadFaceDown:
	case AuPose::DeadFaceLeft:
	case AuPose::DeadFaceRight:
	case AuPose::DeadFaceUp:
	case AuPose::Eat:
	case AuPose::EyesOnly:
	case AuPose::Glide:
	case AuPose::Hover:
	case AuPose::Hug:
	case AuPose::Kick:
	case AuPose::Lash:
	case AuPose::Ladder:
	case AuPose::Point:
	case AuPose::Post_Sit:
	case AuPose::Post_Stand:
	case AuPose::Prone:
	case AuPose::Psionic:
	case AuPose::SitOnGround:
	case AuPose::Spawn:
	case AuPose::Stand:
	case AuPose::Stomp:
	case AuPose::Straddle:
	case AuPose::MateOnBottom:
	case AuPose::MateOnTop:
	case AuPose::Strike:
	case AuPose::Throw:
	case AuPose::TreadWater:
	case AuPose::Upright:
	case AuPose::Wag:
	case AuPose::Wave:
	case AuPose::WritheFaceDown:
	case AuPose::WritheFaceLeft:
	case AuPose::WritheFaceRight:
	case AuPose::WritheFaceUp:
	case AuPose::ReceptiveMate:
	case AuPose::Command_Sit:
	case AuPose::Command_SitStationary:
	case AuPose::Command_Stand:
	case AuPose::Command_StandStationary:
	case AuPose::Engine_Sit:
	case AuPose::Engine_SitStationary:
	case AuPose::Engine_Stand:
	case AuPose::Engine_StandStationary:
	case AuPose::FireControl_Sit:
	case AuPose::FireControl_SitStationary:
	case AuPose::FireControl_Stand:
	case AuPose::FireControl_StandStationary:
	case AuPose::Helm_Sit:
	case AuPose::Helm_SitStationary:
	case AuPose::Helm_Stand:
	case AuPose::Helm_StandStationary:
	case AuPose::Medic_Sit:
	case AuPose::Medic_SitStationary:
	case AuPose::Medic_Stand:
	case AuPose::Medic_StandStationary:
	case AuPose::Nav_Sit:
	case AuPose::Nav_SitStationary:
	case AuPose::Nav_Stand:
	case AuPose::Nav_StandStationary:
	case AuPose::Patient_Sit:
	case AuPose::Patient_SitStationary:
	case AuPose::Patient_Stand:
	case AuPose::Patient_StandStationary:
	case AuPose::Patient_Prone:
	case AuPose::Power_Sit:
	case AuPose::Power_SitStationary:
	case AuPose::Power_Stand:
	case AuPose::Power_StandStationary:
	case AuPose::Sensor_Sit:
	case AuPose::Sensor_SitStationary:
	case AuPose::Sensor_Stand:
	case AuPose::Sensor_StandStationary:
	case AuPose::Shield_Sit:
	case AuPose::Shield_SitStationary:
	case AuPose::Shield_Stand:
	case AuPose::Shield_StandStationary:
	case AuPose::Transporter_Sit:
	case AuPose::Transporter_SitStationary:
	case AuPose::Transporter_Stand:
	case AuPose::Transporter_StandStationary:
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
	case AuPose::MouthPursed:
	case AuPose::MouthClosed:
	case AuPose::MouthOpen:
	case AuPose::MouthAgape:
	case AuPose::MouthOpenWide:
	case AuPose::MouthYawn:
	case AuPose::MouthBite:
	case AuPose::MouthLaugh:
	case AuPose::MouthYap:
	case AuPose::MouthWhisper:
	case AuPose::MouthTalk:
	case AuPose::MouthJabber:
	case AuPose::MouthYell:
	case AuPose::MouthSmile:
	case AuPose::MouthGrin:
	case AuPose::MouthSad:
	case AuPose::MouthFrown:
	case AuPose::MouthCry:
	case AuPose::MouthPerplexed:
	case AuPose::MouthGrim:
	case AuPose::MouthSardonic:
		// These poses don't really have a corresponding "stationary" pose
		// or they ARE stationary poses.
		return pose;
	case AuPose::CrouchCreep:
		return AuPose(AuPose::Crouch);
	case AuPose::SitOnGroundCreep:
		return AuPose(AuPose::SitOnGround);
	case AuPose::Dig:
		return AuPose(AuPose::Stand);
	case AuPose::Fly:
	case AuPose::FlyFast:
		if(dna.ArmType == DhDNA::ARM_WingDragonFly ||
				dna.ArmType == DhDNA::ARM_WingFairy)
			return AuPose(AuPose::Hover);
		else
			return AuPose(AuPose::Glide);
	case AuPose::Jump:
		return AuPose(AuPose::Stand);
	case AuPose::LadderClimb:
		return AuPose(AuPose::Ladder);
	case AuPose::ProneCreep:
		return AuPose(AuPose::Prone);
	case AuPose::StandRun:
	case AuPose::StandWalk:
		return AuPose(AuPose::Stand);
	case AuPose::Swim:
	case AuPose::SwimFast:
		return AuPose(AuPose::TreadWater);
	}
	return pose;
}

float DpBody::MovementSpeed(const AuPose& pose) const
{
	float speed = 0;
	switch(pose.Pose())
	{
	case AuPose::Init:
	case AuPose::EyesOnly:
	case AuPose::Spawn:
		break;
	case AuPose::Beckon:
	case AuPose::Bite:
	case AuPose::Block:
	case AuPose::Eat:
	case AuPose::Kick:
	case AuPose::Lash:
	case AuPose::Point:
	case AuPose::Psionic:
	case AuPose::Stomp:
	case AuPose::Strike:
	case AuPose::Throw:
	case AuPose::Upright:
	case AuPose::Wag:
	case AuPose::Wave:
	case AuPose::MouthPursed:
	case AuPose::MouthClosed:
	case AuPose::MouthOpen:
	case AuPose::MouthAgape:
	case AuPose::MouthOpenWide:
	case AuPose::MouthYawn:
	case AuPose::MouthBite:
	case AuPose::MouthLaugh:
	case AuPose::MouthYap:
	case AuPose::MouthWhisper:
	case AuPose::MouthTalk:
	case AuPose::MouthJabber:
	case AuPose::MouthYell:
	case AuPose::MouthSmile:
	case AuPose::MouthGrin:
	case AuPose::MouthSad:
	case AuPose::MouthFrown:
	case AuPose::MouthCry:
	case AuPose::MouthPerplexed:
	case AuPose::MouthGrim:
	case AuPose::MouthSardonic:
		Q_ASSERT(false);	// Invalid lower torso pose.
		break;
	case AuPose::BerthBottomFaceDown:
	case AuPose::BerthBottomFaceLeft:
	case AuPose::BerthBottomFaceRight:
	case AuPose::BerthBottomFaceUp:
	case AuPose::BerthTopFaceDown:
	case AuPose::BerthTopFaceLeft:
	case AuPose::BerthTopFaceRight:
	case AuPose::BerthTopFaceUp:
	case AuPose::GameTableSeat0_0:
	case AuPose::GameTableSeat1_0:
	case AuPose::GameTableSeat2_0:
	case AuPose::GameTableSeat3_0:
	case AuPose::GameTableSeat0_45:
	case AuPose::GameTableSeat1_45:
	case AuPose::GameTableSeat2_45:
	case AuPose::GameTableSeat3_45:
	case AuPose::ConsoleCommand:
	case AuPose::ConsoleNav:
	case AuPose::ConsoleEngine:
	case AuPose::ConsoleFireControl:
	case AuPose::ConsoleSurgery:
	case AuPose::ConsolePilot:
	case AuPose::ConsolePower:
	case AuPose::ConsoleSensor:
	case AuPose::ConsoleShield:
	case AuPose::ConsoleTransporter:
	case AuPose::ConsoleSurgeryUnit:
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::ConsoleTurretBottom:
	case AuPose::ConsoleTurretTop:
	case AuPose::Crouch:
	case AuPose::DeadFaceDown:
	case AuPose::DeadFaceLeft:
	case AuPose::DeadFaceRight:
	case AuPose::DeadFaceUp:
	case AuPose::Hover:
	case AuPose::Hug:
	case AuPose::Ladder:
	case AuPose::Post_Sit:
	case AuPose::Post_Stand:
	case AuPose::Prone:
	case AuPose::SitOnGround:
	case AuPose::Stand:
	case AuPose::Straddle:
	case AuPose::MateOnBottom:
	case AuPose::MateOnTop:
	case AuPose::TreadWater:
	case AuPose::WritheFaceDown:
	case AuPose::WritheFaceLeft:
	case AuPose::WritheFaceRight:
	case AuPose::WritheFaceUp:
	case AuPose::ReceptiveMate:
	case AuPose::Command_Sit:
	case AuPose::Command_SitStationary:
	case AuPose::Command_Stand:
	case AuPose::Command_StandStationary:
	case AuPose::Engine_Sit:
	case AuPose::Engine_SitStationary:
	case AuPose::Engine_Stand:
	case AuPose::Engine_StandStationary:
	case AuPose::FireControl_Sit:
	case AuPose::FireControl_SitStationary:
	case AuPose::FireControl_Stand:
	case AuPose::FireControl_StandStationary:
	case AuPose::Helm_Sit:
	case AuPose::Helm_SitStationary:
	case AuPose::Helm_Stand:
	case AuPose::Helm_StandStationary:
	case AuPose::Medic_Sit:
	case AuPose::Medic_SitStationary:
	case AuPose::Medic_Stand:
	case AuPose::Medic_StandStationary:
	case AuPose::Nav_Sit:
	case AuPose::Nav_SitStationary:
	case AuPose::Nav_Stand:
	case AuPose::Nav_StandStationary:
	case AuPose::Patient_Sit:
	case AuPose::Patient_SitStationary:
	case AuPose::Patient_Stand:
	case AuPose::Patient_StandStationary:
	case AuPose::Patient_Prone:
	case AuPose::Power_Sit:
	case AuPose::Power_SitStationary:
	case AuPose::Power_Stand:
	case AuPose::Power_StandStationary:
	case AuPose::Sensor_Sit:
	case AuPose::Sensor_SitStationary:
	case AuPose::Sensor_Stand:
	case AuPose::Sensor_StandStationary:
	case AuPose::Shield_Sit:
	case AuPose::Shield_SitStationary:
	case AuPose::Shield_Stand:
	case AuPose::Shield_StandStationary:
	case AuPose::Transporter_Sit:
	case AuPose::Transporter_SitStationary:
	case AuPose::Transporter_Stand:
	case AuPose::Transporter_StandStationary:
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
		speed = 0;
		break;
	case AuPose::LadderClimb:
		speed = .75f;
		break;
	case AuPose::CrouchCreep:
	case AuPose::Dig:
		speed = .5f;
		break;
	case AuPose::SitOnGroundCreep:
		speed = .3f;
		break;
	case AuPose::Fly:
		speed = 2.5f;
		break;
	case AuPose::FlyFast:
		speed = 3.f;
		break;
	case AuPose::Glide:
		speed = 2.f;
		break;
	case AuPose::Jump:
	case AuPose::StandWalk:
		speed = 1.f;
		break;
	case AuPose::StandRun:
		speed = 2.5f;
		break;
	case AuPose::ProneCreep:
		speed = 1.f/3.f;
		break;
	case AuPose::Swim:
		speed = .75f;
		break;
	case AuPose::SwimFast:
		speed = .85f;
		break;
	}

	return speed * ((float)m_DNA.Speed + 1);
}

int DpBody::StaminaTrend() const
{
	switch(CurrentGeneralPose().Pose())
	{
	case AuPose::Init:
	case AuPose::Spawn:
	case AuPose::Beckon:
	case AuPose::Bite:
	case AuPose::Block:
	case AuPose::Eat:
	case AuPose::Hug:
	case AuPose::Kick:
	case AuPose::Lash:
	case AuPose::Point:
	case AuPose::Psionic:
	case AuPose::Stomp:
	case AuPose::Strike:
	case AuPose::Throw:
	case AuPose::Upright:
	case AuPose::Wag:
	case AuPose::Wave:
	case AuPose::MouthPursed:
	case AuPose::MouthClosed:
	case AuPose::MouthOpen:
	case AuPose::MouthAgape:
	case AuPose::MouthOpenWide:
	case AuPose::MouthYawn:
	case AuPose::MouthBite:
	case AuPose::MouthLaugh:
	case AuPose::MouthYap:
	case AuPose::MouthWhisper:
	case AuPose::MouthTalk:
	case AuPose::MouthJabber:
	case AuPose::MouthYell:
	case AuPose::MouthSmile:
	case AuPose::MouthGrin:
	case AuPose::MouthSad:
	case AuPose::MouthFrown:
	case AuPose::MouthCry:
	case AuPose::MouthPerplexed:
	case AuPose::MouthGrim:
	case AuPose::MouthSardonic:
		return 0;	// Not a "general" pose.

	case AuPose::FlyFast:
	case AuPose::MateOnTop:
	case AuPose::StandRun:
	case AuPose::SwimFast:
		// Fast movement uses lots of stamina.
		return -4;
	case AuPose::MateOnBottom:
		return -3;

	case AuPose::Dig:
		return -2;

	case AuPose::ProneCreep:
		return -1;

	case AuPose::CrouchCreep:
	case AuPose::SitOnGroundCreep:
	case AuPose::DeadFaceDown:
	case AuPose::DeadFaceLeft:
	case AuPose::DeadFaceRight:
	case AuPose::DeadFaceUp:
	case AuPose::Fly:
	case AuPose::Jump:			// Jump energy taken when jump initiated.
	case AuPose::WritheFaceDown:
	case AuPose::WritheFaceLeft:
	case AuPose::WritheFaceRight:
	case AuPose::WritheFaceUp:
	case AuPose::ReceptiveMate:
		return 0;

	// Normal speed movement regains a minimal amount of stamina.
	case AuPose::Glide:
	case AuPose::StandWalk:
		return 4;
	case AuPose::Hover:
	case AuPose::LadderClimb:
	case AuPose::Swim:
		return 3;
	case AuPose::Straddle:
		return 5;

	// Remaining motionless regains stamina faster than walking.
	case AuPose::GameTableSeat0_0:
	case AuPose::GameTableSeat1_0:
	case AuPose::GameTableSeat2_0:
	case AuPose::GameTableSeat3_0:
	case AuPose::GameTableSeat0_45:
	case AuPose::GameTableSeat1_45:
	case AuPose::GameTableSeat2_45:
	case AuPose::GameTableSeat3_45:
	case AuPose::ConsoleCommand:
	case AuPose::ConsoleNav:
	case AuPose::ConsoleEngine:
	case AuPose::ConsoleFireControl:
	case AuPose::ConsoleSurgery:
	case AuPose::ConsolePilot:
	case AuPose::ConsolePower:
	case AuPose::ConsoleSensor:
	case AuPose::ConsoleShield:
	case AuPose::ConsoleTransporter:
	case AuPose::ConsoleSurgeryUnit:
	case AuPose::ConsoleTurret0:
	case AuPose::ConsoleTurret45:
	case AuPose::ConsoleTurret90:
	case AuPose::ConsoleTurret135:
	case AuPose::ConsoleTurret180:
	case AuPose::ConsoleTurret225:
	case AuPose::ConsoleTurret270:
	case AuPose::ConsoleTurret315:
	case AuPose::ConsoleTurretBottom:
	case AuPose::ConsoleTurretTop:
	case AuPose::EyesOnly:
	case AuPose::SitOnGround:
	case AuPose::Command_Sit:
	case AuPose::Command_SitStationary:
	case AuPose::Engine_Sit:
	case AuPose::Engine_SitStationary:
	case AuPose::FireControl_Sit:
	case AuPose::FireControl_SitStationary:
	case AuPose::Helm_Sit:
	case AuPose::Helm_SitStationary:
	case AuPose::Medic_Sit:
	case AuPose::Medic_SitStationary:
	case AuPose::Nav_Sit:
	case AuPose::Nav_SitStationary:
	case AuPose::Patient_Sit:
	case AuPose::Patient_SitStationary:
	case AuPose::Post_Sit:
	case AuPose::Power_Sit:
	case AuPose::Power_SitStationary:
	case AuPose::Sensor_Sit:
	case AuPose::Sensor_SitStationary:
	case AuPose::Shield_Sit:
	case AuPose::Shield_SitStationary:
	case AuPose::Transporter_Sit:
	case AuPose::Transporter_SitStationary:
	case AuPose::Turret_Sit:
	case AuPose::Turret_SitStationary:
	case AuPose::Turret_SitSwivel3D:
		return 9;
	case AuPose::Stand:
	case AuPose::Command_Stand:
	case AuPose::Command_StandStationary:
	case AuPose::Engine_Stand:
	case AuPose::Engine_StandStationary:
	case AuPose::FireControl_Stand:
	case AuPose::FireControl_StandStationary:
	case AuPose::Helm_Stand:
	case AuPose::Helm_StandStationary:
	case AuPose::Medic_Stand:
	case AuPose::Medic_StandStationary:
	case AuPose::Nav_Stand:
	case AuPose::Nav_StandStationary:
	case AuPose::Patient_Stand:
	case AuPose::Patient_StandStationary:
	case AuPose::Post_Stand:
	case AuPose::Power_Stand:
	case AuPose::Power_StandStationary:
	case AuPose::Sensor_Stand:
	case AuPose::Sensor_StandStationary:
	case AuPose::Shield_Stand:
	case AuPose::Shield_StandStationary:
	case AuPose::Transporter_Stand:
	case AuPose::Transporter_StandStationary:
	case AuPose::Turret_Stand:
	case AuPose::Turret_StandStationary:
		return 8;
	case AuPose::Crouch:
	case AuPose::Ladder:
		return 7;
	case AuPose::TreadWater:
		return 6;

	// Lying prone regians stamina the fastest.
	case AuPose::Prone:
	case AuPose::Patient_Prone:
	case AuPose::BerthBottomFaceDown:
	case AuPose::BerthBottomFaceLeft:
	case AuPose::BerthBottomFaceRight:
	case AuPose::BerthBottomFaceUp:
	case AuPose::BerthTopFaceDown:
	case AuPose::BerthTopFaceLeft:
	case AuPose::BerthTopFaceRight:
	case AuPose::BerthTopFaceUp:
		return 10;
	}
	return 0;
}

AuDNABase::CHROMATID DpBody::Chromatid() const
{
	return (m_Sex == Male) ? AuDNABase::Alpha : AuDNABase::Beta;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DpBodyPart::DpBodyPart(DpBody* body)
{
	m_DNA = body->DNA();	// Make a copy of the DNA.
	m_Body = body;
}

quint64 DpBodyPart::MemoryUsage() const
{
	quint64 mem = 0;
	mem += sizeof(m_Body);
	mem += sizeof(m_DNA);
	mem += sizeof(m_ConnectLoc);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

void DpTorso::OneTimeCalcs()
{

}

// Origin of the torso skin model is the center of the waist.
void DpTorso::CreateSkin(AuGLEffects& re, AuGLModelCache& mc)
{
	OneTimeCalcs();
	//qDebug("DpTorso::CreateSkin - %d vertebrae, %s wearing body armor", m_DNA.TorsoVertebrae,
			//m_Body->HasBodyPowerArmorOn()?"is":"is not");
	if(!m_DNA.TorsoVertebrae)
		return;	// Nothing to skin.

	// Determine head/neck point for first segment.
	float vertlen = m_Body->TorsoSegmentLength();
	int waist = m_DNA.WaistVertex();
	AuPoint loc(0, vertlen*(waist-1), 0);	// -1 due to -y direction.

	// Draw model
	Au3dsGLSLModelInstance* modelTorso;
	float length = m_DNA.Length(m_DNA.TorsoSegmentLength);
	float thickNess = m_DNA.TorsoThicknessCalc(length);
	DhDNA::BONES prevBone = DhDNA::BONE_Torso0;
	DhDNA::BONES thisBone = DhDNA::BONE_Torso0;
	DhDNA::BONES nextBone = (DhDNA::BONES)(thisBone + 1);
	QString sPrefix = m_Body->GetBodyModelPrefix();
	QString fileInstanceName = sPrefix;
	bool bHasArmor = m_Body->HasBodyPowerArmorOn();
	Au3dsGLSLModelDefinition modelDef(vertlen, m_Body->OverallScale(), m_Body->m_uTOffset, m_Body->m_vTOffset, 
		m_Body->m_uBOffset, m_Body->m_vBOffset, prevBone, thisBone, nextBone, loc, length/1.5f, thickNess/.875f, 1, 1, 1,
		m_Body->m_TopColor[bHasArmor], m_Body->m_TopShininess[bHasArmor], m_Body->m_BottomColor[bHasArmor],
		m_Body->m_BottomShininess[bHasArmor], m_Body->m_Glow[bHasArmor], m_Body->m_Glowdim[bHasArmor], true, true, true);
	// Try and get previous torso model arrangement if it exists
	for(unsigned int iv=0; iv<m_DNA.TorsoVertebrae; iv++)
	{
		if(iv == (unsigned int)m_DNA.TorsoVertebrae-1)
			nextBone = thisBone;
		else
			nextBone = (DhDNA::BONES)(thisBone + 1);
		DpBody::TORSO_SEGMENT torsoSegement = m_Body->TorsoSegment(iv);
		if(torsoSegement == DpBody::TS_None)
			continue;
		if(!bHasArmor)
		{
			// TORSO CREST
			if(m_DNA.TorsoCrestSize > 0)
				fileInstanceName = QString("%1%2TC%3%4").arg(fileInstanceName).arg((int)torsoSegement).arg(m_DNA.CrestSubtype).arg(m_DNA.TorsoCrestSize);
			else
				fileInstanceName = QString("%1%2").arg(fileInstanceName).arg((int)torsoSegement);
		}
		else
			fileInstanceName = QString("%1%2").arg(fileInstanceName).arg((int)torsoSegement);
		prevBone = thisBone;
		thisBone = (DhDNA::BONES)(thisBone + 1);
	}
	modelTorso = mc.CreateBodyModel(re, fileInstanceName, m_Body->m_Min, m_Body->m_Max, false, modelDef);
	if(modelTorso)
	{
		m_Body->m_Models.append(modelTorso);
		return;
	}
	// Arrangement not found. Set to false so that CreateBodyModel knows that
	// we are not expecting it so that the pieces get created.
	modelDef.m_bArrangement = false;
	// Reset variables and make the model arrangement
	prevBone = DhDNA::BONE_Torso0;
	thisBone = DhDNA::BONE_Torso0;
	nextBone = DhDNA::BONE_Torso0;
	for(unsigned int iv=0; iv<m_DNA.TorsoVertebrae; iv++)
	{
		if(iv == (unsigned int)m_DNA.TorsoVertebrae-1)
			nextBone = thisBone;
		else
			nextBone = (DhDNA::BONES)(thisBone + 1);

		QString torsoModel;
		QString modelName;
		DpBody::TORSO_SEGMENT torsoSegement = m_Body->TorsoSegment(iv);
		if(torsoSegement == DpBody::TS_None)
			continue;
		else if(torsoSegement == DpBody::TS_NeckToButt || torsoSegement == DpBody::TS_NeckToHipsAndButt ||
			torsoSegement == DpBody::TS_HipsAndNeckToHipsAndButt || torsoSegement == DpBody::TS_NeckToHipsAndTail)
			modelName = QString("%1").arg("NoLimbClosedToButt");
		else if(torsoSegement == DpBody::TS_NeckToShouldersAndTail || torsoSegement == DpBody::TS_NeckToTail ||
			torsoSegement == DpBody::TS_ShouldersAndNeckToButt || torsoSegement == DpBody::TS_ShouldersAndNeckToHipsAndButt ||
			torsoSegement == DpBody::TS_ShouldersAndNeckToHipsAndTail || torsoSegement == DpBody::TS_ShouldersAndNeckToTail)
			modelName = QString("%1").arg("TopLimbToButt");
		else if(torsoSegement == DpBody::TS_NeckToShouldersAndTorso || torsoSegement == DpBody::TS_TorsoToShouldersAndTorso)
			modelName = QString("%1").arg("NoLimbToArm");
		else if(torsoSegement == DpBody::TS_NeckToTorso)
			modelName = QString("%1").arg("NoLimbClosedToNoLimb");
		else if(torsoSegement == DpBody::TS_NeckToHipsAndTorso)
			modelName = QString("%1").arg("TopLimbToLeg");
		else if(torsoSegement == DpBody::TS_NeckToShouldersAndButt)
			modelName = QString("%1").arg("NoLimbClosedToArmClosed");
		else if(torsoSegement == DpBody::TS_ShouldersAndNeckToShouldersAndTail ||
			torsoSegement == DpBody::TS_ShouldersAndNeckToShouldersAndButt)
			modelName = QString("%1").arg("TopToTop");
		else if(torsoSegement == DpBody::TS_ShouldersAndNeckToShouldersAndTorso)
			modelName = QString("%1").arg("TopLimbToArm");
		else if(torsoSegement == DpBody::TS_ShouldersAndNeckToTorso)
			modelName = QString("%1").arg("TopLimbToNoLimb");
		else if(torsoSegement == DpBody::TS_ShouldersAndNeckToHipsAndTorso)
			modelName = QString("%1").arg("TopLimbToLeg");
		else if(torsoSegement == DpBody::TS_ShouldersAndTorsoToButt || torsoSegement == DpBody::TS_ShouldersAndTorsoToHipsAndButt || 
			torsoSegement == DpBody::TS_ShouldersAndTorsoToHipsAndTail || torsoSegement == DpBody::TS_ShouldersAndTorsoToTail)
			modelName = QString("%1").arg("ArmToButt");
		else if(torsoSegement == DpBody::TS_ShouldersAndTorsoToShouldersAndTail || 
			torsoSegement == DpBody::TS_ShouldersAndTorsoToShouldersAndButt)
			modelName = QString("%1").arg("ArmToArmClosed");
		else if(torsoSegement == DpBody::TS_ShouldersAndTorsoToShouldersAndTorso)
			modelName = QString("%1").arg("ArmToArm");
		else if(torsoSegement == DpBody::TS_ShouldersAndTorsoToTorso)
			modelName = QString("%1").arg("ArmToNoLimb");
		else if(torsoSegement == DpBody::TS_ShouldersAndTorsoToHipsAndTorso)
			modelName = QString("%1").arg("ArmToLeg");
		else if(torsoSegement == DpBody::TS_TorsoToButt || torsoSegement == DpBody::TS_TorsoToHipsAndButt ||
			torsoSegement == DpBody::TS_TorsoToHipsAndTail || torsoSegement == DpBody::TS_TorsoToTail)
			modelName = QString("%1").arg("NoLimbToButt");
		else if(torsoSegement == DpBody::TS_TorsoToShouldersAndTail || torsoSegement == DpBody::TS_TorsoToShouldersAndButt)
			modelName = QString("%1").arg("ModelMissing");
		else if(torsoSegement == DpBody::TS_TorsoToTorso)
			modelName = QString("%1").arg("NoLimbToNoLimb");
		else if(torsoSegement == DpBody::TS_TorsoToHipsAndTorso)
			modelName = QString("%1").arg("NoLimbToLeg");
		else if(torsoSegement == DpBody::TS_HipsAndNeckToButt || torsoSegement == DpBody::TS_HipsAndNeckToTail ||
			torsoSegement == DpBody::TS_HipsAndTorsoToButt || torsoSegement == DpBody::TS_HipsAndTorsoToHipsAndButt ||
			torsoSegement == DpBody::TS_HipsAndTorsoToHipsAndTail || torsoSegement == DpBody::TS_HipsAndTorsoToTail)
			modelName = QString("%1").arg("LegToButt");
		else if(torsoSegement == DpBody::TS_HipsAndTorsoToTorso)
			modelName = QString("%1").arg("LegToNoLimb");
		else if(torsoSegement == DpBody::TS_HipsAndTorsoToHipsAndTorso)
			modelName = QString("%1").arg("LegToLeg");
		else if(torsoSegement == DpBody::TS_HipsAndNeckToHipsAndTorso)
			modelName = QString("%1").arg("LegClosedToLeg");
		else if(torsoSegement == DpBody::TS_HipsAndNeckToTorso)
			modelName = QString("%1").arg("LegClosedToNoLimb");
		else if(torsoSegement == DpBody::TS_HipsAndNeckToHipsAndTail)
			modelName = QString("%1").arg("LegClosedToButt");
		else
			modelName = QString("%1").arg("ModelMissing");

		// Ones not needed or used
		//DpBody::TS_HipsAndNeckToShouldersAndTail,
		//DpBody::TS_HipsAndNeckToShouldersAndTorso,
		//DpBody::TS_HipsAndNeckToShouldersAndButt,
		//DpBody::TS_HipsAndTorsoToShouldersAndTail,
		//DpBody::TS_HipsAndTorsoToShouldersAndTorso,
		//DpBody::TS_HipsAndTorsoToShouldersAndButt,

		torsoModel = QString("%1%2.3dsN").arg(sPrefix).arg(modelName);
		modelDef.SetPrevBone(prevBone);
		modelDef.SetThisBone(thisBone);
		modelDef.SetNextBone(nextBone);
		modelDef.SetOffset(loc);
		Au3dsGLSLModelInstance* modelTorsoPiece = mc.CreateBodyModel(re, torsoModel, m_Body->m_Min, m_Body->m_Max, false, modelDef);
		if(modelTorsoPiece)
		{
			modelTorsoPiece->MergeTorsoWithInstance(modelTorso, .05*(thickNess/.875f)*m_Body->OverallScale());
			mc.DeleteModel(modelTorsoPiece);
		}
		if(!m_Body->HasBodyPowerArmorOn())
		{
			// TORSO CREST
			if(m_DNA.TorsoCrestSize > 0)
			{
				QString torsoCrest = QString("%1TorsoCrest%2%3.3dsN").arg(sPrefix).arg(m_DNA.CrestSubtype).arg(m_DNA.TorsoCrestSize);
				Au3dsGLSLModelInstance* modelTorsoCrest = mc.CreateBodyModel(re, torsoCrest, m_Body->m_Min, m_Body->m_Max, false, modelDef);
				if(modelTorsoCrest)
				{
					modelTorsoCrest->MergeTorsoWithInstance(modelTorso, .05*(thickNess/.875f)*m_Body->OverallScale(), true);
					mc.DeleteModel(modelTorsoCrest);
				}
			}
		}
		prevBone = thisBone;
		thisBone = (DhDNA::BONES)(thisBone + 1);
		loc.y -= vertlen;	// -y direction
	}
	if(modelTorso)
	{
		modelTorso->SetFileName(fileInstanceName);
		mc.CreatePatchedModel(re, modelTorso);
		m_Body->m_Models.append(modelTorso);
	}
}

AuPoint DpTorso::CurrentConnectLoc() const
{
	// Determine waist bone.
	unsigned int waist = m_DNA.WaistVertex();
	unsigned int waistBone = DhDNA::BONE_Torso0;
	if(waist)
		waistBone += (waist - 1);		// Use last bone above waist vertex.

	AuPoint curloc = m_Body->Bone(waistBone).Project(AuPoint(0,0));
	return curloc;
}

// YZ plane rotation
void DpTorso::RotateWaist(float angle)
{
	if(angle == 0.0)
		return;

	// Waist vertex must be greater than zero to bend.
	unsigned int waist = m_DNA.WaistVertex();
	if(!waist)
		return;

	// Make a matrix to rotate around current waist location.
	AuPoint pivot = CurrentConnectLoc();
	AuMatrix4x4 t;
	t.Translate(pivot);
	t.RotateX(angle);
	t.Translate(-pivot);

	// Determine last bone to rotate at waist.
	unsigned int waistBone = DhDNA::BONE_Torso0 + (waist - 1);

	// Rotate torso bones down to waist.
	for(unsigned int ib=DhDNA::BONE_Torso0; ib<=waistBone; ib++)
		m_Body->Bone(ib) = t * m_Body->Bone(ib);
	/*
#if !defined(QT_NO_DEBUG)
	for(int ib=0; ib<DhDNA::BONE_Max; ib++)
	{
		const AuMatrix4x4& bmat = m_Body->Bone(ib);
		if(bmat[3] != 0 || bmat[7] != 0 || bmat[11] != 0 || bmat[15] != 1)
		{
			qDebug("DpTorso::RotateWaist - angle %g pivot %g,%g,%g, Bad Bone Matrix %d in %s", 
					angle*RAD2DEG, pivot.x, pivot.y, pivot.z, ib,
					(const char*)m_DNA.Description().toAscii());
			bmat.Dump("DpTorso::RotateWaist Begin");
			m_DNA.Dump();
			Q_ASSERT(false);
		}
	}
#endif
*/
}

void DpTorso::MoveBy(const AuPoint& vMove)
{
	// Make a matrix to move my bones.
	AuMatrix4x4 t;
	t.Translate(vMove);

	// This logic insures that we move the Torso0 bone, even if there are no actual
	// torso vertebrae.
	unsigned int firstBone = DhDNA::BONE_Torso0;
	unsigned int lastBone = firstBone + (unsigned int)m_DNA.TorsoVertebrae;
	if(lastBone > DhDNA::BONE_Torso6)
		lastBone = DhDNA::BONE_Torso6;
	for(unsigned int ib=firstBone; ib<=lastBone; ib++)
		m_Body->Bone(ib) = t * m_Body->Bone(ib);
}

void DpTorso::RoughExtentLocs(AuPointArray& locs) const
{
	if(!m_DNA.TorsoVertebrae)
		return;	// No skin.

	// Determine head/neck point for first segment.
	float vertlen = m_Body->TorsoSegmentLength();
	int waist = m_DNA.WaistVertex();
	AuPoint loc(0, vertlen*(waist-1), 0);	// -1 due to -y direction.

	DhDNA::BONES thisBone = DhDNA::BONE_Torso0;
	for(unsigned int iv=0; iv<m_DNA.TorsoVertebrae; iv++)
	{
		// Parms appear backwards due to -y direction.
		// Test chunk intersection.
		m_Body->RoughExtentChunk(locs, thisBone, loc, vertlen);

		thisBone = (DhDNA::BONES)(thisBone + 1);

		loc.y -= vertlen;	// -y direction
	}
}

void DpTorso::RenderHitBox(AuGLEffects& re)
{
	if(!m_DNA.TorsoVertebrae)
		return;	// No skin.

	// Determine head/neck point for first segment.
	float vertlen = m_Body->TorsoSegmentLength();
	int waist = m_DNA.WaistVertex();
	AuPoint loc(0, vertlen*(waist-1), 0);	// -1 due to -y direction.

	float thick = m_Body->TorsoThickness();
	float width = m_Body->TorsoWidth();
	//float avgrad = (thick + width)/4.;
	float avgrad;
	if(thick > width)
		avgrad = thick/2.f;
	else
		avgrad = width/2.f;

	DhDNA::BONES thisBone = DhDNA::BONE_Torso0;
	for(unsigned int iv=0; iv<m_DNA.TorsoVertebrae; iv++)
	{
		// Parms appear backwards due to -y direction.
		// Test chunk intersection.
		m_Body->RenderHitBoxChunk(re, thisBone, loc, vertlen, avgrad);

		thisBone = (DhDNA::BONES)(thisBone + 1);

		loc.y -= vertlen;	// -y direction
	}
}

bool DpTorso::SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const
{
	bool hitMe = false;

	if(!m_DNA.TorsoVertebrae)
		return hitMe;	// No torso.

	// Determine head/neck point for first segment.
	float vertlen = m_Body->TorsoSegmentLength();
	int waist = m_DNA.WaistVertex();
	AuPoint loc(0, vertlen*(waist-1), 0);	// -1 due to -y direction.

	float thick = m_Body->TorsoThickness();
	float width = m_Body->TorsoWidth();
	//float avgrad = (thick + width)/4.;
	float avgrad;
	if(thick > width)
		avgrad = thick/2.f;
	else
		avgrad = width/2.f;

	DhDNA::BONES thisBone = DhDNA::BONE_Torso0;
	for(unsigned int iv=0; iv<m_DNA.TorsoVertebrae; iv++)
	{
		// Parms appear backwards due to -y direction.
		// Test chunk intersection.
		if(m_Body->SightChunk(thisBone, loc, loc + AuPoint(0,vertlen,0), avgrad, eyeLoc, sightedLoc))
		{
			sightedPart = thisBone;
			hitMe = true;
		}

		thisBone = (DhDNA::BONES)(thisBone + 1);
		loc.y -= vertlen;	// -y direction
	}

	return hitMe;
}

float DpTorso::Volume() const
{
	if(!m_DNA.TorsoVertebrae)
		return 0.;

	float vertlen = m_Body->TorsoSegmentLength();
	float thick = m_Body->TorsoThickness();
	float width = m_Body->TorsoWidth();
	float volume = width * thick * vertlen * (float)m_DNA.TorsoVertebrae;

	// Fudge factor so human comes out about right.
	volume *= 3.f;

 	//qDebug("DpTorso::Volume volume %g", volume);
	return volume;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

void DpTail::CreateSkin(AuGLEffects& re, AuGLModelCache& mc)
{
	OneTimeCalcs();
	// Butt point for origin.
	AuPoint loc = ConnectLoc();					
	AuPoint bloc(loc);

	// Skin tail bones, if there are any.
	DhDNA::BONES prevBone = DhDNA::BONE_Tail0;
	DhDNA::BONES thisBone = DhDNA::BONE_Tail0;
	DhDNA::BONES nextBone = DhDNA::BONE_Tail0;
	unsigned int tvc = m_DNA.TailVertebrae;
	unsigned int tvt = m_DNA.TailType;
	if(tvt == 0)
		return;
	QString tailModel, modelName;
	if(tvc != 0)
		modelName = QString("Tail%1").arg(tvc*2-1);
	else
		modelName = QString("Tip");
	// Tail model will come through the power armor just like wings
	tailModel = QString(":data/%1%2%3.3dsN").arg(m_DNA.SkinType).arg(modelName).arg(m_DNA.TailType);
	float length = m_DNA.Length(m_DNA.TorsoSegmentLength);
	float thickNess = m_DNA.TorsoThicknessCalc(length);
	Au3dsGLSLModelInstance *modelTailPiece;
	bool bHasArmor = false;	// Tail doesn't wear armor
	Au3dsGLSLModelDefinition modelDef(1, m_Body->OverallScale(), m_Body->m_uTOffset, m_Body->m_vTOffset,
		m_Body->m_uBOffset, m_Body->m_vBOffset, prevBone, thisBone, nextBone, bloc, 1, 1, length/1.5f, thickNess/.875f, 1,
		m_Body->m_TopColor[bHasArmor], m_Body->m_TopShininess[bHasArmor], m_Body->m_BottomColor[bHasArmor],
		m_Body->m_BottomShininess[bHasArmor], m_Body->m_Glow[bHasArmor], m_Body->m_Glowdim[bHasArmor], true);
	modelTailPiece = mc.CreateBodyModel(re, tailModel, m_Body->m_Min, m_Body->m_Max, false, modelDef);
	if(modelTailPiece)
		m_Body->m_Models.append(modelTailPiece);
}

void DpTail::OneTimeCalcs()
{
	// Connect Loc
	float vertlen = m_Body->TorsoSegmentLength();
	int waist = m_DNA.WaistVertex();
	AuPoint loc(0, -vertlen*((int)m_DNA.TorsoVertebrae-waist), 0);
	m_ConnectLoc = loc;
}

// Determine butt point for origin.
AuPoint DpTail::ConnectLoc() const
{
	return m_ConnectLoc;
}

AuPoint DpTail::CurrentConnectLoc() const
{
	AuPoint base = ConnectLoc();
	AuPoint curbase = m_Body->Bone(DhDNA::BONE_Tail0).Project(base);
	return curbase;
}

void DpTail::MoveBy(const AuPoint& vMove)
{
	// Make a matrix to move my bones.
	AuMatrix4x4 t;
	t.Translate(vMove);

	unsigned int firstBone = DhDNA::BONE_Tail0;
	unsigned int lastBone = firstBone;
	if(m_DNA.TailVertebrae)
	{
		unsigned int tailverts = (unsigned int)m_DNA.TailVertebrae*2;	// More serpentine this way.
		lastBone += tailverts - 1;
	}
	for(unsigned int ib=firstBone; ib<=lastBone; ib++)
		m_Body->Bone(ib) = t * m_Body->Bone(ib);
}

void DpTail::RoughExtentLocs(AuPointArray& locs) const
{
	// Butt point for origin.
	AuPoint loc = ConnectLoc();					
	AuPoint bloc(loc);
	float vertlen = m_Body->TorsoSegmentLength() * 2./3.;	// Tail vertebra length.
	bloc.y -= vertlen;

	// Skin tail bones, if there are any.
	DhDNA::BONES tipbone = DhDNA::BONE_Tail0;
	//DhDNA::BONES ptbone = DhDNA::BONE_Tail0;	// Bone prior to the tip bone.

	// Skin type determines size and shape of the tail chunks.
	if(m_DNA.TailVertebrae)
	{
		const int tailverts = (int)m_DNA.TailVertebrae*2;	// Tail vertebrae
		int tipverts = tailverts - 1;					// Minus one vertex for the tip.
		tipbone = (DhDNA::BONES)(DhDNA::BONE_Tail0 + tipverts);
		//ptbone = (DhDNA::BONES)(DhDNA::BONE_Tail0 + tipverts - 1);
		loc.y -= vertlen*tipverts;	// Update location for tip treatment.

		// Parms appear backward due to -y build direction.
		DhDNA::BONES thisBone = DhDNA::BONE_Tail0;
		for(int iv=0; iv<tipverts; iv++)
		{
			// Test chunk intersection.
			m_Body->RoughExtentChunk(locs, thisBone, bloc, vertlen);

			thisBone = (DhDNA::BONES)(thisBone + 1);

			bloc.y -= vertlen;	// -y direction
		}
	}

	// Apply tip treatment.
	if(m_DNA.TailType != DhDNA::TAIL_None)
	{
		switch(m_DNA.SkinType)
		{
		case DhDNA::SKIN_DryExoskeleton:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
				// Test chunk intersection.
				m_Body->RoughExtentChunk(locs, tipbone, loc - AuPoint(0,vertlen*2.5), vertlen*2.5);
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
				m_Body->RoughExtentChunk(locs, tipbone, loc - AuPoint(0,vertlen*2.), vertlen*2.);
			}
			else
			{
			}
			break;
		case DhDNA::SKIN_DryScales:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
				m_Body->RoughExtentChunk(locs, tipbone, loc - AuPoint(0,vertlen/2.), vertlen/2.);
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
			}
			else
			{
			}
			break;
		case DhDNA::SKIN_DrySkin:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
				m_Body->RoughExtentChunk(locs, tipbone, loc - AuPoint(0,vertlen/2.), vertlen/2.);
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
			}
			else
			{
			}
			break;
		case DhDNA::SKIN_Feathers:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
			}
			else
			{
			}
			break;
		case DhDNA::SKIN_Fur:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
				m_Body->RoughExtentChunk(locs, tipbone, bloc, vertlen);
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
			}
			else
			{
			}
			break;
		case DhDNA::SKIN_WetExoskeleton:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
				m_Body->RoughExtentChunk(locs, tipbone, loc - AuPoint(0,vertlen*3.), vertlen*3.);
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
				m_Body->RoughExtentChunk(locs, tipbone, loc - AuPoint(0,vertlen*2.), vertlen*2.);
			}
			else
			{
			}
			break;
		case DhDNA::SKIN_WetScales:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
			}
			else
			{
			}
			break;
		case DhDNA::SKIN_WetSkin:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
			}
			else
			{
			}
			break;
		}
	}
}

void DpTail::RenderHitBox(AuGLEffects& re)
{
	// Butt point for origin.
	AuPoint loc = ConnectLoc();					
	AuPoint bloc(loc);
	float vertlen = m_Body->TorsoSegmentLength() * 2./3.;	// Tail vertebra length.
	bloc.y -= vertlen;

	// Skin tail bones, if there are any.
	DhDNA::BONES tipbone = DhDNA::BONE_Tail0;
	//DhDNA::BONES ptbone = DhDNA::BONE_Tail0;	// Bone prior to the tip bone.

	// Skin type determines size and shape of the tail chunks.
	float endthick = m_Body->TorsoThickness();	// Ending thickness of tail at tip.
	float endwidth = m_Body->TorsoWidth();		// Ending width of tail at tip.
	// Skin the meaty chunks of the tail.
	if(m_DNA.TailVertebrae)
	{
		float thick = endthick;	// Starting thickness, at butt.
		float width = endwidth;	// Starting width, at butt.
		switch(m_DNA.SkinType)
		{
		case DhDNA::SKIN_DryExoskeleton:	// Ant.
		case DhDNA::SKIN_WetExoskeleton:	// Black widow, millipede.
			// Tail sections do not taper.
			break;
		case DhDNA::SKIN_DryScales:		// Lizard.
		case DhDNA::SKIN_WetScales:		// Fish.
			// Tail tapers from body thickness to a narrow tip.
			if(m_DNA.TailType == DhDNA::TAIL_None)
			{
				endthick /= 8.0;
				endwidth /= 8.0;
			}
			else
			{
				endthick /= 4.0;
				endwidth /= 4.0;
			}
			break;
		case DhDNA::SKIN_DrySkin:		// Pig, human.
		case DhDNA::SKIN_Feathers:		// Sparrow, ostrich, chicken.
		case DhDNA::SKIN_Fur:			// Covered with fur. Beaver, horse.
		case DhDNA::SKIN_WetSkin:		// Salamander, kardassian
			// Tail begins at a thickness considerably narrower than the body.
			// Tail tapers from body thickness to a narrow tip.
			if(m_DNA.TailType == DhDNA::TAIL_None)
			{
				thick /= 3.;
				width /= 3.;
				endthick /= 10.0;
				endwidth /= 10.0;
			}
			else
			{
				thick /= 3.;
				width /= 3.;
				endthick /= 5.0;
				endwidth /= 5.0;
			}
			break;
		}

		const int tailverts = (int)m_DNA.TailVertebrae*2;	// Tail vertebrae
		int tipverts = tailverts - 1;					// Minus one vertex for the tip.
		tipbone = (DhDNA::BONES)(DhDNA::BONE_Tail0 + tipverts);
		//ptbone = (DhDNA::BONES)(DhDNA::BONE_Tail0 + tipverts - 1);
		loc.y -= vertlen*tipverts;	// Update location for tip treatment.
		float decthick = (thick-endthick)/(float)tipverts;	// Decrement per vertex.
		float decwidth = (width-endwidth)/(float)tipverts;	// Decrement per vertex.

		// Parms appear backward due to -y build direction.
		DhDNA::BONES thisBone = DhDNA::BONE_Tail0;
		for(int iv=0; iv<tipverts; iv++)
		{
			float nthick = thick - decthick;
			float nwidth = width - decwidth;

			// Test chunk intersection.
			//float avgrad = (width + thick + nwidth + nthick)/8.;
			float avgrad;
			if(width > thick && width > nwidth && width > nthick)
				avgrad = width/2.f;
			else if(thick > nwidth && thick > nthick)
				avgrad = thick/2.f;
			else if(nwidth > nthick)
				avgrad = nwidth/2.f;
			else
				avgrad = nthick/2.f;
			m_Body->RenderHitBoxChunk(re, thisBone, bloc, vertlen, avgrad);

			thick = nthick;
			width = nwidth;
			thisBone = (DhDNA::BONES)(thisBone + 1);

			bloc.y -= vertlen;	// -y direction
		}
	}

	// Apply tip treatment.
	if(m_DNA.TailType != DhDNA::TAIL_None)
	{
		switch(m_DNA.SkinType)
		{
		case DhDNA::SKIN_DryExoskeleton:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
				// Test chunk intersection.
				//float avgrad = (endwidth + endthick)*2.5/4.;
				float avgrad;
				if(endwidth > endthick)
					avgrad = endwidth/2.f;
				else
					avgrad = endthick/2.f;
				m_Body->RenderHitBoxChunk(re, tipbone, loc - AuPoint(0,vertlen*2.5), vertlen*2.5, avgrad);
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
				//float avgrad = (endwidth + endthick)*2./4.;
				float avgrad;
				if(endwidth > endthick)
					avgrad = endwidth/2.f;
				else
					avgrad = endthick/2.f;
				m_Body->RenderHitBoxChunk(re, tipbone, loc - AuPoint(0,vertlen*2.), vertlen*2., avgrad);
			}
			else
			{
			}
			break;
		case DhDNA::SKIN_DryScales:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
				//float avgrad = (endwidth + endthick + endwidth*2. + endthick*2.)/8.;
				float avgrad;
				if(endwidth > endthick)
					avgrad = endwidth/2.f;
				else
					avgrad = endthick/2.f;
				m_Body->RenderHitBoxChunk(re, tipbone, loc - AuPoint(0,vertlen/2.), vertlen/2., avgrad);
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
			}
			else
			{
			}
			break;
		case DhDNA::SKIN_DrySkin:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
				//float avgrad = (endwidth/3. + endthick/3. + endwidth*1.5 +
						//endthick*1.5)/8.;
				float avgrad;
				if(endwidth > endthick)
					avgrad = endwidth/2.f;
				else
					avgrad = endthick/2.f;
				m_Body->RenderHitBoxChunk(re, tipbone, loc - AuPoint(0,vertlen/2.), vertlen/2., avgrad);
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
			}
			else
			{
			}
			break;
		case DhDNA::SKIN_Feathers:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
			}
			else
			{
			}
			break;
		case DhDNA::SKIN_Fur:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
				//float avgrad = (endwidth/3. + endthick/3. + endwidth/2. +
						//endthick/2.)/8.;
				float avgrad;
				if(endwidth > endthick)
					avgrad = endwidth/2.f;
				else
					avgrad = endthick/2.f;
				m_Body->RenderHitBoxChunk(re, tipbone, bloc, vertlen, avgrad);
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
			}
			else
			{
			}
			break;
		case DhDNA::SKIN_WetExoskeleton:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
				//float avgrad = (endwidth*2. + endthick*2.)/4.;
				float avgrad;
				if(endwidth > endthick)
					avgrad = endwidth/2.f;
				else
					avgrad = endthick/2.f;
				m_Body->RenderHitBoxChunk(re, tipbone, loc - AuPoint(0,vertlen*3.), vertlen*3., avgrad);
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
				//float avgrad = (endwidth*1.5 + endthick*1.5)/4.;
				float avgrad;
				if(endwidth > endthick)
					avgrad = endwidth/2.f;
				else
					avgrad = endthick/2.f;
				m_Body->RenderHitBoxChunk(re, tipbone, loc - AuPoint(0,vertlen*2.), vertlen*2., avgrad);
			}
			else
			{
			}
			break;
		case DhDNA::SKIN_WetScales:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
			}
			else
			{
			}
			break;
		case DhDNA::SKIN_WetSkin:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
			}
			else
			{
			}
			break;
		}
	}
}

bool DpTail::SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const
{
	bool hitMe = false;

	// Butt point for origin.
	AuPoint loc = ConnectLoc();					
	AuPoint bloc(loc);
	float vertlen = m_Body->TorsoSegmentLength() * 2./3.;	// Tail vertebra length.
	bloc.y -= vertlen;

	// Skin tail bones, if there are any.
	DhDNA::BONES tipbone = DhDNA::BONE_Tail0;
	//DhDNA::BONES ptbone = DhDNA::BONE_Tail0;	// Bone prior to the tip bone.

	// Skin type determines size and shape of the tail chunks.
	float endthick = m_Body->TorsoThickness();	// Ending thickness of tail at tip.
	float endwidth = m_Body->TorsoWidth();		// Ending width of tail at tip.
	// Skin the meaty chunks of the tail.
	if(m_DNA.TailVertebrae)
	{
		float thick = endthick;	// Starting thickness, at butt.
		float width = endwidth;	// Starting width, at butt.
		switch(m_DNA.SkinType)
		{
		case DhDNA::SKIN_DryExoskeleton:	// Ant.
		case DhDNA::SKIN_WetExoskeleton:	// Black widow, millipede.
			// Tail sections do not taper.
			break;
		case DhDNA::SKIN_DryScales:		// Lizard.
		case DhDNA::SKIN_WetScales:		// Fish.
			// Tail tapers from body thickness to a narrow tip.
			if(m_DNA.TailType == DhDNA::TAIL_None)
			{
				endthick /= 8.0;
				endwidth /= 8.0;
			}
			else
			{
				endthick /= 4.0;
				endwidth /= 4.0;
			}
			break;
		case DhDNA::SKIN_DrySkin:		// Pig, human.
		case DhDNA::SKIN_Feathers:		// Sparrow, ostrich, chicken.
		case DhDNA::SKIN_Fur:			// Covered with fur. Beaver, horse.
		case DhDNA::SKIN_WetSkin:		// Salamander, kardassian
			// Tail begins at a thickness considerably narrower than the body.
			// Tail tapers from body thickness to a narrow tip.
			if(m_DNA.TailType == DhDNA::TAIL_None)
			{
				thick /= 3.;
				width /= 3.;
				endthick /= 10.0;
				endwidth /= 10.0;
			}
			else
			{
				thick /= 3.;
				width /= 3.;
				endthick /= 5.0;
				endwidth /= 5.0;
			}
			break;
		}

		const int tailverts = (int)m_DNA.TailVertebrae*2;	// Tail vertebrae
		int tipverts = tailverts - 1;					// Minus one vertex for the tip.
		tipbone = (DhDNA::BONES)(DhDNA::BONE_Tail0 + tipverts);
		//ptbone = (DhDNA::BONES)(DhDNA::BONE_Tail0 + tipverts - 1);
		loc.y -= vertlen*tipverts;	// Update location for tip treatment.
		float decthick = (thick-endthick)/(float)tipverts;	// Decrement per vertex.
		float decwidth = (width-endwidth)/(float)tipverts;	// Decrement per vertex.

		// Parms appear backward due to -y build direction.
		DhDNA::BONES thisBone = DhDNA::BONE_Tail0;
		for(int iv=0; iv<tipverts; iv++)
		{
			float nthick = thick - decthick;
			float nwidth = width - decwidth;

			// Test chunk intersection.
			//float avgrad = (width + thick + nwidth + nthick)/8.;
			float avgrad;
			if(width > thick && width > nwidth && width > nthick)
				avgrad = width/2.f;
			else if(thick > nwidth && thick > nthick)
				avgrad = thick/2.f;
			else if(nwidth > nthick)
				avgrad = nwidth/2.f;
			else
				avgrad = nthick/2.f;
			if(m_Body->SightChunk(thisBone, bloc, bloc + AuPoint(0,vertlen), avgrad, eyeLoc, sightedLoc))
			{
				sightedPart = thisBone;
				hitMe = true;
			}

			thick = nthick;
			width = nwidth;
			thisBone = (DhDNA::BONES)(thisBone + 1);

			bloc.y -= vertlen;	// -y direction
		}
	}

	// Apply tip treatment.
	if(m_DNA.TailType != DhDNA::TAIL_None)
	{
		switch(m_DNA.SkinType)
		{
		case DhDNA::SKIN_DryExoskeleton:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
				// Test chunk intersection.
				//float avgrad = (endwidth + endthick)*2.5/4.;
				float avgrad;
				if(endwidth > endthick)
					avgrad = endwidth/2.f;
				else
					avgrad = endthick/2.f;
				AuPoint tloc(loc - AuPoint(0,vertlen*2.5));
				if(m_Body->SightChunk(tipbone, tloc, tloc + AuPoint(0,vertlen*2.5), avgrad, eyeLoc, sightedLoc))
				{
					sightedPart = tipbone;
					hitMe = true;
				}
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
				//float avgrad = (endwidth + endthick)*2./4.;
				float avgrad;
				if(endwidth > endthick)
					avgrad = endwidth/2.f;
				else
					avgrad = endthick/2.f;
				AuPoint tloc(loc - AuPoint(0,vertlen*2));
				if(m_Body->SightChunk(tipbone, tloc, tloc + AuPoint(0,vertlen*2.), avgrad, eyeLoc, sightedLoc))
				{
					sightedPart = tipbone;
					hitMe = true;
				}
			}
			else
			{
			}
			break;
		case DhDNA::SKIN_DryScales:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
				//float avgrad = (endwidth + endthick + endwidth*2. + endthick*2.)/8.;
				float avgrad;
				if(endwidth > endthick)
					avgrad = endwidth/2.f;
				else
					avgrad = endthick/2.f;
				AuPoint tloc(loc - AuPoint(0,vertlen/2));
				if(m_Body->SightChunk(tipbone, tloc, tloc + AuPoint(0,vertlen/2.), avgrad, eyeLoc, sightedLoc))
				{
					sightedPart = tipbone;
					hitMe = true;
				}
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
			}
			else
			{
			}
			break;
		case DhDNA::SKIN_DrySkin:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
				//float avgrad = (endwidth/3. + endthick/3. + endwidth*1.5 +
						//endthick*1.5)/8.;
				float avgrad;
				if(endwidth > endthick)
					avgrad = endwidth/2.f;
				else
					avgrad = endthick/2.f;
				AuPoint tloc(loc - AuPoint(0,vertlen/2));
				if(m_Body->SightChunk(tipbone, tloc, tloc + AuPoint(0,vertlen/2.), avgrad, eyeLoc, sightedLoc))
				{
					sightedPart = tipbone;
					hitMe = true;
				}
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
			}
			else
			{
			}
			break;
		case DhDNA::SKIN_Feathers:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
			}
			else
			{
			}
			break;
		case DhDNA::SKIN_Fur:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
				//float avgrad = (endwidth/3. + endthick/3. + endwidth/2. +
						//endthick/2.)/8.;
				float avgrad;
				if(endwidth > endthick)
					avgrad = endwidth/2.f;
				else
					avgrad = endthick/2.f;
				if(m_Body->SightChunk(tipbone, bloc, bloc + AuPoint(0,vertlen), avgrad, eyeLoc, sightedLoc))
				{
					sightedPart = tipbone;
					hitMe = true;
				}
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
			}
			else
			{
			}
			break;
		case DhDNA::SKIN_WetExoskeleton:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
				//float avgrad = (endwidth*2. + endthick*2.)/4.;
				float avgrad;
				if(endwidth > endthick)
					avgrad = endwidth/2.f;
				else
					avgrad = endthick/2.f;
				AuPoint tloc(loc - AuPoint(0,vertlen*3.));
				if(m_Body->SightChunk(tipbone, tloc, tloc + AuPoint(0,vertlen*3.), avgrad, eyeLoc, sightedLoc))
				{
					sightedPart = tipbone;
					hitMe = true;
				}
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
				//float avgrad = (endwidth*1.5 + endthick*1.5)/4.;
				float avgrad;
				if(endwidth > endthick)
					avgrad = endwidth/2.f;
				else
					avgrad = endthick/2.f;
				AuPoint tloc(loc - AuPoint(0,vertlen*2.));
				if(m_Body->SightChunk(tipbone, tloc, tloc + AuPoint(0,vertlen*2.), avgrad, eyeLoc, sightedLoc))
				{
					sightedPart = tipbone;
					hitMe = true;
				}
			}
			else
			{
			}
			break;
		case DhDNA::SKIN_WetScales:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
			}
			else
			{
			}
			break;
		case DhDNA::SKIN_WetSkin:
			if(m_DNA.TailType == DhDNA::TAIL_Primary)
			{
			}
			else if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			{
			}
			else
			{
			}
			break;
		}
	}

	return hitMe;
}

float DpTail::Volume() const
{
	if(!m_DNA.TailVertebrae)
		return 0.;

	float vertlen = m_Body->TorsoSegmentLength() * 2./3.;	// Tail vertebra length.
	float endthick = m_Body->TorsoThickness()/2.;
	float endwidth = m_Body->TorsoWidth()/2.;
	float volume = endthick * endwidth * vertlen * (float)m_DNA.TailVertebrae;

	// Fudge factor so human comes out about right.
	volume *= 1.5f;

	//qDebug("DpTail::Volume volume %g", volume);
	return volume;
}

// X angle increment up/down is per tail vertex
// Z angle increment left/right is per tail vertex
// hipHt is >= 0 if it matters, then the tail won't bend below that height.
// hipHt < 0 is ignored.
void DpTail::Bend(float angIncX, float angIncZ, float hipHt)
{
	AuPoint base = ConnectLoc();	// Base unrotated connection point of tail.
	float vertlen = m_Body->TorsoSegmentLength() * 2.f/3.f;

	unsigned int firstBone = DhDNA::BONE_Tail0;
	unsigned int lastBone = firstBone;
	if(m_DNA.TailVertebrae)
	{
		unsigned int tailverts = (unsigned int)m_DNA.TailVertebrae*2;	// More serpentine this way.
		lastBone += tailverts - 1;
		base.y -= vertlen * (tailverts-1);
	}

	AuPoint pivotbase(base);
	for(unsigned int ripple = lastBone; ripple >= firstBone; ripple--)
	{
		AuPoint pivot = m_Body->Bone(ripple).Project(pivotbase);
		AuMatrix4x4 t;
		t.Translate(pivot);
		t.RotateX(angIncX);
		t.RotateZ(angIncZ);
		t.Translate(-pivot);

		// Rotate bones at connect loc.
		for(unsigned int ib=ripple; ib<=lastBone; ib++)
			m_Body->Bone(ib) = t * m_Body->Bone(ib);

		// Move pivotbase to next tail bone position.
		pivotbase.y += vertlen;
	}

	if(hipHt >= 0)
	{
		// Bend tip back up to keep it above the ground.
		//qDebug("DpTail::Bend - %s tail", (const char*)m_DNA.Description());
		bool anyUnderGround;
		do
		{
			anyUnderGround = false;
			//qDebug("DpTail::Bend - Checking for pivot points below hipHt %g", hipHt);
			AuPoint ckbase(base);
			for(unsigned int ckp = lastBone; ckp > firstBone; ckp--)
			{
				AuPoint ckpivot = m_Body->Bone(ckp).Project(ckbase);
				ckbase.y += vertlen;

				//qDebug("DpTail::Bend - pivot %d is at %gZ", ckp, ckpivot.z);
				if(ckpivot.z < -hipHt)
				{
					// Found a pivot point that is under ground.
					//qDebug("DpTail::Bend - pivot %d is under ground", ckp);
					anyUnderGround = true;

					// Bend the entire tail up a little at every joint.
					float pivang = -1.5f*DEG2RADf;
					AuPoint pivotbase(base);
					for(unsigned int ripple = lastBone; ripple >= firstBone; ripple--)
					{
						AuPoint pivot = m_Body->Bone(ripple).Project(pivotbase);
						AuMatrix4x4 t;
						t.Translate(pivot);
						t.RotateX(pivang);
						t.Translate(-pivot);

						// Rotate bones at connect loc.
						for(unsigned int ib=ripple; ib<=lastBone; ib++)
							m_Body->Bone(ib) = t * m_Body->Bone(ib);

						// Move pivotbase to next tail bone position.
						pivotbase.y += vertlen;
					}

					// Break now because we found an underground bone in this pass and
					// dealt with it. It's time for the next 'do' pass, checking for
					// pivot points under ground.
					break;
				}
			}
		}
		while(anyUnderGround);
	}
}

// return true if this tail bends side-to-side
bool DpTail::CanBendLaterally() const
{
	if(m_DNA.TailType == DhDNA::TAIL_None)
		return false;

	switch(m_DNA.SkinType)
	{
	case DhDNA::SKIN_DryExoskeleton:
	case DhDNA::SKIN_WetExoskeleton:
		if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			return true;
		return false;
	case DhDNA::SKIN_DryScales:
		if(m_DNA.TailType == DhDNA::TAIL_Primary)
			return true;
		return false;
	case DhDNA::SKIN_DrySkin:
	case DhDNA::SKIN_Fur:
	case DhDNA::SKIN_WetScales:
		if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			return false;
		return true;
	case DhDNA::SKIN_Feathers:
	case DhDNA::SKIN_WetSkin:
		if(m_DNA.TailType == DhDNA::TAIL_Tertiary)
			return true;
		return false;
	}
	return false;
}

// return true if this tail bends up-and-down
bool DpTail::CanBendVertically() const
{
	if(m_DNA.TailType == DhDNA::TAIL_None)
		return false;

	switch(m_DNA.SkinType)
	{
	case DhDNA::SKIN_DryExoskeleton:
	case DhDNA::SKIN_WetExoskeleton:
		if(m_DNA.TailType == DhDNA::TAIL_Secondary)
			return false;
		return true;
	case DhDNA::SKIN_DryScales:
	case DhDNA::SKIN_DrySkin:
	case DhDNA::SKIN_Feathers:
	case DhDNA::SKIN_Fur:
		return true;
	case DhDNA::SKIN_WetScales:
		if(m_DNA.TailType == DhDNA::TAIL_Primary)
			return false;
		return true;
	case DhDNA::SKIN_WetSkin:
		if(m_DNA.TailType == DhDNA::TAIL_Tertiary)
			return false;
		return true;
	}
	return false;
}

// Fold tail for sitting on the ground or straddling another creature, if possible.
void DpTail::Fold()
{
	if(!m_DNA.TailVertebrae)
		return;

	unsigned int tailverts = (unsigned int)m_DNA.TailVertebrae*2;	// More serpentine this way.
	float anginc = 45.f*DEG2RADf / (float)tailverts;
	if(anginc == 0)
		return;

	float angincx = 0;	// x angle increment (up/down)
	float angincz = 0;	// z angle increment (left/right)
	if(CanBendLaterally())
	{
		if(m_Body->IsLeftHanded())
			angincz = -anginc;	// bend laterally
		else
			angincz = anginc;	// bend laterally
	}
	else
		angincx = -anginc;	// bend vertically

	AuPoint base = ConnectLoc();	// Base unrotated connection point of tail.
	float vertlen = m_Body->TorsoSegmentLength() * 2.f/3.f;

	unsigned int firstBone = DhDNA::BONE_Tail0;
	unsigned int lastBone = firstBone;
	if(m_DNA.TailVertebrae)
	{
		unsigned int tailverts = (unsigned int)m_DNA.TailVertebrae*2;	// More serpentine this way.
		lastBone += tailverts - 1;
		base.y -= vertlen * (tailverts-1);
	}

	for(unsigned int ripple = lastBone; ripple >= firstBone; ripple--)
	{
		// Bend base of tail at 90 degree angle...ouch.
		if(ripple == firstBone)
		{
			if(CanBendLaterally())
			{
				if(m_Body->IsLeftHanded())
					angincz = -PIf/2.f;	// bend laterally
				else
					angincz = PIf/2.f;	// bend laterally
			}
			else
				angincx = -PIf/2.f;	// bend vertically
		}

		AuPoint pivot = m_Body->Bone(ripple).Project(base);
		AuMatrix4x4 t;
		t.Translate(pivot);
		t.RotateX(angincx);
		t.RotateZ(angincz);
		t.Translate(-pivot);

		// Rotate bones at connect loc.
		for(unsigned int ib=ripple; ib<=lastBone; ib++)
			m_Body->Bone(ib) = t * m_Body->Bone(ib);

		// Move base to next tail bone position.
		base.y += vertlen;
	}
}

void DpTail::Lash(float hipHt, float speedEnc)
{
	if(!m_DNA.TailVertebrae)
		return;

	unsigned int tailverts = (unsigned int)m_DNA.TailVertebrae*2;	// More serpentine this way.
	float persin = m_DNA.Rhythm(AuPose::Lash, PIf, speedEnc);
	float anginc = 30.f*DEG2RADf / (float)tailverts * persin;
	if(anginc == 0)
		return;

	float angincx = 0;	// x angle increment (up/down)
	float angincz = 0;	// z angle increment (left/right)
	if(CanBendLaterally())
		angincz = anginc;	// bend laterally
	else
		angincx = anginc;	// bend vertically

	// When legless and not flying or swimming, don't wave tail downward very
	// much.
	//if(!m_DNA.LegPairs && angincx > 0 &&
			//!m_Body->IsFlyingPose() && !m_Body->IsSwimmingPose())
		//angincx = angincx/10.;

	Bend(angincx, angincz, hipHt);
}

void DpTail::DeadFacingDown()
{
	if(CanBendLaterally())
	{
		float anginc = 15.f*DEG2RADf;
		Slither(0, anginc);
	}
}

void DpTail::DeadFacingLeft()
{
	if(CanBendVertically())
	{
		float anginc = 15.f*DEG2RADf;
		Slither(anginc, 0);
	}
}

void DpTail::DeadFacingRight()
{
	if(CanBendVertically())
	{
		float anginc = 15.f*DEG2RADf;
		Slither(anginc, 0);
	}
}

void DpTail::DeadFacingUp()
{
	if(CanBendLaterally())
	{
		float anginc = 15.f*DEG2RADf;
		Slither(0, anginc);
	}
}

void DpTail::Relax(float hipHt)
{
	if(CanBendVertically() && hipHt > 0)
	{
		// Tail bends vertically and I'm above the ground. Hang with gravity.
		float anginc = 15.f*DEG2RADf;
		Bend(anginc, 0, hipHt);
	}
	else if(CanBendLaterally())
	{
		// Tail cannot bend vertically or I'm on the ground.
		float anginc = 10.f*DEG2RADf;
		Slither(0, anginc);
	}
}

// 'speed' 0=stopped, 1=normal, 2=fast
void DpTail::Slither(const AuPose& pose, float speedEnc)
{
	if(!m_DNA.TailVertebrae)
		return;

	float anginc;
	if(DpBody::StationaryPose(pose, m_DNA) == pose)
	{
		// Stationary tail.
		if(CanBendLaterally())
			anginc = 35.f*DEG2RADf;	// maximum slither
		else
			anginc = 0;				// Tail is straight out behind.
	}
	else
		anginc = 30.f*DEG2RADf * m_DNA.Rhythm(pose, PIf, speedEnc);
	if(anginc == 0)
		return;

	float angincx = 0;	// x angle increment (up/down)
	float angincz = 0;	// z angle increment (left/right)
	if(m_Body->IsFlyingPose() || m_Body->IsSwimmingPose())
	{
		// In the air or water so prefer vertical slither.
		if(CanBendVertically())
			angincz = anginc;	// slither down
		else
			angincx = anginc;	// slither laterally
	}
	else if(CanBendLaterally())
		angincz = anginc;	// On the ground and tail can bend laterally.
	else
		return;	// Cannot bend vertically on the ground.

	Slither(angincx, angincz);
}

// Z angle increment left/right is per tail vertex
void DpTail::Slither(float angIncX, float angIncZ)
{
	AuPoint base = ConnectLoc();	// Base unrotated connection point of tail.
	float vertlen = m_Body->TorsoSegmentLength() * 2.f/3.f;

	unsigned int firstBone = DhDNA::BONE_Tail0;
	unsigned int lastBone = firstBone;
	if(m_DNA.TailVertebrae)
	{
		unsigned int tailverts = (unsigned int)m_DNA.TailVertebrae*2;	// More serpentine this way.
		lastBone += tailverts - 1;
		base.y -= vertlen * (tailverts-1);
	}

	unsigned int middleBone = (firstBone + lastBone)/2;
	for(unsigned int ripple = lastBone; ripple >= firstBone; ripple--)
	{
		if(ripple == firstBone)
		{
			angIncX /= 2.f;
			angIncZ /= 2.f;
		}
		else if(ripple == middleBone)
		{
			angIncX = -angIncX;
			angIncZ = -angIncZ;
		}

		AuPoint pivot = m_Body->Bone(ripple).Project(base);
		AuMatrix4x4 t;
		t.Translate(pivot);
		t.RotateX(angIncX);
		t.RotateZ(angIncZ);
		t.Translate(-pivot);

		// Rotate bones at connect loc.
		for(unsigned int ib=ripple; ib<=lastBone; ib++)
			m_Body->Bone(ib) = t * m_Body->Bone(ib);

		// Move base to next tail bone position.
		base.y += vertlen;
	}
}

void DpTail::Strike(float hipHt)
{
	if(!m_DNA.TailVertebrae)
		return;

	unsigned int tailverts = (unsigned int)m_DNA.TailVertebrae*2;	// More serpentine this way.
	float persin = m_DNA.Rhythm(AuPose::Strike, PIf, 1);
	float angle = (60.f + 70.f*persin)*DEG2RADf;
	float anginc = angle / (float)tailverts;
	if(anginc == 0)
		return;

	float angincx = 0;	// x angle increment (up/down)
	float angincz = 0;	// z angle increment (left/right)
	if(m_Body->IsFlyingPose() || m_Body->IsSwimmingPose())
	{
		// Strike downward if possible.
		if(CanBendVertically())
			angincz = anginc;	// bend down
		else
			angincx = anginc;	// bend laterally
	}
	else if(CanBendLaterally())
		angincz = anginc;	// bend laterally
	else
		angincx = -anginc;	// bend up

	Bend(angincx, angincz, hipHt);
}

void DpTail::Swim(const AuPose& poseRhythm)
{
	if(!m_DNA.TailVertebrae)
		return;

	unsigned int tailverts = (unsigned int)m_DNA.TailVertebrae*2;	// More serpentine this way.
	float persin = m_DNA.Rhythm(poseRhythm, PIf, 1);
	float anginc = 20.f*DEG2RADf / (float)tailverts * persin;
	if(anginc == 0)
		return;

	float angincx = 0;	// x angle increment (up/down)
	float angincz = 0;	// z angle increment (left/right)
	if(CanBendLaterally())
		angincz = anginc;	// bend laterally
	else
		angincx = anginc;	// bend vertically

	Bend(angincx, angincz, -1);	// -1 to ignore hipHt when bending tail
}

void DpTail::Wag(float hipHt)
{
	if(!m_DNA.TailVertebrae)
		return;

	unsigned int tailverts = (unsigned int)m_DNA.TailVertebrae*2;	// More serpentine this way.
	float persin = m_DNA.Rhythm(AuPose::Wag, PIf, 1);
	float anginc = 15.f*DEG2RADf / (float)tailverts * persin;
	if(anginc == 0)
		return;

	float angincx = 0;	// x angle increment (up/down)
	float angincz = 0;	// z angle increment (left/right)
	if(CanBendLaterally() && CanBendVertically())
	{
		angincx = -15.f*DEG2RADf;	// Lift tail for wagging.
		angincz = anginc;			// wag laterally
	}
	else if(CanBendLaterally())
		angincz = anginc;			// wag laterally
	else
		angincx = anginc;			// wag vertically

	// When legless and not flying or swimming, don't wave tail downward very
	// much.
	//if(!m_DNA.LegPairs && angincx > 0 &&
			//!m_Body->IsFlyingPose() && !m_Body->IsSwimmingPose())
		//angincx = angincx/10.;

	Bend(angincx, angincz, hipHt);
}

void DpTail::Wave(float hipHt, float speedEnc)
{
	if(!m_DNA.TailVertebrae)
		return;

	unsigned int tailverts = (unsigned int)m_DNA.TailVertebrae*2;	// More serpentine this way.
	float persin = m_DNA.Rhythm(m_Body->CurrentGeneralPose(), PIf, speedEnc);
	float anginc = 12.f*DEG2RADf / (float)tailverts * persin;
	if(anginc == 0)
		return;

	float angincx = 0;	// x angle increment (up/down)
	float angincz = 0;	// z angle increment (left/right)
	if(CanBendLaterally())
		angincz = anginc;			// wave laterally
	else
		angincx = anginc;			// wave vertically

	// When legless and not flying or swimming, don't wave tail downward very
	// much.
	//if(!m_DNA.LegPairs && angincx > 0 &&
			//!m_Body->IsFlyingPose() && !m_Body->IsSwimmingPose())
		//angincx = angincx/10.;

	Bend(angincx, angincz, hipHt);
}

void DpTail::WritheFacingDown()
{
	if(CanBendLaterally())
	{
		float anginc =
			(15.f + 1.5f*m_DNA.Rhythm(AuPose::WritheFaceDown, 0, 1))*DEG2RADf;
		Slither(0, anginc);
	}
}

void DpTail::WritheFacingLeft()
{
	if(CanBendVertically())
	{
		float anginc =
			(15.f + 1.5f*m_DNA.Rhythm(AuPose::WritheFaceLeft, 0, 1))*DEG2RADf;
		Slither(anginc, 0);
	}
}

void DpTail::WritheFacingRight()
{
	if(CanBendVertically())
	{
		float anginc =
			(15.f + 1.5f*m_DNA.Rhythm(AuPose::WritheFaceRight, 0, 1))*DEG2RADf;
		Slither(anginc, 0);
	}
}

void DpTail::WritheFacingUp()
{
	if(CanBendLaterally())
	{
		float anginc =
			(15.f + 1.5f*m_DNA.Rhythm(AuPose::WritheFaceUp, 0, 1))*DEG2RADf;
		Slither(0, anginc);
	}
}

void DpTail::ReceptiveFacingUp()
{
	if(CanBendLaterally())
	{
		float anginc =
			(15.f + 1.5f*m_DNA.Rhythm(AuPose::ReceptiveMate, 0, 1))*DEG2RADf;
		Slither(0, anginc);
	}
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

void DpHeadAndNeck::CreateSkin(AuGLEffects& re, AuGLModelCache& mc)
{
	OneTimeCalcs();
	// Determine neck point for origin.
	AuPoint headLoc = HeadLoc();
	DhDNA::BONES prevBone = DhDNA::BONE_Head0;
	DhDNA::BONES thisBone = DhDNA::BONE_Head0;
	DhDNA::BONES nextBone = DhDNA::BONE_Head0;
	if(m_Which == 1)
	{
		prevBone = DhDNA::BONE_Head1;
		thisBone = DhDNA::BONE_Head1;
		nextBone = DhDNA::BONE_Head1;
	}
	float length = m_DNA.Length(m_DNA.TorsoSegmentLength);
	float thickNess = m_DNA.TorsoThicknessCalc(length);

	// Try and get previous head model arrangement if it exists
	bool bHasArmor = m_Body->HasHelmetPowerArmorOn();
	Au3dsGLSLModelDefinition modelDef(1, m_Body->OverallScale(), m_Body->m_uTOffset, m_Body->m_vTOffset,
		m_Body->m_uBOffset, m_Body->m_vBOffset, prevBone, thisBone, nextBone, headLoc, 1, 1, length/1.5f, thickNess/.875f, 1,
		m_Body->m_TopColor[bHasArmor], m_Body->m_TopShininess[bHasArmor], m_Body->m_BottomColor[bHasArmor],
		m_Body->m_BottomShininess[bHasArmor], m_Body->m_Glow[bHasArmor], m_Body->m_Glowdim[bHasArmor], true, true, true);
	QString sEyePiece = QObject::tr("%1%2").arg(m_DNA.EyeType).arg(m_DNA.EyeSize+1);
	QString sEarPiece = QObject::tr("%1%2").arg(m_DNA.EarType).arg(m_DNA.EarSize+1);
	QString sNosePiece = QObject::tr("%1%2").arg(m_DNA.NoseType).arg(m_DNA.NoseSize+1);
	QString sMouthPiece = QObject::tr("%1%2").arg(m_DNA.MouthType).arg(m_DNA.MouthSize+1);
	QString sPrefix = m_Body->GetHelmetModelPrefix();
	QString sSkullPrefix = sPrefix;
	if(sPrefix.contains("PA"))
		sSkullPrefix += QString("%1").arg(m_DNA.SkinType);
	QString sFileName = QObject::tr("%1Head%2%3%4%5").arg(sSkullPrefix).arg(sEyePiece).arg(sEarPiece).arg(sNosePiece).arg(sMouthPiece);
	if(m_DNA.HornSize > 0)
		sFileName += QObject::tr("HS%1%2").arg(m_DNA.HornBranches).arg(m_DNA.HornSize);
	if(m_DNA.HeadCrestSize > 0)
		sFileName += QObject::tr("HC%1%2").arg(m_DNA.CrestSubtype).arg(m_DNA.HeadCrestSize);
	Au3dsGLSLModelInstance *modelHeadPiece;
	modelHeadPiece = mc.CreateBodyModel(re, sFileName, m_Body->m_Min, m_Body->m_Max, false, modelDef);
	Au3dsGLSLModelInstance *modelEyePos = modelHeadPiece;
	if(modelHeadPiece)
	{
		if(m_Body->HasHelmetPowerArmorOn())
		{
			// Armor model head piece has been loaded.  Load default head model
			// to get eye position so that the camera doesn't change when putting it on.
			QString skullModel = QString(":data/%1Skull.3dsN").arg(m_DNA.SkinType);
			modelDef.m_bArrangement = false;
			Au3dsGLSLModelInstance *modelHead = mc.CreateBodyModel(re, skullModel, m_Body->m_Min, m_Body->m_Max, false, modelDef);
			modelEyePos = modelHead;
		}
		if(modelEyePos)
			m_Body->m_eyePos = modelEyePos->GetEyePos();
		m_Body->m_Models.append(modelHeadPiece);
	}
	else
	{
		// Arrangement not found. Set to false so that CreateBodyModel knows that
		// we are not expecting it so that the pieces get created.
		modelDef.m_bArrangement = false;
		// Create head model
		Au3dsGLSLModelInstance *modelSkullPiece;
		Au3dsGLSLModelInstance *modelEyePiece;
		Au3dsGLSLModelInstance *modelNosePiece;
		Au3dsGLSLModelInstance *modelMouthPiece;
		Au3dsGLSLModelInstance *modelEarPiece;
		QString headModel, eyeModel, earModel, noseModel, mouthModel;
		headModel = QString("%1Skull.3dsN").arg(sSkullPrefix);
		eyeModel = QString("%1Eye%2%3.3dsN").arg(sPrefix).arg(m_DNA.EyeType).arg(m_DNA.EyeSize+1);
		earModel = QString("%1Ear%2%3.3dsN").arg(sPrefix).arg(m_DNA.EarType).arg(m_DNA.EarSize+1);
		noseModel = QString("%1Nose%2%3.3dsN").arg(sPrefix).arg(m_DNA.NoseType).arg(m_DNA.NoseSize+1);
		mouthModel = QString("%1Mouth%2%3.3dsN").arg(sPrefix).arg(m_DNA.MouthType).arg(m_DNA.MouthSize+1);
		// HEAD
		modelSkullPiece = mc.CreateBodyModel(re, headModel, m_Body->m_Min, m_Body->m_Max, false, modelDef);
		modelEyePos = modelSkullPiece;
		if(m_Body->HasHelmetPowerArmorOn())
		{
			// Armor model head piece has been loaded.  Load default head model
			// to get eye position so that the camera doesn't change when putting it on.
			headModel = QString(":data/%1Skull.3dsN").arg(m_DNA.SkinType);
			Au3dsGLSLModelInstance *modelHead = mc.CreateBodyModel(re, headModel, m_Body->m_Min, m_Body->m_Max, false, modelDef);
			modelEyePos = modelHead;
		}
		if(modelEyePos)
			m_Body->m_eyePos = modelEyePos->GetEyePos();
		bool bMerged = true;
		if(modelSkullPiece)
		{
			modelSkullPiece->MergeHeadWithInstance(modelHeadPiece, .001*(thickNess/.875f)*m_Body->OverallScale());
			mc.DeleteModel(modelSkullPiece);
		}
		if(!m_Body->HasHelmetPowerArmorOn() && modelHeadPiece)
		{
			// EYES
			if(!(m_DNA.SkinType == DhDNA::SKIN_WetExoskeleton && m_DNA.EyeType == DhDNA::EYE_NotApparent))
			{
				modelEyePiece = mc.CreateBodyModel(re, eyeModel, m_Body->m_Min, m_Body->m_Max, false, modelDef);
				if(modelEyePiece)
				{
					bMerged = modelEyePiece->MergeHeadWithInstance(modelHeadPiece, .001*(thickNess/.875f)*m_Body->OverallScale());
					mc.DeleteModel(modelEyePiece);
				}
			}
			if(!bMerged)
				qDebug("Failed to properly merge head piece Skull:%d Eye:%d%d", m_DNA.SkinType, m_DNA.EyeType, m_DNA.EyeSize);
			bMerged = true;
			// EARS
			if(!(m_DNA.SkinType == DhDNA::SKIN_WetExoskeleton && m_DNA.EarType == DhDNA::EAR_NotApparent))
			{
				modelEarPiece = mc.CreateBodyModel(re, earModel, m_Body->m_Min, m_Body->m_Max, false, modelDef);
				if(modelEarPiece)
				{
					bMerged = modelEarPiece->MergeHeadWithInstance(modelHeadPiece, .001*(thickNess/.875f)*m_Body->OverallScale());
					mc.DeleteModel(modelEarPiece);
				}
			}
			if(!bMerged)
				qDebug("Failed to properly merge head piece Skull:%d Eye:%d%d Ear:%d%d",
				m_DNA.SkinType, m_DNA.EyeType, m_DNA.EyeSize, m_DNA.EarType, m_DNA.EarSize);
			bMerged = true;
			// NOSE
			if(!(m_DNA.SkinType == DhDNA::SKIN_WetExoskeleton && m_DNA.NoseType == DhDNA::NOSE_NotApparent))
			{
				modelNosePiece = mc.CreateBodyModel(re, noseModel, m_Body->m_Min, m_Body->m_Max, false, modelDef);
				if(modelNosePiece)
				{
					bMerged = modelNosePiece->MergeHeadWithInstance(modelHeadPiece, .001*(thickNess/.875f)*m_Body->OverallScale());
					mc.DeleteModel(modelNosePiece);
				}
			}
			if(!bMerged)
				qDebug("Failed to properly merge head piece Skull:%d Eye:%d%d Ear:%d%d Nose:%d%d",
				m_DNA.SkinType, m_DNA.EyeType, m_DNA.EyeSize, m_DNA.EarType, m_DNA.EarSize, m_DNA.NoseType, m_DNA.NoseSize);
			bMerged = true;
			// MOUTH
			if(!(m_DNA.SkinType == DhDNA::SKIN_WetExoskeleton && m_DNA.MouthType == DhDNA::MOUTH_NotApparent))
			{
				modelMouthPiece = mc.CreateBodyModel(re, mouthModel, m_Body->m_Min, m_Body->m_Max, false, modelDef);
				if(modelMouthPiece)
				{
					bMerged = modelMouthPiece->MergeHeadWithInstance(modelHeadPiece, .001*(thickNess/.875f)*m_Body->OverallScale());
					mc.DeleteModel(modelMouthPiece);
				}
			}
			if(!bMerged)
				qDebug("Failed to properly merge head piece Skull:%d Eye:%d%d Ear:%d%d Nose:%d%d Mouth:%d%d",
				m_DNA.SkinType, m_DNA.EyeType, m_DNA.EyeSize, m_DNA.EarType, m_DNA.EarSize, m_DNA.NoseType, m_DNA.NoseSize,
				m_DNA.MouthType, m_DNA.MouthSize);
			bMerged = true;
			// HORNS
			if(m_DNA.HornSize > 0)
			{
				Au3dsGLSLModelInstance *modelHornPiece;
				QString hornModel;
				hornModel = QString("%1Horn%2%3.3dsN").arg(sPrefix).arg(m_DNA.HornBranches).arg(m_DNA.HornSize);
				modelHornPiece = mc.CreateBodyModel(re, hornModel, m_Body->m_Min, m_Body->m_Max, false, modelDef);
				if(modelHornPiece)
				{
					bMerged = modelHornPiece->MergeHeadWithInstance(modelHeadPiece, .001*(thickNess/.875f)*m_Body->OverallScale());
					mc.DeleteModel(modelHornPiece);
				}

			}
			if(!bMerged)
				qDebug("Failed to properly merge head piece Skull:%d Eye:%d%d Ear:%d%d Nose:%d%d Mouth:%d%d Horns:%d%d",
				m_DNA.SkinType, m_DNA.EyeType, m_DNA.EyeSize, m_DNA.EarType, m_DNA.EarSize, m_DNA.NoseType, m_DNA.NoseSize,
				m_DNA.MouthType, m_DNA.MouthSize, m_DNA.CrestSubtype, m_DNA.HornSize);
			bMerged = true;
			// HEADCREST
			if(m_DNA.HeadCrestSize > 0)
			{
				Au3dsGLSLModelInstance *modelHeadCrest;
				QString headCrestModel;
				headCrestModel = QString("%1HeadCrest%2%3.3dsN").arg(sPrefix).arg(m_DNA.CrestSubtype).arg(m_DNA.HeadCrestSize);
				modelHeadCrest = mc.CreateBodyModel(re, headCrestModel, m_Body->m_Min, m_Body->m_Max, false, modelDef);
				if(modelHeadCrest)
				{
					bMerged = modelHeadCrest->MergeHeadWithInstance(modelHeadPiece, .001*(thickNess/.875f)*m_Body->OverallScale());
					mc.DeleteModel(modelHeadCrest);
				}
			}
			if(!bMerged)
				qDebug("Failed to properly merge head piece Skull:%d Eye:%d%d Ear:%d%d Nose:%d%d Mouth:%d%d Horns:%d%d HeadCrest:%d",
				m_DNA.SkinType, m_DNA.EyeType, m_DNA.EyeSize, m_DNA.EarType, m_DNA.EarSize, m_DNA.NoseType, m_DNA.NoseSize,
				m_DNA.MouthType, m_DNA.MouthSize, m_DNA.CrestSubtype, m_DNA.HornSize, m_DNA.HeadCrestSize);
		}

		// Patch the head piece
		if(modelHeadPiece)
		{
			// Set file name to head configuration
			modelHeadPiece->SetFileName(sFileName);
			mc.CreatePatchedModel(re, modelHeadPiece);
			m_Body->m_Models.append(modelHeadPiece);
		}
	}

	// NECK
	if(m_DNA.Heads == 1 && m_Which != 1)
		return;
	Au3dsGLSLModelInstance *modelNeckPiece;
	unsigned int nvc = m_DNA.NeckVertebraeCount();
	prevBone = DhDNA::BONE_Neck00;
	thisBone = DhDNA::BONE_Neck00;
	nextBone = DhDNA::BONE_Neck00;
	if(nvc == 2)
	{
		prevBone = DhDNA::BONE_Neck00;
		thisBone = DhDNA::BONE_Neck00;
		nextBone = DhDNA::BONE_Neck01;
	}
	else if(nvc == 3)
	{
		prevBone = DhDNA::BONE_Neck00;
		thisBone = DhDNA::BONE_Neck01;
		nextBone = DhDNA::BONE_Neck02;
	}
	else if(nvc == 0)
		return;
	AuPoint segloc(headLoc);
	float vertlen = m_Body->TorsoSegmentLength() / 2.5;
	segloc.y -= vertlen*nvc;					// Drawing last neck segment from the bottom before head
	QString neckModel;
	neckModel = QString("%1Neck%2%3.3dsN").arg(sPrefix).arg(nvc).arg(m_Which+1);
	modelDef.SetOffset(segloc);
	modelDef.SetPrevBone(prevBone);
	modelDef.SetThisBone(thisBone);
	modelDef.SetThisBone(nextBone);
	modelDef.Skip(false);
	modelNeckPiece = mc.CreateBodyModel(re, neckModel, m_Body->m_Min, m_Body->m_Max, false, modelDef);
	if(modelNeckPiece)
		m_Body->m_Models.append(modelNeckPiece);
}

// Do calculations that only need to happen once
void DpHeadAndNeck::OneTimeCalcs()
{
	// Connect Loc
	float vertlen = m_Body->TorsoSegmentLength();
	int waist = m_DNA.WaistVertex();
	AuPoint loc(0, vertlen*waist, 0);
	m_ConnectLoc = loc;

	// Head Loc
	AuPoint headLoc = m_ConnectLoc;
	if(m_DNA.NeckVertebraeCount())
	{
		// Neck vertebrae are shorter than torso vertebrae.
		float vertlen = m_Body->TorsoSegmentLength() / 2.5;
		float necklen = vertlen * (float)m_DNA.NeckVertebraeCount();
		headLoc.y += necklen;
		headLoc.y -= vertlen/4.0;	// Sink into body slightly.
	}
	m_HeadLoc = headLoc;
}

AuPoint DpHeadAndNeck::ConnectLoc() const
{
	return m_ConnectLoc;
}

AuPoint DpHeadAndNeck::CurrentConnectLoc() const
{
	unsigned int connectBone;
	switch(m_Which)
	{
	default:
	case 0:
		connectBone = DhDNA::BONE_Head0;
		break;
	case 1:
		connectBone = DhDNA::BONE_Head1;
		break;
	}
	connectBone += m_DNA.NeckVertebraeCount();

	AuPoint base = ConnectLoc();
	AuPoint curbase = m_Body->Bone(connectBone).Project(base);
	return curbase;
}

AuPoint DpHeadAndNeck::HeadLoc() const
{
	return m_HeadLoc;
}

AuPoint DpHeadAndNeck::CurrentHeadLoc() const
{
	// Determine head center point.
	AuPoint headLoc = HeadLoc();

	// Bone matrix determines its current position.
	DhDNA::BONES headBone;
	switch(m_Which)
	{
	default:
	case 0:
		headBone = DhDNA::BONE_Head0;
		break;
	case 1:
		headBone = DhDNA::BONE_Head1;
		break;
	}

	headLoc = m_Body->Bone(headBone).Project(headLoc);
	return headLoc;
}

AuPoint DpHeadAndNeck::CurrentCameraLoc() const
{
	// Determine head center point and add eye pos offset
	// Eye pos offset is NOT added on server, which does not load head geometry. This way aiming works properly based on head
	// location, not eye location. This inconsistency must be considered when using camera location on the client and on the
	// server. Do not expect them to be the same.
	AuPoint headLoc = HeadLoc();
	if(!m_Body->m_eyePos.IsZero())
		headLoc += AuPoint(0, m_Body->m_eyePos.y, m_Body->m_eyePos.z);

	// Bone matrix determines its current position.
	DhDNA::BONES headBone;
	switch(m_Which)
	{
	default:
	case 0:
		headBone = DhDNA::BONE_Head0;
		break;
	case 1:
		headBone = DhDNA::BONE_Head1;
		break;
	}

	headLoc = m_Body->Bone(headBone).Project(headLoc);
	return headLoc;
}

void DpHeadAndNeck::MoveBy(const AuPoint& vMove)
{
	if(vMove.IsZero())
		return;

	unsigned int firstBone;
	switch(m_Which)
	{
	default:
	case 0:
		firstBone = DhDNA::BONE_Head0;
		break;
	case 1:
		firstBone = DhDNA::BONE_Head1;
		break;
	}
	unsigned int lastBone = firstBone + m_DNA.NeckVertebraeCount();

	// Make a matrix to move my bones.
	AuMatrix4x4 t;
	t.Translate(vMove);
	for(unsigned int ib=firstBone; ib<=lastBone; ib++)
		m_Body->Bone(ib) = t * m_Body->Bone(ib);
}

// Rotate each neck bone around X. Positive moves head front to back.
void DpHeadAndNeck::BendNeck(float angle)
{
	if(angle == 0.0)
		return;

	unsigned int firstBone;
	switch(m_Which)
	{
	default:
	case 0:
		firstBone = DhDNA::BONE_Head0;
		break;
	case 1:
		firstBone = DhDNA::BONE_Head1;
		break;
	}
	unsigned int lastBone = firstBone + m_DNA.NeckVertebraeCount();

	// Neck vertebrae are shorter than torso vertebrae.
	float vertlen = m_Body->TorsoSegmentLength() / 2.0;
	AuPoint base = ConnectLoc();	// Base unrotated connection point of neck.
	for(unsigned int ripple = lastBone; ripple >= firstBone; ripple--)
	{
		// Make a matrix to rotate around current connect location.
		AuPoint pivot = m_Body->Bone(ripple).Project(base);
		AuMatrix4x4 t;
		t.Translate(pivot);
		t.RotateX(angle);
		t.Translate(-pivot);

		// Rotate bones at connect loc.
		for(unsigned int ib=firstBone; ib<=ripple; ib++)
			m_Body->Bone(ib) = t * m_Body->Bone(ib);

		// Move base to next neck bone position.
		base.y += vertlen;
	}
}

// Rotation around Y axis.
void DpHeadAndNeck::LeanNeck(float angle)
{
	if(angle == 0.0)
		return;

	unsigned int firstBone;
	switch(m_Which)
	{
	default:
	case 0:
		firstBone = DhDNA::BONE_Head0;
		break;
	case 1:
		firstBone = DhDNA::BONE_Head1;
		break;
	}
	unsigned int lastBone = firstBone + m_DNA.NeckVertebraeCount();

	// Make a matrix to rotate around current connect location.
	AuPoint pivot = CurrentConnectLoc();
	AuMatrix4x4 t;
	t.Translate(pivot);
	t.RotateY(angle);
	t.Translate(-pivot);

	// Rotate bones at connect loc.
	for(unsigned int ib=firstBone; ib<=lastBone; ib++)
		m_Body->Bone(ib) = t * m_Body->Bone(ib);
}

// Rotation around X axis.
void DpHeadAndNeck::RotateNeck(float angle)
{
	if(angle == 0.0)
		return;

	unsigned int firstBone;
	switch(m_Which)
	{
	default:
	case 0:
		firstBone = DhDNA::BONE_Head0;
		break;
	case 1:
		firstBone = DhDNA::BONE_Head1;
		break;
	}
	unsigned int lastBone = firstBone + m_DNA.NeckVertebraeCount();

	// Make a matrix to rotate around current connect location.
	AuPoint pivot = CurrentConnectLoc();
	AuMatrix4x4 t;
	t.Translate(pivot);
	t.RotateX(angle);
	t.Translate(-pivot);

	// Rotate bones at connect loc.
	for(unsigned int ib=firstBone; ib<=lastBone; ib++)
		m_Body->Bone(ib) = t * m_Body->Bone(ib);

	/*
#if !defined(QT_NO_DEBUG)
	for(int ib=0; ib<DhDNA::BONE_Max; ib++)
	{
		const AuMatrix4x4& bmat = m_Body->Bone(ib);
		if(bmat[3] != 0 || bmat[7] != 0 || bmat[11] != 0 || bmat[15] != 1)
		{
			qDebug("DpHeadAndNeck::RotateNeck - angle %g pivot %g,%g,%g, Bad Bone Matrix %d in %s", 
					angle*RAD2DEG, pivot.x, pivot.y, pivot.z, ib,
					(const char*)m_DNA.Description().toAscii());
			bmat.Dump(0);
		}
	}
#endif
	*/
}

// Straighten out all bones.
void DpHeadAndNeck::Straighten()
{
	unsigned int firstBone;
	switch(m_Which)
	{
	default:
	case 0:
		firstBone = DhDNA::BONE_Head0;
		break;
	case 1:
		firstBone = DhDNA::BONE_Head1;
		break;
	}
	unsigned int lastBone = firstBone + m_DNA.NeckVertebraeCount();
	for(unsigned int ib=firstBone; ib<=lastBone; ib++)
		m_Body->Bone(ib).LoadIdentity();
}

void DpHeadAndNeck::TiltHeadTo(float tiltAngle, float turnAngle)
{
	// Straighten necks and then bend them to the desired angle.
	AuPoint oloc = CurrentConnectLoc();
	Straighten();
	AuPoint nloc = CurrentConnectLoc();
	MoveBy(oloc-nloc);	// Move neck back to attachment point.

	// Bend the neck and head appropriately.
	AuPoint backvec = AuPoint(0,1);	// Default if we don't have a spine.
	if(m_DNA.TorsoVertebrae)
	{
		// There is at least one body vertebrae so use the first one to
		// determine the neck "unbent" angle.
		backvec = m_Body->Bone(DhDNA::BONE_Torso0).ProjectVector(backvec);
	}
	backvec.Unit();
	float backang = backvec.PolarIncline();	// 90=upright, 0=horizontal
	RotateNeck(backang);

	// Bend the neck to the angle dictated by the head tilt.
	float bendang = tiltAngle - backang + PIf/2.0;
	bendang /= (float)(m_DNA.NeckVertebraeCount() + 1);
	BendNeck(bendang);

	// Tilt head to look in the correct direction.
	unsigned int headBone;
	switch(m_Which)
	{
	default:
	case 0:
		headBone = DhDNA::BONE_Head0;
		break;
	case 1:
		headBone = DhDNA::BONE_Head1;
		break;
	}

	// Make a matrix to set head rotation at its current position.
	oloc = HeadLoc();			// Original location in skeleton.
	nloc = CurrentHeadLoc();	// Current location in skeleton.
	AuMatrix4x4 t;
	t.Translate(nloc);
	t.RotateX(tiltAngle);		// Tilt to requested rotation.
	t.RotateZ(turnAngle);		// Turn to requested rotation.
	t.Translate(-oloc);
	m_Body->Bone(headBone) = t;	// Set the transformation matrix.
}

void DpHeadAndNeck::RoughExtentLocs(AuPointArray& locs) const
{
	// Determine neck point for origin.
	AuPoint headLoc = HeadLoc();
	float hlen = m_Body->HeadLength();

	// Neck
	unsigned int nvc = m_DNA.NeckVertebraeCount();
	//qDebug("DpHeadAndNeck::CreateSkin - %d vertebrae", nvc);
	if(nvc)
	{
		DhDNA::BONES thisBone;
		switch(m_Which)
		{
		default:
		case 0:
			thisBone = DhDNA::BONE_Neck00;
			break;
		case 1:
			thisBone = DhDNA::BONE_Neck10;
			break;
		}

		// Neck vertebrae are shorter than torso vertebrae.
		float vertlen = m_Body->TorsoSegmentLength() / 2.5;

		// NOTE: These chunks are skinned from the head down to the body so the
		// concept of previous and next may appear backward.
		AuPoint segloc(headLoc);
		segloc.y -= hlen * .4f;	// Drawing last neck segment from the bottom before head
		headLoc.y -= hlen/3.0;	// Center head on its connect point.
		headLoc.z += hlen*.7f;	// Move head up by the thickness of the head.
		for(unsigned int iv=0; iv<nvc; iv++)
		{
			// Test chunk intersection.
			m_Body->RoughExtentChunk(locs, thisBone, segloc, vertlen);

			thisBone = (DhDNA::BONES)(thisBone + 1);

			segloc.y -= vertlen;	// -y direction
		}
	}

	// Head
	DhDNA::BONES headBone;
	switch(m_Which)
	{
	default:
	case 0:
		headBone = DhDNA::BONE_Head0;
		break;
	case 1:
		headBone = DhDNA::BONE_Head1;
		break;
	}

	// Test chunk intersection.
	m_Body->RoughExtentChunk(locs, headBone, headLoc, hlen);
}

void DpHeadAndNeck::RenderHitBox(AuGLEffects& re)
{
	// Determine neck point for origin.
	AuPoint headLoc = HeadLoc();
	float hlen = m_Body->HeadLength();
	float hthick = m_Body->HeadThickness();
	float hwid = m_Body->HeadWidth();

	// Neck
	unsigned int nvc = m_DNA.NeckVertebraeCount();
	//qDebug("DpHeadAndNeck::CreateSkin - %d vertebrae", nvc);
	if(nvc)
	{
		DhDNA::BONES thisBone;
		switch(m_Which)
		{
		default:
		case 0:
			thisBone = DhDNA::BONE_Neck00;
			break;
		case 1:
			thisBone = DhDNA::BONE_Neck10;
			break;
		}

		// Neck vertebrae are shorter than torso vertebrae.
		float vertlen = m_Body->TorsoSegmentLength() / 2.5;

		// Skin type determines taper of the neck.
		float thick = m_Body->TorsoThickness();
		float width = m_Body->TorsoWidth();
		float endthick = thick;
		float endwidth = width;
		switch(m_DNA.SkinType)
		{
		case DhDNA::SKIN_DryExoskeleton:	// Ant.
		case DhDNA::SKIN_WetExoskeleton:	// Black widow, millipede.
			// Neck is one or more abdomen sections that do not taper.
			thick /= 4.;
			width /= 4.;
			endthick = thick;	// base of neck
			endwidth = width;
			break;
		case DhDNA::SKIN_DryScales:		// Lizard.
		case DhDNA::SKIN_WetScales:		// Fish.
			// Tail tapers from body thickness to a narrow end.
			thick = hthick;
			width = hwid;
			break;
		case DhDNA::SKIN_DrySkin:		// Pig, human.
		case DhDNA::SKIN_Feathers:		// Sparrow, ostrich, chicken.
		case DhDNA::SKIN_Fur:			// Covered with fur. Beaver, horse.
		case DhDNA::SKIN_WetSkin:		// Salamander, kardassian
			// Tail begins at a thickness a bit narrower than the body.
			// Tail tapers to a bit narrower thickness.
			thick /= 3.5;		// end of neck at head
			width /= 3.5;
			endthick /= 3.0;	// base of neck
			endwidth /= 3.0;
			break;
		}
		float incthick = (endthick-thick)/(float)nvc;	// Increment per vertex.
		float incwidth = (endwidth-width)/(float)nvc;	// Increment per vertex.

		// NOTE: These chunks are skinned from the head down to the body so the
		// concept of previous and next may appear backward.
		AuPoint segloc(headLoc);
		segloc.y -= hlen * .4f;	// Drawing last neck segment from the bottom before head
		for(unsigned int iv=0; iv<nvc; iv++)
		{
			float nthick = thick + incthick;
			float nwidth = width + incwidth;

			// Test chunk intersection.
			//float avgrad = (width + thick + nwidth + nthick)/8.;
			float avgrad;
			if(width > thick && width > nwidth && width > nthick)
				avgrad = width/2.f;
			else if(thick > nwidth && thick > nthick)
				avgrad = thick/2.f;
			else if(nwidth > nthick)
				avgrad = nwidth/2.f;
			else
				avgrad = nthick/2.f;
			m_Body->RenderHitBoxChunk(re, thisBone, segloc, vertlen, avgrad);

			thick = nthick;
			width = nwidth;
			thisBone = (DhDNA::BONES)(thisBone + 1);

			segloc.y -= vertlen;	// -y direction
		}
	}

	// Head
	DhDNA::BONES headBone;
	switch(m_Which)
	{
	default:
	case 0:
		headBone = DhDNA::BONE_Head0;
		break;
	case 1:
		headBone = DhDNA::BONE_Head1;
		break;
	}

	AuPoint headBack(headLoc);
	headBack.y -= hlen/3.0;	// Center head on its connect point.
	headBack.z += hlen*.7f;	// Move head up by the thickness of the head.

	// Test chunk intersection.
	//float avgrad = (hwid + hthick)/4.;
	float avgrad = hlen * .4f;
	//if(hwid > hthick)
		//avgrad = hwid/2.f;
	//else
		//avgrad = hthick/2.f;
	m_Body->RenderHitBoxChunk(re, headBone, headBack, hlen, avgrad);

	// Additional line in head.
	AuPoint headTop(headLoc);
	headTop.z += hlen;
	headLoc = m_Body->Bone(headBone).Project(headLoc);
	headTop = m_Body->Bone(headBone).Project(headTop);
	/*DHH glsl
	re.glBegin(GL_LINES);
		// Bone is blue
		re.glColor4f(0,0,1,1);
		re.glVertex3d(headLoc.x, headLoc.y, headLoc.z);
		re.glVertex3d(headTop.x, headTop.y, headTop.z);
	re.glEnd();
	*/
}

void DpHeadAndNeck::RenderHitBoxHeadOnly(AuGLEffects& re)
{
	// Determine neck point for origin.
	AuPoint headLoc = HeadLoc();
	float hlen = m_Body->HeadLength();
	//float hthick = m_Body->HeadThickness();
	//float hwid = m_Body->HeadWidth();

	// Head
	DhDNA::BONES headBone;
	switch(m_Which)
	{
	default:
	case 0:
		headBone = DhDNA::BONE_Head0;
		break;
	case 1:
		headBone = DhDNA::BONE_Head1;
		break;
	}

	AuPoint headBack(headLoc);
	headBack.y -= hlen/3.0;	// Center head on its connect point.
	headBack.z += hlen*.7f;	// Move head up by the thickness of the head.

	// Test chunk intersection.
	//float avgrad = (hwid + hthick)/4.;
	float avgrad = hlen * .4f;
	//if(hwid > hthick)
		//avgrad = hwid/2.f;
	//else
		//avgrad = hthick/2.f;
	m_Body->RenderHitBoxChunk(re, headBone, headBack, hlen, avgrad);

	// Additional line in head.
	AuPoint headTop(headLoc);
	headTop.z += hlen;
	headLoc = m_Body->Bone(headBone).Project(headLoc);
	headTop = m_Body->Bone(headBone).Project(headTop);
	/*DHH glsl
	re.glBegin(GL_LINES);
		// Bone is blue
		re.glColor4f(0,0,1,1);
		re.glVertex3d(headLoc.x, headLoc.y, headLoc.z);
		re.glVertex3d(headTop.x, headTop.y, headTop.z);
	re.glEnd();
	*/
}

bool DpHeadAndNeck::SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const
{
	bool hitMe = false;

	// Determine neck point for origin.
	AuPoint headLoc = HeadLoc();
	float hlen = m_Body->HeadLength();
	float hthick = m_Body->HeadThickness();
	float hwid = m_Body->HeadWidth();

	// Neck
	unsigned int nvc = m_DNA.NeckVertebraeCount();
	//qDebug("DpHeadAndNeck::CreateSkin - %d vertebrae", nvc);
	if(nvc)
	{
		DhDNA::BONES thisBone;
		switch(m_Which)
		{
		default:
		case 0:
			thisBone = DhDNA::BONE_Neck00;
			break;
		case 1:
			thisBone = DhDNA::BONE_Neck10;
			break;
		}

		// Neck vertebrae are shorter than torso vertebrae.
		float vertlen = m_Body->TorsoSegmentLength() / 2.5;

		// Skin type determines taper of the neck.
		float thick = m_Body->TorsoThickness();
		float width = m_Body->TorsoWidth();
		float endthick = thick;
		float endwidth = width;
		switch(m_DNA.SkinType)
		{
		case DhDNA::SKIN_DryExoskeleton:	// Ant.
		case DhDNA::SKIN_WetExoskeleton:	// Black widow, millipede.
			// Neck is one or more abdomen sections that do not taper.
			thick /= 4.;
			width /= 4.;
			endthick = thick;	// base of neck
			endwidth = width;
			break;
		case DhDNA::SKIN_DryScales:		// Lizard.
		case DhDNA::SKIN_WetScales:		// Fish.
			// Tail tapers from body thickness to a narrow end.
			thick = hthick;
			width = hwid;
			break;
		case DhDNA::SKIN_DrySkin:		// Pig, human.
		case DhDNA::SKIN_Feathers:		// Sparrow, ostrich, chicken.
		case DhDNA::SKIN_Fur:			// Covered with fur. Beaver, horse.
		case DhDNA::SKIN_WetSkin:		// Salamander, kardassian
			// Tail begins at a thickness a bit narrower than the body.
			// Tail tapers to a bit narrower thickness.
			thick /= 3.5;		// end of neck at head
			width /= 3.5;
			endthick /= 3.0;	// base of neck
			endwidth /= 3.0;
			break;
		}
		float incthick = (endthick-thick)/(float)nvc;	// Increment per vertex.
		float incwidth = (endwidth-width)/(float)nvc;	// Increment per vertex.

		// NOTE: These chunks are skinned from the head down to the body so the
		// concept of previous and next may appear backward.
		AuPoint segloc(headLoc);
		segloc.y -= hlen * .4f;	// Drawing last neck segment from the bottom before head
		for(unsigned int iv=0; iv<nvc; iv++)
		{
			float nthick = thick + incthick;
			float nwidth = width + incwidth;

			// Test chunk intersection.
			//float avgrad = (width + thick + nwidth + nthick)/8.;
			float avgrad;
			if(width > thick && width > nwidth && width > nthick)
				avgrad = width/2.f;
			else if(thick > nwidth && thick > nthick)
				avgrad = thick/2.f;
			else if(nwidth > nthick)
				avgrad = nwidth/2.f;
			else
				avgrad = nthick/2.f;
			if(m_Body->SightChunk(thisBone, segloc, segloc + AuPoint(0,vertlen), avgrad, eyeLoc, sightedLoc))
			{
				sightedPart = thisBone;
				hitMe = true;
			}

			thick = nthick;
			width = nwidth;
			thisBone = (DhDNA::BONES)(thisBone + 1);

			segloc.y -= vertlen;	// -y direction
		}
	}

	// Head
	DhDNA::BONES headBone;
	switch(m_Which)
	{
	default:
	case 0:
		headBone = DhDNA::BONE_Head0;
		break;
	case 1:
		headBone = DhDNA::BONE_Head1;
		break;
	}

	AuPoint headBack(headLoc);
	headBack.y -= hlen/3.0;	// Center head on its connect point.
	headBack.z += hlen*.7f;	// Move head up by the thickness of the head.

	// Test chunk intersection.
	//float avgrad = (hwid + hthick)/4.;
	float avgrad = hlen * .4f;
	//if(hwid > hthick)
		//avgrad = hwid/2.f;
	//else
		//avgrad = hthick/2.f;
	if(m_Body->SightChunk(headBone, headBack, headBack + AuPoint(0,hlen), avgrad, eyeLoc, sightedLoc) ||
			m_Body->SightChunk(headBone, headLoc, headLoc + AuPoint(0,0,hlen), avgrad, eyeLoc, sightedLoc))
	{
		sightedPart = headBone;
		hitMe = true;
	}

	return hitMe;
}

bool DpHeadAndNeck::SightTargetDetailsHead(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const
{
	bool hitMe = false;

	// Determine neck point for origin.
	AuPoint headLoc = HeadLoc();
	float hlen = m_Body->HeadLength();
	//float hthick = m_Body->HeadThickness();
	//float hwid = m_Body->HeadWidth();

	// Head
	DhDNA::BONES headBone;
	switch(m_Which)
	{
	default:
	case 0:
		headBone = DhDNA::BONE_Head0;
		break;
	case 1:
		headBone = DhDNA::BONE_Head1;
		break;
	}

	AuPoint headBack(headLoc);
	headBack.y -= hlen/3.0;	// Center head on its connect point.
	headBack.z += hlen*.7f;	// Move head up by the thickness of the head.

	// Test chunk intersection.
	//float avgrad = (hwid + hthick)/4.;
	float avgrad = hlen * .4f;
	//if(hwid > hthick)
		//avgrad = hwid/2.f;
	//else
		//avgrad = hthick/2.f;
	if(m_Body->SightChunk(headBone, headBack, headBack + AuPoint(0,hlen), avgrad, eyeLoc, sightedLoc) ||
			m_Body->SightChunk(headBone, headLoc, headLoc + AuPoint(0,0,hlen), avgrad, eyeLoc, sightedLoc))
	{
		sightedPart = headBone;
		hitMe = true;
	}

	return hitMe;
}

float DpHeadAndNeck::Volume() const
{
	// Head Dimensions
	float hlen = m_Body->HeadLength();
	float hthick = m_Body->HeadThickness();
	float hwid = m_Body->HeadWidth();
	float volume = hlen * hthick * hwid;
	//qDebug() << "DpHeadAndNeck::Volume" << hlen << hthick << hwid << volume;

	// Neck
	int nvc = m_DNA.NeckVertebraeCount();
	if(nvc)
	{
		// Neck vertebrae are shorter than torso vertebrae.
		float vertlen = m_Body->TorsoSegmentLength() / 2.5;
		float thick = m_Body->TorsoThickness()/2.;
		float width = m_Body->TorsoWidth()/2.;
		volume += width * thick * vertlen * nvc;
	}

	// Fudge factor so human comes out about right.
	volume *= 1.25f;

	//qDebug("DpHeadAndNeck::Volume volume %g", volume);
	return volume;
}

quint64 DpHeadAndNeck::MemoryUsage() const
{
	quint64 mem = DpBodyPart::MemoryUsage();
	mem += sizeof(m_Which);
	mem += sizeof(m_HeadLoc);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

void DpArmAndHand::CreateSkin(AuGLEffects& re, AuGLModelCache& mc)
{
	OneTimeCalcs();
	DhDNA::BONES upperBone;
	DhDNA::BONES foreBone;
	DhDNA::BONES handBone;
	Bones(upperBone, foreBone, handBone);
	// Dimensions
	float ul = m_Body->UpperArmLength();
	float fl = m_Body->ForeArmLength();

	// Calculate shoulder position for origin of vertices.
	AuPoint loc = ConnectLoc();

	// Draw model
	Au3dsGLSLModelInstance* modelArmPiece;
	Au3dsGLSLModelInstance* modelHandPiece = 0;
	float uLength, uThick, lLength, lThick, handLength;
	uLength = m_DNA.Length(m_DNA.UpperArmLength);
	uThick = m_DNA.Thickness(m_DNA.UpperArmThickness);
	lLength = m_DNA.Length(m_DNA.ForeArmLength);
	lThick = m_DNA.Thickness(m_DNA.ForeArmThickness);
	handLength = m_DNA.Length(m_DNA.HandLength)/4;
	QString armModel, handModel;
	double vTOffset = m_Body->m_vTOffset;
	double vBOffset = m_Body->m_vBOffset;
	double handWingLength = 1;
	if(m_DNA.ArmType >= 2)
	{
		// WING ARM
		vTOffset = (((m_DNA.WingSubType) * 128))/1024.0;	
		if(vTOffset > 0)
			vTOffset = 1-vTOffset;
		vBOffset = (((m_DNA.WingSubType) * 128))/1024.0;
		if(vBOffset > 0)
			vBOffset = 1-vBOffset;
		handWingLength = handLength/.5f;
	}

	QString sPrefix = m_Body->GetBodyModelPrefix();
	bool bHasArmor = m_Body->HasBodyPowerArmorOn();
	Au3dsGLSLModelDefinition modelDef(1, m_Body->OverallScale(), m_Body->m_uTOffset, m_Body->m_vTOffset,
		m_Body->m_uBOffset, m_Body->m_vBOffset, upperBone, foreBone, handBone, loc, uLength/1.5, uThick/.5*(uLength/1.5),
		lLength/1.5, lThick/.5*(lLength/1.5), handWingLength, m_Body->m_TopColor[bHasArmor], m_Body->m_TopShininess[bHasArmor],
		m_Body->m_BottomColor[bHasArmor], m_Body->m_BottomShininess[bHasArmor], m_Body->m_Glow[bHasArmor], m_Body->m_Glowdim[bHasArmor], true);
	if(upperBone == DhDNA::BONE_UpperArm0R || upperBone == DhDNA::BONE_UpperArm1R || upperBone == DhDNA::BONE_UpperArm2R)
	{
		// RIGHT ARM
		if(m_DNA.ArmType >= 2)	// Wing
			armModel = QString(":data/WingRight%2.3dsN").arg(m_DNA.ArmType);
		else if(bHasArmor)
			armModel = QString("%1ArmRight.3dsN").arg(sPrefix);
		else
			armModel = QString("%1ArmRight%2.3dsN").arg(sPrefix).arg(m_DNA.ArmType);
		modelArmPiece = mc.CreateBodyModel(re, armModel, m_Body->m_Min, m_Body->m_Max, false, modelDef);
		loc.y += ul;
		loc.y += fl;
		if(m_DNA.ArmType < 2)
		{
			// RIGHT HAND
			if(bHasArmor)
				handModel = QString("%1HandRight.3dsN").arg(sPrefix);
			else
				handModel = QString("%1HandRight%2.3dsN").arg(sPrefix).arg(m_DNA.HandType);
			modelDef.SetOffset(loc);
			modelDef.SetLowerLength(handLength/.5f);
			modelHandPiece = mc.CreateBodyModel(re, handModel, m_Body->m_Min, m_Body->m_Max, false, modelDef);
		}
	}
	else
	{
		// LEFT ARM
		if(m_DNA.ArmType >= 2)	// Wing
			armModel = QString(":data/WingLeft%2.3dsN").arg(m_DNA.ArmType);
		else if(bHasArmor)
			armModel = QString("%1ArmLeft.3dsN").arg(sPrefix);
		else
			armModel = QString("%1ArmLeft%2.3dsN").arg(sPrefix).arg(m_DNA.ArmType);
		modelArmPiece = mc.CreateBodyModel(re, armModel, m_Body->m_Min, m_Body->m_Max, false, modelDef);
		loc.y += ul;
		loc.y += fl;
		if(m_DNA.ArmType < 2)
		{
			// LEFT HAND
			if(bHasArmor)
				handModel = QString("%1HandLeft.3dsN").arg(sPrefix);
			else
				handModel = QString("%1HandLeft%2.3dsN").arg(sPrefix).arg(m_DNA.HandType);
			modelDef.SetOffset(loc);
			modelDef.SetLowerLength(handLength/.5f);
			modelHandPiece = mc.CreateBodyModel(re, handModel, m_Body->m_Min, m_Body->m_Max, false, modelDef);
		}
	}
	if(modelArmPiece)
		m_Body->m_Models.append(modelArmPiece);
	if(modelHandPiece && m_DNA.ArmType < 2)
		m_Body->m_Models.append(modelHandPiece);
}

// False and limb must reach out to stand.
// True means the limb can physically reach the position.
bool DpArmAndHand::DoesTouchGroundBelowJoint(float shoulderHt,
		float upperarmlen, float forearmlen, float handlen) const
{
	if(m_DNA.IsExoskeletal())
		return false;	// Exoskeletal creatures always splay their legs.

	float shinlen = 0.0;
	if(m_DNA.HasWristBend())
	{
		// These hand types have a wrist bend.
		shinlen = forearmlen;
	}
	else
	{
		// These hand types do not have a wrist bend.
		shinlen = forearmlen + handlen;
	}

	if(upperarmlen + shinlen <= shoulderHt)
		return false;	// Hand cannot be placed on ground under the shoulder.
	else if(shoulderHt + shinlen <= upperarmlen)
		return false;	// Hand cannot be placed on ground under the shoulder.
	else if(shoulderHt + upperarmlen <= shinlen)
		return false;	// Hand cannot be placed on ground under the shoulder.
	return true;		// Hand can be placed on ground under the shoulder.
}

void DpArmAndHand::OneTimeCalcs()
{
	// Connect Loc
	AuPoint loc;						// This will be our return value.
	float vertlen = m_Body->TorsoSegmentLength();
	int waist = m_DNA.WaistVertex();
	if(IsWing())
	{
		loc = AuPoint(0, vertlen*waist, 0);
		if(m_DNA.TorsoVertebrae)
			loc.y -= vertlen/2.;	// Midway between vertices.

		// Calculate wing pivot position for origin.
		float shoulder = m_Body->TorsoWidth()/4.;	// Horz dist from spine to pivot.
		if(IsLeft())
			loc.x = -shoulder;
		else
			loc.x = shoulder;

		float pivot = m_Body->TorsoThickness()/2.;	// Vert dist from spine to pivot
		if(!m_DNA.TorsoVertebrae)
		{
			loc.x /= 3.0;
			pivot /= 3.0;
		}
		loc.z += pivot;
	}
	else
	{
		unsigned int iv = m_Which%m_DNA.ArmVertices();	// Spine vertex to attach arm.
		// Cycle between available vertices.
		loc = AuPoint(0, vertlen*(waist-iv), 0);

		// Calculate shoulder position for origin.
		float shoulder = m_Body->TorsoWidth()/2.*1.25;	// Horz dist from spine to pivot.
		float shoulderoff = m_Body->UpperArmThickness()/3.;
		//if(shoulder > shoulderoff)
		//	shoulder -= shoulderoff/2.;		// Pull the shoulders closer together.
		//if(shoulder < shoulderoff)
		shoulder += shoulderoff;			// Avoid touching the shoulders together.
		if(IsLeft())
			loc.x = -shoulder;
		else
			loc.x = shoulder;

		if(!m_DNA.TorsoVertebrae)
			loc.x /= 3.0;
	}
	m_ConnectLoc = loc;

	// Elbow Loc
	loc = ConnectLoc();
	if(IsWing())
		loc.y += m_Body->UpperArmLength()*1.5;
	else
		loc.y += m_Body->UpperArmLength();
	m_ElbowLoc = loc;

	// Wrist Loc
	loc = ElbowLoc();
	if(IsWing())
		loc.y += m_Body->ForeArmLength()*1.5;
	else
		loc.y += m_Body->ForeArmLength();
	m_WristLoc = loc;

	// Hand Loc
	loc = WristLoc();
	loc.y += m_Body->HandLength()/2.0;
	m_HandLoc = loc;
}

AuPoint DpArmAndHand::ConnectLoc() const
{
	return m_ConnectLoc;
}

// Calculate loc relative to body origin.
AuPoint DpArmAndHand::ElbowLoc() const
{
	return m_ElbowLoc;
}

// Calculate loc relative to body origin.
AuPoint DpArmAndHand::WristLoc() const
{
	return m_WristLoc;
}

// Calculate loc relative to body origin.
AuPoint DpArmAndHand::HandLoc() const
{
	return m_HandLoc;
}

AuPoint DpArmAndHand::CurrentConnectLoc() const
{
	DhDNA::BONES upperBone;
	DhDNA::BONES foreBone;
	DhDNA::BONES handBone;
	Bones(upperBone, foreBone, handBone);
	AuPoint base = ConnectLoc();
	AuPoint curbase = m_Body->Bone(upperBone).Project(base);
	return curbase;
}

AuPoint DpArmAndHand::CurrentElbowLoc() const
{
	DhDNA::BONES upperBone;
	DhDNA::BONES foreBone;
	DhDNA::BONES handBone;
	Bones(upperBone, foreBone, handBone);
	AuPoint loc = ElbowLoc();
	AuPoint curloc = m_Body->Bone(foreBone).Project(loc);
	return curloc;
}

AuPoint DpArmAndHand::CurrentWristLoc() const
{
	DhDNA::BONES upperBone;
	DhDNA::BONES foreBone;
	DhDNA::BONES handBone;
	Bones(upperBone, foreBone, handBone);
	AuPoint loc = WristLoc();
	AuPoint curloc = m_Body->Bone(handBone).Project(loc);
	return curloc;
}

AuPoint DpArmAndHand::CurrentHandLoc() const
{
	DhDNA::BONES upperBone;
	DhDNA::BONES foreBone;
	DhDNA::BONES handBone;
	Bones(upperBone, foreBone, handBone);
	AuPoint loc = HandLoc();
	AuPoint curloc = m_Body->Bone(handBone).Project(loc);
	return curloc;
}

void DpArmAndHand::MoveBy(const AuPoint& vMove)
{
	DhDNA::BONES upperBone;
	DhDNA::BONES foreBone;
	DhDNA::BONES handBone;
	Bones(upperBone, foreBone, handBone);

	// Make a matrix to move my bones.
	AuMatrix4x4 t;
	t.Translate(vMove);
	m_Body->Bone(upperBone) = t * m_Body->Bone(upperBone);
	m_Body->Bone(foreBone) = t * m_Body->Bone(foreBone);
	m_Body->Bone(handBone) = t * m_Body->Bone(handBone);
}

void DpArmAndHand::CalcStandAngles(float hipHt, float& shoulderang,
		float& elbowbend, float& wristbend) const
{
	DhDNA::BONES upperBone;
	DhDNA::BONES foreBone;
	DhDNA::BONES handBone;
	Bones(upperBone, foreBone, handBone);

	const float upperarmlen = m_Body->UpperArmLength();
	const float forearmlen = m_Body->ForeArmLength();
	const float handlen = m_Body->HandLength();

	elbowbend = 0;
	shoulderang = 0;
	wristbend = 0;
	float shinlen = 0;
	if(m_DNA.HasWristBend())
		shinlen = forearmlen;
	else
		shinlen = forearmlen + handlen;

	// Calculate a height that the arm can reach, even if it's not touching the
	// ground. This is the shoulder height.
	AuPoint loc = m_Body->Bone(upperBone).Translation();
	float curHt = loc.z + hipHt;
	bool reachground = false;
	float shoulderHt = upperarmlen + shinlen;
	shoulderHt *= 0.95f;			// Arms are mostly extended when standing.
	if(GreaterThanOrFuzzyEquals(shoulderHt,curHt))
	{
		reachground = true;
		shoulderHt = curHt;	// Touch the ground, don't go below it.
	}

	// Using the Law of Cosines...
	float shin2 = shinlen*shinlen;
	float shoulderHt2 = shoulderHt*shoulderHt;
	float upperarm2 = upperarmlen*upperarmlen;
	shoulderang = (shin2 - upperarm2 - shoulderHt2) /
		(-2.f * upperarmlen * shoulderHt);
	shoulderang = acosf(shoulderang);
	elbowbend = (shoulderHt2 - upperarm2 - shin2) /
		(-2.f * upperarmlen * shinlen);
	elbowbend = acosf(elbowbend);

	if(reachground)
	{
		// The arms will reach the ground.
		if(!DoesTouchGroundBelowJoint(shoulderHt, upperarmlen, forearmlen,
					handlen))
		{
			// The angles must be calculated more like a spider because the arm
			// dimensions don't allow the contact point to touch the ground
			// immediately below the shoulder...or this type of critter just
			// doesn't do that.
			if(shinlen > 0)
			{
				float elevang = 45.f * DEG2RADf;	// Max angle to elev elbow.
				float addht = sinf(elevang) * upperarmlen;	// Height added at
													// elbow.
				float elbowht = shoulderHt + addht;
				if(elbowht > shinlen)
				{
					// Hand can be placed directly under the elbow.
					// Shin length drives calc.
					elevang = asinf((shinlen - shoulderHt)/upperarmlen);
					elbowbend = PIf/2.f - elevang;
				}
				else
				{
					// Hand cannot be placed under elbow.
					elbowbend = acosf(elbowht/shinlen);
					elbowbend += (PIf/2.f - elevang);
				}
				shoulderang = PIf/2.f + elevang;
				//qDebug("DpArmAndHand::Stand - %s shinlen %g upperarmlen %g shoulder ht %g ang %g elbowbend %g elbowht %g addht %g",
						//(const char*)m_DNA.Description(),
						//shinlen, upperarmlen, shoulderHt, shoulderang*RAD2DEG,
						//elbowbend*RAD2DEG, elbowht, addht
						//);
			}
		}
	}
	else
	{
		// The arms do not reach the ground.
		// The angles we've calculated should look fine.
		//qDebug("DpArmAndHand::Stand %s arm type %d does not reach the ground shoulderHt %g base %g elbowbend %g shoulderang %g",
				//(const char*)m_DNA.Description(),
				//m_DNA.ArmType, shoulderHt, hipHt,
				//elbowbend*RAD2DEG, shoulderang*RAD2DEG);
	}

	// Calculate wrist bend needed.
	if(reachground && m_DNA.HasWristBend())
		wristbend = (elbowbend + shoulderang) - PIf/2.f;
	else
		wristbend = 2.f*DEG2RADf;
	shoulderang = -PIf/2.f - shoulderang;
	elbowbend = PIf - elbowbend;
}

void DpArmAndHand::Beckon(float headTilt)
{
	float perang = 0;
	if(m_DNA.ArmPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.ArmPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2.f/3.f;
		else if(m_Which == 2)
			perang += PIf*4.f/3.f;
	}
	float persin = m_DNA.Rhythm(AuPose::Beckon, perang, 1);

	float elbowbend = (85. + 15.*persin)*DEG2RADf;
	float shoulderang = -10.f*DEG2RADf + headTilt;
	float wristbend;
	if(m_DNA.HasWristBend())
		wristbend = -6.f*DEG2RADf + 10.f*DEG2RADf * persin;
	else
		wristbend = -2.f*DEG2RADf;

	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	SplayShoulder(m_Splay);
	LiftShoulder(m_Lift);
}

void DpArmAndHand::Block(float headTilt)
{
	float perang = 0;
	if(m_DNA.ArmPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.ArmPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2./3;
		else if(m_Which == 2)
			perang += PIf*4./3;
	}
	float persin = m_DNA.Rhythm(AuPose::Block, perang, 1);

	float elbowbend = PIf/3.f;
	float shoulderang = (-30.f + 10.f*persin)*DEG2RADf + headTilt;	// -PIf=straight back
	float wristbend;
	if(m_DNA.HasWristBend())
		wristbend = 80.f*DEG2RADf;
	else
		wristbend = -2.f*DEG2RADf;
	float lift = m_Lift + 10.f*DEG2RADf*persin;

	// Position bones.
	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	SplayShoulder(m_Splay);
	LiftShoulder(lift);
}

void DpArmAndHand::Dig()
{
	float perang = 0;
	if(!IsLeft())
		perang += PIf;
	if(m_DNA.ArmPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.ArmPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2./3;
		else if(m_Which == 2)
			perang += PIf*4./3;
	}
	float persin = m_DNA.Rhythm(AuPose::Dig, perang, 1);

	float elbowbend = (75. - 60.*persin)*DEG2RADf;
	float shoulderang = (90. + 70.*persin)*DEG2RADf - PIf;
	float wristbend;
	if(m_DNA.HasWristBend())
		wristbend = 80.f*DEG2RADf * persin;
	else
		wristbend = -2.f*DEG2RADf;

	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	SplayShoulder(m_Splay);
	LiftShoulder(m_Lift);
}

void DpArmAndHand::Jump(float hipHt, bool bendToHoldObject, float speedEnc)
{
	// When jumping, all limbs move in unison.
	float persin = m_DNA.Rhythm(AuPose::Jump, 0, speedEnc);

	// Outstretched limb angles.
	float elbowout = 10.f*DEG2RADf;
	float shoulderout = -5.f*DEG2RADf;
	float wristout = 0;
	if(m_DNA.HasWristBend())
		wristout = 10.f*DEG2RADf;
	else
		wristout = -2.f*DEG2RADf;

	// Standing joint angles.
	float elbowstand, shoulderstand, wriststand;
	CalcStandAngles(hipHt, shoulderstand, elbowstand, wriststand);

	// Cycle between outstretched and standing limb position.
	float shouldersweep = shoulderstand - shoulderout;
	float shoulderang = shoulderout + (shouldersweep + shouldersweep*persin)/2.f;
	float elbowbend;
	float wristbend;
	if(bendToHoldObject)
	{
		elbowbend = -shoulderang + m_Body->CurrentHeadTilt();
		wristbend = -2*DEG2RADf;
	}
	else
	{
		float elbowsweep = elbowstand - elbowout;
		float wristsweep = wriststand - wristout;
		elbowbend = elbowout + (elbowsweep + elbowsweep*persin)/2.f;
		wristbend = wristout + (wristsweep + wristsweep*persin)/2.f;
	}

	// Position bones.
	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	SplayShoulder(m_Splay);
	LiftShoulder(m_Lift);
}

void DpArmAndHand::Point(float headTilt)
{
	float elbowbend = 5.f*DEG2RADf;
	float shoulderang = -5.f*DEG2RADf + headTilt;
	float wristbend = 0;

	// Position bones.
	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	SplayShoulder(m_Splay);
	LiftShoulder(m_Lift);
}

void DpArmAndHand::DeadFacingDown()
{
	float elbowbend = 2.f*DEG2RADf;
	float shoulderang = -182.f*DEG2RADf;
	float wristbend = 0;

	// Position bones.
	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	SplayShoulder(m_Splay);
}

void DpArmAndHand::DeadFacingLeft()
{
	// Position bones.
	if(IsLeft())
	{
		float shoulderang = -(75.f + m_Which*8.f)*DEG2RADf;
		RotateShoulder(shoulderang);

		float wristbend;
		if(m_DNA.HasWristBend())
			wristbend = 15.f*DEG2RADf;
		else
			wristbend = -2.f*DEG2RADf;
		RotateWrist(wristbend);

		float elbowbend = 65.f*DEG2RADf;
		RotateElbow(elbowbend);

		LiftShoulder(-20.f*DEG2RADf);	// Drop suspended arm slightly.
	}
	else
	{
		float shoulderang = -(125.f + m_Which*10.f)*DEG2RADf;
		RotateShoulder(shoulderang);

		float wristbend;
		if(m_DNA.HasWristBend())
			wristbend = -25.f*DEG2RADf;
		else
			wristbend = -2.f*DEG2RADf;
		RotateWrist(wristbend);

		float elbowbend = 10.f*DEG2RADf;
		RotateElbow(elbowbend);
	}

	SplayShoulder(m_Splay);
}

void DpArmAndHand::DeadFacingRight()
{
	// Position bones.
	if(!IsLeft())
	{
		float shoulderang = -(75.f + m_Which*8.f)*DEG2RADf;
		RotateShoulder(shoulderang);

		float wristbend;
		if(m_DNA.HasWristBend())
			wristbend = 15.f*DEG2RADf;
		else
			wristbend = -2.f*DEG2RADf;
		RotateWrist(wristbend);

		float elbowbend = 65.f*DEG2RADf;
		RotateElbow(elbowbend);

		LiftShoulder(-20.f*DEG2RADf);	// Drop suspended arm slightly.
	}
	else
	{
		float shoulderang = -(125.f + m_Which*10.f)*DEG2RADf;
		RotateShoulder(shoulderang);

		float wristbend;
		if(m_DNA.HasWristBend())
			wristbend = -25.f*DEG2RADf;
		else
			wristbend = -2.f*DEG2RADf;
		RotateWrist(wristbend);

		float elbowbend = 10.f*DEG2RADf;
		RotateElbow(elbowbend);
	}

	SplayShoulder(m_Splay);
}

void DpArmAndHand::DeadFacingUp()
{
	float elbowbend = 2.f*DEG2RADf;
	float shoulderang = -182.f*DEG2RADf;
	float wristbend = 0;

	// Position bones.
	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	SplayShoulder(m_Splay);
}

void DpArmAndHand::DeadWingFacingDown()
{
	if(IsWingSkeletal())
	{
		// This is a complex wing with multiple bones.
		// Wing normal position is straight out ahead of critter along +Y axis,
		// like arms and legs.
		float shoulderang = 0;	// YZ plane rotation
		float elbowbend = 0;		// YZ plane rotation
		float wristbend = 0;		// YZ plane rotation
		float spread = 0;		// XY plane rotation
		float bow = 0;			// XZ plane rotation
		if(m_DNA.ArmType == DhDNA::ARM_WingFeathered)
		{
			// Feathered wing
			shoulderang = 10.f * DEG2RADf;
			elbowbend = -25.f * DEG2RADf;
			wristbend = -10.f * DEG2RADf;
			spread = -85.f * DEG2RADf;
			bow = 15.f * DEG2RADf;
		}
		else
		{
			// Leathery wing
			shoulderang = 10.f * DEG2RADf;
			elbowbend = -15.f * DEG2RADf;
			wristbend = -15.f * DEG2RADf;
			spread = -85.f * DEG2RADf;
			bow = 10.f * DEG2RADf;
		}

		RotateShoulder(shoulderang);
		RotateElbow(elbowbend);
		RotateWrist(wristbend);
		BowWing(bow);
		FlapWing(spread);
	}
	else
	{
		// This is a simple wing with two more-or-less independent lobes.
		DhDNA::BONES upperBone;
		DhDNA::BONES foreBone;
		DhDNA::BONES handBone;
		Bones(upperBone, foreBone, handBone);
		float ux = 0;	// upper wing lobe X rotation
		float lx = 0;	// lower wing lobe X rotation
		float uy = 0;
		float ly = 0;
		float uz = 0;
		float lz = 0;
		if(m_DNA.ArmType == DhDNA::ARM_WingButterfly)
		{
			ux = 45.f * DEG2RADf;
			uz = -86.f * DEG2RADf;
			lx = -30.f * DEG2RADf;
			lz = -83.f * DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingDragonFly)
		{
			ux = 0.f * DEG2RADf;
			uz = -89.f * DEG2RADf;
			lx = -10.f * DEG2RADf;
			lz = -87.f * DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingFairy)
		{
			ux = 60.f * DEG2RADf;
			uz = -85.f * DEG2RADf;
			lx = 5.f * DEG2RADf;
			lz = -82.f * DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingMoth)
		{
			ux = 0.f * DEG2RADf;
			uz = -86.f * DEG2RADf;
			lx = -10.f * DEG2RADf;
			lz = -83.f * DEG2RADf;
		}

		// Make a matrix to rotate around current upper lobe base.
		AuPoint pivot = CurrentConnectLoc();
		AuMatrix4x4 t;
		t.Translate(pivot);
		if(IsLeft())
		{
			t.RotateZ(-uz);
			t.RotateY(-uy);
		}
		else
		{
			t.RotateZ(uz);
			t.RotateY(uy);
		}
		t.RotateX(ux);
		t.Translate(-pivot);
		m_Body->Bone(upperBone) = t * m_Body->Bone(upperBone);

		// Make a matrix to rotate around current lower lobe base.
		t.LoadIdentity();
		t.Translate(pivot);
		if(IsLeft())
		{
			t.RotateZ(-lz);
			t.RotateY(-ly);
		}
		else
		{
			t.RotateZ(lz);
			t.RotateY(ly);
		}
		t.RotateX(lx);
		t.Translate(-pivot);
		m_Body->Bone(foreBone) = t * m_Body->Bone(foreBone);
	}
}

void DpArmAndHand::DeadWingFacingLeft()
{
	if(IsLeft())
	{
		// Standing pose looks fine for high side, even moth.
		StandWing();
		return;
	}

	if(IsWingSkeletal())
	{
		// This is a complex wing with multiple bones.
		// Wing normal position is straight out ahead of critter along +Y axis,
		// like arms and legs.
		float shoulderang = 0;	// YZ plane rotation
		float elbowbend = 0;		// YZ plane rotation
		float wristbend = 0;		// YZ plane rotation
		float spread = 0;		// XY plane rotation
		float bow = 0;			// XZ plane rotation
		if(m_DNA.ArmType == DhDNA::ARM_WingFeathered)
		{
			// Feathered wing
			shoulderang = 10.f * DEG2RADf;
			elbowbend = -25.f * DEG2RADf;
			wristbend = -10.f * DEG2RADf;
			spread = -185.f * DEG2RADf;
			bow = 1.5f * DEG2RADf;
		}
		else
		{
			// Leathery wing
			shoulderang = 10.f * DEG2RADf;
			elbowbend = -15.f * DEG2RADf;
			wristbend = -15.f * DEG2RADf;
			spread = -180.f * DEG2RADf;
			bow = 1.f * DEG2RADf;
		}

		RotateShoulder(shoulderang);
		RotateElbow(elbowbend);
		RotateWrist(wristbend);
		BowWing(bow);
		FlapWing(spread);
	}
	else
	{
		// This is a simple wing with two more-or-less independent lobes.
		DhDNA::BONES upperBone;
		DhDNA::BONES foreBone;
		DhDNA::BONES handBone;
		Bones(upperBone, foreBone, handBone);
		float ux = 0;	// upper wing lobe X rotation
		float lx = 0;	// lower wing lobe X rotation
		float uy = 0;
		float ly = 0;
		float uz = 0;
		float lz = 0;
		if(m_DNA.ArmType == DhDNA::ARM_WingButterfly)
		{
			ux = 45.f * DEG2RADf;
			uz = -181.f * DEG2RADf;
			lx = -30.f * DEG2RADf;
			lz = -180.f * DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingDragonFly)
		{
			ux = 0.f * DEG2RADf;
			uz = -181.f * DEG2RADf;
			lx = -10.f * DEG2RADf;
			lz = -180.f * DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingFairy)
		{
			ux = 60.f * DEG2RADf;
			uz = -181.f * DEG2RADf;
			lx = 5.f * DEG2RADf;
			lz = -180.f * DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingMoth)
		{
			ux = 0.f * DEG2RADf;
			uz = -181.f * DEG2RADf;
			lx = -10.f * DEG2RADf;
			lz = -180.f * DEG2RADf;
		}

		// Make a matrix to rotate around current upper lobe base.
		AuPoint pivot = CurrentConnectLoc();
		AuMatrix4x4 t;
		t.Translate(pivot);
		if(IsLeft())
		{
			t.RotateZ(-uz);
			t.RotateY(-uy);
		}
		else
		{
			t.RotateZ(uz);
			t.RotateY(uy);
		}
		t.RotateX(ux);
		t.Translate(-pivot);
		m_Body->Bone(upperBone) = t * m_Body->Bone(upperBone);

		// Make a matrix to rotate around current lower lobe base.
		t.LoadIdentity();
		t.Translate(pivot);
		if(IsLeft())
		{
			t.RotateZ(-lz);
			t.RotateY(-ly);
		}
		else
		{
			t.RotateZ(lz);
			t.RotateY(ly);
		}
		t.RotateX(lx);
		t.Translate(-pivot);
		m_Body->Bone(foreBone) = t * m_Body->Bone(foreBone);
	}
}

void DpArmAndHand::DeadWingFacingRight()
{
	if(!IsLeft())
	{
		// Standing pose looks fine for high side, even moth.
		StandWing();
		return;
	}

	if(IsWingSkeletal())
	{
		// This is a complex wing with multiple bones.
		// Wing normal position is straight out ahead of critter along +Y axis,
		// like arms and legs.
		float shoulderang = 0;	// YZ plane rotation
		float elbowbend = 0;	// YZ plane rotation
		float wristbend = 0;	// YZ plane rotation
		float spread = 0;		// XY plane rotation
		float bow = 0;			// XZ plane rotation
		if(m_DNA.ArmType == DhDNA::ARM_WingFeathered)
		{
			// Feathered wing
			shoulderang = 10.f * DEG2RADf;
			elbowbend = -25.f * DEG2RADf;
			wristbend = -10.f * DEG2RADf;
			spread = -185.f * DEG2RADf;
			bow = 1.5f * DEG2RADf;
		}
		else
		{
			// Leathery wing
			shoulderang = 10.f * DEG2RADf;
			elbowbend = -15.f * DEG2RADf;
			wristbend = -15.f * DEG2RADf;
			spread = -180.f * DEG2RADf;
			bow = 1.f * DEG2RADf;
		}

		RotateShoulder(shoulderang);
		RotateElbow(elbowbend);
		RotateWrist(wristbend);
		BowWing(bow);
		FlapWing(spread);
	}
	else
	{
		// This is a simple wing with two more-or-less independent lobes.
		DhDNA::BONES upperBone;
		DhDNA::BONES foreBone;
		DhDNA::BONES handBone;
		Bones(upperBone, foreBone, handBone);
		float ux = 0;	// upper wing lobe X rotation
		float lx = 0;	// lower wing lobe X rotation
		float uy = 0;
		float ly = 0;
		float uz = 0;
		float lz = 0;
		if(m_DNA.ArmType == DhDNA::ARM_WingButterfly)
		{
			ux = 45.f * DEG2RADf;
			uz = -181.f * DEG2RADf;
			lx = -30.f * DEG2RADf;
			lz = -180.f * DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingDragonFly)
		{
			ux = 0.f * DEG2RADf;
			uz = -181.f * DEG2RADf;
			lx = -10.f * DEG2RADf;
			lz = -180.f * DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingFairy)
		{
			ux = 60.f * DEG2RADf;
			uz = -181.f * DEG2RADf;
			lx = 5.f * DEG2RADf;
			lz = -180.f * DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingMoth)
		{
			ux = 0.f * DEG2RADf;
			uz = -181.f * DEG2RADf;
			lx = -10.f * DEG2RADf;
			lz = -180.f * DEG2RADf;
		}

		// Make a matrix to rotate around current upper lobe base.
		AuPoint pivot = CurrentConnectLoc();
		AuMatrix4x4 t;
		t.Translate(pivot);
		if(IsLeft())
		{
			t.RotateZ(-uz);
			t.RotateY(-uy);
		}
		else
		{
			t.RotateZ(uz);
			t.RotateY(uy);
		}
		t.RotateX(ux);
		t.Translate(-pivot);
		m_Body->Bone(upperBone) = t * m_Body->Bone(upperBone);

		// Make a matrix to rotate around current lower lobe base.
		t.LoadIdentity();
		t.Translate(pivot);
		if(IsLeft())
		{
			t.RotateZ(-lz);
			t.RotateY(-ly);
		}
		else
		{
			t.RotateZ(lz);
			t.RotateY(ly);
		}
		t.RotateX(lx);
		t.Translate(-pivot);
		m_Body->Bone(foreBone) = t * m_Body->Bone(foreBone);
	}
}

void DpArmAndHand::DeadWingFacingUp()
{
	if(IsWingSkeletal())
	{
		// This is a complex wing with multiple bones.
		// Wing normal position is straight out ahead of critter along +Y axis,
		// like arms and legs.
		float shoulderang = 0;	// YZ plane rotation
		float elbowbend = 0;		// YZ plane rotation
		float wristbend = 0;		// YZ plane rotation
		float spread = 0;		// XY plane rotation
		float bow = 0;			// XZ plane rotation
		if(m_DNA.ArmType == DhDNA::ARM_WingFeathered)
		{
			// Feathered wing
			shoulderang = 10.f * DEG2RADf;
			elbowbend = -25.f * DEG2RADf;
			wristbend = -10.f * DEG2RADf;
			spread = -PIf/2.f;	// Straight out
			bow = 45.f * DEG2RADf;
		}
		else
		{
			// Leathery wing
			shoulderang = 10.f * DEG2RADf;
			elbowbend = -15.f * DEG2RADf;
			wristbend = -15.f * DEG2RADf;
			spread = -PIf/2.f;	// Straight out
			bow = 35.f * DEG2RADf;
		}

		RotateShoulder(shoulderang);
		RotateElbow(elbowbend);
		RotateWrist(wristbend);
		BowWing(bow);
		FlapWing(spread);
	}
	else
	{
		// This is a simple wing with two more-or-less independent lobes.
		DhDNA::BONES upperBone;
		DhDNA::BONES foreBone;
		DhDNA::BONES handBone;
		Bones(upperBone, foreBone, handBone);
		float ux = 0;	// upper wing lobe X rotation
		float lx = 0;	// lower wing lobe X rotation
		float uy = 0;
		float ly = 0;
		float uz = 0;
		float lz = 0;
		if(m_DNA.ArmType == DhDNA::ARM_WingButterfly)
		{
			ux = 45.f * DEG2RADf;
			uz = -89.f * DEG2RADf;
			lx = -30.f * DEG2RADf;
			lz = -87.f * DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingDragonFly)
		{
			ux = 0.f * DEG2RADf;
			uz = -90.f * DEG2RADf;
			lx = -10.f * DEG2RADf;
			lz = -88.f * DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingFairy)
		{
			ux = 60.f * DEG2RADf;
			uz = -88.f * DEG2RADf;
			lx = 5.f * DEG2RADf;
			lz = -86.f * DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingMoth)
		{
			ux = 0.f * DEG2RADf;
			uz = -89.f * DEG2RADf;
			lx = -10.f * DEG2RADf;
			lz = -87.f * DEG2RADf;
		}

		// Make a matrix to rotate around current upper lobe base.
		AuPoint pivot = CurrentConnectLoc();
		AuMatrix4x4 t;
		t.Translate(pivot);
		if(IsLeft())
		{
			t.RotateZ(-uz);
			t.RotateY(-uy);
		}
		else
		{
			t.RotateZ(uz);
			t.RotateY(uy);
		}
		t.RotateX(ux);
		t.Translate(-pivot);
		m_Body->Bone(upperBone) = t * m_Body->Bone(upperBone);

		// Make a matrix to rotate around current lower lobe base.
		t.LoadIdentity();
		t.Translate(pivot);
		if(IsLeft())
		{
			t.RotateZ(-lz);
			t.RotateY(-ly);
		}
		else
		{
			t.RotateZ(lz);
			t.RotateY(ly);
		}
		t.RotateX(lx);
		t.Translate(-pivot);
		m_Body->Bone(foreBone) = t * m_Body->Bone(foreBone);
	}
}

void DpArmAndHand::Run(float hipHt, bool bendToHoldObject, float speedEnc)
{
	float perang = 0;
	if(!IsLeft())
		perang += PIf;
	if(m_DNA.ArmPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.ArmPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2./3;
		else if(m_Which == 2)
			perang += PIf*4./3;
	}
	float persin = m_DNA.Rhythm(AuPose::StandRun, perang, speedEnc);

	float elbowbend, shoulderang, wristbend;
	CalcStandAngles(hipHt, shoulderang, elbowbend, wristbend);

	shoulderang += (10. - 20.*persin)*DEG2RADf;
	if(bendToHoldObject)
	{
		elbowbend = -shoulderang + m_Body->CurrentHeadTilt();
		wristbend = -2*DEG2RADf;
	}
	else
	{
		elbowbend -= (elbowbend + elbowbend*persin);
		if(!m_DNA.WalksOnHands())
			elbowbend += (PIf - elbowbend)/3.;
	}

	// Position bones.
	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	SplayShoulder(m_Splay);
	LiftShoulder(m_Lift);
}

// Puts wing into standing resting position.
void DpArmAndHand::StandWing()
{
	if(IsWingSkeletal())
	{
		// This is a complex wing with multiple bones.
		// Wing normal position is straight out ahead of critter along +Y axis,
		// like arms and legs.
		float shoulderang = 0;	// YZ plane rotation
		float elbowbend = 0;		// YZ plane rotation
		float wristbend = 0;		// YZ plane rotation
		float spread = 0;		// XY plane rotation
		float bow = 0;			// XZ plane rotation
		if(m_DNA.ArmType == DhDNA::ARM_WingFeathered)
		{
			// Feathered wing
			shoulderang = 80 * DEG2RADf;
			elbowbend = -125 * DEG2RADf;
			wristbend = -30 * DEG2RADf;
			spread = -PIf/2.f;	// Straight out so they can wrap around body.
			bow = 20 * DEG2RADf;
		}
		else
		{
			// Leathery wing
			shoulderang = 80 * DEG2RADf;
			elbowbend = -115 * DEG2RADf;
			wristbend = -45 * DEG2RADf;
			spread = 40 * DEG2RADf - PIf;
			bow = 10 * DEG2RADf;
		}
		RotateShoulder(shoulderang);
		RotateElbow(elbowbend);
		RotateWrist(wristbend);
		BowWing(bow);
		FlapWing(spread);
	}
	else
	{
		// This is a simple wing with two more-or-less independent lobes.
		DhDNA::BONES upperBone;
		DhDNA::BONES foreBone;
		DhDNA::BONES handBone;
		Bones(upperBone, foreBone, handBone);
		float ux = 0;	// upper wing lobe X rotation
		float lx = 0;	// lower wing lobe X rotation
		float uy = 0;
		float ly = 0;
		float uz = 0;
		float lz = 0;
		if(m_DNA.ArmType == DhDNA::ARM_WingButterfly)
		{
			ux = 45 * DEG2RADf;
			uz = -176 * DEG2RADf;
			lx = -30 * DEG2RADf;
			lz = -173 * DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingDragonFly)
		{
			ux = 0 * DEG2RADf;
			uz = -97 * DEG2RADf;
			lx = -10 * DEG2RADf;
			lz = -94 * DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingFairy)
		{
			ux = 60 * DEG2RADf;
			uz = -137 * DEG2RADf;
			lx = 5 * DEG2RADf;
			lz = -134 * DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingMoth)
		{
			ux = -75 * DEG2RADf;
			uz = -90 * DEG2RADf;	// Lobes must be parallel left/right or they intersect.
			lx = -80 * DEG2RADf;
			lz = -90 * DEG2RADf;	// Lobes must be parallel left/right or they intersect.
			uy = -3 * DEG2RADf;	// Lift folded upper lobes off folded lower lobes a bit
			ly = -1 * DEG2RADf;	// Lift folded upper lobes off folded lower lobes a bit
			if(IsLeft())
			{
				// This helps interleave and reduces polygon surface collisions.
				uy *= 0.5f;
				ly *= 0.5f;
			}
		}

		// Make a matrix to rotate around current upper lobe base.
		AuPoint pivot = CurrentConnectLoc();
		AuMatrix4x4 t;
		t.Translate(pivot);
		if(IsLeft())
		{
			t.RotateZ(-uz);
			t.RotateY(-uy);
		}
		else
		{
			t.RotateZ(uz);
			t.RotateY(uy);
		}
		t.RotateX(ux);
		t.Translate(-pivot);
		m_Body->Bone(upperBone) = t * m_Body->Bone(upperBone);

		// Make a matrix to rotate around current lower lobe base.
		t.LoadIdentity();
		t.Translate(pivot);
		if(IsLeft())
		{
			t.RotateZ(-lz);
			t.RotateY(-ly);
		}
		else
		{
			t.RotateZ(lz);
			t.RotateY(ly);
		}
		t.RotateX(lx);
		t.Translate(-pivot);
		m_Body->Bone(foreBone) = t * m_Body->Bone(foreBone);
	}
}

void DpArmAndHand::Swim(const AuPose& poseRhythm, bool bendToHoldObject)
{
	float perang = 0;
	if(m_DNA.ArmPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.ArmPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2.f/3.f;
		else if(m_Which == 2)
			perang += PIf*4.f/3.f;
	}
	float persin = m_DNA.Rhythm(poseRhythm, perang, 1);

	float shoulderang = (-60.f - 10.f*persin)*DEG2RADf;
	float elbowbend;
	float wristbend;
	if(bendToHoldObject)
	{
		elbowbend = -shoulderang + m_Body->CurrentHeadTilt();
		wristbend = -2*DEG2RADf;
	}
	else
	{
		elbowbend = (55.f - 45.f*persin)*DEG2RADf;
		wristbend = 0;
	}

	// Position bones.
	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	LiftShoulder(m_Lift);
	SplayShoulder(m_Splay);
}

void DpArmAndHand::TreadWater(bool bendToHoldObject)
{
	float perang = 0;
	if(m_DNA.ArmPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.ArmPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2.f/3.f;
		else if(m_Which == 2)
			perang += PIf*4.f/3.f;
	}
	float persin = m_DNA.Rhythm(AuPose::TreadWater, perang, 1);

	float shoulderang = (-60.f - 10.f*persin)*DEG2RADf;
	float elbowbend;
	float wristbend;
	if(bendToHoldObject)
	{
		elbowbend = -shoulderang + m_Body->CurrentHeadTilt();
		wristbend = -2*DEG2RADf;
	}
	else
	{
		elbowbend = (55.f - 45.f*persin)*DEG2RADf;
		wristbend = 0;
	}

	// Position bones.
	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	LiftShoulder(m_Lift);
	SplayShoulder(m_Splay);
}

void DpArmAndHand::ProneCreep(float /*hipHt*/, bool bendToHoldObject, float speedEnc)
{
	float perang = 0;
	if(!IsLeft())
		perang += PIf;
	if(m_DNA.ArmPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.ArmPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2.f/3.f;
		else if(m_Which == 2)
			perang += PIf*4.f/3.f;
	}
	float persin = m_DNA.Rhythm(AuPose::ProneCreep, perang, speedEnc);

	float shoulderang = 15*DEG2RADf - PIf;	// -PIf is straight back, 0 is forward
	shoulderang += (5.f - 10.f*persin)*DEG2RADf;
	float elbowbend;
	float wristbend;
	if(bendToHoldObject)
	{
		elbowbend = -shoulderang + m_Body->CurrentHeadTilt();
		wristbend = -2*DEG2RADf;
	}
	else
	{
		elbowbend = 165*DEG2RADf;			// 0 is no bend, PIf is collapsed on itself
		wristbend = 0.0;
	}

	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	SplayShoulder(m_Splay);
	LiftShoulder(m_Lift);
}

void DpArmAndHand::Walk(float hipHt, bool bendToHoldObject, float speedEnc)
{
	float perang = 0;
	if(!IsLeft())
		perang += PIf;
	if(m_DNA.ArmPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.ArmPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2.f/3.f;
		else if(m_Which == 2)
			perang += PIf*4.f/3.f;
	}
	float persin = m_DNA.Rhythm(AuPose::StandWalk, perang, speedEnc);

	float elbowbend, shoulderang, wristbend;
	CalcStandAngles(hipHt, shoulderang, elbowbend, wristbend);

	shoulderang += (5. - 10.*persin)*DEG2RADf;
	if(bendToHoldObject)
	{
		elbowbend = -shoulderang + m_Body->CurrentHeadTilt();
		wristbend = -2*DEG2RADf;
	}
	else
		elbowbend -= (elbowbend + elbowbend*persin)/2.;

	// Position bones.
	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	SplayShoulder(m_Splay);
	LiftShoulder(m_Lift);
}

void DpArmAndHand::Wave(float headTilt)
{
	float perang = 0;
	if(m_DNA.ArmPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.ArmPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2.f/3.f;
		else if(m_Which == 2)
			perang += PIf*4.f/3.f;
	}
	float persin = m_DNA.Rhythm(AuPose::Beckon, perang, 1);

	float elbowbend = (15.f + 10.f*persin)*DEG2RADf;
	float shoulderang = (85.f + 1.f*persin)*DEG2RADf + headTilt;
	float wristbend;
	if(m_DNA.HasWristBend())
		wristbend = -6.f*DEG2RADf + 10.f*DEG2RADf * persin;
	else
		wristbend = -2.f*DEG2RADf;

	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	SplayShoulder(m_Splay);
	float liftangle = m_Lift + 10.f*persin*DEG2RADf;
	LiftShoulder(liftangle);
}

void DpArmAndHand::WritheFacingDown()
{
	float elbowbend = 10.f*DEG2RADf;
	float shoulderang = -182.f*DEG2RADf;
	float wristbend = 0;

	float perang = 0;
	if(!IsLeft())
		perang += PIf;
	if(m_DNA.ArmPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.ArmPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2.f/3.f;
		else if(m_Which == 2)
			perang += PIf*4.f/3.f;
	}
	float persin = m_DNA.Rhythm(AuPose::WritheFaceDown, perang, 1);

	elbowbend -= 5.f*DEG2RADf*persin;
	shoulderang += 2.f*DEG2RADf*persin;
	float splay = m_Splay + 1.f*DEG2RADf*persin;

	// Position bones.
	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	SplayShoulder(splay);
}

void DpArmAndHand::WritheFacingLeft()
{
	// Position bones.
	if(IsLeft())
	{
		float perang = 0;
		if(!IsLeft())
			perang += PIf;
		if(m_DNA.ArmPairs == 2 && m_Which == 1)
			perang += PIf;
		else if(m_DNA.ArmPairs == 3)
		{
			if(m_Which == 1)
				perang += PIf*2.f/3.f;
			else if(m_Which == 2)
				perang += PIf*4.f/3.f;
		}
		float persin = m_DNA.Rhythm(AuPose::WritheFaceDown, perang, 1);

		float shoulderang = -(75.f + m_Which*8.f + 1.5f*persin)*DEG2RADf;
		RotateShoulder(shoulderang);

		float wristbend;
		if(m_DNA.HasWristBend())
			wristbend = 15.f*DEG2RADf;
		else
			wristbend = -2.f*DEG2RADf;
		RotateWrist(wristbend);

		float elbowbend = (65.f + 3.5f*persin)*DEG2RADf;
		RotateElbow(elbowbend);

		LiftShoulder(-20.f*DEG2RADf);	// Drop suspended arm slightly.
	}
	else
	{
		float shoulderang = -(125.f + m_Which*10.f)*DEG2RADf;
		RotateShoulder(shoulderang);

		float wristbend;
		if(m_DNA.HasWristBend())
			wristbend = -25.f*DEG2RADf;
		else
			wristbend = -2.f*DEG2RADf;
		RotateWrist(wristbend);

		float elbowbend = 10.f*DEG2RADf;
		RotateElbow(elbowbend);
	}

	SplayShoulder(m_Splay);
}

void DpArmAndHand::WritheFacingRight()
{
	// Position bones.
	if(!IsLeft())
	{
		float perang = 0;
		if(!IsLeft())
			perang += PIf;
		if(m_DNA.ArmPairs == 2 && m_Which == 1)
			perang += PIf;
		else if(m_DNA.ArmPairs == 3)
		{
			if(m_Which == 1)
				perang += PIf*2.f/3.f;
			else if(m_Which == 2)
				perang += PIf*4.f/3.f;
		}
		float persin = m_DNA.Rhythm(AuPose::WritheFaceDown, perang, 1);

		float shoulderang = -(75.f + m_Which*8.f + 1.5f*persin)*DEG2RADf;
		RotateShoulder(shoulderang);

		float wristbend;
		if(m_DNA.HasWristBend())
			wristbend = 15.f*DEG2RADf;
		else
			wristbend = -2.f*DEG2RADf;
		RotateWrist(wristbend);

		float elbowbend = (65.f + 3.5f*persin)*DEG2RADf;
		RotateElbow(elbowbend);

		LiftShoulder(-20.f*DEG2RADf);	// Drop suspended arm slightly.
	}
	else
	{
		float shoulderang = -(125.f + m_Which*10.f)*DEG2RADf;
		RotateShoulder(shoulderang);

		float wristbend;
		if(m_DNA.HasWristBend())
			wristbend = -25.f*DEG2RADf;
		else
			wristbend = -2.f*DEG2RADf;
		RotateWrist(wristbend);

		float elbowbend = 10.f*DEG2RADf;
		RotateElbow(elbowbend);
	}

	SplayShoulder(m_Splay);
}

void DpArmAndHand::WritheFacingUp()
{
	float elbowbend = 3.f*DEG2RADf;
	float shoulderang = -182.f*DEG2RADf;
	float wristbend = 0;

	float perang = 0;
	if(!IsLeft())
		perang += PIf;
	if(m_DNA.ArmPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.ArmPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2.f/3.f;
		else if(m_Which == 2)
			perang += PIf*4.f/3.f;
	}
	float persin = m_DNA.Rhythm(AuPose::WritheFaceUp, perang, 1);

	elbowbend -= 2.f*DEG2RADf*persin;
	float splay = m_Splay + 1.f*DEG2RADf*persin;

	// Position bones.
	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	SplayShoulder(splay);
}

void DpArmAndHand::ReceptiveFacingUp()
{
	float elbowbend = 30.f*DEG2RADf;
	float shoulderang = -182.f*DEG2RADf;
	float wristbend = 0;

	float perang = 0;
	if(!IsLeft())
		perang += PIf;
	if(m_DNA.ArmPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.ArmPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2.f/3.f;
		else if(m_Which == 2)
			perang += PIf*4.f/3.f;
	}
	float persin = m_DNA.Rhythm(AuPose::ReceptiveMate, perang, 1);

	elbowbend -= 2.f*DEG2RADf*persin;
	float splay = m_Splay + 1.f*DEG2RADf*persin;

	// Position bones.
	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	SplayShoulder(splay);
}

void DpArmAndHand::WritheWingFacingDown()
{
	float persin = m_DNA.Rhythm(AuPose::WritheFaceDown, 0, 1);

	if(IsWingSkeletal())
	{
		// This is a complex wing with multiple bones.
		// Wing normal position is straight out ahead of critter along +Y axis,
		// like arms and legs.
		float shoulderang = 0;	// YZ plane rotation
		float elbowbend = 0;		// YZ plane rotation
		float wristbend = 0;		// YZ plane rotation
		float spread = 0;		// XY plane rotation
		float bow = 0;			// XZ plane rotation
		if(m_DNA.ArmType == DhDNA::ARM_WingFeathered)
		{
			// Feathered wing
			shoulderang = 10.f * DEG2RADf;
			elbowbend = -25.f * DEG2RADf;
			wristbend = -10.f * DEG2RADf;
			spread = -85.f * DEG2RADf;
			bow = 15.f * DEG2RADf;
		}
		else
		{
			// Leathery wing
			shoulderang = 10.f * DEG2RADf;
			elbowbend = -15.f * DEG2RADf;
			wristbend = -15.f * DEG2RADf;
			spread = -85.f * DEG2RADf;
			bow = 10.f * DEG2RADf;
		}

		bow += 2.f*DEG2RADf*persin;

		RotateShoulder(shoulderang);
		RotateElbow(elbowbend);
		RotateWrist(wristbend);
		BowWing(bow);
		FlapWing(spread);
	}
	else
	{
		// This is a simple wing with two more-or-less independent lobes.
		DhDNA::BONES upperBone;
		DhDNA::BONES foreBone;
		DhDNA::BONES handBone;
		Bones(upperBone, foreBone, handBone);
		float ux = 0;	// upper wing lobe X rotation
		float lx = 0;	// lower wing lobe X rotation
		float uy = 0;
		float ly = 0;
		float uz = 0;
		float lz = 0;
		if(m_DNA.ArmType == DhDNA::ARM_WingButterfly)
		{
			ux = 45.f * DEG2RADf;
			uz = -86.f * DEG2RADf;
			lx = -30.f * DEG2RADf;
			lz = -83.f * DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingDragonFly)
		{
			ux = 0.f * DEG2RADf;
			uz = -89.f * DEG2RADf;
			lx = -10.f * DEG2RADf;
			lz = -87.f * DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingFairy)
		{
			ux = 60.f * DEG2RADf;
			uz = -85.f * DEG2RADf;
			lx = 5.f * DEG2RADf;
			lz = -82.f * DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingMoth)
		{
			ux = 0.f * DEG2RADf;
			uz = -86.f * DEG2RADf;
			lx = -10.f * DEG2RADf;
			lz = -83.f * DEG2RADf;
		}

		uz += 2.5f*DEG2RADf*persin;
		lz += 2.5f*DEG2RADf*persin;

		// Make a matrix to rotate around current upper lobe base.
		AuPoint pivot = CurrentConnectLoc();
		AuMatrix4x4 t;
		t.Translate(pivot);
		if(IsLeft())
		{
			t.RotateZ(-uz);
			t.RotateY(-uy);
		}
		else
		{
			t.RotateZ(uz);
			t.RotateY(uy);
		}
		t.RotateX(ux);
		t.Translate(-pivot);
		m_Body->Bone(upperBone) = t * m_Body->Bone(upperBone);

		// Make a matrix to rotate around current lower lobe base.
		t.LoadIdentity();
		t.Translate(pivot);
		if(IsLeft())
		{
			t.RotateZ(-lz);
			t.RotateY(-ly);
		}
		else
		{
			t.RotateZ(lz);
			t.RotateY(ly);
		}
		t.RotateX(lx);
		t.Translate(-pivot);
		m_Body->Bone(foreBone) = t * m_Body->Bone(foreBone);
	}
}

void DpArmAndHand::WritheWingFacingLeft()
{
	if(IsLeft())
		WritheWingFacingDown();
	else
		DeadWingFacingLeft();
}

void DpArmAndHand::WritheWingFacingRight()
{
	if(!IsLeft())
		WritheWingFacingDown();
	else
		DeadWingFacingRight();
}

void DpArmAndHand::WritheWingFacingUp()
{
	float persin = m_DNA.Rhythm(AuPose::WritheFaceDown, 0, 1);

	if(IsWingSkeletal())
	{
		// This is a complex wing with multiple bones.
		// Wing normal position is straight out ahead of critter along +Y axis,
		// like arms and legs.
		float shoulderang = 0;	// YZ plane rotation
		float elbowbend = 0;		// YZ plane rotation
		float wristbend = 0;		// YZ plane rotation
		float spread = 0;		// XY plane rotation
		float bow = 0;			// XZ plane rotation
		if(m_DNA.ArmType == DhDNA::ARM_WingFeathered)
		{
			// Feathered wing
			shoulderang = 10.f * DEG2RADf;
			elbowbend = -25.f * DEG2RADf;
			wristbend = -10.f * DEG2RADf;
			spread = -88.f * DEG2RADf;
			bow = 15.f * DEG2RADf;
		}
		else
		{
			// Leathery wing
			shoulderang = 10.f * DEG2RADf;
			elbowbend = -15.f * DEG2RADf;
			wristbend = -15.f * DEG2RADf;
			spread = -90.f * DEG2RADf;
			bow = 10.f * DEG2RADf;
		}

		bow += 5.f*DEG2RADf*persin;

		RotateShoulder(shoulderang);
		RotateElbow(elbowbend);
		RotateWrist(wristbend);
		BowWing(bow);
		FlapWing(spread);
	}
	else
	{
		// This is a simple wing with two more-or-less independent lobes.
		DhDNA::BONES upperBone;
		DhDNA::BONES foreBone;
		DhDNA::BONES handBone;
		Bones(upperBone, foreBone, handBone);
		float ux = 0;	// upper wing lobe X rotation
		float lx = 0;	// lower wing lobe X rotation
		float uy = 0;
		float ly = 0;
		float uz = 0;
		float lz = 0;
		if(m_DNA.ArmType == DhDNA::ARM_WingButterfly)
		{
			ux = 45.f * DEG2RADf;
			uz = -86.f * DEG2RADf;
			lx = -30.f * DEG2RADf;
			lz = -83.f * DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingDragonFly)
		{
			ux = 0.f * DEG2RADf;
			uz = -89.f * DEG2RADf;
			lx = -10.f * DEG2RADf;
			lz = -87.f * DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingFairy)
		{
			ux = 60.f * DEG2RADf;
			uz = -85.f * DEG2RADf;
			lx = 5.f * DEG2RADf;
			lz = -82.f * DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingMoth)
		{
			ux = 0.f * DEG2RADf;
			uz = -86.f * DEG2RADf;
			lx = -10.f * DEG2RADf;
			lz = -83.f * DEG2RADf;
		}

		uz += 1.f*DEG2RADf*persin;
		lz += 1.f*DEG2RADf*persin;

		// Make a matrix to rotate around current upper lobe base.
		AuPoint pivot = CurrentConnectLoc();
		AuMatrix4x4 t;
		t.Translate(pivot);
		if(IsLeft())
		{
			t.RotateZ(-uz);
			t.RotateY(-uy);
		}
		else
		{
			t.RotateZ(uz);
			t.RotateY(uy);
		}
		t.RotateX(ux);
		t.Translate(-pivot);
		m_Body->Bone(upperBone) = t * m_Body->Bone(upperBone);

		// Make a matrix to rotate around current lower lobe base.
		t.LoadIdentity();
		t.Translate(pivot);
		if(IsLeft())
		{
			t.RotateZ(-lz);
			t.RotateY(-ly);
		}
		else
		{
			t.RotateZ(lz);
			t.RotateY(ly);
		}
		t.RotateX(lx);
		t.Translate(-pivot);
		m_Body->Bone(foreBone) = t * m_Body->Bone(foreBone);
	}
}

// Rotates around X axis
void DpArmAndHand::RotateShoulder(float angle)
{
	if(angle == 0.0)
		return;

	DhDNA::BONES upperBone;
	DhDNA::BONES foreBone;
	DhDNA::BONES handBone;
	Bones(upperBone, foreBone, handBone);

	// Make a matrix to rotate around current shoulder location.
	AuPoint pivot = CurrentConnectLoc();
	AuMatrix4x4 t;
	t.Translate(pivot);
	t.RotateX(angle);
	t.Translate(-pivot);

	// Rotate bones at shoulder.
	if(IsWing() && !IsWingSkeletal())
	{
		// Rotate only the upper wing lobe.
		m_Body->Bone(upperBone) = t * m_Body->Bone(upperBone);
	}
	else
	{
		m_Body->Bone(upperBone) = t * m_Body->Bone(upperBone);
		m_Body->Bone(foreBone) = t * m_Body->Bone(foreBone);
		m_Body->Bone(handBone) = t * m_Body->Bone(handBone);
	}
}

// YZ plane rotation
void DpArmAndHand::RotateElbow(float angle)
{
	if(angle == 0)
		return;

	DhDNA::BONES upperBone;
	DhDNA::BONES foreBone;
	DhDNA::BONES handBone;
	Bones(upperBone, foreBone, handBone);

	// Rotate bones at elbow.
	if(IsWing() && !IsWingSkeletal())
	{
		// Make a matrix to rotate around current elbow location.
		AuPoint pivot = CurrentConnectLoc();
		AuMatrix4x4 t;
		t.Translate(pivot);
		t.RotateX(angle);
		t.Translate(-pivot);

		// Rotate only the lower wing lobe.
		m_Body->Bone(foreBone) = t * m_Body->Bone(foreBone);
	}
	else
	{
		// Make a matrix to rotate around current elbow location.
		AuPoint pivot = CurrentElbowLoc();
		AuMatrix4x4 t;
		t.Translate(pivot);
		t.RotateX(angle);
		t.Translate(-pivot);

		m_Body->Bone(foreBone) = t * m_Body->Bone(foreBone);
		m_Body->Bone(handBone) = t * m_Body->Bone(handBone);
	}
}

// YZ plane rotation
void DpArmAndHand::RotateWrist(float angle)
{
	if(angle == 0)
		return;

	DhDNA::BONES upperBone;
	DhDNA::BONES foreBone;
	DhDNA::BONES handBone;
	Bones(upperBone, foreBone, handBone);

	// Make a matrix to rotate around current knee location.
	AuPoint pivot = CurrentWristLoc();
	AuMatrix4x4 t;
	t.Translate(pivot);
	t.RotateX(angle);
	t.Translate(-pivot);

	// Rotate bones at elbow.
	m_Body->Bone(handBone) = t * m_Body->Bone(handBone);
}

// Rotates around Z at each joint.
void DpArmAndHand::BowWing(float angle)
{
	if(angle == 0)
		return;

	DhDNA::BONES upperBone;
	DhDNA::BONES foreBone;
	DhDNA::BONES handBone;
	Bones(upperBone, foreBone, handBone);

	// Left wing angle is reversed.
	if(IsLeft())
		angle = -angle;

	// Make a matrix to rotate around current shoulder location.
	AuPoint pivot = CurrentElbowLoc();
	AuMatrix4x4 t;
	t.Translate(pivot);
	t.RotateZ(angle);
	t.Translate(-pivot);

	// Rotate bones at elbow.
	m_Body->Bone(foreBone) = t * m_Body->Bone(foreBone);
	m_Body->Bone(handBone) = t * m_Body->Bone(handBone);

	// Rotate tip again
	pivot = CurrentWristLoc();
	t.LoadIdentity();
	t.Translate(pivot);
	t.RotateZ(angle);
	t.Translate(-pivot);

	// Rotate bones at wrist.
	m_Body->Bone(handBone) = t * m_Body->Bone(handBone);
}

// XY plane rotation
void DpArmAndHand::FlapWing(float angle)
{
	if(angle == 0)
		return;

	DhDNA::BONES upperBone;
	DhDNA::BONES foreBone;
	DhDNA::BONES handBone;
	Bones(upperBone, foreBone, handBone);

	// Left wing angle is reversed.
	if(IsLeft())
		angle = -angle;

	// Make a matrix to rotate around current shoulder location.
	AuPoint pivot = CurrentConnectLoc();
	AuMatrix4x4 t;
	t.Translate(pivot);
	t.RotateZ(angle);
	t.Translate(-pivot);

	// Rotate bones at shoulder.
	m_Body->Bone(upperBone) = t * m_Body->Bone(upperBone);
	m_Body->Bone(foreBone) = t * m_Body->Bone(foreBone);
	m_Body->Bone(handBone) = t * m_Body->Bone(handBone);
}

void DpArmAndHand::Eat(float hipHt)
{
	float perang = 0;
	if(m_DNA.ArmPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.ArmPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2.f/3.f;
		else if(m_Which == 2)
			perang += PIf*4.f/3.f;
	}
	float persin = m_DNA.Rhythm(AuPose::Eat, perang, 1);

	// Folded limb angles.
	float elbowfold = 165.f*DEG2RADf;			// 0 is no bend, PIf is collapsed
	float shoulderfold = 15.f*DEG2RADf - PIf;	// -PIf is straight back, 0 is forward
	float wristfold = 0;
	if(m_DNA.HasWristBend())
		wristfold = 5.f*DEG2RADf;
	else
		wristfold = -2.f*DEG2RADf;

	// Standing joint angles.
	float elbowstand, shoulderstand, wriststand;
	CalcStandAngles(hipHt, shoulderstand, elbowstand, wriststand);

	// Cycle between folded and standing limb position.
	float shouldersweep = shoulderstand - shoulderfold;
	float shoulderang = shoulderfold +
		(shouldersweep + shouldersweep*persin)/2.f;
	float elbowsweep = elbowstand - elbowfold;
	float elbowbend = elbowfold + (elbowsweep + elbowsweep*persin)/2.f;
	float wristsweep = wriststand - wristfold;
	float wristbend = wristfold + (wristsweep + wristsweep*persin)/2.f;

	// Position bones.
	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	SplayShoulder(m_Splay);
	LiftShoulder(m_Lift);
}

void DpArmAndHand::Climb(bool bendToHoldObject, float speedEnc)
{
	float perang = 0;
	if(!IsLeft())
		perang += PIf;
	if(m_DNA.ArmPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.ArmPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2.f/3.f;
		else if(m_Which == 2)
			perang += PIf*4.f/3.f;
	}
	float persin = m_DNA.Rhythm(AuPose::LadderClimb, perang, speedEnc);

	//float elbowbend = (100.f + 50.f*persin)*DEG2RADf;
	float shoulderang = -(45.f + 30.f*persin)*DEG2RADf;
	float elbowbend;
	float wristbend;
	if(bendToHoldObject)
	{
		elbowbend = -shoulderang + m_Body->CurrentHeadTilt();
		wristbend = -2*DEG2RADf;
	}
	else
	{
		elbowbend = 75.f*DEG2RADf - shoulderang;
		wristbend = -2.f*DEG2RADf;
	}

	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	SplayShoulder(m_Splay);
	LiftShoulder(m_Lift);
}

void DpArmAndHand::Ladder(bool bendToHoldObject)
{
	float shoulderang = -75*DEG2RADf;	// -PIf is straight back, 0 is forward
	float elbowbend;
	float wristbend;

	if(bendToHoldObject)
	{
		elbowbend = -shoulderang + m_Body->CurrentHeadTilt();
		wristbend = -2*DEG2RADf;
	}
	else
	{
		elbowbend = 130*DEG2RADf;		// 0 is no bend, PIf is collapsed on itself
		wristbend = -2.f*DEG2RADf;
	}

	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	SplayShoulder(m_Splay);
	LiftShoulder(m_Lift);
}

void DpArmAndHand::Pummel(float headTilt)
{
	float perang = 0;
	if(m_DNA.ArmPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.ArmPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2.f/3.f;
		else if(m_Which == 2)
			perang += PIf*4.f/3.f;
	}
	float persin = m_DNA.Rhythm(AuPose::Strike, perang, 1);

	float elbowbend = (75.f - 60.f*persin)*DEG2RADf;
	float shoulderang = (90.f + 70.f*persin)*DEG2RADf - PIf + headTilt;
	float wristbend;
	if(m_DNA.HasWristBend())
		wristbend = 80.f*DEG2RADf * persin;
	else
		wristbend = -2.f*DEG2RADf;

	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	SplayShoulder(m_Splay);
	LiftShoulder(m_Lift);
}

// Position for standing at hipHt.
void DpArmAndHand::Relax(float hipHt, bool bendToHoldObject)
{
	float elbowbend, shoulderang, wristbend;
	CalcStandAngles(hipHt, shoulderang, elbowbend, wristbend);

	if(bendToHoldObject)
	{
		elbowbend = -shoulderang + m_Body->CurrentHeadTilt();
		wristbend = -2*DEG2RADf;
	}

	// Position bones.
	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	SplayShoulder(m_Splay);
	LiftShoulder(m_Lift);
}

// Position for straddle.
void DpArmAndHand::Straddle(float hipHt, bool bendToHoldObject)
{
	float elbowbend, shoulderang, wristbend;
	CalcStandAngles(hipHt, shoulderang, elbowbend, wristbend);

	if(bendToHoldObject)
	{
		elbowbend = -shoulderang + m_Body->CurrentHeadTilt();
		wristbend = -2*DEG2RADf;
	}
	else
	{
		if(m_Which == 0)
			elbowbend += 45*DEG2RADf;	// Bend first set of arms up for control.
	}

	// Position bones.
	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	SplayShoulder(m_Splay);
	LiftShoulder(m_Lift);
}

void DpArmAndHand::Throw(float headTilt)
{
	float perang = 0;
	if(m_DNA.ArmPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.ArmPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2.f/3.f;
		else if(m_Which == 2)
			perang += PIf*4.f/3.f;
	}
	float persin = m_DNA.Rhythm(AuPose::Throw, perang, 1);

	float elbowbend = (75.f - 70.f*persin)*DEG2RADf;
	float shoulderang = -(15.f + 15.f*persin)*DEG2RADf + headTilt;	// -PIf=straight back
	float wristbend;
	if(m_DNA.HasWristBend())
		wristbend = -15.f*DEG2RADf * persin;
	else
		wristbend = -2.f*DEG2RADf;

	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	SplayShoulder(m_Splay);
	LiftShoulder(m_Lift);
}

void DpArmAndHand::PsionicWave(float headTilt)
{
	float perang = 0;
	if(!IsLeft())
		perang += PIf;
	if(m_DNA.ArmPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.ArmPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2.f/3.f;
		else if(m_Which == 2)
			perang += PIf*4.f/3.f;
	}
	float persin = m_DNA.Rhythm(AuPose::Psionic, perang, 1);

	float elbowbend = (40.f + 20.f*persin)*DEG2RADf;
	float shoulderang = -10.f*DEG2RADf + headTilt;
	float wristbend = 0;
	if(m_DNA.HasWristBend())
		wristbend = -elbowbend;
	else
		wristbend = -2.f*DEG2RADf;

	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	SplayShoulder(m_Splay);
	LiftShoulder(m_Lift);
}

// Cyclic position for flying.
void DpArmAndHand::FlyWing(int speed)
{
	float flapsin;
	if(speed == 0)
		flapsin = 0;
	else if(speed == 1)
		flapsin = m_DNA.Rhythm(AuPose::Fly, 0, 1);
	else
		flapsin = m_DNA.Rhythm(AuPose::FlyFast, 0, 1);

	if(IsWingSkeletal())
	{
		// This is a complex wing with multiple bones.
		// Wing normal position is straight out ahead of critter along +Y axis,
		// like arms and legs.
		float shoulderang = 0;	// YZ plane rotation
		float elbowbend = 0;		// YZ plane rotation
		float wristbend = 0;		// YZ plane rotation
		float spread = 0;		// XY plane rotation
		float bow = 0;			// XZ plane rotation
		if(m_DNA.ArmType == DhDNA::ARM_WingFeathered)
		{
			// Feathered wing
			shoulderang = 10.f * DEG2RADf;
			elbowbend = -25.f * DEG2RADf;
			wristbend = -10.f * DEG2RADf;
			spread = -PIf/2.f;	// Straight out so they can wrap around body.
			bow = 35.f * DEG2RADf;
		}
		else
		{
			// Leathery wing
			shoulderang = 10.f * DEG2RADf;
			elbowbend = -15.f * DEG2RADf;
			wristbend = -15.f * DEG2RADf;
			spread = 40.f * DEG2RADf - PIf;
			bow = 25.f * DEG2RADf;
		}

		// Motion
		//shoulderang += flapsin * 6.*DEG2RADf;
		//elbowbend += flapsin * 4.*DEG2RADf;
		//wristbend += flapsin * 2.*DEG2RADf;
		bow += flapsin * 15.f*DEG2RADf;
		spread += flapsin * 40.f*DEG2RADf;

		RotateShoulder(shoulderang);
		RotateElbow(elbowbend);
		RotateWrist(wristbend);
		BowWing(bow);
		FlapWing(spread);
	}
	else
	{
		// This is a simple wing with two more-or-less independent lobes.
		DhDNA::BONES upperBone;
		DhDNA::BONES foreBone;
		DhDNA::BONES handBone;
		Bones(upperBone, foreBone, handBone);
		float ux = 0;	// upper wing lobe X rotation
		float lx = 0;	// lower wing lobe X rotation
		float uy = 0;
		float ly = 0;
		float uz = 0;
		float lz = 0;
		if(m_DNA.ArmType == DhDNA::ARM_WingButterfly)
		{
			ux = 45.f * DEG2RADf;
			uz = -146.f * DEG2RADf;
			lx = -30.f * DEG2RADf;
			lz = -143.f * DEG2RADf;

			// Motion
			uz += flapsin * 30.f*DEG2RADf;
			lz += flapsin * 30.f*DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingDragonFly)
		{
			ux = 0.f * DEG2RADf;
			uz = -107.f * DEG2RADf;
			lx = -10.f * DEG2RADf;
			lz = -104.f * DEG2RADf;

			// Motion
			uz += flapsin * 10.f*DEG2RADf;
			lz += flapsin * 10.f*DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingFairy)
		{
			ux = 60.f * DEG2RADf;
			uz = -137.f * DEG2RADf;
			lx = 5.f * DEG2RADf;
			lz = -134.f * DEG2RADf;

			// Motion
			uz += flapsin * 40.f*DEG2RADf;
			lz += flapsin * 40.f*DEG2RADf;
		}
		else if(m_DNA.ArmType == DhDNA::ARM_WingMoth)
		{
			ux = 0.f * DEG2RADf;
			uz = -127.f * DEG2RADf;
			lx = -10.f * DEG2RADf;
			lz = -124.f * DEG2RADf;

			// Motion
			uz += flapsin * 30.f*DEG2RADf;
			lz += flapsin * 30.f*DEG2RADf;
		}

		// Make a matrix to rotate around current upper lobe base.
		AuPoint pivot = CurrentConnectLoc();
		AuMatrix4x4 t;
		t.Translate(pivot);
		if(IsLeft())
		{
			t.RotateZ(-uz);
			t.RotateY(-uy);
		}
		else
		{
			t.RotateZ(uz);
			t.RotateY(uy);
		}
		t.RotateX(ux);
		t.Translate(-pivot);
		m_Body->Bone(upperBone) = t * m_Body->Bone(upperBone);

		// Make a matrix to rotate around current lower lobe base.
		t.LoadIdentity();
		t.Translate(pivot);
		if(IsLeft())
		{
			t.RotateZ(-lz);
			t.RotateY(-ly);
		}
		else
		{
			t.RotateZ(lz);
			t.RotateY(ly);
		}
		t.RotateX(lx);
		t.Translate(-pivot);
		m_Body->Bone(foreBone) = t * m_Body->Bone(foreBone);
	}
}

// Position folded for flying.
void DpArmAndHand::Fold(bool bendToHoldObject)
{
	float shoulderang = 15*DEG2RADf - PIf;	// -PIf is straight back, 0 is forward
	float elbowbend;
	float wristbend;
	if(bendToHoldObject)
	{
		elbowbend = -shoulderang + m_Body->CurrentHeadTilt();
		wristbend = -2*DEG2RADf;
	}
	else
	{
		elbowbend = 165*DEG2RADf;			// 0 is no bend, PIf is collapsed on itself
		wristbend = 0.0;
	}

	RotateShoulder(shoulderang);
	RotateWrist(wristbend);
	RotateElbow(elbowbend);
	SplayShoulder(m_Splay);
	LiftShoulder(m_Lift);
}

// Rotate at shoulder around Z. Positive moves knee away from body.
void DpArmAndHand::LiftShoulder(float angle)
{
	DhDNA::BONES upperBone;
	DhDNA::BONES foreBone;
	DhDNA::BONES handBone;
	Bones(upperBone, foreBone, handBone);

	// Right arm angle is reversed.
	if(!IsLeft())
		angle = -angle;

	// Make a matrix to rotate around current shoulder location.
	AuPoint pivot = CurrentConnectLoc();
	AuMatrix4x4 t;
	t.Translate(pivot);
	t.RotateY(angle);
	t.Translate(-pivot);

	// Rotate bones at shoulder.
	m_Body->Bone(upperBone) = t * m_Body->Bone(upperBone);
	m_Body->Bone(foreBone) = t * m_Body->Bone(foreBone);
	m_Body->Bone(handBone) = t * m_Body->Bone(handBone);
}

// Rotate at shoulder around Z. Positive moves elbow away from body.
void DpArmAndHand::SplayShoulder(float angle)
{
	if(angle == 0.0)
		return;

	DhDNA::BONES upperBone;
	DhDNA::BONES foreBone;
	DhDNA::BONES handBone;
	Bones(upperBone, foreBone, handBone);

	// Right arm angle is reversed.
	if(IsLeft())
		angle = -angle;

	// Make a matrix to rotate around current shoulder location.
	AuPoint pivot = CurrentConnectLoc();
	AuMatrix4x4 t;
	t.Translate(pivot);
	t.RotateZ(angle);
	t.Translate(-pivot);

	// Rotate bones at shoulder.
	m_Body->Bone(upperBone) = t * m_Body->Bone(upperBone);
	m_Body->Bone(foreBone) = t * m_Body->Bone(foreBone);
	m_Body->Bone(handBone) = t * m_Body->Bone(handBone);
}

void DpArmAndHand::RoughExtentLocs(AuPointArray& locs) const
{
	DhDNA::BONES upperBone;
	DhDNA::BONES foreBone;
	DhDNA::BONES handBone;
	Bones(upperBone, foreBone, handBone);

	// Calculate shoulder position for origin of vertices.
	AuPoint loc = ConnectLoc();

	// Dimensions
	float ul = m_Body->UpperArmLength();
	float fl = m_Body->ForeArmLength();
	float hl = m_Body->HandLength();

	if(IsWing() && !IsWingSkeletal())
	{
		// Skeletal wings are treated like arms.
		// Two lobed structure, upper arm leading and fore arm trailing.
		// Test chunk intersection.
		m_Body->RoughExtentChunk(locs, upperBone, loc, ul);
		m_Body->RoughExtentChunk(locs, foreBone, loc, fl);
	}
	else
	{
		//bool isTentacle;
		//if(m_DNA.ArmType == DhDNA::ARM_Tentacle)
			//isTentacle = true;
		//else
			//isTentacle = false;

		// Arm
		// Test chunk intersection.
		m_Body->RoughExtentChunk(locs, upperBone, loc, ul);

		loc.y += ul;

		// Test chunk intersection.
		m_Body->RoughExtentChunk(locs, foreBone, loc, fl);

		loc.y += fl;

		// Hand
		m_Body->RoughExtentChunk(locs, handBone, loc, hl);
	}
}

void DpArmAndHand::RenderHitBox(AuGLEffects& re)
{
	DhDNA::BONES upperBone;
	DhDNA::BONES foreBone;
	DhDNA::BONES handBone;
	Bones(upperBone, foreBone, handBone);

	// Calculate shoulder position for origin of vertices.
	AuPoint loc = ConnectLoc();

	// Dimensions
	float ul = m_Body->UpperArmLength();
	float ut = m_Body->UpperArmThickness();
	float fl = m_Body->ForeArmLength();
	float ft = m_Body->ForeArmThickness();
	float hl = m_Body->HandLength();

	//qDebug("DpArmAndHand::CreateSkin - %s %s ul %g ut %g fl %g ft %g hl %g",
			//IsLeft()?"Left":"Right", IsWing()?"Wing":"Arm",
			//ul,ut,fl,ft,hl);

	if(IsWing() && !IsWingSkeletal())
	{
		// Skeletal wings are treated like arms.
		// Two lobed structure, upper arm leading and fore arm trailing.
		// Test chunk intersection.
		m_Body->RenderHitBoxChunk(re, upperBone, loc, ul, ut);
		m_Body->RenderHitBoxChunk(re, foreBone, loc, fl, ft);
	}
	else
	{
		float ht = hl/2.0;
		if(ht > ft)
			ht = ft * 1.1;	// Hands are a tiny bit bigger than the fore arms.

		//bool isTentacle;
		//if(m_DNA.ArmType == DhDNA::ARM_Tentacle)
			//isTentacle = true;
		//else
			//isTentacle = false;

		// Arm
		// Test chunk intersection.
		//float avgrad = (ut + ut + ft + ft)/8.;
		float avgrad;
		if(ut > ft)
			avgrad = ut/2.f;
		else
			avgrad = ft/2.f;
		m_Body->RenderHitBoxChunk(re, upperBone, loc, ul, avgrad);

		loc.y += ul;

		// Test chunk intersection.
		//avgrad = (ft + ft + ht + ht)/8.;
		if(ft > ht)
			avgrad = ft/2.f;
		else
			avgrad = ht/2.f;
		m_Body->RenderHitBoxChunk(re, foreBone, loc, fl, avgrad);

		loc.y += fl;

		// Hand
		// Test chunk intersection.
		//avgrad = (ht + ht)/4.;
		avgrad = ht/2.f;
		m_Body->RenderHitBoxChunk(re, handBone, loc, hl, avgrad);
	}
}

bool DpArmAndHand::SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const
{
	bool hitMe = false;

	DhDNA::BONES upperBone;
	DhDNA::BONES foreBone;
	DhDNA::BONES handBone;
	Bones(upperBone, foreBone, handBone);

	// Calculate shoulder position for origin of vertices.
	AuPoint loc = ConnectLoc();

	// Dimensions
	float ul = m_Body->UpperArmLength();
	float ut = m_Body->UpperArmThickness();
	float fl = m_Body->ForeArmLength();
	float ft = m_Body->ForeArmThickness();
	float hl = m_Body->HandLength();

	//qDebug("DpArmAndHand::CreateSkin - %s %s ul %g ut %g fl %g ft %g hl %g",
			//IsLeft()?"Left":"Right", IsWing()?"Wing":"Arm",
			//ul,ut,fl,ft,hl);

	if(IsWing() && !IsWingSkeletal())
	{
		// Skeletal wings are treated like arms.
		// Two lobed structure, upper arm leading and fore arm trailing.
		// Test chunk intersection.
		if(m_Body->SightChunk(upperBone, loc, loc + AuPoint(0,ul), ut, eyeLoc, sightedLoc))
		{
			sightedPart = upperBone;
			hitMe = true;
		}
		if(m_Body->SightChunk(foreBone, loc, loc + AuPoint(0,fl), ft, eyeLoc, sightedLoc))
		{
			sightedPart = foreBone;
			hitMe = true;
		}
	}
	else
	{
		float ht = hl/2.0;
		if(ht > ft)
			ht = ft * 1.1;	// Hands are a tiny bit bigger than the fore arms.

		//bool isTentacle;
		//if(m_DNA.ArmType == DhDNA::ARM_Tentacle)
			//isTentacle = true;
		//else
			//isTentacle = false;

		// Arm
		// Test chunk intersection.
		//float avgrad = (ut + ut + ft + ft)/8.;
		float avgrad;
		if(ut > ft)
			avgrad = ut/2.f;
		else
			avgrad = ft/2.f;
		if(m_Body->SightChunk(upperBone, loc, loc + AuPoint(0,ul), avgrad, eyeLoc, sightedLoc))
		{
			sightedPart = upperBone;
			hitMe = true;
		}

		loc.y += ul;

		// Test chunk intersection.
		//avgrad = (ft + ft + ht + ht)/8.;
		if(ft > ht)
			avgrad = ft/2.f;
		else
			avgrad = ht/2.f;
		if(m_Body->SightChunk(foreBone, loc, loc + AuPoint(0,fl), avgrad, eyeLoc, sightedLoc))
		{
			sightedPart = foreBone;
			hitMe = true;
		}

		loc.y += fl;

		// Hand
		// Test chunk intersection.
		//avgrad = (ht + ht)/4.;
		avgrad = ht/2.f;
		if(m_Body->SightChunk(handBone, loc, loc + AuPoint(0,hl), avgrad, eyeLoc, sightedLoc))
		{
			sightedPart = handBone;
			hitMe = true;
		}
	}

	return hitMe;
}

float DpArmAndHand::Volume() const
{
	// Dimensions
	float ul = m_Body->UpperArmLength();
	float ut = m_Body->UpperArmThickness();
	float fl = m_Body->ForeArmLength();
	float ft = m_Body->ForeArmThickness();
	float hl = m_Body->HandLength(true);
	float ht = hl/2.0;
	if(ht > ft)
		ht = ft * 1.1;	// Hands are a tiny bit bigger than the fore arms.

	// Convert diameters to radii squared.
	ut /= 2;
	ut *= ut;
	ft /= 2;
	ft *= ft;
	ht /= 2;
	ht *= ht;

	float uca = ut * PIf;	// approx upper arm cross sectional area
	float fca = ft * PIf;	// approx fore arm cross sectional area
	float hca = ht * PIf;	// approx hand cross sectional area

	float volume = uca * ul + fca * fl + hca * hl;
	//qDebug() << "DpArmAndHand::Volume upper" << uca << ut << ul << "fore" << fca << ft << fl
		//<< "hand" << hca << ht << hl << volume;
	return volume;
}

quint64 DpArmAndHand::MemoryUsage() const
{
	quint64 mem = DpBodyPart::MemoryUsage();
	mem += sizeof(m_Lift);
	mem += sizeof(m_Splay);
	mem += sizeof(m_Which);
	mem += sizeof(m_ElbowLoc);
	mem += sizeof(m_WristLoc);
	mem += sizeof(m_HandLoc);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

void DpLeftArmAndHand::Bones(DhDNA::BONES& upper, DhDNA::BONES& fore,
		DhDNA::BONES& hand) const
{
	switch(m_Which)
	{
	default:
		Q_ASSERT(false);
	case 0:
		upper = DhDNA::BONE_UpperArm0L;
		fore = DhDNA::BONE_ForeArm0L;
		hand = DhDNA::BONE_Hand0L;
		break;
	case 1:
		upper = DhDNA::BONE_UpperArm1L;
		fore = DhDNA::BONE_ForeArm1L;
		hand = DhDNA::BONE_Hand1L;
		break;
	case 2:
		upper = DhDNA::BONE_UpperArm2L;
		fore = DhDNA::BONE_ForeArm2L;
		hand = DhDNA::BONE_Hand2L;
		break;
	}
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

void DpRightArmAndHand::Bones(DhDNA::BONES& upper, DhDNA::BONES& fore,
		DhDNA::BONES& hand) const
{
	switch(m_Which)
	{
	default:
		Q_ASSERT(false);
	case 0:
		upper = DhDNA::BONE_UpperArm0R;
		fore = DhDNA::BONE_ForeArm0R;
		hand = DhDNA::BONE_Hand0R;
		break;
	case 1:
		upper = DhDNA::BONE_UpperArm1R;
		fore = DhDNA::BONE_ForeArm1R;
		hand = DhDNA::BONE_Hand1R;
		break;
	case 2:
		upper = DhDNA::BONE_UpperArm2R;
		fore = DhDNA::BONE_ForeArm2R;
		hand = DhDNA::BONE_Hand2R;
		break;
	}
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

DpLegAndFoot::DpLegAndFoot(const DpLegAndFoot &rhs) : DpBodyPart(rhs)
{
	m_Which = rhs.m_Which;
	m_Splay = rhs.m_Splay;
	m_KneeLoc = rhs.m_KneeLoc;
	m_AnkleLoc = rhs.m_AnkleLoc;
}
void DpLegAndFoot::CreateSkin(AuGLEffects& re, AuGLModelCache& mc)
{
	OneTimeCalcs();
	DhDNA::BONES thighBone;
	DhDNA::BONES calfBone;
	DhDNA::BONES footBone;
	Bones(thighBone, calfBone, footBone);

	// Calculate hip position relative to waist for origin of vertices.
	AuPoint loc = ConnectLoc();

	// Dimensions
	const float tl = m_Body->ThighLength();
	const float cl = m_Body->CalfLength();
	const float ct = m_Body->CalfThickness();
	const float fl = m_Body->FootLength();
	float ft = fl/2.0;
	if(ft > ct)
		ft = ct * 1.1;	// Only let the feet be a tiny bit bigger than the calves.

	// Draw model
	Au3dsGLSLModelInstance* modelLegPiece, *modelFootPiece;
	float uLength, uThick, lLength, lThick, footLength;
	uLength = m_DNA.Length(m_DNA.ThighLength);
	uThick = m_DNA.Thickness(m_DNA.ThighThickness);
	lLength = m_DNA.Length(m_DNA.CalfLength);
	lThick = m_DNA.Thickness(m_DNA.CalfThickness);
	footLength = m_DNA.Length(m_DNA.FootLength)/4;
	QString legModel, footModel;
	QString sPrefix = m_Body->GetBodyModelPrefix();
	bool bHasArmor = m_Body->HasBodyPowerArmorOn();
	Au3dsGLSLModelDefinition modelDef(1, m_Body->OverallScale(), m_Body->m_uTOffset, m_Body->m_vTOffset,
		m_Body->m_uBOffset, m_Body->m_vBOffset, thighBone, calfBone, footBone, loc, uLength/1.5, uThick/.5*(uLength/1.5),
		lLength/1.5, lThick/.5*(lLength/1.5), 1, m_Body->m_TopColor[bHasArmor], m_Body->m_TopShininess[bHasArmor],
		m_Body->m_BottomColor[bHasArmor], m_Body->m_BottomShininess[bHasArmor], m_Body->m_Glow[bHasArmor], m_Body->m_Glowdim[bHasArmor], true);
	if(thighBone == DhDNA::BONE_Thigh0R || thighBone == DhDNA::BONE_Thigh1R || thighBone == DhDNA::BONE_Thigh2R)
	{
		// RIGHT LEG
		if(bHasArmor)
			legModel = QString("%1LegRight.3dsN").arg(sPrefix);
		else
			legModel = QString("%1LegRight%2.3dsN").arg(sPrefix).arg(m_DNA.LegType);
		modelLegPiece = mc.CreateBodyModel(re, legModel, m_Body->m_Min, m_Body->m_Max, false, modelDef);
		loc.y += tl;
		loc.y += cl;
		// RIGHT FOOT
		if(bHasArmor)
			footModel = QString("%1FootRight%2.3dsN").arg(sPrefix).arg(m_DNA.HasAnkleBend());
		else
			footModel = QString("%1FootRight%2.3dsN").arg(sPrefix).arg(m_DNA.FootType);
		modelDef.SetOffset(loc);
		modelDef.SetLowerLength(footLength/.5f);
		modelFootPiece = mc.CreateBodyModel(re, footModel, m_Body->m_Min, m_Body->m_Max, false, modelDef);
	}
	else
	{
		// LEFT LEG
		if(bHasArmor)
			legModel = QString("%1LegLeft.3dsN").arg(sPrefix);
		else
			legModel = QString("%1LegLeft%2.3dsN").arg(sPrefix).arg(m_DNA.LegType);
		modelLegPiece = mc.CreateBodyModel(re, legModel, m_Body->m_Min, m_Body->m_Max, false, modelDef);
		loc.y += tl;
		loc.y += cl;
		// LEFT FOOT
		if(bHasArmor)
			footModel = QString("%1FootLeft%2.3dsN").arg(sPrefix).arg(m_DNA.HasAnkleBend());
		else
			footModel = QString("%1FootLeft%2.3dsN").arg(sPrefix).arg(m_DNA.FootType);
		modelDef.SetOffset(loc);
		modelDef.SetLowerLength(footLength/.5f);
		modelFootPiece = mc.CreateBodyModel(re, footModel, m_Body->m_Min, m_Body->m_Max, false, modelDef);
	}
	if(modelLegPiece)
		m_Body->m_Models.append(modelLegPiece);
	if(modelFootPiece)
		m_Body->m_Models.append(modelFootPiece);
}

// Calculate angles for standing.
void DpLegAndFoot::CalcStandAngles(float hipHt, float& hipang, float& kneebend,
		float& anklebend) const
{
	DhDNA::BONES thighBone;
	DhDNA::BONES calfBone;
	DhDNA::BONES footBone;
	Bones(thighBone, calfBone, footBone);

	const float thighlen = m_Body->ThighLength();
	const float calflen = m_Body->CalfLength();
	const float footlen = m_Body->FootLength();

	// Calculate leg position for proper stance and body carriage.
	hipang = 0.0;	// Rotation in YZ plane, i.e. critter forward/backward.
	kneebend = 0.0;	// Rotation in YZ plane, i.e. critter forward/backward.
	anklebend = 0.0;
	float shinlen = 0.0;
	float kneeht = 0.0;
	float elevang = 0.0;
	if(m_DNA.HasAnkleBend())
		anklebend = PIf/2.0;
	if(DoesTouchGroundBelowJoint(hipHt, thighlen, calflen, footlen))
	{
		// Foot can be placed on the ground under the hip.
		if(m_DNA.HasAnkleBend())
		{
			// These foot types have an ankle bend.
			if(/*m_DNA.FootLength == 0 &&*/ footlen < thighlen && footlen < calflen)
			{
				// This foot is short enough that the creature walks on its
				// heels.
				// Using the Law of Cosines...
				float shinlen = calflen;
				float shin2 = shinlen*shinlen;
				float hipHt2 = hipHt*hipHt;
				float thigh2 = thighlen*thighlen;
				hipang = (shin2 - thigh2 - hipHt2) / (-2.0 * thighlen * hipHt);
				hipang = acosf(hipang);
				kneebend = (hipHt2 - thigh2 - shin2) / (-2.0 * thighlen *
						shinlen);
				kneebend = acosf(kneebend);
			}
			else
			{
				// This foot is long enough to stand on its toes.
				// Using trigonometry and the Law of Cosines...
				float a = atanf(footlen/calflen);	// Angle at knee between
										// calf line and line from knee to toe.
				float shinlen = calflen/cosf(a);	// Length of line from knee
										// to toe.

				float shin2 = shinlen*shinlen;
				float hipHt2 = hipHt*hipHt;
				float thigh2 = thighlen*thighlen;
				hipang = (shin2 - thigh2 - hipHt2) / (-2.0 * thighlen * hipHt);
				hipang = acosf(hipang);
				kneebend = (hipHt2 - thigh2 - shin2) / (-2.0 * thighlen *
						shinlen);
				kneebend = acosf(kneebend);
				kneebend -= a;	// Must deduct shin angle for bend to ankle
								// rather than toe.
			}
		}
		else
		{
			// These foot types do not have an ankle bend.
			// Using the Law of Cosines...
			float shinlen = calflen + footlen;
			float shin2 = shinlen*shinlen;
			float hipHt2 = hipHt*hipHt;
			float thigh2 = thighlen*thighlen;
			hipang = (shin2 - thigh2 - hipHt2) / (-2.0 * thighlen * hipHt);
			hipang = acosf(hipang);
			kneebend = (hipHt2 - thigh2 - shin2) / (-2.0 * thighlen * shinlen);
			kneebend = acosf(kneebend);
		}
		hipang = hipang - PIf/2.0;	// Angle to rotate leg already straight
									// along +Y.
	}
	else
	{
		// Foot cannot be placed on the ground under the hip.
		float a = 0.;
		if(m_DNA.HasAnkleBend())
		{
			//shinlen = calflen; // These foot types have an ankle bend.
			a = atanf(footlen/calflen);	// Angle at knee between calf line and
												// line from knee to toe.
			shinlen = calflen/cosf(a);	// Length of line from knee to toe.
		}
		else
			shinlen = calflen + footlen; // These foot types do not have an ankle bend.
		if(shinlen > 0.0)
		{
			elevang = 45.f * DEG2RADf;	// Max angle to elevate knee.
			float addht = sinf(elevang) * thighlen;	// Height added at knee due to elev.
			kneeht = hipHt + addht;
			if(kneeht > shinlen)
			{
				// Foot can be placed directly under the knee. Shin length drives calc.
				if(thighlen + shinlen > hipHt)
				{
					elevang = asinf((shinlen - hipHt)/thighlen);
					kneebend = PIf/2.0 - elevang - a;
				}
			}
			else
			{
				// Foot cannot be placed under knee.
				kneebend = acosf(kneeht/shinlen);
				kneebend += (PIf/2.0 - elevang);
				kneebend -= a;

				// Calculate ankle bend so foot sits flat.
				// No...don't do that.
				//anklebend = PIf - (elevang + kneebend);
				//qDebug("%s elevang %g kneebend %g anklebend %g",
					//(const char*)m_DNA.Description(),
						//elevang*RAD2DEG, kneebend*RAD2DEG, anklebend*RAD2DEG);
			}
			hipang = elevang;	// Angle to rotate leg already straight along +Y.
			//qDebug("%s carriage %d addht %g kneeht %g hipHt %g shinlen %g thighlen %g elevang %g kneebend %g hipang %g",
					//(const char*)m_DNA.Description(), m_DNA.BodyCarriage,
					//addht, kneeht, hipHt, shinlen, thighlen, elevang*RAD2DEG,
					//kneebend*RAD2DEG, hipang*RAD2DEG);
		}
	}

	kneebend -= PIf;		// Amount to bend leg that is already straight.

	/*
#if !defined(QT_NO_DEBUG)
	for(int ib=0; ib<DhDNA::BONE_Max; ib++)
	{
		const AuMatrix4x4& bmat = m_Body->Bone(ib);
		if(bmat[3] != 0 || bmat[7] != 0 || bmat[11] != 0 || bmat[15] != 1)
		{
			qDebug("DpLegAndFoot::Stand - hipHt %g hipang %g kneebend %g Bad Bone Matrix %d in %s doestouch %s thighlen %g calflen %g footlen %g hasankle %s shinlen %g kneeht %g, elevang %g", 
					hipHt, hipang*RAD2DEG, kneebend*RAD2DEG, ib,
					(const char*)m_DNA.Description().toAscii(),
					DoesTouchGroundBelowJoint(hipHt)?"y":"n",
					thighlen, calflen, footlen, m_DNA.HasAnkleBend()?"y":"n",
					shinlen, kneeht, elevang*RAD2DEG);
			bmat.Dump(0);
		}
	}
#endif
*/
}

void DpLegAndFoot::Stand(float hipHt)
{
	float hipang, kneebend, anklebend;
	CalcStandAngles(hipHt, hipang, kneebend, anklebend);

	// Position bones.
	RotateHip(hipang);		// Move knee forward, rotating at hip.
	RotateKnee(kneebend);	// Move foot backward, bending at knee.
	if(m_DNA.HasAnkleBend())
		RotateAnkle(anklebend);
	SplayHip(m_Splay);
}

void DpLegAndFoot::Fold()
{
	float hipang = -20*DEG2RADf;
	float kneebend = -145*DEG2RADf;
	RotateHip(hipang);		// Move knee forward, rotating at hip.
	RotateKnee(kneebend);	// Move foot backward, bending at knee.
	if(m_DNA.HasAnkleBend())
		RotateAnkle(PIf/2.);
	SplayHip(m_Splay);
}

void DpLegAndFoot::Kick(float hipHt)
{
	float splay = m_Splay;
	float hipang, kneebend, anklebend;
	CalcStandAngles(hipHt, hipang, kneebend, anklebend);
	if(IsFront())
	{
		// Front legs kick forward out of phase with each other.
		float persin;
		if(IsLeft())
			persin = m_DNA.Rhythm(AuPose::Kick, 0, 1);
		else
			persin = m_DNA.Rhythm(AuPose::Kick, PIf, 1);
		hipang += (20.f + 20.f*persin)*DEG2RADf;
		kneebend -= (kneebend + kneebend*persin)/2.f;
		splay /= .3f;
	}
	else
	{
		// Rear legs kick backward in sync with each other.
		float persin = m_DNA.Rhythm(AuPose::Kick, 0, 1);
		hipang -= (20.f + 20.f*persin)*DEG2RADf;
		kneebend -= (kneebend + kneebend*(-persin))/2.f;
		splay += (PIf - splay)*2.f/3.f;
	}

	// Position bones.
	RotateHip(hipang);		// Move knee forward, rotating at hip.
	RotateKnee(kneebend);	// Move foot backward, bending at knee.
	if(m_DNA.HasAnkleBend())
		RotateAnkle(anklebend);
	SplayHip(splay);
}

void DpLegAndFoot::Climb(float hipHt, float speedEnc)
{
	float hipang, kneebend, anklebend;
	CalcStandAngles(hipHt, hipang, kneebend, anklebend);

	float persin;
	if(IsLeft())
		persin = m_DNA.Rhythm(AuPose::LadderClimb, 0, speedEnc);
	else
		persin = m_DNA.Rhythm(AuPose::LadderClimb, PIf, speedEnc);
	hipang += (20. + 20.*persin)*DEG2RADf;
	kneebend -= (20. + 20.*persin)*DEG2RADf;
	float kneesweep = -kneebend/3.;
	kneebend += kneesweep - kneesweep*persin;

	// Position bones.
	RotateHip(hipang);		// Move knee forward, rotating at hip.
	RotateKnee(kneebend);	// Move foot backward, bending at knee.
	if(m_DNA.HasAnkleBend())
		RotateAnkle(anklebend);
	SplayHip(m_Splay/3.);
}

void DpLegAndFoot::Dig(float hipHt, float speedEnc)
{
	float hipang, kneebend, anklebend;
	CalcStandAngles(hipHt, hipang, kneebend, anklebend);

	float persin;
	if(IsLeft())
		persin = m_DNA.Rhythm(AuPose::Dig, 0, speedEnc);
	else
		persin = m_DNA.Rhythm(AuPose::Dig, PIf, speedEnc);
	hipang += (20. + 20.*persin)*DEG2RADf;
	float kneesweep = -kneebend/3.;
	kneebend += kneesweep - kneesweep*persin;

	// Position bones.
	RotateHip(hipang);		// Move knee forward, rotating at hip.
	RotateKnee(kneebend);	// Move foot backward, bending at knee.
	if(m_DNA.HasAnkleBend())
		RotateAnkle(anklebend);
	SplayHip(m_Splay/3.);
}

void DpLegAndFoot::Jump(float hipHt, float speedEnc)
{
	// When jumping, all limbs move in unison.
	float persin = m_DNA.Rhythm(AuPose::Jump, 0, speedEnc);

	// Outstretched limb angles.
	float kneeout = -15.f*DEG2RADf;
	float hipout = -135.f*DEG2RADf;
	float ankleout;
	if(m_DNA.HasAnkleBend())
		ankleout = 20.f*DEG2RADf;
	else
		ankleout = -2.f*DEG2RADf;

	// Standing joint angles.
	float hipstand, kneestand, anklestand;
	CalcStandAngles(hipHt, hipstand, kneestand, anklestand);

	// Cycle between outstretched and standing limb position.
	float hipsweep = hipstand - hipout;
	float hipang = hipout + (hipsweep + hipsweep*persin)/2.f;
	float kneesweep = kneestand - kneeout;
	float kneebend = kneeout + (kneesweep + kneesweep*persin)/2.f;
	float anklesweep = anklestand - ankleout;
	float anklebend = ankleout + (anklesweep + anklesweep*persin)/2.f;

	// Position bones.
	RotateHip(hipang);		// Move knee forward, rotating at hip.
	RotateKnee(kneebend);	// Move foot backward, bending at knee.
	RotateAnkle(anklebend);
	SplayHip(m_Splay);
}

void DpLegAndFoot::DeadFacingDown()
{
	float hipang;
	if(m_DNA.LegPairs == 1 && m_Splay < PIf/4.f)
		hipang = -(PIf - 5.f*DEG2RADf);	// bend to back
	else
		hipang = -5.f*DEG2RADf;	// bend to front
	float kneebend = -10.f*DEG2RADf;
	float anklebend = PIf/2.f - 50.f*DEG2RADf;

	// Position bones.
	RotateHip(hipang);		// Move knee forward, rotating at hip.
	RotateKnee(kneebend);	// Move foot backward, bending at knee.
	if(m_DNA.HasAnkleBend())
		RotateAnkle(anklebend);
	SplayHip(m_Splay);
}

void DpLegAndFoot::DeadFacingLeft()
{
	float hipang, kneebend, anklebend;	//, splay;
	if(m_DNA.LegPairs == 1 && m_Splay < PIf/4.f)
	{
		if(IsLeft())
		{
			hipang = -(PIf - 35.f*DEG2RADf);	// bend to back
			kneebend = -50.f*DEG2RADf;
			anklebend = PIf/2.f - 50.f*DEG2RADf;
		}
		else
		{
			hipang = -(PIf - 5.f*DEG2RADf);	// bend to back
			kneebend = -10.f*DEG2RADf;
			anklebend = PIf/2.f;
		}
		//splay = -m_Splay;
	}
	else
	{
		CalcStandAngles(m_Body->CGHeightStanding(false), hipang, kneebend, anklebend);
		if(IsLeft())
		{
			if(hipang > 0)
			{
				// Leg rises from hip joint when standing.
				hipang = 70.f*DEG2RADf;
				kneebend = -170.f*DEG2RADf;
			}
			else
			{
				// Leg drops from hip joint when standing.
				hipang = -75.f*DEG2RADf;
				kneebend = -30.f*DEG2RADf;
			}
		}
		else
		{
			if(hipang > 0)
			{
				// Leg rises from hip joint when standing.
				hipang = 88.f*DEG2RADf;	// flat up against body
				kneebend = -2.f*DEG2RADf;
			}
			else
			{
				// Leg drops from hip joint when standing.
				hipang = -PIf/2.f;	// flat down against body
				kneebend = -30.f*DEG2RADf;
			}
		}
		//splay = m_Splay;
	}

	// Position bones.
	RotateHip(hipang);		// Move knee forward, rotating at hip.
	RotateKnee(kneebend);	// Move foot backward, bending at knee.
	if(m_DNA.HasAnkleBend())
		RotateAnkle(anklebend);
	SplayHip(m_Splay);
}

void DpLegAndFoot::DeadFacingRight()
{
	float hipang, kneebend, anklebend;	//, splay;
	if(m_DNA.LegPairs == 1 && m_Splay < PIf/4.f)
	{
		if(!IsLeft())
		{
			hipang = -(PIf - 35.f*DEG2RADf);	// bend to back
			kneebend = -50.f*DEG2RADf;
			anklebend = PIf/2.f - 50.f*DEG2RADf;
		}
		else
		{
			hipang = -(PIf - 5.f*DEG2RADf);	// bend to back
			kneebend = -10.f*DEG2RADf;
			anklebend = PIf/2.f;
		}
		//splay = -m_Splay;
	}
	else
	{
		CalcStandAngles(m_Body->CGHeightStanding(false), hipang, kneebend, anklebend);
		if(!IsLeft())
		{
			if(hipang > 0)
			{
				// Leg rises from hip joint when standing.
				hipang = 70.f*DEG2RADf;
				kneebend = -170.f*DEG2RADf;
			}
			else
			{
				// Leg drops from hip joint when standing.
				hipang = -75.f*DEG2RADf;
				kneebend = -30.f*DEG2RADf;
			}
		}
		else
		{
			if(hipang > 0)
			{
				// Leg rises from hip joint when standing.
				hipang = 88.f*DEG2RADf;	// flat up against body
				kneebend = -2.f*DEG2RADf;
			}
			else
			{
				// Leg drops from hip joint when standing.
				hipang = -PIf/2.f;	// flat down against body
				kneebend = -30.f*DEG2RADf;
			}
		}
		//splay = m_Splay;
	}

	// Position bones.
	RotateHip(hipang);		// Move knee forward, rotating at hip.
	RotateKnee(kneebend);	// Move foot backward, bending at knee.
	if(m_DNA.HasAnkleBend())
		RotateAnkle(anklebend);
	SplayHip(m_Splay);
}

void DpLegAndFoot::DeadFacingUp()
{
	float hipang;
	if(m_DNA.LegPairs == 1 && m_Splay < PIf/4.f)
		hipang = -(PIf - 5.f*DEG2RADf);	// bend to back
	else
		hipang = -5.f*DEG2RADf;	// bend to front
	float kneebend = -10.f*DEG2RADf;
	float anklebend = PIf/2.f - 50.f*DEG2RADf;

	// Position bones.
	RotateHip(hipang);		// Move knee forward, rotating at hip.
	RotateKnee(kneebend);	// Move foot backward, bending at knee.
	if(m_DNA.HasAnkleBend())
		RotateAnkle(anklebend);
	SplayHip(m_Splay);
}

void DpLegAndFoot::Prone()
{
	float hipang;
	if(m_DNA.LegPairs == 1 && m_Splay < PIf/4.f)
		hipang = -(PIf - 5.f*DEG2RADf);	// bend to back
	else
		hipang = -5.f*DEG2RADf;	// bend to front
	float kneebend = -10.f*DEG2RADf;
	float anklebend = PIf/2.f - 50.f*DEG2RADf;

	// Position bones.
	RotateHip(hipang);		// Move knee forward, rotating at hip.
	RotateKnee(kneebend);	// Move foot backward, bending at knee.
	if(m_DNA.HasAnkleBend())
		RotateAnkle(anklebend);
	SplayHip(m_Splay);
}

void DpLegAndFoot::ProneCreep(float speedEnc)
{
	float perang = 0;
	if(IsLeft() &&
			!(m_DNA.LegPairs == 2 ||
				(m_DNA.LegPairs == 1 && m_DNA.HasArms() && m_DNA.WalksOnHands())))
		perang += PIf;	// Stagger left/right unless 2 leg pairs
	if(m_DNA.LegPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.LegPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2./3;
		else if(m_Which == 2)
			perang += PIf*4./3;
	}
	float persin = m_DNA.Rhythm(AuPose::ProneCreep, perang, speedEnc);

	float hipang;
	if(m_DNA.LegPairs == 1 && m_Splay < PIf/4.f)
		hipang = -(PIf - 5.f*DEG2RADf);	// bend to back
	else
		hipang = -5.f*DEG2RADf;	// bend to front
	float kneebend = -10.f*DEG2RADf;
	float anklebend = PIf/2.f - 50.f*DEG2RADf;

	float kneesweep = 10.f*DEG2RADf;
	kneebend -= (kneesweep - kneesweep*persin);

	float splay = m_Splay * (1 - persin);

	// Position bones.
	RotateHip(hipang);		// Move knee forward, rotating at hip.
	RotateKnee(kneebend);	// Move foot backward, bending at knee.
	if(m_DNA.HasAnkleBend())
		RotateAnkle(anklebend);
	SplayHip(splay);
}

void DpLegAndFoot::Run(float hipHt, float speedEnc)
{
	float perang = 0;
	if(IsLeft() &&
			!(m_DNA.LegPairs == 2 ||
				(m_DNA.LegPairs == 1 && m_DNA.HasArms() && m_DNA.WalksOnHands())))
		perang += PIf;	// Stagger left/right unless 2 leg pairs
	if(m_DNA.LegPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.LegPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2./3;
		else if(m_Which == 2)
			perang += PIf*4./3;
	}
	float persin = m_DNA.Rhythm(AuPose::StandRun, perang, speedEnc);

	float hipang, kneebend, anklebend;
	CalcStandAngles(hipHt, hipang, kneebend, anklebend);

	hipang += (-3 - 40.*persin)*DEG2RADf;
	float splaysin = sinf(fabs(m_Splay)) * .8;
	float kneesweep = (PIf + kneebend)/6. * (1 - splaysin);
	kneebend -= (kneesweep - kneesweep*persin);

	float splay = m_Splay;
	float splaysweep = splay;
	if(splaysweep > PIf/2)
		splaysweep = PIf - splaysweep;
	splaysweep *= splaysin/2.;
	splay += persin*splaysweep;

	// Position bones.
	RotateHip(hipang);		// Move knee forward, rotating at hip.
	RotateKnee(kneebend);	// Move foot backward, bending at knee.
	if(m_DNA.HasAnkleBend())
		RotateAnkle(anklebend);
	SplayHip(splay);
}

void DpLegAndFoot::Stomp(float hipHt)
{
	float hipang, kneebend, anklebend;
	CalcStandAngles(hipHt, hipang, kneebend, anklebend);

	float persin;
	if(IsLeft())
		persin = m_DNA.Rhythm(AuPose::Stomp, 0, 1);
	else
		persin = m_DNA.Rhythm(AuPose::Stomp, PIf, 1);
	hipang += (20. + 20.*persin)*DEG2RADf;
	float kneesweep = -kneebend/3.;
	kneebend += kneesweep - kneesweep*persin;

	// Position bones.
	RotateHip(hipang);		// Move knee forward, rotating at hip.
	RotateKnee(kneebend);	// Move foot backward, bending at knee.
	if(m_DNA.HasAnkleBend())
		RotateAnkle(anklebend);
	SplayHip(m_Splay/3.);
}

void DpLegAndFoot::Straddle()
{
	float hipang = -25*DEG2RADf;
	float kneebend = -100*DEG2RADf;
	float anklebend = 100*DEG2RADf;

	// Position bones.
	RotateHip(hipang);		// Move knee forward, rotating at hip.
	RotateKnee(kneebend);	// Move foot backward, bending at knee.
	if(m_DNA.HasAnkleBend())
		RotateAnkle(anklebend);

	/*
	// Push knees outward, away from body.
	float splay = m_Splay;
	if(splay < PIf/2.f)
		splay += (PIf/2.f - splay)/2.f;
	else
		splay -= (splay - PIf/2.f)/2.f;
	SplayHip(splay);
	*/
	SplayHip(m_Splay);
}

void DpLegAndFoot::Swim(const AuPose& poseRhythm)
{
	float perang = 0;
	if(IsLeft())
		perang += PIf;
	if(m_DNA.LegPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.LegPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2.f/3.f;
		else if(m_Which == 2)
			perang += PIf*4.f/3.f;
	}
	float persin = m_DNA.Rhythm(poseRhythm, perang, 1);

	float hipang = -(PIf - 5.f*DEG2RADf) + persin*5.f*DEG2RADf;
	float kneebend = -10.f*DEG2RADf + persin*10.f*DEG2RADf;
	float anklebend = PIf/2.f - 50.f*DEG2RADf;

	// Position bones.
	RotateHip(hipang);		// Move knee forward, rotating at hip.
	RotateKnee(kneebend);	// Move foot backward, bending at knee.
	if(m_DNA.HasAnkleBend())
		RotateAnkle(anklebend);
	//SplayHip(m_Splay);
}

void DpLegAndFoot::TreadWater(float hipHt)
{
	float perang = 0;
	if(IsLeft())
		perang += PIf;
	if(m_DNA.LegPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.LegPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2.f/3.f;
		else if(m_Which == 2)
			perang += PIf*4.f/3.f;
	}
	float persin = m_DNA.Rhythm(AuPose::TreadWater, perang, 1);

	float hipang, kneebend, anklebend;
	CalcStandAngles(hipHt, hipang, kneebend, anklebend);

	float hipsweep = hipang - -PIf/2.f;
	hipsweep /= 2.f;
	hipang -= hipsweep;
	hipang += 10.f*persin*DEG2RADf;

	float kneesweep = kneebend/2.;
	kneebend -= kneesweep + kneesweep*persin;

	anklebend -= 50.f*DEG2RADf;

	// Position bones.
	RotateHip(hipang);		// Move knee forward, rotating at hip.
	RotateKnee(kneebend);	// Move foot backward, bending at knee.
	if(m_DNA.HasAnkleBend())
		RotateAnkle(anklebend);
	SplayHip(m_Splay);
}

void DpLegAndFoot::Walk(float hipHt, float speedEnc)
{
	float perang = 0;
	if(IsLeft())
		perang += PIf;
	if(m_DNA.LegPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.LegPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2.f/3.f;
		else if(m_Which == 2)
			perang += PIf*4.f/3.f;
	}
	float persin = m_DNA.Rhythm(AuPose::StandWalk, perang, speedEnc);

	float hipang, kneebend, anklebend;
	CalcStandAngles(hipHt, hipang, kneebend, anklebend);

	hipang += (-3 - 10.*persin)*DEG2RADf;
	float splaysin = sinf(fabs(m_Splay)) * .8;
	float kneesweep = kneebend/2. * (1 - splaysin);
	kneebend -= (kneesweep - kneesweep*persin);

	float splay = m_Splay;
	float splaysweep = splay;
	if(splaysweep > PIf/2)
		splaysweep = PIf - splaysweep;
	splaysweep *= splaysin/3.;
	splay += persin*splaysweep;

	// Position bones.
	RotateHip(hipang);		// Move knee forward, rotating at hip.
	RotateKnee(kneebend);	// Move foot backward, bending at knee.
	if(m_DNA.HasAnkleBend())
		RotateAnkle(anklebend);
	SplayHip(splay);
}

void DpLegAndFoot::WritheFacingDown()
{
	float perang = 0;
	if(IsLeft())
		perang += PIf;
	if(m_DNA.LegPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.LegPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2.f/3.f;
		else if(m_Which == 2)
			perang += PIf*4.f/3.f;
	}
	float persin = m_DNA.Rhythm(AuPose::WritheFaceDown, perang, 1);

	float kneebend = -10.f*DEG2RADf;
	float hipang;
	if(m_DNA.LegPairs == 1 && m_Splay < PIf/4.f)
	{
		hipang = -(PIf - 5.f*DEG2RADf);	// bend to back
	}
	else
	{
		hipang = -5.f*DEG2RADf;	// bend to front
		hipang += 1.f*DEG2RADf * persin;
	}
	float anklebend = PIf/2.f - 50.f*DEG2RADf;

	kneebend -= 2.f*DEG2RADf * persin;

	// Position bones.
	RotateHip(hipang);		// Move knee forward, rotating at hip.
	RotateKnee(kneebend);	// Move foot backward, bending at knee.
	if(m_DNA.HasAnkleBend())
		RotateAnkle(anklebend);
	SplayHip(m_Splay);
}

void DpLegAndFoot::WritheFacingLeft()
{
	float hipang, kneebend, anklebend;	//, splay;
	if(m_DNA.LegPairs == 1 && m_Splay < PIf/4.f)
	{
		if(IsLeft())
		{
			hipang = -(PIf - 35.f*DEG2RADf);	// bend to back
			kneebend = -50.f*DEG2RADf;
			anklebend = PIf/2.f - 50.f*DEG2RADf;
		}
		else
		{
			hipang = -(PIf - 5.f*DEG2RADf);	// bend to back
			kneebend = -10.f*DEG2RADf;
			anklebend = PIf/2.f;
		}
		//splay = -m_Splay;
	}
	else
	{
		CalcStandAngles(m_Body->CGHeightStanding(false), hipang, kneebend, anklebend);
		if(IsLeft())
		{
			if(hipang > 0)
			{
				// Leg rises from hip joint when standing.
				hipang = 70.f*DEG2RADf;
				kneebend = -170.f*DEG2RADf;
			}
			else
			{
				// Leg drops from hip joint when standing.
				hipang = -75.f*DEG2RADf;
				kneebend = -30.f*DEG2RADf;
			}
		}
		else
		{
			if(hipang > 0)
			{
				// Leg rises from hip joint when standing.
				hipang = 88.f*DEG2RADf;	// flat up against body
				kneebend = -2.f*DEG2RADf;
			}
			else
			{
				// Leg drops from hip joint when standing.
				hipang = -PIf/2.f;	// flat down against body
				kneebend = -30.f*DEG2RADf;
			}
		}
		//splay = m_Splay;
	}

	// Writhing motion.
	if(IsLeft())
	{
		float perang = 0;
		if(IsLeft())
			perang += PIf;
		if(m_DNA.LegPairs == 2 && m_Which == 1)
			perang += PIf;
		else if(m_DNA.LegPairs == 3)
		{
			if(m_Which == 1)
				perang += PIf*2.f/3.f;
			else if(m_Which == 2)
				perang += PIf*4.f/3.f;
		}
		float persin = m_DNA.Rhythm(AuPose::WritheFaceDown, perang, 1);

		hipang += 1.f*DEG2RADf*persin;
		kneebend -= (5.f + 5.f*persin)*DEG2RADf;
	}

	// Position bones.
	RotateHip(hipang);		// Move knee forward, rotating at hip.
	RotateKnee(kneebend);	// Move foot backward, bending at knee.
	if(m_DNA.HasAnkleBend())
		RotateAnkle(anklebend);
	SplayHip(m_Splay);
}

void DpLegAndFoot::WritheFacingRight()
{
	float hipang, kneebend, anklebend;	//, splay;
	if(m_DNA.LegPairs == 1 && m_Splay < PIf/4.f)
	{
		if(!IsLeft())
		{
			hipang = -(PIf - 35.f*DEG2RADf);	// bend to back
			kneebend = -50.f*DEG2RADf;
			anklebend = PIf/2.f - 50.f*DEG2RADf;
		}
		else
		{
			hipang = -(PIf - 5.f*DEG2RADf);	// bend to back
			kneebend = -10.f*DEG2RADf;
			anklebend = PIf/2.f;
		}
		//splay = -m_Splay;
	}
	else
	{
		CalcStandAngles(m_Body->CGHeightStanding(false), hipang, kneebend, anklebend);
		if(!IsLeft())
		{
			if(hipang > 0)
			{
				// Leg rises from hip joint when standing.
				hipang = 70.f*DEG2RADf;
				kneebend = -170.f*DEG2RADf;
			}
			else
			{
				// Leg drops from hip joint when standing.
				hipang = -75.f*DEG2RADf;
				kneebend = -30.f*DEG2RADf;
			}
		}
		else
		{
			if(hipang > 0)
			{
				// Leg rises from hip joint when standing.
				hipang = 88.f*DEG2RADf;	// flat up against body
				kneebend = -2.f*DEG2RADf;
			}
			else
			{
				// Leg drops from hip joint when standing.
				hipang = -PIf/2.f;	// flat down against body
				kneebend = -30.f*DEG2RADf;
			}
		}
		//splay = m_Splay;
	}

	// Writhing motion.
	if(!IsLeft())
	{
		float perang = 0;
		if(IsLeft())
			perang += PIf;
		if(m_DNA.LegPairs == 2 && m_Which == 1)
			perang += PIf;
		else if(m_DNA.LegPairs == 3)
		{
			if(m_Which == 1)
				perang += PIf*2.f/3.f;
			else if(m_Which == 2)
				perang += PIf*4.f/3.f;
		}
		float persin = m_DNA.Rhythm(AuPose::WritheFaceDown, perang, 1);

		hipang += 1.f*DEG2RADf*persin;
		kneebend -= (5.f + 5.f*persin)*DEG2RADf;
	}

	// Position bones.
	RotateHip(hipang);		// Move knee forward, rotating at hip.
	RotateKnee(kneebend);	// Move foot backward, bending at knee.
	if(m_DNA.HasAnkleBend())
		RotateAnkle(anklebend);
	SplayHip(m_Splay);
}

void DpLegAndFoot::WritheFacingUp()
{
	float perang = 0;
	if(IsLeft())
		perang += PIf;
	if(m_DNA.LegPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.LegPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2.f/3.f;
		else if(m_Which == 2)
			perang += PIf*4.f/3.f;
	}
	float persin = m_DNA.Rhythm(AuPose::WritheFaceUp, perang, 1);

	float kneebend = -10.f*DEG2RADf;
	float hipang;
	if(m_DNA.LegPairs == 1 && m_Splay < PIf/4.f)
	{
		hipang = -(PIf - 5.f*DEG2RADf);	// bend to back
		kneebend -= 2.f*DEG2RADf * persin;
	}
	else
	{
		hipang = -5.f*DEG2RADf;	// bend to front
	}
	float anklebend = PIf/2.f - 50.f*DEG2RADf;

	hipang += 1.f*DEG2RADf * persin;

	// Position bones.
	RotateHip(hipang);		// Move knee forward, rotating at hip.
	RotateKnee(kneebend);	// Move foot backward, bending at knee.
	if(m_DNA.HasAnkleBend())
		RotateAnkle(anklebend);
	SplayHip(m_Splay);
}

void DpLegAndFoot::ReceptiveFacingUp()
{
	float perang = 0;
	if(IsLeft())
		perang += PIf;
	if(m_DNA.LegPairs == 2 && m_Which == 1)
		perang += PIf;
	else if(m_DNA.LegPairs == 3)
	{
		if(m_Which == 1)
			perang += PIf*2.f/3.f;
		else if(m_Which == 2)
			perang += PIf*4.f/3.f;
	}
	float persin = m_DNA.Rhythm(AuPose::ReceptiveMate, perang, 1);

	float kneebend = -10.f*DEG2RADf;
	float hipang;
	if(m_DNA.LegPairs == 1 && m_Splay < PIf/4.f)
	{
		hipang = -(PIf - 5.f*DEG2RADf);	// bend to back
		kneebend -= 2.f*DEG2RADf * persin;
	}
	else
	{
		hipang = -5.f*DEG2RADf;	// bend to front
	}
	float anklebend = PIf/2.f - 12.f*DEG2RADf;

	hipang += 1.f*DEG2RADf * persin;

	// Position bones.
	RotateHip(hipang);		// Move knee forward, rotating at hip.
	RotateKnee(kneebend);	// Move foot backward, bending at knee.
	if(m_DNA.HasAnkleBend())
		RotateAnkle(anklebend);
	SplayHip(m_Splay);
}

// False and limb must reach out to stand.
// True means the limb can physically reach the position.
bool DpLegAndFoot::DoesTouchGroundBelowJoint(float hipHt,
		float thighlen, float calflen, float footlen) const
{
	if(m_DNA.IsExoskeletal())
		return false;	// Exoskeletal creatures always splay their legs.
	if(m_DNA.LegPairs > m_DNA.LegVertices())
		return false;	// Creatures with multiple legs per vertex splay their
						// legs.

	float shinlen = 0.0;
	if(m_DNA.HasAnkleBend())
	{
		// These foot types have an ankle bend.
		if(hipHt < footlen)
			return false;	// Foot won't fit.
		float a = atanf(footlen/calflen);	// Angle at knee between calf line
											// and line from knee to toe.
		shinlen = calflen/cosf(a);	// Length of line from knee to toe.
	}
	else
	{
		// These foot types do not have an ankle bend.
		shinlen = calflen + footlen;
	}

	if(thighlen + shinlen <= hipHt)
		return false;	// Foot cannot be placed on the ground under the hip.
	else if(hipHt + shinlen <= thighlen)
		return false;	// Foot cannot be placed on the ground under the hip.
	else if(hipHt + thighlen <= shinlen)
		return false;	// Foot cannot be placed on the ground under the hip.
	return true;		// Foot can be placed on the ground under the hip.
}

void DpLegAndFoot::OneTimeCalcs()
{
	// Connect Loc
	float vertlen = m_Body->TorsoSegmentLength();
	int waist = m_DNA.WaistVertex();
	AuPoint loc;
	if(m_Which == 0)
		loc = AuPoint(0,0);				// First pair of legs is always at the waist.
	else if(m_Which == 1)
		loc = AuPoint(0, -vertlen*((int)m_DNA.TorsoVertebrae-waist), 0);	// 2nd = butt.
	else
	{
		// Put this leg pair in between the waist and the butt pairs.
		int iv = ((int)m_DNA.TorsoVertebrae - waist)/2;
		loc = AuPoint(0, -vertlen*iv, 0);
	}

	float hip = m_Body->TorsoWidth()/2.0;	// Distance from spine to center of joint.
	float thighoff = m_Body->ThighThickness()/2.0;
	if(hip > thighoff)
		hip -= thighoff/2.0;	// Pull the thighs closer together.
	if(hip < thighoff)
		hip = thighoff;			// Avoid touching the thighs together.

	if(IsLeft())
		loc.x = -hip;
	else
		loc.x = hip;

	if(!m_DNA.TorsoVertebrae)
		loc.x /= 3.0;
	m_ConnectLoc = loc;

	// Knee Loc
	loc = ConnectLoc();
	loc.y += m_Body->ThighLength();
	m_KneeLoc = loc;

	// Ankle Loc
	loc = KneeLoc();
	loc.y += m_Body->CalfLength();
	m_AnkleLoc = loc;
}

// Calculate loc relative to body origin.
AuPoint DpLegAndFoot::AnkleLoc() const
{
	return m_AnkleLoc;
}

// Calculate loc relative to body origin.
AuPoint DpLegAndFoot::ConnectLoc() const
{
	return m_ConnectLoc;
}

// Calculate loc relative to body origin.
AuPoint DpLegAndFoot::KneeLoc() const
{
	return m_KneeLoc;
}

AuPoint DpLegAndFoot::CurrentAnkleLoc() const
{
	DhDNA::BONES thighBone;
	DhDNA::BONES calfBone;
	DhDNA::BONES footBone;
	Bones(thighBone, calfBone, footBone);
	AuPoint loc = AnkleLoc();
	AuPoint curloc = m_Body->Bone(footBone).Project(loc);
	return curloc;
}

AuPoint DpLegAndFoot::CurrentConnectLoc() const
{
	DhDNA::BONES thighBone;
	DhDNA::BONES calfBone;
	DhDNA::BONES footBone;
	Bones(thighBone, calfBone, footBone);
	AuPoint loc = ConnectLoc();
	AuPoint curloc = m_Body->Bone(thighBone).Project(loc);
	return curloc;
}

AuPoint DpLegAndFoot::CurrentKneeLoc() const
{
	DhDNA::BONES thighBone;
	DhDNA::BONES calfBone;
	DhDNA::BONES footBone;
	Bones(thighBone, calfBone, footBone);
	AuPoint loc = KneeLoc();
	AuPoint curloc = m_Body->Bone(calfBone).Project(loc);
	return curloc;
}

// YZ plane rotation
void DpLegAndFoot::RotateAnkle(float angle)
{
	if(angle == 0.0)
		return;

	DhDNA::BONES thighBone;
	DhDNA::BONES calfBone;
	DhDNA::BONES footBone;
	Bones(thighBone, calfBone, footBone);

	// Make a matrix to rotate around current knee location.
	AuPoint pivot = CurrentAnkleLoc();
	AuMatrix4x4 t;
	t.Translate(pivot);
	t.RotateX(angle);
	t.Translate(-pivot);

	// Rotate bones at knee.
	m_Body->Bone(footBone) = t * m_Body->Bone(footBone);
}

// YZ plane rotation
void DpLegAndFoot::RotateKnee(float angle)
{
	if(angle == 0.0)
		return;

	DhDNA::BONES thighBone;
	DhDNA::BONES calfBone;
	DhDNA::BONES footBone;
	Bones(thighBone, calfBone, footBone);

	// Make a matrix to rotate around current knee location.
	AuPoint pivot = CurrentKneeLoc();
	AuMatrix4x4 t;
	t.Translate(pivot);
	t.RotateX(angle);
	t.Translate(-pivot);

	// Rotate bones at knee.
	m_Body->Bone(calfBone) = t * m_Body->Bone(calfBone);
	m_Body->Bone(footBone) = t * m_Body->Bone(footBone);
}

// YZ plane rotation
void DpLegAndFoot::RotateHip(float angle)
{
	if(angle == 0.0)
		return;

	DhDNA::BONES thighBone;
	DhDNA::BONES calfBone;
	DhDNA::BONES footBone;
	Bones(thighBone, calfBone, footBone);

	// Make a matrix to rotate around current hip location.
	AuPoint pivot = CurrentConnectLoc();
	AuMatrix4x4 t;
	t.Translate(pivot);
	t.RotateX(angle);
	t.Translate(-pivot);

	// Rotate bones at hip.
	m_Body->Bone(thighBone) = t * m_Body->Bone(thighBone);
	m_Body->Bone(calfBone) = t * m_Body->Bone(calfBone);
	m_Body->Bone(footBone) = t * m_Body->Bone(footBone);
}

// Rotate at hip in XY plane.
// Positive moves knee away from body.
void DpLegAndFoot::SplayHip(float angle)
{
	if(angle == 0.0)
		return;

	DhDNA::BONES thighBone;
	DhDNA::BONES calfBone;
	DhDNA::BONES footBone;
	Bones(thighBone, calfBone, footBone);

	// Right leg angle is reversed.
	if(!IsLeft())
		angle = -angle;

	// Make a matrix to rotate around current hip location.
	AuPoint pivot = CurrentConnectLoc();
	AuMatrix4x4 t;
	t.Translate(pivot);
	t.RotateZ(angle);
	t.Translate(-pivot);

	// Rotate bones at hip.
	m_Body->Bone(thighBone) = t * m_Body->Bone(thighBone);
	m_Body->Bone(calfBone) = t * m_Body->Bone(calfBone);
	m_Body->Bone(footBone) = t * m_Body->Bone(footBone);
}

void DpLegAndFoot::MoveBy(const AuPoint& vMove)
{
	DhDNA::BONES thighBone;
	DhDNA::BONES calfBone;
	DhDNA::BONES footBone;
	Bones(thighBone, calfBone, footBone);

	// Make a matrix to move my bones.
	AuMatrix4x4 t;
	t.Translate(vMove);
	m_Body->Bone(thighBone) = t * m_Body->Bone(thighBone);
	m_Body->Bone(calfBone) = t * m_Body->Bone(calfBone);
	m_Body->Bone(footBone) = t * m_Body->Bone(footBone);
}

void DpLegAndFoot::RoughExtentLocs(AuPointArray& locs) const
{
	DhDNA::BONES thighBone;
	DhDNA::BONES calfBone;
	DhDNA::BONES footBone;
	Bones(thighBone, calfBone, footBone);

	// Calculate hip position relative to waist for origin of vertices.
	AuPoint loc = ConnectLoc();

	// Dimensions
	const float tl = m_Body->ThighLength();
	const float tt = m_Body->ThighThickness();
	const float cl = m_Body->CalfLength();
	const float fl = m_Body->FootLength();

	// Leg
	// A different loc is calculated to give the legs more of a hip around the joint,
	// rather than ending perfectly AT the joint.
	float hipthick = tt/2.0;	// radius of hip around joint
	AuPoint tloc(loc);
	tloc.y -= hipthick;			// Leg is being constructed UP the +Y axis.

	// Test chunk intersection.
	m_Body->RoughExtentChunk(locs, thighBone, tloc, tl + hipthick);

	loc.y += tl;

	// Test chunk intersection.
	m_Body->RoughExtentChunk(locs, calfBone, loc, cl);

	loc.y += cl;

	// Foot
	m_Body->RoughExtentChunk(locs, footBone, loc, fl);
}

void DpLegAndFoot::RenderHitBox(AuGLEffects& re)
{
	DhDNA::BONES thighBone;
	DhDNA::BONES calfBone;
	DhDNA::BONES footBone;
	Bones(thighBone, calfBone, footBone);

	// Calculate hip position relative to waist for origin of vertices.
	AuPoint loc = ConnectLoc();

	// Dimensions
	const float tl = m_Body->ThighLength();
	const float tt = m_Body->ThighThickness();
	const float cl = m_Body->CalfLength();
	const float ct = m_Body->CalfThickness();
	const float fl = m_Body->FootLength();
	float ft = fl/2.0;
	if(ft > ct)
		ft = ct * 1.1;	// Only let the feet be a tiny bit bigger than the calves.

	//qDebug("DpArmAndHand::CreateSkin - %s %s tl %g tt %g cl %g ct %g fl %g",
			//IsLeft()?"Left":"Right", "Leg", tl,tt,cl,ct,fl);

	// Leg
	// A different loc is calculated to give the legs more of a hip around the joint,
	// rather than ending perfectly AT the joint.
	float hipthick = tt/2.0;	// radius of hip around joint
	AuPoint tloc(loc);
	tloc.y -= hipthick;			// Leg is being constructed UP the +Y axis.

	// Test chunk intersection.
	//float avgrad = (tt + tt + ct + ct)/8.;
	float avgrad;
	if(tt > ct)
		avgrad = tt/2.f;
	else
		avgrad = ct/2.f;
	m_Body->RenderHitBoxChunk(re, thighBone, tloc, tl + hipthick, avgrad);

	loc.y += tl;

	// Test chunk intersection.
	//avgrad = (ct + ct + ft + ft)/8.;
	if(ct > ft)
		avgrad = ct/2.f;
	else
		avgrad = ft/2.f;
	m_Body->RenderHitBoxChunk(re, calfBone, loc, cl, avgrad);

	loc.y += cl;

	// Foot
	// Test chunk intersection.
	//avgrad = (ft + ft)/4.;
	avgrad = ft/2.f;
	m_Body->RenderHitBoxChunk(re, footBone, loc, fl, avgrad);
}

bool DpLegAndFoot::SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const
{
	bool hitMe = false;

	DhDNA::BONES thighBone;
	DhDNA::BONES calfBone;
	DhDNA::BONES footBone;
	Bones(thighBone, calfBone, footBone);

	// Calculate hip position relative to waist for origin of vertices.
	AuPoint loc = ConnectLoc();

	// Dimensions
	const float tl = m_Body->ThighLength();
	const float tt = m_Body->ThighThickness();
	const float cl = m_Body->CalfLength();
	const float ct = m_Body->CalfThickness();
	const float fl = m_Body->FootLength();
	float ft = fl/2.0;
	if(ft > ct)
		ft = ct * 1.1;	// Only let the feet be a tiny bit bigger than the calves.

	//qDebug("DpArmAndHand::CreateSkin - %s %s tl %g tt %g cl %g ct %g fl %g",
			//IsLeft()?"Left":"Right", "Leg", tl,tt,cl,ct,fl);

	// Leg
	// A different loc is calculated to give the legs more of a hip around the joint,
	// rather than ending perfectly AT the joint.
	float hipthick = tt/2.0;	// radius of hip around joint
	AuPoint tloc(loc);
	tloc.y -= hipthick;			// Leg is being constructed UP the +Y axis.

	// Test chunk intersection.
	//float avgrad = (tt + tt + ct + ct)/8.;
	float avgrad;
	if(tt > ct)
		avgrad = tt/2.f;
	else
		avgrad = ct/2.f;
	if(m_Body->SightChunk(thighBone, tloc, tloc + AuPoint(0,tl + hipthick), avgrad, eyeLoc, sightedLoc))
	{
		sightedPart = thighBone;
		hitMe = true;
	}

	loc.y += tl;

	// Test chunk intersection.
	//avgrad = (ct + ct + ft + ft)/8.;
	if(ct > ft)
		avgrad = ct/2.f;
	else
		avgrad = ft/2.f;
	if(m_Body->SightChunk(calfBone, loc, loc + AuPoint(0,cl), avgrad, eyeLoc, sightedLoc))
	{
		sightedPart = calfBone;
		hitMe = true;
	}

	loc.y += cl;

	// Foot
	// Test chunk intersection.
	//avgrad = (ft + ft)/4.;
	avgrad = ft/2.f;
	if(m_Body->SightChunk(footBone, loc, loc + AuPoint(0,fl), avgrad, eyeLoc, sightedLoc))
	{
		sightedPart = footBone;
		hitMe = true;
	}

	return hitMe;
}

float DpLegAndFoot::Volume() const
{
	// Dimensions
	float tl = m_Body->ThighLength();
	float tt = m_Body->ThighThickness();
	float cl = m_Body->CalfLength();
	float ct = m_Body->CalfThickness();
	float fl = m_Body->FootLength(true);
	float ft = fl/2.0;
	if(ft > ct)
		ft = ct * 1.1;	// Only let the feet be a tiny bit bigger than the calves.

	// Convert diameters to radii squared.
	tt /= 2;
	tt *= tt;
	ct /= 2;
	ct *= ct;
	ft /= 2;
	ft *= ft;

	float tca = tt * PIf;	// approx thigh cross sectional area
	float cca = ct * PIf;	// approx calf cross sectional area
	float fca = ft * PIf;	// approx foot cross sectional area

	float volume = tca * tl + cca * cl + fca * fl;
	//qDebug("DpLegAndFoot::Volume volume %g", volume);
	return volume;
}

quint64 DpLegAndFoot::MemoryUsage() const
{
	quint64 mem = DpBodyPart::MemoryUsage();
	mem += sizeof(m_Splay);
	mem += sizeof(m_Which);
	mem += sizeof(m_KneeLoc);
	mem += sizeof(m_AnkleLoc);
	return mem;
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

void DpLeftLegAndFoot::Bones(DhDNA::BONES& thigh, DhDNA::BONES& calf,
		DhDNA::BONES& foot) const
{
	switch(m_Which)
	{
	case 0:
		thigh = DhDNA::BONE_Thigh0L;
		calf = DhDNA::BONE_Calf0L;
		foot = DhDNA::BONE_Foot0L;
		break;
	case 1:
		thigh = DhDNA::BONE_Thigh1L;
		calf = DhDNA::BONE_Calf1L;
		foot = DhDNA::BONE_Foot1L;
		break;
	case 2:
		thigh = DhDNA::BONE_Thigh2L;
		calf = DhDNA::BONE_Calf2L;
		foot = DhDNA::BONE_Foot2L;
		break;
	}
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

void DpRightLegAndFoot::Bones(DhDNA::BONES& thigh, DhDNA::BONES& calf,
		DhDNA::BONES& foot) const
{
	switch(m_Which)
	{
	case 0:
		thigh = DhDNA::BONE_Thigh0R;
		calf = DhDNA::BONE_Calf0R;
		foot = DhDNA::BONE_Foot0R;
		break;
	case 1:
		thigh = DhDNA::BONE_Thigh1R;
		calf = DhDNA::BONE_Calf1R;
		foot = DhDNA::BONE_Foot1R;
		break;
	case 2:
		thigh = DhDNA::BONE_Thigh2R;
		calf = DhDNA::BONE_Calf2R;
		foot = DhDNA::BONE_Foot2R;
		break;
	}
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
