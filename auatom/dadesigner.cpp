//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dadesigner.cpp,v 1.264 2022/05/17 16:03:17 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dadesigner.h"
//
#include "auatomrenderinfo.h"
#include "aubddesign.h"
#include "aubox.h"
#include "aufilever.h"
#include "augleffects.h"
#include "auglobal.h"
#include "auitdesign.h"
#include "auscdesign.h"
#include "auscparts.h"
#include "ausynccontroller.h"
#include "daanimal.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dadesignplane.h"
#include "daitem.h"
#include "danpc.h"
#include "daperson.h"
#include "dasc.h"
#include "dasolarsystem.h"
#include "davehicle.h"
#include "daworldglobe.h"
#include "daworldring.h"
#include "dgdata.h"
#include "hashpack.h"
#include <QProgressDialog>
#include <QSqlError>
#include <QSqlQuery>

AuDesignUndoRedoItem::AuDesignUndoRedoItem(TYPE type)
{
	m_Parts = new AuScParts;
	m_Type = type;
}

AuDesignUndoRedoItem::AuDesignUndoRedoItem()
{
	m_Parts = new AuScParts;
	m_Type = Add;
}

AuDesignUndoRedoItem::AuDesignUndoRedoItem(const AuDesignUndoRedoItem& c)
{
	m_Parts = new AuScParts(*c.m_Parts);
	m_Type = c.m_Type;
}

AuDesignUndoRedoItem::~AuDesignUndoRedoItem()
{
	delete m_Parts;
}

AuScParts& AuDesignUndoRedoItem::Parts() const
{
	return *m_Parts;
}

quint64 AuDesignUndoRedoItem::MemoryUsage() const
{
	quint64 mem = sizeof(AuDesignUndoRedoItem);
	if(m_Parts)
		mem += m_Parts->MemoryUsage();
	return mem;
}

const AuDesignUndoRedoItem& AuDesignUndoRedoItem::operator=(const AuDesignUndoRedoItem& c)
{
	m_Parts = new AuScParts(*c.m_Parts);
	m_Type = c.m_Type;
	return *this;
}

void AuDesignUndoRedoItem::operator>>(QDataStream& out) const
{
	out << m_Type;
	out << *m_Parts;
}

void AuDesignUndoRedoItem::operator<<(QDataStream& in)
{
	in >> m_Type;
	in >> *m_Parts;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuDesignUndoRedoTransaction::operator>>(QDataStream& out) const
{
	out << m_Desc;
	out << (quint16)size();
	for(const_iterator ii = cbegin(); ii != cend(); ii++)
		out << *ii;
}

void AuDesignUndoRedoTransaction::operator<<(QDataStream& in)
{
	in >> m_Desc;
	clear();
	quint16 cnt;
	in >> cnt;
	while(cnt--)
	{
		AuDesignUndoRedoItem i;
		in >> i;
		append(i);
	}
}

bool AuDesignUndoRedoTransaction::CanReplace(const AuDesignUndoRedoTransaction& trans) const
{
	if(m_Desc != trans.m_Desc)
		return false;

	if(size() != 1 || trans.size() != 1)
		return false;

	// The only kind of item that we replace is a change item.
	const AuDesignUndoRedoItem& myItem = first();
	if(myItem.Type() != AuDesignUndoRedoItem::Change)
		return false;
	const AuDesignUndoRedoItem& otherItem = trans.first();
	if(otherItem.Type() != AuDesignUndoRedoItem::Change)
		return false;

	// Compare the part id keys.
	QList<quint16> myKeys = myItem.Parts().keys();
	QList<quint16> otherKeys = otherItem.Parts().keys();
	return myKeys == otherKeys;
}

quint64 AuDesignUndoRedoTransaction::MemoryUsage() const
{
	quint64 mem = sizeof(AuDesignUndoRedoTransaction);
	mem += StringMemoryUsage(m_Desc);
	for(const_iterator ui = cbegin(); ui != cend(); ui++)
		mem += (*ui).MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuDesignUndoRedoList::MergeLast()
{
	if(size() < 2)
		return;

	iterator last = end() - 1;
	iterator nextToLast = last - 1;
	if((*last).CanReplace(*nextToLast))
	{
		erase(last);
		m_NextUndo = size()-1;
	}
}

quint64 AuDesignUndoRedoList::MemoryUsage() const
{
	quint64 mem = sizeof(AuDesignUndoRedoList);
	for(const_iterator ui = cbegin(); ui != cend(); ui++)
		mem += (*ui).MemoryUsage();
	return mem;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DaDesigner::DaDesigner(quint16 studioId) : DaDesignBase(studioId, DaDesignBase::DESIGN_Unknown),
	m_CmdEmpty(),
	m_EngineEmpty(),
	m_FireEmpty(),
	m_NavEmpty(),
	m_PilotEmpty(),
	m_PowerEmpty(),
	m_SensorEmpty(),
	m_ShieldEmpty(),
	m_SurgeryEmpty(),
	m_TransEmpty()
{
	m_Design = 0;
	m_DesignModified = false;
	for(int eff=EFF_None; eff<EFF_Count; eff++)
		m_EffectCur[eff] = 0;
	m_EffectFrame = 0;
	m_EffectRandFrame = 0;
	for(int eff=EFF_None; eff<EFF_Count+6; eff++)
		for(int frame=0; frame<6; frame++)
			m_EffectTex[eff][frame] = 0;
	m_GroundTex = 0;
	m_Preview = 0;
	m_PreviewGenSeed = 0;
	m_PreviewHasPower = true;
	m_ViewOnly = false;
}

DaDesigner::DaDesigner() : m_CmdEmpty(),
	m_EngineEmpty(),
	m_FireEmpty(),
	m_NavEmpty(),
	m_PilotEmpty(),
	m_PowerEmpty(),
	m_SensorEmpty(),
	m_ShieldEmpty(),
	m_SurgeryEmpty(),
	m_TransEmpty()
{
	m_Design = 0;
	m_DesignModified = false;
	for(int eff=EFF_None; eff<EFF_Count; eff++)
		m_EffectCur[eff] = 0;
	m_EffectFrame = 0;
	m_EffectRandFrame = 0;
	for(int eff=EFF_None; eff<EFF_Count+6; eff++)
		for(int frame=0; frame<6; frame++)
			m_EffectTex[eff][frame] = 0;
	m_GroundTex = 0;
	m_Preview = 0;
	m_PreviewGenSeed = 0;
	m_PreviewHasPower = true;
	m_ViewOnly = false;
}

DaDesigner::~DaDesigner()
{
	if(m_Design)
		delete m_Design;
	if(m_Preview)
		delete m_Preview;
}

DaDesigner::DaDesigner(const DaDesigner &c) : DaDesignBase(c),
	m_CmdEmpty(c.m_CmdEmpty),
	m_EngineEmpty(c.m_EngineEmpty),
	m_FireEmpty(c.m_FireEmpty),
	m_NavEmpty(c.m_NavEmpty),
	m_PilotEmpty(c.m_PilotEmpty),
	m_PowerEmpty(c.m_PowerEmpty),
	m_SensorEmpty(c.m_SensorEmpty),
	m_ShieldEmpty(c.m_ShieldEmpty),
	m_SurgeryEmpty(c.m_SurgeryEmpty),
	m_TransEmpty(c.m_TransEmpty)
{
	m_DesignModified = c.m_DesignModified;
	for(int eff=EFF_None; eff<EFF_Count; eff++)
		m_EffectCur[eff] = c.m_EffectCur[eff];
	m_EffectFrame = c.m_EffectFrame;
	m_EffectRandFrame = c.m_EffectRandFrame;
	for(int eff=EFF_None; eff<EFF_Count+6; eff++)
		for(int frame=0; frame<6; frame++)
			m_EffectTex[eff][frame] = c.m_EffectTex[eff][frame];
	m_GroundTex = 0;
	m_ViewOnly = c.m_ViewOnly;

	c.m_EnvLock.lockForRead();

	m_Architect = c.m_Architect;
	m_idArchitect = c.m_idArchitect;
	m_idOwner = c.m_idOwner;
	if(c.m_Design)
		m_Design = c.m_Design->Clone();
	else
		m_Design = 0;
	if(c.m_Preview)
		m_Preview = c.m_Preview->Clone();
	else
		m_Preview = 0;
	m_PreviewGenSeed = c.m_PreviewGenSeed;
	m_PreviewHasPower = c.m_PreviewHasPower;

	m_UndoRedo = c.m_UndoRedo;

	c.m_EnvLock.unlock();
}

DaAtom *DaDesigner::Clone() const
{
	return new DaDesigner(*this);
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaDesigner::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx, 
		const DaPerson& sceneAtom)
{
	DaDesignBase::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);

	quint8 hasWhat = 0;
	if(m_DesignModified)
		hasWhat |= 0x01;

	if(lastTx.IsNull() || !m_Design || m_DesignChangeTimes.empty())
		hasWhat |= 0x02;		// Design record with parts. Also works for no design record.
	else
	{
		QHash<quint16, AuElapsedTimer>::const_iterator cti = m_DesignChangeTimes.find(0);
		if(cti != m_DesignChangeTimes.cend() && *cti >= lastTx)
			hasWhat |= 0x04;	// Design record without parts.
	}

	if(m_ViewOnly)
		hasWhat |= 0x08;

	if(m_Preview)
		hasWhat |= 0x10;

	toClient << hasWhat;

	m_EnvLock.lockForRead();
	toClient << m_idOwner;

	if(hasWhat & 0x02)
	{
		// Send the design record with its parts.
		if(m_Design)
			toClient << qCompress(m_Design->Pack());
		else
			toClient << QByteArray();
	}
	else
	{
		if(hasWhat & 0x04)
		{
			// Send the design record without its parts.
			AuDesign* d = m_Design->Clone();
			d->ClearParts();
			toClient << (quint8)d->Type();
			toClient << *d;
			delete d;
		}

		// Send only the parts that were added or changed.
		for(QHash<quint16, AuElapsedTimer>::const_iterator cti = m_DesignChangeTimes.cbegin();
				cti != m_DesignChangeTimes.cend(); cti++)
		{
			if(cti.key() == 0)
				continue;	// Change time for design record itself, already handled.

			if(*cti < lastTx)
				continue;

			AuScParts::const_iterator pi = m_Design->Parts().find(cti.key());
			if(pi == m_Design->Parts().cend())
				continue;

			toClient << cti.key();
			toClient << (*pi)->Pack();
		}
		toClient << (quint16)0;	// Marks the end.

		// Send the list of all part ids, so deleted parts will be removed.
		QList<quint16> partIds = m_Design->Parts().keys();
		toClient << (quint16)partIds.size();
		for(QList<quint16>::const_iterator pidi = partIds.cbegin(); pidi != partIds.cend(); pidi++)
			toClient << *pidi;
	}

	if(hasWhat & 0x10)
	{
		// Send the design record with its parts.
		if(m_Preview)
			toClient << qCompress(m_Preview->Pack());
		else
			toClient << QByteArray();
		toClient << m_PreviewGenSeed;
	}

	m_EnvLock.unlock();
}

// Receive updates to this atom from a server.
void DaDesigner::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaDesignBase::RxUpdate(fromServer, isFirstUpdate);

	quint8 hasWhat;
	fromServer >> hasWhat;

	m_DesignModified = hasWhat & 0x01;
	m_ViewOnly = hasWhat & 0x08;

	fromServer >> m_idOwner;

	if(hasWhat & 0x02)
	{
		if(m_Design)
			delete m_Design;

		QByteArray bac;
		fromServer >> bac;
		if(!bac.isEmpty())
		{
			QByteArray packed = AuUncompress(bac, __FILE__, __LINE__,Id().StringId());
			m_Design = AuDesign::Unpack(packed);
		}
	}
	else
	{
		if(hasWhat & 0x04)
		{
			// Receive the design record without its parts.
			quint8 type;
			fromServer >> type;
			AuDesign* d = AuDesign::Create((AuDesign::BLUEPRINT_TYPE)type);
			if(m_Design->Type() == d->Type())
			{
				fromServer >> *d;
				d->Parts() = m_Design->Parts();
				delete m_Design;
				m_Design = d;
			}
			else
				delete d;	// D: Something is wrong.
		}

		// Read parts that changed.
		quint16 partId;
		do
		{
			// Read part ids until we get a zero.
			fromServer >> partId;
			if(partId)
			{
				QByteArray ba;
				fromServer >> ba;
				AuScPart* p = AuScPart::Unpack(ba);
				if(p)
				{
					AuScParts::iterator dpi = m_Design->Parts().find(partId);
					if(dpi != m_Design->Parts().end())
						delete *dpi;	// Replace an existing part.
					m_Design->Parts().insert(partId, p);
				}
			}
		}
		while(partId);

		// Read the list of part ids that are supposed to be in my list.
		QList<quint16> partIds;
		quint16 pcnt;
		fromServer >> pcnt;
		while(pcnt--)
		{
			quint16 partId;
			fromServer >> partId;
			partIds.append(partId);
		}

		// Delete parts that are no longer in the list.
		AuScParts::iterator pi = m_Design->Parts().begin();
		while(pi != m_Design->Parts().end())
		{
			if(partIds.contains(pi.key()))
				pi++;
			else
			{
				// The part was removed.
				delete *pi;
				pi = m_Design->Parts().erase(pi);
			}
		}
	}

	if(hasWhat & 0x10)
	{
		if(m_Preview)
			delete m_Preview;

		QByteArray bac;
		fromServer >> bac;
		if(!bac.isEmpty())
		{
			QByteArray packed = AuUncompress(bac, __FILE__, __LINE__,Id().StringId());
			m_Preview = AuDesign::Unpack(packed);
			fromServer >> m_PreviewGenSeed;
		}
		//m_PreviewHasPower is a client-side only setting.
	}
	else
	{
		if(m_Preview)
		{
			delete m_Preview;
			m_Preview = 0;
		}
		m_PreviewGenSeed = 0;
		//m_PreviewHasPower is a client-side only setting.
	}
	//qDebug("DaDesigner::RxUpdate %d", Id().GetID());
}

// Refresh data from valid counterpart.
bool DaDesigner::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	//qDebug() << "DaDesigner::Refresh" << Id() << "re" << (re ? "yes" : "no");
	DaDesignBase::Refresh(pAtom, ari);

	const DaDesigner* pA = (const DaDesigner*)pAtom;
	m_DesignModified = pA->m_DesignModified;
	m_ViewOnly = pA->m_ViewOnly;

	pA->m_EnvLock.lockForRead();
	m_EnvLock.lockForWrite();
	m_idOwner = pA->m_idOwner;

	if(m_Design)
		delete m_Design;
	if(pA->m_Design)
		m_Design = pA->m_Design->Clone();
	else
		m_Design = 0;

	if(m_Preview)
		delete m_Preview;
	if(pA->m_Preview)
		m_Preview = pA->m_Preview->Clone();
	else
		m_Preview = 0;
	m_PreviewGenSeed = pA->m_PreviewGenSeed;
	//m_PreviewHasPower is a client-side only setting.

	m_EnvLock.unlock();
	pA->m_EnvLock.unlock();

	if(ari)
	{
		//DHH Designer This could be smarter about when/what vao's to destroy.
		for(QHash<quint16, AuScVAO>::iterator vi = m_OpaqueVAOs.begin(); vi != m_OpaqueVAOs.end(); vi++)
			(*vi).DeleteGL();
		m_OpaqueVAOs.clear();

		for(QHash<quint16, AuScVAO>::iterator vi = m_TransparentVAOs.begin(); vi != m_TransparentVAOs.end(); vi++)
			(*vi).DeleteGL();
		m_TransparentVAOs.clear();

		for(QHash<quint16, AuScVAO>::iterator vi = m_OwnOpaqueVAOs.begin(); vi != m_OwnOpaqueVAOs.end(); vi++)
			(*vi).DeleteGL();
		m_OwnOpaqueVAOs.clear();

		for(QHash<quint16, AuScVAO>::iterator vi = m_OwnTransparentVAOs.begin(); vi != m_OwnTransparentVAOs.end(); vi++)
			(*vi).DeleteGL();
		m_OwnTransparentVAOs.clear();

		for(QHash<quint16, AuScVAO>::iterator vi = m_DecalOpaqueVAOs.begin(); vi != m_DecalOpaqueVAOs.end(); vi++)
			(*vi).DeleteGL();
		m_DecalOpaqueVAOs.clear();

		for(QHash<quint16, AuScVAO>::iterator vi = m_DecalTransparentVAOs.begin(); vi != m_DecalTransparentVAOs.end(); vi++)
			(*vi).DeleteGL();
		m_DecalTransparentVAOs.clear();

		for(QHash<quint16, AuScVAO>::iterator vi = m_OwnDecalOpaqueVAOs.begin(); vi != m_OwnDecalOpaqueVAOs.end(); vi++)
			(*vi).DeleteGL();
		m_OwnDecalOpaqueVAOs.clear();

		for(QHash<quint16, AuScVAO>::iterator vi = m_OwnDecalTransparentVAOs.begin(); vi != m_OwnDecalTransparentVAOs.end(); vi++)
			(*vi).DeleteGL();
		m_OwnDecalTransparentVAOs.clear();

		for(QHash<quint16, AuScVAO>::iterator vi = m_WindowTranslucentVAOs.begin(); vi != m_WindowTranslucentVAOs.end(); vi++)
			(*vi).DeleteGL();
		m_WindowTranslucentVAOs.clear();

		for(QHash<quint16, AuScVAO>::iterator vi = m_WindowTransparentVAOs.begin(); vi != m_WindowTransparentVAOs.end(); vi++)
			(*vi).DeleteGL();
		m_WindowTransparentVAOs.clear();
	}

	//qDebug() << "DaDesigner::Refresh" << Id() << "done";
	return false;
}

void DaDesigner::ListSceneSubatoms(const DaPerson* pSceneAtom, const AuPoint& cameraEnvLoc, DaAtomList& sceneAtoms)
{
	const DaAtom* sa = FindIdRecursive(pSceneAtom->Id());
	if(!sa)
		return;	// Do not get subatoms if the scene atom is not in this design environment.

	DaDesignBase::ListSceneSubatoms(pSceneAtom, cameraEnvLoc, sceneAtoms);
}

// Renders an entire scene from the perspective of this object.
// View transformation has already been set up.
void DaDesigner::RenderEnvironment(const AuMatrix4x4& projection, const AuMatrix4x4& farProjection, const AuMatrix4x4& model,
		AuAtomRenderInfo& ari, DaPerson* sceneAtom)
{
	if(!m_CmdEmpty.HasDeviceObjects())
	{
		// Create models of unmanned consoles.
		//qDebug() << "DaDesigner::RenderEnvironment" << Id();
		m_CmdEmpty.CreateDeviceObjectsEmpty(ari, *this, "Empty", 0, false);
		m_NavEmpty.CreateDeviceObjectsEmpty(ari, *this, QString("NCon%1").arg(Id().GetID()), ":data/txConsoleNav.bmp", false);
		m_EngineEmpty.CreateDeviceObjectsEmpty(ari, *this, QString("ECon%1").arg(Id().GetID()), ":data/ec_Console.bmp", false);
		m_FireEmpty.CreateDeviceObjectsEmpty(ari, *this, QString("WCon%1").arg(Id().GetID()), ":data/txConsoleWeapon.bmp",
				false);
		m_PilotEmpty.CreateDeviceObjectsEmpty(ari, *this, QString("PCon%1").arg(Id().GetID()), ":data/pc_Console.bmp", false);
		m_PowerEmpty.CreateDeviceObjectsEmpty(ari, *this, QString("DCon%1").arg(Id().GetID()), ":data/dc_Console.bmp", false);
		m_SensorEmpty.CreateDeviceObjectsEmpty(ari, *this, QString("SCon%1").arg(Id().GetID()), ":data/sc_Console.bmp", false);
		m_ShieldEmpty.CreateDeviceObjectsEmpty(ari, *this, QString("HCon%1").arg(Id().GetID()), ":data/txConsoleShield.bmp",
				false);
		m_SurgeryEmpty.CreateDeviceObjectsEmpty(ari, *this, QString("MCon%1").arg(Id().GetID()), ":data/txConsoleSurgery.bmp",
				false);
		m_TransEmpty.CreateDeviceObjectsEmpty(ari, *this, QString("TCon%1").arg(Id().GetID()), ":data/txConsoleTransporter.bmp",
				false);

		// Load all frames of the effect textures.
		for(int eff=0; eff<EFF_Count; eff++)
			for(int frame=0; frame<6; frame++)
				m_EffectTex[eff][frame] = AuTextureLibItem::EffectTexture(*ari.re, (SURFACE_EFFECT)eff, frame);

		// Maneuver drives in low power mode.
		m_EffectTex[EFF_Count][0] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_ManeuverDriveLow1);
		m_EffectTex[EFF_Count][1] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_ManeuverDriveLow2);
		m_EffectTex[EFF_Count][2] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_ManeuverDriveLow3);
		m_EffectTex[EFF_Count][3] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_ManeuverDriveLow4);
		m_EffectTex[EFF_Count][4] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_ManeuverDriveLow5);
		m_EffectTex[EFF_Count][5] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_ManeuverDriveLow6);

		// Transporter pads in low power mode.
		m_EffectTex[EFF_Count+1][0] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_TransporterLow1);
		m_EffectTex[EFF_Count+1][1] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_TransporterLow2);
		m_EffectTex[EFF_Count+1][2] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_TransporterLow3);
		m_EffectTex[EFF_Count+1][3] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_TransporterLow4);
		m_EffectTex[EFF_Count+1][4] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_TransporterLow5);
		m_EffectTex[EFF_Count+1][5] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_TransporterLow6);

		// FTL hull in warp.
		m_EffectTex[EFF_Count+2][0] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWarp1);
		m_EffectTex[EFF_Count+2][1] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWarp2);
		m_EffectTex[EFF_Count+2][2] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWarp3);
		m_EffectTex[EFF_Count+2][3] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWarp4);
		m_EffectTex[EFF_Count+2][4] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWarp5);
		m_EffectTex[EFF_Count+2][5] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWarp6);

		// FTL drives in low power mode.
		m_EffectTex[EFF_Count+3][0] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_FTLDriveLow1);
		m_EffectTex[EFF_Count+3][1] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_FTLDriveLow2);
		m_EffectTex[EFF_Count+3][2] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_FTLDriveLow3);
		m_EffectTex[EFF_Count+3][3] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_FTLDriveLow4);
		m_EffectTex[EFF_Count+3][4] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_FTLDriveLow5);
		m_EffectTex[EFF_Count+3][5] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_FTLDriveLow6);

		// FTL hull in negative wormhole.
		m_EffectTex[EFF_Count+4][0] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeg1);
		m_EffectTex[EFF_Count+4][1] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeg2);
		m_EffectTex[EFF_Count+4][2] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeg3);
		m_EffectTex[EFF_Count+4][3] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeg4);
		m_EffectTex[EFF_Count+4][4] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeg5);
		m_EffectTex[EFF_Count+4][5] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeg6);

		// FTL hull in neutral wormhole.
		m_EffectTex[EFF_Count+5][0] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeu1);
		m_EffectTex[EFF_Count+5][1] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeu2);
		m_EffectTex[EFF_Count+5][2] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeu3);
		m_EffectTex[EFF_Count+5][3] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeu4);
		m_EffectTex[EFF_Count+5][4] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeu5);
		m_EffectTex[EFF_Count+5][5] = AuTextureLibItem::EffectTexture(*ari.re, AuTextureLibItem::EI_HullWormNeu6);
	}

	AnimateEffectTextures();

	if(ari.m_DesignerShowGround)
	{
		if(!m_GroundTex)
			m_GroundTex = DaWorldLL::CreateTerrainTexture(*ari.re, WORLDTEX_DirtCoarse, g_Au.GroundTextureSize());

		if(!m_GroundVAO.VAO())
		{
			// Make a VAO for the ground that is the size of the ground spawn limit.
			double boxRad = GroundSpawnSizeLimit();

			const float s = 0;
			const float t = 0;
			const float u = boxRad / (2*FeetPerMeter());
			const float v = u;

			AuTriTex tri0;
			tri0.m_V0 = AuPointTex(-boxRad,-boxRad,0, s,t);
			tri0.m_V1 = AuPointTex(-boxRad,boxRad,0, s,v);
			tri0.m_V2 = AuPointTex(boxRad,boxRad,0, u,v);
			AuTriTex tri1;
			tri1.m_V0 = AuPointTex(boxRad,boxRad,0, u,v);
			tri1.m_V1 = AuPointTex(boxRad,-boxRad,0, u,t);
			tri1.m_V2 = AuPointTex(-boxRad,-boxRad,0, s,t);
			QList<AuTriTex> tris;
			tris << tri0 << tri1;

			m_GroundVAO.CreateForTexture(*ari.re, tris, GL_STATIC_DRAW);
		}
	}

	// Multiply by inverse of subatom transformation (Translation, rotation, and scale.)
	AuMatrix4x4 tsub;
	sceneAtom->GetTransformRelativeToParent(tsub, this);
	tsub = tsub.Inverse();

	// Set up lighting for the scene.
	AuMatrix4x4 tmodel = model * tsub;
	SetupLights(sceneAtom, tmodel, *ari.re);

	AuModelMatrixHash rmod;
	CollectModelMatrices(tmodel, rmod);

	DaAtomList shadSc;
	DaAtomList shadScFog;
	DaAtomList shadScGlobe;
	DaAtomList shadScGlobeFog;
	DaAtomList shadScRingworld;
	DaAtomList shadScRingworldFog;

	bool isPreview = IsPreview();
	if(isPreview)
	{
		// Render the solar system's environment first.
		if(!sceneAtom->IsEyeUnderWater())
		{
			const DaSolarSystem* system = sceneAtom->FindParentSolarSystem();
			if(system)
			{
				DaAtom* env = system->FindEnvironmentAtom();
				if(env)
				{
					env->RenderEnvironment(projection, farProjection, model, ari, sceneAtom);
					ari.re->glClear(GL_DEPTH_BUFFER_BIT);
				}
			}
		}

		// Render atoms in this solar system.
		// The renderLast map accumulates info about atoms that must be rendered in a final pass, usually due to blending effects.
		// The map is keyed by z-depth so the resulting contents are z-sorted, farthest to nearest.
		DaAtomList subs;
		ListSubatoms(subs);
		if(!subs.empty())
		{
			// Render distant objects.
			// Far atoms are drawn with a different near clip distance, so they don't have Z collisions. Then we clear the Z buffer.
			//qDebug() << "DaDesigner::RenderEnvironment" << Id() << Name() << "atoms" << subatoms->count();
			AuRenderLastMap renderFarLast;
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			{
				//qDebug() << (*it)->Id() << (*it)->Name() << (*it)->AtomState();
				(*it)->RenderEnvironmentObject(farProjection, ari, rmod, sceneAtom, renderFarLast, PASS_Far, shadSc, shadScFog,
						shadScGlobe, shadScGlobeFog, shadScRingworld, shadScRingworldFog);
			}

			RenderSc(farProjection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
					shadScRingworldFog, PASS_Far);

			// RenderLast worlds.
			// Render worlds that must be drawn last. The render last map is sorted by z-depth so the farthest items render first.
			for(AuRenderLastMap::iterator flri = renderFarLast.begin(); flri != renderFarLast.end(); flri++)
				if(dynamic_cast<DaWorldRing*>((*flri).second))
					(*flri).second->RenderLast(farProjection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog,
							shadScRingworld, shadScRingworldFog);

			RenderSc(farProjection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
					shadScRingworldFog, PASS_FarBlend);

			// Render warp tunnel effect or nebula cloud around scene atom before RenderLast objects.
			// Find scene-atom's top-most spaceship.
			{
				DaVessel* ship = sceneAtom->Parent()->FindParentVesselInEnv();
				while(ship && ship->Parent()->FindParentVessel())
				{
					DaVessel* pship = ship->Parent()->FindParentVessel();
					if(pship && pship->InDesignEnvironment())
						ship = pship;
					else
						break;
				}
				if(ship)
					ship->RenderWarpOrNebula(farProjection, rmod.Matrix(ship->Id()), ari);
				else
				{
					DaVehicle* vehicle = dynamic_cast<DaVehicle*>(sceneAtom->Parent());
					if(vehicle)
						vehicle->RenderWarpOrNebula(farProjection, rmod.Matrix(vehicle->Id()), ari);
					else
						sceneAtom->RenderWarpOrNebula(farProjection, model, ari);
				}
			}

			// Render non-worlds that must be drawn last. The render last map is sorted by z-depth so farthest items render first.
			for(AuRenderLastMap::iterator flri = renderFarLast.begin(); flri != renderFarLast.end(); flri++)
				if(!dynamic_cast<DaWorldRing*>((*flri).second))
					(*flri).second->RenderLast(farProjection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog,
							shadScRingworld, shadScRingworldFog);

			RenderSc(farProjection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
					shadScRingworldFog, PASS_FarBlend);

			// Don't render wormholes unless the person is outside the atmosphere range of a planet.
			//if(!sceneAtom->IsInAtmosphere())
				//RenderWormholes(farProjection, tmodel, PASS_Far, *ari.re, rmod);

			// Clear the depth buffer for the remainder of the scene rendering. The depth buffer is not compatible with rendering
			// of near objects using a different near clip distance. The contents of the scene so far simple fall into the backdrop.
			ari.re->glClear(GL_DEPTH_BUFFER_BIT);
		}

		AuRenderLastMap renderLast;
		for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
			(*it)->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, PASS_Near, shadSc, shadScFog, shadScGlobe,
					shadScGlobeFog, shadScRingworld, shadScRingworldFog);

		RenderSc(projection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
				shadScRingworldFog, PASS_Near);

		// Don't render wormholes unless the person is outside the atmosphere range of a planet.
		//if(!sceneAtom->IsInAtmosphere())
			//RenderWormholes(projection, tmodel, PASS_Near, *ari.re, rmod);

		// Render atoms that must be drawn last. The render last map is sorted by z-depth so the farthest items render first.
		for(AuRenderLastMap::iterator lri = renderLast.begin(); lri != renderLast.end(); lri++)
			(*lri).second->RenderLast(projection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog,
					shadScRingworld, shadScRingworldFog);

		RenderSc(projection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
				shadScRingworldFog, PASS_NearBlend);
	}
	else
	{
		// Render the skydome if in solid rendering
		if(ari.m_DesignerShowSkyBox)
		{
			const DaSolarSystem* system = sceneAtom->FindParentSolarSystem();
			if(system)
			{
				DaAtom* env = system->FindEnvironmentAtom();
				if(env)
				{
					env->RenderEnvironment(projection, farProjection, model, ari, sceneAtom);
					ari.re->glClear(GL_DEPTH_BUFFER_BIT);
				}
			}
		}

		// Render warp tunnel effect or nebula cloud around scene atom before RenderLast objects.
		// Find scene-atom's top-most spaceship.
		{
			DaVessel* ship = sceneAtom->Parent()->FindParentVesselInEnv();
			while(ship && ship->Parent()->FindParentVessel())
			{
				DaVessel* pship = ship->Parent()->FindParentVessel();
				if(pship && pship->InDesignEnvironment())
					ship = pship;
				else
					break;
			}

			if(ship)
				ship->RenderWarpOrNebula(farProjection, rmod.Matrix(ship->Id()), ari);
			else
			{
				DaVehicle* vehicle = dynamic_cast<DaVehicle*>(sceneAtom->Parent());
				if(vehicle)
					vehicle->RenderWarpOrNebula(farProjection, rmod.Matrix(vehicle->Id()), ari);
				else
					sceneAtom->RenderWarpOrNebula(farProjection, model, ari);
			}
		}

		// Clear the depth buffer for the remainder of the scene rendering. The depth buffer is not compatible with the rendering
		// of near objects using a different near clip distance. The contents of the scene so far simple fall into the backdrop.
		ari.re->glClear(GL_DEPTH_BUFFER_BIT);

		// Render the ground.
		if(ari.m_DesignerShowGround && m_Design)
		{
			const AuScDesign* scd = dynamic_cast<const AuScDesign*>(m_Design);
			if(scd)
			{
				AuPoint min,max;
				scd->Parts().HullExtents(min,max);

				AuMatrix4x4 gmv(tmodel);
				gmv.Translate(0,0, min.z);

				ari.re->glDisable(GL_CULL_FACE);

				// Configure shader.
				ari.re->GLSL().TextureShader().Select();
				ari.re->GLSL().TextureShader().SetProjectionModelView(projection * gmv);
				ari.re->GLSL().TextureShader().SetColor(.5,.5,.5,1);
				ari.re->GLSL().TextureShader().SetTexture(m_GroundTex);

				m_GroundVAO.SelectGL();
				m_GroundVAO.DrawGL();
				m_GroundVAO.DeselectGL();

				ari.re->GLSL().TextureShader().Deselect();
				ari.re->glEnable(GL_CULL_FACE);
			}
		}

		DaAtomList subs;
		ListSubatoms(subs);
		if(!subs.empty())
		{
			// The renderLast map accumulates info about atoms that must be rendered in a final pass, usually due to blending.
			// The map is keyed by z-depth so the resulting contents are z-sorted, farthest to nearest.
			AuRenderLastMap renderLast;

			// Render atoms in this design room.
			//qDebug() << "DaDesigner::RenderEnvironment" << Id() << Name() << "atoms" << subatoms->count();
			for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
				(*it)->RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, PASS_Near, shadSc, shadScFog,
						shadScGlobe, shadScGlobeFog, shadScRingworld, shadScRingworldFog);

			RenderSc(projection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
					shadScRingworldFog, PASS_Near);

			// Render the design solid faces.
			if(ari.m_DesignerShowFaces || ari.m_DesignerShowBacks)
				RenderDesign(projection, tmodel, ari, sceneAtom, false);

			// Render the design edge lines.
			if(ari.m_DesignerShowEdges && m_Design)
				m_Design->RenderEdges(projection, tmodel, *ari.re, ari.DesignerPartObjectSelection(), ari.DesignerObjectMode(),
						ari.IsDragging(), ari.m_DragT, ari.m_DragCopy, ari.DesignerObjectIsolate());

			// Render atoms that must be drawn last. The render last map is sorted by z-depth so the farthest items render first.
			for(AuRenderLastMap::iterator lri = renderLast.begin(); lri != renderLast.end(); lri++)
				(*lri).second->RenderLast(projection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog,
						shadScRingworld, shadScRingworldFog);

			RenderSc(projection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
					shadScRingworldFog, PASS_NearBlend);
		}

		RenderDesignPaths(projection, tmodel, ari, sceneAtom);

		// Render the design transparent faces, i.e. windows.
		if(ari.m_DesignerShowFaces || ari.m_DesignerShowBacks)
			RenderDesign(projection, tmodel, ari, sceneAtom, true);

		RenderDesignVoids(projection, tmodel, ari, sceneAtom);

		// Render the vertex sprites in object mode.
		if(ari.DesignerObjectMode() && m_Design)
		{
			if(g_Au.DesignerFindFaces())
				m_Design->RenderFaceSprites(projection, tmodel, *ari.re, ari.DesignerPartObjectSelection(), ari.m_DragT);
			if(g_Au.DesignerFindVertices())
				m_Design->RenderVertexSprites(projection, tmodel, *ari.re, ari.DesignerPartObjectSelection(), ari.m_DragT);
		}
	}
}

void DaDesigner::DeleteDeviceObjects(AuAtomRenderInfo& ari)
{
	DaDesignBase::DeleteDeviceObjects(ari);

	m_CmdEmpty.DeleteDeviceObjects(*ari.re);
	m_EngineEmpty.DeleteDeviceObjects(*ari.re);
	m_FireEmpty.DeleteDeviceObjects(*ari.re);
	m_NavEmpty.DeleteDeviceObjects(*ari.re);
	m_PilotEmpty.DeleteDeviceObjects(*ari.re);
	m_PowerEmpty.DeleteDeviceObjects(*ari.re);
	m_SensorEmpty.DeleteDeviceObjects(*ari.re);
	m_ShieldEmpty.DeleteDeviceObjects(*ari.re);
	m_SurgeryEmpty.DeleteDeviceObjects(*ari.re);
	m_TransEmpty.DeleteDeviceObjects(*ari.re);

	if(m_GroundVAO.VAO())
		m_GroundVAO.DeleteGL();

	for(QHash<quint16, AuScVAO>::iterator vi = m_OpaqueVAOs.begin(); vi != m_OpaqueVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_OpaqueVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_TransparentVAOs.begin(); vi != m_TransparentVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_TransparentVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_OwnOpaqueVAOs.begin(); vi != m_OwnOpaqueVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_OwnOpaqueVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_OwnTransparentVAOs.begin(); vi != m_OwnTransparentVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_OwnTransparentVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_DecalOpaqueVAOs.begin(); vi != m_DecalOpaqueVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_DecalOpaqueVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_DecalTransparentVAOs.begin(); vi != m_DecalTransparentVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_DecalTransparentVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_OwnDecalOpaqueVAOs.begin(); vi != m_OwnDecalOpaqueVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_OwnDecalOpaqueVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_OwnDecalTransparentVAOs.begin(); vi != m_OwnDecalTransparentVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_OwnDecalTransparentVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_WindowTranslucentVAOs.begin(); vi != m_WindowTranslucentVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_WindowTranslucentVAOs.clear();

	for(QHash<quint16, AuScVAO>::iterator vi = m_WindowTransparentVAOs.begin(); vi != m_WindowTransparentVAOs.end(); vi++)
		(*vi).DeleteGL();
	m_WindowTransparentVAOs.clear();

	/*
	if(m_Building)
		m_Building->DeleteDeviceObjects(re, mm, true);
		*/
}

void DaDesigner::NewDesign(const AuId& idArchitect, const QString& architect, const AuDesign& design)
{
	switch(design.Type())
	{
	case AuDesign::BLUEPRINT_Invalid:
	case AuDesign::BLUEPRINT_Count:
		return;
	case AuDesign::BLUEPRINT_Building:
		SetDesignMode(DESIGN_Building);
		SetPreview(0, 0, 0);
		break;
	case AuDesign::BLUEPRINT_Spacecraft:
		SetDesignMode(DESIGN_Spacecraft);
		SetPreview(0, 0, 0);
		break;
	case AuDesign::BLUEPRINT_Item:
		SetDesignMode(DESIGN_Item);
		SetPreview(0, 0, 0);
		break;
	}

	m_EnvLock.lockForWrite();
	if(m_Design)
		delete m_Design;
	if(m_Preview)
	{
		delete m_Preview;
		m_Preview = 0;
	}
	m_Design = design.Clone();
	m_DesignChangeTimes.clear();
	m_Design->SetId(0);											// New design does not have a db id yet.
	m_Design->SetArchitectId(idArchitect);
	m_Design->SetArchitectName(architect);
	m_Architect = architect;
	m_idArchitect = idArchitect;								// Architect of the new design.
	m_idOwner = idArchitect;									// Owner becomes the architect that starts the nu design.
	m_UndoRedo.clear();											// Discard undo data for all architects present.
	m_EnvLock.unlock();

	m_DesignModified = false;									// Until modified.
	m_ViewOnly = false;

	InitDesignPlane(idArchitect);
	ResetPositions();
	SetModified();
}

// Reset the transformation matrices of all the contents.
void DaDesigner::ResetPositions()
{
	// Get people out of ship and off of consoles.
	if(HasSubatoms())
	{
		DaAtomList people;
		FindPeopleRecursive(people);	// Only finds IsNormal people.
		for(DaAtomList::const_iterator it = people.begin(); it != people.end(); it++)
		{
			// Put the person at the origin with zero rotation, if they are aboard the preview ship.
			DaPerson* person = (DaPerson*)(*it);
			if(Is(person->ParentId()))
				continue;
			person->Stand();
			AuMatrix4x4 t;
			t.Translate(0, 0, person->GroundHeightAboveCG());
			person->ForcePosStop(AuAtomPos(Id(), t));
			person->SetModified();
		}
	}
}

QString DaDesigner::EntryPassword() const
{
	// The owner person atom establishes the password for others to enter.
	AuId idOwner = Owner();
	if(!idOwner.IsNull())
	{
		// The owner id looks valid. Get the password from the person atom.
		const DaAtom* owner = FindIdRecursive(idOwner);
		if(owner)
		{
			const DaPerson* person = (const DaPerson*)owner;
			return person->DesignPassword();
		}
	}

	// The owner person must not be here. Get the password from anybody in the studio.
	const DaPerson* person = FindPersonRecursive();
	if(person)
		return person->DesignPassword();

	// Nobody here so return a bogus password, which prohibits any matches.
	// We get here when somebody tries to enter a studio immediately after the last occupant leaves.
	// This place is probably slated for death anyway.
	return "*** Entry Prohibited ***";
}

// Insure a design plane exists for a particular architect.
void DaDesigner::InitDesignPlane(const AuId& idArchitect)
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	//qDebug() << "DaDesigner::InitDesignPlane" << idArchitect;
	DaDesignPlane* pdp = FindDesignPlane(idArchitect);
	if(!pdp)
	{
		// No design plane so we should create one owned by the architect.
		pdp = new DaDesignPlane(idArchitect);
		pdp->ConnectSignals();
		pdp->Create();
		pdp->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
		ether->AddAtom(pdp);
		g_Au.AtomAdded(pdp->Id());
		pdp->SetNormal();	// New atom is ready for normal thread processing.
	}
	else
	{
		// Move design plane to origin.
		AuMatrix4x4 t;
		pdp->ChangePosNotParent(AuAtomPos(Id(), t));

		// Turn off 3D shaded mode.
		if(pdp->IsRenderSolid())
			pdp->RenderModeToggle();

		// Restore default grid lines.
		pdp->EnableHorizontalLines();
		pdp->EnableVerticalLines();
		pdp->DisableDiagonal45Lines();
		pdp->DisableDiagonal135Lines();

		pdp->SetModified();
	}
	pdp->TickTock();
}

void DaDesigner::SetDesignPlane(const AuId& idArchitect, const AuPoint &fOrigin, const AuPoint &vX, const AuPoint &vY)
{
	DaDesignPlane *pdp = FindDesignPlane(idArchitect);
	if(!pdp)
		return;

	AuMatrix4x4 tdp;
	tdp.Translate(fOrigin);
	tdp.Rotate(AuPoint::CalculateRotation(vX, vY));
	pdp->ChangePosNotParent(AuAtomPos(Id(), tdp));
	pdp->SetModified();
}

AuMatrix4x4 DaDesigner::GetDesignPlaneTransform(const AuId& idArchitect) const
{
	if(DesignMode() <= DESIGN_Unknown)
		return AuMatrix4x4();

	DaDesignPlane *pdp = FindDesignPlane(idArchitect);
	Q_ASSERT(pdp);
	return pdp->Transform();
}

double DaDesigner::GetDesignPlaneGridSpacing(const AuId& idArchitect) const
{
	if(DesignMode() <= DESIGN_Unknown)
		return FeetPerMeter();

	DaDesignPlane *pdp = FindDesignPlane(idArchitect);
	Q_ASSERT(pdp);
	if (pdp)
		return pdp->GridSpacing();
	else
		return 0;
}

int DaDesigner::LoadBdDesign(quint32 designId, const AuId& idLoader, bool viewOnly)
{
	//qDebug() << "DaDesigner::LoadBdDesign" << Id() << "loader" << idLoader << "design" << designId << "viewOnly" << viewOnly;

	// Locate the existing design.
	AuDesign* d = 0;
	const DgData& sdd = g_Au.LockBdDesignsForRead();
	AuBdDesignHash::const_iterator di = sdd.m_BdDesigns->find(designId);
	if(di != sdd.m_BdDesigns->cend())
		d = (*di).second.Clone();
	g_Au.UnlockBdDesigns();
	if(!d)
	{
		// Design not found. Database load requested.
		g_Au.BdDesignNeeded(designId);
		//qWarning() << "warning: DaDesigner::LoadBdDesign design not found. Load requested." << designId;
		return 1;
	}

	int result = LoadDesign(d, idLoader, viewOnly);
	return result;
}

int DaDesigner::LoadItDesign(quint32 designId, const AuId& idLoader, bool viewOnly)
{
	qDebug() << "DaDesigner::LoadItDesign" << Id() << "loader" << idLoader << "design" << designId << "viewOnly" << viewOnly;

	// Locate the existing design.
	AuDesign* d = 0;
	const DgData& sdd = g_Au.LockItDesignsForRead();
	AuItDesignHash::const_iterator di = sdd.m_ItDesigns->find(designId);
	if(di != sdd.m_ItDesigns->cend())
		d = (*di).second.Clone();
	g_Au.UnlockItDesigns();
	if(!d)
	{
		// Design not found. Database load requested.
		g_Au.ItDesignNeeded(designId);
		//qWarning() << "warning: DaDesigner::LoadItDesign design not found. Load requested." << designId;
		return 1;
	}

	int result = LoadDesign(d, idLoader, viewOnly);
	return result;
}

int DaDesigner::LoadScDesign(quint32 designId, const AuId& idLoader, bool viewOnly)
{
	//qDebug() << "DaDesigner::LoadScDesign" << Id() << "loader" << idLoader << "design" << designId << "viewOnly" << viewOnly;

	// Locate the existing design.
	AuDesign* d = 0;
	const DgData& sdd = g_Au.LockScDesignsForRead();
	AuScDesignHash::const_iterator di = sdd.m_ScDesigns->find(designId);
	if(di != sdd.m_ScDesigns->cend())
		d = (*di).second.Clone();
	g_Au.UnlockScDesigns();
	if(!d)
	{
		// Design not found. Database load requested.
		g_Au.ScDesignNeeded(designId);
		//qWarning() << "warning: DaDesigner::LoadScDesign design not found. Load requested." << designId;
		return 1;
	}

	int result = LoadDesign(d, idLoader, viewOnly);
	return result;
}

int DaDesigner::LoadDesign(AuDesign* d, const AuId& idLoader, bool viewOnly)
{
	switch(d->Type())
	{
	case AuDesign::BLUEPRINT_Invalid:
	case AuDesign::BLUEPRINT_Count:
		delete d;
		return 2;
	case AuDesign::BLUEPRINT_Building:
		SetDesignMode(DESIGN_Building);
		SetPreview(0, 0, 0);
		break;
	case AuDesign::BLUEPRINT_Spacecraft:
		SetDesignMode(DESIGN_Spacecraft);
		SetPreview(0, 0, 0);
		break;
	case AuDesign::BLUEPRINT_Item:
		SetDesignMode(DESIGN_Item);
		SetPreview(0, 0, 0);
		break;
	}

	m_EnvLock.lockForWrite();
	if(m_Design)
		delete m_Design;
	if(m_Preview)
	{
		delete m_Preview;
		m_Preview = 0;
	}
	m_Design = d;								// I'll keep the design now.
	m_Design->SetId(0);							// Ignore whatever id got loaded from the library.
	if(!viewOnly)
		m_Design->SetState(AuDesign::Edit);
	m_DesignChangeTimes.clear();
	m_Architect = m_Design->ArchitectName();
	m_idArchitect = m_Design->ArchitectId();
	m_idOwner = idLoader;						// The loader is the owner of the design studio.
	m_UndoRedo.clear();							// Discard undo data for all architects present.
	m_EnvLock.unlock();

	m_DesignModified = false;					// Until modified.
	m_ViewOnly = viewOnly;

	InitDesignPlane(idLoader);
	ResetPositions();
	SetModified();
	return 0;
}

// Insert first time database record.
void DaDesigner::Insert(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QSqlQuery qa(QString(), db);
	QString sq("INSERT INTO a_Designer (id,idp,locX,locY,locZ,rotX,rotY,rotZ,timeCreate, "
				"timeModified,timeTock,timeDeath, archid,ownerid,design, "
				"studioid,modified,archname,designMode,viewOnly) VALUES ("
				"?,?,?,?,?,?,?,?,?, "
				"?,?,?, ?,?,?, "
				"?,?,?,?,?);");
	qa.prepare(sq);
	qa.addBindValue(Id().GetID());
	qa.addBindValue(pos.ParentId().GetID());
	qa.addBindValue(FuzzyClamp(vLoc.x));
	qa.addBindValue(FuzzyClamp(vLoc.y));
	qa.addBindValue(FuzzyClamp(vLoc.z));
	qa.addBindValue(FuzzyClamp(vRot.x));
	qa.addBindValue(FuzzyClamp(vRot.y));
	qa.addBindValue(FuzzyClamp(vRot.z));
	qa.addBindValue(CreatedTime().UTC());
	qa.addBindValue(ModifyTime().UTC());
	qa.addBindValue(TockTime().UTC());
	qa.addBindValue(DeathTime().UTC());

	// DaDesigner values
	qa.addBindValue(m_idArchitect.GetID());
	qa.addBindValue(m_idOwner.GetID());

	m_EnvLock.lockForRead();
	if(m_Design)
	{
		QByteArray ba(m_Design->Pack());
		QByteArray bac(qCompress(ba));
		if(bac.size() > (int)m_Design->CompressedSizeLimit())
		{
			// Store nothing when the size is too large.
			ba.clear();
			bac = qCompress(ba);
		}
		m_EnvLock.unlock();
		qa.addBindValue(bac);
	}
	else
	{
		m_EnvLock.unlock();
		qa.addBindValue(qCompress(QByteArray()));
	}

	qa.addBindValue(StudioId());
	qa.addBindValue(m_DesignModified);
	qa.addBindValue(m_Architect);
	qa.addBindValue(DesignMode());
	qa.addBindValue(m_ViewOnly);

	if(!qa.exec())
		qWarning("DaDesigner::Insert %s FAILED.", (const char*)Name().toLatin1());
}

// Update existing database record.
void DaDesigner::Update(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QString sq;
	sq = QString("UPDATE a_Designer SET idp=?, locX=?, locY=?, locZ=?, rotX=?, rotY=?, rotZ=?, "
				"timeModified=?, timeTock=?, timeDeath=?, archid=?, ownerid=?, "
				"design=?, modified=?, archname=?, designMode=?, viewOnly=? "
				"WHERE id=%2;").arg(Id().GetID());
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.addBindValue(pos.ParentId().GetID());
	qa.addBindValue(FuzzyClamp(vLoc.x));
	qa.addBindValue(FuzzyClamp(vLoc.y));
	qa.addBindValue(FuzzyClamp(vLoc.z));
	qa.addBindValue(FuzzyClamp(vRot.x));
	qa.addBindValue(FuzzyClamp(vRot.y));
	qa.addBindValue(FuzzyClamp(vRot.z));
	qa.addBindValue(ModifyTime().UTC());
	qa.addBindValue(TockTime().UTC());
	qa.addBindValue(DeathTime().UTC());

	// DaDesigner values
	qa.addBindValue(m_idArchitect.GetID());
	qa.addBindValue(m_idOwner.GetID());

	m_EnvLock.lockForRead();
	if(m_Design)
	{
		QByteArray ba(m_Design->Pack());
		QByteArray bac(qCompress(ba));
		if(bac.size() > (int)m_Design->CompressedSizeLimit())
		{
			// Store nothing when the size is too large.
			ba.clear();
			bac = qCompress(ba);
		}
		m_EnvLock.unlock();
		qa.addBindValue(bac);
	}
	else
	{
		m_EnvLock.unlock();
		qa.addBindValue(qCompress(QByteArray()));
	}

	qa.addBindValue(m_DesignModified);
	qa.addBindValue(m_Architect);
	qa.addBindValue(DesignMode());
	qa.addBindValue(m_ViewOnly);

	if(!qa.exec())
	{
		// Execution of the query failed. Report the error.
		QSqlError err = qa.lastError();
		qWarning("DaDesigner::Update %d %s FAILED '%s' err '%s'.",
				Id().GetID(), (const char*)Name().toLatin1(), (const char*)sq.toLatin1(), (const char*)err.text().toLatin1());
	}
	else if(qa.numRowsAffected() == 0)
	{
		// Execution succeeded but did not affect any records.
		// This happens when an atom is created that gets transferred to another server before the INSERT query is ever posted.
		// On arrival to the destination server, all atoms are touched with SetModified so they'll get saved.
		//qDebug("DaDesigner::Update %d %s affected NO records. Trying Insert instead.",
				//Id().GetID(), (const char*)Name().toAscii());
		Insert(db);
	}
}

// static
// Load all of these atoms whose parent is idParent.
// Returns 0 if 'idToLoad' == 0, which loads all atoms subordinate to 'aParent'.
// Returns address of loaded atom if 'idToLoad' != 0 and it is successfully loaded.
DaDesigner* DaDesigner::LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad)
{
	QString sq("SELECT id,locX,locY,locZ,rotX,rotY,rotZ,timeCreate,timeModified,"
				"timeTock,timeDeath, "
				"archid,ownerid,design,studioid,modified,archname,designMode,viewOnly "
				"FROM a_Designer WHERE ");
	if(!idToLoad.IsNull())
		sq += QString(" id=%1").arg(idToLoad.GetID());
	else
		sq += QString(" idp=%1").arg(parentId.GetID());
	sq += ";";
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		DaDesigner *pnu = new DaDesigner;
		if(pnu)
		{
			// DaAtom values.
			int qv=0;	// query value index
			AuId id = AuId(qa.value(qv++).toUInt());
			pnu->SetId(id);
			double vx = qa.value(qv++).toDouble();
			double vy = qa.value(qv++).toDouble();
			double vz = qa.value(qv++).toDouble();
			double rx = qa.value(qv++).toDouble();
			double ry = qa.value(qv++).toDouble();
			double rz = qa.value(qv++).toDouble();
			AuAtomPos pos(parentId, vx,vy,vz, rx,ry,rz);
			pnu->SetPosAndOrient(pos);

			QDateTime tmCreated = qa.value(qv++).toDateTime();
			QDateTime tmModify = qa.value(qv++).toDateTime();
			QDateTime tmTocked = qa.value(qv++).toDateTime();
			QDateTime tmDeath = qa.value(qv++).toDateTime();
			tmCreated.setTimeSpec(Qt::UTC);
			tmModify.setTimeSpec(Qt::UTC);
			tmTocked.setTimeSpec(Qt::UTC);
			tmDeath.setTimeSpec(Qt::UTC);
			QDateTime now(QDateTime::currentDateTime());
			now.setTimeSpec(Qt::UTC);
			if(tmCreated > now)
				tmCreated = now;
			if(tmModify > now)
				tmModify = now;
			if(tmTocked > now)
				tmTocked = now;
			pnu->SetCreatedTime(tmCreated);
			pnu->SetModifyTime(tmModify);
			pnu->SetTockTime(tmTocked);
			pnu->SetDeathTime(tmDeath);

			pnu->m_idArchitect = AuId(qa.value(qv++).toUInt());
			pnu->m_idOwner = AuId(qa.value(qv++).toUInt());

			// Design record
			if(pnu->m_Design)
			{
				delete pnu->m_Design;
				pnu->m_Design = 0;
			}
			if(pnu->m_Preview)
			{
				delete pnu->m_Preview;
				pnu->m_Preview = 0;
			}
			int qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				QByteArray bac(qa.value(qvi).toByteArray());
				if(!bac.isEmpty())
				{
					QByteArray ba = AuUncompress(bac, __FILE__, __LINE__,id.StringId());
					if(ba.isEmpty())
					{
						qDebug() << "DaDesigner::LoadSubatoms Z_DATA_ERROR handled ok.";
						pnu->m_Design = AuDesign::Unpack(bac);	// Unpack the data that did not uncompress.
					}
					else
						pnu->m_Design = AuDesign::Unpack(ba);	// Unpack the uncompressed data.
				}
			}

			pnu->SetStudioId(qa.value(qv++).toUInt());
			pnu->m_DesignModified = qa.value(qv++).toBool();
			pnu->m_Architect = qa.value(qv++).toString();
			pnu->SetDesignMode((DESIGNMODE)qa.value(qv++).toUInt());
			pnu->m_ViewOnly = qa.value(qv++).toBool();

			pnu->ConnectSignals();
			ether.AddAtom(pnu);
			pnu->SetNormal();	// New atom is ready for normal thread processing. This must be done before loading subatoms.

			// Load subatoms of the nu atom AFTER adding it to its pnu parent.
			//qDebug("DaDesigner - Loading Subatoms ");
			DaDesignPlane::LoadSubatoms(ether, db, pnu->Id());
			DaStar::LoadSubatoms(ether, db, pnu->Id(), (LOADSUBATOMS)(LOAD_Moons|LOAD_GamePieces));
			if(pnu->DesignMode() == DESIGN_Spacecraft)
				DaSc::LoadSubatoms(ether, db, pnu->Id());
			//else if(pnu->DesignMode() == DESIGN_Item)
				//DaItem::LoadSubatoms(ether, db, pnu->Id());
			// Preview Bd is always subordinate to a world.
			//else if(pnu->DesignMode() == DESIGN_Building)
				//DaBd::LoadSubatoms(ether, db, pnu->Id());
			DaPerson::LoadSubatoms(ether, db, pnu->Id());
			DaItem::LoadSubatoms(ether, db, pnu->Id());
		}
		if(!idToLoad.IsNull())
			return pnu;
	}
	return 0;
}

bool DaDesigner::IsGravityOn(const AuId& idArchitect) const
{
	const DaDesignPlane* pdp = FindDesignPlane(idArchitect);
	if(pdp)
		return pdp->IsGravityOn();
	return false;
}

bool DaDesigner::IsRenderSolid(const AuId& idArchitect) const
{
	const DaDesignPlane* pdp = FindDesignPlane(idArchitect);
	if(pdp)
		return pdp->IsRenderSolid();
	return false;
}

short int DaDesigner::GridOnBits(const AuId& idArchitect) const
{
	short int gridonbits = 0;
	const DaDesignPlane *pdp = FindDesignPlane(idArchitect);
	if(pdp)
		gridonbits = pdp->GridOnBits();
	return gridonbits;
}

unsigned int DaDesigner::IsTocker() const
{
	return 10*d_MSecPerSec;
}

void DaDesigner::Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans)
{
	DaDesignBase::Tock(last, now, scripts, tockTrans);
	if(!IsNormal())
		return;

	// Kill the design environment if there are no other people present.
	// It may need to be scheduled to die in a short while. Otherwise the kill
	// signal might reaches the client before the person atom is reparented to
	// the terrain on the client, which causes severe problems. We need to be
	// sure that we don't permit entry into a design environment that is dying.
	if(!FindPersonRecursive())
	{
		Kill();
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}
}

// Called at the completion of a client sending a ship design to the server.
void DaDesigner::DesignLoadCompleted()
{
	m_EnvLock.lockForWrite();
	m_DesignChangeTimes.clear();
	m_EnvLock.unlock();

	SetDesignModified();
}

// Checks to see if a particle moving from oloc to nloc is blocked by walls or
// other objects that are part of the ship design.
// 'hitNormal' is an output parameter.
bool DaDesigner::IsPathBlocked(const AuPoint& ol, const AuPoint& nl, AuPoint& hitLoc, AuPoint& hitNormal, const DaAtom* toWho)
{
	if(IsPreview())
		return DaDesignBase::IsPathBlocked(ol, nl, hitLoc, hitNormal, toWho);

	if(!g_Au.DesignerBlockPaths())
		return false;

	bool v = false;
	m_EnvLock.lockForRead();
	if(m_Design)
		v = m_Design->IsPathBlocked(ol, nl, hitLoc, hitNormal);
	m_EnvLock.unlock();
	return v;
}

double DaDesigner::Gravity(double cgDistance) const
{
	Q_UNUSED(cgDistance);
	return GravityStandard();
}

QString DaDesigner::DesignName() const
{
	m_EnvLock.lockForRead();
	QString v;
	if(m_Design)
		v = m_Design->Name();
	m_EnvLock.unlock();
	return v;
}

quint32 DaDesigner::DbDesignId() const
{
	m_EnvLock.lockForRead();
	quint32 v;
	if(m_Design)
		v = m_Design->Id();
	else
		v = 0;
	m_EnvLock.unlock();
	return v;
}

const AuDesign* DaDesigner::LockDesignForRead()
{
	//qDebug() << "DaDesigner::LockDesignForRead" << Id() << this;
	m_EnvLock.lockForRead();
	if(m_Preview)
		return m_Preview;
	return m_Design;
}

void DaDesigner::UnlockDesign()
{
	m_EnvLock.unlock();
	//qDebug() << "DaDesigner::UnlockDesign" << Id() << this;
}

void DaDesigner::SetProperties(const AuDesign& design)
{
	if(!m_Design)
		return;

	m_EnvLock.lockForWrite();
	m_Design->SetProperties(design);
	m_DesignChangeTimes.insert(0, AuElapsedTimer::Now());	// id 0 is the record itself.
	m_EnvLock.unlock();

	SetDesignModified();
}

void DaDesigner::PartAdd(const AuId& archId, const AuScPart& part)
{
	if(!m_Design)
		return;

	// We only permit one instance of some parts in a design.
	if(part.IsOnePerDesign())
	{
		m_EnvLock.lockForRead();
		quint16 existingPartId = m_Design->FindPart(part.Type());
		m_EnvLock.unlock();
		if(existingPartId)
		{
			// Change the existing bulb instead of adding a new one.
			AuDesignUndoRedoItem uitem(AuDesignUndoRedoItem::Change);

			m_EnvLock.lockForWrite();
			if(m_Design->PartUpdate(existingPartId, part, uitem.Parts()))
			{
				StartUndo(archId, tr("Change %1").arg(part.PartName())).append(uitem);
				m_DesignChangeTimes.insert(existingPartId, AuElapsedTimer::Now());
				SetDesignModified();
			}
			m_EnvLock.unlock();
			return;
		}
	}

	// Add the part.

	// Prepare the undo transaction.
	AuDesignUndoRedoItem uitem(AuDesignUndoRedoItem::Add);

	m_EnvLock.lockForWrite();
	quint16 partId = m_Design->PartAdd(part, uitem.Parts());
	if(partId)
	{
		// Create the undo transaction.
		StartUndo(archId, tr("Add %1").arg(part.PartName())).append(uitem);
		m_DesignChangeTimes.insert(partId, AuElapsedTimer::Now());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::PartUpdate(const AuId& archId, quint16 partId, const AuScPart& part)
{
	if(!m_Design)
		return;

	// Prepare the undo transaction.
	AuDesignUndoRedoItem uitem(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartUpdate(partId, part, uitem.Parts()))
	{
		StartUndo(archId, tr("Change %1").arg(part.PartName())).append(uitem);
		m_DesignChangeTimes.insert(partId, AuElapsedTimer::Now());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

bool DaDesigner::SaveToFile(const AuId& idArchitect, const QString& fileName, const QImage* thumb) const
{
	if(!m_Design || m_ViewOnly)
		return false;

	m_EnvLock.lockForRead();

	AuDesign* c = m_Design->Clone();
	c->ClearArchitectForFileSave(idArchitect);
	if(thumb)
		c->SetThumbnail(*thumb);
	bool success = c->SaveToFile(fileName, SOH_FILE_TYPE);
	delete c;

	m_EnvLock.unlock();

	if(!success)
		return false;

	// Clear design modified flag since it is now saved.
	//m_DesignModified = false;	//DHH This really doesn't work since this function is called on the client.
	//SetModified();

	return true;
}

bool DaDesigner::LoadedFromFile(const AuDesign& design)
{
	switch(design.Type())
	{
	case AuDesign::BLUEPRINT_Invalid:
	case AuDesign::BLUEPRINT_Count:
		return false;
	case AuDesign::BLUEPRINT_Building:
		SetDesignMode(DESIGN_Building);
		SetPreview(0, 0, 0);
		break;
	case AuDesign::BLUEPRINT_Spacecraft:
		SetDesignMode(DESIGN_Spacecraft);
		SetPreview(0, 0, 0);
		break;
	case AuDesign::BLUEPRINT_Item:
		SetDesignMode(DESIGN_Item);
		SetPreview(0, 0, 0);
		break;
	}

	m_EnvLock.lockForWrite();
	if(m_Design)
		delete m_Design;
	if(m_Preview)
	{
		delete m_Preview;
		m_Preview = 0;
	}
	m_Design = design.Clone();
	m_Design->SetId(0);											// Ignore whatever id got loaded from the file.
	m_Design->SetState(AuDesign::Edit);							// Load from file in Edit state.
	m_DesignChangeTimes.clear();
	// DtChatChannel set the architect information to match the avatar who loaded the design. In solo mode, we don't care.
	m_Architect = m_Design->ArchitectName();					// Architect name and id were set to the loader on the client.
	m_idArchitect = m_Design->ArchitectId();
	m_idOwner = m_Design->ArchitectId();						// The loader becomes the owner of the design studio.
	m_UndoRedo.clear();											// Discard undo data for all architects present.
	m_EnvLock.unlock();

	m_DesignModified = false;									// Until modified.
	m_ViewOnly = false;

	InitDesignPlane(design.ArchitectId());
	ResetPositions();
	SetModified();
	return true;
}

void DaDesigner::PartSelectionIds(AuScSelection& partIds) const
{
	if(!m_Design)
		return;

	m_EnvLock.lockForRead();
	m_Design->PartIds(partIds);
	m_EnvLock.unlock();
}

void DaDesigner::PartSelectionEdgeIds(AuScSelection& partIds) const
{
	if(!m_Design)
		return;

	m_EnvLock.lockForRead();
	m_Design->PartEdgeIds(partIds);
	m_EnvLock.unlock();
}

void DaDesigner::PartSelectionFaceIds(AuScSelection& partIds) const
{
	if(!m_Design)
		return;

	m_EnvLock.lockForRead();
	m_Design->PartFaceIds(partIds);
	m_EnvLock.unlock();
}

void DaDesigner::PartSelectionVertexIds(AuScSelection& partIds) const
{
	if(!m_Design)
		return;

	m_EnvLock.lockForRead();
	m_Design->PartVertexIds(partIds);
	m_EnvLock.unlock();
}

quint16 DaDesigner::FindEdgeAtPixel(const AuMatrix4x4& projection, const AuMatrix4x4& model, const DaPerson* sceneAtom,
		const QPointF& pixel, const AuScSelection& sel, bool objectMode, bool objectIsolate, AuPoint* snapLoc) const
{
	if(!m_Design)
		return 0;

	// Multiply by inverse of subatom transformation.
	AuMatrix4x4 tsub;
	sceneAtom->GetTransformRelativeToParent(tsub, this);
	tsub = tsub.Inverse();

	// Create model view projection matrix to put things into the eye space of the scene atom.
	AuMatrix4x4 mvp = projection * model * tsub;
	m_EnvLock.lockForRead();
	quint16 partId = m_Design->FindEdgeAtPixel(mvp, pixel, sel, objectMode, objectIsolate, snapLoc);
	m_EnvLock.unlock();
	return partId;
}

quint16 DaDesigner::FindVertexAtPixel(const AuMatrix4x4& projection, const AuMatrix4x4& model, const DaPerson* sceneAtom,
		const QPointF& pixel, const AuScSelection& sel, bool objectMode, bool objectIsolate, AuPoint* snapLoc) const
{
	if(!m_Design)
		return 0;

	// Multiply by inverse of subatom transformation.
	AuMatrix4x4 tsub;
	sceneAtom->GetTransformRelativeToParent(tsub, this);
	tsub = tsub.Inverse();

	// Create model view projection matrix to put things into the eye space of the scene atom.
	AuMatrix4x4 mvp = projection * model * tsub;
	m_EnvLock.lockForRead();
	quint16 partId = m_Design->FindVertexAtPixel(mvp, pixel, sel, objectMode, objectIsolate, snapLoc);
	m_EnvLock.unlock();
	return partId;
}

// Fills 'shapeIds' set with all shapes that are in the pixel rectangle from 'start' to 'end', even partially.
void DaDesigner::FindPartsInArea(const AuMatrix4x4& projection, const AuMatrix4x4& model, const DaPerson* sceneAtom,
		const QRectF& pixelRect, AuScSelection& partIds) const
{
	if(!m_Design)
		return;

	// Multiply by inverse of subatom transformation.
	AuMatrix4x4 tsub;
	sceneAtom->GetTransformRelativeToParent(tsub, this);
	tsub = tsub.Inverse();

	// Create model view projection matrix to put things into the eye space of the scene atom.
	AuMatrix4x4 mvp = projection * model * tsub;
	m_EnvLock.lockForRead();
	m_Design->FindPartsInArea(mvp, pixelRect, partIds, g_Au.DesignerFindVertices(), g_Au.DesignerFindEdges(),
			g_Au.DesignerFindFaces());
	m_EnvLock.unlock();
}

quint16 DaDesigner::FindPartAtPixel(const AuMatrix4x4& projection, const AuMatrix4x4& model, const DaPerson* sceneAtom,
		const QPointF& pixel) const
{
	if(!m_Design)
		return 0;

	// Multiply by inverse of subatom transformation.
	AuMatrix4x4 tsub;
	sceneAtom->GetTransformRelativeToParent(tsub, this);
	tsub = tsub.Inverse();

	// Create model view projection matrix to put things into the eye space of the scene atom.
	AuMatrix4x4 mvp = projection * model * tsub;
	m_EnvLock.lockForRead();
	quint16 partId = m_Design->FindPartObjectAtPixel(mvp, pixel, true, true, false);
	m_EnvLock.unlock();
	return partId;
}

quint16 DaDesigner::FindPartObjectAtPixel(const AuMatrix4x4& projection, const AuMatrix4x4& model, const DaPerson* sceneAtom,
		const QPointF& pixel, const AuScSelection& partIds) const
{
	if(!m_Design)
		return 0;

	// Multiply by inverse of subatom transformation.
	AuMatrix4x4 tsub;
	sceneAtom->GetTransformRelativeToParent(tsub, this);
	tsub = tsub.Inverse();

	// Create model view projection matrix to put things into the eye space of the scene atom.
	AuMatrix4x4 mvp = projection * model * tsub;
	m_EnvLock.lockForRead();
	quint16 partId = m_Design->FindPartObjectAtPixel(mvp, pixel, partIds, g_Au.DesignerFindVertices(), g_Au.DesignerFindEdges(),
			            g_Au.DesignerFindFaces());
	m_EnvLock.unlock();
	return partId;
}

void DaDesigner::FindPartObjectsInArea(const AuMatrix4x4& projection, const AuMatrix4x4& model, const DaPerson* sceneAtom,
		const QRectF& pixelRect, AuScSelection& partIds) const
{
	if(!m_Design)
		return;

	// Multiply by inverse of subatom transformation.
	AuMatrix4x4 tsub;
	sceneAtom->GetTransformRelativeToParent(tsub, this);
	tsub = tsub.Inverse();

	// Create model view projection matrix to put things into the eye space of the scene atom.
	AuMatrix4x4 mvp = projection * model * tsub;
	m_EnvLock.lockForRead();
	m_Design->FindPartObjectsInArea(mvp, pixelRect, partIds, g_Au.DesignerFindVertices(), g_Au.DesignerFindEdges(),
			g_Au.DesignerFindFaces());
	m_EnvLock.unlock();
}

// Creates a custom mime data object of the shapes identified.
QMimeData* DaDesigner::CreateMIME(const AuId& architectId, bool objectMode, const AuScSelection& partIds,
		const AuMatrix4x4& designPlane) const
{
	if(!m_Design || m_ViewOnly)
		return 0;

	m_EnvLock.lockForRead();
	QMimeData* d = m_Design->CreateMIME(architectId, objectMode, partIds, designPlane);
	m_EnvLock.unlock();
	return d;
}

// Reads MIME encoded shapes. Returns them in a list.
void DaDesigner::ReadMIME(const AuId& architectId, const QString& architectName, bool objectMode, const QMimeData& mimeData,
		AuScParts& parts) const
{
	return AuScParts::ReadMIME(architectId, architectName, objectMode, mimeData, parts);
}

// Checks for MIME encoded shapes. Returns true if they are found.
bool DaDesigner::CheckMIME(bool objectMode, const QMimeData& mimeData) const
{
	return AuScParts::CheckMIME(objectMode, mimeData);
}

void DaDesigner::AddParts(const AuId& archId, const AuScParts& parts)
{
	if(!m_Design)
		return;

	// Prepare the undo transaction.
	AuDesignUndoRedoItem uadd(AuDesignUndoRedoItem::Add);
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	QHash<quint16, quint16> nuPartIds;
	m_EnvLock.lockForWrite();
	for(AuScParts::const_iterator pi = parts.cbegin(); pi != parts.cend(); pi++)
	{
		// We only permit one instance of some parts in a design. e.g. capt berth, headlight bulb
		if((*pi)->IsOnePerDesign())
		{
			quint16 existingPartId = m_Design->FindPart((*pi)->Type());
			if(existingPartId)
			{
				// Change the existing unique item instead of adding a new one.
				m_Design->PartUpdate(existingPartId, **pi, uchg.Parts());
				continue;
			}
		}

		// We get here if it was not a unique item or if it is a unique item that we don't already have.
		quint16 nuPartId = m_Design->PartAdd(**pi, uadd.Parts());	// Add to the undo transaction.
		nuPartIds.insert(pi.key(), nuPartId);						// Remember old part id and nu part id to fix associations.
	}
	m_Design->FixPartAssociations(nuPartIds);

	if(!uadd.Parts().empty() || !uchg.Parts().empty())
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Add Parts"));
		if(!uadd.Parts().empty())
			undo.append(uadd);
		if(!uchg.Parts().empty())
			undo.append(uchg);
		SetPartChangeTimesNoLock(uadd.Parts());
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::SetPartChangeTimes(const AuScParts& parts)
{
	m_EnvLock.lockForWrite();
	SetPartChangeTimesNoLock(parts);
	m_EnvLock.unlock();
}

void DaDesigner::SetPartChangeTimesNoLock(const AuScParts& parts)
{
	if(parts.empty())
		return;

	for(AuScParts::const_iterator pi = parts.cbegin(); pi != parts.cend(); pi++)
		m_DesignChangeTimes.insert(pi.key(), AuElapsedTimer::Now());
}

void DaDesigner::AddPartObjects(const AuId& archId, quint16 partId, const AuScParts& parts)
{
	if(!m_Design)
		return;

	// Prepare the undo transaction.
	AuDesignUndoRedoItem uitem(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	for(AuScParts::const_iterator pi = parts.cbegin(); pi != parts.cend(); pi++)
		m_Design->PartObjectAdd(partId, **pi, uitem.Parts());	// Add to the undo transaction.

	if(!uitem.Parts().empty())
	{
		StartUndo(archId, tr("Add Part Objects")).append(uitem);	// Create the undo transaction.
		SetPartChangeTimesNoLock(uitem.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::DeleteParts(const AuId& archId, const AuScSelection& sel)
{
	if(!m_Design)
		return;

	// Prepare the undo transaction.
	AuDesignUndoRedoItem uitem(AuDesignUndoRedoItem::Delete);

	m_EnvLock.lockForWrite();
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
		m_Design->PartDelete(si.key(), uitem.Parts());					// Add to the undo transaction.

	if(!uitem.Parts().empty())
	{
		StartUndo(archId, tr("Delete Parts")).append(uitem);	// Create the undo transaction.
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::DeletePartObjects(const AuId& archId, const AuScSelection& sel)
{
	if(!m_Design)
		return;

	// Prepare the undo transaction.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);
	AuDesignUndoRedoItem udel(AuDesignUndoRedoItem::Delete);

	m_EnvLock.lockForWrite();
	for(AuScSelection::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		m_Design->PartObjectDelete(si.key(), *si, uchg.Parts(), udel.Parts());			// Add to the undo transaction.
		if(!udel.Parts().empty())
			break;	// 'partId' got deleted.
	}

	if(!udel.Parts().empty() && !uchg.Parts().empty())
	{
		AuDesignUndoRedoTransaction& utrans = StartUndo(archId, tr("Delete Part Objects"));
		utrans.append(uchg);
		utrans.append(udel);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	else if(!udel.Parts().empty())
	{
		StartUndo(archId, tr("Delete Part Objects Destroyed Part")).append(udel);	// Create the undo transaction.
		SetDesignModified();
	}
	else if(!uchg.Parts().empty())
	{
		StartUndo(archId, tr("Delete Part Objects")).append(uchg);	// Create the undo transaction.
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::CopyParts(const AuId& archId, const AuScSelection& sel, const AuMatrix4x4& t)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uadditem(AuDesignUndoRedoItem::Add);

	// Copy the selected parts in place.
	m_EnvLock.lockForWrite();
	if(m_Design->PartsCopy(sel, uadditem.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& utrans = StartUndo(archId, tr("Copy Parts"));
		utrans.append(uadditem);

		if(!t.IsIdentity())
		{
			// Move the selected parts, leaving behind the new parts.
			// This order might seem odd but the result leaves the selected parts at the new position, instead of at the
			// original position. This makes multiple copies of the same parts easier.
			AuDesignUndoRedoItem uchgitem(AuDesignUndoRedoItem::Change);
			if(m_Design->PartsTransform(sel, t, uchgitem.Parts(), false))
			{
				utrans.append(uchgitem);
				SetPartChangeTimesNoLock(uchgitem.Parts());
			}
		}

		SetPartChangeTimesNoLock(uadditem.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::CopyPartObjects(const AuId& archId, const AuScSelection& sel, const AuMatrix4x4& t)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uitem(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsTransformCopy(sel, t, uitem.Parts()))
	{
		// Create the undo transaction.
		StartUndo(archId, tr("Copy Part Objects")).append(uitem);
		SetPartChangeTimesNoLock(uitem.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::CopyPartsSymmetrical(const AuId& archId, const AuScSelection& sel, const AuMatrix4x4& gridPlane)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uadditem(AuDesignUndoRedoItem::Add);

	// Copy the selected parts in place.
	m_EnvLock.lockForWrite();
	if(m_Design->PartsCopySymmetrical(sel, gridPlane, uadditem.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& utrans = StartUndo(archId, tr("Copy Parts"));
		utrans.append(uadditem);

		SetPartChangeTimesNoLock(uadditem.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::MoveLiftStops(const AuId& archId, const AuScSelection& sel, const AuMatrix4x4& t)
{
	if(!m_Design)
		return;

	if(t.IsIdentity())
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uitem(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsTransformLiftStops(sel, t, uitem.Parts()))
	{
		// Create the undo transaction.
		StartUndo(archId, tr("Move Turbo Lift Stops")).append(uitem);
		SetPartChangeTimesNoLock(uitem.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::MoveParts(const AuId& archId, const AuScSelection& sel, const AuMatrix4x4& t)
{
	if(!m_Design)
		return;

	if(t.IsIdentity())
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uitem(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsTransform(sel, t, uitem.Parts(), false))
	{
		// Create the undo transaction.
		StartUndo(archId, tr("Move Parts")).append(uitem);
		SetPartChangeTimesNoLock(uitem.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::MovePartObjects(const AuId& archId, const AuScSelection& sel, const AuMatrix4x4& t)
{
	if(!m_Design)
		return;

	if(t.IsIdentity())
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uitem(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsTransform(sel, t, uitem.Parts(), true))
	{
		// Create the undo transaction.
		StartUndo(archId, tr("Move Part Objects")).append(uitem);
		SetPartChangeTimesNoLock(uitem.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

AuDesignUndoRedoTransaction& DaDesigner::StartUndo(const AuId& architectId, const QString& desc)
{
	// Start a new undo/redo transaction.
	AuDesignUndoRedoTransaction undo(desc);
	QHash<AuId, AuDesignUndoRedoList>::iterator	uri = m_UndoRedo.find(architectId);
	if(uri == m_UndoRedo.end())
	{
		// This is the first undo transaction created by the architect. Start a list of undo/redo transactions for them.
		AuDesignUndoRedoList urlist;
		uri = m_UndoRedo.insert(architectId, urlist);
	}

	// Add the new undo transaction. Truncates pending redo transactions.
	// The new transaction is empty of items. Each undo/redo event must call UndoTransaction to get this current transaction and
	// undo transactions can then be added to the transaction.
	return (*uri).AddUndo(undo);
}

void DaDesigner::EndUndo(const AuId& architectId)
{
	QHash<AuId, AuDesignUndoRedoList>::iterator	uri = m_UndoRedo.find(architectId);
	if(uri == m_UndoRedo.end())
		return;

	(*uri).MergeLast();
}

void DaDesigner::Undo(const AuId& architectId)
{
	if(!m_Design)
		return;

	m_EnvLock.lockForWrite();
	QHash<AuId, AuDesignUndoRedoList>::iterator	uri = m_UndoRedo.find(architectId);
	if(uri == m_UndoRedo.end())
	{
		m_EnvLock.unlock();
		return;							// No undo/redo transactions exist for architectId.
	}
	AuDesignUndoRedoTransaction* undo = (*uri).Undo();
	if(!undo)
	{
		m_EnvLock.unlock();
		return;							// No undo returned. Must be no pending undo transaction.
	}

	// The undo transaction can contain multiple items that describe the entire transaction.
	// We undo them in reverse order because they were created sequentially when the transaction was made.
	for(AuDesignUndoRedoTransaction::iterator uei = undo->end()-1; uei != undo->begin()-1; uei--)
	{
		switch((*uei).Type())
		{
		case AuDesignUndoRedoItem::Add:
			// Undo an 'add' operation by removing the corresponding shapes from the model.
			m_Design->UndoRedoDelete((*uei).Parts().keys());
			break;
		case AuDesignUndoRedoItem::Change:
			// Undo a 'change' operation by replacing the changed shapes with their old clones, made prior to the change.
			// The replaced shapes are swapped into the undo transaction's shape list in preparation for the next redo.
			m_Design->UndoRedoSwap((*uei).Parts());
			SetPartChangeTimesNoLock((*uei).Parts());
			break;
		case AuDesignUndoRedoItem::Delete:
			// Undo a 'delete' operation by restoring the deleted shapes to the model.
			m_Design->UndoRedoAdd((*uei).Parts());
			SetPartChangeTimesNoLock((*uei).Parts());
			break;
		}
	}
	m_EnvLock.unlock();

	SetDesignModified();
}

void DaDesigner::Redo(const AuId& architectId)
{
	if(!m_Design)
		return;

	m_EnvLock.lockForWrite();
	QHash<AuId, AuDesignUndoRedoList>::iterator	uri = m_UndoRedo.find(architectId);
	if(uri == m_UndoRedo.end())
	{
		m_EnvLock.unlock();
		return;							// No undo/redo transactions exist for architectId.
	}
	AuDesignUndoRedoTransaction* redo = (*uri).Redo();
	if(!redo)
	{
		m_EnvLock.unlock();
		return;							// No redo returned. Must be no pending redo transaction.
	}

	// The redo transaction can contain multiple items that describe the entire transaction.
	// We redo them in order because they were created sequentially when the transaction was made.
	for(AuDesignUndoRedoTransaction::iterator uei = redo->begin(); uei != redo->end(); uei++)
	{
		switch((*uei).Type())
		{
		case AuDesignUndoRedoItem::Add:
			// Redo an 'add' operation by restoring the added shapes to the model.
			m_Design->UndoRedoAdd((*uei).Parts());
			SetPartChangeTimesNoLock((*uei).Parts());
			break;
		case AuDesignUndoRedoItem::Change:
			// Redo a 'change' operation by replacing the old shapes with their changed clones, swapped here during the last undo.
			// The replaced shapes are swapped into the redo transaction's shape list in preparation for the next undo.
			m_Design->UndoRedoSwap((*uei).Parts());
			SetPartChangeTimesNoLock((*uei).Parts());
			break;
		case AuDesignUndoRedoItem::Delete:
			// Redo a 'delete' operation by removing the corresponding shapes from the model.
			m_Design->UndoRedoDelete((*uei).Parts().keys());
			break;
		}
	}
	m_EnvLock.unlock();

	SetDesignModified();
}

QString DaDesigner::PartSelectedDesc(quint16 partId) const
{
	if(!m_Design)
		return "";

	m_EnvLock.lockForRead();
	QString desc = m_Design->PartSelectedDesc(partId);
	m_EnvLock.unlock();
	return desc;
}

bool DaDesigner::SelectPartTextureVertices(quint16 partId, quint32 partVertex, AuAtomRenderInfo& ari) const
{
	if(!m_Design)
		return false;

	m_EnvLock.lockForRead();
	bool result = m_Design->SelectPartTextureVertices(partId, partVertex, ari.DesignerSelection());
	m_EnvLock.unlock();
	return result;
}

bool DaDesigner::DeselectPartTextureVertices(quint16 partId, quint32 partVertex, AuAtomRenderInfo& ari) const
{
	if(!m_Design)
		return false;

	m_EnvLock.lockForRead();
	bool result = m_Design->DeselectPartTextureVertices(partId, partVertex, ari.DesignerSelection());
	m_EnvLock.unlock();
	return result;
}

bool DaDesigner::SelectPartObjectsAtPixel(const AuMatrix4x4& projection, const AuMatrix4x4& model, const DaPerson* sceneAtom,
			const QPointF& pixel, AuScSelection& sel) const
{
	if(!m_Design)
		return false;

	// Multiply by inverse of subatom transformation.
	AuMatrix4x4 tsub;
	sceneAtom->GetTransformRelativeToParent(tsub, this);
	tsub = tsub.Inverse();

	// Create model view projection matrix to put things into the eye space of the scene atom.
	AuMatrix4x4 mvp = projection * model * tsub;
	m_EnvLock.lockForRead();
	bool result = m_Design->SelectPartObjectsAtPixel(mvp, pixel, sel, g_Au.DesignerFindVertices(), g_Au.DesignerFindEdges(),
			g_Au.DesignerFindFaces());
	m_EnvLock.unlock();
	return result;
}

bool DaDesigner::DeselectPartObjectsAtPixel(const AuMatrix4x4& projection, const AuMatrix4x4& model, const DaPerson* sceneAtom,
			const QPointF& pixel, AuScSelection& sel) const
{
	if(!m_Design)
		return false;

	// Multiply by inverse of subatom transformation.
	AuMatrix4x4 tsub;
	sceneAtom->GetTransformRelativeToParent(tsub, this);
	tsub = tsub.Inverse();

	// Create model view projection matrix to put things into the eye space of the scene atom.
	AuMatrix4x4 mvp = projection * model * tsub;
	m_EnvLock.lockForRead();
	bool result = m_Design->DeselectPartObjectsAtPixel(mvp, pixel, sel, g_Au.DesignerFindVertices(), g_Au.DesignerFindEdges(),
			g_Au.DesignerFindFaces());
	m_EnvLock.unlock();
	return result;
}

void DaDesigner::InvertPartObjectSelection(AuScSelection& sel) const
{
	if(!m_Design)
		return;

	m_EnvLock.lockForRead();
	m_Design->InvertPartObjectSelection(sel);
	m_EnvLock.unlock();
}

void DaDesigner::TogglePartObjectSelection(const AuMatrix4x4& projection, const AuMatrix4x4& modelView, const DaPerson* sceneAtom,
		AuScSelection& sel, const QRectF& pixelRect) const
{
	if(!m_Design)
		return;

	// Multiply by inverse of subatom transformation.
	AuMatrix4x4 tsub;
	sceneAtom->GetTransformRelativeToParent(tsub, this);
	tsub = tsub.Inverse();

	// Create model view projection matrix to put things into the eye space of the scene atom.
	AuMatrix4x4 mvp = projection * modelView * tsub;
	m_EnvLock.lockForRead();
	m_Design->TogglePartObjectSelectionInArea(mvp, pixelRect, sel);
	m_EnvLock.unlock();
}

void DaDesigner::JigCutParts(const AuId& archId, const AuScSelection& sel)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uadd(AuDesignUndoRedoItem::Add);
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);
	AuDesignUndoRedoItem udel(AuDesignUndoRedoItem::Delete);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsJigCut(archId, sel, uadd.Parts(), uchg.Parts(), udel.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Jig Cut Parts"));
		if(!uadd.Parts().empty())
			undo.append(uadd);
		if(!uchg.Parts().empty())
			undo.append(uchg);
		if(!udel.Parts().empty())
			undo.append(udel);
		SetPartChangeTimesNoLock(uadd.Parts());
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::JoinParts(const AuId& archId, quint16 partId, const AuScSelection& sel)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);
	AuDesignUndoRedoItem udel(AuDesignUndoRedoItem::Delete);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsJoin(partId, sel, uchg.Parts(), udel.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Join Parts"));
		undo.append(uchg);
		undo.append(udel);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::ReverseFaces(const AuId& archId, const AuScSelection& sel)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartFacesReverse(sel, uchg.Parts()))
	{
		// Create the undo transaction.
		StartUndo(archId, tr("Reverse Faces")).append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::FractureFaces(const AuId& archId, const AuScSelection& sel)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartFacesFracture(sel, uchg.Parts()))
	{
		// Create the undo transaction.
		StartUndo(archId, tr("Fracture Faces")).append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::FuseFaces(const AuId& archId, const AuScSelection& sel)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartFacesFuse(sel, uchg.Parts()))
	{
		// Create the undo transaction.
		StartUndo(archId, tr("Fuse Faces")).append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::SplitFaces(const AuId& archId, const AuScSelection& sel)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uadd(AuDesignUndoRedoItem::Add);
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsSplit(sel, uadd.Parts(), uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Split Faces"));
		undo.append(uchg);
		undo.append(uadd);
		SetPartChangeTimesNoLock(uadd.Parts());
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::FlattenVertices(const AuId& archId, const AuScSelection& sel)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartVerticesFlatten(sel, uchg.Parts()))
	{
		// Create the undo transaction.
		StartUndo(archId, tr("Flatten Vertices")).append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::SmoothVertices(const AuId& archId, const AuScSelection& sel)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartVerticesSmooth(sel, uchg.Parts()))
	{
		// Create the undo transaction.
		StartUndo(archId, tr("Smooth Vertices")).append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

// Fills a boundary in object mode by adding a face to the first part in 'sel'.
void DaDesigner::PartFillFace(const AuId& archId, const AuScSelection& sel, const AuPointArray& boundary,
		const AuMatrix4x4& gridPlane)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartFillFace(sel, boundary, gridPlane, uchg.Parts()))
	{
		// Create the undo transaction.
		StartUndo(archId, tr("Fill Face")).append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::PartFillQuad(const AuId& archId, const AuScSelection& sel, const AuPoint& v1, const AuPoint& v2, const AuPoint& v3,
		const AuPoint& v4)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartFillQuad(sel, v1, v2, v3, v4, uchg.Parts()))
	{
		// Create the undo transaction.
		StartUndo(archId, tr("Fill Quad")).append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::PartFillTriangle(const AuId& archId, const AuScSelection& sel, const AuPoint& v1, const AuPoint& v2,
		const AuPoint& v3)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartFillTriangle(sel, v1, v2, v3, uchg.Parts()))
	{
		// Create the undo transaction.
		StartUndo(archId, tr("Fill Triangle")).append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

// Const access to the parts of the design.
const AuScParts* DaDesigner::LockPartsForRead()
{
	//qDebug() << "DaDesigner::LockPartsForRead" << Id() << this;
	m_EnvLock.lockForRead();
	if(!m_Design)
		return 0;
	return &m_Design->Parts();
}

void DaDesigner::UnlockParts()
{
	m_EnvLock.unlock();
	//qDebug() << "DaDesigner::UnlockParts" << Id() << this;
}

void DaDesigner::SetPartsHidden(const AuId& archId, const AuScIdSet& partIds, bool hide)
{
	if(!m_Design)
		return;

	Q_UNUSED(archId);
	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	// Change all the parts.
	m_EnvLock.lockForWrite();
	for(QSet<quint16>::const_iterator pi = partIds.m_Ids.cbegin(); pi != partIds.m_Ids.cend(); pi++)
		m_Design->PartSetHidden(*pi, hide, uchg.Parts());

	// Finish undo if anything changed.
	if(!uchg.Parts().empty())
	{
		/*
		if(hide)
			StartUndo(archId, tr("Hide Parts")).append(uchg);
		else
			StartUndo(archId, tr("Show Parts")).append(uchg);
		*/
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::SetPartsLocked(const AuId& archId, const AuScIdSet& partIds, bool lock)
{
	if(!m_Design)
		return;

	Q_UNUSED(archId);
	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	// Change all the parts.
	m_EnvLock.lockForWrite();
	for(QSet<quint16>::const_iterator pi = partIds.m_Ids.cbegin(); pi != partIds.m_Ids.cend(); pi++)
		m_Design->PartSetLocked(*pi, lock, uchg.Parts());

	// Finish undo if anything changed.
	if(!uchg.Parts().empty())
	{
		/*
		if(lock)
			StartUndo(archId, tr("Lock Parts")).append(uchg);
		else
			StartUndo(archId, tr("Unlock Parts")).append(uchg);
			*/
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::SetPartsOmitted(const AuId& archId, const AuScIdSet& partIds, bool omit)
{
	if(!m_Design)
		return;

	Q_UNUSED(archId);
	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	// Change all the parts.
	m_EnvLock.lockForWrite();
	for(QSet<quint16>::const_iterator pi = partIds.m_Ids.cbegin(); pi != partIds.m_Ids.cend(); pi++)
		m_Design->PartSetOmitted(*pi, omit, uchg.Parts());

	// Finish undo if anything changed.
	if(!uchg.Parts().empty())
	{
		/*
		if(omit)
			StartUndo(archId, tr("Omit Parts")).append(uchg);
		else
			StartUndo(archId, tr("Include Parts")).append(uchg);
			*/
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::SetPartsSealed(const AuId& archId, const AuScIdSet& partIds, bool seal)
{
	if(!m_Design)
		return;

	Q_UNUSED(archId);
	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	// Change all the parts.
	m_EnvLock.lockForWrite();
	for(QSet<quint16>::const_iterator pi = partIds.m_Ids.cbegin(); pi != partIds.m_Ids.cend(); pi++)
		m_Design->PartSetSealed(*pi, seal, uchg.Parts());

	// Finish undo if anything changed.
	if(!uchg.Parts().empty())
	{
		/*
		if(seal)
			StartUndo(archId, tr("Seal Parts")).append(uchg);
		else
			StartUndo(archId, tr("Unseal Parts")).append(uchg);
			*/
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::SetPartStopName(const AuId& archId, quint16 partId, const QString& name)
{
	if(!m_Design)
		return;

	if(!name.isEmpty() && !IsNameValid(name))
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartSetStopName(partId, name, uchg.Parts()))
	{
		// Create the undo transaction.
		StartUndo(archId, tr("Change Part Turbo Lift Stop Name")).append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::SetPartOpacity(const AuId& archId, quint16 partId, float opacity)
{
	if(!m_Design)
		return;

	Q_UNUSED(archId);
	if(opacity < 0 || opacity > 1)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartSetOpacity(partId, opacity, uchg.Parts()))
	{
		// Create the undo transaction.
		//StartUndo(archId, tr("Change Part Opacity")).append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::SetPartState(const AuId& archId, quint16 partId, quint8 state)
{
	Q_UNUSED(archId);
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartSetState(partId, state, uchg.Parts()))
	{
		// Create the undo transaction.
		//StartUndo(archId, tr("Change Part State")).append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::AddPartState(const AuId& archId, quint16 partId, quint8 state)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartAddState(partId, state, uchg.Parts()))
	{
		// Create the undo transaction.
		StartUndo(archId, tr("Add Part State")).append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::DeletePartState(const AuId& archId, quint16 partId, quint8 state)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartDeleteState(partId, state, uchg.Parts()))
	{
		// Create the undo transaction.
		StartUndo(archId, tr("Delete Part State")).append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::PartAddFaces(const AuId& archId, quint16 partId, const AuScFaceList& faces)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartAddFaces(partId, faces, uchg.Parts()))
	{
		// Create the undo transaction.
		StartUndo(archId, tr("Add Part Faces")).append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::PartReplaceFaces(const AuId& archId, quint16 partId, const AuScSelection& sel, const AuScFaceList& faces)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartAddFaces(partId, faces, uchg.Parts()))
	{
		AuScSelection::const_iterator si = sel.find(partId);
		if(si != sel.cend())
		{
			// Remove the selected faces but discard the undo result. We already have a copy of the part before any changes.
			AuDesignUndoRedoItem uchg2(AuDesignUndoRedoItem::Change);
			AuDesignUndoRedoItem udel2(AuDesignUndoRedoItem::Delete);
			m_Design->PartObjectDelete(partId, *si, uchg2.Parts(), udel2.Parts());
		}

		// Create the undo transaction.
		StartUndo(archId, tr("Replace Part Faces")).append(uchg);

		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::RenderEdges(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re,
		const AuScFaceList& faces) const
{
	AuPointArray edges;
	for(AuScFaceList::const_iterator fi = faces.cbegin(); fi != faces.cend(); fi++)
	{
		AuSegmentArray segs;
		(*fi)->EdgeSegments(segs);

		QMap<AuSegment, bool> alreadyChecked;
		for(AuSegmentArray::const_iterator si = segs.cbegin(); si != segs.cend(); si++)
		{
			if(alreadyChecked.contains(*si))
				continue;

			AuSegment segrev((*si).m_fLoc2, (*si).m_fLoc1);	// Segment with points in reverse order, for alreadyChecked check.
			if(alreadyChecked.contains(segrev))
				continue;

			edges.append((*si).m_fLoc1);
			edges.append((*si).m_fLoc2);
		}
	}
	if(edges.empty())
		return;

	AuMatrix4x4 pmv = projection * model;
	re.GLSL().LineShader().Select();
	re.GLSL().LineShader().SetProjectionModelView(pmv);
	re.GLSL().LineShader().SetColor(g_Au.DesignerLineColor());

	// Make temporary VAO of the edge segments.
	AuGLVAO vao;
	vao.CreateForLines(re, edges, false, GL_STREAM_DRAW);

	// Display the VAO once and clean it up.
	vao.SelectGL();
	vao.DrawGL(GL_LINES);
	vao.DeselectGL();
	vao.DeleteGL();

	re.GLSL().LineShader().Deselect();
}

void DaDesigner::RenderFaces(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuGLEffects& re, const AuScFaceList& faces,
		quint32 empireId) const
{
	Q_UNUSED(projection);
	if(faces.empty())
		return;

	QColor themeColors[AuScFace::THEME_Count];
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireThemeColors(empireId, themeColors);
	g_Au.UnlockData();

	AuScTriRenderList opaqueTris, transparentTris;
	faces.RenderTriangles(opaqueTris, transparentTris, themeColors);
	if(opaqueTris.empty() && transparentTris.empty())
		return;

	AuScVAO opaqueVAO, transparentVAO;
	if(!opaqueTris.empty())
	{
		opaqueVAO.Create(re, opaqueTris, GL_STREAM_DRAW);
		if(!opaqueVAO.VAO())
			return;
	}
	if(!transparentTris.empty())
	{
		transparentVAO.Create(re, transparentTris, GL_STREAM_DRAW);
		if(!transparentVAO.VAO())
			return;
	}
	if(!opaqueVAO.VAO() && !transparentVAO.VAO())
		return;

	// Get/load the texture ids of the design's texture palette.
	AuTexturePalette surfaceTex;
	m_Design->ShaderTexturePalette(m_Design->HullModule(), re, surfaceTex);

	re.GLSL().ScShader().Select(projection, re);
	re.GLSL().ScShader().SetModelView(model);
	re.GLSL().ScShader().SetSurfaceTextures(surfaceTex);
	re.GLSL().ScShader().SetGlowDiscard(0,0,0, .01f);
	re.GLSL().ScShader().SetVertAlpha(1);

	// Ambient light
	float ambientR = AuGLLight::MaxAmbientStarlight();
	float ambientG = AuGLLight::MaxAmbientStarlight();
	float ambientB = AuGLLight::MaxAmbientStarlight();
	AuPoint ambientUp(0,0,1);
	re.GLSL().ScShader().SetAmbientLight(ambientR, ambientG, ambientB);
	re.GLSL().ScShader().SetAmbientLightUp(ambientUp);

	if(opaqueVAO.VAO())
	{
		opaqueVAO.DrawSc(re.GLSL().ScShader(), EffectTextures(), 2);
		opaqueVAO.DeleteGL();
	}

	if(transparentVAO.VAO())
	{
		re.glEnable(GL_BLEND);
		re.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		transparentVAO.DrawSc(re.GLSL().ScShader(), EffectTextures(), 2);
		transparentVAO.DeleteGL();
		re.glDisable(GL_BLEND);
	}

	re.GLSL().ScShader().Deselect();
}

void DaDesigner::Faces(AuScFaceList& faces, const AuScSelection& sel) const
{
	if(!m_Design)
		return;
	m_EnvLock.lockForRead();
	m_Design->Faces(faces, sel);
	m_EnvLock.unlock();
}

void DaDesigner::VisibleFaces(AuScFaceList& faces) const
{
	if(!m_Design)
		return;
	m_EnvLock.lockForRead();
	m_Design->VisibleFaces(faces);
	m_EnvLock.unlock();
}

void DaDesigner::VisibleFaces(AuScFaceList& faces, const QString& groupName) const
{
	if(!m_Design)
		return;
	m_EnvLock.lockForRead();
	m_Design->VisibleFaces(faces, groupName);
	m_EnvLock.unlock();
}

void DaDesigner::CutFaces(const AuId& archId, const AuScSelection& sel, const AuPointArray& cutLine)
{
	if(!m_Design)
		return;

	// Prepare the undo transaction.
	AuDesignUndoRedoItem uitem(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	m_Design->CutFaces(sel, cutLine, uitem.Parts());					// Add to the undo transaction.

	if(!uitem.Parts().empty())
	{
		StartUndo(archId, tr("Cut Faces")).append(uitem);	// Create the undo transaction.
		SetPartChangeTimesNoLock(uitem.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::PunchFaces(const AuId& archId, const AuScSelection& sel, const AuPointArray& cutBnd)
{
	if(!m_Design)
		return;

	// Prepare the undo transaction.
	AuDesignUndoRedoItem uitem(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	m_Design->PunchFaces(sel, cutBnd, uitem.Parts());					// Add to the undo transaction.

	if(!uitem.Parts().empty())
	{
		StartUndo(archId, tr("Punch Faces")).append(uitem);	// Create the undo transaction.
		SetPartChangeTimesNoLock(uitem.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::Loop(const AuScSelection& sel, const AuPoint& loc, AuPointArray& loop) const
{
	if(!m_Design)
		return;

	m_EnvLock.lockForRead();
	m_Design->Loop(sel, loc, loop);
	m_EnvLock.unlock();
}

void DaDesigner::DivideFaces(const AuId& archId, const AuScSelection& sel, quint8 divisions)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartFacesDivide(sel, divisions, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Divide Faces"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::MakeNavLightLens(const AuId& archId, const AuScSelection& sel, const QColor& color)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uadd(AuDesignUndoRedoItem::Add);
	AuDesignUndoRedoItem udel(AuDesignUndoRedoItem::Delete);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsMakeNavLightLens(sel, color, uadd.Parts(), udel.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Make Navigation Light"));
		undo.append(udel);
		undo.append(uadd);
		SetPartChangeTimesNoLock(uadd.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::MakeDecal(const AuId& archId, const AuScSelection& sel)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uadd(AuDesignUndoRedoItem::Add);
	AuDesignUndoRedoItem udel(AuDesignUndoRedoItem::Delete);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsMakeDecal(sel, uadd.Parts(), udel.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Make Decal"));
		undo.append(udel);
		undo.append(uadd);
		SetPartChangeTimesNoLock(uadd.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::MakeDoor(const AuId& archId, const AuScSelection& sel)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uadd(AuDesignUndoRedoItem::Add);
	AuDesignUndoRedoItem udel(AuDesignUndoRedoItem::Delete);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsMakeDoor(sel, uadd.Parts(), udel.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Make Door"));
		undo.append(udel);
		undo.append(uadd);
		SetPartChangeTimesNoLock(uadd.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

/*
void DaDesigner::MakeHeadlightLens(const AuId& archId, const AuScSelection& sel)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uadd(AuDesignUndoRedoItem::Add);
	AuDesignUndoRedoItem udel(AuDesignUndoRedoItem::Delete);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsMakeHeadlightLens(sel, uadd.Parts(), udel.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Make Headlight"));
		undo.append(udel);
		undo.append(uadd);
		SetPartChangeTimesNoLock(uadd.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}
*/

void DaDesigner::MakeLightLens(const AuId& archId, const AuScSelection& sel, const QColor& lightColor)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uadd(AuDesignUndoRedoItem::Add);
	AuDesignUndoRedoItem udel(AuDesignUndoRedoItem::Delete);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsMakeLightLens(sel, lightColor, uadd.Parts(), udel.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Make Light Lens"));
		undo.append(udel);
		undo.append(uadd);
		SetPartChangeTimesNoLock(uadd.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::MakeRoom(const AuId& archId, const AuScSelection& sel)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uadd(AuDesignUndoRedoItem::Add);
	AuDesignUndoRedoItem udel(AuDesignUndoRedoItem::Delete);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsMakeRoom(sel, uadd.Parts(), udel.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Make Room"));
		undo.append(udel);
		undo.append(uadd);
		SetPartChangeTimesNoLock(uadd.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::MakeHull(const AuId& archId, const AuScSelection& sel)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uadd(AuDesignUndoRedoItem::Add);
	AuDesignUndoRedoItem udel(AuDesignUndoRedoItem::Delete);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsMakeHull(sel, uadd.Parts(), udel.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Make Hull"));
		undo.append(udel);
		undo.append(uadd);
		SetPartChangeTimesNoLock(uadd.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::MakeLandingGear(const AuId& archId, const AuScSelection& sel)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uadd(AuDesignUndoRedoItem::Add);
	AuDesignUndoRedoItem udel(AuDesignUndoRedoItem::Delete);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsMakeLandingGear(sel, uadd.Parts(), udel.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Make Landing Gear"));
		undo.append(udel);
		undo.append(uadd);
		SetPartChangeTimesNoLock(uadd.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::MakeMesh(const AuId& archId, const AuScSelection& sel)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uadd(AuDesignUndoRedoItem::Add);
	AuDesignUndoRedoItem udel(AuDesignUndoRedoItem::Delete);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsMakeMesh(sel, uadd.Parts(), udel.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Make Mesh"));
		undo.append(udel);
		undo.append(uadd);
		SetPartChangeTimesNoLock(uadd.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::MakeRotatingBeaconLens(const AuId& archId, const AuScSelection& sel, const QColor& lightColor,
		const AuPoint& rotationAxis)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uadd(AuDesignUndoRedoItem::Add);
	AuDesignUndoRedoItem udel(AuDesignUndoRedoItem::Delete);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsMakeRotatingBeaconLens(sel, lightColor, rotationAxis, uadd.Parts(), udel.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Make Rotating Beacon"));
		undo.append(udel);
		undo.append(uadd);
		SetPartChangeTimesNoLock(uadd.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::MakeStrobeLens(const AuId& archId, const AuScSelection& sel, const QColor& lightColor)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uadd(AuDesignUndoRedoItem::Add);
	AuDesignUndoRedoItem udel(AuDesignUndoRedoItem::Delete);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsMakeStrobeLens(sel, lightColor, uadd.Parts(), udel.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Make Strobe"));
		undo.append(udel);
		undo.append(uadd);
		SetPartChangeTimesNoLock(uadd.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::MakeTurboLift(const AuId& archId, const AuScSelection& sel)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uadd(AuDesignUndoRedoItem::Add);
	AuDesignUndoRedoItem udel(AuDesignUndoRedoItem::Delete);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsMakeTurboLift(sel, uadd.Parts(), udel.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Make Turbo Lift"));
		undo.append(udel);
		undo.append(uadd);
		SetPartChangeTimesNoLock(uadd.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

bool DaDesigner::MakeTurret(const AuId& archId, const AuScSelection& sel, const AuPoint& basePivot, const AuPoint& baseAxis)
{
	if(!m_Design)
		return false;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uadd(AuDesignUndoRedoItem::Add);
	AuDesignUndoRedoItem udel(AuDesignUndoRedoItem::Delete);

	m_EnvLock.lockForWrite();
	bool didIt = m_Design->PartsMakeTurret(sel, basePivot, baseAxis, uadd.Parts(), udel.Parts());
	if(didIt)
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Make Turret"));
		undo.append(udel);
		undo.append(uadd);
		SetPartChangeTimesNoLock(uadd.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
	return didIt;
}

void DaDesigner::MakeWindow(const AuId& archId, const AuScSelection& sel, float opacity)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uadd(AuDesignUndoRedoItem::Add);
	AuDesignUndoRedoItem udel(AuDesignUndoRedoItem::Delete);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsMakeWindow(sel, opacity, uadd.Parts(), udel.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Make Window"));
		undo.append(udel);
		undo.append(uadd);
		SetPartChangeTimesNoLock(uadd.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

bool DaDesigner::SetTurretBase(const AuId& archId, const AuScSelection& sel, const AuPoint& basePivot, const AuPoint& baseAxis)
{
	if(!m_Design)
		return false;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	bool didIt = m_Design->SetTurretBase(sel, basePivot, baseAxis, uchg.Parts());
	if(didIt)
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Set Turret Base Pivot"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
	return didIt;
}

bool DaDesigner::SetTurretGun(const AuId& archId, const AuScSelection& sel, const AuPoint& gunPivot, const AuPoint& gunAxis,
		const AuPoint& gunEmitLoc, const AuPoint& gunEmitVec, const AuPoint& gunSightLoc)
{
	if(!m_Design)
		return false;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	bool didIt = m_Design->SetTurretGun(sel, gunPivot, gunAxis, gunEmitLoc, gunEmitVec, gunSightLoc, uchg.Parts());
	if(didIt)
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Set Turret Gun"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
	return didIt;
}

bool DaDesigner::SetTurretStation(const AuId& archId, const AuScSelection& sel, quint8 gunnerPos, const AuPoint& gunnerLoc,
		const AuPoint& gunnerFacing, const AuPoint& corner1, const AuPoint& corner2, const AuMatrix4x4& gridPlane,
		COMMODITYID module)
{
	if(!m_Design)
		return false;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	bool didIt = m_Design->SetTurretStation(sel, (AuScTurret::GUNNER_POSITION)gunnerPos, gunnerLoc, gunnerFacing, corner1, corner2,
			gridPlane, module, uchg.Parts());
	if(didIt)
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Set Turret Station"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
	return didIt;
}

void DaDesigner::DeletePartStop(const AuId& archId, quint16 partId, const QString& station)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartDeleteStop(partId, station, uchg.Parts()))
	{
		// Create the undo transaction.
		StartUndo(archId, tr("Delete Stop")).append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

QString DaDesigner::PartStop(quint16 partId) const
{
	if(!m_Design)
		return "";

	m_EnvLock.lockForRead();
	QString s = m_Design->PartStop(partId);
	m_EnvLock.unlock();
	return s;
}

void DaDesigner::PartStops(quint16 partId, QStringList& stations) const
{
	if(!m_Design)
		return;

	m_EnvLock.lockForRead();
	m_Design->PartStops(partId, stations);
	m_EnvLock.unlock();
}

void DaDesigner::SetPartStop(const AuId& archId, quint16 partId, const QString& station)
{
	Q_UNUSED(archId);
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartSetStop(partId, station, uchg.Parts()))
	{
		// Create the undo transaction.
		//StartUndo(archId, tr("Change Stop")).append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::AddPartStop(const AuId& archId, quint16 partId, const QString& station)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartAddStop(partId, station, uchg.Parts()))
	{
		// Create the undo transaction.
		StartUndo(archId, tr("Add Stop")).append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::EffectIntensityVertices(const AuId& archId, const AuScSelection& sel, float intensity)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartVerticesEffectIntensity(sel, intensity, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Effect Intensity Vertices"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::GlowVertices(const AuId& archId, const AuScSelection& sel, const QColor& glow)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartVerticesGlow(sel, glow, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Glow Vertices"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::PaintFaces(const AuId& archId, const AuScSelection& sel, const QColor& paint)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartFacesPaint(sel, paint, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Paint Faces"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::PaintFaces(const AuId& archId, const AuScSelection& sel, AuScFace::COLOR_THEME theme, quint8 opacity)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartFacesPaint(sel, theme, opacity, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Paint Faces"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::PolishFaces(const AuId& archId, const AuScSelection& sel, float shine, float reflectivity)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartFacesPolish(sel, shine, reflectivity, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Polish Faces"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::SetFaceSelfIllumination(const AuId& archId, const AuScSelection& sel, float selfIllum)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsSetFaceSelfIllumination(sel, selfIllum, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Set Self Illumination of Faces"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::SetFaceOpacity(const AuId& archId, const AuScSelection& sel, float opacity)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsSetFaceOpacity(sel, opacity, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Set Opacity of Faces"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::TextureMove(const AuId& archId, const AuScSelection& sel, const QPointF& vec, quint8 undoSeq)
{
	//qDebug() << "DaDesigner::TextureMove" << sel.size() << vec;
	if(!m_Design || sel.empty())
		return;
	if(vec == QPointF(0,0))
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsTextureMove(sel, vec, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Texture Move %1").arg(undoSeq));
		undo.append(uchg);
		EndUndo(archId);
	}
	m_EnvLock.unlock();

	if(!uchg.Parts().empty())
	{
		SetPartChangeTimes(uchg.Parts());
		SetDesignModified();
	}
	//qDebug() << "DaDesigner::TextureMove did any" << !uchg.Parts().empty();
}

void DaDesigner::TextureRotate(const AuId& archId, const AuScSelection& sel, const QPointF& pivot, double angle, quint8 undoSeq)
{
	//qDebug() << "DaDesigner::TextureRotate" << sel.size() << vec;
	if(!m_Design || sel.empty())
		return;
	if(angle == 0 || angle == PI*2)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsTextureRotate(sel, pivot, angle, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Texture Rotate %1").arg(undoSeq));
		undo.append(uchg);
		EndUndo(archId);
	}
	m_EnvLock.unlock();

	if(!uchg.Parts().empty())
	{
		SetPartChangeTimes(uchg.Parts());
		SetDesignModified();
	}
	//qDebug() << "DaDesigner::TextureRotate did any" << !uchg.Parts().empty();
}

void DaDesigner::TextureStretch(const AuId& archId, const AuScSelection& sel, const QPointF& base, double scaleX, double scaleY,
		quint8 undoSeq)
{
	//qDebug() << "DaDesigner::TextureStretch" << sel.size() << vec;
	if(!m_Design || sel.empty())
		return;
	if(scaleX == 1 && scaleY == 1)
		return;
	if(scaleX == 0 || scaleY == 0)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsTextureStretch(sel, base, scaleX, scaleY, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Texture Stretch %1").arg(undoSeq));
		undo.append(uchg);
		EndUndo(archId);
	}
	m_EnvLock.unlock();

	if(!uchg.Parts().empty())
	{
		SetPartChangeTimes(uchg.Parts());
		SetDesignModified();
	}
	//qDebug() << "DaDesigner::TextureStretch did any" << !uchg.Parts().empty();
}

void DaDesigner::TextureFaces(const AuId& archId, const AuScSelection& sel, SC_TEXTURE_INDEX texIndex)
{
	if(!m_Design || sel.empty())
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsTextureFaces(sel, texIndex, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Texture Faces"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::TextureFacesCylindrical(const AuId& archId, const AuScSelection& sel, SC_TEXTURE_INDEX texIndex,
		quint16 texRepsAtEquator, double texSize, const AuMatrix4x4& gridPlane)
{
	if(!m_Design || sel.empty())
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsTextureFacesCylindrical(sel, texIndex, texRepsAtEquator, texSize, gridPlane, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Texture Faces Cylindrical"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::TextureFacesProject(const AuId& archId, const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, double texSize,
		const AuMatrix4x4& gridPlane)
{
	if(!m_Design || sel.empty())
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsTextureFacesProject(sel, texIndex, texSize, gridPlane, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Texture Faces Project"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::TextureFacesSpherical(const AuId& archId, const AuScSelection& sel, SC_TEXTURE_INDEX texIndex,
		quint16 texRepsAtEquator, const AuMatrix4x4& gridPlane)
{
	if(!m_Design || sel.empty())
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsTextureFacesSpherical(sel, texIndex, texRepsAtEquator, gridPlane, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Texture Faces Spherical"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::EffectMove(const AuId& archId, const AuScSelection& sel, const QPointF& vec, quint8 undoSeq)
{
	//qDebug() << "DaDesigner::EffectMove" << sel.size() << vec;
	if(!m_Design || sel.empty())
		return;
	if(vec == QPointF(0,0))
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsEffectMove(sel, vec, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Effect Move %1").arg(undoSeq));
		undo.append(uchg);
		EndUndo(archId);
	}
	m_EnvLock.unlock();

	if(!uchg.Parts().empty())
	{
		SetPartChangeTimes(uchg.Parts());
		SetDesignModified();
	}
	//qDebug() << "DaDesigner::EffectMove did any" << !uchg.Parts().empty();
}

void DaDesigner::EffectRotate(const AuId& archId, const AuScSelection& sel, const QPointF& pivot, double angle, quint8 undoSeq)
{
	//qDebug() << "DaDesigner::EffectRotate" << sel.size() << vec;
	if(!m_Design || sel.empty())
		return;
	if(angle == 0 || angle == PI*2)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsEffectRotate(sel, pivot, angle, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Effect Rotate %1").arg(undoSeq));
		undo.append(uchg);
		EndUndo(archId);
	}
	m_EnvLock.unlock();

	if(!uchg.Parts().empty())
	{
		SetPartChangeTimes(uchg.Parts());
		SetDesignModified();
	}
	//qDebug() << "DaDesigner::EffectRotate did any" << !uchg.Parts().empty();
}

void DaDesigner::EffectStretch(const AuId& archId, const AuScSelection& sel, const QPointF& base, double scaleX, double scaleY,
		quint8 undoSeq)
{
	//qDebug() << "DaDesigner::EffectStretch" << sel.size() << vec;
	if(!m_Design || sel.empty())
		return;
	if(scaleX == 1 && scaleY == 1)
		return;
	if(scaleX == 0 || scaleY == 0)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsEffectStretch(sel, base, scaleX, scaleY, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Effect Stretch %1").arg(undoSeq));
		undo.append(uchg);
		EndUndo(archId);
	}
	m_EnvLock.unlock();

	if(!uchg.Parts().empty())
	{
		SetPartChangeTimes(uchg.Parts());
		SetDesignModified();
	}
	//qDebug() << "DaDesigner::EffectStretch did any" << !uchg.Parts().empty();
}

void DaDesigner::EffectFaces(const AuId& archId, const AuScSelection& sel, SURFACE_EFFECT effect)
{
	if(!m_Design || sel.empty())
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsEffectFaces(sel, effect, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Texture Effect Faces"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::EffectFacesCylindrical(const AuId& archId, const AuScSelection& sel, SURFACE_EFFECT effect,
		quint16 texRepsAtEquator, double texSize, const AuMatrix4x4& gridPlane)
{
	if(!m_Design || sel.empty())
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsEffectFacesCylindrical(sel, effect, texRepsAtEquator, texSize, gridPlane, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Texture Effect Faces Cylindrical"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::EffectFacesProject(const AuId& archId, const AuScSelection& sel, SURFACE_EFFECT effect, double texSize,
		const AuMatrix4x4& gridPlane)
{
	if(!m_Design || sel.empty())
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsEffectFacesProject(sel, effect, texSize, gridPlane, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Texture Effect Faces Project"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::EffectFacesSpherical(const AuId& archId, const AuScSelection& sel, SURFACE_EFFECT effect,
		quint16 texRepsAtEquator, const AuMatrix4x4& gridPlane)
{
	if(!m_Design || sel.empty())
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsEffectFacesSpherical(sel, effect, texRepsAtEquator, gridPlane, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Texture Effect Faces Spherical"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::EffectUnwrap(const AuId& archId, const AuScSelection& sel, SURFACE_EFFECT effect, double texSize,
		quint8 unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	//qDebug() << "DaDesigner::TextureUnwrap" << sel.size() << vec;
	if(!m_Design || sel.empty())
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsEffectUnwrap(sel, effect, texSize, unwrap, angleTol, gridAlign, alignAxis, gridPlane, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Texture Effect Unwrap %1").arg(unwrap));
		undo.append(uchg);
		EndUndo(archId);
	}
	m_EnvLock.unlock();

	if(!uchg.Parts().empty())
	{
		SetPartChangeTimes(uchg.Parts());
		SetDesignModified();
	}
	//qDebug() << "DaDesigner::TextureUnwrap did any" << !uchg.Parts().empty();
}

void DaDesigner::RenderDesign(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
		DaPerson* sceneAtom, bool transparentPass)
{
	if(!m_Design)
		return;

	QColor themeColors[AuScFace::THEME_Count];
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireThemeColors(sceneAtom->EmpireId(), themeColors);
	g_Au.UnlockData();

	if(m_OpaqueVAOs.empty() && m_TransparentVAOs.empty() &&
			m_DecalOpaqueVAOs.empty() && m_DecalTransparentVAOs.empty() &&
			m_OwnDecalOpaqueVAOs.empty() && m_OwnDecalTransparentVAOs.empty() &&
			m_OwnOpaqueVAOs.empty() && m_OwnTransparentVAOs.empty() &&
			m_WindowTranslucentVAOs.empty() && m_WindowTransparentVAOs.empty())
	{
		// We have no VAOs of anything.
		// Make a VAO of each part of the ship.
		const AuScParts& parts = m_Design->Parts();
		if(parts.empty())
			return;

		for(AuScParts::const_iterator pi = parts.cbegin(); pi != parts.cend(); pi++)
		{
			if((*pi)->IsHidden())
				continue;
			if((*pi)->IsVoid() || (*pi)->IsBarrier())
				continue;
			if((*pi)->IsPath())
				continue;
			if((*pi)->IsConsole())
				continue;

			if((*pi)->IsWindow())
			{
				// Window VAO.
				AuScTriRenderList translucentTris, transparentTris;
				(*pi)->RenderTriangles(translucentTris, transparentTris, themeColors);
				if(translucentTris.empty() && transparentTris.empty())
					continue;

				if(!translucentTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, translucentTris, GL_STATIC_DRAW);
					if(vao.VAO())
						m_WindowTranslucentVAOs.insert(pi.key(), vao);
				}

				if(!transparentTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, transparentTris, GL_STATIC_DRAW);
					if(vao.VAO())
						m_WindowTransparentVAOs.insert(pi.key(), vao);
				}
			}
			else if((*pi)->IsTurret())
			{
				// Turret uses regular textures for the solid model parts, own texture for the screen.
				AuScTurret* t = (AuScTurret*)(*pi)->Clone();
				{
					// Solid model parts use regular texturing.
					AuScTriRenderList opaqueTris, transparentTris;
					t->SetState(AuScTurret::Base);
					t->RenderTriangles(opaqueTris, transparentTris, themeColors);
					t->SetState(AuScTurret::Gun);
					t->RenderTriangles(opaqueTris, transparentTris, themeColors);

					if(!opaqueTris.empty())
					{
						AuScVAO vao;
						vao.Create(*ari.re, opaqueTris, GL_STATIC_DRAW);
						if(vao.VAO())
							m_OpaqueVAOs.insert(pi.key(), vao);
					}

					if(!transparentTris.empty())
					{
						AuScVAO vao;
						vao.Create(*ari.re, transparentTris, GL_STATIC_DRAW);
						if(vao.VAO())
							m_TransparentVAOs.insert(pi.key(), vao);
					}
				}

				{
					// Screen uses it's own custom texture.
					// Own VAOs.
					// Load/select the private texture.
					GLuint texId = (*pi)->OwnTexture(*ari.re);
					if(!texId)
					{
						delete t;
						continue;
					}
					m_OwnTextures.insert(pi.key(), texId);

					AuScTriRenderList opaqueTris, transparentTris;
					(*pi)->RenderTriangles(opaqueTris, transparentTris, themeColors);

					if(!opaqueTris.empty())
					{
						AuScVAO vao;
						vao.Create(*ari.re, opaqueTris, GL_STATIC_DRAW);
						if(vao.VAO())
							m_OwnOpaqueVAOs.insert(pi.key(), vao);
					}

					if(!transparentTris.empty())
					{
						AuScVAO vao;
						vao.Create(*ari.re, transparentTris, GL_STATIC_DRAW);
						if(vao.VAO())
							m_OwnTransparentVAOs.insert(pi.key(), vao);
					}
				}
				delete t;
			}
			else if((*pi)->IsDecal() && (*pi)->HasOwnTexture())
			{
				// Own Decal VAOs.
				// Load/select the private texture.
				GLuint texId = (*pi)->OwnTexture(*ari.re);
				if(!texId)
					continue;
				m_OwnTextures.insert(pi.key(), texId);

				AuScTriRenderList opaqueTris, transparentTris;
				(*pi)->RenderTriangles(opaqueTris, transparentTris, themeColors);
				if(opaqueTris.empty() && transparentTris.empty())
					continue;

				if(!opaqueTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, opaqueTris, GL_STATIC_DRAW);
					if(vao.VAO())
						m_OwnDecalOpaqueVAOs.insert(pi.key(), vao);
				}

				if(!transparentTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, transparentTris, GL_STATIC_DRAW);
					if(vao.VAO())
						m_OwnDecalTransparentVAOs.insert(pi.key(), vao);
				}
			}
			else if((*pi)->HasOwnTexture())
			{
				// Own VAOs.
				// Load/select the private texture.
				GLuint texId = (*pi)->OwnTexture(*ari.re);
				if(!texId)
					continue;
				m_OwnTextures.insert(pi.key(), texId);

				AuScTriRenderList opaqueTris, transparentTris;
				(*pi)->RenderTriangles(opaqueTris, transparentTris, themeColors);
				if(opaqueTris.empty() && transparentTris.empty())
					continue;

				if(!opaqueTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, opaqueTris, GL_STATIC_DRAW);
					if(vao.VAO())
						m_OwnOpaqueVAOs.insert(pi.key(), vao);
				}

				if(!transparentTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, transparentTris, GL_STATIC_DRAW);
					if(vao.VAO())
						m_OwnTransparentVAOs.insert(pi.key(), vao);
				}
			}
			else if((*pi)->IsDecal())
			{
				// Decal VAOs.
				AuScTriRenderList opaqueTris, transparentTris;
				(*pi)->RenderTriangles(opaqueTris, transparentTris, themeColors);
				if(opaqueTris.empty() && transparentTris.empty())
					continue;

				if(!opaqueTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, opaqueTris, GL_STATIC_DRAW);
					if(vao.VAO())
						m_DecalOpaqueVAOs.insert(pi.key(), vao);
				}

				if(!transparentTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, transparentTris, GL_STATIC_DRAW);
					if(vao.VAO())
						m_DecalTransparentVAOs.insert(pi.key(), vao);
				}
			}
			else
			{
				// Routine VAOs.
				AuScTriRenderList opaqueTris, transparentTris;
				(*pi)->RenderTriangles(opaqueTris, transparentTris, themeColors);
				if(opaqueTris.empty() && transparentTris.empty())
					continue;

				if(!opaqueTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, opaqueTris, GL_STATIC_DRAW);
					if(vao.VAO())
						m_OpaqueVAOs.insert(pi.key(), vao);
				}

				if(!transparentTris.empty())
				{
					AuScVAO vao;
					vao.Create(*ari.re, transparentTris, GL_STATIC_DRAW);
					if(vao.VAO())
						m_TransparentVAOs.insert(pi.key(), vao);
				}
			}
		}
	}

	// Don't continue if we have nothing to draw in this pass.
	if(transparentPass)
	{
		if(m_TransparentVAOs.empty() &&
				m_DecalTransparentVAOs.empty() &&
				m_OwnDecalTransparentVAOs.empty() &&
				m_OwnTransparentVAOs.empty() &&
				m_WindowTransparentVAOs.empty())
			return;	// No transparent VAOs to draw.
	}
	else
	{
		if(m_OpaqueVAOs.empty() &&
				m_DecalOpaqueVAOs.empty() &&
				m_OwnDecalOpaqueVAOs.empty() &&
				m_OwnOpaqueVAOs.empty() &&
				m_WindowTranslucentVAOs.empty())
			return;	// No opaque VAOs to draw.
	}

	// Get/load the texture ids of the design's texture palette.
	AuTexturePalette surfaceTex;
	m_Design->ShaderTexturePalette(m_Design->HullModule(), *ari.re, surfaceTex);

	ari.re->GLSL().ScShader().Select(projection, *ari.re);
	ari.re->GLSL().ScShader().SetGlowDiscard(0,0,0, .01f);
	ari.re->GLSL().ScShader().SetVertAlpha(1);
	ari.re->GLSL().ScShader().SetSurfaceTextures(surfaceTex);

	// Ambient light
	float ambientR = AuGLLight::MaxAmbientStarlight();
	float ambientG = AuGLLight::MaxAmbientStarlight();
	float ambientB = AuGLLight::MaxAmbientStarlight();
	AuPoint ambientUp(0,0,1);
	ari.re->GLSL().ScShader().SetAmbientLight(ambientR, ambientG, ambientB);
	ari.re->GLSL().ScShader().SetAmbientLightUp(ambientUp);

	if(transparentPass)
	{
		ari.re->glEnable(GL_BLEND);
		ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Render non-window parts that use the texture palette that we have configured.
		RenderDesignVAOs(m_TransparentVAOs, model, ari, themeColors, true, false);

		// Render windows; they use the texture palette that we have already configured.
		// Windows are affected by internal lighting conditions.
		RenderDesignVAOs(m_WindowTransparentVAOs, model, ari, themeColors, true, false);

		// Render decals that use the texture palette that we have configured.
		if(!m_DecalTransparentVAOs.empty())
		{
			// Decals are always blended; it's already enabled in transparent pass.
			// Depth test is changed to enable coplanar render of decal. We'll see if we have to resort to polygon offset.
			ari.re->glEnable(GL_POLYGON_OFFSET_FILL);
			ari.re->glPolygonOffset(-1.f, -1.f);
			ari.re->glDepthFunc(GL_LEQUAL);

			RenderDesignVAOs(m_DecalTransparentVAOs, model, ari, themeColors, true, false);

			ari.re->glDepthFunc(GL_LESS);
			ari.re->glDisable(GL_POLYGON_OFFSET_FILL);
		}

		// Render parts that have their own textures.
		// So far, own texture things are icons, screens and sketches. We like to see them from the back too, mainly the sketches.
		ari.re->glDisable(GL_CULL_FACE);
		RenderDesignVAOs(m_OwnTransparentVAOs, model, ari, themeColors, true, true);
		ari.re->glEnable(GL_CULL_FACE);

		// Render decals that have their own textures.
		if(!m_OwnDecalTransparentVAOs.empty())
		{
			// Decals are always blended; it's already enabled in transparent pass.
			// Depth test is changed to enable coplanar render of decal. We'll see if we have to resort to polygon offset.
			ari.re->glEnable(GL_POLYGON_OFFSET_FILL);
			ari.re->glPolygonOffset(-1.f, -1.f);
			ari.re->glDepthFunc(GL_LEQUAL);

			RenderDesignVAOs(m_OwnDecalTransparentVAOs, model, ari, themeColors, true, true);

			ari.re->glDepthFunc(GL_LESS);
			ari.re->glDisable(GL_POLYGON_OFFSET_FILL);
		}

		ari.re->glDisable(GL_BLEND);
	}
	else
	{
		// Render non-window parts that use the texture palette that we have configured.
		RenderDesignVAOs(m_OpaqueVAOs, model, ari, themeColors, false, false);

		// Render windows; they use the texture palette that we have already configured.
		// Windows are affected by internal lighting conditions.
		RenderDesignVAOs(m_WindowTranslucentVAOs, model, ari, themeColors, false, false);

		// Render decals that use the texture palette that we have configured.
		if(!m_DecalOpaqueVAOs.empty())
		{
			// Decals are always blended.
			// Depth test is changed to enable coplanar render of decal. We'll see if we have to resort to polygon offset.
			ari.re->glEnable(GL_BLEND);
			ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			ari.re->glEnable(GL_POLYGON_OFFSET_FILL);
			ari.re->glPolygonOffset(-1.f, -1.f);
			ari.re->glDepthFunc(GL_LEQUAL);

			RenderDesignVAOs(m_DecalOpaqueVAOs, model, ari, themeColors, false, false);

			ari.re->glDepthFunc(GL_LESS);
			ari.re->glDisable(GL_POLYGON_OFFSET_FILL);
			ari.re->glDisable(GL_BLEND);
		}

		// Render parts that have their own textures.
		// So far, own texture things are icons, screens and sketches. We like to see them from the back too, mainly the sketches.
		ari.re->glDisable(GL_CULL_FACE);
		RenderDesignVAOs(m_OwnOpaqueVAOs, model, ari, themeColors, false, true);
		ari.re->glEnable(GL_CULL_FACE);

		// Render decals that have their own textures.
		if(!m_OwnDecalOpaqueVAOs.empty())
		{
			// Decals are always blended.
			// Depth test is changed to enable coplanar render of decal. We'll see if we have to resort to polygon offset.
			ari.re->glEnable(GL_BLEND);
			ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			ari.re->glEnable(GL_POLYGON_OFFSET_FILL);
			ari.re->glPolygonOffset(-1.f, -1.f);
			ari.re->glDepthFunc(GL_LEQUAL);

			RenderDesignVAOs(m_OwnDecalOpaqueVAOs, model, ari, themeColors, false, true);

			ari.re->glDepthFunc(GL_LESS);
			ari.re->glDisable(GL_POLYGON_OFFSET_FILL);
			ari.re->glDisable(GL_BLEND);
		}
	}

	ari.re->GLSL().ScShader().Deselect();

	// Render consoles.
	for(AuScParts::const_iterator pi = m_Design->Parts().cbegin(); pi != m_Design->Parts().cend(); pi++)
	{
		if((*pi)->IsHidden())
			continue;
		if(ari.DesignerObjectMode() && ari.DesignerObjectIsolate() && !ari.DesignerPartObjectSelection().contains(pi.key()))
			continue;	// Skip unselected part in isolated object mode.

		switch((*pi)->Type())
		{
		default:
			break;

		case AuScPart::Part_ConsoleCommand:
			{
				// Position the model according to up in the room.
				AuMatrix4x4 tcon(model * (*pi)->PositionMatrix());
				m_CmdEmpty.Render(projection, projection, tcon, *sceneAtom, 0, ari);
			}
			break;

		case AuScPart::Part_ConsoleEngineer:
			{
				// Position the model according to up in the room.
				AuMatrix4x4 tcon(model * (*pi)->PositionMatrix());
				m_EngineEmpty.Render(projection, projection, tcon, *sceneAtom, 0, ari);
			}
			break;

		case AuScPart::Part_ConsoleFireControl:
			{
				// Position the model according to up in the room.
				AuMatrix4x4 tcon(model * (*pi)->PositionMatrix());
				m_FireEmpty.Render(projection, projection, tcon, *sceneAtom, 0, ari);
			}
			break;

		case AuScPart::Part_ConsoleHelm:
			{
				// Position the model according to up in the room.
				AuMatrix4x4 tcon(model * (*pi)->PositionMatrix());
				m_PilotEmpty.Render(projection, projection, tcon, *sceneAtom, 0, ari);
			}
			break;

		case AuScPart::Part_ConsoleMedic:
			{
				// Position the model according to up in the room.
				AuMatrix4x4 tcon(model * (*pi)->PositionMatrix());
				m_SurgeryEmpty.Render(projection, projection, tcon, *sceneAtom, 0, ari);
			}
			break;

		case AuScPart::Part_ConsoleNavigator:
			{
				// Position the model according to up in the room.
				AuMatrix4x4 tcon(model * (*pi)->PositionMatrix());
				m_NavEmpty.Render(projection, projection, tcon, *sceneAtom, 0, ari);
			}
			break;

		case AuScPart::Part_ConsolePowerRelay:
			{
				// Position the model according to up in the room.
				AuMatrix4x4 tcon(model * (*pi)->PositionMatrix());
				m_PowerEmpty.Render(projection, projection, tcon, *sceneAtom, 0, ari);
			}
			break;

		case AuScPart::Part_ConsoleSensor:
			{
				// Position the model according to up in the room.
				AuMatrix4x4 tcon(model * (*pi)->PositionMatrix());
				m_SensorEmpty.Render(projection, projection, tcon, *sceneAtom, 0, ari);
			}
			break;

		case AuScPart::Part_ConsoleShield:
			{
				// Position the model according to up in the room.
				AuMatrix4x4 tcon(model * (*pi)->PositionMatrix());
				m_ShieldEmpty.Render(projection, projection, tcon, *sceneAtom, 0, ari);
			}
			break;

		case AuScPart::Part_ConsoleTransporter:
			{
				// Position the model according to up in the room.
				AuMatrix4x4 tcon(model * (*pi)->PositionMatrix());
				m_TransEmpty.Render(projection, projection, tcon, *sceneAtom, 0, ari);
			}
			break;
		}
	}
}

void DaDesigner::RenderDesignVoids(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
		DaPerson* sceneAtom)
{
	Q_UNUSED(projection);
	if(!m_Design)
		return;

	// Make a temporary VAO of each void of the design.
	const AuScParts& parts = m_Design->Parts();
	if(parts.empty())
		return;

	QColor themeColors[AuScFace::THEME_Count];
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireThemeColors(sceneAtom->EmpireId(), themeColors);
	g_Au.UnlockData();

	QHash<quint16, AuScVAO> opaqueVAOs, transparentVAOs;

	for(AuScParts::const_iterator pi = parts.cbegin(); pi != parts.cend(); pi++)
	{
		if((*pi)->IsHidden())
			continue;
		if(!(*pi)->IsVoid() && !(*pi)->IsBarrier())
			continue;
		// We can skip unwanted parts here since we remake these in every pass?
		if(ari.DesignerObjectMode() && ari.DesignerObjectIsolate() && !ari.DesignerPartObjectSelection().contains(pi.key()))
			continue;	// Skip unselected part in isolated object mode.

		quint16 partId = (*pi)->AssociatedPartId();
		if(partId)
		{
			quint8 partState = (*pi)->AssociatedPartState();
			AuScParts::const_iterator api = parts.find(partId);
			if(api != parts.cend() && (*api)->State() != partState)
				continue;	// Skip when associated with a part that is in a different state.
		}

		// Routine VAOs.
		AuScTriRenderList opaqueTris, transparentTris;
		(*pi)->RenderTriangles(opaqueTris, transparentTris, themeColors);
		if(opaqueTris.empty() && transparentTris.empty())
			continue;

		if(!opaqueTris.empty())
		{
			AuScVAO vao;
			vao.Create(*ari.re, opaqueTris, GL_STATIC_DRAW);
			if(vao.VAO())
				opaqueVAOs.insert(pi.key(), vao);
		}

		if(!transparentTris.empty())
		{
			AuScVAO vao;
			vao.Create(*ari.re, transparentTris, GL_STATIC_DRAW);
			if(vao.VAO())
				transparentVAOs.insert(pi.key(), vao);
		}
	}

	if(opaqueVAOs.empty() && transparentVAOs.empty())
		return;

	// Get/load the texture ids of the design's texture palette.
	AuTexturePalette surfaceTex;
	m_Design->ShaderTexturePalette(m_Design->HullModule(), *ari.re, surfaceTex);

	ari.re->GLSL().ScShader().Select(projection, *ari.re);
	ari.re->GLSL().ScShader().SetGlowDiscard(0,0,0, .01f);
	ari.re->GLSL().ScShader().SetVertAlpha(1);
	ari.re->GLSL().ScShader().SetSurfaceTextures(surfaceTex);

	// Ambient light
	float ambientR = AuGLLight::MaxAmbientStarlight();
	float ambientG = AuGLLight::MaxAmbientStarlight();
	float ambientB = AuGLLight::MaxAmbientStarlight();
	AuPoint ambientUp(0,0,1);
	ari.re->GLSL().ScShader().SetAmbientLight(ambientR, ambientG, ambientB);
	ari.re->GLSL().ScShader().SetAmbientLightUp(ambientUp);

	RenderDesignVAOs(opaqueVAOs, model, ari, themeColors, false, false);

	ari.re->glEnable(GL_BLEND);
	ari.re->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	RenderDesignVAOs(transparentVAOs, model, ari, themeColors, true, false);
	ari.re->glDisable(GL_BLEND);

	ari.re->GLSL().ScShader().Deselect();

	for(QHash<quint16, AuScVAO>::iterator vi = opaqueVAOs.begin(); vi != opaqueVAOs.end(); vi++)
		(*vi).DeleteGL();
	for(QHash<quint16, AuScVAO>::iterator vi = transparentVAOs.begin(); vi != transparentVAOs.end(); vi++)
		(*vi).DeleteGL();
}

void DaDesigner::RenderDesignPaths(const AuMatrix4x4& projection, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
		DaPerson* sceneAtom)
{
	Q_UNUSED(sceneAtom);
	if(!m_Design)
		return;

	const AuScParts& parts = m_Design->Parts();
	if(parts.empty())
		return;

	// Collect path segments.
	AuPointArray walkSegs, ladderSegs;
	AuPointArray walkSegsSel, ladderSegsSel;
	for(AuScParts::const_iterator pi = parts.cbegin(); pi != parts.cend(); pi++)
	{
		if((*pi)->IsHidden())
			continue;
		if(!(*pi)->IsPath())
			continue;
		if(ari.DesignerObjectMode() && ari.DesignerObjectIsolate() && !ari.DesignerPartObjectSelection().contains(pi.key()))
			continue;	// Skip unselected part in isolated object mode.

		quint16 partId = (*pi)->AssociatedPartId();
		if(partId)
		{
			quint8 partState = (*pi)->AssociatedPartState();
			AuScParts::const_iterator api = parts.find(partId);
			if(api != parts.cend() && (*api)->State() != partState)
				continue;	// Skip when associated with a part that is in a different state.
		}

		AuSegment seg((*pi)->PathSegment(0).m_Seg);
		if((*pi)->IsLadderPath())
		{
			if(ari.DesignerPartObjectSelection().contains(pi.key()))
			{
				ladderSegsSel.append(seg.m_fLoc1);
				ladderSegsSel.append(seg.m_fLoc2);
			}
			else
			{
				ladderSegs.append(seg.m_fLoc1);
				ladderSegs.append(seg.m_fLoc2);
			}
		}
		else
		{
			if(ari.DesignerPartObjectSelection().contains(pi.key()))
			{
				walkSegsSel.append(seg.m_fLoc1);
				walkSegsSel.append(seg.m_fLoc2);
			}
			else
			{
				walkSegs.append(seg.m_fLoc1);
				walkSegs.append(seg.m_fLoc2);
			}
		}
	}

	if(ladderSegs.empty() && walkSegs.empty() && ladderSegsSel.empty() && walkSegsSel.empty())
		return;

	AuMatrix4x4 pmv = projection * model;
	ari.re->GLSL().LineShader().Select();
	ari.re->GLSL().LineShader().SetProjectionModelView(pmv);
	//ari.re->glLineWidth(2.5);
	ari.re->glEnable(GL_LINE_SMOOTH);	// This makes them look thicker.

	if(!ladderSegs.empty())
	{
		AuGLVAO vao;
		vao.CreateForLines(*ari.re, ladderSegs, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			// Display the VAO once and clean it up.
			QColor color(g_Au.DesignerLadderPathColor());
			ari.re->GLSL().LineShader().SetColor(color);
			vao.SelectGL();
			vao.DrawGL(GL_LINES);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}

	if(!walkSegs.empty())
	{
		AuGLVAO vao;
		vao.CreateForLines(*ari.re, walkSegs, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			// Display the VAO once and clean it up.
			QColor color(g_Au.DesignerWalkPathColor());
			ari.re->GLSL().LineShader().SetColor(color);
			vao.SelectGL();
			vao.DrawGL(GL_LINES);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}

	if(!ladderSegsSel.empty())
	{
		AuGLVAO vao;
		vao.CreateForLines(*ari.re, ladderSegsSel, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			// Display the VAO once and clean it up.
			QColor color(g_Au.DesignerSelColor());
			ari.re->GLSL().LineShader().SetColor(color);
			vao.SelectGL();
			vao.DrawGL(GL_LINES);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}

	if(!walkSegsSel.empty())
	{
		AuGLVAO vao;
		vao.CreateForLines(*ari.re, walkSegsSel, false, GL_STREAM_DRAW);
		if(vao.VAO())
		{
			// Display the VAO once and clean it up.
			QColor color(g_Au.DesignerSelColor());
			ari.re->GLSL().LineShader().SetColor(color);
			vao.SelectGL();
			vao.DrawGL(GL_LINES);
			vao.DeselectGL();
			vao.DeleteGL();
		}
	}

	ari.re->glDisable(GL_LINE_SMOOTH);
	ari.re->GLSL().LineShader().Deselect();

	// Put a sprite at the center of each segment.
	// Create a temporary vao for the icon.
	AuGLVAO vao;
	{
		AuTriTex tri0;
		tri0.m_V0 = AuPointTex(-.5,-.5,0, 0,0);
		tri0.m_V1 = AuPointTex(-.5,.5,0, 0,1);
		tri0.m_V2 = AuPointTex(.5,.5,0, 1,1);
		AuTriTex tri1;
		tri1.m_V0 = AuPointTex(.5,.5,0, 1,1);
		tri1.m_V1 = AuPointTex(.5,-.5,0, 1,0);
		tri1.m_V2 = AuPointTex(-.5,-.5,0, 0,0);
		QList<AuTriTex> tris;
		tris << tri0 << tri1;
		vao.CreateForTexture(*ari.re, tris, GL_STATIC_DRAW);
	}

	// Configure render parameters.
	ari.re->glEnable(GL_BLEND);
	ari.re->glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
	ari.re->GLSL().TextureShader().Select();
	ari.re->GLSL().TextureShader().SetColor(1,1,1,1);

	if(!walkSegs.empty() && !walkSegsSel.empty())
	{
		GLuint tex = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "WalkPath", ":data/ScWalkPath.png");
		ari.re->GLSL().TextureShader().SetTexture(tex);

		vao.SelectGL();
		for(AuPointArray::const_iterator si = walkSegs.cbegin(); si != walkSegs.cend(); si++)
		{
			// Calculate center of icon.
			AuPoint p = *si;
			si++;
			AuPoint c = (p + *si)/2;

			// Set projection matrix to put the icon there.
			AuMatrix4x4 im(model);
			im.Translate(c);
			ari.re->GLSL().TextureShader().SetProjectionModelView(projection * im);

			// Draw the icon.
			vao.DrawGL();
		}
		for(AuPointArray::const_iterator si = walkSegsSel.cbegin(); si != walkSegsSel.cend(); si++)
		{
			// Calculate center of icon.
			AuPoint p = *si;
			si++;
			AuPoint c = (p + *si)/2;

			// Set projection matrix to put the icon there.
			AuMatrix4x4 im(model);
			im.Translate(c);
			ari.re->GLSL().TextureShader().SetProjectionModelView(projection * im);

			// Draw the icon.
			vao.DrawGL();
		}
	}

	if(!ladderSegs.empty() && !ladderSegsSel.empty())
	{
		GLuint tex = ari.re->CreateTextureFromBitmap(STATIC_TEXTURE_ID, "LadderPath", ":data/ScLadderPath.png");
		ari.re->GLSL().TextureShader().SetTexture(tex);

		vao.SelectGL();
		for(AuPointArray::const_iterator si = ladderSegs.cbegin(); si != ladderSegs.cend(); si++)
		{
			// Calculate center of icon.
			AuPoint p = *si;
			si++;
			AuPoint c = (p + *si)/2;

			// Set projection matrix to put the icon there.
			AuMatrix4x4 im(model);
			im.Translate(c);
			ari.re->GLSL().TextureShader().SetProjectionModelView(projection * im);

			// Draw the icon.
			vao.DrawGL();
		}
		for(AuPointArray::const_iterator si = ladderSegsSel.cbegin(); si != ladderSegsSel.cend(); si++)
		{
			// Calculate center of icon.
			AuPoint p = *si;
			si++;
			AuPoint c = (p + *si)/2;

			// Set projection matrix to put the icon there.
			AuMatrix4x4 im(model);
			im.Translate(c);
			ari.re->GLSL().TextureShader().SetProjectionModelView(projection * im);

			// Draw the icon.
			vao.DrawGL();
		}
	}

	ari.re->GLSL().TextureShader().Deselect();
	ari.re->glDisable(GL_BLEND);

	vao.DeselectGL();
	vao.DeleteGL();
}

void DaDesigner::RenderDesignVAOs(QHash<quint16, AuScVAO>& vaos, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
		const QColor themeColors[AuScFace::THEME_Count], bool transparentPass, bool ownTextures)
{
	if(ari.m_DesignerShowFaces)
		RenderDesignVAOs(vaos, model, ari, themeColors, transparentPass, ownTextures, false);

	if(ari.m_DesignerShowBacks)
	{
		// Render back faces.
		ari.re->glFrontFace(GL_CCW);
		RenderDesignVAOs(vaos, model, ari, themeColors, transparentPass, ownTextures, true);
		ari.re->glFrontFace(GL_CW);
	}
}

void DaDesigner::RenderDesignVAOs(QHash<quint16, AuScVAO>& vaos, const AuMatrix4x4& model, AuAtomRenderInfo& ari,
		const QColor themeColors[AuScFace::THEME_Count], bool transparentPass, bool ownTextures, bool isBack)
{
	if(vaos.empty())
		return;

	ari.re->GLSL().ScShader().SetModelView(model);
	if(isBack)
		ari.re->GLSL().ScShader().SetGlowDiscard(g_Au.DesignerBackColor(), .01f);
	else
		ari.re->GLSL().ScShader().SetGlowDiscard(0,0,0, .01f);

	if(!ari.DesignerHasPartSelection())
	{
		// No parts are selected. Display all part VAOs. This also implies that we are not in object mode.
		// Render solid parts.
		for(QHash<quint16, AuScVAO>::iterator vi = vaos.begin(); vi != vaos.end(); vi++)
		{
			if(ownTextures)
			{
				QHash<quint16, GLuint>::const_iterator ti = m_OwnTextures.find(vi.key());
				if(*ti)
					ari.re->GLSL().ScShader().SetSurfaceTexture(0, *ti);
			}

			(*vi).DrawSc(ari.re->GLSL().ScShader(), EffectTextures(), 2);
		}
	}
	else
	{
		// One or more parts are selected.
		// When selected parts are being dragged for a copy, we draw both if they will appear in two different locations.
		bool copyingSelection = ari.IsDragCopying() && !ari.m_DragT.IsIdentity();

		// Render unselected parts, if not in isolated object mode.
		if(!ari.DesignerObjectMode() || !ari.DesignerObjectIsolate())
		{
			for(QHash<quint16, AuScVAO>::iterator vi = vaos.begin(); vi != vaos.end(); vi++)
			{
				if(!copyingSelection && ari.DesignerPartObjectSelection().contains(vi.key()))
					continue;	// Skip selected part.

				if(ownTextures)
				{
					QHash<quint16, GLuint>::const_iterator ti = m_OwnTextures.find(vi.key());
					if(*ti)
						ari.re->GLSL().ScShader().SetSurfaceTexture(0, *ti);
				}

				(*vi).DrawSc(ari.re->GLSL().ScShader(), EffectTextures(), 2);
			}
		}

		if(ari.DesignerObjectMode())
		{
			// Render selected parts in normal color. The face grips indicate highlighting.
			// Dragging in object mode means individual faces of selected parts can be moving or dragging.
			if(ari.IsDragging())
			{
				// Render selected parts.
				for(AuScParts::const_iterator pi = m_Design->Parts().cbegin(); pi != m_Design->Parts().cend(); pi++)
				{
					if((*pi)->HasOwnTexture() != ownTextures)
						continue;

					AuScSelection::const_iterator si = ari.DesignerPartObjectSelection().find(pi.key());
					if(si == ari.DesignerPartObjectSelection().cend())
						continue;	// Skip unselected part.

					// Make a clone that we can transform.
					AuScPart* c = (*pi)->Clone();
					c->Transform(ari.m_DragT, *si);

					// Display a temporary VAO of the clone.
					AuScTriRenderList opaqueTris, transparentTris;
					c->RenderTriangles(opaqueTris, transparentTris, themeColors);
					if(transparentPass)
					{
						if(!transparentTris.empty())
						{
							AuScVAO vao;
							vao.Create(*ari.re, transparentTris, GL_STREAM_DRAW);
							if(vao.VAO())
							{
								if(ownTextures)
								{
									QHash<quint16, GLuint>::const_iterator ti = m_OwnTextures.find(pi.key());
									if(*ti)
										ari.re->GLSL().ScShader().SetSurfaceTexture(0, *ti);
								}

								vao.DrawSc(ari.re->GLSL().ScShader(), EffectTextures(), 2);
								vao.DeleteGL();
							}
						}
					}
					else
					{
						if(!opaqueTris.empty())
						{
							AuScVAO vao;
							vao.Create(*ari.re, opaqueTris, GL_STREAM_DRAW);
							if(vao.VAO())
							{
								if(ownTextures)
								{
									QHash<quint16, GLuint>::const_iterator ti = m_OwnTextures.find(pi.key());
									if(*ti)
										ari.re->GLSL().ScShader().SetSurfaceTexture(0, *ti);
								}

								vao.DrawSc(ari.re->GLSL().ScShader(), EffectTextures(), 2);
								vao.DeleteGL();
							}
						}
					}

					// Cleanup the clone.
					delete c;
				}
			}
			else
			{
				// Render selected parts in normal color. The grips indicate highlighting.
				for(QHash<quint16, AuScVAO>::iterator vi = vaos.begin(); vi != vaos.end(); vi++)
				{
					if(!ari.DesignerPartObjectSelection().contains(vi.key()))
						continue;	// Skip unselected part.

					if(ownTextures)
					{
						QHash<quint16, GLuint>::const_iterator ti = m_OwnTextures.find(vi.key());
						if(*ti)
							ari.re->GLSL().ScShader().SetSurfaceTexture(0, *ti);
					}

					(*vi).DrawSc(ari.re->GLSL().ScShader(), EffectTextures(), 2);
				}
			}
		}
		else
		{
			// Render selected parts with highlight glow, at drag position.
			QColor selColor(g_Au.DesignerSelColor());
			ari.re->GLSL().ScShader().SetGlowDiscard(selColor.redF(), selColor.greenF(), selColor.blueF(), .01f);
			ari.re->GLSL().ScShader().SetModelView(model * ari.m_DragT);
			for(QHash<quint16, AuScVAO>::iterator vi = vaos.begin(); vi != vaos.end(); vi++)
			{
				if(!ari.DesignerPartObjectSelection().contains(vi.key()))
					continue;	// Skip unselected part.

				if(ownTextures)
				{
					QHash<quint16, GLuint>::const_iterator ti = m_OwnTextures.find(vi.key());
					if(*ti)
						ari.re->GLSL().ScShader().SetSurfaceTexture(0, *ti);
				}

				(*vi).DrawSc(ari.re->GLSL().ScShader(), EffectTextures(), 2);
			}
		}
	}
}

void DaDesigner::SetTexturePalette(const AuDesignTex texturePalette[SCTEXI_Count])
{
	if(!m_Design)
		return;
	m_EnvLock.lockForWrite();
	m_Design->SetTexturePalette(texturePalette);
	m_DesignChangeTimes.insert(0, AuElapsedTimer::Now());	// id 0 is the record itself.
	m_EnvLock.unlock();
	SetDesignModified();
}

void DaDesigner::TexturePalette(AuDesignTex texturePalette[SCTEXI_Count]) const
{
	if(!m_Design)
		return;
	m_Design->TexturePalette(texturePalette);
}

void DaDesigner::TextureUnwrap(const AuId& archId, const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, double texSize,
		quint8 unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	//qDebug() << "DaDesigner::TextureUnwrap" << sel.size() << vec;
	if(!m_Design || sel.empty())
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsTextureUnwrap(sel, texIndex, texSize, unwrap, angleTol, gridAlign, alignAxis, gridPlane, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Texture Unwrap %1").arg(unwrap));
		undo.append(uchg);
		EndUndo(archId);
	}
	m_EnvLock.unlock();

	if(!uchg.Parts().empty())
	{
		SetPartChangeTimes(uchg.Parts());
		SetDesignModified();
	}
	//qDebug() << "DaDesigner::TextureUnwrap did any" << !uchg.Parts().empty();
}

void DaDesigner::RefreshObstructions()
{
	if(!m_Design)
		return;

	QProgressDialog progress(tr("Refreshing Obstructions..."), tr("Cancel Refresh"), 0, 1);
	progress.setWindowModality(Qt::WindowModal);
	progress.setMinimumDuration(0);
	progress.setValue(0);

	m_Design->RefreshObstructionsCache(progress);

	progress.cancel();
}

bool DaDesigner::RefreshObstructionsIfNeeded()
{
	if(!m_Design)
		return false;
	return m_Design->RefreshObstructionsCacheIfNeeded();
}

void DaDesigner::AssociateParts(const AuId& archId, const AuScSelection& partIds)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsAssociate(partIds, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Associate Parts"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::DissociateParts(const AuId& archId, const AuScSelection& partIds)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsDissociate(partIds, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Dissociate Parts"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::GetAtomEnvironment(AuQueryEnvironment& qe, const DaAtom* atom) const
{
	// Atmosphere is always favorable inside design environment.
	qe.m_Environment |= EB_Design;
	if(IsPreview())
		qe.m_Environment |= EB_DesignPreview;
	const DaStar* star = FindStar();
	if(star)
		return;							// Do no more when we have created a preview solar system.

	qe.m_AtmType = ATMTYPE_Standard;
	if(qe.IsInSpacecraft())
	{
		qe.m_fAirDensity = .5f;
		qe.m_Environment |= EB_Atmosphere;
		qe.m_Environment |= EB_Breathable;
		return;
	}

	// Atom is not inside the preview spacecraft.
	qe.m_GravityNormal = atom->Transform().UnProjectVector(AuPoint(0,0,1));
	qe.m_terNormal = qe.m_GravityNormal;
	if(atom == FindVessel())
	{
		// Atom is the preview spacecraft. Air density makes it fly slow.
		qe.m_fAirDensity = 0;
	}
	else
	{
		// Atom is not the preview spacecraft. Air is breathable.
		qe.m_fAirDensity = .5f;
		qe.m_Environment |= EB_Atmosphere;
		qe.m_Environment |= EB_Breathable;
		if(IsGravityOn(atom->Id()))
		{
			// The design tells us information about gravity and the floor, considering different gravity orientations.
			AuPoint floorLoc, gravityN, terN;
			m_EnvLock.lockForRead();
			GRAVITY grav = m_Design->QueryGravity(atom->Location(), floorLoc, gravityN, terN);
			m_EnvLock.unlock();
			if(grav != G_Count)
			{
				// 'loc' is inside the obstruction model.
				qe.m_terNormal = atom->Transform().UnProjectVector(terN);		// Surface normal at floor, relative to the atom.
				switch(grav)
				{
				case G_Natural:
					qe.m_Environment |= EB_Gravity;
					gravityN = AuPoint(0,0,1);									// Gravity normal, relative to the environ.
					break;

				case G_Artificial:
					qe.m_Environment |= EB_Gravity;
					qe.m_GravityNormal = atom->Transform().UnProjectVector(gravityN);	// Gravity normal, relative to the atom.
					break;

				case G_Zero:
					gravityN = AuPoint(0,0,1);									// Gravity normal, relative to the environ.
					break;

				case G_Count:
					break;
				}

				// Fall to the height my CG wants to be above the floor.
				// Positive is down, negative is up.
				qe.m_FallDist = (atom->Location() - floorLoc).Dot(gravityN) - (atom->GroundHeightAboveCG() - atom->CGOffset().z);
				//qDebug() << "qe.m_FallDist" << qe.m_FallDist;
				if(qe.m_FallDist < d_GroundTolerance)
					qe.m_OnGround = OG_HardFloor;								// We are touching or below the ground.
			}
		}
	}
}

void DaDesigner::SetRoomProperties(const AuId& archId, const AuScSelection& partIds, bool isSunlit, INTERIOR_LIGHT initLighting)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsSetRoomProperties(partIds, isSunlit, initLighting, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Set Room Properties"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::SetDoorProperties(const AuId& archId, const AuScSelection& partIds, Qt::CheckState isAutomatic,
		Qt::CheckState initLocked, Qt::CheckState remoteSpacecraft, Qt::CheckState remoteVehicle)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsSetDoorProperties(partIds, isAutomatic, initLocked, remoteSpacecraft, remoteVehicle, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Set Door Properties"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::SetSiteProperties(const AuId& archId, const AuScSelection& partIds, quint8 grade, quint8 grass, quint8 pavement,
		quint8 rocks, quint8 shrubs, quint8 terBlend, quint8 trees)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsSetSiteProperties(partIds, grade, grass, pavement, rocks, shrubs, terBlend, trees, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Set Site Properties"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::SetLightBulbProperties(const AuId& archId, const AuScSelection& partIds, const QColor& color, float kL, float kQ,
		float cosOuterCone, float cosInnerCone)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsSetLightBulbProperties(partIds, color, kL, kQ, cosOuterCone, cosInnerCone, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Set Light Bulb Properties"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::SetLightLensProperties(const AuId& archId, const AuScSelection& partIds, const QColor& color)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsSetLightLensProperties(partIds, color, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Set Light Lens Properties"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::SetVoidProperties(const AuId& archId, const AuScSelection& partIds, GRAVITY gravityType, const AuPoint& gravityUp,
		HOLD_ACCESS holdAccess, quint8 personnelAccess)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsSetVoidProperties(partIds, gravityType, gravityUp, holdAccess, personnelAccess, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Set Void Properties"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::UpdateHullDoors(const AuId& archId, const AuScSelection& partIds)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsUpdateHullDoors(partIds, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Update Hull Doors"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

bool DaDesigner::HasSealedParts(const AuId& architectId) const
{
	if(!m_Design)
		return false;

	m_EnvLock.lockForRead();
	bool hasSealedParts = m_Design->HasSealedParts(architectId);
	m_EnvLock.unlock();
	return hasSealedParts;
}

bool DaDesigner::HasSealedParts(const AuId& architectId, const AuScSelection& sel) const
{
	if(!m_Design)
		return false;

	m_EnvLock.lockForRead();
	bool hasSealedParts = m_Design->HasSealedParts(architectId, sel);
	m_EnvLock.unlock();
	return hasSealedParts;
}

void DaDesigner::GoToSelection(const AuId& archId, const AuScSelection& sel)
{
	if(archId.IsNull())
		return;
	if(sel.empty())
		return;

	// Determine the center point of the selected parts.
	m_EnvLock.lockForRead();
	AuPoint selCenter = m_Design->Parts().CenterOfSelection(sel);
	m_EnvLock.unlock();

	DaPerson* person = dynamic_cast<DaPerson*>(FindIdRecursive(archId));
	if(!person)
		return;

	AuMatrix4x4 t;
	t.Translate(selCenter);
	person->ForcePosStop(AuAtomPos(Id(), t));
	person->SetModified();
}

void DaDesigner::MoveDoorSwitches(const AuId& archId, const AuScSelection& partIds, const AuPoint& switch1, const AuPoint& switch2)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartsMoveDoorSwitches(partIds, switch1, switch2, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Move Door Switches"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::TransporterPadLocs(const AuId& archId, const AuScSelection& partIds, const AuPointArray& padLocs)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->SetTransporterPadLocs(partIds, padLocs, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Transporter Pad Locations"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::WeaponEmitLoc(const AuId& archId, const AuScSelection& partIds, const AuPoint& emitLoc)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->SetWeaponEmitLoc(partIds, emitLoc, uchg.Parts()))
	{
		// Create the undo transaction.
		AuDesignUndoRedoTransaction& undo = StartUndo(archId, tr("Weapon Emit Location"));
		undo.append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

void DaDesigner::SetPreview(quint8 previewType, const AuDesign* design, quint32 genSeed)
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	//qDebug() << "DaDesigner::SetPreview" << preview;
	if(previewType && design && !IsPreview())
	{
		// Find someone in the designer to get an empire id from.
		const DaPerson* person = dynamic_cast<const DaPerson*>(FindIdRecursive(Owner()));
		if(!person)
			person = FindPersonRecursive();
		if(!person)
			return;

		m_EnvLock.lockForWrite();
		m_PreviewGenSeed = genSeed;
		if(m_Preview)
		{
			delete m_Preview;
			m_Preview = 0;
		}
		m_EnvLock.unlock();

		if(previewType >= 2)
		{
			// Make a sun.
			// Earth's sun Sol is a G2 V.
			DaStar* star = new DaStar;
			star->ConnectSignals();
			star->Create();
			star->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
			ether->AddAtom(star);
			g_Au.AtomAdded(star->Id());

			// Rename my pseudo solar system.
			star->CreateForPreview(previewType);
			star->SetNormal();
			star->SolarSystemNameChanged("", Name(), true);
			star->TickTockRecursive();
		}

		switch(DesignMode())
		{
		case InvalidMode:
		case DESIGN_Unknown:
			break;
		case DESIGN_Spacecraft:
			{
				const AuScDesign* scd = dynamic_cast<const AuScDesign*>(design);
				if(scd)
				{
					// Grab the preview design and create a spacecraft.
					m_EnvLock.lockForWrite();
					m_Preview = scd->Clone();
					m_EnvLock.unlock();
					DaVessel* v = FindVessel();
					if(!v)
					{
						// Create a ship atom.
						DaSc* pship = new DaSc;
						pship->ConnectSignals();
						pship->Create();
						pship->SetPosAndOrient(AuAtomPos(Id(), AuMatrix4x4()));
						ether->AddAtom(pship);
						pship->InitPreview(previewType, Name(), *scd, *person);	// Positions pship in orbit.
						g_Au.AtomAdded(pship->Id());
						pship->SetNormal();	// New atom is ready for normal thread processing.
						pship->TickTock();
					}
				}
			}
			break;

		case DESIGN_Building:
			{
				const AuBdDesign* bdd = dynamic_cast<const AuBdDesign*>(design);
				if(bdd)
				{
					// Grab the preview design but wait for them to place one.
					m_EnvLock.lockForWrite();
					m_Preview = bdd->Clone();
					m_EnvLock.unlock();
				}
			}
			break;
		case DESIGN_Item:
			break;

		//case DESIGN_Vehicle:
			//break;
		}

		if(previewType >= 2)
		{
			// Reposition people in the designer.
			DaAtomList people;
			FindPeopleRecursive(people);					// Only finds IsNormal people.
			for(DaAtomList::const_iterator it = people.begin(); it != people.end(); it++)
			{
				DaPerson* person = (DaPerson*)*it;
				person->InitPreview(previewType);
			}
		}
		SetModified();
	}
	else
	{
		// Cancel design preview.
		ResetPositions();				// Remove people from aboard subordinate objects.
		DaCapsule* v = FindCapsule();
		if(v)
			v->Kill();
		m_EnvLock.lockForWrite();
		if(m_Preview)
		{
			delete m_Preview;
			m_Preview = 0;
		}
		m_EnvLock.unlock();
		DaStar* star = FindStar();
		if(star)
			star->Kill();				// This will take out all subatoms of the star too.
		SetModified();
	}
}

bool DaDesigner::IsPreview() const
{
	return (m_Preview && FindVessel()) || FindStar();
}

QString DaDesigner::Name() const
{
	QString v;
	if(DesignName().isEmpty())
		v = tr("Studio %1, Unnamed Design").arg(StudioId());
	else
		v = tr("Studio %1, %2").arg(StudioId()).arg(DesignName());
	return v;
}

const GLuint* DaDesigner::EffectTextures() const
{
	return m_EffectCur;
}

void DaDesigner::AnimateEffectTextures()
{
	int lowHigh = AuElapsedTimer::CurrentSecIsOdd() ? 1 : 2;	// For things to alternate between low and high power version.
	bool capacitorWorking = true;
	int ftlHullHot = 0;	// 0=no, 1=warp, 2=wormhole positive, 3=wormhole negative, 4=wormhole neutral
	int ftlWorking[4];
	for(int s=0; s<4; s++)
		ftlWorking[s] = lowHigh;
	bool lifeSupportWorking = true;
	int maneuverWorking = lowHigh;	// 0=no, 1=low, 2=high
	if(maneuverWorking == 2 && BuildingStructure() == BD_AerialPlatform)
		maneuverWorking = 1;
	bool powerPlantWorking = true;
	bool sensorWorking = true;
	bool shieldWorking = true;
	int transporterWorking[10];
	for(int s=0; s<10; s++)
		transporterWorking[s] = lowHigh;
	bool weaponWorking[10] = {true, true, true, true, true, true, true, true, true, true};

	float secoff = AuTime::CurrentSecCycle();
	int secframe = secoff * 6;
	int randframe = m_EffectRandFrame;
	if(secframe != m_EffectFrame)
	{
		// Only change random frame when the sequential frame changes or the random frames change too fast.
		do
		{
			randframe = g_Au.Dice().Roll(1,6,-1);
		}
		while(randframe == m_EffectRandFrame);
		m_EffectRandFrame = randframe;
	}

	m_EffectCur[EFF_None] = m_EffectTex[EFF_None][0];

	for(int s=0; s<4; s++)
	{
		if(ftlWorking[s] == 2)
			m_EffectCur[EFF_FTLDrive1+s] = m_EffectTex[EFF_FTLDrive1+s][randframe];	// High
		else if(ftlWorking[s] == 1)
			m_EffectCur[EFF_FTLDrive1+s] = m_EffectTex[EFF_Count+3][randframe];	// Low
		else
			m_EffectCur[EFF_FTLDrive1+s] = m_EffectTex[EFF_None][0];	// Off
	}

	if(lifeSupportWorking)
		m_EffectCur[EFF_LifeSupport] = m_EffectTex[EFF_LifeSupport][secframe];
	else
		m_EffectCur[EFF_LifeSupport] = m_EffectTex[EFF_None][0];

	if(maneuverWorking == 2)
		m_EffectCur[EFF_ManeuverDrive] = m_EffectTex[EFF_ManeuverDrive][randframe];	// High
	else if(maneuverWorking == 1)
		m_EffectCur[EFF_ManeuverDrive] = m_EffectTex[EFF_Count][randframe];	// Low
	else
		m_EffectCur[EFF_ManeuverDrive] = m_EffectTex[EFF_None][0];	// Off

	if(powerPlantWorking)
		m_EffectCur[EFF_PowerPlant] = m_EffectTex[EFF_PowerPlant][secframe];
	else
		m_EffectCur[EFF_PowerPlant] = m_EffectTex[EFF_None][0];

	if(sensorWorking)
		m_EffectCur[EFF_Sensor] = m_EffectTex[EFF_Sensor][randframe];
	else
		m_EffectCur[EFF_Sensor] = m_EffectTex[EFF_None][0];

	if(shieldWorking)
		m_EffectCur[EFF_Shield] = m_EffectTex[EFF_Shield][randframe];
	else
		m_EffectCur[EFF_Shield] = m_EffectTex[EFF_None][0];

	for(int s=0; s<10; s++)
	{
		if(transporterWorking[s] == 2)
			m_EffectCur[EFF_TransporterPad1+s] = m_EffectTex[EFF_TransporterPad1+s][secframe];	// High
		else if(transporterWorking[s] == 1)
			m_EffectCur[EFF_TransporterPad1+s] = m_EffectTex[EFF_Count+1][secframe];	// Low
		else
			m_EffectCur[EFF_TransporterPad1+s] = m_EffectTex[EFF_None][0];	// Off
	}

	for(int s=0; s<10; s++)
	{
		if(weaponWorking[s])
			m_EffectCur[EFF_Weapon1+s] = m_EffectTex[EFF_Weapon1+s][randframe];
		else
			m_EffectCur[EFF_Weapon1+s] = m_EffectTex[EFF_None][0];
	}

	if(capacitorWorking)
		m_EffectCur[EFF_Capacitor] = m_EffectTex[EFF_Capacitor][secframe];
	else
		m_EffectCur[EFF_Capacitor] = m_EffectTex[EFF_None][0];

	if(ftlHullHot == 1)
		m_EffectCur[EFF_Hull] = m_EffectTex[EFF_Count+2][secframe];	// warp
	else if(ftlHullHot == 2)
		m_EffectCur[EFF_Hull] = m_EffectTex[EFF_Hull][secframe];	// wormhole positive
	else if(ftlHullHot == 3)
		m_EffectCur[EFF_Hull] = m_EffectTex[EFF_Count+4][secframe];	// wormhole negative
	else if(ftlHullHot == 4)
		m_EffectCur[EFF_Hull] = m_EffectTex[EFF_Count+5][secframe];	// wormhole neutral
	else
		m_EffectCur[EFF_Hull] = m_EffectTex[EFF_None][0];	// not ftl

	m_EffectFrame = secframe;
}

quint32 DaDesigner::GenSeed() const
{
	m_EnvLock.lockForRead();
	quint32 v = m_PreviewGenSeed;
	m_EnvLock.unlock();
	if(v == 0)
		v = Id().GetID();
	return v;
}

quint64 DaDesigner::MemoryUsage() const
{
	quint64 mem = DaDesignBase::MemoryUsage();
	mem += sizeof(DaDesigner) - sizeof(DaDesignBase);
	mem += StringMemoryUsage(m_Architect);
	if(m_Design)
		mem += m_Design->MemoryUsage();
	mem += (sizeof(quint16) + sizeof(AuElapsedTimer)) * m_DesignChangeTimes.size();
	if(m_Preview)
		mem += m_Preview->MemoryUsage();
	for(QHash<AuId, AuDesignUndoRedoList>::const_iterator ui = m_UndoRedo.cbegin(); ui != m_UndoRedo.cend(); ui++)
		mem += sizeof(AuId) + (*ui).MemoryUsage();
	mem += (sizeof(quint16) + sizeof(AuGLVAO)) * m_OpaqueVAOs.size();
	mem += (sizeof(quint16) + sizeof(AuGLVAO)) * m_TransparentVAOs.size();
	mem += (sizeof(quint16) + sizeof(AuGLVAO)) * m_OwnOpaqueVAOs.size();
	mem += (sizeof(quint16) + sizeof(GLuint)) * m_OwnTextures.size();
	mem += (sizeof(quint16) + sizeof(AuGLVAO)) * m_OwnTransparentVAOs.size();
	mem += (sizeof(quint16) + sizeof(AuGLVAO)) * m_DecalOpaqueVAOs.size();
	mem += (sizeof(quint16) + sizeof(AuGLVAO)) * m_DecalTransparentVAOs.size();
	mem += (sizeof(quint16) + sizeof(AuGLVAO)) * m_OwnDecalOpaqueVAOs.size();
	mem += (sizeof(quint16) + sizeof(AuGLVAO)) * m_OwnDecalTransparentVAOs.size();
	mem += (sizeof(quint16) + sizeof(AuGLVAO)) * m_WindowTranslucentVAOs.size();
	mem += (sizeof(quint16) + sizeof(AuGLVAO)) * m_WindowTransparentVAOs.size();

	mem -= sizeof(DpConsoleEmpty) * 10;
	mem += m_CmdEmpty.MemoryUsage();
	mem += m_EngineEmpty.MemoryUsage();
	mem += m_FireEmpty.MemoryUsage();
	mem += m_NavEmpty.MemoryUsage();
	mem += m_PilotEmpty.MemoryUsage();
	mem += m_PowerEmpty.MemoryUsage();
	mem += m_SensorEmpty.MemoryUsage();
	mem += m_ShieldEmpty.MemoryUsage();
	mem += m_SurgeryEmpty.MemoryUsage();
	mem += m_TransEmpty.MemoryUsage();
	return mem;
}

DaDesigner* DaDesigner::FindParentDesignerInEnv()
{
	return this;
}

const DaDesigner* DaDesigner::FindParentDesignerInEnv() const
{
	return this;
}

BUILDING_STRUCTURE DaDesigner::BuildingStructure() const
{
	if(DesignMode() != DESIGN_Building)
		return BD_Invalid;
	BUILDING_STRUCTURE structure = BD_Invalid;
	m_EnvLock.lockForRead();
	const AuBdDesign* bdd = dynamic_cast<const AuBdDesign*>(m_Design);
	if(bdd)
		structure = bdd->Structure();
	m_EnvLock.unlock();
	return structure;
}

void DaDesigner::PartSelectionDoorProperties(const AuScSelection& partIds, Qt::CheckState& isAutomatic, Qt::CheckState& initLocked,
		Qt::CheckState& remoteSpacecraft, Qt::CheckState& remoteVehicle) const
{
	if(!m_Design)
		return;

	m_EnvLock.lockForRead();
	m_Design->PartDoorProperties(partIds, isAutomatic, initLocked, remoteSpacecraft, remoteVehicle);
	m_EnvLock.unlock();
}

void DaDesigner::PartSelectionSiteProperties(const AuScSelection& partIds, quint8& grade, quint8& grass, quint8& pavement,
		quint8& rocks, quint8& shrubs, quint8& terBlend, quint8& trees) const
{
	if(!m_Design)
		return;

	m_EnvLock.lockForRead();
	m_Design->PartSiteProperties(partIds, grade, grass, pavement, rocks, shrubs, terBlend, trees);
	m_EnvLock.unlock();
}

void DaDesigner::PartSelectionLightBulbProperties(const AuScSelection& partIds, QColor& color, float& kL, float& kQ,
		float& cosOuterCone, float& cosInnerCone) const
{
	if(!m_Design)
		return;

	m_EnvLock.lockForRead();
	m_Design->PartLightBulbProperties(partIds, color, kL, kQ, cosOuterCone, cosInnerCone);
	m_EnvLock.unlock();
}

void DaDesigner::PartSelectionLightLensProperties(const AuScSelection& partIds, QColor& color) const
{
	if(!m_Design)
		return;

	m_EnvLock.lockForRead();
	m_Design->PartLightLensProperties(partIds, color);
	m_EnvLock.unlock();
}

void DaDesigner::PaintInUse(QList<QColor>& paintInUse) const
{
	if(!m_Design)
		return;

	m_EnvLock.lockForRead();
	m_Design->PaintInUse(paintInUse);
	m_EnvLock.unlock();
}

void DaDesigner::RenamePartStop(const AuId& archId, quint16 partId, const QString& oldStopName, const QString& nuStopName)
{
	if(!m_Design)
		return;

	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	m_EnvLock.lockForWrite();
	if(m_Design->PartRenameStop(partId, oldStopName, nuStopName, uchg.Parts()))
	{
		// Create the undo transaction.
		StartUndo(archId, tr("Rename Stop")).append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

QSet<quint8> DaDesigner::ViewPositions() const
{
	QSet<quint8> posIds;
	m_EnvLock.lockForRead();
	if(m_Design)
		posIds = m_Design->ViewPositions();
	m_EnvLock.unlock();
	return posIds;
}

void DaDesigner::ViewPositionSave(const AuId& avatarId, quint8 number)
{
	DaPerson* person = dynamic_cast<DaPerson*>(FindIdRecursive(avatarId));
	if(!person)
		return;

	AuDesignPosition pos(person->EnvTransform(), person->CurrentHeadTilt(), person->CurrentGeneralPose());
	m_EnvLock.lockForRead();
	int result = m_Design->ViewPositionSave(number, pos);
	m_EnvLock.unlock();

	QString resultMsg;
	switch(result)
	{
	case 0:	// ok
		SetModified();
		resultMsg = tr("View position %1 saved in the blueprint.").arg(number);
		break;
	case 1:
		SetModified();
		resultMsg = tr("View position %1 updated in the blueprint.").arg(number);
		break;
	case 2:
		resultMsg = tr("%1 is not a valid view position number.").arg(number);
		break;
	case 3:
		resultMsg = tr("My current position cannot be saved.");
		break;
	}
	if(!resultMsg.isEmpty())
		person->Thought(resultMsg);
}

void DaDesigner::ViewPositionGrid(const AuId& avatarId, const AuDesignPosition& pos)
{
	DaPerson* person = dynamic_cast<DaPerson*>(FindIdRecursive(avatarId));
	if(!person)
		return;

	// Don't change current body pose if it is not valid for positioning. We don't want to raise the dead here.
	if(!DpBody::CanRestorePositionPose(person->CurrentGeneralPose()))
		return;

	person->SetGeneralPose(pos.Pose());
	person->TiltHeadTo(pos.HeadTilt());
	person->SyncController(AuSyncPose(pos.Pose(), pos.HeadTilt(), 0, 0));
	person->ForcePosStop(AuAtomPos(Id(), pos.EnvTransform()));
	person->SetModified();
	QString resultMsg = tr("I moved to the grid.");
	person->Thought(resultMsg);
}

void DaDesigner::ViewPositionOrigin(const AuId& avatarId)
{
	DaPerson* person = dynamic_cast<DaPerson*>(FindIdRecursive(avatarId));
	if(!person)
		return;

	// Don't change current body pose if it is not valid for positioning. We don't want to raise the dead here.
	if(!DpBody::CanRestorePositionPose(person->CurrentGeneralPose()))
		return;

	AuDesignPosition pos(AuMatrix4x4(), 0, AuPose::Stand);
	person->SetGeneralPose(pos.Pose());
	person->TiltHeadTo(pos.HeadTilt());
	person->SyncController(AuSyncPose(pos.Pose(), pos.HeadTilt(), 0, 0));
	person->ForcePosStop(AuAtomPos(Id(), pos.EnvTransform()));
	person->SetModified();
	QString resultMsg = tr("I moved to the blueprint origin.");
	person->Thought(resultMsg);
}

void DaDesigner::ViewPositionLoad(const AuId& avatarId, quint8 number)
{
	DaPerson* person = dynamic_cast<DaPerson*>(FindIdRecursive(avatarId));
	if(!person)
		return;

	// Don't change current body pose if it is not valid for positioning. We don't want to raise the dead here.
	if(!DpBody::CanRestorePositionPose(person->CurrentGeneralPose()))
		return;

	AuDesignPosition pos;
	m_EnvLock.lockForRead();
	int result = m_Design->ViewPositionLoad(number, pos);
	m_EnvLock.unlock();

	QString resultMsg;
	switch(result)
	{
	case 0:	// ok
		{
			person->SetGeneralPose(pos.Pose());
			person->TiltHeadTo(pos.HeadTilt());
			person->SyncController(AuSyncPose(pos.Pose(), pos.HeadTilt(), 0, 0));
			person->ForcePosStop(AuAtomPos(Id(), pos.EnvTransform()));
			person->SetModified();
			resultMsg = tr("I moved to blueprint view position %1.").arg(number);
		}
		break;
	case 1:
		resultMsg = tr("%1 is not a valid view position number.").arg(number);
		break;
	case 2:
		resultMsg = tr("This blueprint has no view position %1 to restore.").arg(number);
		break;
	case 3:
		resultMsg = tr("View position %1 was not restored because the position data is invalid.").arg(number);
		break;
	}
	if(!resultMsg.isEmpty())
		person->Thought(resultMsg);
}

QSet<quint8> DaDesigner::GridPositions() const
{
	QSet<quint8> posIds;
	m_EnvLock.lockForRead();
	posIds = m_Design->GridPositions();
	m_EnvLock.unlock();
	return posIds;
}

void DaDesigner::GridPositionSave(const AuId& avatarId, quint8 number, const AuMatrix4x4& gridM)
{
	DaPerson* person = dynamic_cast<DaPerson*>(FindIdRecursive(avatarId));
	if(!person)
		return;

	m_EnvLock.lockForRead();
	int result = m_Design->GridPositionSave(number, gridM);
	if(result == 0 || result == 1)
		m_DesignChangeTimes.insert(0, AuElapsedTimer::Now());	// id 0 is the design record itself.
	m_EnvLock.unlock();

	QString resultMsg;
	switch(result)
	{
	case 0:	// ok saved first time
		SetModified();
		resultMsg = tr("Grid position %1 saved in the blueprint.").arg(number);
		break;
	case 1:	// ok updated
		SetModified();
		resultMsg = tr("Grid position %1 updated in the blueprint.").arg(number);
		break;
	case 2:
		resultMsg = tr("%1 is not a valid grid position number.").arg(number);
		break;
	}
	if(!resultMsg.isEmpty())
		person->Thought(resultMsg);
}

int DaDesigner::GridPosition(quint8 number, AuMatrix4x4& gridM) const
{
	m_EnvLock.lockForRead();
	int result = m_Design->GridPositionLoad(number, gridM);
	m_EnvLock.unlock();
	return result;
}

/*
void DaDesigner::GridPositionOrigin(const AuId& avatarId)
{
	DaPerson* person = dynamic_cast<DaPerson*>(FindIdRecursive(avatarId));
	if(!person)
		return;
	DaDesignPlane* pdp = FindDesignPlane(avatarId);
	if(!pdp)
		return;
	pdp->SetTransform(AuMatrix4x4());
	pdp->SetModified();
	QString resultMsg = tr("Grid moved to the blueprint origin.");
	person->Thought(resultMsg);
}

void DaDesigner::GridPositionLoad(const AuId& avatarId, quint8 number)
{
	DaPerson* person = dynamic_cast<DaPerson*>(FindIdRecursive(avatarId));
	if(!person)
		return;
	DaDesignPlane* pdp = FindDesignPlane(avatarId);
	if(!pdp)
		return;

	AuMatrix4x4 pos;
	m_EnvLock.lockForRead();
	int result = m_Design->GridPositionLoad(number, pos);
	m_EnvLock.unlock();

	QString resultMsg;
	switch(result)
	{
	case 0:	// ok
		{
			pdp->SetTransform(pos);
			pdp->SetModified();
			resultMsg = tr("Grid moved to blueprint grid position %1.").arg(number);
		}
		break;
	case 1:
		resultMsg = tr("%1 is not a valid grid position number.").arg(number);
		break;
	case 2:
		resultMsg = tr("This blueprint has no grid position %1 to restore.").arg(number);
		break;
	case 3:
		resultMsg = tr("Grid position %1 was not restored because the position data is invalid.").arg(number);
		break;
	}
	if(!resultMsg.isEmpty())
		person->Thought(resultMsg);
}
*/

void DaDesigner::PreviewPeople(const AuSpecies& species)
{
	AuEther* ether = Ether();
	if(!ether)
		return;
	//qDebug() << "DaDesigner::PreviewPeople" << *this << species.Description();
	if(!m_Preview)
		return;
	int animals = 0;
	int crew = 0;
	int officers = 0;
	int passengers = 0;
	int troops = 0;
	int workers = 0;
	DaAtomList existing;
	FindCitizensRecursive(existing);
	if(existing.empty())
	{
		switch(m_Preview->Type())
		{
		case AuDesign::BLUEPRINT_Invalid:
		case AuDesign::BLUEPRINT_Item:
		case AuDesign::BLUEPRINT_Count:
			break;
		case AuDesign::BLUEPRINT_Building:
			{
				DaAtomList bds;
				FindBdRecursive(bds);
				for(DaAtomList::const_iterator bi = bds.begin(); bi != bds.end(); bi++)
				{
					DaBd* bd = (DaBd*)*bi;

					AuScPartSet citizenPosts;
					bd->CitizenPosts(citizenPosts);
					for(QSet<quint16>::const_iterator pi = citizenPosts.cbegin(); pi != citizenPosts.cend(); pi++)
					{
						AuStation station;
						if(!bd->Station(*pi, station))
							continue;

						// Create a worker.
						AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)g_Au.Dice().Roll(1,2,-1);
						HANDEDNESS hand = (HANDEDNESS)g_Au.Dice().Roll(1,2,-1);
						DaNpc* worker = new DaNpc(ROLE_Worker, species, chroma, hand, g_Au.Dice().RandomPersonName());
						worker->ConnectSignals();
						worker->Create();

						worker->InitTraining();
						worker->ResetHitPoints();
						worker->ResetHunger();
						worker->ResetStamina();
						worker->SetEmpireId(bd->EmpireId());

						station.OffsetOpM(worker->ConsolePoseOffset(station.Type(), station.OccPos(), worker->CGHeightStanding(),
									worker->TorsoThickness(), station.SeatIndex()));

						// Position the atom in the environment.
						worker->SetPosAndOrient(AuAtomPos(bd->Id(), station.OpM()));
						ether->AddAtom(worker);

						worker->SitAtConsole(*pi, station.Type(), station.OccPos(), station.SeatIndex(), 0);
						worker->InitPirateGear(1);
						g_Au.AtomAdded(worker->Id());
						worker->SetInScene();
						worker->SetNormal();	// New atom is ready for normal thread processing.
						worker->TickTock();
						workers++;
					}

					AuScPartSet troopPosts;
					bd->TroopPosts(troopPosts);
					for(QSet<quint16>::const_iterator ti = troopPosts.cbegin(); ti != troopPosts.cend(); ti++)
					{
						AuStation station;
						if(!bd->Station(*ti, station))
							continue;

						// Create a troop.
						AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)g_Au.Dice().Roll(1,2,-1);
						HANDEDNESS hand = (HANDEDNESS)g_Au.Dice().Roll(1,2,-1);
						DaNpc* troop = new DaNpc(ROLE_Troop, species, chroma, hand, g_Au.Dice().RandomPersonName());
						troop->ConnectSignals();
						troop->Create();

						troop->InitTraining();
						troop->ResetHitPoints();
						troop->ResetHunger();
						troop->ResetStamina();
						troop->SetEmpireId(bd->EmpireId());
						station.OffsetOpM(troop->ConsolePoseOffset(station.Type(), station.OccPos(), troop->CGHeightStanding(),
									troop->TorsoThickness(), station.SeatIndex()));

						// Position the atom in the environment.
						troop->SetPosAndOrient(AuAtomPos(bd->Id(), station.OpM()));
						ether->AddAtom(troop);

						troop->SitAtConsole(*ti, station.Type(), station.OccPos(), station.SeatIndex(), 0);
						troop->InitPirateGear(1);
						g_Au.AtomAdded(troop->Id());
						troop->SetInScene();
						troop->SetNormal();	// New atom is ready for normal thread processing.
						troop->TickTock();
						troops++;
					}

					AuScPartSet livestockPosts;
					bd->LivestockPosts(livestockPosts);
					DhDNA animalDNA;	//DHH animalia
					animalDNA.RandomMonster(10, g_Au.Dice());
					for(QSet<quint16>::const_iterator ai = livestockPosts.cbegin(); ai != livestockPosts.cend(); ai++)
					{
						AuStation station;
						if(!bd->Station(*ai, station))
							continue;

						// Create an animal.
						AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)g_Au.Dice().Roll(1,2,-1);
						HANDEDNESS hand = (HANDEDNESS)g_Au.Dice().Roll(1,2,-1);
						DaAnimal* animal = new DaAnimal(animalDNA.Species(), chroma, hand, Mature);
						animal->ConnectSignals();
						animal->Create();

						animal->ResetHitPoints();
						animal->ResetHunger();
						animal->ResetStamina();
						animal->SetEmpireId(bd->EmpireId());

						// Position the atom in the environment.
						station.OffsetOpM(animal->ConsolePoseOffset(station.Type(), station.OccPos(), animal->CGHeightStanding(),
									animal->TorsoThickness(), station.SeatIndex()));
						animal->SetPosAndOrient(AuAtomPos(bd->Id(), station.OpM()));
						ether->AddAtom(animal);

						animal->SitAtConsole(*ai, station.Type(), station.OccPos(), station.SeatIndex(), 0);
						g_Au.AtomAdded(animal->Id());
						animal->SetInScene();
						animal->SetNormal();	// New atom is ready for normal thread processing.
						animal->TickTock();
						animals++;
					}
				}
			}
			break;
		case AuDesign::BLUEPRINT_Spacecraft:
			{
				DaAtomList vessels;
				FindVesselsRecursive(vessels);
				if(vessels.empty())
					break;
				DaVessel* v = (DaVessel*)(vessels.first());
				if(!v)
					break;

				// Officers
				int berths = v->OfficerBerthsAvailable();
				officers += berths;
				while(berths--)
				{
					DaNpc* officer = v->LoadPirateOfficer(v->EmpireId(), species);
					officer->SetHomeWorld(AuId(1), Name(), BODY_TYPE_Count, AuId(2), tr("Designer System"), AuId(3),
							tr("Designer Sector"), Galaxy());
				}

				// Crew
				berths = v->CrewBerthsAvailable();
				crew += berths;
				while(berths--)
					v->LoadPirateCrew(v->EmpireId(), species);

				// Troops
				berths = v->TroopBerthsAvailable();
				troops += berths;
				while(berths--)
					v->LoadPirateTroop(v->EmpireId(), species);

				// Passengers
				berths = v->PassengerBerthsAvailable();
				passengers += berths;
				while(berths--)
					v->LoadPiratePassenger(v->EmpireId(), species);

				// Fill livestock posts. We don't fill citizen or troop posts because that is our crew.
				AuScPartSet livestockPosts;
				v->LivestockPosts(livestockPosts);
				DhDNA animalDNA;	//DHH animalia
				animalDNA.RandomMonster(10, g_Au.Dice());
				for(QSet<quint16>::const_iterator ai = livestockPosts.cbegin(); ai != livestockPosts.cend(); ai++)
				{
					AuStation station;
					if(!v->Station(*ai, station))
						continue;

					// Create an animal.
					AuDNABase::CHROMATID chroma = (AuDNABase::CHROMATID)g_Au.Dice().Roll(1,2,-1);
					HANDEDNESS hand = (HANDEDNESS)g_Au.Dice().Roll(1,2,-1);
					DaAnimal* animal = new DaAnimal(animalDNA.Species(), chroma, hand, Mature);
					animal->ConnectSignals();
					animal->Create();

					animal->ResetHitPoints();
					animal->ResetHunger();
					animal->ResetStamina();
					animal->SetEmpireId(v->EmpireId());

					// Position the atom in the environment.
					station.OffsetOpM(animal->ConsolePoseOffset(station.Type(), station.OccPos(), animal->CGHeightStanding(),
								animal->TorsoThickness(), station.SeatIndex()));
					animal->SetPosAndOrient(AuAtomPos(v->Id(), station.OpM()));
					ether->AddAtom(animal);

					animal->SitAtConsole(*ai, station.Type(), station.OccPos(), station.SeatIndex(), 0);
					g_Au.AtomAdded(animal->Id());
					animal->SetInScene();
					animal->SetNormal();	// New atom is ready for normal thread processing.
					animal->TickTock();
					animals++;
				}
			}
			break;
		}
	}

	// Send a message to the person. DHH There could be more than one person on line.
	if(animals || crew || officers || passengers || troops || workers)
	{
		const DaPerson* person = FindPersonRecursive();
		if(person)
		{
			// Make a list of what we created.
			QStringList created;
			if(officers == 1)
				created << tr("1 officer");
			else if(officers)
				created << tr("%1 officers").arg(officers);

			if(crew == 1)
				created << tr("1 crew");
			else if(crew)
				created << tr("%1 crew").arg(crew);

			if(troops == 1)
				created << tr("1 troop");
			else if(troops)
				created << tr("%1 troops").arg(troops);

			if(passengers == 1)
				created << tr("1 passenger");
			else if(passengers)
				created << tr("%1 passengers").arg(passengers);

			if(workers == 1)
				created << tr("1 worker");
			else if(workers)
				created << tr("%1 workers").arg(workers);

			if(animals == 1)
				created << tr("1 animal");
			else if(animals)
				created << tr("%1 animals").arg(animals);

			// Build the message.
			QString text = tr("Created ");
			for(QStringList::const_iterator si = created.cbegin(); si != created.cend(); si++)
			{
				if(si != created.cbegin())
					text += ", ";
				text += *si;
			}
			text += tr(". DNA is %2.").arg(species.Description());
			person->Thought(text);
		}
	}
}

bool DaDesigner::SelectContiguous(double angleTol, AuScSelection& sel) const
{
	if(!m_Design)
		return false;
	m_EnvLock.lockForRead();
	bool result = m_Design->SelectContiguous(angleTol, sel);
	m_EnvLock.unlock();
	return result;
}

DaAtom::ATOM_TYPE DaDesigner::AtomType() const
{
	return ATOM_Designer;
}

void DaDesigner::PartGroupNames(QStringList& groupNames) const
{
	if(!m_Design)
		return;
	m_EnvLock.lockForRead();
	m_Design->PartGroupNames(groupNames);
	m_EnvLock.unlock();
}

void DaDesigner::SetPartsGroup(const AuId& archId, const AuScIdSet& partIds, const QString& groupName)
{
	if(!m_Design)
		return;
	// Group name must be valid if it is not empty; empty is ok.
	if(!groupName.isEmpty() && !IsNameValid(groupName))
		return;

	Q_UNUSED(archId);
	// Create undo item. We won't create a transaction unless something happens.
	AuDesignUndoRedoItem uchg(AuDesignUndoRedoItem::Change);

	// Change all the parts.
	m_EnvLock.lockForWrite();
	for(QSet<quint16>::const_iterator pi = partIds.m_Ids.cbegin(); pi != partIds.m_Ids.cend(); pi++)
		m_Design->PartSetGroup(*pi, groupName, uchg.Parts());

	// Finish undo if anything changed.
	if(!uchg.Parts().empty())
	{
		if(groupName.isEmpty())
			StartUndo(archId, tr("Clear Group")).append(uchg);
		else
			StartUndo(archId, tr("Set Group %1").arg(groupName)).append(uchg);
		SetPartChangeTimesNoLock(uchg.Parts());
		SetDesignModified();
	}
	m_EnvLock.unlock();
}

COMMODITYID DaDesigner::ItemCommodity() const
{
	if(DesignMode() != DESIGN_Item)
		return CID_Invalid;
	COMMODITYID cid = CID_Invalid;
	m_EnvLock.lockForRead();
	const AuItDesign* itd = dynamic_cast<const AuItDesign*>(m_Design);
	if(itd)
		cid = itd->CommodityId();
	m_EnvLock.unlock();
	return cid;
}

void DaDesigner::operator>>(QDataStream& out) const
{
	DaDesignBase::operator>>(out);
	out << m_DesignModified;
	out << m_ViewOnly;

	m_EnvLock.lockForRead();
	out << m_Architect;
	out << m_idArchitect;
	out << m_idOwner;

	if(m_Design)
	{
		out << (quint8)m_Design->Type();
		out << *m_Design;
	}
	else
		out << (quint8)AuDesign::BLUEPRINT_Invalid;

	QHash<quint16, AuTime> designChangeTimes;
	out << designChangeTimes;

	if(m_Preview)
	{
		out << (quint8)m_Preview->Type();
		out << *m_Preview;
	}
	else
		out << (quint8)AuDesign::BLUEPRINT_Invalid;

	out << m_PreviewGenSeed;
	out << m_PreviewHasPower;
	out << m_UndoRedo;
	m_EnvLock.unlock();
}

void DaDesigner::operator<<(QDataStream& in)
{
	DaDesignBase::operator<<(in);
	in >> m_DesignModified;
	in >> m_ViewOnly;

	m_EnvLock.lockForWrite();
	in >> m_Architect;
	in >> m_idArchitect;
	in >> m_idOwner;

	if(m_Design)
		delete m_Design;
	quint8 ui8;
	in >> ui8;
	m_Design = AuDesign::Create((AuDesign::BLUEPRINT_TYPE)ui8);
	if(m_Design)
		in >> *m_Design;

	QHash<quint16, AuTime> designChangeTimes;
	in >> designChangeTimes;

	if(m_Preview)
		delete m_Preview;
	in >> ui8;
	m_Preview = AuDesign::Create((AuDesign::BLUEPRINT_TYPE)ui8);
	if(m_Preview)
		in >> *m_Preview;

	in >> m_PreviewGenSeed;
	in >> m_PreviewHasPower;
	in >> m_UndoRedo;
	m_EnvLock.unlock();
}

