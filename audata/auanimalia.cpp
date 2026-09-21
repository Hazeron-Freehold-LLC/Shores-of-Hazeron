//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auanimalia.cpp,v 1.112 2022/05/09 16:11:35 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auanimalia.h"
//
#include "auanimalpart.h"
#include "aucombat.h"
#include "auelapsedtimer.h"
#include "austation.h"

AuAnimalia::AuAnimalia(const AuDNA& dna, AuDNA::CHROMATID chain, HANDEDNESS handedness, GROWTHSTAGE stage)
{
	m_BendDominantArm = false;
	m_Chain = chain;
	m_DNA = dna;
	m_GrowthStage = stage;
	m_Haircut = AuHairStyle::Unkempt;
	m_Handedness = handedness;
	m_HeadTilt = 0;
	m_HeadTurn = 0;
	m_HitPoints = 0;
	for(int ps=0; ps<AuAnimalPart::PS_Count; ps++)
		m_Pose[ps] = AuPose::Init;
	m_PoseHz = 0;
	m_Shave = AuHairStyle::Unshaven;

	m_DNA.MakeParts(*this, m_Chain);
}

AuAnimalia::AuAnimalia()
{
	m_BendDominantArm = false;
	m_Chain = AuDNA::Alpha;
	m_GrowthStage = Juvenile;
	m_Haircut = AuHairStyle::Unkempt;
	m_Handedness = LeftHanded;
	m_HeadTilt = 0;
	m_HeadTurn = 0;
	m_HitPoints = 0;
	for(int ps=0; ps<AuAnimalPart::PS_Count; ps++)
		m_Pose[ps] = AuPose::Init;
	m_PoseHz = 0;
	m_Shave = AuHairStyle::Unshaven;
}

AuAnimalia::~AuAnimalia()
{
	for(iterator mpi=begin(); mpi != end(); mpi++)
		delete *mpi;
}

AuAnimalia::AuAnimalia(const AuAnimalia& c) : QList<AuAnimalPart*>(), m_DNA(c.m_DNA), m_Clothing(c.m_Clothing)
{
	m_BendDominantArm = c.m_BendDominantArm;
	m_Chain = c.m_Chain;
	m_GrowthStage = c.m_GrowthStage;
	m_Haircut = c.m_Haircut;
	m_Handedness = c.m_Handedness;
	m_HeadTilt = c.m_HeadTilt;
	m_HeadTurn = c.m_HeadTurn;
	m_HitPoints = c.m_HitPoints;
	m_LastSpecUse = c.m_LastSpecUse;
	for(int ps=0; ps<AuAnimalPart::PS_Count; ps++)
		m_Pose[ps] = c.m_Pose[ps];
	m_PoseHz = c.m_PoseHz;
	m_Shave = c.m_Shave;

	for(const_iterator mpi = c.cbegin(); mpi != c.cend(); mpi++)
		append((*mpi)->Clone());

	m_AnimalClothingTex = c.m_AnimalClothingTex;

	m_AnimalVAO = c.m_AnimalVAO;
	m_AnimalClothingVAO = c.m_AnimalClothingVAO;
	m_AnimalClothingBlendVAO = c.m_AnimalClothingBlendVAO;
	m_AnimalHelmetBlendVAO = c.m_AnimalHelmetBlendVAO;
	m_AnimalHairBottomVAO = c.m_AnimalHairBottomVAO;
	m_AnimalHairTopVAO = c.m_AnimalHairTopVAO;
	m_HeadVAO = c.m_HeadVAO;
	m_HeadClothingVAO = c.m_HeadClothingVAO;
	m_HeadClothingBlendVAO = c.m_HeadClothingBlendVAO;
	m_HeadHelmetBlendVAO = c.m_HeadHelmetBlendVAO;
	m_HeadHairBottomVAO = c.m_HeadHairBottomVAO;
	m_HeadHairTopVAO = c.m_HeadHairTopVAO;
}

const AuAnimalia& AuAnimalia::operator=(const AuAnimalia& c)
{
	m_BendDominantArm = c.m_BendDominantArm;
	m_Chain = c.m_Chain;
	m_Clothing = c.m_Clothing;
	m_DNA = c.m_DNA;
	m_GrowthStage = c.m_GrowthStage;
	m_Haircut = c.m_Haircut;
	m_Handedness = c.m_Handedness;
	m_HeadTilt = c.m_HeadTilt;
	m_HeadTurn = c.m_HeadTurn;
	m_HitPoints = c.m_HitPoints;
	m_LastSpecUse = c.m_LastSpecUse;
	for(int ps=0; ps<AuAnimalPart::PS_Count; ps++)
		m_Pose[ps] = c.m_Pose[ps];
	m_PoseHz = c.m_PoseHz;
	m_Shave = c.m_Shave;

	while(!empty())
		delete takeFirst();
	for(const_iterator mpi = c.cbegin(); mpi != c.cend(); mpi++)
		append((*mpi)->Clone());

	m_AnimalClothingTex = c.m_AnimalClothingTex;

	m_AnimalVAO = c.m_AnimalVAO;
	m_AnimalClothingVAO = c.m_AnimalClothingVAO;
	m_AnimalClothingBlendVAO = c.m_AnimalClothingBlendVAO;
	m_AnimalHelmetBlendVAO = c.m_AnimalHelmetBlendVAO;
	m_AnimalHairBottomVAO = c.m_AnimalHairBottomVAO;
	m_AnimalHairTopVAO = c.m_AnimalHairTopVAO;
	m_HeadVAO = c.m_HeadVAO;
	m_HeadClothingVAO = c.m_HeadClothingVAO;
	m_HeadClothingBlendVAO = c.m_HeadClothingBlendVAO;
	m_HeadHelmetBlendVAO = c.m_HeadHelmetBlendVAO;
	m_HeadHairBottomVAO = c.m_HeadHairBottomVAO;
	m_HeadHairTopVAO = c.m_HeadHairTopVAO;
	return *this;
}

void AuAnimalia::ClearVAOs()
{
	//m_AnimalClothingTex.clear();
	m_AnimalVAO = AuGLVAO();
	m_AnimalClothingVAO = AuGLVAO();
	m_AnimalClothingBlendVAO = AuGLVAO();
	m_AnimalHelmetBlendVAO = AuGLVAO();
	m_AnimalHairBottomVAO = AuGLVAO();
	m_AnimalHairTopVAO = AuGLVAO();
	m_HeadVAO = AuGLVAO();
	m_HeadClothingVAO = AuGLVAO();
	m_HeadClothingBlendVAO = AuGLVAO();
	m_HeadHelmetBlendVAO = AuGLVAO();
	m_HeadHairBottomVAO = AuGLVAO();
	m_HeadHairTopVAO = AuGLVAO();
}

void AuAnimalia::CopyVAOs(const AuAnimalia& c)
{
	m_BendDominantArm = c.m_BendDominantArm;
	m_Clothing = c.m_Clothing;
	m_Haircut = c.m_Haircut;
	m_Shave = c.m_Shave;

	m_AnimalClothingTex = c.m_AnimalClothingTex;

	m_AnimalVAO = c.m_AnimalVAO;
	m_AnimalClothingVAO = c.m_AnimalClothingVAO;
	m_AnimalClothingBlendVAO = c.m_AnimalClothingBlendVAO;
	m_AnimalHelmetBlendVAO = c.m_AnimalHelmetBlendVAO;
	m_AnimalHairBottomVAO = c.m_AnimalHairBottomVAO;
	m_AnimalHairTopVAO = c.m_AnimalHairTopVAO;
	m_HeadVAO = c.m_HeadVAO;
	m_HeadClothingVAO = c.m_HeadClothingVAO;
	m_HeadClothingBlendVAO = c.m_HeadClothingBlendVAO;
	m_HeadHelmetBlendVAO = c.m_HeadHelmetBlendVAO;
	m_HeadHairBottomVAO = c.m_HeadHairBottomVAO;
	m_HeadHairTopVAO = c.m_HeadHairTopVAO;
}

bool AuAnimalia::Is(const AuDNA& dna, AuDNA::CHROMATID chain, HANDEDNESS handedness) const
{
	if(chain != m_Chain)
		return false;
	if(handedness != m_Handedness)
		return false;
	if(dna != m_DNA)
		return false;
	return true;
}

//static
void AuAnimalia::InitPoses(const AuPose& pose, AuPose poses[AuAnimalPart::PS_Count])
{
	if(pose.IsMouth())
	{
		// Body poses must not be mouth poses. Init them to standing.
		for(int ps=0; ps<AuAnimalPart::PS_Mouth; ps++)
			poses[ps] = AuPose(AuPose::Stand);
		// Assign the mouth pose.
		poses[AuAnimalPart::PS_Mouth] = pose;
	}
	else
	{
		// Assign the body pose to all its parts.
		for(int ps=0; ps<AuAnimalPart::PS_Mouth; ps++)
			poses[ps] = pose;
		// The mouth pose must not be a body pose.
		poses[AuAnimalPart::PS_Mouth] = AuPose(AuPose::MouthClosed);
	}
}

bool AuAnimalia::Pose(const AuId& id, const AuPose pose[AuAnimalPart::PS_Count], bool bendDominantArm, float poseHz)
{
	bool anychange = false;
	for(int ps=0; ps<AuAnimalPart::PS_Count; ps++)
	{
		if(m_Pose[ps] != pose[ps])
		{
			m_Pose[ps] = pose[ps];
			anychange = true;
		}
	}
	m_BendDominantArm = bendDominantArm;
	m_PoseHz = poseHz;
	if(!m_Pose[AuAnimalPart::PS_Mouth].IsMouth())
	{
		qWarning() << "error: AuAnimalia::Pose not a mouth pose" << m_Pose[AuAnimalPart::PS_Mouth];
		m_Pose[AuAnimalPart::PS_Mouth] = AuPose(AuPose::MouthClosed);
	}
	AnimatePose(id, AuElapsedTimer::Now());
	return anychange;
}

void AuAnimalia::PoseMouth(const AuId& id, const AuPose& pose)
{
	Q_UNUSED(id);
	if(pose.IsMouth())
		m_Pose[AuAnimalPart::PS_Mouth] = pose;
	else
	{
		qWarning() << "error: AuAnimalia::PoseMouth not a mouth pose" << pose;
		m_Pose[AuAnimalPart::PS_Mouth] = AuPose(AuPose::MouthClosed);
	}
	AnimatePose(id, AuElapsedTimer::Now());
}

// Position body based on poses of each section.
void AuAnimalia::AnimatePose(const AuId& id, const AuElapsedTimer& when)
{
	for(iterator mpi=begin(); mpi != end(); mpi++)
		if((*mpi)->AnimatePose(id, when, *this))
		{
			// The body part has finished a do-once pose action. Restore a sensible pose to that section.
			AuAnimalPart::SECTION ps = (*mpi)->PoseSection();
			if(ps != AuAnimalPart::PS_General)
				m_Pose[ps] = m_Pose[AuAnimalPart::PS_General];	// Return the section to the general body pose.
			else
				m_Pose[ps] = AuPose::Stand;			// The section is the general body so restore it to standing.
		}
}

void AuAnimalia::RenderAnimal(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
		quint8 visorState, const AuPoint& ambientUp)
{
	if(CurrentGeneralPose().HeadOnly())
	{
		RenderHead(projection, model, re, blended, visorState, ambientUp);
		return;
	}

	if(!m_AnimalVAO.VAO())
		return;
	if(!blended)
	{
		QString topSkinTexId, topSkinTexFile, bottomSkinTexId, bottomSkinTexFile, decalTexId, decalTexFile;
		QColor topSkinColor, bottomSkinColor;
		m_DNA.SkinRenderParameters(m_Chain, topSkinTexId, topSkinTexFile, topSkinColor, bottomSkinTexId, bottomSkinTexFile,
				bottomSkinColor, decalTexId, decalTexFile);
		if(topSkinTexFile.isEmpty())
		{
			topSkinTexId = QString("DNASkinNoTop");
			topSkinTexFile = QString(":data/NoTop.png");
		}
		if(bottomSkinTexFile.isEmpty())
		{
			bottomSkinTexId = QString("DNASkinNoBottom");
			bottomSkinTexFile = QString(":data/NoBottom.png");
		}
		if(decalTexFile.isEmpty())
		{
			decalTexId = QString("DNASkinNoDecal");
			decalTexFile = QString(":data/NoDecal.png");
		}

		GLuint skinTop = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, topSkinTexId, topSkinTexFile, GL_LINEAR, GL_LINEAR, true);
		GLuint skinBottom = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, bottomSkinTexId, bottomSkinTexFile, GL_LINEAR, GL_LINEAR,
				true);
		GLuint skinDetail = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, decalTexId, decalTexFile, GL_LINEAR, GL_LINEAR, true);

		// Configure shader.
		re.GLSL().AnimalShader().Select(projection, re);
		re.GLSL().AnimalShader().SetModelView(model);
		re.GLSL().AnimalShader().SetSkinBottomColor(bottomSkinColor);
		re.GLSL().AnimalShader().SetSkinBottomTexture(skinBottom);
		re.GLSL().AnimalShader().SetSkinDetailTexture(skinDetail);
		re.GLSL().AnimalShader().SetSkinTopColor(topSkinColor);
		re.GLSL().AnimalShader().SetSkinTopTexture(skinTop);
		re.GLSL().AnimalShader().SetGlowAlpha(0, 0, 0, 1);

		// Bones
		quint8 boneIndex = 0;
		for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
		{
			for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
			{
				re.GLSL().AnimalShader().SetBone(boneIndex, (*bi).Matrix());
				boneIndex++;
			}
		}

		// Ambient light
		re.GLSL().AnimalShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

		// Render the model.
		m_AnimalVAO.SelectGL();
		m_AnimalVAO.DrawGL();
		m_AnimalVAO.DeselectGL();

		re.GLSL().AnimalShader().Deselect();
	}

	// Hair must be drawn before clothing so it is visible inside a space suit helmet.
	RenderAnimalHair(projection, model, re, blended, ambientUp);

	RenderAnimalClothing(projection, model, re, blended, ambientUp);

	if(visorState)
		RenderAnimalHelmetVisor(projection, model, re, blended, ambientUp);
}

void AuAnimalia::RenderAnimalGlobeFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
		quint8 visorState, const AuPoint& ambientUp, const AuPoint& globeP)
{
	if(CurrentGeneralPose().HeadOnly())
	{
		RenderHeadGlobeFog(projection, model, re, blended, visorState, ambientUp, globeP);
		return;
	}

	if(!m_AnimalVAO.VAO())
		return;
	if(!blended)
	{
		QString topSkinTexId, topSkinTexFile, bottomSkinTexId, bottomSkinTexFile, decalTexId, decalTexFile;
		QColor topSkinColor, bottomSkinColor;
		m_DNA.SkinRenderParameters(m_Chain, topSkinTexId, topSkinTexFile, topSkinColor, bottomSkinTexId, bottomSkinTexFile,
				bottomSkinColor, decalTexId, decalTexFile);
		if(topSkinTexFile.isEmpty())
		{
			topSkinTexId = QString("DNASkinNoTop");
			topSkinTexFile = QString(":data/NoTop.png");
		}
		if(bottomSkinTexFile.isEmpty())
		{
			bottomSkinTexId = QString("DNASkinNoBottom");
			bottomSkinTexFile = QString(":data/NoBottom.png");
		}
		if(decalTexFile.isEmpty())
		{
			decalTexId = QString("DNASkinNoDecal");
			decalTexFile = QString(":data/NoDecal.png");
		}

		GLuint skinTop = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, topSkinTexId, topSkinTexFile, GL_LINEAR, GL_LINEAR, true);
		GLuint skinBottom = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, bottomSkinTexId, bottomSkinTexFile, GL_LINEAR, GL_LINEAR,
				true);
		GLuint skinDetail = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, decalTexId, decalTexFile, GL_LINEAR, GL_LINEAR, true);

		// Configure shader.
		re.GLSL().AnimalGlobeFogShader().Select(projection, re);
		re.GLSL().AnimalGlobeFogShader().SetModelView(model);
		re.GLSL().AnimalGlobeFogShader().SetSkinBottomColor(bottomSkinColor);
		re.GLSL().AnimalGlobeFogShader().SetSkinBottomTexture(skinBottom);
		re.GLSL().AnimalGlobeFogShader().SetSkinDetailTexture(skinDetail);
		re.GLSL().AnimalGlobeFogShader().SetSkinTopColor(topSkinColor);
		re.GLSL().AnimalGlobeFogShader().SetSkinTopTexture(skinTop);
		re.GLSL().AnimalGlobeFogShader().SetGlowAlpha(0, 0, 0, 1);

		// Bones
		quint8 boneIndex = 0;
		for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
		{
			for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
			{
				re.GLSL().AnimalGlobeFogShader().SetBone(boneIndex, (*bi).Matrix());
				boneIndex++;
			}
		}

		// Ambient light
		re.GLSL().AnimalGlobeFogShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);
		re.GLSL().AnimalGlobeFogShader().SetGlobeLocation(globeP.x, globeP.y, globeP.z);

		// Render the model.
		m_AnimalVAO.SelectGL();
		m_AnimalVAO.DrawGL();
		m_AnimalVAO.DeselectGL();

		re.GLSL().AnimalGlobeFogShader().Deselect();
	}

	// Hair must be drawn before clothing so it is visible inside a space suit helmet.
	RenderAnimalHairGlobeFog(projection, model, re, blended, ambientUp, globeP);

	RenderAnimalClothingGlobeFog(projection, model, re, blended, ambientUp, globeP);

	if(visorState)
		RenderAnimalHelmetVisorGlobeFog(projection, model, re, blended, ambientUp, globeP);
}

void AuAnimalia::RenderAnimalRingworldFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
		quint8 visorState, const AuPoint& ambientUp, float shadowPanelSunAngles[MAX_SUNS])
{
	if(CurrentGeneralPose().HeadOnly())
	{
		RenderHeadRingworldFog(projection, model, re, blended, visorState, ambientUp, shadowPanelSunAngles);
		return;
	}

	if(!m_AnimalVAO.VAO())
		return;
	if(!blended)
	{
		QString topSkinTexId, topSkinTexFile, bottomSkinTexId, bottomSkinTexFile, decalTexId, decalTexFile;
		QColor topSkinColor, bottomSkinColor;
		m_DNA.SkinRenderParameters(m_Chain, topSkinTexId, topSkinTexFile, topSkinColor, bottomSkinTexId, bottomSkinTexFile,
				bottomSkinColor, decalTexId, decalTexFile);
		if(topSkinTexFile.isEmpty())
		{
			topSkinTexId = QString("DNASkinNoTop");
			topSkinTexFile = QString(":data/NoTop.png");
		}
		if(bottomSkinTexFile.isEmpty())
		{
			bottomSkinTexId = QString("DNASkinNoBottom");
			bottomSkinTexFile = QString(":data/NoBottom.png");
		}
		if(decalTexFile.isEmpty())
		{
			decalTexId = QString("DNASkinNoDecal");
			decalTexFile = QString(":data/NoDecal.png");
		}

		GLuint skinTop = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, topSkinTexId, topSkinTexFile, GL_LINEAR, GL_LINEAR, true);
		GLuint skinBottom = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, bottomSkinTexId, bottomSkinTexFile, GL_LINEAR, GL_LINEAR,
				true);
		GLuint skinDetail = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, decalTexId, decalTexFile, GL_LINEAR, GL_LINEAR, true);

		// Configure shader.
		re.GLSL().AnimalRingworldFogShader().Select(projection, re);
		re.GLSL().AnimalRingworldFogShader().SetModelView(model);
		re.GLSL().AnimalRingworldFogShader().SetSkinBottomColor(bottomSkinColor);
		re.GLSL().AnimalRingworldFogShader().SetSkinBottomTexture(skinBottom);
		re.GLSL().AnimalRingworldFogShader().SetSkinDetailTexture(skinDetail);
		re.GLSL().AnimalRingworldFogShader().SetSkinTopColor(topSkinColor);
		re.GLSL().AnimalRingworldFogShader().SetSkinTopTexture(skinTop);
		re.GLSL().AnimalRingworldFogShader().SetGlowAlpha(0, 0, 0, 1);

		// Bones
		quint8 boneIndex = 0;
		for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
		{
			for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
			{
				re.GLSL().AnimalRingworldFogShader().SetBone(boneIndex, (*bi).Matrix());
				boneIndex++;
			}
		}

		// Ambient light
		re.GLSL().AnimalRingworldFogShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

		// Suns
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
			re.GLSL().AnimalRingworldFogShader().SetSunShadowAngle(sunIndex, shadowPanelSunAngles[sunIndex]);

		// Render the model.
		m_AnimalVAO.SelectGL();
		m_AnimalVAO.DrawGL();
		m_AnimalVAO.DeselectGL();

		re.GLSL().AnimalRingworldFogShader().Deselect();
	}

	// Hair must be drawn before clothing so it is visible inside a space suit helmet.
	RenderAnimalHairRingworldFog(projection, model, re, blended, ambientUp, shadowPanelSunAngles);

	RenderAnimalClothingRingworldFog(projection, model, re, blended, ambientUp, shadowPanelSunAngles);

	if(visorState)
		RenderAnimalHelmetVisorRingworldFog(projection, model, re, blended, ambientUp, shadowPanelSunAngles);
}

void AuAnimalia::RenderHead(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
		quint8 visorState, const AuPoint& ambientUp)
{
	if(!m_HeadVAO.VAO())
		return;
	if(!blended)
	{
		QString topSkinTexId, topSkinTexFile, bottomSkinTexId, bottomSkinTexFile, decalTexId, decalTexFile;
		QColor topSkinColor, bottomSkinColor;
		m_DNA.SkinRenderParameters(m_Chain, topSkinTexId, topSkinTexFile, topSkinColor, bottomSkinTexId, bottomSkinTexFile,
				bottomSkinColor, decalTexId, decalTexFile);
		if(topSkinTexFile.isEmpty())
		{
			topSkinTexId = QString("DNASkinNoTop");
			topSkinTexFile = QString(":data/NoTop.png");
		}
		if(bottomSkinTexFile.isEmpty())
		{
			bottomSkinTexId = QString("DNASkinNoBottom");
			bottomSkinTexFile = QString(":data/NoBottom.png");
		}
		if(decalTexFile.isEmpty())
		{
			decalTexId = QString("DNASkinNoDecal");
			decalTexFile = QString(":data/NoDecal.png");
		}

		GLuint skinTop = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, topSkinTexId, topSkinTexFile, GL_LINEAR, GL_LINEAR, true);
		GLuint skinBottom = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, bottomSkinTexId, bottomSkinTexFile, GL_LINEAR, GL_LINEAR,
				true);
		GLuint skinDetail = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, decalTexId, decalTexFile, GL_LINEAR, GL_LINEAR, true);

		// Configure shader.
		re.GLSL().AnimalShader().Select(projection, re);
		re.GLSL().AnimalShader().SetModelView(model);
		re.GLSL().AnimalShader().SetSkinBottomColor(bottomSkinColor);
		re.GLSL().AnimalShader().SetSkinBottomTexture(skinBottom);
		re.GLSL().AnimalShader().SetSkinDetailTexture(skinDetail);
		re.GLSL().AnimalShader().SetSkinTopColor(topSkinColor);
		re.GLSL().AnimalShader().SetSkinTopTexture(skinTop);
		re.GLSL().AnimalShader().SetGlowAlpha(0, 0, 0, 1);

		// Bones
		quint8 boneIndex = 0;
		for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
		{
			for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
			{
				re.GLSL().AnimalShader().SetBone(boneIndex, (*bi).Matrix());
				boneIndex++;
			}
		}

		// Ambient light
		re.GLSL().AnimalShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

		// Render the model.
		m_HeadVAO.SelectGL();
		m_HeadVAO.DrawGL();
		m_HeadVAO.DeselectGL();

		re.GLSL().AnimalShader().Deselect();
	}

	// Hair must be drawn before clothing so it is visible inside a space suit helmet.
	RenderHeadHair(projection, model, re, blended, ambientUp);

	RenderHeadClothing(projection, model, re, blended, ambientUp);

	if(visorState)
		RenderHeadHelmetVisor(projection, model, re, blended, ambientUp);
}

void AuAnimalia::RenderHeadGlobeFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
		quint8 visorState, const AuPoint& ambientUp, const AuPoint& globeP)
{
	if(!m_HeadVAO.VAO())
		return;
	if(!blended)
	{
		QString topSkinTexId, topSkinTexFile, bottomSkinTexId, bottomSkinTexFile, decalTexId, decalTexFile;
		QColor topSkinColor, bottomSkinColor;
		m_DNA.SkinRenderParameters(m_Chain, topSkinTexId, topSkinTexFile, topSkinColor, bottomSkinTexId, bottomSkinTexFile,
				bottomSkinColor, decalTexId, decalTexFile);
		if(topSkinTexFile.isEmpty())
		{
			topSkinTexId = QString("DNASkinNoTop");
			topSkinTexFile = QString(":data/NoTop.png");
		}
		if(bottomSkinTexFile.isEmpty())
		{
			bottomSkinTexId = QString("DNASkinNoBottom");
			bottomSkinTexFile = QString(":data/NoBottom.png");
		}
		if(decalTexFile.isEmpty())
		{
			decalTexId = QString("DNASkinNoDecal");
			decalTexFile = QString(":data/NoDecal.png");
		}

		GLuint skinTop = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, topSkinTexId, topSkinTexFile, GL_LINEAR, GL_LINEAR, true);
		GLuint skinBottom = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, bottomSkinTexId, bottomSkinTexFile, GL_LINEAR, GL_LINEAR,
				true);
		GLuint skinDetail = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, decalTexId, decalTexFile, GL_LINEAR, GL_LINEAR, true);

		// Configure shader.
		re.GLSL().AnimalGlobeFogShader().Select(projection, re);
		re.GLSL().AnimalGlobeFogShader().SetModelView(model);
		re.GLSL().AnimalGlobeFogShader().SetSkinBottomColor(bottomSkinColor);
		re.GLSL().AnimalGlobeFogShader().SetSkinBottomTexture(skinBottom);
		re.GLSL().AnimalGlobeFogShader().SetSkinDetailTexture(skinDetail);
		re.GLSL().AnimalGlobeFogShader().SetSkinTopColor(topSkinColor);
		re.GLSL().AnimalGlobeFogShader().SetSkinTopTexture(skinTop);
		re.GLSL().AnimalGlobeFogShader().SetGlowAlpha(0, 0, 0, 1);

		// Bones
		quint8 boneIndex = 0;
		for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
		{
			for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
			{
				re.GLSL().AnimalGlobeFogShader().SetBone(boneIndex, (*bi).Matrix());
				boneIndex++;
			}
		}

		// Ambient light
		re.GLSL().AnimalGlobeFogShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);
		re.GLSL().AnimalGlobeFogShader().SetGlobeLocation(globeP.x, globeP.y, globeP.z);

		// Render the model.
		m_HeadVAO.SelectGL();
		m_HeadVAO.DrawGL();
		m_HeadVAO.DeselectGL();

		re.GLSL().AnimalGlobeFogShader().Deselect();
	}

	// Hair must be drawn before clothing so it is visible inside a space suit helmet.
	RenderHeadHairGlobeFog(projection, model, re, blended, ambientUp, globeP);

	RenderHeadClothingGlobeFog(projection, model, re, blended, ambientUp, globeP);

	if(visorState)
		RenderHeadHelmetVisorGlobeFog(projection, model, re, blended, ambientUp, globeP);
}

void AuAnimalia::RenderHeadRingworldFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
		quint8 visorState, const AuPoint& ambientUp, float shadowPanelSunAngles[MAX_SUNS])
{
	if(!m_HeadVAO.VAO())
		return;
	if(!blended)
	{
		QString topSkinTexId, topSkinTexFile, bottomSkinTexId, bottomSkinTexFile, decalTexId, decalTexFile;
		QColor topSkinColor, bottomSkinColor;
		m_DNA.SkinRenderParameters(m_Chain, topSkinTexId, topSkinTexFile, topSkinColor, bottomSkinTexId, bottomSkinTexFile,
				bottomSkinColor, decalTexId, decalTexFile);
		if(topSkinTexFile.isEmpty())
		{
			topSkinTexId = QString("DNASkinNoTop");
			topSkinTexFile = QString(":data/NoTop.png");
		}
		if(bottomSkinTexFile.isEmpty())
		{
			bottomSkinTexId = QString("DNASkinNoBottom");
			bottomSkinTexFile = QString(":data/NoBottom.png");
		}
		if(decalTexFile.isEmpty())
		{
			decalTexId = QString("DNASkinNoDecal");
			decalTexFile = QString(":data/NoDecal.png");
		}

		GLuint skinTop = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, topSkinTexId, topSkinTexFile, GL_LINEAR, GL_LINEAR, true);
		GLuint skinBottom = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, bottomSkinTexId, bottomSkinTexFile, GL_LINEAR, GL_LINEAR,
				true);
		GLuint skinDetail = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, decalTexId, decalTexFile, GL_LINEAR, GL_LINEAR, true);

		// Configure shader.
		re.GLSL().AnimalRingworldFogShader().Select(projection, re);
		re.GLSL().AnimalRingworldFogShader().SetModelView(model);
		re.GLSL().AnimalRingworldFogShader().SetSkinBottomColor(bottomSkinColor);
		re.GLSL().AnimalRingworldFogShader().SetSkinBottomTexture(skinBottom);
		re.GLSL().AnimalRingworldFogShader().SetSkinDetailTexture(skinDetail);
		re.GLSL().AnimalRingworldFogShader().SetSkinTopColor(topSkinColor);
		re.GLSL().AnimalRingworldFogShader().SetSkinTopTexture(skinTop);
		re.GLSL().AnimalRingworldFogShader().SetGlowAlpha(0, 0, 0, 1);

		// Bones
		quint8 boneIndex = 0;
		for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
		{
			for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
			{
				re.GLSL().AnimalRingworldFogShader().SetBone(boneIndex, (*bi).Matrix());
				boneIndex++;
			}
		}

		// Ambient light
		re.GLSL().AnimalRingworldFogShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

		// Suns
		for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
			re.GLSL().AnimalRingworldFogShader().SetSunShadowAngle(sunIndex, shadowPanelSunAngles[sunIndex]);

		// Render the model.
		m_HeadVAO.SelectGL();
		m_HeadVAO.DrawGL();
		m_HeadVAO.DeselectGL();

		re.GLSL().AnimalRingworldFogShader().Deselect();
	}

	// Hair must be drawn before clothing so it is visible inside a space suit helmet.
	RenderHeadHairRingworldFog(projection, model, re, blended, ambientUp, shadowPanelSunAngles);

	RenderHeadClothingRingworldFog(projection, model, re, blended, ambientUp, shadowPanelSunAngles);

	if(visorState)
		RenderHeadHelmetVisorRingworldFog(projection, model, re, blended, ambientUp, shadowPanelSunAngles);
}

void AuAnimalia::RenderAnimalClothing(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
		const AuPoint& ambientUp)
{
	if(blended)
	{
		if(!m_AnimalClothingBlendVAO.VAO())
			return;
	}
	else
	{
		if(!m_AnimalClothingVAO.VAO())
			return;
	}

	QString texFileDecal(":data/ClothDecal.png");
	GLuint clothDecals = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "ClothDecal", texFileDecal, GL_LINEAR, GL_LINEAR, true, 1);

	// Configure shader.
	re.GLSL().AnimalClothingShader().Select(projection, re);
	re.GLSL().AnimalClothingShader().SetModelView(model);
	re.GLSL().AnimalClothingShader().SetClothingDecalTexture(clothDecals);
	int idx = 0;
	for(AuClothingTextureArray::const_iterator cti = m_AnimalClothingTex.cbegin(); cti != m_AnimalClothingTex.cend(); cti++, idx++)
		re.GLSL().AnimalClothingShader().SetClothingTexture(idx, (*cti).Color(), (*cti).Texture());
	re.GLSL().AnimalClothingShader().SetGlowAlpha(0, 0, 0, 1);

	// Bones
	quint8 boneIndex = 0;
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
		{
			re.GLSL().AnimalClothingShader().SetBone(boneIndex, (*bi).Matrix());
			boneIndex++;
		}
	}

	// Ambient light
	re.GLSL().AnimalClothingShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

	// Render the model.
	if(!blended)
	{
		re.glDisable(GL_CULL_FACE);

		m_AnimalClothingVAO.SelectGL();
		m_AnimalClothingVAO.DrawGL();
		m_AnimalClothingVAO.DeselectGL();

		re.glEnable(GL_CULL_FACE);
	}
	else
	{
		//re.glDisable(GL_CULL_FACE);
		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_AnimalClothingBlendVAO.SelectGL();
		m_AnimalClothingBlendVAO.DrawGL();
		m_AnimalClothingBlendVAO.DeselectGL();

		re.glDisable(GL_BLEND);
		//re.glEnable(GL_CULL_FACE);
	}

	re.GLSL().AnimalClothingShader().Deselect();
}

void AuAnimalia::RenderAnimalClothingGlobeFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		bool blended, const AuPoint& ambientUp, const AuPoint& globeP)
{
	if(blended)
	{
		if(!m_AnimalClothingBlendVAO.VAO())
			return;
	}
	else
	{
		if(!m_AnimalClothingVAO.VAO())
			return;
	}

	QString texFileDecal(":data/ClothDecal.png");
	GLuint clothDecals = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "ClothDecal", texFileDecal, GL_LINEAR, GL_LINEAR, true, 1);

	// Configure shader.
	re.GLSL().AnimalClothingGlobeFogShader().Select(projection, re);
	re.GLSL().AnimalClothingGlobeFogShader().SetModelView(model);
	re.GLSL().AnimalClothingGlobeFogShader().SetClothingDecalTexture(clothDecals);
	int idx = 0;
	for(AuClothingTextureArray::const_iterator cti = m_AnimalClothingTex.cbegin(); cti != m_AnimalClothingTex.cend(); cti++, idx++)
		re.GLSL().AnimalClothingGlobeFogShader().SetClothingTexture(idx, (*cti).Color(), (*cti).Texture());
	re.GLSL().AnimalClothingGlobeFogShader().SetGlowAlpha(0, 0, 0, 1);

	// Bones
	quint8 boneIndex = 0;
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
		{
			re.GLSL().AnimalClothingGlobeFogShader().SetBone(boneIndex, (*bi).Matrix());
			boneIndex++;
		}
	}

	// Ambient light
	re.GLSL().AnimalClothingGlobeFogShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);
	re.GLSL().AnimalClothingGlobeFogShader().SetGlobeLocation(globeP.x, globeP.y, globeP.z);

	// Render the model.
	if(!blended)
	{
		re.glDisable(GL_CULL_FACE);

		m_AnimalClothingVAO.SelectGL();
		m_AnimalClothingVAO.DrawGL();
		m_AnimalClothingVAO.DeselectGL();

		re.glEnable(GL_CULL_FACE);
	}
	else
	{
		//re.glDisable(GL_CULL_FACE);
		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_AnimalClothingBlendVAO.SelectGL();
		m_AnimalClothingBlendVAO.DrawGL();
		m_AnimalClothingBlendVAO.DeselectGL();

		re.glDisable(GL_BLEND);
		//re.glEnable(GL_CULL_FACE);
	}

	re.GLSL().AnimalClothingGlobeFogShader().Deselect();
}

void AuAnimalia::RenderAnimalClothingRingworldFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		bool blended, const AuPoint& ambientUp, float shadowPanelSunAngles[MAX_SUNS])
{
	if(blended)
	{
		if(!m_AnimalClothingBlendVAO.VAO())
			return;
	}
	else
	{
		if(!m_AnimalClothingVAO.VAO())
			return;
	}

	QString texFileDecal(":data/ClothDecal.png");
	GLuint clothDecals = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "ClothDecal", texFileDecal, GL_LINEAR, GL_LINEAR, true, 1);

	// Configure shader.
	re.GLSL().AnimalClothingRingworldFogShader().Select(projection, re);
	re.GLSL().AnimalClothingRingworldFogShader().SetModelView(model);
	re.GLSL().AnimalClothingRingworldFogShader().SetClothingDecalTexture(clothDecals);
	int idx = 0;
	for(AuClothingTextureArray::const_iterator cti = m_AnimalClothingTex.cbegin(); cti != m_AnimalClothingTex.cend(); cti++, idx++)
		re.GLSL().AnimalClothingRingworldFogShader().SetClothingTexture(idx, (*cti).Color(), (*cti).Texture());
	re.GLSL().AnimalClothingRingworldFogShader().SetGlowAlpha(0, 0, 0, 1);

	// Bones
	quint8 boneIndex = 0;
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
		{
			re.GLSL().AnimalClothingRingworldFogShader().SetBone(boneIndex, (*bi).Matrix());
			boneIndex++;
		}
	}

	// Ambient light
	re.GLSL().AnimalClothingRingworldFogShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

	// Suns
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		re.GLSL().AnimalClothingRingworldFogShader().SetSunShadowAngle(sunIndex, shadowPanelSunAngles[sunIndex]);

	// Render the model.
	if(!blended)
	{
		re.glDisable(GL_CULL_FACE);

		m_AnimalClothingVAO.SelectGL();
		m_AnimalClothingVAO.DrawGL();
		m_AnimalClothingVAO.DeselectGL();

		re.glEnable(GL_CULL_FACE);
	}
	else
	{
		//re.glDisable(GL_CULL_FACE);
		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_AnimalClothingBlendVAO.SelectGL();
		m_AnimalClothingBlendVAO.DrawGL();
		m_AnimalClothingBlendVAO.DeselectGL();

		re.glDisable(GL_BLEND);
		//re.glEnable(GL_CULL_FACE);
	}

	re.GLSL().AnimalClothingRingworldFogShader().Deselect();
}

void AuAnimalia::RenderAnimalHelmetVisor(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
		const AuPoint& ambientUp)
{
	if(blended)
	{
		if(!m_AnimalHelmetBlendVAO.VAO())
			return;
	}
	else
	{
		// Currently no non-blended visor parts.
		return;
	}

	QString texFileDecal(":data/ClothDecal.png");
	GLuint clothDecals = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "ClothDecal", texFileDecal, GL_LINEAR, GL_LINEAR, true, 1);

	// Configure shader.
	re.GLSL().AnimalClothingShader().Select(projection, re);
	re.GLSL().AnimalClothingShader().SetModelView(model);
	re.GLSL().AnimalClothingShader().SetClothingDecalTexture(clothDecals);
	int idx = 0;
	for(AuClothingTextureArray::const_iterator cti = m_AnimalClothingTex.cbegin(); cti != m_AnimalClothingTex.cend(); cti++, idx++)
		re.GLSL().AnimalClothingShader().SetClothingTexture(idx, (*cti).Color(), (*cti).Texture());
	re.GLSL().AnimalClothingShader().SetGlowAlpha(0, 0, 0, 1);

	// Bones
	quint8 boneIndex = 0;
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
		{
			re.GLSL().AnimalClothingShader().SetBone(boneIndex, (*bi).Matrix());
			boneIndex++;
		}
	}

	// Ambient light
	re.GLSL().AnimalClothingShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

	// Render the model.
	if(!blended)
	{
		/*
		re.glDisable(GL_CULL_FACE);

		m_AnimalClothingVAO.SelectGL();
		m_AnimalClothingVAO.DrawGL();
		m_AnimalClothingVAO.DeselectGL();

		re.glEnable(GL_CULL_FACE);
		*/
	}
	else
	{
		//re.glDisable(GL_CULL_FACE);
		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_AnimalHelmetBlendVAO.SelectGL();
		m_AnimalHelmetBlendVAO.DrawGL();
		m_AnimalHelmetBlendVAO.DeselectGL();

		re.glDisable(GL_BLEND);
		//re.glEnable(GL_CULL_FACE);
	}

	re.GLSL().AnimalClothingShader().Deselect();
}

void AuAnimalia::RenderAnimalHelmetVisorGlobeFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		bool blended, const AuPoint& ambientUp, const AuPoint& globeP)
{
	if(blended)
	{
		if(!m_AnimalHelmetBlendVAO.VAO())
			return;
	}
	else
	{
		// Currently no non-blended visor parts.
		return;
	}

	QString texFileDecal(":data/ClothDecal.png");
	GLuint clothDecals = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "ClothDecal", texFileDecal, GL_LINEAR, GL_LINEAR, true, 1);

	// Configure shader.
	re.GLSL().AnimalClothingGlobeFogShader().Select(projection, re);
	re.GLSL().AnimalClothingGlobeFogShader().SetModelView(model);
	re.GLSL().AnimalClothingGlobeFogShader().SetClothingDecalTexture(clothDecals);
	int idx = 0;
	for(AuClothingTextureArray::const_iterator cti = m_AnimalClothingTex.cbegin(); cti != m_AnimalClothingTex.cend(); cti++, idx++)
		re.GLSL().AnimalClothingGlobeFogShader().SetClothingTexture(idx, (*cti).Color(), (*cti).Texture());
	re.GLSL().AnimalClothingGlobeFogShader().SetGlowAlpha(0, 0, 0, 1);

	// Bones
	quint8 boneIndex = 0;
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
		{
			re.GLSL().AnimalClothingGlobeFogShader().SetBone(boneIndex, (*bi).Matrix());
			boneIndex++;
		}
	}

	// Ambient light
	re.GLSL().AnimalClothingGlobeFogShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);
	re.GLSL().AnimalClothingGlobeFogShader().SetGlobeLocation(globeP.x, globeP.y, globeP.z);

	// Render the model.
	if(!blended)
	{
		/*
		re.glDisable(GL_CULL_FACE);

		m_AnimalClothingVAO.SelectGL();
		m_AnimalClothingVAO.DrawGL();
		m_AnimalClothingVAO.DeselectGL();

		re.glEnable(GL_CULL_FACE);
		*/
	}
	else
	{
		//re.glDisable(GL_CULL_FACE);
		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_AnimalHelmetBlendVAO.SelectGL();
		m_AnimalHelmetBlendVAO.DrawGL();
		m_AnimalHelmetBlendVAO.DeselectGL();

		re.glDisable(GL_BLEND);
		//re.glEnable(GL_CULL_FACE);
	}

	re.GLSL().AnimalClothingGlobeFogShader().Deselect();
}

void AuAnimalia::RenderAnimalHelmetVisorRingworldFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		bool blended, const AuPoint& ambientUp, float shadowPanelSunAngles[MAX_SUNS])
{
	if(blended)
	{
		if(!m_AnimalHelmetBlendVAO.VAO())
			return;
	}
	else
	{
		// Currently no non-blended visor parts.
		return;
	}

	QString texFileDecal(":data/ClothDecal.png");
	GLuint clothDecals = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "ClothDecal", texFileDecal, GL_LINEAR, GL_LINEAR, true, 1);

	// Configure shader.
	re.GLSL().AnimalClothingRingworldFogShader().Select(projection, re);
	re.GLSL().AnimalClothingRingworldFogShader().SetModelView(model);
	re.GLSL().AnimalClothingRingworldFogShader().SetClothingDecalTexture(clothDecals);
	int idx = 0;
	for(AuClothingTextureArray::const_iterator cti = m_AnimalClothingTex.cbegin(); cti != m_AnimalClothingTex.cend(); cti++, idx++)
		re.GLSL().AnimalClothingRingworldFogShader().SetClothingTexture(idx, (*cti).Color(), (*cti).Texture());
	re.GLSL().AnimalClothingRingworldFogShader().SetGlowAlpha(0, 0, 0, 1);

	// Bones
	quint8 boneIndex = 0;
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
		{
			re.GLSL().AnimalClothingRingworldFogShader().SetBone(boneIndex, (*bi).Matrix());
			boneIndex++;
		}
	}

	// Ambient light
	re.GLSL().AnimalClothingRingworldFogShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

	// Suns
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		re.GLSL().AnimalClothingRingworldFogShader().SetSunShadowAngle(sunIndex, shadowPanelSunAngles[sunIndex]);

	// Render the model.
	if(!blended)
	{
		/*
		re.glDisable(GL_CULL_FACE);

		m_AnimalClothingVAO.SelectGL();
		m_AnimalClothingVAO.DrawGL();
		m_AnimalClothingVAO.DeselectGL();

		re.glEnable(GL_CULL_FACE);
		*/
	}
	else
	{
		//re.glDisable(GL_CULL_FACE);
		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_AnimalHelmetBlendVAO.SelectGL();
		m_AnimalHelmetBlendVAO.DrawGL();
		m_AnimalHelmetBlendVAO.DeselectGL();

		re.glDisable(GL_BLEND);
		//re.glEnable(GL_CULL_FACE);
	}

	re.GLSL().AnimalClothingRingworldFogShader().Deselect();
}

void AuAnimalia::RenderHeadClothing(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
		const AuPoint& ambientUp)
{
	if(blended)
	{
		if(!m_HeadClothingBlendVAO.VAO())
			return;
	}
	else
	{
		if(!m_HeadClothingVAO.VAO())
			return;
	}

	QString texFileDecal(":data/ClothDecal.png");
	GLuint clothDecals = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "ClothDecal", texFileDecal, GL_LINEAR, GL_LINEAR, true, 1);

	// Configure shader.
	re.GLSL().AnimalClothingShader().Select(projection, re);
	re.GLSL().AnimalClothingShader().SetModelView(model);
	re.GLSL().AnimalClothingShader().SetClothingDecalTexture(clothDecals);
	int idx = 0;
	for(AuClothingTextureArray::const_iterator cti = m_AnimalClothingTex.cbegin(); cti != m_AnimalClothingTex.cend(); cti++, idx++)
		re.GLSL().AnimalClothingShader().SetClothingTexture(idx, (*cti).Color(), (*cti).Texture());
	re.GLSL().AnimalClothingShader().SetGlowAlpha(0, 0, 0, 1);

	// Bones
	quint8 boneIndex = 0;
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
		{
			re.GLSL().AnimalClothingShader().SetBone(boneIndex, (*bi).Matrix());
			boneIndex++;
		}
	}

	// Ambient light
	re.GLSL().AnimalClothingShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

	// Render the model.
	if(!blended)
	{
		re.glDisable(GL_CULL_FACE);

		m_HeadClothingVAO.SelectGL();
		m_HeadClothingVAO.DrawGL();
		m_HeadClothingVAO.DeselectGL();

		re.glEnable(GL_CULL_FACE);
	}
	else
	{
		//re.glDisable(GL_CULL_FACE);
		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_HeadClothingBlendVAO.SelectGL();
		m_HeadClothingBlendVAO.DrawGL();
		m_HeadClothingBlendVAO.DeselectGL();

		re.glDisable(GL_BLEND);
		//re.glEnable(GL_CULL_FACE);
	}

	re.GLSL().AnimalClothingShader().Deselect();
}

void AuAnimalia::RenderHeadClothingGlobeFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		bool blended, const AuPoint& ambientUp, const AuPoint& globeP)
{
	if(blended)
	{
		if(!m_HeadClothingBlendVAO.VAO())
			return;
	}
	else
	{
		if(!m_HeadClothingVAO.VAO())
			return;
	}

	QString texFileDecal(":data/ClothDecal.png");
	GLuint clothDecals = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "ClothDecal", texFileDecal, GL_LINEAR, GL_LINEAR, true, 1);

	// Configure shader.
	re.GLSL().AnimalClothingGlobeFogShader().Select(projection, re);
	re.GLSL().AnimalClothingGlobeFogShader().SetModelView(model);
	re.GLSL().AnimalClothingGlobeFogShader().SetClothingDecalTexture(clothDecals);
	int idx = 0;
	for(AuClothingTextureArray::const_iterator cti = m_AnimalClothingTex.cbegin(); cti != m_AnimalClothingTex.cend(); cti++, idx++)
		re.GLSL().AnimalClothingGlobeFogShader().SetClothingTexture(idx, (*cti).Color(), (*cti).Texture());
	re.GLSL().AnimalClothingGlobeFogShader().SetGlowAlpha(0, 0, 0, 1);

	// Bones
	quint8 boneIndex = 0;
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
		{
			re.GLSL().AnimalClothingGlobeFogShader().SetBone(boneIndex, (*bi).Matrix());
			boneIndex++;
		}
	}

	// Ambient light
	re.GLSL().AnimalClothingGlobeFogShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);
	re.GLSL().AnimalClothingGlobeFogShader().SetGlobeLocation(globeP.x, globeP.y, globeP.z);

	// Render the model.
	if(!blended)
	{
		re.glDisable(GL_CULL_FACE);

		m_HeadClothingVAO.SelectGL();
		m_HeadClothingVAO.DrawGL();
		m_HeadClothingVAO.DeselectGL();

		re.glEnable(GL_CULL_FACE);
	}
	else
	{
		//re.glDisable(GL_CULL_FACE);
		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_HeadClothingBlendVAO.SelectGL();
		m_HeadClothingBlendVAO.DrawGL();
		m_HeadClothingBlendVAO.DeselectGL();

		re.glDisable(GL_BLEND);
		//re.glEnable(GL_CULL_FACE);
	}

	re.GLSL().AnimalClothingGlobeFogShader().Deselect();
}

void AuAnimalia::RenderHeadClothingRingworldFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		bool blended, const AuPoint& ambientUp, float shadowPanelSunAngles[MAX_SUNS])
{
	if(blended)
	{
		if(!m_HeadClothingBlendVAO.VAO())
			return;
	}
	else
	{
		if(!m_HeadClothingVAO.VAO())
			return;
	}

	QString texFileDecal(":data/ClothDecal.png");
	GLuint clothDecals = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "ClothDecal", texFileDecal, GL_LINEAR, GL_LINEAR, true, 1);

	// Configure shader.
	re.GLSL().AnimalClothingRingworldFogShader().Select(projection, re);
	re.GLSL().AnimalClothingRingworldFogShader().SetModelView(model);
	re.GLSL().AnimalClothingRingworldFogShader().SetClothingDecalTexture(clothDecals);
	int idx = 0;
	for(AuClothingTextureArray::const_iterator cti = m_AnimalClothingTex.cbegin(); cti != m_AnimalClothingTex.cend(); cti++, idx++)
		re.GLSL().AnimalClothingRingworldFogShader().SetClothingTexture(idx, (*cti).Color(), (*cti).Texture());
	re.GLSL().AnimalClothingRingworldFogShader().SetGlowAlpha(0, 0, 0, 1);

	// Bones
	quint8 boneIndex = 0;
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
		{
			re.GLSL().AnimalClothingRingworldFogShader().SetBone(boneIndex, (*bi).Matrix());
			boneIndex++;
		}
	}

	// Ambient light
	re.GLSL().AnimalClothingRingworldFogShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

	// Suns
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		re.GLSL().AnimalClothingRingworldFogShader().SetSunShadowAngle(sunIndex, shadowPanelSunAngles[sunIndex]);

	// Render the model.
	if(!blended)
	{
		re.glDisable(GL_CULL_FACE);

		m_HeadClothingVAO.SelectGL();
		m_HeadClothingVAO.DrawGL();
		m_HeadClothingVAO.DeselectGL();

		re.glEnable(GL_CULL_FACE);
	}
	else
	{
		//re.glDisable(GL_CULL_FACE);
		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_HeadClothingBlendVAO.SelectGL();
		m_HeadClothingBlendVAO.DrawGL();
		m_HeadClothingBlendVAO.DeselectGL();

		re.glDisable(GL_BLEND);
		//re.glEnable(GL_CULL_FACE);
	}

	re.GLSL().AnimalClothingRingworldFogShader().Deselect();
}

void AuAnimalia::RenderHeadHelmetVisor(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
		const AuPoint& ambientUp)
{
	if(blended)
	{
		if(!m_HeadHelmetBlendVAO.VAO())
			return;
	}
	else
	{
		// Currently no non-blended visor parts.
		return;
	}

	QString texFileDecal(":data/ClothDecal.png");
	GLuint clothDecals = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "ClothDecal", texFileDecal, GL_LINEAR, GL_LINEAR, true, 1);

	// Configure shader.
	re.GLSL().AnimalClothingShader().Select(projection, re);
	re.GLSL().AnimalClothingShader().SetModelView(model);
	re.GLSL().AnimalClothingShader().SetClothingDecalTexture(clothDecals);
	int idx = 0;
	for(AuClothingTextureArray::const_iterator cti = m_AnimalClothingTex.cbegin(); cti != m_AnimalClothingTex.cend(); cti++, idx++)
		re.GLSL().AnimalClothingShader().SetClothingTexture(idx, (*cti).Color(), (*cti).Texture());
	re.GLSL().AnimalClothingShader().SetGlowAlpha(0, 0, 0, 1);

	// Bones
	quint8 boneIndex = 0;
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
		{
			re.GLSL().AnimalClothingShader().SetBone(boneIndex, (*bi).Matrix());
			boneIndex++;
		}
	}

	// Ambient light
	re.GLSL().AnimalClothingShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

	// Render the model.
	if(!blended)
	{
		/*
		re.glDisable(GL_CULL_FACE);

		m_HeadClothingVAO.SelectGL();
		m_HeadClothingVAO.DrawGL();
		m_HeadClothingVAO.DeselectGL();

		re.glEnable(GL_CULL_FACE);
		*/
	}
	else
	{
		//re.glDisable(GL_CULL_FACE);
		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_HeadHelmetBlendVAO.SelectGL();
		m_HeadHelmetBlendVAO.DrawGL();
		m_HeadHelmetBlendVAO.DeselectGL();

		re.glDisable(GL_BLEND);
		//re.glEnable(GL_CULL_FACE);
	}

	re.GLSL().AnimalClothingShader().Deselect();
}

void AuAnimalia::RenderHeadHelmetVisorGlobeFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		bool blended, const AuPoint& ambientUp, const AuPoint& globeP)
{
	if(blended)
	{
		if(!m_HeadHelmetBlendVAO.VAO())
			return;
	}
	else
	{
		// Currently no non-blended visor parts.
		return;
	}

	QString texFileDecal(":data/ClothDecal.png");
	GLuint clothDecals = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "ClothDecal", texFileDecal, GL_LINEAR, GL_LINEAR, true, 1);

	// Configure shader.
	re.GLSL().AnimalClothingGlobeFogShader().Select(projection, re);
	re.GLSL().AnimalClothingGlobeFogShader().SetModelView(model);
	re.GLSL().AnimalClothingGlobeFogShader().SetClothingDecalTexture(clothDecals);
	int idx = 0;
	for(AuClothingTextureArray::const_iterator cti = m_AnimalClothingTex.cbegin(); cti != m_AnimalClothingTex.cend(); cti++, idx++)
		re.GLSL().AnimalClothingGlobeFogShader().SetClothingTexture(idx, (*cti).Color(), (*cti).Texture());
	re.GLSL().AnimalClothingGlobeFogShader().SetGlowAlpha(0, 0, 0, 1);

	// Bones
	quint8 boneIndex = 0;
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
		{
			re.GLSL().AnimalClothingGlobeFogShader().SetBone(boneIndex, (*bi).Matrix());
			boneIndex++;
		}
	}

	// Ambient light
	re.GLSL().AnimalClothingGlobeFogShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);
	re.GLSL().AnimalClothingGlobeFogShader().SetGlobeLocation(globeP.x, globeP.y, globeP.z);

	// Render the model.
	if(!blended)
	{
		/*
		re.glDisable(GL_CULL_FACE);

		m_HeadClothingVAO.SelectGL();
		m_HeadClothingVAO.DrawGL();
		m_HeadClothingVAO.DeselectGL();

		re.glEnable(GL_CULL_FACE);
		*/
	}
	else
	{
		//re.glDisable(GL_CULL_FACE);
		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_HeadHelmetBlendVAO.SelectGL();
		m_HeadHelmetBlendVAO.DrawGL();
		m_HeadHelmetBlendVAO.DeselectGL();

		re.glDisable(GL_BLEND);
		//re.glEnable(GL_CULL_FACE);
	}

	re.GLSL().AnimalClothingGlobeFogShader().Deselect();
}

void AuAnimalia::RenderHeadHelmetVisorRingworldFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		bool blended, const AuPoint& ambientUp, float shadowPanelSunAngles[MAX_SUNS])
{
	if(blended)
	{
		if(!m_HeadHelmetBlendVAO.VAO())
			return;
	}
	else
	{
		// Currently no non-blended visor parts.
		return;
	}

	QString texFileDecal(":data/ClothDecal.png");
	GLuint clothDecals = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "ClothDecal", texFileDecal, GL_LINEAR, GL_LINEAR, true, 1);

	// Configure shader.
	re.GLSL().AnimalClothingRingworldFogShader().Select(projection, re);
	re.GLSL().AnimalClothingRingworldFogShader().SetModelView(model);
	re.GLSL().AnimalClothingRingworldFogShader().SetClothingDecalTexture(clothDecals);
	int idx = 0;
	for(AuClothingTextureArray::const_iterator cti = m_AnimalClothingTex.cbegin(); cti != m_AnimalClothingTex.cend(); cti++, idx++)
		re.GLSL().AnimalClothingRingworldFogShader().SetClothingTexture(idx, (*cti).Color(), (*cti).Texture());
	re.GLSL().AnimalClothingRingworldFogShader().SetGlowAlpha(0, 0, 0, 1);

	// Bones
	quint8 boneIndex = 0;
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
		{
			re.GLSL().AnimalClothingRingworldFogShader().SetBone(boneIndex, (*bi).Matrix());
			boneIndex++;
		}
	}

	// Ambient light
	re.GLSL().AnimalClothingRingworldFogShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

	// Suns
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		re.GLSL().AnimalClothingRingworldFogShader().SetSunShadowAngle(sunIndex, shadowPanelSunAngles[sunIndex]);

	// Render the model.
	if(!blended)
	{
		/*
		re.glDisable(GL_CULL_FACE);

		m_HeadClothingVAO.SelectGL();
		m_HeadClothingVAO.DrawGL();
		m_HeadClothingVAO.DeselectGL();

		re.glEnable(GL_CULL_FACE);
		*/
	}
	else
	{
		//re.glDisable(GL_CULL_FACE);
		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_HeadHelmetBlendVAO.SelectGL();
		m_HeadHelmetBlendVAO.DrawGL();
		m_HeadHelmetBlendVAO.DeselectGL();

		re.glDisable(GL_BLEND);
		//re.glEnable(GL_CULL_FACE);
	}

	re.GLSL().AnimalClothingRingworldFogShader().Deselect();
}

void AuAnimalia::RenderAnimalHair(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
		const AuPoint& ambientUp)
{
	if(!blended)
		return;
	if(!m_AnimalHairTopVAO.VAO() && !m_AnimalHairBottomVAO.VAO())
		return;

	GLuint hair = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Hair1", ":data/Hair1.png", GL_LINEAR, GL_LINEAR, true, 1);

	// Configure shader.
	re.GLSL().AnimalHairShader().Select(projection, re);
	re.GLSL().AnimalHairShader().SetModelView(model);
	re.GLSL().AnimalHairShader().SetHairTexture(hair);
	re.GLSL().AnimalHairShader().SetGlowAlpha(0, 0, 0, 1);

	// Bones
	quint8 boneIndex = 0;
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
		{
			re.GLSL().AnimalHairShader().SetBone(boneIndex, (*bi).Matrix());
			boneIndex++;
		}
	}

	// Ambient light
	re.GLSL().AnimalHairShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

	// Render the model.
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	re.glDisable(GL_CULL_FACE);					// Disable face culling.

	// Draw once with no depth mask so the hair does not mask itself.
	re.glDepthMask(GL_FALSE);
	if(m_AnimalHairBottomVAO.VAO())
	{
		m_AnimalHairBottomVAO.SelectGL();
		m_AnimalHairBottomVAO.DrawGL();
		m_AnimalHairBottomVAO.DeselectGL();
	}
	if(m_AnimalHairTopVAO.VAO())
	{
		m_AnimalHairTopVAO.SelectGL();
		m_AnimalHairTopVAO.DrawGL();
		m_AnimalHairTopVAO.DeselectGL();
	}
	re.glDepthMask(GL_TRUE);

	// Draw top again with depth mask enabled so hair doesn't fall behind distant objects.
	if(m_AnimalHairTopVAO.VAO())
	{
		m_AnimalHairTopVAO.SelectGL();
		m_AnimalHairTopVAO.DrawGL();
		m_AnimalHairTopVAO.DeselectGL();
	}

	re.glEnable(GL_CULL_FACE);					// Enable face culling.

	re.glDisable(GL_BLEND);

	re.GLSL().AnimalHairShader().Deselect();
}

void AuAnimalia::RenderAnimalHairGlobeFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
		const AuPoint& ambientUp, const AuPoint& globeP)
{
	if(!blended)
		return;
	if(!m_AnimalHairTopVAO.VAO() && !m_AnimalHairBottomVAO.VAO())
		return;

	GLuint hair = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Hair1", ":data/Hair1.png", GL_LINEAR, GL_LINEAR, true, 1);

	// Configure shader.
	re.GLSL().AnimalHairGlobeFogShader().Select(projection, re);
	re.GLSL().AnimalHairGlobeFogShader().SetModelView(model);
	re.GLSL().AnimalHairGlobeFogShader().SetHairTexture(hair);
	re.GLSL().AnimalHairGlobeFogShader().SetGlowAlpha(0, 0, 0, 1);

	// Bones
	quint8 boneIndex = 0;
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
		{
			re.GLSL().AnimalHairGlobeFogShader().SetBone(boneIndex, (*bi).Matrix());
			boneIndex++;
		}
	}

	// Ambient light
	re.GLSL().AnimalHairGlobeFogShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);
	re.GLSL().AnimalHairGlobeFogShader().SetGlobeLocation(globeP.x, globeP.y, globeP.z);

	// Render the model.
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	re.glDisable(GL_CULL_FACE);					// Disable face culling.

	// Draw once with no depth mask so the hair does not mask itself.
	re.glDepthMask(GL_FALSE);
	if(m_AnimalHairBottomVAO.VAO())
	{
		m_AnimalHairBottomVAO.SelectGL();
		m_AnimalHairBottomVAO.DrawGL();
		m_AnimalHairBottomVAO.DeselectGL();
	}
	if(m_AnimalHairTopVAO.VAO())
	{
		m_AnimalHairTopVAO.SelectGL();
		m_AnimalHairTopVAO.DrawGL();
		m_AnimalHairTopVAO.DeselectGL();
	}
	re.glDepthMask(GL_TRUE);

	// Draw top again with depth mask enabled so hair doesn't fall behind distant objects.
	if(m_AnimalHairTopVAO.VAO())
	{
		m_AnimalHairTopVAO.SelectGL();
		m_AnimalHairTopVAO.DrawGL();
		m_AnimalHairTopVAO.DeselectGL();
	}

	re.glEnable(GL_CULL_FACE);					// Enable face culling.

	re.glDisable(GL_BLEND);

	re.GLSL().AnimalHairGlobeFogShader().Deselect();
}

void AuAnimalia::RenderAnimalHairRingworldFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		bool blended, const AuPoint& ambientUp, float shadowPanelSunAngles[MAX_SUNS])
{
	if(!blended)
		return;
	if(!m_AnimalHairTopVAO.VAO() && !m_AnimalHairBottomVAO.VAO())
		return;

	GLuint hair = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Hair1", ":data/Hair1.png", GL_LINEAR, GL_LINEAR, true, 1);

	// Configure shader.
	re.GLSL().AnimalHairRingworldFogShader().Select(projection, re);
	re.GLSL().AnimalHairRingworldFogShader().SetModelView(model);
	re.GLSL().AnimalHairRingworldFogShader().SetHairTexture(hair);
	re.GLSL().AnimalHairRingworldFogShader().SetGlowAlpha(0, 0, 0, 1);

	// Bones
	quint8 boneIndex = 0;
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
		{
			re.GLSL().AnimalHairRingworldFogShader().SetBone(boneIndex, (*bi).Matrix());
			boneIndex++;
		}
	}

	// Ambient light
	re.GLSL().AnimalHairRingworldFogShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

	// Suns
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		re.GLSL().AnimalHairRingworldFogShader().SetSunShadowAngle(sunIndex, shadowPanelSunAngles[sunIndex]);

	// Render the model.
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	re.glDisable(GL_CULL_FACE);					// Disable face culling.

	// Draw once with no depth mask so the hair does not mask itself.
	re.glDepthMask(GL_FALSE);
	if(m_AnimalHairBottomVAO.VAO())
	{
		m_AnimalHairBottomVAO.SelectGL();
		m_AnimalHairBottomVAO.DrawGL();
		m_AnimalHairBottomVAO.DeselectGL();
	}
	if(m_AnimalHairTopVAO.VAO())
	{
		m_AnimalHairTopVAO.SelectGL();
		m_AnimalHairTopVAO.DrawGL();
		m_AnimalHairTopVAO.DeselectGL();
	}
	re.glDepthMask(GL_TRUE);

	// Draw top again with depth mask enabled so hair doesn't fall behind distant objects.
	if(m_AnimalHairTopVAO.VAO())
	{
		m_AnimalHairTopVAO.SelectGL();
		m_AnimalHairTopVAO.DrawGL();
		m_AnimalHairTopVAO.DeselectGL();
	}

	re.glEnable(GL_CULL_FACE);					// Enable face culling.

	re.glDisable(GL_BLEND);

	re.GLSL().AnimalHairRingworldFogShader().Deselect();
}

void AuAnimalia::RenderHeadHair(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
		const AuPoint& ambientUp)
{
	if(!blended)
		return;
	if(!m_HeadHairTopVAO.VAO() || !m_HeadHairBottomVAO.VAO())
		return;

	GLuint hair = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Hair1", ":data/Hair1.png", GL_LINEAR, GL_LINEAR, true, 1);

	// Configure shader.
	re.GLSL().AnimalHairShader().Select(projection, re);
	re.GLSL().AnimalHairShader().SetModelView(model);
	re.GLSL().AnimalHairShader().SetHairTexture(hair);
	re.GLSL().AnimalHairShader().SetGlowAlpha(0, 0, 0, 1);

	// Bones
	quint8 boneIndex = 0;
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
		{
			re.GLSL().AnimalHairShader().SetBone(boneIndex, (*bi).Matrix());
			boneIndex++;
		}
	}

	// Ambient light
	re.GLSL().AnimalHairShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

	// Render the model.
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	re.glDisable(GL_CULL_FACE);					// Disable face culling.

	// Draw once with no depth mask so the hair does not mask itself.
	re.glDepthMask(GL_FALSE);
	m_HeadHairBottomVAO.SelectGL();
	m_HeadHairBottomVAO.DrawGL();
	m_HeadHairBottomVAO.DeselectGL();
	m_HeadHairTopVAO.SelectGL();
	m_HeadHairTopVAO.DrawGL();
	m_HeadHairTopVAO.DeselectGL();
	re.glDepthMask(GL_TRUE);

	// Draw top again with depth mask enabled so hair doesn't fall behind distant objects.
	m_HeadHairTopVAO.SelectGL();
	m_HeadHairTopVAO.DrawGL();
	m_HeadHairTopVAO.DeselectGL();

	re.glEnable(GL_CULL_FACE);					// Enable face culling.

	re.glDisable(GL_BLEND);

	re.GLSL().AnimalHairShader().Deselect();
}

void AuAnimalia::RenderHeadHairGlobeFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, bool blended,
		const AuPoint& ambientUp, const AuPoint& globeP)
{
	if(!blended)
		return;
	if(!m_HeadHairTopVAO.VAO() || !m_HeadHairBottomVAO.VAO())
		return;

	GLuint hair = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Hair1", ":data/Hair1.png", GL_LINEAR, GL_LINEAR, true, 1);

	// Configure shader.
	re.GLSL().AnimalHairGlobeFogShader().Select(projection, re);
	re.GLSL().AnimalHairGlobeFogShader().SetModelView(model);
	re.GLSL().AnimalHairGlobeFogShader().SetHairTexture(hair);
	re.GLSL().AnimalHairGlobeFogShader().SetGlowAlpha(0, 0, 0, 1);

	// Bones
	quint8 boneIndex = 0;
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
		{
			re.GLSL().AnimalHairGlobeFogShader().SetBone(boneIndex, (*bi).Matrix());
			boneIndex++;
		}
	}

	// Ambient light
	re.GLSL().AnimalHairGlobeFogShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);
	re.GLSL().AnimalHairGlobeFogShader().SetGlobeLocation(globeP.x, globeP.y, globeP.z);

	// Render the model.
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	re.glDisable(GL_CULL_FACE);					// Disable face culling.

	// Draw once with no depth mask so the hair does not mask itself.
	re.glDepthMask(GL_FALSE);
	m_HeadHairBottomVAO.SelectGL();
	m_HeadHairBottomVAO.DrawGL();
	m_HeadHairBottomVAO.DeselectGL();
	m_HeadHairTopVAO.SelectGL();
	m_HeadHairTopVAO.DrawGL();
	m_HeadHairTopVAO.DeselectGL();
	re.glDepthMask(GL_TRUE);

	// Draw top again with depth mask enabled so hair doesn't fall behind distant objects.
	m_HeadHairTopVAO.SelectGL();
	m_HeadHairTopVAO.DrawGL();
	m_HeadHairTopVAO.DeselectGL();

	re.glEnable(GL_CULL_FACE);					// Enable face culling.

	re.glDisable(GL_BLEND);

	re.GLSL().AnimalHairGlobeFogShader().Deselect();
}

void AuAnimalia::RenderHeadHairRingworldFog(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		bool blended, const AuPoint& ambientUp, float shadowPanelSunAngles[MAX_SUNS])
{
	if(!blended)
		return;
	if(!m_HeadHairTopVAO.VAO() || !m_HeadHairBottomVAO.VAO())
		return;

	GLuint hair = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "Hair1", ":data/Hair1.png", GL_LINEAR, GL_LINEAR, true, 1);

	// Configure shader.
	re.GLSL().AnimalHairRingworldFogShader().Select(projection, re);
	re.GLSL().AnimalHairRingworldFogShader().SetModelView(model);
	re.GLSL().AnimalHairRingworldFogShader().SetHairTexture(hair);
	re.GLSL().AnimalHairRingworldFogShader().SetGlowAlpha(0, 0, 0, 1);

	// Bones
	quint8 boneIndex = 0;
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		for(QList<AuAnimalBone>::const_iterator bi = (*mpi)->Bones().cbegin(); bi != (*mpi)->Bones().cend(); bi++)
		{
			re.GLSL().AnimalHairRingworldFogShader().SetBone(boneIndex, (*bi).Matrix());
			boneIndex++;
		}
	}

	// Ambient light
	re.GLSL().AnimalHairRingworldFogShader().SetAmbientLightUp(ambientUp.x, ambientUp.y, ambientUp.z);

	// Suns
	for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
		re.GLSL().AnimalHairRingworldFogShader().SetSunShadowAngle(sunIndex, shadowPanelSunAngles[sunIndex]);

	// Render the model.
	re.glEnable(GL_BLEND);
	re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	re.glDisable(GL_CULL_FACE);					// Disable face culling.

	// Draw once with no depth mask so the hair does not mask itself.
	re.glDepthMask(GL_FALSE);
	m_HeadHairBottomVAO.SelectGL();
	m_HeadHairBottomVAO.DrawGL();
	m_HeadHairBottomVAO.DeselectGL();
	m_HeadHairTopVAO.SelectGL();
	m_HeadHairTopVAO.DrawGL();
	m_HeadHairTopVAO.DeselectGL();
	re.glDepthMask(GL_TRUE);

	// Draw top again with depth mask enabled so hair doesn't fall behind distant objects.
	m_HeadHairTopVAO.SelectGL();
	m_HeadHairTopVAO.DrawGL();
	m_HeadHairTopVAO.DeselectGL();

	re.glEnable(GL_CULL_FACE);					// Enable face culling.

	re.glDisable(GL_BLEND);

	re.GLSL().AnimalHairRingworldFogShader().Deselect();
}

bool AuAnimalia::HasArms() const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuAnimalArm* mp = dynamic_cast<const AuAnimalArm*>(*ti);
		if(mp)
			return true;
	}
	return false;
}

bool AuAnimalia::HasNeck() const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuAnimalTorso* mp = dynamic_cast<const AuAnimalTorso*>(*ti);
		if(mp)
			return mp->HasNeck();
	}
	return false;
}

bool AuAnimalia::HasWings() const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuAnimalWing* mp = dynamic_cast<const AuAnimalWing*>(*ti);
		if(mp)
			return true;
	}
	return false;
}

int AuAnimalia::LegCount() const
{
	int c = 0;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuAnimalLeg* mp = dynamic_cast<const AuAnimalLeg*>(*ti);
		if(mp)
			c++;
	}
	return c;
}

bool AuAnimalia::HasLegs() const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuAnimalLeg* mp = dynamic_cast<const AuAnimalLeg*>(*ti);
		if(mp)
			return true;
	}
	return false;
}

bool AuAnimalia::HasTail() const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuAnimalTorso* mp = dynamic_cast<const AuAnimalTorso*>(*ti);
		if(mp)
			return mp->HasTail();
	}
	return false;
}

void AuAnimalia::CreateClothingTex(AuGLEffects& re)
{
	m_AnimalClothingTex.clear();
	m_Clothing.CreateTextures(re, m_AnimalClothingTex);
}

void AuAnimalia::CreateMeshes(AuMeatMesh& skinMesh, AuClothingMesh& clothingMesh, quint32 unique) const
{
	AuHairStyle hairStyle;
	float headSpace = .25f/12;
	hairStyle.Init(m_Haircut, m_Shave, headSpace, m_Handedness == RightHanded);

	// Body and clothing
	// Model is created facing along the +Y axis, +Z up, +X right, ready for bending.
	skinMesh.clear();
	AuMeatMesh clothedMesh;
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		(*ti)->GenSkinMesh(m_DNA.Chain(m_Chain), hairStyle, m_Clothing, m_GrowthStage);
		(*ti)->GetSkinMesh(skinMesh);

		(*ti)->GenClothedMesh(m_DNA.Chain(m_Chain), m_GrowthStage);
		(*ti)->GetClothedMesh(clothedMesh);
	}

	// Construct the clothing mesh.
	clothingMesh.clear();
	m_Clothing.GetMesh(clothingMesh, m_DNA.Chain(m_Chain), clothedMesh, m_AnimalClothingTex, unique);
}

void AuAnimalia::CreateVAOs(AuOpenGLFunctions& re, const AuMeatMesh& skinMesh, const AuClothingMesh& clothingMesh)
{
//#if !defined(QT_NO_DEBUG)
	//qDebug() << "AuAnimalia::CreateVAOs" << Description() << "skinTris" << skinMesh.size()
		//<< "haircut" << AuHairStyle::Name(m_Haircut) << "shave" << AuHairStyle::ShaveName(m_Shave)
		//<< "hairTris" << skinMesh.Hair().size()
		//<< "clothingTris" << clothingMesh.size();
//#endif
	// Delete my old VAOs.
	if(m_AnimalVAO.VAO())
		m_AnimalVAO.DeleteGL();
	if(m_AnimalHairBottomVAO.VAO())
		m_AnimalHairBottomVAO.DeleteGL();
	if(m_AnimalHairTopVAO.VAO())
		m_AnimalHairTopVAO.DeleteGL();
	if(m_AnimalClothingVAO.VAO())
		m_AnimalClothingVAO.DeleteGL();
	if(m_AnimalClothingBlendVAO.VAO())
		m_AnimalClothingBlendVAO.DeleteGL();
	if(m_AnimalHelmetBlendVAO.VAO())
		m_AnimalHelmetBlendVAO.DeleteGL();
	if(m_HeadVAO.VAO())
		m_HeadVAO.DeleteGL();
	if(m_HeadHairBottomVAO.VAO())
		m_HeadHairBottomVAO.DeleteGL();
	if(m_HeadHairTopVAO.VAO())
		m_HeadHairTopVAO.DeleteGL();

	// Create new VAOs.
	if(!skinMesh.isEmpty())
	{
		m_AnimalVAO = skinMesh.CreateAnimalVAO(re);
		m_AnimalHairBottomVAO = skinMesh.CreateAnimalHairVAO(re, false);
		m_AnimalHairTopVAO = skinMesh.CreateAnimalHairVAO(re, true);

		m_HeadVAO = skinMesh.CreateHeadVAO(re);
		m_HeadHairBottomVAO = skinMesh.CreateHeadHairVAO(re, false);
		m_HeadHairTopVAO = skinMesh.CreateHeadHairVAO(re, true);
	}

	if(!clothingMesh.isEmpty())
	{
		m_AnimalClothingVAO = clothingMesh.CreateAnimalClothingVAO(re);
		//DHH so far the only blended clothing part is a helmet visor
		m_AnimalHelmetBlendVAO = clothingMesh.CreateAnimalClothingBlendVAO(re);

		m_HeadClothingVAO = clothingMesh.CreateHeadClothingVAO(re);
		//DHH so far the only blended clothing part is a helmet visor
		m_HeadHelmetBlendVAO = clothingMesh.CreateHeadClothingBlendVAO(re);
	}
}

void AuAnimalia::CreateAnimalVAO(AuOpenGLFunctions& re, quint32 unique)
{
	AuMeatMesh skinMesh;
	AuClothingMesh clothingMesh;
	CreateMeshes(skinMesh, clothingMesh, unique);
	CreateVAOs(re, skinMesh, clothingMesh);
}

void AuAnimalia::DeleteGL()
{
	if(m_AnimalVAO.VAO())
		m_AnimalVAO.DeleteGL();
	if(m_AnimalClothingVAO.VAO())
		m_AnimalClothingVAO.DeleteGL();
	if(m_AnimalClothingBlendVAO.VAO())
		m_AnimalClothingBlendVAO.DeleteGL();
	if(m_AnimalHelmetBlendVAO.VAO())
		m_AnimalHelmetBlendVAO.DeleteGL();
	if(m_AnimalHairBottomVAO.VAO())
		m_AnimalHairBottomVAO.DeleteGL();
	if(m_AnimalHairTopVAO.VAO())
		m_AnimalHairTopVAO.DeleteGL();
	if(m_HeadVAO.VAO())
		m_HeadVAO.DeleteGL();
	if(m_HeadHairBottomVAO.VAO())
		m_HeadHairBottomVAO.DeleteGL();
	if(m_HeadHairTopVAO.VAO())
		m_HeadHairTopVAO.DeleteGL();
}

const AuMatrix4x4& AuAnimalia::HeadBaseMatrix() const
{
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		const AuAnimalTorso* mp = dynamic_cast<const AuAnimalTorso*>(*mpi);
		if(mp)
			return mp->HeadBaseMatrix();
	}

	// No torso found?
	return AuMatrix4x4::IdentityMatrix();
}

const AuMatrix4x4& AuAnimalia::NeckBaseMatrix() const
{
	// Return torso connect point. We have sorted these so the vertical torso is first, just for this search now.
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		const AuAnimalTorso* mp = dynamic_cast<const AuAnimalTorso*>(*mpi);
		if(mp)
			return mp->NeckBaseMatrix();
	}

	// No torso found?
	return AuMatrix4x4::IdentityMatrix();
}

const AuMatrix4x4& AuAnimalia::TorsoBaseMatrix() const
{
	// Return torso connect point. We have sorted these so the vertical torso is first, just for this search now.
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		const AuAnimalTorso* mp = dynamic_cast<const AuAnimalTorso*>(*mpi);
		if(mp)
			return mp->BaseMatrix();
	}

	// No torso found?
	return AuMatrix4x4::IdentityMatrix();
}

float AuAnimalia::CGHeight() const
{
	AuPose pose(CurrentGeneralPose());
	switch(pose.Pose())
	{
	case AuPose::EyesOnly:
		return 0;

	case AuPose::Init:
		return TorsoThickness()/2;

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
		return 0;

	case AuPose::BerthBottomFaceDown:
	case AuPose::BerthBottomFaceUp:
		return TorsoThickness()/2;
		//return 1.5;	// Height of bottom bunk in old style ships.

	case AuPose::BerthBottomFaceLeft:
	case AuPose::BerthBottomFaceRight:
		return TorsoWidth()/2.f;
		//return 1.5;	// Height of bottom bunk in old style ships.

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
	case AuPose::Straddle:
	case AuPose::WritheFaceDown:
	case AuPose::WritheFaceUp:
	case AuPose::ReceptiveMate:
	case AuPose::MateOnBottom:
	case AuPose::Patient_Prone:
		return TorsoThickness()/2;

	case AuPose::MateOnTop:
		return TorsoThickness() * 1.25f;	// Positioned above the mate, ready for up/down pumping action.

	case AuPose::DeadFaceLeft:
	case AuPose::DeadFaceRight:
	case AuPose::WritheFaceLeft:
	case AuPose::WritheFaceRight:
		return TorsoWidth()/2.f;

	case AuPose::Crouch:
	case AuPose::CrouchCreep:
	case AuPose::ConsoleSurgeryUnit:
	case AuPose::Dig:
	case AuPose::Ladder:
	case AuPose::LadderClimb:
	case AuPose::Post_Stand:
	case AuPose::Spawn:
	case AuPose::Stand:
	case AuPose::StandRun:
	case AuPose::StandWalk:
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
	case AuPose::Jump:
		break;

	case AuPose::Swim:
	case AuPose::SwimFast:
		return 0;
	case AuPose::TreadWater:
		return -HeadLocation().z;
	}

	// Calculate height based on leg pose and torso thickness.
	float tallestHip = 0;
	const AuAnimalTorso* torso = 0;
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		const AuAnimalLeg* mp = dynamic_cast<const AuAnimalLeg*>(*mpi);
		if(mp)
		{
			float hipHt = mp->HipAltitudePosed(m_DNA.Chain(m_Chain));
			if(tallestHip < hipHt)
				tallestHip = hipHt;
		}
		if(!torso)
			torso = dynamic_cast<const AuAnimalTorso*>(*mpi);
	}
	if(tallestHip > 0)
		return tallestHip;

	// No legs found.
	if(torso)
		return torso->BottomThickness()/2;

	// No torso? wtf
	return 0;
}

AuPoint AuAnimalia::JumpVector() const
{
	return CurrentGeneralPose().JumpVector(JumpForce());
}

quint8 AuAnimalia::GrowthTime() const
{
	int mins = (int)ceilf(sqrtf(MaxHitPoints()) * 2);	// Minutes to grow to adulthood. Human = 50hp = 14 minutes.
	if(mins > 255)
		return 255;										// That's a big creature. 255 min = 16,256 hp
	return mins;
}

GENDER AuAnimalia::Gender() const
{
	return m_DNA.Chain(m_Chain).Gender();
}

bool AuAnimalia::IsLeftHanded() const
{
	return Handedness() == LeftHanded;
}

float AuAnimalia::HearingRange() const
{
	return m_DNA.Chain(m_Chain).HearingRange();
}

float AuAnimalia::SightRange() const
{
	return m_DNA.Chain(m_Chain).SightRange();
}

float AuAnimalia::SmellRange() const
{
	return m_DNA.Chain(m_Chain).SmellRange();
}

float AuAnimalia::FieldOfViewAngle() const
{
	return m_DNA.Chain(m_Chain).FieldOfViewAngle();
}

void AuAnimalia::PositionArms(float hipHt)
{
	//DHH animalia
}

void AuAnimalia::PositionHeads()
{
	//DHH animalia
}

void AuAnimalia::PositionLegs(float hipHt)
{
	//DHH animalia
}

void AuAnimalia::PositionTail(float hipHt)
{
	//DHH animalia
}

float AuAnimalia::PositionTorso(float hipHt)
{
	//DHH animalia
	return 0;
}

void AuAnimalia::PositionWings(float waistRot)
{
	//DHH animalia
}

bool AuAnimalia::LivesInAir() const
{
	return false;	//DHH animalia
}

bool AuAnimalia::LivesInWater() const
{
	return false;	//DHH animalia
}

bool AuAnimalia::LivesOnLand() const
{
	return true;	//DHH animalia
}

bool AuAnimalia::LivesUnderGround() const
{
	return false;	//DHH animalia
}

qint16 AuAnimalia::MaxHitPoints() const
{
	// This formula causes a human-sized creature to have exactly 50 hit points.
	float vol = Volume();
	// To determine the constant after changes to the body volume calc, trap here in the debugger and calculate c = 50 / sqrt(vol).
	unsigned short int hp = (unsigned short int)(sqrtf(vol) * 39.238738209f);
	//qDebug() << "AuAnimalia::MaxHitPoints vol" << vol << "hp" << hp;
	if(hp > 0x7fff)
	{
		qWarning() << "AuAnimalia::MaxHitPoints vol" << vol << "hp" << hp << "exceeds hit points maximum";
		hp = 0x7fff;
	}

	if(IsFlyer())
		hp = hp / 2;					// Fliers have 1/2 the hit points of non-flyers.
	//else if(IsLeaper())
		//hp = hp * 2 / 3;				// Leapers have 2/3 the hit points of non-leapers.

	// The body must have at least 1 hit point.
	if(hp <= 0)
		hp = 1;

	return hp;
}

quint8 AuAnimalia::Stamina() const
{
	return 50;	//DHH animalia
}

float AuAnimalia::BiteRange() const
{
	return 10;	//DHH animalia
}

float AuAnimalia::BurdenMax() const
{
	return 200;	//DHH animalia
}

bool AuAnimalia::LooksLikeVermin() const
{
	return false;	//DHH animalia
}

bool AuAnimalia::PoseAttackFlying(const AuId& id)
{
	return false;	//DHH animalia
}

bool AuAnimalia::PoseAttackHovering(const AuId& id)
{
	return false;	//DHH animalia
}

bool AuAnimalia::PoseAttackStanding(const AuId& id)
{
	return false;	//DHH animalia
}

bool AuAnimalia::PoseAttackUnderGround(const AuId& id)
{
	return PoseAttackStanding(id);
}

bool AuAnimalia::PoseAttackUnderWater(const AuId& id)
{
	return false;	//DHH animalia
}

bool AuAnimalia::PoseBurrow(const AuId& id, float poseFrequency)
{
	return false;	//DHH animalia
}

bool AuAnimalia::PoseEatOffGround(const AuId& id)
{
	return false;	//DHH animalia
}

bool AuAnimalia::IsFlyer() const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuAnimalWing* mp = dynamic_cast<const AuAnimalWing*>(*ti);
		if(mp && mp->CanFly())
			return true;
	}
	return false;
}

bool AuAnimalia::IsLeaper() const
{
	return false;	//DHH animalia
}

float AuAnimalia::MaxSpeedToJump() const
{
	const float mySpeed = 7;	//DHH animalia
	const float maxSpeed = 3.0f;
	return maxSpeed * (mySpeed + 1);
}

AuDNABase::ECOSTRATEGY AuAnimalia::EcoStrategy() const
{
	return AuDNABase::ECO_Hunter;	//DHH animalia
}

ECOROLE AuAnimalia::EcoRole() const
{
	return m_DNA.EcoRole();
}

float AuAnimalia::MovementSpeed() const
{
	const float mySpeed = 7;	//DHH animalia
	float speed = 0;
	switch(CurrentGeneralPose().Pose())
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
	case AuPose::TreadWater:
	case AuPose::WritheFaceDown:
	case AuPose::WritheFaceLeft:
	case AuPose::WritheFaceRight:
	case AuPose::WritheFaceUp:
	case AuPose::ReceptiveMate:
	case AuPose::MateOnBottom:
	case AuPose::MateOnTop:
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
	return speed * (mySpeed + 1);
}

void AuAnimalia::SetHitPoints(qint16 hp)
{
	// Prevent negative hit points below "dead" level.
	// 0...dead is the "dying" zone where the body loses one hit point per second until "dead".
	if(hp >= DeadHitPoints())
		m_HitPoints = hp;
	else
		m_HitPoints = DeadHitPoints();
}

void AuAnimalia::SetLeftHanded(bool isLefty)
{
	m_Handedness = isLefty ? LeftHanded : RightHanded;
}

AuPoint AuAnimalia::HeadLocation() const
{
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		const AuAnimalHead* mp = dynamic_cast<const AuAnimalHead*>(*mpi);
		if(mp)
			return mp->Bones()[0].PosedLoc();
	}

	// No head found?
	return AuPoint(0,0);
}

AuPoint AuAnimalia::CameraLocation() const
{
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		const AuAnimalHead* mp = dynamic_cast<const AuAnimalHead*>(*mpi);
		if(mp)
		{
			if(mp->Bones().size() > 4)
			{
				if(m_Handedness == LeftHanded || mp->Bones().size() < 6)
					return mp->Bones()[4].PosedLoc();
				else
					return mp->Bones()[5].PosedLoc();
			}
			else
				return mp->Bones()[0].PosedLoc();
		}
	}

	// No head found?
	return AuPoint(0,0);
}

AuPoint AuAnimalia::TargetLocation() const
{
	return HeadLocation();
}

float AuAnimalia::CycleSpeed() const
{
	return 2;	//DHH animalia
}

float AuAnimalia::JumpForce() const
{
	return DNAChain().JumpForce();
}

quint8 AuAnimalia::StaminaToJump() const
{
	return 1;	//DHH animalia
}

float AuAnimalia::TurnRate() const
{
	// Calculate turn rate based on Speed dna.
	const float mySpeed = 7;	//DHH animalia
	float turn = (mySpeed+1)/6.f * PIf;
	return turn;
}

float AuAnimalia::Volume() const
{
	return DNAChain().Volume();
}

void AuAnimalia::ResetHitPoints()
{
	m_HitPoints = MaxHitPoints();
}

float AuAnimalia::TorsoThickness() const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuAnimalTorso* torso = dynamic_cast<const AuAnimalTorso*>(*ti);
		if(torso)
			return torso->BottomThickness();
	}

	// No torso? wtf
	return 0;
}

float AuAnimalia::TorsoWidth() const
{
	for(const_iterator ti = cbegin(); ti != cend(); ti++)
	{
		const AuAnimalTorso* torso = dynamic_cast<const AuAnimalTorso*>(*ti);
		if(torso)
			return torso->Width();
	}

	// No torso? wtf
	return 0;
}

void AuAnimalia::Init()
{
	//DHH animalia
}

float AuAnimalia::HeadThickness() const
{
	return .5f;	//DHH animalia
}

float AuAnimalia::Density() const
{
	// Creatures that live in the water are essentially the same density as water, so they can swim up and down readily.
	// Minor variations in density to control bouyancy are outside our current scope.
	if(LivesInWater())
		return (float)DensityOfWater();

	// All other creatures like to float a bit.
	// Wikipedia says the body has a relative density of .98 to water.
	return (float)DensityOfWater() * .98f;
}

void AuAnimalia::SightExtents(AuPoint& min, AuPoint& max) const
{
	if(empty())
	{
		min = AuPoint(0,0);
		max = AuPoint(0,0);
		return;
	}

	const_iterator mpi = cbegin();
	(*mpi)->SightExtents(min, max);
	for(mpi++; mpi != cend(); mpi++)
	{
		AuPoint smin, smax;
		(*mpi)->SightExtents(smin, smax);
		min = min.Min(smin);
		max = max.Max(smax);
	}
}

bool AuAnimalia::CanHover() const
{
	return false;	//DHH animalia
}

bool AuAnimalia::IsExoskeletal() const
{
	return false;	//DHH animalia
}

QString AuAnimalia::Description() const
{
	return "Animalia";	//DHH animalia
}

void AuAnimalia::Refresh(const AuAnimalia& c)
{
	//m_BendDominantArm = c.m_BendDominantArm;
	m_Chain = c.m_Chain;
	//m_DNA = c.m_DNA;
	//m_GrowthStage = c.m_GrowthStage;
	//m_Haircut = c.m_Haircut;
	m_Handedness = c.m_Handedness;
	//m_HeadTilt = c.m_HeadTilt;
	//m_HeadTurn = c.m_HeadTurn;
	m_HitPoints = c.m_HitPoints;
	m_LastSpecUse = c.m_LastSpecUse;
	//for(int ps=0; ps<AuAnimalPart::PS_Count; ps++)
		//m_Pose[ps] = c.m_Pose[ps];
	//m_Shave = c.m_Shave;

	//for(const_iterator mpi = c.cbegin(); mpi != c.cend(); mpi++)
		//append((*mpi)->Clone());

	//SetDNA(c.m_DNA);
}

/*
AuPoint AuAnimalia::CurrentLeftHandLoc() const
{
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		const AuAnimalArm* mp = dynamic_cast<const AuAnimalArm*>(*mpi);
		if(mp && mp->Which().Index() == 0)
		{
			AuPoint knuckle(mp->Bones()[3].PosedLoc());				// Location of knuckle.
			AuPoint palmVec(mp->Bones()[2].PosedLoc() - knuckle);	// Vector from knuckle toward wrist.
			palmVec /= 3;											// Offset will be toward the palm.
			return (knuckle + mp->Bones()[4].PosedLoc())/2 + palmVec;
		}
	}
	return AuPoint(0,0);
}

AuPoint AuAnimalia::CurrentRightHandLoc() const
{
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		const AuAnimalArm* mp = dynamic_cast<const AuAnimalArm*>(*mpi);
		if(mp && mp->Which().Index() == 1)
		{
			AuPoint knuckle(mp->Bones()[3].PosedLoc());				// Location of knuckle.
			AuPoint palmVec(mp->Bones()[2].PosedLoc() - knuckle);	// Vector from knuckle toward wrist.
			palmVec /= 3;											// Offset will be toward the palm.
			return (knuckle + mp->Bones()[4].PosedLoc())/2 + palmVec;
		}
	}
	return AuPoint(0,0);
}
*/

AuMatrix4x4 AuAnimalia::LeftHandMatrix() const
{
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		const AuAnimalArm* mp = dynamic_cast<const AuAnimalArm*>(*mpi);
		if(mp && mp->Which().Index() == 0)
		{
			AuPoint knuckle(mp->Bones()[3].PosedLoc());				// Location of knuckle.
			AuPoint palmVec(mp->Bones()[2].PosedLoc() - knuckle);	// Vector from knuckle toward wrist.
			palmVec /= 3;											// Offset will be toward the palm.
			AuPoint origin((knuckle + mp->Bones()[4].PosedLoc())/2 + palmVec);

			// Unposed hand is created pointing downward with palm facing right.
			AuPoint xvec = mp->Bones()[2].Matrix().ProjectVector(AuPoint(0,0,-1));
			xvec.Unit();

			AuPoint zvec = mp->Bones()[2].Matrix().ProjectVector(AuPoint(0,1,0));	// direction of thumb
			zvec.Unit();

			AuPoint yvec = mp->Bones()[2].Matrix().ProjectVector(AuPoint(-1,0,0));
			yvec.Unit();

			return AuMatrix4x4(origin, xvec, yvec);
		}
	}
	return AuMatrix4x4();	// Identity
}

AuMatrix4x4 AuAnimalia::RightHandMatrix() const
{
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
	{
		const AuAnimalArm* mp = dynamic_cast<const AuAnimalArm*>(*mpi);
		if(mp && mp->Which().Index() == 1)
		{
			AuPoint knuckle(mp->Bones()[3].PosedLoc());				// Location of knuckle.
			AuPoint palmVec(mp->Bones()[2].PosedLoc() - knuckle);	// Vector from knuckle toward wrist.
			palmVec /= 3;											// Offset will be toward the palm.
			AuPoint origin((knuckle + mp->Bones()[4].PosedLoc())/2 + palmVec);

			// Unposed hand is created pointing downward with palm facing left.
			AuPoint xvec = mp->Bones()[2].Matrix().ProjectVector(AuPoint(0,0,-1));
			xvec.Unit();

			AuPoint zvec = mp->Bones()[2].Matrix().ProjectVector(AuPoint(0,1,0));	// direction of thumb
			zvec.Unit();

			AuPoint yvec = mp->Bones()[2].Matrix().ProjectVector(AuPoint(-1,0,0));
			yvec.Unit();

			return AuMatrix4x4(origin, xvec, yvec);
		}
	}
	return AuMatrix4x4();	// Identity
}

void AuAnimalia::SetDNA(const AuDNA& dna, AuDNA::CHROMATID chromatid)
{
	if(m_Chain == chromatid && m_DNA == dna)
		return;

	// Remove my current body parts.
	for(iterator mpi=begin(); mpi != end(); mpi++)
		delete *mpi;
	clear();

	// Change my DNA and create new body parts.
	m_DNA = dna;
	m_Chain = chromatid;
	m_DNA.MakeParts(*this, m_Chain);
}

bool AuAnimalia::HasVAOs() const
{
	return m_AnimalVAO.VAO();
}

COMMODITYID AuAnimalia::WornArmor() const
{
	return m_Clothing.ArmorCID();
}

COMMODITYID AuAnimalia::WornClothing() const
{
	return m_Clothing.ClothingCID();
}

COMMODITYID AuAnimalia::WornHat() const
{
	return m_Clothing.HatCID();
}

int AuAnimalia::VisualCompare(const AuAnimalia& c) const
{
	// These all have to add up to 100 in a perfect match.
	int score = 0;
	if(m_GrowthStage == c.m_GrowthStage)
		score += 50;
	if(m_Clothing.ArmorCID() == c.m_Clothing.ArmorCID())
		score += 20;
	if(m_Clothing.ClothingCID() == c.m_Clothing.ClothingCID())
		score += 10;
	if(m_Clothing.HatCID() == c.m_Clothing.HatCID())
		score += 10;
	if(m_Haircut == c.m_Haircut)
		score += 5;
	if(m_Shave == c.m_Shave)
		score += 5;
	return score;
}

bool AuAnimalia::MouthPoseExpired(const AuElapsedTimer& poseStarted, unsigned int durationMs) const
{
	unsigned int poseMs;
	if(durationMs)
		poseMs = durationMs;								// A specific pose duration was specified.
	else
	{
		poseMs = m_Pose[AuAnimalPart::PS_Mouth].Period();	// No specific duration so use the cycle time of the pose.
		if(!poseMs)
			return false;	// Some poses do not expire.
	}
	qint64 ms = poseStarted.elapsed();
	return ms > poseMs;
}

bool AuAnimalia::SightTargetDetails(const AuPoint& eyeLoc, AuPoint& sightedLoc, unsigned int& sightedPart) const
{
	bool anyHit = false;
	for(const_iterator mpi = cbegin(); mpi != cend(); mpi++)
		if((*mpi)->SightTargetDetails(eyeLoc, sightedLoc, sightedPart))
			anyHit = true;
	return anyHit;
}

bool AuAnimalia::HasMaleGenitals() const
{
	return m_DNA.Chain(m_Chain).HasMaleGenitals();
}

// Dying pose returns the corresponding dead pose within 3 seconds of actual death. So the body looks really dead briefly.
AuPose AuAnimalia::ApparentSectionPose(AuAnimalPart::SECTION ps) const
{
	switch(m_Pose[ps].Pose())
	{
	case AuPose::WritheFaceDown:
		if(m_HitPoints < DeadHitPoints() + 10)
			return AuPose::DeadFaceDown;		// Stop writhing.
		break;
	case AuPose::WritheFaceLeft:
		if(m_HitPoints < DeadHitPoints() + 10)
			return AuPose::DeadFaceLeft;		// Stop writhing.
		break;
	case AuPose::WritheFaceRight:
		if(m_HitPoints < DeadHitPoints() + 10)
			return AuPose::DeadFaceRight;		// Stop writhing.
		break;
	case AuPose::WritheFaceUp:
		if(m_HitPoints < DeadHitPoints() + 10)
			return AuPose::DeadFaceUp;			// Stop writhing.
		break;
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
		if(m_Pose[AuAnimalPart::PS_General].IsDying() || m_Pose[AuAnimalPart::PS_General].IsDead())
			return AuPose::MouthSmile;	// Faces relax and appear to smile when dead.
		break;
	default:
		// Not a dying pose so return the pose.
		break;
	}
	return m_Pose[ps];
}

bool AuAnimalia::BestWeaponAtRange(float targetRange, AuDNABase::WEAPONLOC& loc, AuCombatWeapon& weapon,
	bool okToUseSpecialWeapon) const
{
	//DHH animalia
	return false;

	/*
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
	*/

}

void AuAnimalia::WeaponAtPos(AuDNABase::WEAPONLOC loc, bool useSpecialWeapon, AuCombatWeapon& weapon) const
{
	if(useSpecialWeapon)
	{
		//DHH animalia Special weapon
		weapon = AuCombatWeapon(WM_Not);
		return;
	}

	switch(loc)
	{
	case AuDNABase::WL_None:
		break;
	case AuDNABase::WL_Feet:
		{
			// Find my biggest leg/foot.
			const AuAnimalLeg* leg = 0;
			float legVol = 0;
			for(const_iterator ti = cbegin(); ti != cend(); ti++)
			{
				const AuAnimalLeg* mp = dynamic_cast<const AuAnimalLeg*>(*ti);
				if(!mp)
					continue;
				float v = mp->Volume();
				if(!leg || v > legVol)
				{
					leg = mp;
					legVol = v;
				}
			}
			if(leg && leg->Weapon(DNAChain(), loc, weapon))
				return;
		}
		break;
	case AuDNABase::WL_Hands:
		{
			// Find my biggest arm/hand.
			const AuAnimalArm* arm = 0;
			float armVol = 0;
			for(const_iterator ti = cbegin(); ti != cend(); ti++)
			{
				const AuAnimalArm* mp = dynamic_cast<const AuAnimalArm*>(*ti);
				if(!mp)
					continue;
				float v = mp->Volume();
				if(!arm || v > armVol)
				{
					arm = mp;
					armVol = v;
				}
			}
			if(arm && arm->Weapon(DNAChain(), loc, weapon))
				return;
		}
		break;
	case AuDNABase::WL_Eyes:
	case AuDNABase::WL_Head:
		{
			// Find my head.
			const AuAnimalHead* head = 0;
			for(const_iterator ti = cbegin(); ti != cend(); ti++)
			{
				const AuAnimalHead* mp = dynamic_cast<const AuAnimalHead*>(*ti);
				if(!mp)
					continue;
				head = mp;
				break;
			}
			if(head && head->Weapon(DNAChain(), loc, weapon))
				return;
		}
		break;
	case AuDNABase::WL_Anus:
	case AuDNABase::WL_Tail:
	case AuDNABase::WL_Torso:
		{
			// Find my torso.
			const AuAnimalTorso* torso = 0;
			for(const_iterator ti = cbegin(); ti != cend(); ti++)
			{
				const AuAnimalTorso* mp = dynamic_cast<const AuAnimalTorso*>(*ti);
				if(!mp)
					continue;
				torso = mp;
				break;
			}
			if(torso && torso->Weapon(DNAChain(), loc, weapon))
				return;
		}
		break;
	};
	weapon = AuCombatWeapon(WM_Not);
}

int AuAnimalia::AbilityPointsUsed() const
{
	return 0;	//DHH animalia
}

bool AuAnimalia::IsAbilityPointsValidForPerson() const
{
	return AbilityPointsUsed() <= 60;
}

bool AuAnimalia::IsHitPointsValidForPerson() const
{
	unsigned short int maxHits = MaxHitPoints();
	if(IsFlyer())
		return maxHits >= 12 && maxHits <= 40;
	else
		return maxHits >= 25 && maxHits <= 80;
}

bool AuAnimalia::IsCameraHeightValidForPerson() const
{
	float cameraHeight = CGHeight() + CameraLocation().z;
	return cameraHeight >= 0.5f && cameraHeight <= 7.5f;
}

bool AuAnimalia::IsCgHeightValidForPerson() const
{
	float cgHeight = CGHeight();
	return cgHeight >= 0.1245f && cgHeight <= 7.0f;
}

bool AuAnimalia::IsHeadHeightValidForPerson() const
{
	float headHeight = CGHeight() + HeadLocation().z;
	return headHeight >= 0.5f && headHeight <= 7.0f;
}

int AuAnimalia::AvatarCheck()
{
	// Preserve my current body poses.
	AuPose pose[AuAnimalPart::PS_Count];
	for(int p=0; p<AuAnimalPart::PS_Count; p++)
		pose[p] = m_Pose[p];

	// Check ability points and hit points.
	int dnaCheck = 0;
	if(!IsAbilityPointsValidForPerson())
		dnaCheck |= 0x0001;
	if(!IsHitPointsValidForPerson())
		dnaCheck |= 0x0002;

	// Check body geometry in my standing pose.
	Pose(AuId(1), AuPose::Stand, true, 1);
	if(!IsCameraHeightValidForPerson())
		dnaCheck |= 0x0004;
	if(!IsCgHeightValidForPerson())
		dnaCheck |= 0x0008;
	if(!IsHeadHeightValidForPerson())
		dnaCheck |= 0x0010;

	// Check body geometry when sitting at a console.
	Pose(AuId(1), AuPose::ConsoleCommand, true, 1);
	if(!IsCameraHeightValidForPerson())
		dnaCheck |= 0x0020;
	if(!IsHeadHeightValidForPerson())
		dnaCheck |= 0x0040;

	// Restore my body poses.
	for(int p=0; p<AuAnimalPart::PS_Count; p++)
		m_Pose[p] = pose[p];
	AnimatePose(AuId(1), AuElapsedTimer::Now());

	// Check that they can breathe standard or tainted atmosphere.
	ATMTYPE atmoType = m_DNA.Chain(m_Chain).BreathesAir();
	if(atmoType != ATMTYPE_Standard && atmoType != ATMTYPE_Tainted)
		dnaCheck |= 0x0080;

	return dnaCheck;
}

