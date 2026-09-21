//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audbglobal.h,v 1.75 2022/02/19 18:39:24 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDbGlobal_H
#define AuDbGlobal_H

#include "auatomchat.h"
#include "auexchangerating.h"
#include "auglobal.h"
#include "auid.h"
#include "aujoboffer.h"
#include "austarmapdata.h"
#include "austory.h"
#include <QList>
#include <QMutex>

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

enum DESIGN_ACTION
{
	D_Invalid,
	D_DesignAdd,
	D_DesignChange,
	D_DesignDelete,
	D_DesignDeactivate,	// Deletion of a read-only design.
};

class AuDesignTransaction
{
public:
	AuDesignTransaction()
	{
		m_Action = D_Invalid;
		m_idDesign = 0;
	}

	AuDesignTransaction(DESIGN_ACTION action, quint32 idDesign)
	{
		m_Action = action;
		m_idDesign = idDesign;
	}

public:
	DESIGN_ACTION	m_Action;	// action performed by this transaction
	quint32			m_idDesign;	// Space ship design id.
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuDbGlobal : public AuGlobal
{
public:
	AuDbGlobal(const QString& appName, const QString& company, AuGlobal::MODE mode, DgData* data, AuEther* etherium=0);

	virtual void AtomAdded(const AuId& id);

	virtual void AtomChanged(const AuId& id);

	virtual void AtomDeleted(const AuId& id);

	virtual void AtomDeparted(const AuId& id);

	inline bool AtomIdsNeedRestock() const
	{
		return m_RestockAtomCount ? false : true;
	}

	inline quint16 AtomIdsRestockQuantity() const
	{
		if(IsSceneWorker())
			return 1000 * 2;
		else
			return 1000 * 2 * 32;	// max 0xffff				// Enough for 32 worker servers to get their initial batch.
	}

	// Adds a bid from the sender id on the named channel.
	// Revokes all previous bids by the sender on the channel.
	// NOTE: Does not verify the comm packet actually represents a bid. Caller does that.
	virtual void BidPlaced(const AuCommPacket* msgBid);

	virtual void BidRevoked(const AuId& idFrom, const QString& channelName);

	inline bool BuildingDesignIdsNeedRestock() const
	{
		return m_RestockBuildingDesignCount ? false : true;
	}

	inline quint16 BuildingDesignIdsRestockQuantity() const
	{
		if(IsSceneWorker())
			return 100;
		else
			return 2000;
	}

	virtual void ClearEtherium();

	inline const AuId& CurrentAtomId() const		// Get next atom id without incrementing.
	{
		return m_NextAtomId;
	}

	inline quint32 CurrentBuildingDesignId() const		// Get next building design id without incrementing.
	{
		return m_NextBuildingDesignId;
	}

	inline quint32 CurrentEmpireId() const			// Get next empire id without incrementing.
	{
		return m_NextEmpireId;
	}

	inline quint32 CurrentItemDesignId() const		// Get next item design id without incrementing.
	{
		return m_NextItemDesignId;
	}

	inline quint32 CurrentSpaceShipDesignId() const	// Get next space ship design id without incrementing.
	{
		return m_NextSpaceShipDesignId;
	}

	inline quint32 CurrentStoryId() const			// Get next story id without incrementing.
	{
		return m_NextStoryId;
	}

	virtual void DecayActiveBids();

	virtual void DecayAtomChat();

	virtual void DecayRecentAtomChat();

	virtual size_t DeleteEtheriumTrashAtom();

	inline bool HasAtomAddsOrChanges() const
	{
		m_muAtomTransactions.lock();
		bool hasAddOrChg = (!m_AddedAtomIds.isEmpty() || !m_ChangedAtomIds.isEmpty()) ? true : false;
		m_muAtomTransactions.unlock();
		return hasAddOrChg;
	}

	inline bool HasAtomIds() const
	{
		if(IsSceneMaster())
			return m_NextAtomId.IsNull() ? false : true;			// Must wait until db thread calls InitNextIds.
		else
			return AtomIdsNeedRestock() ? false : true;				// Must wait until we are stocked with ids.
	}

	inline bool HasBuildingDesignIds() const
	{
		if(IsSceneMaster())
			return m_NextBuildingDesignId ? true : false;			// Must wait until db thread calls InitNextIds.
		else
			return BuildingDesignIdsNeedRestock() ? false : true;	// Must wait until we are stocked with ids.
	}

	inline bool HasItemDesignIds() const
	{
		if(IsSceneMaster())
			return m_NextItemDesignId ? true : false;				// Must wait until db thread calls InitNextIds.
		else
			return ItemDesignIdsNeedRestock() ? false : true;	// Must wait until we are stocked with ids.
	}

	// Checks for pending Bid that corresponds to the msg,
	// expected to be an Accept or Bid message.
	// Return false if there is no match or the buyer has no active bid.
	virtual bool HasPendingBid(const AuCommPacket* msg) const;

	// Checks for pending bid that corresponds to the msg, expected to be an AcceptBarter or BidBarter message.
	// Return false if there is no match or the buyer has no active bid.
	virtual bool HasPendingBidBarter(const AuCommPacket* msg) const;

	virtual bool HasPendingBidShipment(const AuCommPacket* msg) const;

	// Checks for pending colonist notices that corresponds to the colonist message.
	// Return false if there is no match or the requestor has no active colonist notices.
	virtual bool HasPendingColonists(const AuCommPacket* msg) const;

	// Checks for pending crew notices that corresponds to the crew message.
	// Return false if there is no match or the requestor has no active crew notices.
	virtual bool HasPendingCrew(const AuCommPacket* msg) const;

	// Checks for pending cryo notices that corresponds to the crew message.
	// Return false if there is no match or the requestor has no active crew notices.
	virtual bool HasPendingCryo(const AuCommPacket* msg) const;

	// Checks for pending officer notices that corresponds to the officer message.
	// Return false if there is no match or the requestor has no active officer notices.
	virtual bool HasPendingOfficers(const AuCommPacket* msg) const;

	// Checks for pending troop notices that corresponds to the troop message.
	// Return false if there is no match or the requestor has no active troop notices.
	virtual bool HasPendingTroops(const AuCommPacket* msg) const;

	// Checks for pending OfferRepairXXX that corresponds to the msg, expected to be an AcceptRepair message.
	// Return false if there is no match or the buyer has no active bid.
	virtual bool HasPendingRepairOffer(const AuCommPacket* msg) const;

	inline bool HasSpaceShipDesignIds() const
	{
		if(IsSceneMaster())
			return m_NextSpaceShipDesignId ? true : false;			// Must wait until db thread calls InitNextIds.
		else
			return SpaceShipDesignIdsNeedRestock() ? false : true;	// Must wait until we are stocked with ids.
	}

	inline bool HasStoryIds() const
	{
		if(IsSceneMaster())
			return m_NextStoryId ? true : false;					// Must wait until db thread calls InitNextIds.
		else
			return StoryIdsNeedRestock() ? false : true;			// Must wait until we are stocked with ids.
	}

	void InitNextIds(const AuId& nextAtom, quint32 nextEmpire, quint32 nextSpaceShipDesign, quint32 nextStory,
			quint32 nextBuildingDesign, quint32 nextItemDesign);

	virtual bool IsStoryActive(quint32 storyInstance);

	inline bool ItemDesignIdsNeedRestock() const
	{
		return m_RestockItemDesignCount ? false : true;
	}

	inline quint16 ItemDesignIdsRestockQuantity() const
	{
		if(IsSceneWorker())
			return 100;
		else
			return 2000;
	}

	void ListDesignsInUse(QSet<quint32>& bdDesigns, QSet<quint32>& scDesigns);

	void LoadScript(quint32 scriptId);

	void LoadScripts();

	virtual AuAtomChatList* LockAtomChat()
	{
		m_muAtomChat.lock();
		return &m_AtomChat;
	}

	AuAtomChatList* LockRecentAtomChat()
	{
		m_muRecentAtomChat.lock();
		return &m_RecentAtomChat;
	}

	inline void LockBdDesignsNeeded()
	{
		m_muDbDesignsNeededBd.lock();
	}

	inline void LockBdDesignTransactions()
	{
		m_muDbDesignTransBd.lock();
	}

	inline void LockBdExchangeTransactions()
	{
		m_muDbExchangeTransBd.lock();
	}

	virtual AuEther& LockEtheriumDelete();

	inline void LockItDesignsNeeded()
	{
		m_muDbDesignsNeededIt.lock();
	}

	inline void LockItDesignTransactions()
	{
		m_muDbDesignTransIt.lock();
	}

	inline void LockScDesignsNeeded()
	{
		m_muDbDesignsNeededSc.lock();
	}

	inline void LockScDesignTransactions()
	{
		m_muDbDesignTransSc.lock();
	}

	inline void LockScExchangeTransactions()
	{
		m_muDbExchangeTransSc.lock();
	}

	inline QList<AuExchangeRatingRec> TakeBdRatings()
	{
		m_RatingsLock.lock();
		QList<AuExchangeRatingRec> ratings(m_BdRatings);
		m_BdRatings.clear();
		m_RatingsLock.unlock();
		return ratings;
	}

	inline QList<AuExchangeRatingRec> TakeScRatings()
	{
		m_RatingsLock.lock();
		QList<AuExchangeRatingRec> ratings(m_ScRatings);
		m_ScRatings.clear();
		m_RatingsLock.unlock();
		return ratings;
	}

	virtual const AuJobOffers* LockJobOffersForRead()
	{
		m_JobOffersLock.lockForRead();
		return &m_JobOffers;
	}

	virtual AuJobOffers* LockJobOffersForWrite()
	{
		m_JobOffersLock.lockForWrite();
		return &m_JobOffers;
	}

	inline void LockDbStoryIds()
	{
		m_muDbStoryIds.lock();
	}

	virtual const AuScripts* LockScripts()
	{
		m_rwScripts.lockForRead();
		return &m_Scripts;
	}

	virtual const AuStories* LockStoriesForRead() const
	{
		m_rwStories.lockForRead();
		return &m_Stories;
	}

	virtual AuStories* LockStoriesForWrite()
	{
		m_rwStories.lockForWrite();
		return &m_Stories;
	}

	virtual AuStory* LockStory(quint32 storyInstance);

	// Functions for managing unique database ids.
	// Functions that claim one or more unique database ids will return the first id.
	virtual AuId NextAtomId(unsigned int howMany=1);
	virtual quint32 NextBuildingDesignId(unsigned int howMany=1);
	virtual quint32 NextEmpireId();	// NOTE: Only called on the scene master. No doling logic required for subs.
	virtual quint32 NextItemDesignId(unsigned int howMany=1);
	virtual quint32 NextSpaceShipDesignId(unsigned int howMany=1);
	virtual quint32 NextStoryId(unsigned int howMany=1);

	// Queues a channel to be opened to another atom.
	virtual void OpenChannel(const QString& channelName, AuCommStdChannel::SCOPE scope, const AuId& idFrom, const QString& fromName,
			const AuId& fromSector, GALAXY fromGalaxy, const AuId& idTo, bool connectBeyondSolarSystem);

	void PostCriticalUpdates();

	virtual void RateBlueprintBd(const AuExchangeRatingRec& rating);

	virtual void RateBlueprintSc(const AuExchangeRatingRec& rating);

	// Stores a comm packet temporarily, for people who are traveling when it arrives.
	void RecentAtomChat(const AuCommPacket& msg);

	virtual void RemoveStory(quint32 storyInstance);

	virtual void ReloadStory(quint32 storyInstance);

	void RestockAtomIds(const AuId& nextId, unsigned int howMany);
	void RestockBuildingDesignIds(quint32 nextId, unsigned int howMany);
	void RestockItemDesignIds(quint32 nextId, unsigned int howMany);
	void RestockSpaceShipDesignIds(quint32 nextId, unsigned int howMany);
	void RestockStoryIds(quint32 nextId, unsigned int howMany);

	virtual void SendAtomChat(const AuCommPacket& msg);

	virtual void BdDesignAdded(quint32 designId);
	virtual void BdDesignChanged(quint32 designId);
	virtual void BdDesignDeactivated(quint32 designId);
	virtual void BdDesignDeleted(quint32 designId);
	virtual void BdDesignNeeded(quint32 designId);
	virtual void BdDesignsNeeded(const AuId& architectId);

	virtual void BdExchangeEntryAdded(quint32 designId);
	virtual void BdExchangeEntryChanged(quint32 designId);
	virtual void BdExchangeEntryDeleted(quint32 designId);

	virtual void ItDesignAdded(quint32 designId);
	virtual void ItDesignChanged(quint32 designId);
	virtual void ItDesignDeactivated(quint32 designId);
	virtual void ItDesignDeleted(quint32 designId);
	virtual void ItDesignNeeded(quint32 designId);
	virtual void ItDesignsNeeded(const AuId& architectId);

	virtual void ScDesignAdded(quint32 designId);
	virtual void ScDesignChanged(quint32 designId);
	virtual void ScDesignDeactivated(quint32 designId);
	virtual void ScDesignDeleted(quint32 designId);
	virtual void ScDesignNeeded(quint32 designId);
	virtual void ScDesignsNeeded(const AuId& architectId);

	virtual void ScExchangeEntryAdded(quint32 designId);
	virtual void ScExchangeEntryChanged(quint32 designId);
	virtual void ScExchangeEntryDeleted(quint32 designId);

	inline bool SpaceShipDesignIdsNeedRestock() const
	{
		return m_RestockSpaceShipDesignCount ? false : true;
	}

	inline quint16 SpaceShipDesignIdsRestockQuantity() const
	{
		if(IsSceneWorker())
			return 100;
		else
			return 2000;
	}

	virtual void StoryAdded(quint32 storyInstance);

	virtual QString StoryAuthor(quint32 storyInstance);

	virtual QString StoryBrief(quint32 storyInstance);

	virtual void StoryChanged(quint32 storyInstance);

	virtual void StoryDeleted(quint32 storyInstance);

	inline bool StoryIdsNeedRestock() const
	{
		return m_RestockStoryCount ? false : true;
	}

	inline quint16 StoryIdsRestockQuantity()
	{
		if(IsSceneWorker())
			return 250;
		else
			return 5000;
	}

	virtual QString StoryTitle(quint32 storyInstance);

	virtual int TestEtheriumLock(int timeout);

	virtual bool TestEtheriumTrashLock();

	virtual void UnlockAtomChat()
	{
		m_muAtomChat.unlock();
	}

	void UnlockRecentAtomChat()
	{
		m_muRecentAtomChat.unlock();
	}

	virtual void UnlockEtheriumDelete();

	virtual void UnlockJobOffers()
	{
		m_JobOffersLock.unlock();
	}

	virtual void UnlockScripts()
	{
		m_rwScripts.unlock();
	}

	virtual void UnlockStories()
	{
		m_rwStories.unlock();
	}

	virtual void UnlockStory(quint32 storyInstance);

	inline void UnlockBdDesignsNeeded()
	{
		m_muDbDesignsNeededBd.unlock();
	}

	inline void UnlockBdDesignTransactions()
	{
		m_muDbDesignTransBd.unlock();
	}

	inline void UnlockBdExchangeTransactions()
	{
		m_muDbExchangeTransBd.unlock();
	}

	inline void UnlockItDesignsNeeded()
	{
		m_muDbDesignsNeededIt.unlock();
	}

	inline void UnlockItDesignTransactions()
	{
		m_muDbDesignTransIt.unlock();
	}

	inline void UnlockScDesignsNeeded()
	{
		m_muDbDesignsNeededSc.unlock();
	}

	inline void UnlockScDesignTransactions()
	{
		m_muDbDesignTransSc.unlock();
	}

	inline void UnlockScExchangeTransactions()
	{
		m_muDbExchangeTransSc.unlock();
	}

	inline void UnlockDbStoryIds()
	{
		m_muDbStoryIds.unlock();
	}

private:
	AuScripts				m_Scripts;				// Story scripts
	mutable QReadWriteLock	m_rwScripts;			// Guards m_Scripts.

	AuStories				m_Stories;				// Stories that are open
	mutable QReadWriteLock	m_rwStories;			// Guards m_Stories.

public:
	// m_muDbStoryIds controls these.
	QList<quint32>			m_AddedStoryIds;		// Stories that have been added.
	QList<quint32>			m_ChangedStoryIds;		// Stories that have been changed.
	QList<quint32>			m_DeletedStoryIds;		// Stories that have been deleted.
	QList<quint32>			m_RemoveStoryIds;		// Stories that have been deleted elsewhere.
	QList<quint32>			m_ReloadStoryIds;		// Stories that have been changed elsewhere.

	// Sc design record changes.
	QList<AuDesignTransaction>			m_DesignTransactionsSc;					// Guarded by m_muDbDesignTransSc
	QSet<quint32>						m_RecentSc;								// Recently transacted bd designs to keep loaded.

	// Sc exchange record changes.
	QList<AuDesignTransaction>			m_ExchangeTransactionsSc;				// Guarded by m_muDbExchangeTransSc

	// Sc design load requests.
	// m_muDbDesignsNeededSc controls these.
	QList<quint32>						m_DesignsNeededSc;						// Individual designs.
	QList<AuId>							m_ArchDesignsNeededSc;					// Designs by an architect.

	// Bd design record changes.
	QList<AuDesignTransaction>			m_DesignTransactionsBd;					// Guarded by m_muDbDesignTransBd
	QSet<quint32>						m_RecentBd;								// Recently transacted bd designs to keep loaded.

	// Bd exchange record changes.
	QList<AuDesignTransaction>			m_ExchangeTransactionsBd;				// Guarded by m_muDbExchangeTransBd

	// Bd design load requests.
	// m_muDbDesignsNeededBd controls these.
	QList<quint32>						m_DesignsNeededBd;						// Individual designs.
	QList<AuId>							m_ArchDesignsNeededBd;					// Designs by an architect.

	// It design record changes.
	QList<AuDesignTransaction>			m_DesignTransactionsIt;					// Guarded by m_muDbDesignTransIt
	QSet<quint32>						m_RecentIt;								// Recently transacted designs to keep loaded.

	// It design load requests.
	// m_muDbDesignsNeededIt controls these.
	QList<quint32>						m_DesignsNeededIt;						// Individual designs.
	QList<AuId>							m_ArchDesignsNeededIt;					// Designs by an architect.

private:
	QMutex								m_RatingsLock;
	QList<AuExchangeRatingRec>			m_BdRatings;
	QList<AuExchangeRatingRec>			m_ScRatings;

private:
	QMutex								m_muDbDesignsNeededSc;					// Guards m_DesignsNeededSc and
																				// m_ArchDesignsNeededSc.
	QMutex								m_muDbDesignTransSc;					// Guards m_DesignTransactionsSc.

	QMutex								m_muDbExchangeTransSc;					// Guards m_ExchangeTransactionsSc.

	QMutex								m_muDbDesignsNeededBd;					// Guards m_DesignsNeededBd and
																				// m_ArchDesignsNeededBd.
	QMutex								m_muDbDesignTransBd;					// Guards m_DesignTransactionsBd.

	QMutex								m_muDbExchangeTransBd;					// Guards m_ExchangeTransactionsBd.

	QMutex								m_muDbDesignsNeededIt;					// Guards m_DesignsNeededIt and
																				// m_ArchDesignsNeededIt.
	QMutex								m_muDbDesignTransIt;					// Guards m_DesignTransactionsIt.

	QMutex					m_muDbStoryIds;			// Added/Changed StoryIds

	AuId					m_NextAtomId;			// 0=invalid
	AuId					m_LastAtomId;			// last assigned from boss
	AuId					m_RestockAtomId;		// first id in unused stock
	unsigned int			m_RestockAtomCount;		// ids in unused stock
	QMutex					m_muNextAtomId;

	quint32					m_NextEmpireId;			// 0=invalid
	QMutex					m_muNextEmpireId;

	quint32								m_NextBuildingDesignId;					// 0=invalid
	quint32								m_LastBuildingDesignId;					// last assigned from boss
	quint32								m_RestockBuildingDesignId;				// first id in unused stock
	quint32								m_RestockBuildingDesignCount;			// ids in unused stock
	QMutex								m_muNextBuildingDesignId;

	quint32								m_NextItemDesignId;						// 0=invalid
	quint32								m_LastItemDesignId;						// last assigned from boss
	quint32								m_RestockItemDesignId;					// first id in unused stock
	quint32								m_RestockItemDesignCount;				// ids in unused stock
	QMutex								m_muNextItemDesignId;

	quint32								m_NextSpaceShipDesignId;				// 0=invalid
	quint32								m_LastSpaceShipDesignId;				// last assigned from boss
	quint32								m_RestockSpaceShipDesignId;				// first id in unused stock
	quint32								m_RestockSpaceShipDesignCount;			// ids in unused stock
	QMutex								m_muNextSpaceShipDesignId;

	quint32					m_NextStoryId;			// 0=invalid
	quint32					m_LastStoryId;			// last assigned from boss
	quint32					m_RestockStoryId;		// first id in unused stock
	unsigned int			m_RestockStoryCount;	// ids in unused stock
	QMutex					m_muNextStoryId;

	AuAtomChatList						m_ActiveBids;			// Active bids.
	mutable QMutex						m_muActiveBids;			// Guards m_ActiveBids
	AuAtomChatList						m_AtomChat;				// Atom chat message queue and recent memory.
	QMutex								m_muAtomChat;			// Guards m_AtomChat
	AuAtomChatList						m_RecentAtomChat;		// Atom chat messages received recently, for warp travelers mostly.
	QMutex								m_muRecentAtomChat;		// Guards m_RecentAtomChat

private:
	QReadWriteLock						m_JobOffersLock;		// Controls m_JobOffers.
	AuJobOffers							m_JobOffers;			// Job offers that are posted. These come in from the entire farm.

public:
	// Update notifications.
	mutable QMutex			m_muAtomTransactions;	// Guards m_HasCriticalUpdates, m_AddedAtomIds, m_ChangedAtomIds,
													// m_DeletedAtomIds, and m_DepartedAtoms.
	AuIdList				m_AddedAtomIds;			// Atoms that have been added.
	AuIdList				m_ChangedAtomIds;		// Atoms that have been changed.
	AuIdList				m_DeletedAtomIds;		// Atoms that have been deleted.
	AuIdList				m_DepartedAtoms;		// Atoms that have departed this server.
	bool					m_HasCriticalUpdates;	// true requires db update asap
};

#define g_AuDb		((AuDbGlobal&)(*gp_AuGlobal))

#endif // AuDbGlobal_H
