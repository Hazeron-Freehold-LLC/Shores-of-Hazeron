//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auatomrenderinfo.cpp,v 1.8 2022/02/19 17:39:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auatomrenderinfo.h"
//
#include "au3dsglsl.h"
#include "auglmodel.h"
#include "auglobal.h"
#include "ausound.h"

AuAtomRenderInfo::AuAtomRenderInfo(QOpenGLWidget* pglw, AuSound* pal, AuGLModelMaker& pmm, AuGLModelCache& mc,
		const QFont& atomFont) : al(pal), font(atomFont), glw(pglw), mm(pmm), m_ModelCache(mc)
{
	m_AnchorPart = 0;
	fade = 0;
	m_DesignerObjectIsolate = false;
	m_DesignerObjectMode = false;
	m_DesignerShowBacks = true;
	m_DesignerShowEdges = true;
	m_DesignerShowFaces = true;
	m_DesignerShowGround = false;
	m_DesignerShowSkyBox = true;
	m_Drag = false;
	m_DragCopy = false;
	m_DragPart = 0;
	m_ShowHUD = true;
	m_ShowPaths = false;
	m_TopViewInfo = VI_None;
	m_ViewAlt = 300;
	m_ViewAngle1st = 0;
	m_ViewAngle3rd = 20.f * DEG2RADf;
	m_ViewDist1st = 0;
	m_ViewDist3rd = 10.f;
	m_ViewMode = VM_FirstPerson;
	re = 0;
	textBrightness = 0;
	viewDeck = 0;
}

AuAtomRenderInfo::~AuAtomRenderInfo()
{
	if(re)
		delete re;
}

void AuAtomRenderInfo::InitializeGL()
{
	if(!re)
		re = new AuGLEffects;

	/*DHH
	if(audioToo)
	{
		re->InitializeAudio();

		// Configure custom sound overrides and gain using current global settings.
		for(int s=SOUND_None+1; s < SOUND_Count; s++)
		{
			SOUND_EFFECT sound = (SOUND_EFFECT)s;
			AuGLSoundOverride sndOver;
			if(g_Au.SoundOverride(sound, sndOver))
				re->SetSoundOverride(sound, sndOver);

			float gain = g_Au.SoundGain(sound);
			re->SetSoundGain(sound, gain);
		}
	}
	*/

	re->InitializeGL(g_Au.MaxLights());
}

void AuAtomRenderInfo::DragStart(const AuPoint& dragRef, const AuMatrix4x4& designPlane)
{
	m_Drag = true;

	// Drag plane is parallel to the design plane, but it is centered at the drag reference. That way the drag reference moves
	// logically and doesn't jump around.
	m_DragPlane.LoadIdentity();
	m_DragPlane.Translate(dragRef);
	m_DragPlane.Rotate(designPlane.GetRotation());
}

void AuAtomRenderInfo::DragEnd()
{
	m_Drag = false;

	m_DragCopy = false;
	m_DragT.LoadIdentity();
}

QString AuAtomRenderInfo::TopViewInfoName() const
{
	switch(TopViewInfo())
	{
	case VI_None:
		return QObject::tr("No Overlay");
	case VI_RocksAll:
		return QObject::tr("Minerals");
	case VI_RocksSome:
		return QObject::tr("Uncommon Minerals\nStone, Ice and Eludium Omitted");
	case VI_Plants:
		return QObject::tr("Vegetation\nLarge = Tree, Small = Shrub");
	case VI_Buildings:
		return QObject::tr("Buildings");
	case VI_Roads:
		return QObject::tr("Roads and Landscaping");
	case VI_Flags:
		return QObject::tr("Flags");
	case VI_CityNames:
		return QObject::tr("City Names");
	}
	return QObject::tr("Overlay %1?").arg(TopViewInfo());
}

// The texture returned is flipped in Y, for sprites.
GLuint AuAtomRenderInfo::CommodityTextureF(COMMODITYID cid) const
{
	QHash<COMMODITYID, GLuint>::iterator ti = m_CommodityTextures.find(cid);
	if(ti == m_CommodityTextures.end())
	{
		QImage bmcid(g_Au.CommodityBitmapName((COMMODITYID)cid));
		if(bmcid.isNull())
		{
			qWarning() << "error: AuAtomRenderInfo::CommodityTexture failed to load commodity bitmap" << cid
				<< g_Au.CommodityBitmapName(cid);
			return 0;
		}
		QImage bmt(bmcid.width()+2, bmcid.height()+2, QImage::Format_ARGB32);
		if(bmt.isNull())
			return 0;
		bmt.fill(Qt::transparent);

		QPainter p(&bmt);
		p.translate(0,bmcid.height()+2);		// The bitmap must be drawn up-side-down to appear right-side-up on sprites.
		p.scale(1,-1);
		p.drawImage(1,1, bmcid);
		p.end();

		QString sid = QString("CID%1").arg(cid);
		GLuint tid = re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, sid, bmt);
		if(!tid)
			return 0;
		ti = m_CommodityTextures.insert(cid, tid);
	}
	return *ti;
}

// The texture returned is NOT flipped in Y.
GLuint AuAtomRenderInfo::CommodityTextureN(COMMODITYID cid) const
{
	QHash<COMMODITYID, GLuint>::iterator ti = m_CommodityTexturesN.find(cid);
	if(ti == m_CommodityTexturesN.end())
	{
		QImage bmcid(g_Au.CommodityBitmapName((COMMODITYID)cid));
		if(bmcid.isNull())
		{
			qWarning() << "error: AuAtomRenderInfo::CommodityTextureN failed to load commodity bitmap" << cid
				<< g_Au.CommodityBitmapName(cid);
			return 0;
		}
		QImage bmt(bmcid.width()+2, bmcid.height()+2, QImage::Format_ARGB32);
		if(bmt.isNull())
			return 0;
		bmt.fill(Qt::transparent);

		QPainter p(&bmt);
		p.drawImage(1,1, bmcid);
		p.end();

		QString sid = QString("CIDN%1").arg(cid);
		GLuint tid = re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, sid, bmt);
		if(!tid)
			return 0;
		ti = m_CommodityTexturesN.insert(cid, tid);
	}
	return *ti;
}

Au3dsGLSLModelInstance* AuAtomRenderInfo::CreateBodyModelGLSL(const QString& fileName, AuPoint& min, AuPoint& max, bool bLightsOn,
		const Au3dsGLSLModelDefinition& modelDef)
{
	return m_ModelCache.CreateBodyModel(*re, fileName, min, max, bLightsOn, modelDef.m_rScale, modelDef.m_rOverallScale,
		modelDef.m_iPrevBone,
		modelDef.m_iThisBone, modelDef.m_iNextBone, modelDef.m_pOffset, modelDef.m_ruLength, modelDef.m_ruThickness,
		modelDef.m_rlLength, modelDef.m_rlThickness, modelDef.m_rhwLength, modelDef.m_fTColor, modelDef.m_fTShininess,
		modelDef.m_fBColor, modelDef.m_fBShininess, modelDef.m_fGlow, modelDef.m_fGlowdim, modelDef.m_bCreate, modelDef.m_bSkip,
		modelDef.m_bArrangement, modelDef.m_idestBlendFunc);
}

AuGLModelInstance* AuAtomRenderInfo::CreateModelGLSL(const QString& fileName, bool bLightsOn, double scale)
{
	return m_ModelCache.CreateModel(fileName, *re, bLightsOn, scale);
}

void AuAtomRenderInfo::CreatePatchedModelGLSL(Au3dsGLSLModelInstance* pm)
{
	m_ModelCache.CreatePatchedModel(*re, pm);
}

void AuAtomRenderInfo::DeleteModelGLSL(AuGLModelInstance* pm)
{
	m_ModelCache.DeleteModel(pm);
}

AuGLModelInstance* AuAtomRenderInfo::LoadModelThreaded(const QString& fileName, bool bLightsOn, double scale)
{
	return m_ModelCache.LoadModelThreaded(fileName, bLightsOn, scale);
}

void AuAtomRenderInfo::PlaySound(unsigned int& audioSource, int& curSound, int sound, float posX, float posY, float posZ,
		float gain, float pitch, bool loopSound)
{
	if(!al)
		return;
	SOUND_EFFECT cur = (SOUND_EFFECT)curSound;
	al->Play(audioSource, cur, (SOUND_EFFECT)sound, posX, posY, posZ, gain, pitch, loopSound);
	curSound = cur;
}

// 'curSound' and 'sound' are SOUND_EFFECT values
void AuAtomRenderInfo::PlaySound(unsigned int& audioSource, int& curSound, int sound, float posX, float posY, float posZ,
		float gain, float pitch, bool loopSound, float referenceDistance, float rolloffFactor, float secOffset)
{
	if(!al)
		return;
	SOUND_EFFECT cur = (SOUND_EFFECT)curSound;
	al->Play(audioSource, cur, (SOUND_EFFECT)sound, posX, posY, posZ, gain, pitch, loopSound, referenceDistance, rolloffFactor,
			secOffset);
	curSound = cur;
}

void AuAtomRenderInfo::PlaySound(unsigned int& audioSource, unsigned int audioBuffer, float posX, float posY, float posZ,
		float gain, float pitch, float referenceDistance, float rolloffFactor)
{
	if(!al)
		return;
	al->Play(audioSource, audioBuffer, posX, posY, posZ, gain, pitch, referenceDistance, rolloffFactor);
}

void AuAtomRenderInfo::StopSound(unsigned int& audioSource)
{
	if(!al)
		return;
	al->StopSound(audioSource);
}

void AuAtomRenderInfo::DeleteSound(unsigned int audioBuffer)
{
	if(!al)
		return;
	al->DeleteSound(audioBuffer);
}

