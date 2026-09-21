//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumodel.cpp,v 1.133 2022/04/15 17:01:21 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aumodel.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auview.h"
#include "daatomlist.h"
#include "dabd.h"
#include "dagalaxy.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "davehicle.h"
#include "daworldll.h"

#define d_SyncInterval					10						// millisecs between syncronizations with data

AuModel::AuModel(QObject* parent) : QThread(parent), m_Ether("AuModel::AuModel"), m_semAlive(1)
{
	m_AllStarsPresent = false;

	m_PosHeadTilt = 0;
	m_PosParentStationId = 0;
	for(int ps=0; ps<DpBody::PS_Count; ps++)
	{
		m_PosPose[ps] = AuPose::Init;
		m_PosParentPose[ps] = AuPose::Init;
		m_PrevPose[ps] = AuPose::Init;
		m_PrevParentPose[ps] = AuPose::Init;
	}
	m_PosParentValid = false;
	m_PosParentWhichConsole = 0;
	m_PosSeatIndex = 0;
	m_PosStationId = 0;
	m_PosValid = false;
	m_PosWhichConsole = 0;

	m_PrevHeadTilt = 0;
	m_PrevParentStationId = 0;
	m_PrevParentWhichConsole = 0;
	m_PrevSeatIndex = 0;
	m_PrevStationId = 0;
	m_PrevWhichConsole = 0;

	m_semAlive.acquire(1);			// I live...
}

AuModel::~AuModel()
{
}

// Reconcile local model against real data.
void AuModel::run()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
	{
		qDebug() << "warning: AuModel::run no view, exiting";
		return;
	}

	AuElapsedTimer elapseTimer;
	while(Alive())
	{
		elapseTimer.start();
		if(g_AuClient.SyncAtomCount() < 100)
		{
			// Sync queue isn't flooded so give it some more.
			// Locate the scene atom in the root data.
			// The server only sends us 'normal' atoms.
			// An atom will be set to 'traveling' instead of 'normal' when it enters a wormhole or deadheads during local client
			// movement.
			// The server disconnects us when it is notified of the wormhole/deadhead.
			// We then reconnect at our new location to get fresh 'normal' atoms and we're off to the races once again.
			AuEther& ether = g_Au.LockEtheriumDelete();
			DaPerson* sceneRef = dynamic_cast<DaPerson*>(ether.FindNormalAtom(auApp->AvatarId()));
			if(!sceneRef)
			{
				// Do not send position updates when the scene atom is not found or is not 'normal'.
				g_Au.UnlockEtheriumDelete();
				msleep(d_SyncInterval);
				continue;
			}
			DaAtom* sceneControl = sceneRef->ParentControlAtom();
			if(sceneControl && !sceneControl->IsNormal())
			{
				// Do not send position updates when the atom controlled by the scene atom is not 'normal'.
				g_Au.UnlockEtheriumDelete();
				msleep(d_SyncInterval);
				continue;
			}

			if(auApp->IsSingleUserMode())
			{
				AuTime startTime(AuTime::Now());
#if !defined(QT_NO_DEBUG)
				// Debug build plays in architect mode.
				sceneRef->SetOnLine(ACCT_Architect, 0, startTime);
#else
				// Release build plays in normal player mode.
				sceneRef->SetOnLine(ACCT_Active, 0, startTime);
#endif
				DaSolarSystem* system = sceneRef->FindParentSolarSystem();
				system->SetLastPersonScene();

				// Keep the star map up-to-date with the current unit's empire id. If it changes, the star map is erased.
				bool mapCleared = g_AuClient.StarMapSetEmpire(sceneRef->EmpireId());	// Erases star map if empire changes.
				if(mapCleared)
					emit StarMapErased();

				DaGalaxy* galaxy = system->FindParentGalaxy();
				if(galaxy)
				{
					DaAtomList sectors;
					ether.ListNormalSubatoms(galaxy->Id(), sectors);
					DaSector* parSector = sceneRef->FindParentSector();
					for(DaAtomList::const_iterator si = sectors.cbegin(); si != sectors.cend(); si++)
					{
						// Update star map with any sectors that are not in the star map already.
						const DaSector* sector = dynamic_cast<const DaSector*>(*si);
						if(sector && (sector == parSector || sector->IsNeighbor(*parSector)))
						{
							if(!g_AuClient.StarMapHasSector(sector->Id()))
								emit StarMapAddSector(sector->Id());
							continue;
						}
					}
				}

				// Update list of non-permanent scene atoms.
				g_AuClient.m_SceneAtomIds.clear();
				if(sceneRef->InDesignEnvironment())
				{
					sceneRef->ResetHunger();							// Don't starve to death in the off-line design studio.
					sceneRef->ResetStamina();
					sceneRef->SetModified();
				}
			}
			else
			{
				AuElapsedTimer tnow;
				tnow.start();
				if(auApp->IsHazeronCommander())
				{
					// TickOnClient resolves building designs, paths, etc. at the root level; it does not move or reparent atoms.
					ether.TickOnClient(m_LastTick);
				}
				else
				{
					// Restore locally known position of scene atom and the atom it is controlling, if any.
					RestoreScenePosition(sceneRef);

					// TickOnClient resolves building designs, paths, etc. at the root level; it does not move or reparent atoms.
					ether.TickOnClient(m_LastTick);

					// Tick movement of the scene atom and the atom controlled by the scene atom for smooth client-side movement.
					sceneRef->TickMovement(m_LastTick);
					if(sceneControl)
						sceneControl->TickMovement(m_LastTick);

					// Preserve locally calculated position of scene atom and the atom it is controlling, if any.
					PreserveScenePosition(sceneRef);

					// Transmit current position report to server.
					TransmitPosition();
				}
				m_LastTick = tnow;
			}

			// Update list of non-permanent scene atoms.
			g_AuClient.m_SceneAtomIds.clear();
			DaAtomList sceneAtoms;
			sceneRef->ListSceneAtoms(sceneAtoms);
			if(auApp->IsSingleUserMode())
			{
				for(DaAtomList::const_iterator si = sceneAtoms.begin(); si != sceneAtoms.end(); si++)
				{
					DaAtom* pa = *si;
					g_AuClient.m_SceneAtomIds.insert(pair<AuId, bool>(pa->Id(), true));

					// Reset atom scene flag when it appears in a scene.
					pa->SetInScene();

					// Manage political stance.
					quint32 empireId = pa->EmpireId();
					if(empireId)
					{
						// Monitor for units of other empires so they can become "known" to the scene atom's empire.
						DaUnit* unit = dynamic_cast<DaUnit*>(pa);
						if(unit)
						{
							// The atom is a unit with an empire id.
							if(m_AtomEmpires.find(empireId) == m_AtomEmpires.cend())
							{
								// Empire id does not exist in the list of empires in our scene.
								// Test political stance at least once with all empires we encounter, to make contact.
								sceneRef->PoliticalStanceMakeContact(unit);

								// Add the unit's empire id to the list of empires in the scene and the list of empires we've made
								// contact with.
								m_AtomEmpires.insert(pair<quint32, bool>(empireId, true));
								m_AtomStances.insert(pair<quint32, bool>(empireId, true));
							}
							else if(m_AtomStances.find(empireId) == m_AtomStances.cend())
							{
								// Test political stance at least once with all empires we encounter, to make contact.
								sceneRef->PoliticalStanceMakeContact(unit);

								// Add the unit's empire id to the list of empires we've made contact with.
								m_AtomStances.insert(pair<quint32, bool>(empireId, true));
							}
						}
						else
						{
							// The atom is not a unit but it has an empire id, most likely a world that was claimed.
							if(m_AtomEmpires.find(empireId) == m_AtomEmpires.cend())
							{
								// Add the world's empire id to the list of empires in the scene.
								m_AtomEmpires.insert(pair<quint32, bool>(empireId, true));
							}
						}
					}
				}
			}
			else
			{
				for(DaAtomList::const_iterator si = sceneAtoms.begin(); si != sceneAtoms.end(); si++)
					g_AuClient.m_SceneAtomIds.insert(pair<AuId, bool>((*si)->Id(), true));
			}

			// Star Field
			DaSector* parSector = (DaSector*)sceneRef->FindParentSector();
			if(parSector && (!m_AllStarsPresent || parSector->Id() != m_idSyncSector))
			{
				// Scene atom's sector was found in the assigned data.
				// It is not the same sector we used for the star field last time.
				// It will query its own neighbors to provide the needed star field.
				DaGalaxy* galaxy = parSector->FindParentGalaxy();
				if(galaxy)
					galaxy->InvalidateSectorLocCache();						// Cache will be recreated when needed.
				m_AllStarsPresent = parSector->ListStarFieldSubatoms(sceneRef, sceneAtoms);
			}

			// Reconcile remaining atoms in the model against the list of atoms that are supposed to be in the scene.
			for(DaAtomList::iterator it = sceneAtoms.begin(); it != sceneAtoms.end(); it++)
			{
				// Find the atom in the model.
				DaAtom* dAtom = *it;
				Q_ASSERT(dAtom);
				DaAtom* mAtom = m_Ether.FindAtom(dAtom->Id());
				if(mAtom)
				{
					// The atom is already present in the model so update it if it has changed.
					// We must always update the scene atom and the atom it is controlling because local smoothing touches
					// the modify time.
					if(mAtom->CompareModifyTime(dAtom->ModifyTime()) != 0 || dAtom == sceneRef || dAtom == sceneControl)
					{
						// We must get the parent of the model atom to know its row index.
						// Change parent if necessary.
						if(mAtom->ParentId() != dAtom->ParentId())
						{
							// The parent of the atom has changed.
							mAtom->ChangePos(mAtom->Pos());
							mAtom->InvalidateEnvironment();

							// Refresh the model atom using the data atom before notifying the view.
							mAtom->Refresh(dAtom, 0);
							g_AuClient.SyncAtomParentChanged(mAtom);
							dAtom->RefreshComplete();
							mAtom->RefreshComplete();	// Safe to do now because the client made a clone.
						}
						else
						{
							// Refresh the model atom using the data atom.
							mAtom->Refresh(dAtom, 0);
							g_AuClient.SyncAtomChanged(mAtom);
							dAtom->RefreshComplete();
							mAtom->RefreshComplete();	// Safe to do now because the client made a clone.
						}
					}
				}
				else
				{
					// The atom is not present in the model so add it.
					DaAtom* newsub = dAtom->Clone();
					newsub->ConnectSignals();
					m_Ether.AddAtom(newsub);

					// Dirty the galaxy sector locs cache when adding an atom. We only ever add sectors to galaxy atoms.
					DaGalaxy* galaxy = dynamic_cast<DaGalaxy*>(m_Ether.FindAtom(newsub->ParentId()));
					if(galaxy)
						galaxy->InvalidateSectorLocCache();			// Cache will be recreated when needed.
					g_AuClient.SyncAtomAdded(newsub);
					dAtom->RefreshComplete();
					newsub->RefreshComplete();
				}
			}

			// Remove unneeded atoms after repositioning and reparenting them.
			const DaAtom* pSceneAtom = m_Ether.FindAtom(auApp->AvatarId());
			SyncRemove(AuId::Universe(), pSceneAtom);

			// We remember the sector of the scene atom for reference when we sync the
			// next scene. Reacquire this info because it may have changed during the sync.
			parSector = (DaSector*)sceneRef->FindParentSector();
			if(parSector)
				m_idSyncSector = parSector->Id();
			else
				m_idSyncSector = AuId();
			g_Au.UnlockEtheriumDelete();
		}

		// Process atom and UI controls in the view.
		pView->ProcessControls();

		// Delete an atom from the trash, if any are in there. I don't have a separate trash thread for this ether.
		m_Ether.DeleteTrashAtom();

		// Wait for the time needed to try to even out the cycle time.
		int waitMs = d_SyncInterval - elapseTimer.elapsed();
		if(waitMs <= 0)
			waitMs = 1;
		msleep(waitMs);
	}
}

// Remove atoms from the model that are no longer needed in the scene.
// This function is used when syncronizing a parent atom with the real scene data.
// The parent atom is not modified, other than by killing children.
// The parent calls this function recursively.
void AuModel::SyncRemove(const AuId& parentId, const DaAtom* pSceneAtom)
{
	DaAtomList subs;
	m_Ether.ListSubatoms(parentId, subs);
	if(subs.empty())
		return;

	// The atom has subordinates.
	// Must SyncRemove the subordinates first so they are removed from the
	// bottom of the tree up to the top due to recursion.
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		if(!(*it)->HasSubatoms())
			continue;	// Skip atoms with no children.
		SyncRemove((*it)->Id(), pSceneAtom);
	}

	// Insure each is in the list of atoms that are supposed to be in the scene.
	for(DaAtomList::const_iterator it = subs.cbegin(); it != subs.cend(); it++)
	{
		DaAtom* atom = *it;
		if(atom->IsPermanentInScene())
			continue;					// Skip atom that is permanent in the scene.
		if(atom->HasSubatoms())
			continue;					// Skip atoms with children.
		AuId atomId = atom->Id();
		if(g_AuClient.m_SceneAtomIds.find(atomId) != g_AuClient.m_SceneAtomIds.end())
			continue;					// Atom is in the hash of non-permanent scene atoms so it belongs in the scene.
		if(pSceneAtom && pSceneAtom->FindParentAtomId(atomId))
			continue;					// Atom is a direct ascendent of the scene atom.

		// The subatom is no longer required in the scene.
		DaBd* bd = dynamic_cast<DaBd*>(atom);
		if(bd && bd->HasEnclosedEnvironment())
		{
			// Enclosure updates parent world's cache when it departs.
			DaWorldLL* world = bd->FindParentWorldInEnv();
			m_Ether.TrashAtom(atomId);
			if(world)
			{
				world->InvalidateEnclosureCache();
				world->InvalidateEnvironmentRecursive();
			}
		}
		else
			m_Ether.TrashAtom(atomId);

		// Scene is notified after we clean up root's copy of the atom.
		g_AuClient.SyncAtomDeparted(atomId);
	}
}

void AuModel::ConnectionEstablished()
{
	//qDebug() << "AuModel::ConnectionEstablished";
	// Clear sync data so we'll behave properly with the new server.
	m_Ether.DeleteEther();
	m_idSyncSector = AuId();
	g_AuClient.SyncAtomsCleared();

	// Reset position values to match the constructor defaults on the scene server.
	m_PosLock.lock();

	m_PosValid = false;
	m_PosParentValid = false;

	m_PosHeadTilt = 0;
	m_PrevHeadTilt = 0;

	m_Pos = AuAtomPos();
	m_PrevPos = AuAtomPos();

	m_PosParent = AuAtomPos();
	m_PrevParentPos = AuAtomPos();

	m_PosParentStationId = 0;
	m_PrevParentStationId = 0;

	for(int ps=0; ps<DpBody::PS_Count; ps++)
	{
		m_PosPose[ps] = AuPose::Init;
		m_PosParentPose[ps] = AuPose::Init;
		m_PrevPose[ps] = AuPose::Init;
		m_PrevParentPose[ps] = AuPose::Init;
	}

	m_PosParentVec = AuPoint();
	m_PrevParentVec = AuPoint();

	m_PosParentWhichConsole = 0;
	m_PrevParentWhichConsole = 0;

	m_PosSeatIndex = 0;
	m_PrevSeatIndex = 0;

	m_PosStationId = 0;
	m_PrevStationId = 0;

	m_PosVec = AuPoint();
	m_PrevVec = AuPoint();

	m_PosWhichConsole = 0;
	m_PrevWhichConsole = 0;

	m_PosLock.unlock();

	// State of all abnormal atoms needs to be reset to normal.
	// If I drive a ship through a wormhole, it ends up in Travelling state locally.
	AuEther& ether = g_Au.LockEtheriumDelete();
	ether.ResetAtomsToNormal();
	g_Au.UnlockEtheriumDelete();
}

void AuModel::TransmitPosition()
{
	if(!auApp->SceneSyncReset())
		return;

	// Update current pos values.
	m_PosLock.lock();
	if(!m_PosValid)
	{
		m_PosLock.unlock();
		return;
	}
	if(m_Pos.ParentId().IsNull())
	{
		m_PosLock.unlock();
		return;
	}

	quint16 hasWhat = 0;
	if(m_PosHeadTilt != m_PrevHeadTilt)
	{
		hasWhat |= 0x0001;
		m_PrevHeadTilt = m_PosHeadTilt;
	}
	if(m_Pos != m_PrevPos)
	{
		hasWhat |= 0x0002;
		m_PrevPos = m_Pos;
	}

	for(int ps=0; ps<DpBody::PS_Count; ps++)
		if(m_PosPose[ps] != m_PrevPose[ps])
		{
			hasWhat |= 0x0008;
			break;
		}
	if(m_PosWhichConsole != m_PrevWhichConsole || m_PosStationId != m_PrevStationId)
		hasWhat |= 0x0008;
	if(hasWhat & 0x0008)
	{
		for(int ps=0; ps<DpBody::PS_Count; ps++)
			m_PrevPose[ps] = m_PosPose[ps];
		m_PrevStationId = m_PosStationId;
		m_PrevWhichConsole = m_PosWhichConsole;
	}

	if(m_PosVec != m_PrevVec)
	{
		hasWhat |= 0x0010;
		m_PrevVec = m_PosVec;
	}
	if(m_PosAtRest)
		hasWhat |= 0x0020;
	m_PrevAtRest = m_PosAtRest;
	if(m_PosSeatIndex != m_PrevSeatIndex)
	{
		hasWhat |= 0x0040;
		m_PrevSeatIndex = m_PosSeatIndex;
	}

	if(m_PosParentValid && !m_PosParent.ParentId().IsNull())
	{
		hasWhat |= 0x0100;
		if(m_PosParent != m_PrevParentPos)
		{
			hasWhat |= 0x0200;
			m_PrevParentPos = m_PosParent;
		}

		for(int ps=0; ps<DpBody::PS_Count; ps++)
			if(m_PosParentPose[ps] != m_PrevParentPose[ps])
			{
				hasWhat |= 0x0800;
				break;
			}
		if(m_PosParentWhichConsole != m_PrevParentWhichConsole || m_PosParentStationId != m_PrevParentStationId)
			hasWhat |= 0x0800;
		if(hasWhat & 0x0800)
		{
			for(int ps=0; ps<DpBody::PS_Count; ps++)
				m_PrevParentPose[ps] = m_PosParentPose[ps];
			m_PrevParentStationId = m_PosParentStationId;
			m_PrevParentWhichConsole = m_PosParentWhichConsole;
		}

		if(m_PosParentVec != m_PrevParentVec)
		{
			hasWhat |= 0x1000;
			m_PrevParentVec = m_PosParentVec;
		}
		if(m_PosParentAtRest)
			hasWhat |= 0x2000;
		m_PrevParentAtRest = m_PosParentAtRest;
	}
	else
	{
		m_PrevParentPos = AuAtomPos();
		for(int ps=0; ps<DpBody::PS_Count; ps++)
			m_PrevParentPose[ps] = AuPose::Init;
		m_PrevParentVec = AuPoint();
		m_PrevParentAtRest = false;
	}

	// Transmit the position report to the server.
	auApp->SendPosition(hasWhat, m_PrevHeadTilt, m_PrevPos, m_PrevPose, m_PrevWhichConsole, m_PrevStationId, m_PrevVec,
			m_PrevSeatIndex, m_PrevParentPos, m_PrevParentPose, m_PrevParentWhichConsole, m_PrevParentStationId, m_PrevParentVec);
	m_PosLock.unlock();
}

void AuModel::PreserveScenePosition(DaPerson* sceneAtom)
{
	if(sceneAtom->NeedsInit())
	{
		m_PosLock.lock();
		m_PosValid = false;
		m_Pos = AuAtomPos();
		m_PosLock.unlock();
		return;
	}

	// Update current position values for scene atom.
	m_PosLock.lock();
	m_PosValid = true;
	m_PosId = sceneAtom->Id();
	m_PosAtRest = sceneAtom->IsAtRest();
	m_PosHeadTilt = sceneAtom->CurrentHeadTilt();
	m_Pos = sceneAtom->Pos();
	if(sceneAtom->HasDNA3())
		sceneAtom->CurrentPoses3(m_PosPose);
	else
		sceneAtom->CurrentPoses4(m_PosPose);
	m_PosSeatIndex = sceneAtom->VehicleSeatIndex();
	m_PosWhichConsole = sceneAtom->ControlId();
	m_PosStationId = sceneAtom->StationId();
	m_PosVec = sceneAtom->Vector();

	// Update current position values for atom controlled by the scene atom.
	DaAtom* sceneControl = sceneAtom->ParentControlAtom();
	m_PosParentValid = sceneControl ? true : false;
	if(m_PosParentValid)
	{
		m_PosParentAtRest = sceneControl->IsAtRest();
		m_PosParent = sceneControl->Pos();
		DaCreature* mount = dynamic_cast<DaCreature*>(sceneControl);
		if(mount)
		{
			if(mount->HasDNA3())
				mount->CurrentPoses3(m_PosParentPose);
			else
				mount->CurrentPoses4(m_PosParentPose);
			m_PosParentStationId = mount->StationId();
			m_PosParentWhichConsole = mount->ControlId();
		}
		else
		{
			for(int ps=0; ps<DpBody::PS_Count; ps++)
				m_PosParentPose[ps] = AuPose::Init;
			m_PosParentStationId = 0;
			m_PosParentWhichConsole = 0;
		}
		m_PosParentVec = sceneControl->Vector();
	}
	else
	{
		m_PosParentAtRest = false;
		m_PosParent = AuAtomPos();
		for(int ps=0; ps<DpBody::PS_Count; ps++)
			m_PosParentPose[ps] = AuPose::Init;
		m_PosParentStationId = 0;
		m_PosParentVec = AuPoint();
		m_PosParentWhichConsole = 0;
	}
	m_PosLock.unlock();
}

void AuModel::RestoreScenePosition(DaPerson* sceneAtom)
{
	// Do not restore when the scene atom changed since we stored the settings. That causes strange problems.
	m_PosLock.lock();
	if(!sceneAtom->Is(m_PosId))
	{
		m_PosLock.unlock();
		return;
	}
	else if(sceneAtom->IsBodyDyingOrDead())
	{
		m_PosLock.unlock();
		return;
	}
	else if(sceneAtom->NeedsInit())
	{
		m_PosLock.unlock();
		return;
	}

	// Tick movement of scene atom for smooth client-side movement.
	if(m_PosValid)
	{
		// Change my restore parameters to match those in the sync controller data.
		if(sceneAtom->HasDNA3())
		{
			AuPose sp = m_PosPose[DpBody::PS_LowerTorso];
			sceneAtom->ControllerSync(m_Pos, m_PosAtRest, m_PosVec, m_PosHeadTilt, sp, m_PosStationId, m_PosSeatIndex);
			if(sp != m_PosPose[DpBody::PS_LowerTorso])
			{
				// Sync pose change means general pose changed.
				sceneAtom->SetGeneralPose(sp);
				sceneAtom->CurrentPoses3(m_PosPose);
			}
		}
		else
		{
			AuPose sp = m_PosPose[AuAnimalPart::PS_General];
			sceneAtom->ControllerSync(m_Pos, m_PosAtRest, m_PosVec, m_PosHeadTilt, sp, m_PosStationId, m_PosSeatIndex);
			if(sp != m_PosPose[AuAnimalPart::PS_General])
			{
				// Sync pose change means general pose changed.
				sceneAtom->SetGeneralPose(sp);
				sceneAtom->CurrentPoses4(m_PosPose);
			}
		}

		// Position data is valid for scene atom.
		sceneAtom->TiltHeadTo(m_PosHeadTilt);
		sceneAtom->ChangePos(m_Pos);

		// Insure my mating on bottom pose is correct.
		bool chggp = false;
		AuPose gp(sceneAtom->CurrentGeneralPose());
		if(gp == AuPose::ReceptiveMate)
		{
			// If I have a mate on top, change my current pose.
			const DaCreature* rider = dynamic_cast<const DaCreature*>(sceneAtom->Rider());
			if(rider)// && rider->CurrentGeneralPose() == AuPose::MateOnTop)
			{
				gp = AuPose::MateOnBottom;	// Change my current gp to mate on the bottom.
				chggp = true;
			}
		}

		if(sceneAtom->HasDNA3())
		{
			if(chggp)
				m_PosPose[DpBody::PS_LowerTorso] = gp;
			sceneAtom->SetPoses3(m_PosPose, m_PosStationId, m_PosWhichConsole);	//DHH animalia
		}
		else
		{
			// Preserve mouth pose. It is managed on the server.
			AuPose curPose[AuAnimalPart::PS_Count];
			sceneAtom->CurrentPoses4(curPose);
			m_PosPose[AuAnimalPart::PS_Mouth] = curPose[AuAnimalPart::PS_Mouth];
			if(chggp)
				m_PosPose[AuAnimalPart::PS_General] = gp;
			sceneAtom->SetPoses4(m_PosPose, m_PosStationId, m_PosWhichConsole);	//DHH animalia
		}
		if(m_PosAtRest)
			sceneAtom->SetAtRest();
		else
		{
			sceneAtom->SetInMotion();
			sceneAtom->SetVector(m_PosVec);
		}

		// We can get the correct seat index from the parent vehicle based on my position.
		int seatIndex;
		DaVehicle* vehicle = dynamic_cast<DaVehicle*>(sceneAtom->Parent());
		if(vehicle)
			seatIndex = vehicle->SeatAt(sceneAtom->Location());
		else
			seatIndex = m_PosSeatIndex;
		sceneAtom->SetVehicleSeatIndex(seatIndex);
		sceneAtom->InvalidateEnvironment();
	}

	if(m_PosParentValid)
	{
		// Position data is valid for atom controlled by the scene atom and it is still subordinate to the same parent as when the
		// data was grabbed.
		DaUnit* sceneControl = sceneAtom->ParentControlAtom();
		if(sceneControl && sceneControl->Is(m_Pos.ParentId()))
		{
			// Change parent restore parameters to match those in the sync controller data.
			DaCreature* mount = dynamic_cast<DaCreature*>(sceneControl);
			bool dna3;
			float headTilt;
			quint8 seatIndex;
			if(mount)
			{
				dna3 = mount->HasDNA3();
				headTilt = mount->CurrentHeadTilt();
				seatIndex = mount->VehicleSeatIndex();
			}
			else
			{
				dna3 = false;
				headTilt = 0;
				seatIndex = 0;
			}
			if(dna3)
				sceneControl->ControllerSync(m_PosParent, m_PosParentAtRest, m_PosParentVec, headTilt,
						m_PosParentPose[DpBody::PS_LowerTorso], m_PosParentStationId, seatIndex);
			else
				sceneControl->ControllerSync(m_PosParent, m_PosParentAtRest, m_PosParentVec, headTilt,
						m_PosParentPose[AuAnimalPart::PS_Mouth], m_PosParentStationId, seatIndex);

			if(!mount || !mount->IsBodyDyingOrDead())
			{
				// Position data is valid for controlled atom.
				sceneControl->TiltHeadTo(headTilt);
				sceneControl->ChangePos(m_PosParent);
				if(mount)
				{
					if(dna3)
						mount->SetPoses3(m_PosParentPose, m_PosParentStationId, m_PosParentWhichConsole);	//DHH animalia
					else
					{
						// Preserve mouth pose. It is managed on the server.
						AuPose curPose[AuAnimalPart::PS_Count];
						mount->CurrentPoses4(curPose);
						m_PosParentPose[AuAnimalPart::PS_Mouth] = curPose[AuAnimalPart::PS_Mouth];
						mount->SetPoses4(m_PosParentPose, m_PosParentStationId, m_PosParentWhichConsole);	//DHH animalia
					}
				}
				if(m_PosParentAtRest)
					sceneControl->SetAtRest();
				else
				{
					sceneControl->SetInMotion();
					sceneControl->SetVector(m_PosParentVec);
				}
				sceneControl->InvalidateEnvironment();
			}
		}
	}
	m_PosLock.unlock();
}

