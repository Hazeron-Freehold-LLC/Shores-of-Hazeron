//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: daitem.cpp,v 1.572 2022/05/17 16:07:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "daitem.h"
//
#include "auatomrenderinfo.h"
#include "aucombat.h"
#include "aucommpacket.h"
#include "aufireparticles.h"
#include "auglmodelmaker.h"
#include "ausituation.h"
#include "ausound.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dacapsule.h"
#include "daeffect.h"
#include "daperson.h"
#include "daanimal.h"
#include "dagalaxy.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dastar.h"
#include "dauniverse.h"
#include "davehicle.h"
#include "daworldringsection.h"
#include "deconstructionprocess.h"
#include "demanufacturingprocess.h"
#include <dgdata.h>
#include "dicontrolinput.h"
#include <QSqlError>
#include <QSqlQuery>
#include <aubox.h>
#include <augleffects.h>
#include <auquad.h>

DaItem::DaItem(const AuItem& item)
{
	m_Item = item.Clone();
	m_OwnerEmpire = 0;
	m_StaticDisplay = false;
}

DaItem::DaItem()
{
	m_Item = 0;
	m_OwnerEmpire = 0;
	m_StaticDisplay = false;
}

DaItem::~DaItem()
{
	if(m_Item)
		delete m_Item;
}

DaItem::DaItem(const DaItem &rhs) : DaAtom(rhs)
{
	rhs.m_ItemLock.lockForRead();
	if(rhs.m_Item)
		m_Item = rhs.m_Item->Clone();
	else
		m_Item = 0;
	m_ItemChangeTime = rhs.m_ItemChangeTime;
	rhs.m_ItemLock.unlock();

	rhs.m_OwnerLock.lockForRead();
	m_OwnerChangeTime = rhs.m_OwnerChangeTime;
	m_OwnerEmpire = rhs.m_OwnerEmpire;
	m_OwnerId = rhs.m_OwnerId;
	m_OwnerName = rhs.m_OwnerName;
	m_StaticRoom = rhs.m_StaticRoom;
	rhs.m_OwnerLock.unlock();

	m_StaticDisplay = rhs.m_StaticDisplay;
}

// Transmit updates to this atom to a client.
// 'tmLastRefresh' is the last time the client received an update.
void DaItem::TxUpdate(QDataStream& toClient, const AuTime& tmLastUpdate, const AuElapsedTimer& lastTx, const DaPerson& sceneAtom)
{
	DaAtom::TxUpdate(toClient, tmLastUpdate, lastTx, sceneAtom);

	// Decide what to send.
	quint8 hasWhat = 0;

	m_ItemLock.lockForRead();
	if((lastTx.IsNull() || m_ItemChangeTime >= lastTx) && m_Item)
		hasWhat |= 0x01;
	m_ItemLock.unlock();

	m_OwnerLock.lockForRead();
	if(lastTx.IsNull() || m_OwnerChangeTime >= lastTx)
		hasWhat |= 0x80;
	m_OwnerLock.unlock();

	if(IsArmedFuseBomb())
		hasWhat |= 0x40;
	if(IsStaticDisplay())
		hasWhat |= 0x20;

	// Tell them what we are going to send.
	toClient << hasWhat;

	// Send what we said we would send.
	if(hasWhat & 0x01)
	{
		m_ItemLock.lockForRead();
		toClient << (quint8)m_Item->Type();
		toClient << *m_Item;
		m_ItemLock.unlock();
	}

	if(hasWhat & 0x80)
	{
		m_OwnerLock.lockForRead();
		toClient << m_OwnerId;
		toClient << m_OwnerName;
		toClient << m_StaticRoom;
		m_OwnerLock.unlock();
	}
}

// Receive updates to this atom from a server.
void DaItem::RxUpdate(QDataStream& fromServer, bool isFirstUpdate)
{
	DaAtom::RxUpdate(fromServer, isFirstUpdate);

	// Determine what was sent.
	quint8 hasWhat;
	fromServer >> hasWhat;

	// Receive what they said they would send.
	if(hasWhat & 0x01)
	{
		if(m_Item)
			delete m_Item;

		quint8 itemType;
		fromServer >> itemType;
		m_Item = AuItem::Create(itemType);
		if(m_Item)
			fromServer >> *m_Item;
	}

	if(hasWhat & 0x80)
	{
		// Receive owner info for defusing bombs and static display items.
		fromServer >> m_OwnerId;
		fromServer >> m_OwnerName;
		fromServer >> m_StaticRoom;
	}
	else if(hasWhat & 0x40)
	{
		// Received an armed fuse bomb. Set up member variables so 'IsArmedFuseBomb' will return true.
		m_OwnerId = AuId(1);
	}

	m_StaticDisplay = hasWhat & 0x20;
}

// Refresh data from valid counterpart.
bool DaItem::Refresh(DaAtom* pAtom, AuAtomRenderInfo* ari)
{
	bool transformChanged = DaAtom::Refresh(pAtom, ari);

	const DaItem* pA = (const DaItem*)pAtom;

	m_StaticDisplay = pA->m_StaticDisplay;

	pA->m_ItemLock.lockForRead();
	m_ItemLock.lockForWrite();
	if(m_Item && pA->m_Item)
	{
		// We each have an item.
		if(m_Item->CommodityId() == pA->m_Item->CommodityId())
		{
			// Our commodity ids haven't changed. Refresh without breaking GL render objects.
			if(ari)
				m_Item->Refresh(pA->m_Item, ari->re, ari->al);
			else
				m_Item->Refresh(pA->m_Item, 0, 0);
		}
		else
		{
			// Our commodity ids have changed. They can change when meat turns into a bone, for example.
			if(ari)
				m_Item->DeleteRenderObjects(*ari->re, ari->al);	// Forget my old model so I'll make a nu one.

			// Get a fresh clone of the incoming item.
			delete m_Item;
			m_Item = pA->m_Item->Clone();
		}
	}
	else
	{
		// One of us has an item and the other doesn't.
		if(pA->m_Item)
			m_Item = pA->m_Item->Clone();
		else if(m_Item)
		{
			delete m_Item;
			m_Item = 0;
		}
	}
	m_ItemLock.unlock();
	pA->m_ItemLock.unlock();

	pA->m_OwnerLock.lockForRead();
	m_OwnerLock.lockForWrite();
	m_OwnerId = pA->m_OwnerId;
	m_OwnerName = pA->m_OwnerName;
	m_StaticRoom = pA->m_StaticRoom;
	m_OwnerLock.unlock();
	pA->m_OwnerLock.unlock();

	if(ari && transformChanged)
		InvalidateEnvironment();

	return transformChanged;
}

DaAtom *DaItem::Clone() const
{
	return new DaItem(*this);
}

void DaItem::Create()
{
	DaAtom::Create();

	// Prevent immediate forage from plants that have been dropped from someone's inventory.
	m_LastForage.start();
}

bool DaItem::RoughExtents(AuBox &fExt) const
{
	if(CommodityId() == CID_Head)
	{
		AuPoint min(-1,-1,-1);
		AuPoint max(1,1,1);
		fExt.SetFromMinMax(min, max);
	}
	else
	{
		// Use 3D extents of the 3ds model.
		AuPoint min,max;
		m_ItemLock.lockForRead();
		if(m_Item)
			m_Item->ModelExtents(min, max);
		m_ItemLock.unlock();
		fExt.SetFromMinMax(min, max);
	}
	return true;
}

void DaItem::RenderEnvironmentObject(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, AuRenderLastMap& renderLast, RENDERPASS pass, DaAtomList& shadSc, DaAtomList& shadScFog,
		DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog, DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	Q_UNUSED(shadSc);
	Q_UNUSED(shadScFog);
	Q_UNUSED(shadScGlobe);
	Q_UNUSED(shadScGlobeFog);
	Q_UNUSED(shadScRingworld);
	Q_UNUSED(shadScRingworldFog);
	if(g_Au.m_DebugSwitches & 0x08000000)
		return;
	// Do not render distant items.
	if(pass != PASS_Near)
		return;

	RenderItem(projection, ari, rmod, sceneAtom, renderLast, 1);

	// OpenAL audio sources
	RenderAudioSources(sceneAtom, rmod.Matrix(Id()), ari.al, ari.fade);
}

void DaItem::RenderItem(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, AuRenderLastMap& renderLast, double scale)
{
	if(!m_Item)
		return;

	if(CommodityId() == CID_Head)
	{
		// Gen4 heads take a lot of work to make.
		bool waitingForModel = false;
		m_ItemLock.lockForRead();
		const AuItemDNA* idna = dynamic_cast<const AuItemDNA*>(m_Item);
		if(idna && idna->DNAWantsAnimaliaModel())
		{
			AuAnimalia* animalia = new AuAnimalia(*((const AuDNA*)idna->DNA()), idna->DNAChromatid(), idna->DNAHandedness(),
					idna->DNAGrowthStage());
			animalia->SetHairStyle(idna->DNAHair(), idna->DNAShave());
			animalia->Pose(Id(), AuPose::EyesOnly, false, 1);
			animalia->CreateClothingTex(*ari.re);
			ari.mm.Animalia(*ari.re, Id(), *animalia); 
			if(animalia->HasVAOs())
				m_Item->DNASetAnimaliaModel(animalia);
			else
			{
				waitingForModel = true;
				delete animalia;
			}
		}
		m_ItemLock.unlock();
		if(waitingForModel)
			return;
	}

	const AuMatrix4x4& model = rmod.Matrix(Id());

	// Get ambient lighting and fog values.
	float ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB;
	bool sunsBlocked;
	SceneRenderParameters(sceneAtom, ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB, ari.IsViewTopDown(), sunsBlocked);
	AuPoint ambientUp = model.ProjectVector(GravityNormal());

	// Flower pot wants wind vector in eye space.
	AuPoint windVec;
	if(IsFlowerPot())
		windVec = model.ProjectVector(Wind());

	// Disable suns when they are blocked at my location. Prepare to restore them.
	QList<int> sunsDisabled;
	if(sunsBlocked)
		ari.re->DisableSuns(sunsDisabled);

	// The scene atom's environment information is always up-to-date.
	if(sceneAtom->IsInAtmosphere())
	{
		const DaWorld* world = FindParentPlanetInEnv();
		if(world)
		{
			const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(world);
			if(rsworld)
			{
				// Inside atmosphere of a ringworld.
				float shadowPanelSunAngles[MAX_SUNS];
				for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
					shadowPanelSunAngles[sunIndex] = rsworld->ShadowPanelSunAngle(ari.re->Sun(sunIndex).m_ShadowAngle);

				double sceneDepth = m_Item->RenderOnRingworld(projection, model, *ari.re, ari.Models(), Id(), ambientR, ambientG,
						ambientB, ambientUp, GravityNormal(), fogDensity, fogR, fogG, fogB, shadowPanelSunAngles, windVec, scale,
						1);
				if(sceneDepth)
					renderLast.Insert(sceneDepth, this);
			}
			else
			{
				// Inside atmosphere of a globe.
				// Globe position in eye space is needed to calculate sun over-the-horizon effect.
				AuMatrix4x4 m;
				GetTransformRelativeToParent(m, world);
				AuPoint globeP = m.UnProject(0,0,0);		// Globe's position relative to me.
				globeP = model.Project(globeP);				// Globe's position in eye space.

				double sceneDepth = m_Item->RenderOnGlobe(projection, model, *ari.re, ari.Models(), Id(), ambientR, ambientG,
						ambientB, ambientUp, GravityNormal(), fogDensity, fogR, fogG, fogB, globeP, windVec, scale, 1);
				if(sceneDepth)
					renderLast.Insert(sceneDepth, this);
			}
		}
	}
	else
	{
		// Not inside atmosphere range of a world.
		double sceneDepth = m_Item->Render(projection, model, *ari.re, ari.Models(), Id(), ambientR, ambientG, ambientB, ambientUp,
				GravityNormal(), windVec, scale, 1);
		if(sceneDepth)
			renderLast.Insert(sceneDepth, this);
	}

	// Restore suns that I disabled.
	ari.re->EnableSuns(sunsDisabled);

	// Base atom class needs to render special effects.
	DaAtomList shadSc;
	DaAtomList shadScFog;
	DaAtomList shadScGlobe;
	DaAtomList shadScGlobeFog;
	DaAtomList shadScRingworld;
	DaAtomList shadScRingworldFog;
	DaAtom::RenderEnvironmentObject(projection, ari, rmod, sceneAtom, renderLast, PASS_Near, shadSc, shadScFog, shadScGlobe,
			shadScGlobeFog, shadScRingworld, shadScRingworldFog);
}

void DaItem::RenderLast(const AuMatrix4x4& projection, AuAtomRenderInfo& ari, AuModelMatrixHash& rmod,
		const DaPerson* sceneAtom, DaAtomList& shadSc, DaAtomList& shadScFog, DaAtomList& shadScGlobe, DaAtomList& shadScGlobeFog,
		DaAtomList& shadScRingworld, DaAtomList& shadScRingworldFog)
{
	DaAtom::RenderLast(projection, ari, rmod, sceneAtom, shadSc, shadScFog, shadScGlobe, shadScGlobeFog, shadScRingworld,
			shadScRingworldFog);

	// Flower pot does not need wind in second pass.
	AuPoint windVec;

	// Get ambient lighting and fog values.
	float ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB;
	bool sunsBlocked;
	SceneRenderParameters(sceneAtom, ambientR, ambientG, ambientB, fogDensity, fogR, fogG, fogB, ari.IsViewTopDown(), sunsBlocked);
	const AuMatrix4x4& model = rmod.Matrix(Id());
	AuPoint ambientUp = model.ProjectVector(GravityNormal());

	// Disable suns when they are blocked at my location. Prepare to restore them.
	QList<int> sunsDisabled;
	if(sunsBlocked)
		ari.re->DisableSuns(sunsDisabled);

	if(sceneAtom->IsInAtmosphere())
	{
		const DaWorld* world = FindParentPlanetInEnv();
		if(world)
		{
			const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(world);
			if(rsworld)
			{
				// Inside atmosphere of a ringworld.
				float shadowPanelSunAngles[MAX_SUNS];
				for(int sunIndex=0; sunIndex<MAX_SUNS; sunIndex++)
					shadowPanelSunAngles[sunIndex] = rsworld->ShadowPanelSunAngle(ari.re->Sun(sunIndex).m_ShadowAngle);

				m_Item->RenderOnRingworld(projection, model, *ari.re, ari.Models(), Id(), ambientR, ambientG, ambientB, ambientUp,
						GravityNormal(), fogDensity, fogR, fogG, fogB, shadowPanelSunAngles, windVec, 1, 2);
			}
			else
			{
				// Inside atmosphere of a globe.
				// Globe position in eye space is needed to calculate sun over-the-horizon effect.
				AuMatrix4x4 m;
				GetTransformRelativeToParent(m, world);
				AuPoint globeP = m.UnProject(0,0,0);		// Globe's position relative to me.
				globeP = model.Project(globeP);				// Globe's position in eye space.

				m_Item->RenderOnGlobe(projection, model, *ari.re, ari.Models(), Id(), ambientR, ambientG, ambientB, ambientUp,
						GravityNormal(), fogDensity, fogR, fogG, fogB, globeP, windVec, 1, 2);
			}
		}
	}
	else
	{
		// Not inside atmosphere range of a world.
		m_Item->Render(projection, model, *ari.re, ari.Models(), Id(), ambientR, ambientG, ambientB, ambientUp, GravityNormal(),
				windVec, 1, 2);
	}

	// Restore suns that I disabled.
	ari.re->EnableSuns(sunsDisabled);
}

void DaItem::DeleteDeviceObjects(AuAtomRenderInfo& ari)
{
	// Base class takes care of my texures.
	DaAtom::DeleteDeviceObjects(ari);

	if(m_Item)
		m_Item->DeleteRenderObjects(*ari.re, ari.al);
}

double DaItem::OrientationSpeed() const
{
	return PI*2;
}

bool DaItem::OkToOrient() const
{
	if(!IsInGravity() || !IsOnGround())
		return false;
	if(IsStaticDisplay())
		return false;
	return DaAtom::OkToOrient();
}

bool DaItem::Orient(const AuPoint& atLoc, const AuPoint& terNormal)
{
	if(!OkToOrient())
	{
		SetOrientRotation(Rotation());
		return false;	// Rotation was not changed.
	}

	AuPoint upvec = GroundUpVector();
	if(upvec.Dot(TerrainNormal()) > .999)
	{
		//qDebug() << "DaItem::Orient" << Id() << Name() << "already close enough";
		SetOrientRotation(Rotation());
		return false;
	}

	// Calculate rotation to orient level with the up vector.
	AuPoint vx,vy;
	if(upvec.y == 0)
	{
		vy = Transform().ProjectVector(AuPoint(0,1,0));
		vy.Unit();
		if(fabs(vy.Dot(terNormal)) > .9)
		{
			//qDebug() << "DaItem::Orient" << Id() << Name() << "orient on X";
			// Orient based on X.
			vx = Transform().ProjectVector(AuPoint(1,0,0));
			vx.Unit();
			vy = terNormal.Cross(vx);
			vy.Unit();
		}
		else
		{
			//qDebug() << "DaItem::Orient" << Id() << Name() << "orient on Y";
			// Orient based on Y.
			vx = vy.Cross(terNormal);
			vx.Unit();
			vy = terNormal.Cross(vx);
			vy.Unit();
		}
	}
	else
	{
		vx = Transform().ProjectVector(AuPoint(1,0,0));
		vx.Unit();
		if(fabs(vx.Dot(terNormal)) > .9)
		{
			//qDebug() << "DaItem::Orient2" << Id() << Name() << "orient on X";
			// Orient based on X.
			vy = Transform().ProjectVector(AuPoint(0,1,0));
			vy.Unit();
			vx = vy.Cross(terNormal);
			vx.Unit();
		}
		else
		{
			//qDebug() << "DaItem::Orient2" << Id() << Name() << "orient on Y";
			// Orient based on Y.
			vy = terNormal.Cross(vx);
			vy.Unit();
			vx = vy.Cross(terNormal);
			vx.Unit();
		}
	}

	AuMatrix4x4 tOrient;
	tOrient.Translate(atLoc);
	tOrient.Rotate(AuPoint::CalculateRotation(vx,vy));
	if(upvec != AuPoint(0,0,1))
		tOrient *= upvec.GetAlignmentTransform(AuPoint(0,0,1));

	AuPoint nuRot = tOrient.GetRotation();
	SetOrientRotation(nuRot);

	if(nuRot.FuzzyEquals(Rotation()))
		return false;	// No significant change in rotation.

	if(!IsAtRest())
		return false;

	// Preserve movement as a vector relative to parent. This will be immune to changes in my rotation.
	AuAtomPos pos(Pos());
	AuPoint otn = pos.Transform().ProjectVector(TerrainNormal());
	AuPoint omv = pos.Transform().ProjectVector(Vector());
	AuPoint ogn = pos.Transform().ProjectVector(GravityNormal());

	// Orient item to the terrain.
	ChangePosNotParent(AuAtomPos(pos.ParentId(), tOrient));

	// Restore movement as a vector relative to me.
	SetVector(tOrient.UnProjectVector(omv));
	SetGravityNormal(tOrient.UnProjectVector(ogn));
	SetTerrainNormal(tOrient.UnProjectVector(otn));
	return true;
}

// Attempts to consume this commodity into the construction process passed.
// If this atom is consumed, it dies.
bool DaItem::FetchConstructionMaterials(DeConstructionProcess &cp)
{
	m_ItemLock.lockForWrite();
	if(!m_Item)
	{
		m_ItemLock.unlock();
		return false;
	}

	if(m_Item->Content())
	{
		// Fetch items from this container.
		bool f = m_Item->FetchConstructionMaterials(cp);
		m_ItemLock.unlock();
		return f;
	}

	// Offer my item to the construction process.
	cp.ApplyFetchedMaterial(*m_Item);

	// If the construction process consumed the item, I'm gone.
	if(m_Item->Count() == 0)
	{
		//qDebug("DaItem::FetchConstructionMaterials - Consumed. %d killed.", Id().GetID());
		m_ItemLock.unlock();
		Kill();
		return true;
	}

	m_ItemLock.unlock();
	return false;
}

// Attempts to consume this commodity into the construction process passed.
// If this atom is consumed, it dies.
void DaItem::FetchManufacturingMaterials(DeManufacturingProcess &mp, quint8 elecSubQ, quint8 waterSubQ)
{
	m_ItemLock.lockForWrite();
	if(!m_Item)
	{
		m_ItemLock.unlock();
		return;
	}

	if(m_Item->Content() && !m_Item->Content()->empty())
	{
		// If a non-empty "crate" is emptied, it vanishes.
		if(!m_Item->FetchManufacturingMaterials(mp, elecSubQ, waterSubQ) && IsCrate())
		{
			//qDebug("DaItem::FetchManufacturingMaterials - Empty crate %d killed.", Id().GetID());
			Kill();
		}
	}
	else
	{
		// Offer myself to the Manufacturing process as an inventory item.
		mp.ApplyFetchedMaterial(*m_Item, elecSubQ, waterSubQ);

		// If the Manufacturing process consumed the item, I'm gone.
		if(m_Item->Count() == 0)
		{
			//qDebug("DaItem::FetchManufacturingMaterials - Consumed. %d killed.", Id().GetID());
			Kill();
		}
	}

	m_ItemLock.unlock();
}

void DaItem::EnumControlPanels(AuQuadArray &aControlPanels, const AuMatrix4x4 &tModel, QVector<qint16>& idPanels,
		const DaAtom* sceneAtom) const
{
	//Q_ASSERT(g_Au.IsSceneApp());
	Q_UNUSED(sceneAtom);
	if(IsFlowerPot())
	{
		const AuItemFloraDNA* pottedPlant = dynamic_cast<const AuItemFloraDNA*>(m_Item);
		if(pottedPlant && pottedPlant->HasDNA())
		{
			// Plant may have a resizer control that is inside the extents quads of the pot.
			AuBox fExt;
			RoughExtents(fExt);
			fExt.Transform(tModel);
			fExt.GetQuads(aControlPanels);
			if(IsStaticDisplay())
			{
				// Cannot control size of static display plant.
				idPanels.append(0);
				idPanels.append(0);
				idPanels.append(0);
				idPanels.append(0);
				idPanels.append(0);
				idPanels.append(0);
			}
			else
			{
				// Remove the top quad so we can hit the plant resizer control.
				aControlPanels.removeAt(4);	
				idPanels.append(0);
				idPanels.append(0);
				idPanels.append(0);
				idPanels.append(0);
				idPanels.append(0);

				// Make quad to control the size. It has an up/down control on it. We will interpret from the panel location.
				AuPoint sizerCenter;
				if(CommodityId() == CID_FlowerPotLarge)
					sizerCenter = AuPoint(-.0003, -.8437, 1.2829);
				else
					sizerCenter = AuPoint(.0005, -.4015, .6290);
				AuPoint sll(sizerCenter + AuPoint(-.1,-.05));
				AuPoint sul(sizerCenter + AuPoint(-.1,.05));
				AuPoint sur(sizerCenter + AuPoint(.1,.05));
				AuPoint slr(sizerCenter + AuPoint(.1,-.05));
				AuQuad sizeQ(sll, sul, sur, slr);
				sizeQ.Transform(tModel);
				aControlPanels.append(sizeQ);
				idPanels.append(1);
			}

			// Make quads to pick the fruit.
			AuPoint preb;											// Previous base point.
			AuPoint pret;											// Previous top point.
			for(int side=0; side<=8; side++)
			{
				// Panel id is the commodity id, and a tree indicator bit.
				float angle = PIf*2 * side / 8.f;					// Angle of this pass.
				AuPoint curb = AuPoint(.35, 0, 0).RotateZ(angle);
				AuPoint curt = AuPoint(.35, 0, 6).RotateZ(angle);
				if(side)
				{
					// Not the first loop iteration so we have preb and pret points to make a quad.
					// Create a quad at the resource location relative to the terrain.
					AuQuad trunkQ = AuQuad(preb, pret, curt, curb);

					// Transform the quad to be relative to the viewer.
					trunkQ.Transform(tModel);
					aControlPanels.append(trunkQ);
					idPanels.append(2);
				}
				preb = curb;
				pret = curt;
			}
			return;
		}
	}
	else if(CommodityId() == CID_Tepee || CommodityId() == CID_PressureTent)
	{
		// Make a panel for each side of the model extents, for pick up action.
		AuBox fExt;
		RoughExtents(fExt);
		fExt.Transform(tModel);
		fExt.GetQuads(aControlPanels);

		// Remove the bottom quad, which is the last one returned by GetQuads.
		aControlPanels.removeLast();

		idPanels.append(0);
		idPanels.append(0);
		idPanels.append(0);
		idPanels.append(0);
		idPanels.append(0);
		return;
	}

	// Make a panel for each side of the model extents, for pick up action.
	AuBox fExt;
	RoughExtents(fExt);
	fExt.Transform(tModel);
	fExt.GetQuads(aControlPanels);

	idPanels.append(0);
	idPanels.append(0);
	idPanels.append(0);
	idPanels.append(0);
	idPanels.append(0);
	idPanels.append(0);
}

void DaItem::EnumViewPanelControls(qint16 panelId, QList<AuControlButton>& controls, AuAtomRenderInfo& ari) const
{
	Q_UNUSED(panelId);
	GLuint texCID = ari.CommodityTextureN(CommodityId());
	if(!IsStaticDisplay())
		controls.append(AuControlButton(tr("Grab"), Id(), 0, CI_PickUp, texCID));
	else
		controls.append(AuControlButton(tr("Touch"), Id(), 0, CI_PickUp, texCID));

	m_ItemLock.lockForRead();
	const AuItemFloraDNA* pottedPlant = dynamic_cast<const AuItemFloraDNA*>(m_Item);
	if(pottedPlant && pottedPlant->HasDNA())
	{
		if(pottedPlant->DNAIsViable())
		{
			GLuint texPlantCID = ari.CommodityTextureN(pottedPlant->ProduceCommodityId());
			if(pottedPlant->ProduceCommodityId() == CID_Seed)
				controls.append(AuControlButton(tr("Seed"), Id(), 2, CI_PickProduceSeed, texPlantCID));
			else
			{
				GLuint texSeed = ari.CommodityTextureN(CID_Seed);
				controls.append(AuControlButton(tr("Forage"), Id(), 2, CI_PickProduceCID, texPlantCID));
				controls.append(AuControlButton(tr("Seed"), Id(), 2, CI_PickProduceSeed, texSeed));
			}
		}

		if(!IsStaticDisplay())
		{
			controls.append(AuControlButton(tr("Grow 25"), Id(), 1, CI_Grow3, texCID));
			controls.append(AuControlButton(tr("Grow 5"), Id(), 1, CI_Grow2, texCID));
			controls.append(AuControlButton(tr("Grow 1"), Id(), 1, CI_Grow1, texCID));
			controls.append(AuControlButton(tr("Shrink 1"), Id(), 1, CI_Shrink1, texCID));
			controls.append(AuControlButton(tr("Shrink 5"), Id(), 1, CI_Shrink2, texCID));
			controls.append(AuControlButton(tr("Shrink 25"), Id(), 1, CI_Shrink3, texCID));
		}
	}
	m_ItemLock.unlock();
}

// This is how mouse control inputs activate controls in the control input array.
// Controls in aActiveControls are activated accordingly.
void DaItem::SampleControlPanelInput(qint16 idPanel, const AuPoint& fPanelLoc, DiControlInputArray& aActiveControls,
		bool bContinuous)
{
	//Q_ASSERT(g_Au.IsSceneApp());
	Q_UNUSED(bContinuous);
	//qDebug() << "DaItem::SampleControlPanelInput" << Id() << Name() << "panel" << idPanel << "loc" << fPanelLoc
		//<< "cont" << bContinuous;
	switch(idPanel)
	{
	default:
	case 0:
		// Pick up the item.
		aActiveControls.append(DiControlInput(idPanel, CI_PickUp));
		break;

	case 1:
		// Resize potted plant control.
		if(!IsStaticDisplay())
		{
			double panloc = fPanelLoc.x - .1;
			if(panloc > 0)
			{
				// Click was in right half of panel, which is .2 wide.
				if(panloc > .1*2./3.)
					aActiveControls.append(DiControlInput(idPanel, CI_Grow3));
				else if(panloc > .1/3.)
					aActiveControls.append(DiControlInput(idPanel, CI_Grow2));
				else
					aActiveControls.append(DiControlInput(idPanel, CI_Grow1));
			}
			else
			{
				// Click was in left half of panel, which is .2 wide.
				panloc = fabs(panloc);
				if(panloc > .1*2./3.)
					aActiveControls.append(DiControlInput(idPanel, CI_Shrink3));
				else if(panloc > .1/3.)
					aActiveControls.append(DiControlInput(idPanel, CI_Shrink2));
				else
					aActiveControls.append(DiControlInput(idPanel, CI_Shrink1));
			}
		}
		break;

	case 2:
		// Pick produce from potted plant.
		aActiveControls.append(DiControlInput(idPanel, CI_PickProduce));
		break;
	}
}

// This is how all control inputs are passed to an atom.
// Only control inputs that have been returned by EnumControlInputs will be passed.
// Only control inputs that have been activated will be passed.
void DaItem::ProcessControlInput(const DiControlInputArray &aActiveControls, DaPerson *pSceneAtom, const AuGearPos& pos,
		qint8 chSelValue, CONTROL_CONTEXT cc, int msElapsed)
{
	Q_UNUSED(pos);
	Q_UNUSED(msElapsed);
	// When simulating local motion, we don't change atoms in the environment.
	// Just return and wait for the server to do the work.
	if(cc == CONTROL_Client)
		return;

	for(int ic=0; ic<aActiveControls.count(); ic++)
	{
		const DiControlInput &ci = aActiveControls[ic];
		switch(ci.GetId())
		{
		case CI_Grow1:
			if(!IsStaticDisplay())
			{
				m_ItemLock.lockForWrite();
				AuItemFloraDNA* pottedPlant = dynamic_cast<AuItemFloraDNA*>(m_Item);
				if(pottedPlant && pottedPlant->IncreaseSize(1))
				{
					m_ItemChangeTime.Set();
					SetModified();
				}
				m_ItemLock.unlock();
			}
			break;

		case CI_Grow2:
			if(!IsStaticDisplay())
			{
				m_ItemLock.lockForWrite();
				AuItemFloraDNA* pottedPlant = dynamic_cast<AuItemFloraDNA*>(m_Item);
				if(pottedPlant && pottedPlant->IncreaseSize(5))
				{
					m_ItemChangeTime.Set();
					SetModified();
				}
				m_ItemLock.unlock();
			}
			break;

		case CI_Grow3:
			if(!IsStaticDisplay())
			{
				m_ItemLock.lockForWrite();
				AuItemFloraDNA* pottedPlant = dynamic_cast<AuItemFloraDNA*>(m_Item);
				if(pottedPlant && pottedPlant->IncreaseSize(25))
				{
					m_ItemChangeTime.Set();
					SetModified();
				}
				m_ItemLock.unlock();
			}
			break;

		case CI_Shrink1:
			if(!IsStaticDisplay())
			{
				m_ItemLock.lockForWrite();
				AuItemFloraDNA* pottedPlant = dynamic_cast<AuItemFloraDNA*>(m_Item);
				if(pottedPlant && pottedPlant->DecreaseSize(1))
				{
					m_ItemChangeTime.Set();
					SetModified();
				}
				m_ItemLock.unlock();
			}
			break;

		case CI_Shrink2:
			if(!IsStaticDisplay())
			{
				m_ItemLock.lockForWrite();
				AuItemFloraDNA* pottedPlant = dynamic_cast<AuItemFloraDNA*>(m_Item);
				if(pottedPlant && pottedPlant->DecreaseSize(5))
				{
					m_ItemChangeTime.Set();
					SetModified();
				}
				m_ItemLock.unlock();
			}
			break;

		case CI_Shrink3:
			if(!IsStaticDisplay())
			{
				m_ItemLock.lockForWrite();
				AuItemFloraDNA* pottedPlant = dynamic_cast<AuItemFloraDNA*>(m_Item);
				if(pottedPlant && pottedPlant->DecreaseSize(25))
				{
					m_ItemChangeTime.Set();
					SetModified();
				}
				m_ItemLock.unlock();
			}
			break;

		case CI_PickUp:
			// A person picks up the item if it fits into their inventory.
			if(IsArmedDetonatorBomb() && !IsArmedBy(pSceneAtom->Id()))
			{
				// Picking up a detonator bomb that was armed by someone else.
				// chSelValue is the wire index from the defuse bomb UI.
				int defuseWire = Id().GetID() % 4;				// Index of the wire that defuses this bomb.
				if(chSelValue != defuseWire)
				{
					DetonateInitiate();						// Initiate explosion reaction.
					break;
				}
			}

			// Scene atom picks up the item.
			pSceneAtom->PickUp(this);
			break;

		case CI_PickProduce:
			{
				m_ItemLock.lockForRead();
				const AuItemFloraDNA* pottedPlant = dynamic_cast<const AuItemFloraDNA*>(m_Item);
				if(pottedPlant && pottedPlant->HasDNA())
				{
					if(pottedPlant->DNAIsViable())
					{
						if(!m_LastForage.isValid() || m_LastForage.elapsed()/d_MSecPerMin >= 10)
						{
							m_LastForage.restart();
							pSceneAtom->Forage(Quality(), pottedPlant->ProduceCommodityId(), pottedPlant->FloraDNA(),
									pottedPlant->PlantType(), pottedPlant->WorldGenSeed(), pottedPlant->WorldResourceZone(),
									pottedPlant->LatitudeRegion(), 0);
						}
						else
						{
							AuCommThoughtMessage msg(AuCommThoughtMessage::NothingRipe, pSceneAtom->Id(), pSceneAtom->Name());
							g_Au.SendAtomChat(msg);
						}
					}
					else
					{
						AuCommThoughtMessage msg(AuCommThoughtMessage::StaticDisplayFruit, pSceneAtom->Id(), pSceneAtom->Name());
						g_Au.SendAtomChat(msg);
					}
				}
				m_ItemLock.unlock();
			}
			break;
		case CI_PickProduceCID:
			{
				m_ItemLock.lockForRead();
				const AuItemFloraDNA* pottedPlant = dynamic_cast<const AuItemFloraDNA*>(m_Item);
				if(pottedPlant && pottedPlant->HasDNA())
				{
					if(pottedPlant->DNAIsViable())
					{
						if(!m_LastForage.isValid() || m_LastForage.elapsed()/d_MSecPerMin >= 10)
						{
							m_LastForage.restart();
							pSceneAtom->Forage(Quality(), pottedPlant->ProduceCommodityId(), pottedPlant->FloraDNA(),
									pottedPlant->PlantType(), pottedPlant->WorldGenSeed(), pottedPlant->WorldResourceZone(),
									pottedPlant->LatitudeRegion(), 1);
						}
						else
						{
							AuCommThoughtMessage msg(AuCommThoughtMessage::NothingRipe, pSceneAtom->Id(), pSceneAtom->Name());
							g_Au.SendAtomChat(msg);
						}
					}
					else
					{
						AuCommThoughtMessage msg(AuCommThoughtMessage::StaticDisplayFruit, pSceneAtom->Id(), pSceneAtom->Name());
						g_Au.SendAtomChat(msg);
					}
				}
				m_ItemLock.unlock();
			}
			break;
		case CI_PickProduceSeed:
			{
				m_ItemLock.lockForRead();
				const AuItemFloraDNA* pottedPlant = dynamic_cast<const AuItemFloraDNA*>(m_Item);
				if(pottedPlant && pottedPlant->HasDNA())
				{
					if(pottedPlant->DNAIsViable())
					{
						if(!m_LastForage.isValid() || m_LastForage.elapsed()/d_MSecPerMin >= 10)
						{
							m_LastForage.restart();
							pSceneAtom->Forage(Quality(), pottedPlant->ProduceCommodityId(), pottedPlant->FloraDNA(),
									pottedPlant->PlantType(), pottedPlant->WorldGenSeed(), pottedPlant->WorldResourceZone(),
									pottedPlant->LatitudeRegion(), 2);
						}
						else
						{
							AuCommThoughtMessage msg(AuCommThoughtMessage::NothingRipe, pSceneAtom->Id(), pSceneAtom->Name());
							g_Au.SendAtomChat(msg);
						}
					}
					else
					{
						AuCommThoughtMessage msg(AuCommThoughtMessage::StaticDisplayFruit, pSceneAtom->Id(), pSceneAtom->Name());
						g_Au.SendAtomChat(msg);
					}
				}
				m_ItemLock.unlock();
			}
			break;
		}
	}
}

// Insert first time database record.
void DaItem::Insert(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	QSqlQuery qa(QString(), db);
	QString sq("INSERT INTO a_Item (id,idp,locX,locY,locZ,rotX,rotY,rotZ,"
				"timeCreate,timeModified,timeTock,timeDeath, item,atRest,vecX,"
				"vecY,vecZ,cid,ownerId,ownerName,ownerEmpire,staticDisplay,staticRoom"
				") VALUES (?,?,?,?,?,?,?,?,?,?,?,?, ?,?,?,?,?,?,?,?,?,?,?);");
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

	// DaItem values.
	m_ItemLock.lockForRead();
	if(m_Item)
		qa.addBindValue(m_Item->Pack());
	else
		qa.addBindValue(QByteArray());
	m_ItemLock.unlock();
	qa.addBindValue(IsAtRest());
	qa.addBindValue(FuzzyClamp(Vector().x));
	qa.addBindValue(FuzzyClamp(Vector().y));
	qa.addBindValue(FuzzyClamp(Vector().z));
	qa.addBindValue(CommodityId());
	qa.addBindValue(m_OwnerId.GetID());
	qa.addBindValue(m_OwnerName);
	qa.addBindValue(m_OwnerEmpire);
	qa.addBindValue(m_StaticDisplay);
	qa.addBindValue(m_StaticRoom);

	if(!qa.exec())
	{
		QSqlError err = qa.lastError();
		qWarning("DaItem::Insert %s FAILED '%s' err '%s'",
				(const char*)Name().toLatin1(),
				(const char*)sq.toLatin1(),
				(const char*)err.text().toLatin1());
	}
}

// Update existing database record.
void DaItem::Update(QSqlDatabase& db) const
{
	if(IsDead())
		return;

	AuAtomPos pos(Pos());
	AuPoint vLoc = pos.Translation();
	AuPoint vRot = pos.Rotation();

	// This atom is known to the server so I need to update my existing record.
	QString sq;
	sq = QString("UPDATE a_Item SET idp=?, locX=?, locY=?, locZ=?, rotX=?, rotY=?, "
				"rotZ=?, timeModified=?, timeTock=?, timeDeath=?, item=?, "
				"atRest=?, vecX=?, vecY=?, vecZ=?, cid=?, ownerId=?, ownerName=?, ownerEmpire=?, staticDisplay=?, staticRoom=? "
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

	// DaItem values.
	m_ItemLock.lockForRead();
	if(m_Item)
		qa.addBindValue(m_Item->Pack());
	else
		qa.addBindValue(QByteArray());
	m_ItemLock.unlock();
	qa.addBindValue(IsAtRest());
	qa.addBindValue(FuzzyClamp(Vector().x));
	qa.addBindValue(FuzzyClamp(Vector().y));
	qa.addBindValue(FuzzyClamp(Vector().z));
	qa.addBindValue(CommodityId());
	qa.addBindValue(m_OwnerId.GetID());
	qa.addBindValue(m_OwnerName);
	qa.addBindValue(m_OwnerEmpire);
	qa.addBindValue(m_StaticDisplay);
	qa.addBindValue(m_StaticRoom);

	if(!qa.exec())
	{
		// Execution of the query failed. Report the error.
		QSqlError err = qa.lastError();
		qWarning("DaItem::Update %d %s FAILED '%s' err '%s'.",
				Id().GetID(), (const char*)Name().toLatin1(), (const char*)sq.toLatin1(), (const char*)err.text().toLatin1());
	}
	else if(qa.numRowsAffected() == 0)
	{
		// Execution succeeded but did not affect any records.
		// This happens when an atom is created that gets transferred to another server before the INSERT query is ever posted.
		// On arrival to the destination server, all atoms are touched with SetModified so they'll get saved.
		//qDebug("DaItem::Update %d %s affected NO records. Trying Insert instead.",
				//Id().GetID(), (const char*)Name().toAscii());
		Insert(db);
	}
}

// static
// Load all of these atoms whose parent is idParent.
// Returns 0 if 'idToLoad' == 0, which loads all atoms subordinate to 'aParent'.
// Returns address of loaded atom if 'idToLoad' != 0 and it is successfully
// loaded.
DaItem* DaItem::LoadSubatoms(AuEther& ether, QSqlDatabase& db, const AuId& parentId, const AuId& idToLoad)
{
	QString sq("SELECT id,locX,locY,locZ,rotX,rotY,rotZ,timeCreate,"
				"timeModified,timeTock,timeDeath, item,atRest,vecX,vecY,vecZ,ownerId,ownerName,ownerEmpire,staticDisplay,"
				"staticRoom "
				"FROM a_Item WHERE ");
	if(!idToLoad.IsNull())
		sq += " id=" + QVariant(idToLoad.GetID()).toString();
	else
		sq += " idp=" + QVariant(parentId.GetID()).toString();
	sq += ";";
	QSqlQuery qa(QString(), db);
	qa.prepare(sq);
	qa.exec();
	while(qa.next())
	{
		DaItem *pnu = new DaItem;
		if(pnu)
		{
			// DaAtom values.
			int qv=0;
			quint32 id = qa.value(qv++).toUInt();
			pnu->SetId(AuId(id));
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

			// DaItem values.
			int qvi = qv++;
			if(!qa.value(qvi).isNull())
			{
				if(pnu->m_Item)
					delete pnu->m_Item;
				pnu->m_Item = AuItem::Unpack(qa.value(qvi).toByteArray());
			}
			pnu->SetAtRestState(qa.value(qv++).toBool());
			vx = qa.value(qv++).toDouble();
			vy = qa.value(qv++).toDouble();
			vz = qa.value(qv++).toDouble();
			pnu->SetVector(AuPoint(vx,vy,vz));
			pnu->m_OwnerId = AuId(qa.value(qv++).toUInt());
			pnu->m_OwnerName = qa.value(qv++).toString();
			pnu->m_OwnerEmpire = qa.value(qv++).toUInt();
			pnu->m_StaticDisplay = qa.value(qv++).toBool();
			pnu->m_StaticRoom = qa.value(qv++).toString();

			pnu->SetInScene();							// Don't decay too fast right away.

			pnu->ConnectSignals();
			ether.AddAtom(pnu);
			pnu->SetNormal();	// New atom is ready for normal thread processing. This must be done before loading subatoms.

			// Kill off items that are obsolete. We kill them so they get deleted from the database, instead of just deleting the
			// item here.
			if(DbCommodity::IsObsolete(pnu->CommodityId()))
				pnu->Kill();
		}
		if(!idToLoad.IsNull())
			return pnu;
	}
	return 0;
}

bool DaItem::IsSightTarget(const DaAtom* toWho) const
{
	// Don't sight myself.
	if(toWho == this)
		return false;

	// This IsDying test prevents bombs that have been initiated for detonation from being targeted repeatedly.
	if(IsDying())
		return false;

	if(IsStaticDisplay())
		return false;

	// Items must not be a sight target to spacecraft or cities because they block shots by bay weapons and base weapons.
	if(dynamic_cast<const DaCapsule*>(toWho))
		return false;											// Bay|base weapons do not hit items.

	return true;
}

// Drag in ft/sec/sec.
// Drag is such that an object will fall at 120mph in standard atmosphere at
// standard gravity acceleration (32ft/sec/sec).
/*
#define ITEM_DRAG_FACTOR(maxMph, accel)	((5280./3600.*maxMph) * (5280./3600.*maxMph) / accel)
const double s_ItemDF = ITEM_DRAG_FACTOR(120, 32);
AuPoint DaItem::Drag() const
{
	float airDensity;
	if(IsInSpacecraft() && IsAirBreathable())
		airDensity = .5f;	// standard atmosphere aboard spacecraft
	else
		airDensity = AtmosphereDensity();
	double drag = (Vector().Length2() / s_ItemDF) * airDensity * 2;
	return AuPoint(drag, drag, drag);
}
*/

AuPoint DaItem::Acceleration() const
{
	if(IsStaticDisplay())
		return AuPoint(0,0);
	return DaAtom::Acceleration();
}

AuPoint DaItem::Friction() const
{
	double fricFwd = 0, fricLat = 0, fricVrt = 0;
	if(IsInVisibleWater())
	{
		// In water induces some drag.
		fricFwd = 10;
		fricLat = 10;
		fricVrt = 10;
	}

	// No need to orient friction values to object rotation when not touching the ground.
	if(!IsInGravity() || !IsTouchingGround())
		return AuPoint(fricFwd, fricLat, fricVrt);

	// Determine friction due to ground surface contact.
	// Assumes I'm oriented upright to the surface.
	switch(OnGround())
	{
	case OG_None:			// Not on ground so no more drag from environment.
	case OG_WaterSoundOnly:	// Not really returned by OnGround().
		break;
	case OG_Forest:
		fricFwd += 60;
		fricLat += 60;
		fricVrt += 60;
		break;
	case OG_Snow:
		fricFwd += 70;
		fricLat += 70;
		fricVrt += 70;
		break;
	case OG_HardFloor:
		fricFwd += 20;
		fricLat += 20;
		fricVrt += 20;
		break;
	case OG_Metal:
		fricFwd += 20;
		fricLat += 20;
		fricVrt += 20;
		break;
	case OG_Pavement:
		fricFwd += 20;
		fricLat += 20;
		fricVrt += 20;
		break;
	case OG_WoodFloor:
		fricFwd += 20;
		fricLat += 20;
		fricVrt += 20;
		break;
	case OG_Rock:
		fricFwd += 50;
		fricLat += 50;
		fricVrt += 50;
		break;
	case OG_Sand:
		fricFwd += 30;
		fricLat += 30;
		fricVrt += 30;
		break;
	case OG_Savanna:
		fricFwd += 40;
		fricLat += 40;
		fricVrt += 40;
		break;
	}

	return AuPoint(fricLat, fricFwd, fricVrt);
}

void DaItem::FilterFinalMoveVector(AuPoint& vector) const
{
	// When sitting on the ground, eliminate the downward portion of the movement vector.
	if(IsInGravity() && IsTouchingGround())
	{
		AuPoint gup(GroundUpVector());
		if(gup == AuPoint(0,0,1))
		{
			if(vector.z < 0)
				vector.z = 0;
		}
		else
		{
			AuMatrix4x4 obRot = gup.GetAlignmentTransform(AuPoint(0,0,1));
			AuPoint vec = obRot.Project(vector);
			//qDebug("DaItem::FilterFinalMoveVector %d %s vector %g,%g,%g vec %g,%g,%g", 
					//Id().GetID(), (const char*)Name().toAscii(), vector.x, vector.y, vector.z, vec.x,vec.y,vec.z);
			if(vec.z < 0)
			{
				vec.z = 0;
				vector = obRot.UnProject(vec);
				if(IsFuzzyZero(vector.x))
					vector.x = 0;
				if(IsFuzzyZero(vector.y))
					vector.y = 0;
				if(IsFuzzyZero(vector.z))
					vector.z = 0;
				//qDebug("DaItem::FilterFinalMoveVector %d %s result vector %g,%g,%g",
						//Id().GetID(), (const char*)Name().toAscii(), vector.x, vector.y, vector.z);
			}
		}
	}
}

void DaItem::FilterPostMoveVector(AuPoint& vector) const
{
	if(!IsInGravity())
		return;
	// When sitting on the ground, eliminate the downward portion of the movement vector.
	// Eliminate gravity vector when blocked to help get free from the side of buildings.
	if(IsTouchingGround())
	{
		AuPoint gup(GroundUpVector());
		if(gup == AuPoint(0,0,1))
		{
			if(vector.z < 0)
				vector.z = 0;
		}
		else
		{
			AuMatrix4x4 obRot = gup.GetAlignmentTransform(AuPoint(0,0,1));
			AuPoint vec = obRot.Project(vector);
			//qDebug("DaItem::FilterPostMoveVector %d %s vector %g,%g,%g vec %g,%g,%g", 
					//Id().GetID(), (const char*)Name().toAscii(), vector.x, vector.y, vector.z, vec.x,vec.y,vec.z);
			if(vec.z < 0)
			{
				vec.z = 0;
				vector = obRot.UnProject(vec);
				if(IsFuzzyZero(vector.x))
					vector.x = 0;
				if(IsFuzzyZero(vector.y))
					vector.y = 0;
				if(IsFuzzyZero(vector.z))
					vector.z = 0;
				//qDebug("DaItem::FilterPostMoveVector %d %s result vector %g,%g,%g",
						//Id().GetID(), (const char*)Name().toAscii(), vector.x, vector.y, vector.z);
			}
		}
	}
}

void DaItem::AnimateDeviceObjects(AuAtomRenderInfo& ari, const DaPerson* sceneAtom, const AuElapsedTimer& lastAnim)
{
	DaAtom::AnimateDeviceObjects(ari, sceneAtom, lastAnim);
}

bool DaItem::IsTicker() const
{
	if(DaAtom::IsTicker())
		return true;
	if(IsDying() && DoesExplodeWhenDamaged())
		return true;
	return false;
}

void DaItem::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now)
{
	// When a detonator or fuse weapon is dying, detonation has been initiated. Explode now. This allows for chain
	// reaction explosions without hogging the CPU.
	if(IsDying() && DoesExplodeWhenDamaged())
		if(AuTime::Now() >= DeathTime())
			DetonateTerminate();								// BOOM!
	DaAtom::Tick(last, now);
}

unsigned int DaItem::IsTocker() const
{
	unsigned int ms = DaAtom::IsTocker();
	if(ms == 0 || ms > d_MSecPerSec)
	{
		if(!m_Item)
			return d_MSecPerSec;
	}
	if(!IsNormal())
		return ms;
	if(ms == 0 || ms > d_MSecPerSec)
	{
		if(!m_OwnerId.IsNull())
		{
			m_ItemLock.lockForRead();
			bool isf = m_Item->IsFuseWeapon();
			m_ItemLock.unlock();
			if(isf)
				ms = d_MSecPerSec;
		}
		else
		{
			const DaStar* parentStar = dynamic_cast<const DaStar*>(Parent());
			if(parentStar)
				ms = d_MSecPerSec;
		}
	}
	if(ms == 0 || ms > d_MSecPerMin)
	{
		m_ItemLock.lockForWrite();
		if(m_Item->Content())
			ms = d_MSecPerMin;
		else if(m_Item->StoryInstance() && !g_Au.IsStoryActive(m_Item->StoryInstance()))
		{
			// My item is associated with a defunct story.
			if(!m_Item->Content() || m_Item->Content()->empty())
				ms = d_MSecPerMin;
		}
		m_ItemLock.unlock();
	}

	if(!IsStaticDisplay())
	{
		switch(CommodityId())
		{
		default:
			break;
		case CID_Beacon:
			if(ms == 0 || ms > d_MSecPerMin)
			{
				if(ItemState() == 1)
					ms = d_MSecPerMin;
			}
			break;
		case CID_TypeOPreons:
		case CID_TypeBPreons:
		case CID_TypeAPreons:
		case CID_TypeFPreons:
		case CID_TypeGPreons:
		case CID_TypeKPreons:
		case CID_TypeMPreons:
			if(ms == 0 || ms > d_MSecPerSec)
			{
				// Stargate keys open stargates.
				if(IsOnGround())
				{
					DaWorldRingSection* ringWorld = dynamic_cast<DaWorldRingSection*>(Parent());
					if(ringWorld && ringWorld->IsActivationLocation(Location()))
						ms = d_MSecPerSec;					// Stargate can be opened.
				}
			}
			break;
		case CID_PreonRingO:
		case CID_PreonRingB:
		case CID_PreonRingA:
		case CID_PreonRingF:
		case CID_PreonRingG:
		case CID_PreonRingK:
		case CID_PreonRingM:
			if(ms == 0 || ms > d_MSecPerSec)
			{
				// Henge keys activate henge portals.
				if(IsOnGround())
				{
					DaWorldLL* worldll = dynamic_cast<DaWorldLL*>(Parent());
					if(worldll && worldll->IsHengeActivationLocation(Location()))
						ms = d_MSecPerSec;					// I am in the activation area of a henge.
				}
			}
			break;
		}

		if(IsActiveMissile())
			return ms;
	}
	else
	{
		if(IsInSpacecraft() || IsInAtmosphere())
			return ms;
	}

	if(ms == 0 || ms > 30*d_MSecPerSec)
		ms = 30*d_MSecPerSec;

	return ms;
}

void DaItem::Tock(const AuTime& last, const AuTime& now, const AuScripts& scripts, DtTockTransactions& tockTrans)
{
	if(!m_Item)
	{
		qDebug() << "DaItem::Tock" << Id() << Name() << "KILLED! Item is null.";
		Kill();
		return;
	}

	// When a detonator or fuse weapon is dying, detonation has been initiated. Explode now. This allows for chain
	// reaction explosions without hogging the CPU.
	if(IsDying() && DoesExplodeWhenDamaged())
		if(AuTime::Now() >= DeathTime())
		{
			DetonateTerminate();								// BOOM!
			tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
		}

	if(!m_OwnerId.IsNull() && last.ElapsedSecTicks(now) > 0)
	{
		// This may be an armed fuse weapon.
		m_ItemLock.lockForRead();
		bool isf = m_Item->IsFuseWeapon();
		m_ItemLock.unlock();
		if(isf)
		{
			// The item is a fused weapon with an owner id assigned, which means it is set to go off. How about 5 seconds?
			if(CreatedTime().SecsTo(now) >= 5)
				DetonateTerminate();								// BOOM!
		}
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	DaAtom::Tock(last, now, scripts, tockTrans);
	if(!IsNormal())
		return;

	// Items that are associated with a story instance disappear when their story instance disappears.
	bool minTicked = last.ElapsedMinTicks(now);
	if(minTicked)
	{
		m_ItemLock.lockForWrite();

		// Destruct contained items.
		if(m_Item->Content() && m_Item->Content()->DestructStoryItems())
			SetModified();

		if(m_Item->StoryInstance() && !g_Au.IsStoryActive(m_Item->StoryInstance()))
		{
			// My item is associated with a defunct story.
			if(!m_Item->Content() || m_Item->Content()->empty())
			{
				// My item is not a container or it is empty.
				m_ItemLock.unlock();
				Kill();
				return;
			}
		}
		m_ItemLock.unlock();
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	// Test for solar radiation damage if my parent is a sun.
	const DaStar* parentStar = dynamic_cast<const DaStar*>(Parent());
	if(parentStar)
	{
		ApplySolarRadiation(parentStar, 100000);
		tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
	}

	if(!IsStaticDisplay())
	{
		// Some items do things.
		switch(CommodityId())
		{
		default:
			// Most items do nothing.
			break;

		case CID_Beacon:
			// Beacons broadcast their message every two minutes when on.
			if(ItemState() == 1)
			{
				// The beacon is on.
				if(minTicked && now.MinsSinceEpoch()&1)
				{
					// Build the broadcast message.
					QString text = BeaconMessage();
					if(!text.isEmpty())
					{
						AuCommHailReply msg(text, FindEnvironmentAtom()->Id(), AuCommStdChannel::HailChannelName(), Id(), Name(),
								FindParentSector()->Id(), Galaxy(), AuCommStdChannel::SolarSystem, AuCommStdChannel::Everyone);
						g_Au.SendAtomChat(msg);
					}
				}

				// Active beacons do not decay until they have been alive for a week.
				// This only applies when in an atmosphere. This beacon won't decay if it is inside a spacecraft or vehicle anyway.
				// In vacuum we want it to decay.
				if(IsInAtmosphere() && CreatedTime().DaysTo(now) < 7)
					return;
			}
			break;

		case CID_TypeOPreons:
		case CID_TypeBPreons:
		case CID_TypeAPreons:
		case CID_TypeFPreons:
		case CID_TypeGPreons:
		case CID_TypeKPreons:
		case CID_TypeMPreons:
			// Stargate keys open stargates.
			if(IsOnGround() && !InDesignEnvironment())
			{
				// Stargate key is lying on the ground. Does it open a stargate?
				DaWorldRingSection* ringWorld = dynamic_cast<DaWorldRingSection*>(Parent());
				if(ringWorld && ringWorld->IsActivationLocation(Location()))
				{
					// I am on the activation area of a ringworld control center.
					if(ringWorld->CanOpenStargate())
					{
						// Stargate can be opened.
						m_ItemLock.lockForRead();
						const AuItemGalaxyLoc* stargateKey = dynamic_cast<const AuItemGalaxyLoc*>(m_Item);
						if(stargateKey)
						{
							DaUniverse* u = ringWorld->FindParentUniverse();
							DaStar* star = (DaStar*)ringWorld->Parent();
							if(u && star)
							{
								// Find the destination galaxy.
								// Live 30 minutes, or until shadow panel clips beams.
								AuTime expire = AuTime::Now().AddSecs(60 * 30);
								star->OpenStargate(stargateKey->GalaxyId(), stargateKey->GalaxyLoc(), true, expire);
								Kill();
							}
						}
						m_ItemLock.unlock();
					}

					// Don't decay here; I might open the stargate later.
					return;
				}
			}

			// Stargate keys are also henge keys.
			[[fallthrough]];

		case CID_PreonRingO:
		case CID_PreonRingB:
		case CID_PreonRingA:
		case CID_PreonRingF:
		case CID_PreonRingG:
		case CID_PreonRingK:
		case CID_PreonRingM:
			// Henge keys activate henge portals.
			if(IsOnGround() && !InDesignEnvironment())
			{
				DaWorldLL* worldll = dynamic_cast<DaWorldLL*>(Parent());
				if(worldll && worldll->IsHengeActivationLocation(Location()))
				{
					// I am in the activation area of a henge.
					// Opening a henge does not destroy the preon but it is not immune to decay.
					worldll->ActivateHengeContaining(Location());
				}
			}
			break;
		}

		// Active missiles do not decay because they left the scene.
		if(IsActiveMissile())
			return;
	}
	else
	{
		// Static display items do not decay aboard spacecraft or on planets.
		if(IsInSpacecraft() || IsInAtmosphere())
			return;
	}
	tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));

	// Decay for garbage collection.
	if(IsInAtmosphere())
	{
		// Item is inside atmosphere range of a world, whether or not it has any air.
		if(IsOnGround() && !IsInWater() && BdId().IsNull() && !IsInSpacecraft() && !IsAboardVehicle())
		{
			// This item lying in the environment is protected from decay if it is near a village and not in a city. Villages move
			// periodically so the waste they leave behind will go away.
			bool decayAtVillage = false;
			m_ItemLock.lockForRead();
			if(m_Item->IsFood() && m_Item->FoodNutritionValue())
				decayAtVillage = true;	// Including heads.
			m_ItemLock.unlock();

			if(!decayAtVillage)
			{
				// This item does not decay when in a village.
				const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(Parent());
				if(llworld && llworld->HasVillage())
				{
					// World has a populated village.
					AuGlobeLLF vll = llworld->VillageLL();
					double valt = llworld->TerrainAltitudeMSL(vll);
					AuPoint vloc = llworld->LocOfLL(vll, valt);
					if(vloc.Distance2(Location()) < 50*FeetPerMeter()*50*FeetPerMeter())
						return;	// I am within range of the village. I could be a tent or the fire. I do not decay.
				}
			}
		}
	}
	else
	{
		// Item is outside the atmosphere range of any world.
		if(!IsInAnyScene() && !IsInsideSpacecraft() && !IsAboardVehicle())
		{
			// Eliminate junk that is not in any scene and not in a spacecraft or vehicle and not on a planet.
			Kill();
			return;
		}
	}
	tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));

	// Items that are part of a story do not decay. They are removed above when their story is no longer active. That check is only
	// made once per minute.
	m_ItemLock.lockForRead();
	bool inStory = m_Item->StoryInstance();
	m_ItemLock.unlock();

	if(!inStory && g_Au.m_EnableUnitDecay)
	{
		// Food, e.g. animal meat, decays faster than other stuff.
		bool decay = false;
		m_ItemLock.lockForRead();
		if(m_Item->IsFood() && m_Item->FoodNutritionValue())
		{
			// Item is food.
			if(IsInsideCapsule() && CapsuleRoomType() == ROOM_Fuel)
			{
				m_ItemLock.unlock();
				Kill();												// Items in spacecraft fuel cells are destroyed immediately.
				return;
			}
			else if(IsInAnyScene())
			{
				// Decay even if it's inside a vehicle or spacecraft.
				if(IsInVehicle())
				{
					// Treat enclosed vehicle like standard atmosphere.
					if(minTicked)
						decay = true;
				}
				else if(IsInsideSpacecraft())
				{
					// Carcasses in someone's scene may spawn a rat inside a spacecraft.
					// Static display items do not attract rats; the logic prior to this does not let them get this far.
					if(minTicked)
					{
						DaVessel* ship = FindParentVesselInEnv();
						if(ship && IsAirBreathable() && DbCommodity::IsScavengerFood(m_Item->CommodityId()))
							ship->RatEncounter(Location());
						decay = true;
					}
				}
				else if(IsInLava())
				{
					m_ItemLock.unlock();
					Kill();
					return;
				}
				else if(IsInVacuum())
				{
					if(IsInVisibleWater())
					{
						if(minTicked)
							decay = true;
					}
					else
					{
						if(last.ElapsedHourTicks(now) > 0)
							decay = true;
					}
				}
				else
				{
					switch(InAtmosphereType())
					{
					case ATMTYPE_Count:
						break;
					case ATMTYPE_Standard:
					case ATMTYPE_Tainted:
					case ATMTYPE_Exotic:
						if(IsInVisibleWater())
						{
							decay = true;
						}
						else if(InIndustry() != IID_None && IsAirBreathable() &&
								DbCommodity::IsScavengerFood(m_Item->CommodityId()))
						{
							// Carcasses in a breathable area of someone's scene may spawn a rat if the city has a rat infestation.
							// Static display items do not attract rats; the logic prior to this does not let them get this far.
							if(minTicked)
							{
							/*DHH DaCity
								DaWorldLL* planet = dynamic_cast<DaWorldLL*>(FindParentPlanetInEnv());
								if(planet)
								{
									const DaCity* city = planet->FindCity(InCityId());
									if(city)
									{
										const AuSitRats* rats =
											dynamic_cast<const AuSitRats*>(city->LockFindSituation(AuSitRats::SituationType()));
										if(rats)
											planet->RatEncounter(Location(), rats->DNA());	// Create a hungry rat there.
										city->UnlockSituations();
										decay = true;	// City is not infested with rats. Simply rot.
									}
									else
										decay = true;	// City not found? Not likely since we are in an industry.
								}
								else
							*/
									decay = true;	// Planet or terrain not found? Not likely since we are in an industry.
							}
						}
						else
						{
							if(minTicked)
								decay = true;
						}
						break;
					case ATMTYPE_Corrosive:
					case ATMTYPE_Insidious:
						decay = true;
						break;
					}
				}
			}
			else
			{
				// Decay quickly when not in anyone's scene.
				decay = true;
			}
		}
		else
		{
			// Item is not food.
			// Non-food items in spacecraft and vehicles do not decay. The parent will be subject
			// to its own decay rules, which will take the item with it when it goes.
			// Flower pots do not decay when they are at rest. They impose a flower pot population limit on the parent atom instead.
			if(IsInsideCapsule())
			{
				//qDebug() << "DaItem::Tock" << Id() << Name()
					//<< "in scene inside ship, combits" << m_Item->m_combits
					//<< "FoodNutritionValue" << m_Item->FoodNutritionValue()
					//<< "inVehicle" << IsInVehicle() << "inShip" << IsInsideSpacecraft()
					//<< "lastMin" << last.MinsSinceEpoch() << "curMin" << AuTime::CurrentMinsSinceEpoch();
				// Items in spacecraft are destroyed if they end up in a fuel cell.
				if(CapsuleRoomType() == ROOM_Fuel)
				{
					m_ItemLock.unlock();
					Kill();
					return;
				}
			}
			else if(!IsInVehicle())
			{
				// Non-food item is not inside an enclosed vehicle or spacecraft; apply environmental decay.
				if(IsInAnyScene() || (m_Item->Content() && !m_Item->Content()->empty()))
				{
					//qDebug() << "DaItem::Tock" << Id() << Name()
						//<< "in scene not in ship or vehicle, FoodNutritionValue" << m_Item->FoodNutritionValue()
						//<< "inVehicle" << IsInVehicle() << "inShip" << IsInsideSpacecraft()
						//<< "lastMin" << last.MinsSinceEpoch() << "curMin" << AuTime::CurrentMinsSinceEpoch();
					// Decay slowly while in anyone's scene or while a container has contents.
					if(IsInLava())
					{
						m_ItemLock.unlock();
						Kill();
						return;
					}
					else if(IsInVacuum())
					{
						if(IsInVisibleWater())
						{
							if(last.ElapsedHourTicks(now) > 0)
								decay = true;
						}
						else
						{
							if(last.ElapsedDayTicks(now) > 0)
								decay = true;
						}
					}
					else
					{
						switch(InAtmosphereType())
						{
						case ATMTYPE_Count:
							break;
						case ATMTYPE_Standard:
						case ATMTYPE_Tainted:
						case ATMTYPE_Exotic:
							if(IsInVisibleWater())
							{
								if(minTicked)
									decay = true;
							}
							else
							{
								if(last.ElapsedHourTicks(now) > 0)
									decay = true;
							}
							break;
						case ATMTYPE_Corrosive:
							if(IsInVisibleWater())
							{
								decay = true;
							}
							else
							{
								if(minTicked)
									decay = true;
							}
							break;
						case ATMTYPE_Insidious:
							decay = true;
							break;
						}
					}
				}
				else
				{
					// Decay quickly when not in anyone's scene and not a container with contents.
					decay = true;
				}
			}
		}
		m_ItemLock.unlock();

		// Decay on this item is imposed by "using" the item. When it is used up, it goes away.
		// This way quality of the item affects its decay time.
		if(decay)
			UseItem();
	}
	tockTrans.Add(DtTockTransaction(__FILE__, __LINE__, now));
}

QString DaItem::Name() const
{
	QString n;

	m_ItemLock.lockForRead();
	if(m_Item)
		n = m_Item->ListName();
	m_ItemLock.unlock();

	return n;
}

QString DaItem::SpotDescription(quint32 /*spotterEmpire*/) const
{
	QString s;

	m_ItemLock.lockForRead();
	if(m_Item)
		s = m_Item->SayName();
	m_ItemLock.unlock();

	return s;
}

bool DaItem::DepartingSolarSystem()
{
	// Items do not leave the solar system.
	Kill();
	return false;
}

void DaItem::CombatApplyDamage(const AuCombatAttacker& attacker, const AuCombatWeapon& weapon, const AuPoint& wpnLoc,
		const AuPoint& hitLoc, AuCombatResult& result)
{
	// Do not accept additional damage or return true if this atom is already dead or dying.
	if(!IsNormal())
		return;

	// Static display items cannot be damaged.
	if(IsStaticDisplay())
		return;

	if(!m_Item)
	{
		// This item is invalid.
		Kill();
		result.m_IsDestroyed = true;
		return;
	}

	// Items that are part of a story instance cannot be damaged. If the story becomes inactive, they self destruct in Tock.
	m_ItemLock.lockForRead();
	bool inStory = m_Item->StoryInstance();
	m_ItemLock.unlock();
	if(inStory)
		return;		// The item is part of a story.

	DaAtom::CombatApplyDamage(attacker, weapon, wpnLoc, hitLoc, result);

	m_ItemLock.lockForRead();
	int curHits = m_Item->HitPoints();
	m_ItemLock.unlock();

	if(result.m_Damage1)
	{
		// Certain weapon effects do not affect items.
		switch(weapon.m_w1.m_Effect)
		{
		case WE_None:
		case WE_Audible:
		case WE_Invalid:
		case WE_Bleeding:
		case WE_Disease:
		case WE_Suffocate:
		case WE_Stun:
		case WE_Poison:
		case WE_Radiation:
		case WE_Implant:
		case WE_Starving:
			break;
		case WE_Acid:
		case WE_Bludgeon:
		case WE_Cold:
		case WE_Corrosion:
		case WE_Crush:
		case WE_Electric:
		case WE_Unused:
		case WE_Heat:
		case WE_Pierce:
		case WE_Slash:
		case WE_Sonic:
		case WE_Generic:
			// Roll "saving throw" based on manufactured quality.
			if(g_Au.Dice().Roll(1,100) < Quality()/20)
				break;	// No damage.
			if((int)result.m_Damage1 >= curHits)
				curHits = 0;
			else
				curHits -= result.m_Damage1;
			break;
		}
	}
	if(result.m_Damage2 && curHits)
	{
		// Certain weapon effects do not affect items.
		switch(weapon.m_w2.m_Effect)
		{
		case WE_None:
		case WE_Audible:
		case WE_Invalid:
		case WE_Bleeding:
		case WE_Disease:
		case WE_Suffocate:
		case WE_Stun:
		case WE_Poison:
		case WE_Radiation:
		case WE_Implant:
		case WE_Starving:
			break;
		case WE_Acid:
		case WE_Bludgeon:
		case WE_Cold:
		case WE_Corrosion:
		case WE_Crush:
		case WE_Electric:
		case WE_Unused:
		case WE_Heat:
		case WE_Pierce:
		case WE_Slash:
		case WE_Sonic:
		case WE_Generic:
			if(g_Au.Dice().Roll(1,100) < Quality()/20)
				break;	// No damage.
			if((int)result.m_Damage2 >= curHits)
				curHits = 0;
			else
				curHits -= result.m_Damage2;
			break;
		}
	}

	//qDebug() << "DaItem::CombatApplyDamage" << weaponCID << weaponQ << wpnLoc << hitLoc << criticalHits
		//<< weapon.m_w1.m_Effect << result.m_Damage1 << kineticArmor1 << energyArmor1
		//<< weapon.m_w2.m_Effect << result.m_Damage2 << kineticArmor2 << energyArmor2
		//<< whoId << whoName << AuTime::Now() << "mass" << Mass() << "curhits" << curHits;

	// Destroy this atom.
	if(DoesExplodeWhenDamaged())
	{
		//qDebug() << "DaItem::CombatApplyDamage" << Id() << Name() << "detonation initiated";
		if(!attacker.m_Id.IsNull())
			ArmWeapon(attacker.m_Id, attacker.m_Name, attacker.m_EmpireId);	// So killer gets the credit.
		DetonateInitiate();									// Bombs explode when hit.
		result.m_IsDestroyed = true;
	}
	else if(!curHits)
	{
		Kill();
		result.m_IsDestroyed = true;
	}
	else
	{
		// Kick this object into motion.
		if(Mass() > 0 && wpnLoc != hitLoc)
		{
			// Calculate movement based on kinetic force applied.
			double movement = result.Armor1() + result.Armor2();
			if(IsKineticEffect(weapon.m_w1.m_Effect))
				movement += result.m_Damage1;
			if(IsKineticEffect(weapon.m_w2.m_Effect))
				movement += result.m_Damage2;
			if(movement > 0)
			{
				movement *= KineticDamageFtLbsPerSec() / Mass();
				movement /= 10;									// Must scale it back or the result is ridiculous.
				AuPoint fromDir = wpnLoc - hitLoc;
				fromDir.Unit(movement);

				SetVector(Vector() - fromDir);
				SetInMotion();
				TickTock();
			}
		}

		m_ItemLock.lockForWrite();
		if(curHits != m_Item->HitPoints())
			m_Item->SetHitPoints(curHits);
		m_ItemLock.unlock();

		SetModified();
	}
}

bool DaItem::DoesExplodeWhenDamaged() const
{
	m_ItemLock.lockForRead();
	bool does = m_Item->DoesExplodeWhenDamaged();
	m_ItemLock.unlock();
	return does;
}

// Fill the array with quads for determining hit locations when targeting this atom.
void DaItem::EnumSightSurfaces(AuQuadArray &aTargetSurfaces, const AuMatrix4x4 &tModel) const
{
	// Get the six surfaces created by the rough extents.
	AuBox fExt;
	if(RoughExtents(fExt))
	{
		fExt.Transform(tModel);
		fExt.GetQuads(aTargetSurfaces);
	}

	// Add two interior crisscross surfaces. This helps when the person is standing inside a crate or other large object.
	if(fExt.m_fCorners[0].Distance2(fExt.m_fCorners[6]) > 25)
	{
		aTargetSurfaces.AddQuad(fExt.m_fCorners[0], fExt.m_fCorners[5], fExt.m_fCorners[6], fExt.m_fCorners[3]);
		aTargetSurfaces.AddQuad(fExt.m_fCorners[2], fExt.m_fCorners[7], fExt.m_fCorners[4], fExt.m_fCorners[1]);
	}
}

void DaItem::SetAtRest()
{
	//qDebug("DaItem::SetAtRest %d %s", Id().GetID(), (const char*)Name().toAscii());
	DaAtom::SetAtRest();
	Orient(Location(), Transform().ProjectVector(TerrainNormal()));	// Instant orient when at rest.
}

void DaItem::operator>>(QDataStream& out) const
{
	DaAtom::operator>>(out);
	out << m_StaticDisplay;

	m_ItemLock.lockForRead();
	if(m_Item)
	{
		out << m_Item->Type();
		out << *m_Item;
	}
	else
		out << (quint8)0;
	out << AuTime();	//m_ItemChangeTime;
	m_ItemLock.unlock();

	m_OwnerLock.lockForRead();
	out << AuTime();	//m_OwnerChangeTime;
	out << m_OwnerEmpire;
	out << m_OwnerId;
	out << m_OwnerName;
	out << m_StaticRoom;
	m_OwnerLock.unlock();
}

void DaItem::operator<<(QDataStream& in)
{
	DaAtom::operator<<(in);
	in >> m_StaticDisplay;

	m_ItemLock.lockForWrite();
	if(m_Item)
		delete m_Item;
	quint8 itemType;
	in >> itemType;
	m_Item = AuItem::Create(itemType);
	if(m_Item)
	{
		in >> *m_Item;

		// Items rarely change type. e.g. Computer changed from a regular item to a storage device.
		AuItem* replacement = m_Item->FixItem();
		if(replacement)
		{
			delete m_Item;
			m_Item = replacement;
		}
	}
	AuTime itemChangeTime;
	in >> itemChangeTime;
	m_ItemLock.unlock();

	m_OwnerLock.lockForWrite();
	AuTime ownerChangeTime;
	in >> ownerChangeTime;
	in >> m_OwnerEmpire;
	in >> m_OwnerId;
	in >> m_OwnerName;
	in >> m_StaticRoom;
	m_OwnerLock.unlock();
}

void DaItem::TestHullStress()
{
	// Certain tests have already been made.
	Q_ASSERT(IsInAtmosphere());
	Q_ASSERT(!IsInVehicle());
	Q_ASSERT(!IsInsideSpacecraft());

	if(!IsCGSubmerged() && AirDensity() > .95f)
	{
		// High atmospheric pressure. Induce 10% damage per second.
		AuCombatAttacker attacker(tr("atmospheric pressure"));

		AuCombatWeapon weapon(WM_Internal);
		weapon.m_w1.m_Effect = WE_Crush;

		AuCombatResult result;
		result.m_Damage1 = 1;

		CombatApplyDamage(attacker, weapon, AuPoint(0,0), AuPoint(0,0), result);
		SetModified();
	}
}

// Begins detonation of an explosive. Detonation completes in the next Tick cycle by calling DetonateComplete.
// That way chain reactions of lots of bombs don't freeze the server while they all explode simultaneously.
void DaItem::DetonateInitiate()
{
	// Detonation is initiated by scheduling atom death. Tick watches for the dying state on explosives.
	//qDebug() << "DaItem::DetonateInitiate" << Id() << Name() << "owner" << m_OwnerId << m_OwnerName;
	Q_ASSERT(IsNormal());
	DieBy(AuTime::Now().AddSecs(1));
	TickTock();
}

void DaItem::DetonateTerminate()
{
	//qDebug() << "DaItem::DetonateTerminate BOOM!" << Id() << Name() << "owner" << m_OwnerId << m_OwnerName;
	// This item atom is killed immediately so it doesn't blow itself up again.
	if(!m_Item)
	{
		Kill();
		return;
	}
	SetDormant();		// So I am ignored by my explosion.

	// Create an effect atom to carry the explosion effect.
	DaAtom* parent = Parent();
	if(parent)
	{
		DaEffect* explode = parent->CreateEffect();

		// Blast center is slightly above my insertion point so it's not exactly on the floor or wall of a spaceship.
		AuPoint blastCenter = GroundUpVector() * .25;
		int doorDamage = 0;

		AuCombatWeapon weapon(WM_Not);
		m_ItemLock.lockForRead();
		if(m_Item)
		{
			if(m_Item->IsDetonatorWeapon() || m_Item->IsFuseWeapon())
				weapon = AuCombatWeapon(*m_Item, IPS_Weapon1);
			else if(m_Item->DoesExplodeWhenDamaged())
				weapon = AuCombatWeapon(*m_Item);
		}
		m_ItemLock.unlock();

		if(weapon.m_Mode == WM_Area || weapon.m_Mode == WM_Detonator || weapon.m_Mode == WM_Fuse)
		{
			float gain = MAX(weapon.m_w1.m_Radius, weapon.m_w2.m_Radius) / 5.f;
			AuEffect effect(Location(), Rotation(), Transform().ProjectVector(Vector()), SOUND_HitExplosion, gain, 1,
					VISUAL_Explosion);
			if(weapon.m_w1.m_Effect != WE_None && weapon.m_w1.m_Radius > 0)
				effect.SetExplosionEffect1(weapon.m_w1.m_Effect, weapon.m_w1.m_Radius);
			if(weapon.m_w2.m_Effect != WE_None && weapon.m_w2.m_Radius > 0)
				effect.SetExplosionEffect2(weapon.m_w2.m_Effect, weapon.m_w2.m_Radius);
			explode->AddEffect(effect);
			if(IsInSpacecraft() && (weapon.m_Mode == WM_Detonator || weapon.m_Mode == WM_Fuse))
				doorDamage = g_Au.Dice().Roll(1, weapon.m_w1.m_Damage, weapon.m_w1.m_DamageMod);

			// Bombs are not detonated by pirates.
			m_OwnerLock.lockForRead();
			AuCombatAttacker attacker(m_OwnerId, m_OwnerName, m_OwnerEmpire);
			m_OwnerLock.unlock();

			AuCombatDefender defender(blastCenter, false);

			AuCombatResult result;

			AttackTarget(attacker, defender, weapon, true, result);
		}

		MakeCrater(AuPoint(0,0), weapon);

		// Detonated and fused weapons may blow open a door when they are aboard a ship.
		if(doorDamage)
		{
			// Blast affects doors and this bomb is subordinate to a spacecraft.
			DaCapsule* ship = dynamic_cast<DaCapsule*>(parent);
			if(ship)
			{
				// I am directly subordinate to the spacecraft. Check for a door in range to jam open.
				double doorDist;
				int closestDoor = ship->ClosestDoor(Location(), doorDist);
				if(closestDoor >= 0 && doorDist < FeetPerMeter())
				{
					// There is a door close enough to open with the blast.
					float baseAbsorb, baseBlock, auxAbsorb, auxBlock;
					ship->CombatArmorEffectiveness(WM_Contact, WE_Bludgeon, false, Location(), Location(), baseAbsorb, baseBlock,
							auxAbsorb, auxBlock);

					// Reduce door damage based on armor, including shields for hull doors.
					if(ship->IsHullDoor(closestDoor))
					{
						// Hull doors are protected by hull armor and shields.
						doorDamage -= (int)((baseAbsorb + auxAbsorb) * doorDamage + (baseBlock + auxBlock))/2;
					}
					else
					{
						// Interior doors are only protected by the hull, not shields.
						doorDamage -= (int)(baseAbsorb * doorDamage + baseBlock)/2;
					}

					// Any damage remaining results in the door jamming open.
					if(doorDamage > 0)
						ship->JamDoorOpen(closestDoor);		// Blast open the door.
				}
			}
		}
	}
	Kill();
}

void DaItem::MakeCrater(const AuPoint& loc, const AuCombatWeapon& weapon)
{
	if(!IsInAtmosphere())
		return;
	if(IsInSpacecraft())
		return;
	if(IsInVehicle())
		return;
	float craterRadius = weapon.CraterRadius();
	if(craterRadius < FeetPerMeter()*2)
		return;

	float agl = AltitudeAGL();
	if(agl > craterRadius)
		return;

	// Create a bomb crater on the world.
	DaWorldLL* world = dynamic_cast<DaWorldLL*>(Parent());
	if(!world)
		return;

	float debrisHeight = craterRadius / 5;
	if(agl > 0)
	{
		// Reduce crater radius due to altitude.
		float frac = agl / craterRadius;						// 0=ground level, 1=crater radius
		float ang = acosf(frac);								// 0=PI, 1=0
		float sf = sinf(ang);
		craterRadius *= sf;
		debrisHeight *= sf;
	}
	float bottomMSL = AltitudeMSL() - craterRadius * .66f;		// Bottom of the crater.
	world->AddBombCrater(Transform().Project(loc), bottomMSL, craterRadius, debrisHeight);
}

DaAtom* DaItem::SightTarget(const DaAtom* sightingAtom, const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, DaAtom* pBest,
		AuPoint& sightedLoc, unsigned int& sightedPart, bool& sightedShield, AuPoint* sightedSurfaceNormal) const
{
	//qDebug() << "DaItem::SightTarget" << Id() << Name();
	Q_UNUSED(sightedSurfaceNormal);
	if(!IsSightTarget(sightingAtom))
		return pBest;
	if(SightTargetDetails(eyeLoc, tModel, sightedLoc, sightedPart, sightedShield))
	{
		pBest = (DaAtom*)this;			// Must cast me to non-const.
		if(sightedSurfaceNormal)
			*sightedSurfaceNormal = AuPoint(0,0);
	}
	return pBest;
}

// 'hit' is the atom that was hit.
// 'hitNormal' is the surface normal at the point of collision, relative to this atom.
// Returns the undampened bounce vector. The length is the speed in feet per second that this atom was moving when the hit
// occurred. It is simply the reflected movement vector when the hit occurred.
AuPoint DaItem::MovementBlocked(DaAtom* hit, const AuPoint& hitLoc, const AuPoint& hitNormal, bool hitShield)
{
	if(hitShield)
		Kill();	// All items are destroyed if moving fast enough to hit a shield bubble.
	if(hitNormal.IsZero())
	{
		SetVector(AuPoint(0,0));
		return AuPoint(0,0);
	}
	return DaAtom::MovementBlocked(hit, hitLoc, hitNormal, hitShield);
}

bool DaItem::IsInScene(const DaPerson* sceneAtom, const AuPoint& cameraEnvLoc) const
{
	Q_UNUSED(cameraEnvLoc);
	switch(CommodityId())
	{
	case CID_PressureTent:
	case CID_Tepee:
		return DistanceToAtom2(sceneAtom, 0) < (400*FeetPerMeter() * 400*FeetPerMeter());
	case CID_BurningBarrel:
		return DistanceToAtom2(sceneAtom, 0) < (500*FeetPerMeter() * 500*FeetPerMeter());
	case CID_Drum:          // So I can hear it a long way away.
	case CID_StoneFirePit:
		return DistanceToAtom2(sceneAtom, 0) < (750*FeetPerMeter() * 750*FeetPerMeter());
	case CID_Beacon:
		if(ItemState() == 1)
			return DistanceToAtom2(sceneAtom, 0) < (750*FeetPerMeter() * 750*FeetPerMeter());
		// Not blinking so use default range.
		break;
	default:
		break;
	}
	return DistanceToAtom2(sceneAtom, 0) < (100*FeetPerMeter() * 100*FeetPerMeter());
}

void DaItem::UseItem()
{
	if(!m_Item)
		return;
	if(IsStaticDisplay())
		return;

	m_ItemLock.lockForWrite();
	if(m_Item->UseItem())
	{
		m_ItemLock.unlock();
		SetModified();	// Item was not fully consumed.
	}
	else if(m_Item->CommodityId() == CID_Animal || m_Item->CommodityId() == CID_Head)
	{
		// Animal carcass and head turns into a bone. Since both are AuItemDNA objects, the DNA is preserved.
		m_Item->SetCommodity(CID_Bone, m_Item->Quality());
		m_Item->ResetCondition();
		m_ItemChangeTime.Set();
		m_ItemLock.unlock();

		SetInMotion();
		TickTock();
		SetModified();

		Orient(Location(), Transform().ProjectVector(TerrainNormal()));
	}
	else
	{
		m_ItemLock.unlock();
		Kill();							// Item was consumed.
	}
}

// 'sightedPart' will be the index into the array of surfaces returned by EnumSightSurfaces.
// Returns true if this atom is hit and 'sightedLoc' and 'sightedPart' are output.
bool DaItem::SightTargetDetails(const AuPoint& eyeLoc, const AuMatrix4x4 &tModel, AuPoint& sightedLoc,
		unsigned int& sightedPart, bool& sightedShield) const
{
	// Return value.
	bool hitMe = false;

	// Get the sightable surfaces.
	AuQuadArray solidSurfs;
	EnumSightSurfaces(solidSurfs, tModel);

	// Find the closest surface intersection.
	unsigned int part = 0;
	AuPoint intersect, normal;
	for(AuQuadArray::const_iterator it = solidSurfs.constBegin(); it != solidSurfs.constEnd(); it++, part++)
	{
		if((*it).TestSegmentIntersection(intersect, normal, eyeLoc, sightedLoc))
		{
			// We found an intersection with a solid surface.
			// Move the look location to the intersection.
			sightedLoc = intersect;
			sightedPart = part;
			sightedShield = false;
			hitMe = true;
		}
	}
	return hitMe;
}

void DaItem::GetRenderLightsRecursive(const DaPerson& person, const AuMatrix4x4& model, QList<AuGLSun>& suns,
		QList<AuGLLight>& lights) const
{
	if(m_Item->IsLight() && m_Item->State() == 1)
	{
		// I am a light that is switched on.
		// Calculate flickering light effect for torch.
		float lightFlicker = m_Item->LightFlicker(Id());
		if(IsInsideCapsule())
			lightFlicker /= 4.f;		// Tone down lights a lot when aboard a spacecraft or inside a building.

		AuPoint lightPos = model.Project(m_Item->LightPosition());
		QColor colLight = m_Item->LightColor();
		float lightRadius = m_Item->LightRadius() * lightFlicker;
		float kQ = 1.f/(lightRadius * lightRadius);
		float kL = .0065f;
		lights.append(AuGLLight(Id(), AuGLLight::Item, kL,kQ, colLight, lightPos));
	}
	DaAtom::GetRenderLightsRecursive(person, model, suns, lights);
}

// 'blips' is the sensor blip array that is accumulating the scan data. Locations are relative to the solar system.
// 'scannerLoc' is the location of the atom who is performing the scan, relative to the solar sytem.
// 'scannerId' is the id of the atom who is performing the scan. Currently either a ship or a city.
// 'scannerEmpire' is the empire id of the atom who is performing the scan.
// 'scannerSensorRange' is the range in feet of the scanner atom's sensors.
// 'scannerWeaponRange' is the range in feet of the scanner atom's bay/base weapons.
// 'scanContent' is a bitmask of data that is sought in the scan
//   b0 = sensor console blips
//   b1 = fire control console blips
//   b2 = transporter console blips
//   b3 = locked-on blip
//   b4 = AI pilot obstacle blips
//   b5 = AI turret gunner target blips
//   b6 = Scanning spacecraft wants its own subatoms to be scanned.
// 'lockedBlip' is the id of the currently locked sensor blip
void DaItem::SensorScan(AuSensorBlipList& blips, const AuPoint& scannerLoc, const AuId& scannerId, quint32 /*scannerEmpire*/,
		int scannerDomain, const AuId& scannerWorld,
		float /*scannerSensorRange*/, float /*scannerWeaponRange*/, int scanContent,
		const AuId& lockedBlip, unordered_map<quint32, POLITICAL_STANCE, AuEmpireIdHash>& /*stanceCache*/) const
{
	Q_UNUSED(scannerLoc);
	// Active beacon is the only item detected by sensors so far.
	if(CommodityId() != CID_Beacon || ItemState() == 0)
		return;
	if(IsStaticDisplay())
		return;

	// Must scan me if looking for locked-on blip and I'm it.
	if((!(scanContent & 0x0008) || !Is(lockedBlip)))
	{
		// Not looking for locked-on blip or I am not the locked-on blip.
		// I am only scanned for sensor consoles.
		if(!(scanContent & 0x0001))
			return;
	}

	// Scanners don't detect things inside the photon sphere.
	if(InPhotonSphere())
		return;

	if(IsInSpacecraft())
		return;	// Beacons don't show up when aboard a spacecraft.

	// Atoms in far domains do not show up on sensors.
	if(!IsGunnerDomainTarget(scannerDomain, scannerWorld, scannerId))
		return;

	blips.Add(AuBlipBeacon(BeaconMessage(), Id(), EnvLocation()));
}

bool DaItem::IsControl(const DaAtom* sceneAtom) const
{
	Q_UNUSED(sceneAtom);
	return true;
}

// Returns true if this is a static display item that can be controlled by 'personId'.
bool DaItem::IsStaticDisplayControllableBy(const AuId& personId, quint32 personEmpireId) const
{
	// Is this item even a static display item?
	if(!IsStaticDisplay())
		return false;

	// Person who placed the item on display always has control.
	if(m_OwnerId == personId)
		return true;

	// If this item is aboard a ship, an officer of the ship can control it.
	if(IsInSpacecraft())
	{
		// Aboard a ship, an officer of the ship can control this static display item.
		const DaVessel* ship = FindParentVesselInEnv();
		if(ship)
			return ship->IsOfficer(personId);
	}
	else if(!BdId().IsNull())
	{
		// In a city, an person in the chain-of-command of the city can control this static display item.
		const DaWorld* world = FindParentPlanetInEnv();
		if(world)
		{
			const DaBd* bd = world->FindBd(BdId());
			if(bd)
				return bd->InChainOfCommand(personId, personEmpireId);
		}
	}

	// Not in a ship or city so anybody can control this static display item.
	return true;
}

void DaItem::RenderAudioSources(const DaPerson* sceneAtom, const AuMatrix4x4& model, AuSound* al, float fade)
{
	if(!al)
		return;
	if(!m_Item || !m_Item->HasSound())
		return;

	if(m_Item->IsAudible())
	{
		// Emit the sound.
		float gain = RelativeAudioGain(sceneAtom) * fade;
		float pitch = 1;
		m_Item->RenderSound(model, *al, gain, pitch);
	}
	else
		m_Item->RenderSound(model, *al, 0, 0);	// Turn off the sound.
}

void DaItem::StoryMakePermanent(quint32 storyInstance)
{
	// Check to see if this item is associated with the story instance.
	m_ItemLock.lockForRead();
	quint32 myStory = m_Item->StoryInstance();
	m_ItemLock.unlock();
	if(myStory != storyInstance)
		return;	// I am not in that story.

	// Make this item permanent by clearing the story instance id.
	m_ItemLock.lockForWrite();
	m_Item->SetStoryInstance(0);
	m_ItemLock.unlock();
	SetModified();
}

void DaItem::StorySetDropOnDeath(quint32 storyInstance, int drop)
{
	// Check to see if this item is associated with the story instance.
	m_ItemLock.lockForRead();
	quint32 myStory = m_Item->StoryInstance();
	m_ItemLock.unlock();
	if(myStory != storyInstance)
		return;	// I am not in that story.

	// Make this item permanent by clearing the story instance id.
	m_ItemLock.lockForWrite();
	m_Item->SetDropOnDeath(drop);
	m_ItemLock.unlock();
	SetModified();
}

void DaItem::SetItemMessage(const QString& text)
{
	m_ItemLock.lockForWrite();
	m_Item->SetMessage(text);
	m_ItemChangeTime.Set();
	m_ItemLock.unlock();
	SetModified();
}

void DaItem::SetItemState(quint8 state)
{
	m_ItemLock.lockForWrite();
	m_Item->SetState(state);
	m_ItemChangeTime.Set();
	m_ItemLock.unlock();
	SetModified();
}

QString DaItem::BeaconMessage() const
{
	// Build the broadcast message.
	QString im = ItemMessage();
	QString text;
	if(IsInAtmosphere())
	{
		const DaWorld* world = FindParentPlanetInEnv();
		const DaWorldLL* llworld = dynamic_cast<const DaWorldLL*>(world);
		if(llworld)
		{
			if(im.isEmpty())
				text = tr("%1 at %2").arg(llworld->Name()).arg(GlobeLL().ToString());
			else
				text = tr("%3 : %1 at %2").arg(llworld->Name()).arg(GlobeLL().ToString()).arg(im);
		}
		else
		{
			if(im.isEmpty())
				text = world->Name();
			else
				text = QString("%2 : %1").arg(world->Name()).arg(im);
		}
	}
	else if(IsInSpacecraft())
	{
		const DaVessel* ship = FindParentVesselInEnv();
		if(ship)
		{
			if(im.isEmpty())
				text = ship->NameWithEmpire();
			else
				text = QString("%2 : %1").arg(ship->NameWithEmpire()).arg(im);
		}
	}
	return text;
}

TRANSPORTER_TARGET DaItem::TransporterTarget() const
{
	switch(CommodityId())
	{
	case CID_Beacon:
		if(IsStaticDisplay())
			break;
		if(ItemState() == 0)
			break;						// Beacon must be on to be a transporter target.
		return TT_Beacon;
	default:
		break;
	}
	return TT_Invalid;
}

QString DaItem::TransporterTargetName() const
{
	QString placeName = tr("%1 at %2")
		.arg(g_Au.CommodityName(CommodityId()))
		.arg(GlobeLL().ToString());
	return placeName;
}

quint64 DaItem::MemoryUsage() const
{
	quint64 mem = DaAtom::MemoryUsage();
	mem += sizeof(m_StaticDisplay);
	mem += sizeof(m_ItemLock);
	mem += sizeof(m_Item);
	if(m_Item)
		mem += m_Item->MemoryUsage();
	mem += sizeof(m_ItemChangeTime);
	mem += sizeof(m_OwnerLock);
	mem += sizeof(m_OwnerChangeTime);
	mem += sizeof(m_OwnerEmpire);
	mem += sizeof(m_OwnerId);
	mem += StringMemoryUsage(m_OwnerName);
	mem += StringMemoryUsage(m_StaticRoom);
	mem += sizeof(m_LastForage);
	return mem;
}

void DaItem::BuildingSurrendered(quint32 surrenderingEmpire, quint32 toEmpire)
{
	Q_UNUSED(surrenderingEmpire);
	Q_UNUSED(toEmpire);
}

double DaItem::FallSafeSpeed() const
{
	return 10 * FeetPerMeter();
}

void DaItem::FindBeaconsRecursive(DaAtomList& bds) const
{
	//if(!IsNormal())
		//return;
	DaAtom::FindBeaconsRecursive(bds);
	if(CommodityId() == CID_Beacon)
		bds.append((DaItem*)this);	// Cast away constness.
}

void DaItem::FindItemsRecursive(DaAtomList& items, bool includeStaticDisplayedItems) const
{
	//if(!IsNormal())
		//return;
	DaAtom::FindItemsRecursive(items, includeStaticDisplayedItems);
	if(includeStaticDisplayedItems || !IsStaticDisplay())
		items.append((DaItem*)this);	// Cast away constness.
}

bool DaItem::PlantSeed(const AuItem& seedItem)
{
	if(IsStaticDisplay())
		return false;
	if(!IsFlowerPot())
		return false;
	bool planted = false;
	m_ItemLock.lockForWrite();
	AuItemFloraDNA* pot = dynamic_cast<AuItemFloraDNA*>(m_Item);
	if(pot)
	{
		planted = pot->PlantSeedInPot(seedItem);
		if(planted)
			m_ItemChangeTime.Set();
	}
	m_ItemLock.unlock();
	if(planted)
		SetModified();
	return planted;
}

void DaItem::ClaimTitleRecursive(const AuId& propertyId, const QString& propertyName, const AuId& ownerId,
		const QString& ownerName, quint32 ownerEmpire)
{
	DaAtom::ClaimTitleRecursive(propertyId, propertyName, ownerId, ownerName, ownerEmpire);
	if(IsDead())
		return;

	bool anyChange = false;
	m_ItemLock.lockForWrite();
	if(m_Item)
		anyChange = m_Item->VoidOwnershipTitle(propertyId, ownerId, ownerName);
	m_ItemLock.unlock();
	if(anyChange)
		SetModified();
}

DaAtom::ATOM_TYPE DaItem::AtomType() const
{
	return ATOM_Item;
}

float DaItem::GroundHeightAboveCG() const
{
	//return g_Au.Commodity(CommodityId()).m_insertHeight + 1./12.;	// Let items settle a little bit above the ground.
	m_ItemLock.lockForRead();
	float cgHt = m_Item->GroundHeightAboveCG();
	m_ItemLock.unlock();
	return cgHt;
}

AuPoint DaItem::GroundUpVector() const
{
	m_ItemLock.lockForRead();
	AuPoint vup(m_Item->GroundUpVector());
	m_ItemLock.unlock();
	return vup;
}

AuDNABase::CHROMATID DaItem::DNAChromatid() const
{
	m_ItemLock.lockForRead();
	const AuItemDNA* dnaItem = dynamic_cast<const AuItemDNA*>(m_Item);
	AuDNABase::CHROMATID ch = dnaItem ? dnaItem->DNAChromatid() : AuDNABase::Alpha;
	m_ItemLock.unlock();
	return ch;
}

QString DaItem::CapsuleRoomName() const
{
	// Static displayed items sometimes end up outside of a room void so they won't have a room name other than "Hull".
	// To get the lighting right, we use the room in which the displayer was standing when they displayed me.
	QString room;
	if(IsStaticDisplay() && CapsuleRoomType() == ROOM_Hull)
	{
		// I am on static display and I am not inside a room.
		m_OwnerLock.lockForRead();
		if(!m_StaticRoom.isEmpty())
			room = m_StaticRoom;
		m_OwnerLock.unlock();
	}
	if(!room.isEmpty())
		return room;
	return DaAtom::CapsuleRoomName();
}

bool DaItem::IsEthereal() const
{
	return false;
}

bool DaItem::OpensAutomaticDoors() const
{
	if(!IsNormal())
		return false;
	return !IsAtRest();
}

