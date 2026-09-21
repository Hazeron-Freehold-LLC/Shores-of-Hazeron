//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audbglobal.cpp,v 1.90 2022/05/16 22:30:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audbglobal.h"
//
#include "auether.h"
#include "aubddesign.h"
#include "auscdesign.h"
#include "dgdata.h"
#include "dtsceneserver.h"

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDbGlobal::AuDbGlobal(const QString& appName, const QString& company, AuGlobal::MODE mode, DgData* data, AuEther* ether) :
	AuGlobal(appName, company, mode, data, ether)
{
	m_HasCriticalUpdates = false;
	m_RestockAtomCount = 0;				// ids in unused stock

	m_NextEmpireId = 0;					// 0=invalid

	m_NextBuildingDesignId = 0;			// 0=invalid
	m_RestockBuildingDesignId = 0;		// first id in unused stock
	m_RestockBuildingDesignCount = 0;	// ids in unused stock

	m_NextItemDesignId = 0;				// 0=invalid
	m_RestockItemDesignId = 0;			// first id in unused stock
	m_RestockItemDesignCount = 0;		// ids in unused stock

	m_NextSpaceShipDesignId = 0;		// 0=invalid
	m_RestockSpaceShipDesignId = 0;		// first id in unused stock
	m_RestockSpaceShipDesignCount = 0;	// ids in unused stock

	m_NextStoryId = 0;
	m_RestockStoryId = 0;
	m_RestockStoryCount = 0;

	if(IsSceneMaster())
	{
		m_LastAtomId = AuId((quint32)-1);
		m_LastBuildingDesignId = (quint32)-1;	// last assigned from boss
		m_LastItemDesignId = (quint32)-1;		// last assigned from boss
		m_LastSpaceShipDesignId = (quint32)-1;	// last assigned from boss
		m_LastStoryId = (quint32)-1;
	}
	else
	{
		m_LastBuildingDesignId = 0;		// last assigned from boss
		m_LastItemDesignId = 0;			// last assigned from boss
		m_LastSpaceShipDesignId = 0;	// last assigned from boss
		m_LastStoryId = 0;
	}
}

void AuDbGlobal::InitNextIds(const AuId& nextAtom, quint32 nextEmpire, quint32 nextSpaceShipDesign, quint32 nextStory,
		quint32 nextBuildingDesign, quint32 nextItemDesign)
{
	//Q_ASSERT(IsSceneMaster());
	m_NextAtomId = nextAtom;
	m_NextBuildingDesignId = nextBuildingDesign;
	m_NextEmpireId = nextEmpire;
	m_NextItemDesignId = nextItemDesign;
	m_NextSpaceShipDesignId = nextSpaceShipDesign;
	m_NextStoryId = nextStory;

	QString logmsg;
	logmsg = QObject::tr("Next Ids Initialized - atom %1, building design %5, empire %2, "
			"space ship design %3, story %4, item %6")
		.arg(m_NextAtomId.GetID())
		.arg(m_NextEmpireId)
		.arg(m_NextSpaceShipDesignId)
		.arg(m_NextStoryId)
		.arg(m_NextBuildingDesignId)
		.arg(m_NextItemDesignId);
	g_Au.Log(logmsg);
}

AuId AuDbGlobal::NextAtomId(unsigned int howMany)
{
	m_muNextAtomId.lock();
	if(m_NextAtomId + howMany > m_LastAtomId)
	{
		//qDebug("AuDbGlobal::NextAtomId - next %d howMany %d > last %d restock "
				//"id %d count %d", m_NextAtomId.GetID(), howMany, m_LastAtomId.GetID(),
				//m_RestockAtomId.GetID(), m_RestockAtomCount);
		// The number requested exceeds my ready stock.
		if(m_RestockAtomCount)
		{
			// Rotate back stock of ids to the ready stock.
			m_NextAtomId = m_RestockAtomId - 1;
			m_LastAtomId = m_NextAtomId + m_RestockAtomCount;

			// Clear back stock. DtClientChannel will refill it.
			m_RestockAtomId = 0;
			m_RestockAtomCount = 0;
		}
		else
		{
			// This is a critical problem!
			m_muNextAtomId.unlock();
			g_Au.Error(QString("NextAtomId overrun. Next %1 howMany %2 Last %3. Terminating now!")
					.arg(m_NextAtomId.GetID()).arg(howMany).arg(m_LastAtomId.GetID())); 
			exit(0);
			return 0;
		}
	}

	AuId nextId = m_NextAtomId + 1;
	m_NextAtomId = m_NextAtomId + howMany;
	m_muNextAtomId.unlock();
	return nextId;
}

quint32 AuDbGlobal::NextEmpireId()
{
	Q_ASSERT(m_NextEmpireId != 0);
	m_muNextEmpireId.lock();
	m_NextEmpireId++;
	m_muNextEmpireId.unlock();
	return m_NextEmpireId;
}

quint32 AuDbGlobal::NextBuildingDesignId(unsigned int howMany)
{
	m_muNextBuildingDesignId.lock();
	if(m_NextBuildingDesignId + howMany > m_LastBuildingDesignId)
	{
		// The number requested exceeds my ready stock.
		if(m_RestockBuildingDesignCount)
		{
			// Rotate back stock of ids to the ready stock.
			m_NextBuildingDesignId = m_RestockBuildingDesignId - 1;
			m_LastBuildingDesignId = m_NextBuildingDesignId + m_RestockBuildingDesignCount;

			// Clear back stock. DtClientChannel will refill it.
			m_RestockBuildingDesignId = 0;
			m_RestockBuildingDesignCount = 0;
		}
		else
		{
			// This is a critical problem!
			m_muNextBuildingDesignId.unlock();
			g_Au.Error(QString("NextBuildingDesignId overrun. Next %1 howMany %2 Last %3.")
					.arg(m_NextBuildingDesignId).arg(howMany).arg(m_LastBuildingDesignId)); 
			return 0;
		}
	}

	quint32 nextId = m_NextBuildingDesignId + 1;
	m_NextBuildingDesignId += howMany;
	m_muNextBuildingDesignId.unlock();
	return nextId;
}

quint32 AuDbGlobal::NextSpaceShipDesignId(unsigned int howMany)
{
	m_muNextSpaceShipDesignId.lock();
	if(m_NextSpaceShipDesignId + howMany > m_LastSpaceShipDesignId)
	{
		// The number requested exceeds my ready stock.
		if(m_RestockSpaceShipDesignCount)
		{
			// Rotate back stock of ids to the ready stock.
			m_NextSpaceShipDesignId = m_RestockSpaceShipDesignId - 1;
			m_LastSpaceShipDesignId = m_NextSpaceShipDesignId +
				m_RestockSpaceShipDesignCount;

			// Clear back stock. DtClientChannel will refill it.
			m_RestockSpaceShipDesignId = 0;
			m_RestockSpaceShipDesignCount = 0;
		}
		else
		{
			// This is a critical problem!
			m_muNextSpaceShipDesignId.unlock();
			g_Au.Error(QString("NextSpaceShipDesignId overrun. Next %1 howMany %2 Last %3.")
					.arg(m_NextSpaceShipDesignId).arg(howMany).arg(m_LastSpaceShipDesignId)); 
			return 0;
		}
	}

	quint32 nextId = m_NextSpaceShipDesignId + 1;
	m_NextSpaceShipDesignId += howMany;
	m_muNextSpaceShipDesignId.unlock();
	return nextId;
}

quint32 AuDbGlobal::NextStoryId(unsigned int howMany)
{
	m_muNextStoryId.lock();
	if(m_NextStoryId + howMany > m_LastStoryId)
	{
		// The number requested exceeds my ready stock.
		if(m_RestockStoryCount)
		{
			// Rotate back stock of ids to the ready stock.
			m_NextStoryId = m_RestockStoryId - 1;
			m_LastStoryId = m_NextStoryId + m_RestockStoryCount;

			// Clear back stock. DtClientChannel will refill it.
			m_RestockStoryId = 0;
			m_RestockStoryCount = 0;
		}
		else
		{
			// This is a critical problem!
			m_muNextStoryId.unlock();
			g_Au.Error(QString("NextStoryId overrun. Next %1 howMany %2 Last %3.")
					.arg(m_NextStoryId).arg(howMany).arg(m_LastStoryId)); 
			return 0;
		}
	}

	quint32 nextId = m_NextStoryId + 1;
	m_NextStoryId += howMany;
	m_muNextStoryId.unlock();
	return nextId;
}

// Restocks the "back stock" of ids.
void AuDbGlobal::RestockAtomIds(const AuId& nextId, unsigned int howMany)
{
	//Q_ASSERT(!IsSceneMaster());
	m_muNextAtomId.lock();
	m_RestockAtomId = nextId;
	m_RestockAtomCount = howMany;
	m_muNextAtomId.unlock();
}

// Restocks the "back stock" of ids.
void AuDbGlobal::RestockBuildingDesignIds(quint32 nextId, unsigned int howMany)
{
	//Q_ASSERT(!IsSceneMaster());
	m_muNextBuildingDesignId.lock();
	m_RestockBuildingDesignId = nextId;
	m_RestockBuildingDesignCount = howMany;
	m_muNextBuildingDesignId.unlock();
}

// Restocks the "back stock" of ids.
void AuDbGlobal::RestockSpaceShipDesignIds(quint32 nextId, unsigned int howMany)
{
	//Q_ASSERT(!IsSceneMaster());
	m_muNextSpaceShipDesignId.lock();
	m_RestockSpaceShipDesignId = nextId;
	m_RestockSpaceShipDesignCount = howMany;
	m_muNextSpaceShipDesignId.unlock();
}

// Restocks the "back stock" of ids.
void AuDbGlobal::RestockStoryIds(quint32 nextId, unsigned int howMany)
{
	//Q_ASSERT(!IsSceneMaster());
	m_muNextStoryId.lock();
	m_RestockStoryId = nextId;
	m_RestockStoryCount = howMany;
	m_muNextStoryId.unlock();
}

void AuDbGlobal::LoadScripts()
{
	QSet<quint32> bdDesigns;
	QSet<quint32> scDesigns;
	m_rwScripts.lockForWrite();
	m_Scripts.LoadAllLib();
	m_Scripts.ListDesignsNeeded(bdDesigns, scDesigns);
	m_rwScripts.unlock();

	// After scripts are loaded, trigger load of designs that are used by scripts.
	for(QSet<quint32>::const_iterator bdi = bdDesigns.cbegin(); bdi != bdDesigns.cend(); bdi++)
	{
		AuBdDesign design;
		const DgData& bdd = g_Au.LockBdDesignsForRead();
		bdd.BdDesign(*bdi, design);		// Dr. Karlov's ship.
		g_Au.UnlockBdDesigns();
		qDebug() << "AuDbGlobal::LoadScripts requested load of bd blueprint" << *bdi;
	}
	for(QSet<quint32>::const_iterator sci = scDesigns.cbegin(); sci != scDesigns.cend(); sci++)
	{
		AuScDesign design;
		const DgData& scd = g_Au.LockScDesignsForRead();
		scd.ScDesign(*sci, design);		// Dr. Karlov's ship.
		g_Au.UnlockScDesigns();
		qDebug() << "AuDbGlobal::LoadScripts requested load of sc blueprint" << *sci;
	}
}

void AuDbGlobal::LoadScript(quint32 scriptId)
{
	if(!scriptId)
		return;
	m_rwScripts.lockForWrite();
	m_Scripts.Load(scriptId);
	m_rwScripts.unlock();
}

QString AuDbGlobal::StoryAuthor(quint32 storyInstance)
{
	// Determine the script id of the story instance.
	unsigned int scriptId;
	m_rwStories.lockForRead();
	AuStories::const_iterator it = m_Stories.find(storyInstance);
	if(it != m_Stories.cend())
		scriptId = (*it).second->ScriptId();
	else
		scriptId = 0;
	m_rwStories.unlock();

	// Get the story author from the script.
	QString author;
	if(scriptId)
	{
		m_rwScripts.lockForRead();
		AuScripts::const_iterator sit = m_Scripts.find(scriptId);
		if(sit != m_Scripts.cend())
			author = (*sit).second.Author();
		m_rwScripts.unlock();
	}
	return author;
}

QString AuDbGlobal::StoryBrief(quint32 storyInstance)
{
	// Determine the script id of the story instance.
	unsigned int scriptId;
	m_rwStories.lockForRead();
	AuStories::const_iterator it = m_Stories.find(storyInstance);
	if(it != m_Stories.cend())
		scriptId = (*it).second->ScriptId();
	else
		scriptId = 0;
	m_rwStories.unlock();

	// Get the story brief from the script.
	QString brief;
	if(scriptId)
	{
		m_rwScripts.lockForRead();
		AuScripts::const_iterator sit = m_Scripts.find(scriptId);
		if(sit != m_Scripts.cend())
			brief = (*sit).second.Brief();
		m_rwScripts.unlock();
	}
	return brief;
}

QString AuDbGlobal::StoryTitle(quint32 storyInstance)
{
	// Determine the script id of the story instance.
	unsigned int scriptId;
	m_rwStories.lockForRead();
	AuStories::const_iterator it = m_Stories.find(storyInstance);
	if(it != m_Stories.cend())
		scriptId = (*it).second->ScriptId();
	else
		scriptId = 0;
	m_rwStories.unlock();

	// Get the story title from the script.
	QString title;
	if(scriptId)
	{
		m_rwScripts.lockForRead();
		AuScripts::const_iterator sit = m_Scripts.find(scriptId);
		if(sit != m_Scripts.cend())
			title = (*sit).second.Title();
		m_rwScripts.unlock();
	}
	return title;
}

bool AuDbGlobal::IsStoryActive(quint32 storyInstance)
{
	m_rwStories.lockForRead();
	bool isActive = (m_Stories.find(storyInstance) == m_Stories.cend()) ? false : true;
	m_rwStories.unlock();
	return isActive;
}

void AuDbGlobal::StoryAdded(quint32 storyInstance)
{
	m_muDbStoryIds.lock();

	// If by chance the story has already found its way into the deleted list,
	// remove it from the deleted and changed lists and don't add it to the add
	// list.
	int idxDel = m_DeletedStoryIds.indexOf(storyInstance);
	if(idxDel >= 0)
	{
		// Remove it from the deleted list.
		m_DeletedStoryIds.removeAt(idxDel);	// Remove it from the delete list.
	}
	else if(!m_AddedStoryIds.contains(storyInstance))
	{
		// Add it to the nuly added list so it will be saved by the db thread.
		//qDebug("AuDbGlobal::StoryAdded - id %d", storyInstance);
		m_AddedStoryIds.append(storyInstance);
	}

	// Remove it from the change list.
	// Sometimes an atom has already been added to the change list because it
	// gets modified before actually being added to a parent atom.
	int idxChg = m_ChangedStoryIds.indexOf(storyInstance);
	if(idxChg >= 0)
		m_ChangedStoryIds.removeAt(idxChg);	// Remove it from the change list.

	m_muDbStoryIds.unlock();
}

void AuDbGlobal::StoryChanged(quint32 storyInstance)
{
	m_muDbStoryIds.lock();

	// No need to update the story if it hasn't been inserted into the db yet.
	// The insert will save all its data.
	bool addPending = m_AddedStoryIds.contains(storyInstance);
	if(!addPending)
	{
		// No need to update the story if it is scheduled for deletion.
		bool delPending = m_DeletedStoryIds.contains(storyInstance);
		if(!delPending)
		{
			// Add the story to the changed list so it will be updated by the
			// db thread.
			if(!m_ChangedStoryIds.contains(storyInstance))
			{
				//qDebug("AuDbGlobal::StoryChanged - id %d", storyInstance);
				m_ChangedStoryIds.append(storyInstance);
			}
		}
	}

	m_muDbStoryIds.unlock();
}

void AuDbGlobal::StoryDeleted(quint32 storyInstance)
{
	m_muDbStoryIds.lock();

	// No need to update the story if it's going to be deleted.
	int idxChg = m_ChangedStoryIds.indexOf(storyInstance);
	if(idxChg >= 0)
		m_ChangedStoryIds.removeAt(idxChg);	// Remove it from the change list.

	// No need to add the story if it's going to be deleted.
	int idxAdd = m_AddedStoryIds.indexOf(storyInstance);
	if(idxAdd >= 0)
	{
		// Remove it from the add list.
		m_AddedStoryIds.removeAt(idxAdd);
	}

	// Delete the story even if it was never added, so appropriate signals are generated to finalize the cleanup of a deleted story.
	if(!m_DeletedStoryIds.contains(storyInstance))
	{
		// Add the story to the delete stories list so it will be deleted by the db thread.
		//qDebug("AuDbGlobal::StoryDeleted - id %d", storyInstance);
		m_DeletedStoryIds.append(storyInstance);
	}

	m_muDbStoryIds.unlock();
	PostCriticalUpdates();		// Trigger update right away so everything gets cleanup up quickly.
}

void AuDbGlobal::RemoveStory(quint32 storyInstance)
{
	m_muDbStoryIds.lock();

	// Add the story to the remove list so it will be removeed by the db
	// thread.
	if(!m_RemoveStoryIds.contains(storyInstance))
		m_RemoveStoryIds.append(storyInstance);

	// Remove the story from the reload queue.
	int idxDel = m_ReloadStoryIds.indexOf(storyInstance);
	if(idxDel >= 0)
		m_ReloadStoryIds.removeAt(idxDel);

	m_muDbStoryIds.unlock();
}

void AuDbGlobal::ReloadStory(quint32 storyInstance)
{
	m_muDbStoryIds.lock();

	// Add the story to the reload list so it will be reloaded by the db
	// thread.
	if(!m_ReloadStoryIds.contains(storyInstance))
		m_ReloadStoryIds.append(storyInstance);

	m_muDbStoryIds.unlock();
}

AuStory* AuDbGlobal::LockStory(quint32 storyInstance)
{
	m_rwStories.lockForRead();					// Insure stories container does not change while the story is locked.
	return m_Stories.LockStory(storyInstance);	// Return the story pointer once the lock is acquired.
}

void AuDbGlobal::UnlockStory(quint32 storyInstance)
{
	m_Stories.UnlockStory(storyInstance);		// Unlock the story mutex.
	m_rwStories.unlock();						// Unlock the stories container.
}

void AuDbGlobal::AtomAdded(const AuId& idAtom)
{
	m_muAtomTransactions.lock();
	// If by chance the atom has already found its way into the deleted list,
	// remove it from the deleted and changed lists and don't add it to the add
	// list.
	int idxDel = m_DeletedAtomIds.indexOf(idAtom);
	if(idxDel >= 0)
	{
		// Remove it from the deleted list.
		m_DeletedAtomIds.removeAt(idxDel);	// Remove it from the delete list.
	}
	else if(!m_AddedAtomIds.contains(idAtom))
	{
		// Add it to the nuly added list so it will be saved by the db thread.
		//qDebug("AuDbGlobal::AtomAdded - id %d", idAtom.GetID());
		m_AddedAtomIds.append(idAtom);
	}

	// Remove it from the change list.
	// Sometimes an atom has already been added to the change list because it
	// gets modified before actually being added to a parent atom.
	int idxChg = m_ChangedAtomIds.indexOf(idAtom);
	if(idxChg >= 0)
		m_ChangedAtomIds.removeAt(idxChg);	// Remove it from the change list.
	m_muAtomTransactions.unlock();
}

void AuDbGlobal::AtomChanged(const AuId& idAtom)
{
	m_muAtomTransactions.lock();
	// No need to update the atom if it hasn't been inserted into the db yet.
	// The insert will save all its data.
	bool addPending = m_AddedAtomIds.contains(idAtom);
	if(!addPending)
	{
		// No need to update the atom if it is scheduled for deletion.
		bool delPending = m_DeletedAtomIds.contains(idAtom);
		if(!delPending)
		{
			// Add the atom to the changed list so it will be updated by the
			// db thread.
			if(!m_ChangedAtomIds.contains(idAtom))
			{
				//qDebug("AuDbGlobal::AtomChanged - id %d", idAtom.GetID());
				m_ChangedAtomIds.append(idAtom);
			}
		}
	}

	// If we got a change notification for an atom, then it must be removed from the departed list.
	// It could have departed and then arrived back again before we had a chance to update the database.
	m_DepartedAtoms.Remove(idAtom);

	m_muAtomTransactions.unlock();
}

void AuDbGlobal::AtomDeleted(const AuId& idAtom)
{
	m_muAtomTransactions.lock();
	// No need to update the atom if it's going to be deleted.
	int idxChg = m_ChangedAtomIds.indexOf(idAtom);
	if(idxChg >= 0)
		m_ChangedAtomIds.removeAt(idxChg);	// Remove it from the change list.

	// No need to delete the atom if it hasn't been added to the db yet.
	// We can simply remove it from the add list.
	int idxAdd = m_AddedAtomIds.indexOf(idAtom);
	if(idxAdd >= 0)
	{
		// Remove it from the add list.
		m_AddedAtomIds.removeAt(idxAdd);
	}
	else if(!m_DeletedAtomIds.contains(idAtom))
	{
		// Add the atom to the delete atoms list so it will be deleted
		// by the db thread.
		//qDebug("AuDbGlobal::AtomDeleted - id %d", idAtom.GetID());
		m_DeletedAtomIds.append(idAtom);
	}

	// If we got a delete notification for an atom, then it must be removed from the departed list.
	// It could have departed and then arrived back again before we had a chance to update the database.
	m_DepartedAtoms.Remove(idAtom);

	m_muAtomTransactions.unlock();
}

void AuDbGlobal::AtomDeparted(const AuId& id)
{
	m_muAtomTransactions.lock();
	m_DepartedAtoms.AddIfUnique(id);
	m_HasCriticalUpdates = true;					// This needs to trigger an update right away.
	m_muAtomTransactions.unlock();
}

void AuDbGlobal::PostCriticalUpdates()
{
	m_muAtomTransactions.lock();
	m_HasCriticalUpdates = true;
	m_muAtomTransactions.unlock();
}

void AuDbGlobal::ScDesignAdded(quint32 designId)
{
	m_muDbDesignTransSc.lock();
	m_DesignTransactionsSc.append(AuDesignTransaction(D_DesignAdd, designId));
	m_muDbDesignTransSc.unlock();
}

void AuDbGlobal::ScDesignChanged(quint32 designId)
{
	m_muDbDesignTransSc.lock();
	m_DesignTransactionsSc.append(AuDesignTransaction(D_DesignChange, designId));
	m_muDbDesignTransSc.unlock();
}

void AuDbGlobal::ScDesignDeactivated(quint32 designId)
{
	m_muDbDesignTransSc.lock();
	m_DesignTransactionsSc.append(AuDesignTransaction(D_DesignDeactivate, designId));
	m_muDbDesignTransSc.unlock();
}

void AuDbGlobal::ScDesignDeleted(quint32 designId)
{
	m_muDbDesignTransSc.lock();
	m_DesignTransactionsSc.append(AuDesignTransaction(D_DesignDelete, designId));
	m_muDbDesignTransSc.unlock();
}

void AuDbGlobal::ScDesignNeeded(quint32 designId)
{
	m_muDbDesignsNeededSc.lock();
	if(!m_DesignsNeededSc.contains(designId))
		m_DesignsNeededSc.append(designId);
	m_muDbDesignsNeededSc.unlock();
}

void AuDbGlobal::ScDesignsNeeded(const AuId& architectId)
{
	m_muDbDesignsNeededSc.lock();
	if(!m_ArchDesignsNeededSc.contains(architectId))
		m_ArchDesignsNeededSc.append(architectId);
	m_muDbDesignsNeededSc.unlock();
}

void AuDbGlobal::BdDesignAdded(quint32 designId)
{
	m_muDbDesignTransBd.lock();
	m_DesignTransactionsBd.append(AuDesignTransaction(D_DesignAdd, designId));
	m_muDbDesignTransBd.unlock();
}

void AuDbGlobal::BdDesignChanged(quint32 designId)
{
	m_muDbDesignTransBd.lock();
	m_DesignTransactionsBd.append(AuDesignTransaction(D_DesignChange, designId));
	m_muDbDesignTransBd.unlock();
}

void AuDbGlobal::BdDesignDeactivated(quint32 designId)
{
	m_muDbDesignTransBd.lock();
	m_DesignTransactionsBd.append(AuDesignTransaction(D_DesignDeactivate, designId));
	m_muDbDesignTransBd.unlock();
}

void AuDbGlobal::BdDesignDeleted(quint32 designId)
{
	m_muDbDesignTransBd.lock();
	m_DesignTransactionsBd.append(AuDesignTransaction(D_DesignDelete, designId));
	m_muDbDesignTransBd.unlock();
}

void AuDbGlobal::BdDesignNeeded(quint32 designId)
{
	m_muDbDesignsNeededBd.lock();
	if(!m_DesignsNeededBd.contains(designId))
		m_DesignsNeededBd.append(designId);
	m_muDbDesignsNeededBd.unlock();
}

void AuDbGlobal::BdDesignsNeeded(const AuId& architectId)
{
	m_muDbDesignsNeededBd.lock();
	if(!m_ArchDesignsNeededBd.contains(architectId))
		m_ArchDesignsNeededBd.append(architectId);
	m_muDbDesignsNeededBd.unlock();
}

// Adds a bid from the sender id on the named channel.
// Revokes all previous bids by the sender on the channel.
// NOTE: Does not verify the comm packet actually represents a bid. Caller does that.
void AuDbGlobal::BidPlaced(const AuCommPacket* msg)
{
	// Revoke any current bids by the sender on the channel.
	BidRevoked(msg->FromId(), msg->ChannelName());

	// Add the nu bid to the queue.
	m_muActiveBids.lock();
	m_ActiveBids.append(AuAtomChat(*msg));
	m_muActiveBids.unlock();
}

// Removes all bids from the sender id on the named channel.
// There should only be one at most but we'll scan the entire list to be thorough.
void AuDbGlobal::BidRevoked(const AuId& idFrom, const QString& channelName)
{
	m_muActiveBids.lock();
	QList<AuAtomChat>::iterator it = m_ActiveBids.begin();
	while(it != m_ActiveBids.end())
	{
		if(!(*it).HasCommPacket())
			continue;

		if((*it).CommPacket()->FromId() == idFrom &&
				(*it).CommPacket()->ChannelName() == channelName)
			it = m_ActiveBids.erase(it);
		else
			it++;
	}
	//qDebug("AuDbGlobal::BidRevoked - id %d channel %s, %d active bids",
			//idFrom.GetID(), (const char*)channelName.toAscii(), m_ActiveBids.count());
	m_muActiveBids.unlock();
}

void AuDbGlobal::DecayActiveBids()
{
	m_muActiveBids.lock();
	QList<AuAtomChat>::iterator it = m_ActiveBids.begin();
	while(it != m_ActiveBids.end())
	{
		if((*it).IsBidExpired())
			it = m_ActiveBids.erase(it);
		else
			it++;
	}
	m_muActiveBids.unlock();
}

// Checks for pending crew notices that corresponds to the crew message.
// Return false if there is no match or the requestor has no active crew notices.
bool AuDbGlobal::HasPendingCrew(const AuCommPacket* msg) const
{
	AuId idLoader;
	const AuCommBoardCrew* msgBoardCrew = dynamic_cast<const AuCommBoardCrew*>(msg);
	if(msgBoardCrew)
		idLoader = msgBoardCrew->LoaderId();
	else
	{
		const AuCommCrewBoarded* msgCrewBoarded = dynamic_cast<const AuCommCrewBoarded*>(msg);
		if(msgCrewBoarded)
			idLoader = msgCrewBoarded->LoaderId();
	}
	if(idLoader.IsNull())
		return false;

	// Look for the matching crew board/debark notice.
	bool foundMatchingBid = false;
	m_muActiveBids.lock();
	for(QList<AuAtomChat>::const_iterator it = m_ActiveBids.cbegin(); it != m_ActiveBids.cend(); it++)
	{
		if(!(*it).HasCommPacket())
			continue;

		// Different types of crew notices can be active. Check the message type.
		if((*it).CommPacket()->Type() != AuCommPacket::BoardCrew)
			continue;

		if((*it).CommPacket()->FromId() == idLoader &&
				(*it).CommPacket()->ChannelName() == msg->ChannelName())
		{
			// Found the active crew notice by the sender on the channel; there can be only one. See AuDbGlobal::BidPlaced.
			// Insure the crew notice has not expired.
			if(!(*it).IsBidExpired())
				foundMatchingBid = true;	// Accept matches the Bid.
			break;
		}
	}
	m_muActiveBids.unlock();
	return foundMatchingBid;
}

// Checks for pending crew notices that corresponds to the crew message.
// Return false if there is no match or the requestor has no active crew notices.
bool AuDbGlobal::HasPendingCryo(const AuCommPacket* msg) const
{
	AuId idLoader;
	const AuCommBoardCryo* msgBoardCryo = dynamic_cast<const AuCommBoardCryo*>(msg);
	if(msgBoardCryo)
		idLoader = msgBoardCryo->LoaderId();
	else
	{
		const AuCommCryoBoarded* msgCryoBoarded = dynamic_cast<const AuCommCryoBoarded*>(msg);
		if(msgCryoBoarded)
			idLoader = msgCryoBoarded->LoaderId();
		else
		{
			const AuCommCryoDebarked* msgCryoDebarked = dynamic_cast<const AuCommCryoDebarked*>(msg);
			if(msgCryoDebarked)
				idLoader = msgCryoDebarked->UnloaderId();
			else
			{
				const AuCommDebarkCryo* msgDebarkCryo = dynamic_cast<const AuCommDebarkCryo*>(msg);
				if(msgDebarkCryo)
					idLoader = msgDebarkCryo->UnloaderId();
			}
		}
	}
	if(idLoader.IsNull())
		return false;

	// Look for the matching crew board/debark notice.
	bool foundMatchingBid = false;
	m_muActiveBids.lock();
	for(QList<AuAtomChat>::const_iterator it = m_ActiveBids.cbegin(); it != m_ActiveBids.cend(); it++)
	{
		if(!(*it).HasCommPacket())
			continue;

		// Different types of crew notices can be active. Check the message type.
		if((*it).CommPacket()->Type() != AuCommPacket::BoardCryo &&
			(*it).CommPacket()->Type() != AuCommPacket::DebarkCryo)
			continue;

		if((*it).CommPacket()->FromId() == idLoader &&
				(*it).CommPacket()->ChannelName() == msg->ChannelName())
		{
			// Found the active crew notice by the sender on the channel; there can be only one. See AuDbGlobal::BidPlaced.
			// Insure the crew notice has not expired.
			if(!(*it).IsBidExpired())
				foundMatchingBid = true;	// Accept matches the Bid.
			break;
		}
	}
	m_muActiveBids.unlock();
	return foundMatchingBid;
}

// Checks for pending colonist notices that corresponds to the colonist message.
// Return false if there is no match or the requestor has no active colonist notices.
bool AuDbGlobal::HasPendingColonists(const AuCommPacket* msg) const
{
	AuId idLoader;
	const AuCommBoardPassengers* msgBoardPassengers = dynamic_cast<const AuCommBoardPassengers*>(msg);
	if(msgBoardPassengers)
		idLoader = msgBoardPassengers->LoaderId();
	else
	{
		const AuCommPassengersBoarded* msgPassengersBoarded = dynamic_cast<const AuCommPassengersBoarded*>(msg);
		if(msgPassengersBoarded)
			idLoader = msgPassengersBoarded->LoaderId();
		else
		{
			const AuCommPassengersDebarked* msgPassengersDebarked = dynamic_cast<const AuCommPassengersDebarked*>(msg);
			if(msgPassengersDebarked)
				idLoader = msgPassengersDebarked->UnloaderId();
			else
			{
				const AuCommDebarkPassengers* msgDebarkPassengers = dynamic_cast<const AuCommDebarkPassengers*>(msg);
				if(msgDebarkPassengers)
					idLoader = msgDebarkPassengers->UnloaderId();
			}
		}
	}
	if(idLoader.IsNull())
		return false;

	// Look for the matching colonist board/debark notice.
	bool foundMatchingBid = false;
	m_muActiveBids.lock();
	for(QList<AuAtomChat>::const_iterator it = m_ActiveBids.cbegin(); it != m_ActiveBids.cend(); it++)
	{
		if(!(*it).HasCommPacket())
			continue;

		// Different types of colonist notices can be active.
		// Check the message type.
		if((*it).CommPacket()->Type() != AuCommPacket::BoardPassengers &&
			(*it).CommPacket()->Type() != AuCommPacket::DebarkPassengers)
			continue;

		if((*it).CommPacket()->FromId() == idLoader &&
				(*it).CommPacket()->ChannelName() == msg->ChannelName())
		{
			// Found the active colonist notice by the sender on the channel;
			// there can be only one. See AuDbGlobal::BidPlaced.
			// Insure the colonist notice has not expired.
			if(!(*it).IsBidExpired())
				foundMatchingBid = true;	// Accept matches the Bid.
			break;
		}
	}
	m_muActiveBids.unlock();
	return foundMatchingBid;
}

// Checks for pending officer notices that corresponds to the officer message.
// Return false if there is no match or the requestor has no active officer notices.
bool AuDbGlobal::HasPendingOfficers(const AuCommPacket* msg) const
{
	AuId idLoader;
	const AuCommBoardOfficers* msgBoardOfficers = dynamic_cast<const AuCommBoardOfficers*>(msg);
	if(msgBoardOfficers)
		idLoader = msgBoardOfficers->LoaderId();
	else
	{
		const AuCommOfficerBoarded* msgOfficersBoarded = dynamic_cast<const AuCommOfficerBoarded*>(msg);
		if(msgOfficersBoarded)
			idLoader = msgOfficersBoarded->LoaderId();
	}
	if(idLoader.IsNull())
		return false;

	// Look for the matching troop board/debark notice.
	bool foundMatchingBid = false;
	m_muActiveBids.lock();
	for(QList<AuAtomChat>::const_iterator it = m_ActiveBids.cbegin(); it != m_ActiveBids.cend(); it++)
	{
		if(!(*it).HasCommPacket())
			continue;

		// Different types of troop notices can be active.
		// Check the message type.
		if((*it).CommPacket()->Type() != AuCommPacket::BoardOfficers)
			continue;

		if((*it).CommPacket()->FromId() == idLoader &&
				(*it).CommPacket()->ChannelName() == msg->ChannelName())
		{
			// Found the active officer notice by the sender on the channel;
			// there can be only one. See AuDbGlobal::BidPlaced.
			// Insure the officer notice has not expired.
			if(!(*it).IsBidExpired())
				foundMatchingBid = true;	// Accept matches the Bid.
			break;
		}
	}
	m_muActiveBids.unlock();
	return foundMatchingBid;
}

// Checks for pending troop notices that corresponds to the troop message.
// Return false if there is no match or the requestor has no active troop notices.
bool AuDbGlobal::HasPendingTroops(const AuCommPacket* msg) const
{
	AuId idLoader;
	const AuCommBoardTroops* msgBoardTroops = dynamic_cast<const AuCommBoardTroops*>(msg);
	if(msgBoardTroops)
		idLoader = msgBoardTroops->LoaderId();
	else
	{
		const AuCommTroopsBoarded* msgTroopsBoarded = dynamic_cast<const AuCommTroopsBoarded*>(msg);
		if(msgTroopsBoarded)
			idLoader = msgTroopsBoarded->LoaderId();
	}
	if(idLoader.IsNull())
		return false;	// Need a troop message.

	// Look for the matching troop board/debark notice.
	bool foundMatchingBid = false;
	m_muActiveBids.lock();
	for(QList<AuAtomChat>::const_iterator it = m_ActiveBids.cbegin(); it != m_ActiveBids.cend(); it++)
	{
		if(!(*it).HasCommPacket())
			continue;

		// Different types of troop notices can be active.
		// Check the message type.
		if((*it).CommPacket()->Type() != AuCommPacket::BoardTroops)
			continue;

		if((*it).CommPacket()->FromId() == idLoader &&
				(*it).CommPacket()->ChannelName() == msg->ChannelName())
		{
			// Found the active troop notice by the sender on the channel;
			// there can be only one. See AuDbGlobal::BidPlaced.
			// Insure the troop notice has not expired.
			if(!(*it).IsBidExpired())
				foundMatchingBid = true;	// Accept matches the Bid.
			break;
		}
	}
	m_muActiveBids.unlock();
	return foundMatchingBid;
}

// Checks for pending Bid that corresponds to the msg,
// expected to be an Accept or Bid message.
// Return false if there is no match or the buyer has no active bid.
bool AuDbGlobal::HasPendingBid(const AuCommPacket* msg) const
{
	AuId idBuyer;
	const AuCommAccept* msgAccept = dynamic_cast<const AuCommAccept*>(msg);
	if(msgAccept)
		idBuyer = msgAccept->BuyerId();
	else
	{
		const AuCommBid* msgBid = dynamic_cast<const AuCommBid*>(msg);
		if(msgBid)
			idBuyer = msgBid->FromId();
	}
	if(idBuyer.IsNull())
		return false;	// Need to pass an Accept or Bid message.

	// Look for the matching bid.
	bool foundMatchingBid = false;
	m_muActiveBids.lock();
	for(QList<AuAtomChat>::const_iterator it = m_ActiveBids.cbegin(); it != m_ActiveBids.cend(); it++)
	{
		if(!(*it).HasCommPacket())
			continue;

		// Different types of bids can be active.
		const AuCommBid* bid = dynamic_cast<const AuCommBid*>((*it).CommPacket());
		if(!bid)
			continue;

		if(bid->FromId() == idBuyer && bid->ChannelName() == msg->ChannelName())
		{
			// Found the active bid by the buyer on the channel; there can be only one. See AuDbGlobal::BidPlaced.
			//qDebug("AuDbGlobal::HasPendingBid found bid %s, bidText %s",
					//(const char*)(*it).m_CommPacket->RawText().toAscii(),
					//(const char*)bidText.toAscii());
			if(bid->CorrespondsTo(msg))
			{
				// Insure the bid has not expired.
				if(!(*it).IsBidExpired())
					foundMatchingBid = true;	// Accept or Bid matches the Bid.
				//else
					//qDebug("AuDbGlobal::HasPendingBid found expired bid");
				break;
			}
		}
	}
	m_muActiveBids.unlock();
	return foundMatchingBid;
}

// Checks for pending Bid that corresponds to the msg,
// expected to be an Accept or Bid message.
// Return false if there is no match or the buyer has no active bid.
bool AuDbGlobal::HasPendingBidBarter(const AuCommPacket* msg) const
{
	AuId idBuyer;
	const AuCommAcceptBarter* msgAccept = dynamic_cast<const AuCommAcceptBarter*>(msg);
	if(msgAccept)
		idBuyer = msgAccept->BuyerId();
	else
	{
		const AuCommBidBarter* msgBid = dynamic_cast<const AuCommBidBarter*>(msg);
		if(msgBid)
			idBuyer = msgBid->FromId();
	}
	if(idBuyer.IsNull())
		return false;	// Need to pass an Accept or Bid message.

	// Look for the matching bid.
	bool foundMatchingBid = false;
	m_muActiveBids.lock();
	for(QList<AuAtomChat>::const_iterator it = m_ActiveBids.cbegin(); it != m_ActiveBids.cend(); it++)
	{
		if(!(*it).HasCommPacket())
			continue;

		// Different types of bids can be active.
		const AuCommBidBarter* bid = dynamic_cast<const AuCommBidBarter*>((*it).CommPacket());
		if(!bid)
			continue;

		if(bid->FromId() == idBuyer && bid->ChannelName() == msg->ChannelName())
		{
			// Found the active bid by the buyer on the channel; there can be only one. See AuDbGlobal::BidPlaced.
			//qDebug("AuDbGlobal::HasPendingBid found bid %s, bidText %s",
					//(const char*)(*it).m_CommPacket->RawText().toAscii(),
					//(const char*)bidText.toAscii());
			if(bid->CorrespondsTo(msg))
			{
				// Insure the bid has not expired.
				if(!(*it).IsBidExpired())
					foundMatchingBid = true;	// Accept or Bid matches the Bid.
				//else
					//qDebug("AuDbGlobal::HasPendingBid found expired bid");
				break;
			}
		}
	}
	m_muActiveBids.unlock();
	return foundMatchingBid;
}

bool AuDbGlobal::HasPendingBidShipment(const AuCommPacket* msg) const
{
	AuId idBuyer;
	const AuCommAcceptPickupShipment* msgAccept = dynamic_cast<const AuCommAcceptPickupShipment*>(msg);
	if(msgAccept)
		idBuyer = msgAccept->BuyerId();
	else
	{
		const AuCommBidPickupShipment* msgBid = dynamic_cast<const AuCommBidPickupShipment*>(msg);
		if(msgBid)
			idBuyer = msgBid->FromId();
	}
	if(idBuyer.IsNull())
		return false;	// Need to pass an Accept or Bid message.

	// Look for the matching bid.
	bool foundMatchingBid = false;
	m_muActiveBids.lock();
	for(QList<AuAtomChat>::const_iterator it = m_ActiveBids.cbegin(); it != m_ActiveBids.cend(); it++)
	{
		if(!(*it).HasCommPacket())
			continue;

		// Different types of bids can be active.
		const AuCommBidPickupShipment* bid = dynamic_cast<const AuCommBidPickupShipment*>((*it).CommPacket());
		if(!bid)
			continue;

		if(bid->FromId() == idBuyer && bid->ChannelName() == msg->ChannelName())
		{
			// Found the active bid by the buyer on the channel; there can be only one. See AuDbGlobal::BidPlaced.
			if(bid->CorrespondsTo(msg))
			{
				// Insure the bid has not expired.
				if(!(*it).IsBidExpired())
					foundMatchingBid = true;	// Accept or Bid matches the Bid.
				break;
			}
		}
	}
	m_muActiveBids.unlock();
	return foundMatchingBid;
}

// Checks for pending OfferRepairXXX that corresponds to the msg, expected to be an AcceptRepair message.
// Return false if there is no match or the buyer has no active bid.
bool AuDbGlobal::HasPendingRepairOffer(const AuCommPacket* msg) const
{
	AuId repairShipId = msg->RepairShipId();
	if(repairShipId.IsNull())
		return false;

	// The message type must correspond to the offer type.
	AuCommPacket::TYPE offerType;
	AuId cityid;
	const AuCommAcceptRepairAll* msgAcceptRepairAll = dynamic_cast<const AuCommAcceptRepairAll*>(msg);
	if(msgAcceptRepairAll)
	{
		offerType = AuCommPacket::OfferRepairAll;
		cityid = msgAcceptRepairAll->CityId();
	}
	else
	{
		const AuCommAcceptRepairSystem* msgAcceptRepairSystem = dynamic_cast<const AuCommAcceptRepairSystem*>(msg);
		if(msgAcceptRepairSystem)
		{
			offerType = AuCommPacket::OfferRepairSystem;
			cityid = msgAcceptRepairSystem->CityId();
		}
		else
		{
			offerType = AuCommPacket::OfferRepairAll;	// Eliminates a compiler warning.
		}
	}
	if(cityid.IsNull())
		return false;

	// Look for the matching OfferRepairXXX notice.
	bool foundMatchingBid = false;
	m_muActiveBids.lock();
	for(QList<AuAtomChat>::const_iterator it = m_ActiveBids.cbegin(); it != m_ActiveBids.cend(); it++)
	{
		if(!(*it).HasCommPacket())
			continue;

		// Different types of repair offers can be active.
		// Check the message type.
		if((*it).CommPacket()->Type() != offerType)
			continue;

		if((*it).CommPacket()->FromId() == cityid &&
				(*it).CommPacket()->RepairShipId() == repairShipId &&
				(*it).CommPacket()->ChannelName() == msg->ChannelName())
		{
			// Found the active offer by the sender on the channel; there can be only one. See AuDbGlobal::BidPlaced.
			// Insure the offer has not expired.
			if(!(*it).IsBidExpired())
				foundMatchingBid = true;	// Accept matches the Bid.
			break;
		}
	}
	m_muActiveBids.unlock();
	return foundMatchingBid;
}

void AuDbGlobal::OpenChannel(const QString& channelName, AuCommStdChannel::SCOPE scope, const AuId& idFrom, const QString& fromName,
		const AuId& fromSector, GALAXY fromGalaxy, const AuId& idTo, bool connectBeyondSolarSystem)
{
	LockAtomChat();
	AuOpenChannel openchan(channelName, scope, idFrom, fromName, fromSector, fromGalaxy, idTo, connectBeyondSolarSystem);
	m_OpenChannels.append(openchan);
	UnlockAtomChat();
}

void AuDbGlobal::RecentAtomChat(const AuCommPacket& msg)
{
	//qDebug("AuDbGlobal::RecentAtomChat type %d msg %s",
	if(!msg.IsRepeatedAtLogin())
		return;
	AuAtomChatList* queue = LockRecentAtomChat();
	AuAtomChat chat(msg);
	queue->append(chat);
	UnlockRecentAtomChat();
	//qDebug("AuDbGlobal::RecentAtomChat done");
}

void AuDbGlobal::SendAtomChat(const AuCommPacket& msg)
{
	//qDebug("AuDbGlobal::SendAtomChat type %d msg %s",
			//msg.Type(), (const char*)msg.Text().toAscii());
	AuAtomChat chat(msg);
	LockAtomChat()->append(chat);
	UnlockAtomChat();
	//qDebug("AuDbGlobal::SendAtomChat done");
}

void AuDbGlobal::DecayAtomChat()
{
	LockAtomChat()->Decay();
	UnlockAtomChat();
}

void AuDbGlobal::DecayRecentAtomChat()
{
	AuAtomChatList* chat = LockRecentAtomChat();
	QList<AuAtomChat>::iterator it = chat->begin();
	while(it != chat->end())
	{
		if((*it).TimeStamp().ElapsedSec() > 5*60)
			it = chat->erase(it);
		else
			it++;
	}
	UnlockRecentAtomChat();
}

void AuDbGlobal::ScExchangeEntryAdded(quint32 designId)
{
	m_muDbExchangeTransSc.lock();
	m_ExchangeTransactionsSc.append(AuDesignTransaction(D_DesignAdd, designId));
	m_muDbExchangeTransSc.unlock();
}

void AuDbGlobal::ScExchangeEntryChanged(quint32 designId)
{
	m_muDbExchangeTransSc.lock();
	m_ExchangeTransactionsSc.append(AuDesignTransaction(D_DesignChange, designId));
	m_muDbExchangeTransSc.unlock();
}

void AuDbGlobal::ScExchangeEntryDeleted(quint32 designId)
{
	m_muDbExchangeTransSc.lock();
	m_ExchangeTransactionsSc.append(AuDesignTransaction(D_DesignDelete, designId));
	m_muDbExchangeTransSc.unlock();
}

void AuDbGlobal::BdExchangeEntryAdded(quint32 designId)
{
	m_muDbExchangeTransBd.lock();
	m_ExchangeTransactionsBd.append(AuDesignTransaction(D_DesignAdd, designId));
	m_muDbExchangeTransBd.unlock();
}

void AuDbGlobal::BdExchangeEntryChanged(quint32 designId)
{
	m_muDbExchangeTransBd.lock();
	m_ExchangeTransactionsBd.append(AuDesignTransaction(D_DesignChange, designId));
	m_muDbExchangeTransBd.unlock();
}

void AuDbGlobal::BdExchangeEntryDeleted(quint32 designId)
{
	m_muDbExchangeTransBd.lock();
	m_ExchangeTransactionsBd.append(AuDesignTransaction(D_DesignDelete, designId));
	m_muDbExchangeTransBd.unlock();
}

void AuDbGlobal::ListDesignsInUse(QSet<quint32>& bdDesigns, QSet<quint32>& scDesigns)
{
	m_muDbDesignTransBd.lock();
	for(QList<AuDesignTransaction>::const_iterator tit = m_DesignTransactionsBd.cbegin();
			tit != m_DesignTransactionsBd.cend(); tit++)
		bdDesigns.insert((*tit).m_idDesign);
	for(QSet<quint32>::const_iterator ri = m_RecentBd.cbegin(); ri != m_RecentBd.cend(); ri++)
		bdDesigns.insert(*ri);
	m_muDbDesignTransBd.unlock();

	m_muDbDesignTransSc.lock();
	for(QList<AuDesignTransaction>::const_iterator tit = m_DesignTransactionsSc.cbegin();
			tit != m_DesignTransactionsSc.cend(); tit++)
		scDesigns.insert((*tit).m_idDesign);
	for(QSet<quint32>::const_iterator ri = m_RecentSc.cbegin(); ri != m_RecentSc.cend(); ri++)
		scDesigns.insert(*ri);
	m_muDbDesignTransSc.unlock();
}

void AuDbGlobal::RateBlueprintBd(const AuExchangeRatingRec& rating)
{
	m_RatingsLock.lock();
	m_BdRatings.append(rating);
	m_RatingsLock.unlock();
}

void AuDbGlobal::RateBlueprintSc(const AuExchangeRatingRec& rating)
{
	m_RatingsLock.lock();
	m_ScRatings.append(rating);
	m_RatingsLock.unlock();
}

void AuDbGlobal::ItDesignAdded(quint32 designId)
{
	m_muDbDesignTransIt.lock();
	m_DesignTransactionsIt.append(AuDesignTransaction(D_DesignAdd, designId));
	m_muDbDesignTransIt.unlock();
}

void AuDbGlobal::ItDesignChanged(quint32 designId)
{
	m_muDbDesignTransIt.lock();
	m_DesignTransactionsIt.append(AuDesignTransaction(D_DesignChange, designId));
	m_muDbDesignTransIt.unlock();
}

void AuDbGlobal::ItDesignDeactivated(quint32 designId)
{
	m_muDbDesignTransIt.lock();
	m_DesignTransactionsIt.append(AuDesignTransaction(D_DesignDeactivate, designId));
	m_muDbDesignTransIt.unlock();
}

void AuDbGlobal::ItDesignDeleted(quint32 designId)
{
	m_muDbDesignTransIt.lock();
	m_DesignTransactionsIt.append(AuDesignTransaction(D_DesignDelete, designId));
	m_muDbDesignTransIt.unlock();
}

void AuDbGlobal::ItDesignNeeded(quint32 designId)
{
	m_muDbDesignsNeededIt.lock();
	if(!m_DesignsNeededIt.contains(designId))
		m_DesignsNeededIt.append(designId);
	m_muDbDesignsNeededIt.unlock();
}

void AuDbGlobal::ItDesignsNeeded(const AuId& architectId)
{
	m_muDbDesignsNeededIt.lock();
	if(!m_ArchDesignsNeededIt.contains(architectId))
		m_ArchDesignsNeededIt.append(architectId);
	m_muDbDesignsNeededIt.unlock();
}

quint32 AuDbGlobal::NextItemDesignId(unsigned int howMany)
{
	m_muNextItemDesignId.lock();
	if(m_NextItemDesignId + howMany > m_LastItemDesignId)
	{
		// The number requested exceeds my ready stock.
		if(m_RestockItemDesignCount)
		{
			// Rotate back stock of ids to the ready stock.
			m_NextItemDesignId = m_RestockItemDesignId - 1;
			m_LastItemDesignId = m_NextItemDesignId + m_RestockItemDesignCount;

			// Clear back stock. DtClientChannel will refill it.
			m_RestockItemDesignId = 0;
			m_RestockItemDesignCount = 0;
		}
		else
		{
			// This is a critical problem!
			m_muNextItemDesignId.unlock();
			g_Au.Error(QString("NextItemDesignId overrun. Next %1 howMany %2 Last %3.")
					.arg(m_NextItemDesignId).arg(howMany).arg(m_LastItemDesignId)); 
			return 0;
		}
	}

	quint32 nextId = m_NextItemDesignId + 1;
	m_NextItemDesignId += howMany;
	m_muNextItemDesignId.unlock();
	return nextId;
}

// Restocks the "back stock" of ids.
void AuDbGlobal::RestockItemDesignIds(quint32 nextId, unsigned int howMany)
{
	//Q_ASSERT(!IsSceneMaster());
	m_muNextItemDesignId.lock();
	m_RestockItemDesignId = nextId;
	m_RestockItemDesignCount = howMany;
	m_muNextItemDesignId.unlock();
}

AuEther& AuDbGlobal::LockEtheriumDelete()
{
	m_Etherium->LockDelete();
	//return AuGlobal::LockEtheriumDelete();	// Base class does nothing.
	return *m_Etherium;
}

void AuDbGlobal::UnlockEtheriumDelete()
{
	m_Etherium->UnlockDelete();
	//AuGlobal::UnlockEtheriumDelete();	// Base class does nothing.
}

size_t AuDbGlobal::DeleteEtheriumTrashAtom()
{
	return m_Etherium->DeleteTrashAtom();
}

void AuDbGlobal::ClearEtherium()
{
	m_Etherium->DeleteEther();
}

bool AuDbGlobal::TestEtheriumTrashLock()
{
	return m_Etherium->TestTrashLock();
}

int AuDbGlobal::TestEtheriumLock(int timeout)
{
	return m_Etherium->TestLock(timeout);
}

