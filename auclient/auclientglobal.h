//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auclientglobal.h,v 1.33 2022/04/08 16:34:47 build Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuClientGlobal_H
#define AuClientGlobal_H

#include "auatomchat.h"
#include "aubddesignexchange.h"
#include "auclientlib.h"
#include "auexchangerating.h"
#include "auglobal.h"
#include "auitdesign.h"
#include "aujoboffer.h"
#include "auscdesignexchange.h"
#include "austarmapdata.h"
#include "austory.h"
#include <QMutex>
#include <QUrl>

class AuScReport;
class DaAtom;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// Scene sync helper.

class AUCLIENT_DLL AuSyncItem
{
public:
	enum TYPE
	{
		AtomAdded,
		AtomChanged,
		AtomDeparted,					// Sends only the m_AtomId.
		AtomParentChanged,
		AtomsCleared,					// Entire list of atoms in the model was cleared.
	};

	TYPE								m_Type;
	DaAtom*								m_Atom;
	AuId								m_AtomId;

public:
	// AtomAdded
	// AtomChanged
	// AtomParentChanged
	AuSyncItem(TYPE type, const DaAtom* atom);

	// AtomDeparted
	AuSyncItem(const AuId& atomId);

	// AtomsCleared
	AuSyncItem();

	~AuSyncItem();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuScDesignExchangeEntryClient : public AuScDesignExchangeEntry
{
	QImage								m_Thumb;

public:	// Constructors
	inline AuScDesignExchangeEntryClient(const AuScDesignExchangeEntry& libraryEntry, const QImage& thumb) :
		AuScDesignExchangeEntry(libraryEntry), m_Thumb(thumb)
	{
	}

	inline AuScDesignExchangeEntryClient(const AuScDesignExchangeEntryClient& c) : AuScDesignExchangeEntry(c), m_Thumb(c.m_Thumb)
	{
	}

public:	// Operators
	inline const AuScDesignExchangeEntryClient& operator=(const AuScDesignExchangeEntryClient& c)
	{
		AuScDesignExchangeEntry::operator=(c);
		m_Thumb = c.m_Thumb;
		return *this;
	}

public:	// Methods
	inline const QImage& Thumb() const
	{
		return m_Thumb;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuScDesignListEntry
{
	AuScReport*							m_Report;
	quint8								m_State;				// AuDesign::STATE enum value.
	QImage								m_Thumb;

public:	// Constructors
	AuScDesignListEntry(const AuScReport& report, quint8 state, const QImage& thumb);

	AuScDesignListEntry(const AuScDesignListEntry& c);

	~AuScDesignListEntry();

public:	// Operators
	const AuScDesignListEntry& operator=(const AuScDesignListEntry& c);

public:	// Methods
	const AuId& ArchitectId() const;

	const QString& ArchitectName() const;

	const QString& Name() const;

	const AuScReport& Report() const;

	QString ReportText() const;

	// Return value is a AuDesign::STATE enum value.
	inline quint8 State() const
	{
		return m_State;
	}

	QString StateName() const;

	inline const QImage& Thumb() const
	{
		return m_Thumb;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuBdDesignExchangeEntryClient : public AuBdDesignExchangeEntry
{
	QImage								m_Thumb;

public:	// Constructors
	inline AuBdDesignExchangeEntryClient(const AuBdDesignExchangeEntry& libraryEntry, const QImage& thumb) :
		AuBdDesignExchangeEntry(libraryEntry), m_Thumb(thumb)
	{
	}

	inline AuBdDesignExchangeEntryClient(const AuBdDesignExchangeEntryClient& c) : AuBdDesignExchangeEntry(c), m_Thumb(c.m_Thumb)
	{
	}

public:	// Operators
	inline const AuBdDesignExchangeEntryClient& operator=(const AuBdDesignExchangeEntryClient& c)
	{
		AuBdDesignExchangeEntry::operator=(c);
		m_Thumb = c.m_Thumb;
		return *this;
	}

public:	// Methods
	inline const QImage& Thumb() const
	{
		return m_Thumb;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuBdDesignListEntry
{
	AuBdReport*							m_Report;
	quint8								m_State;				// AuDesign::STATE enum value.
	QImage								m_Thumb;

public:	// Constructors
	AuBdDesignListEntry(const AuBdReport& report, quint8 state, const QImage& thumb);

	AuBdDesignListEntry(const AuBdDesignListEntry& c);

	~AuBdDesignListEntry();

public:	// Operators
	const AuBdDesignListEntry& operator=(const AuBdDesignListEntry& c);

public:	// Methods
	const AuId& ArchitectId() const;

	const QString& ArchitectName() const;

	const QString& Name() const;

	const AuBdReport& Report() const;

	QString ReportText() const;

	// Return value is a AuDesign::STATE enum value.
	inline quint8 State() const
	{
		return m_State;
	}

	QString StateName() const;

	inline const QImage& Thumb() const
	{
		return m_Thumb;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuItDesignListEntry
{
	AuItReport*							m_Report;
	quint8								m_State;				// AuDesign::STATE enum value.
	QImage								m_Thumb;

public:	// Constructors
	AuItDesignListEntry(const AuItReport& report, quint8 state, const QImage& thumb);

	AuItDesignListEntry(const AuItDesignListEntry& c);

	~AuItDesignListEntry();

public:	// Operators
	const AuItDesignListEntry& operator=(const AuItDesignListEntry& c);

public:	// Methods
	const AuId& ArchitectId() const;

	const QString& ArchitectName() const;

	const QString& Name() const;

	const AuItReport& Report() const;

	QString ReportText() const;

	// Return value is a AuDesign::STATE enum value.
	inline quint8 State() const
	{
		return m_State;
	}

	QString StateName() const;

	inline const QImage& Thumb() const
	{
		return m_Thumb;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuImageDownload
{
	enum REQUEST_STATE
	{
		New,
		Posted,
		Finished,
	};

private:
	quint8								m_NetRequestState;		// A REQUEST_STATE enum value.

public:
	inline AuImageDownload()
	{
		m_NetRequestState = New;
	}

	inline AuImageDownload(const AuImageDownload& c)
	{
		m_NetRequestState = c.m_NetRequestState;
	}

public:	// Operators
	inline const AuImageDownload& operator=(const AuImageDownload& c)
	{
		m_NetRequestState = c.m_NetRequestState;
		return *this;
	}

public:
	// Returns true when the net request is complete.
	inline bool NetRequestFinished() const
	{
		return m_NetRequestState == Finished;
	}

	// Returns true after the request has been posted to the net.
	inline bool NetRequestPosted() const
	{
		return m_NetRequestState != New;
	}

	inline void SetNetRequestFinished()
	{
		m_NetRequestState = Finished;
	}

	inline void SetNetRequestPosted()
	{
		m_NetRequestState = Posted;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuClientGlobal : public AuGlobal
{
public:
	// Solitaire trade with indigenous.
	AuAtomChatList						m_ActiveBids;			// Active bids.
	mutable QMutex						m_muActiveBids;			// Guards m_ActiveBids

	// The semantic for accessing m_SceneAtomIds is to use the root mutex.
	unordered_map<AuId, bool, AuIdHash>	m_SceneAtomIds;	// List of non-permanent scene atoms received with the last scene sync.

	// Use the LockStarMap functions to access this variable.
	quint32								m_StarMapEmpire;		// Empire id of star map. Guarded by m_rwStarMap.

private:
	bool								m_DesignerGridFollowsWork;
	double								m_DesignerGridRadius;
	double								m_DesignerGridSpacing;	// in Meters
	quint8								m_DesignerGridSpacingOm;	// Order of magnitude for brighter grid lines.
	quint8								m_DesignerGridSpacingNoM;	// Next order of magnitude. Brightest grid lines.
	float								m_DesignerSelectionAngleTolerance;	// Plane tolerance when selecting contiguous faces.
	bool								m_DesignerShowBacks;
	bool								m_DesignerShowEdges;
	bool								m_DesignerShowFaces;
	bool								m_DesignerShowGrid;
	bool								m_DesignerShowGround;
	bool								m_DesignerShowSkyBox;
	bool								m_DesignerSnapToEdges;
	bool								m_DesignerSnapToGrid;
	bool								m_DesignerSnapToVertices;
	UNIT_OF_MEASURE						m_DesignerUnitOfMeasure;
	int									m_DossierDoubleClickStatus;	// Status applied to dossier documents when double-clicked.
	QFont								m_FontClose;			// On-screen message font for close range messages.
	QFont								m_FontGalaxy;			// On-screen message font for galaxy range messages.
	QFont								m_FontHail;				// On-screen message font for hail range messages.
	QFont								m_FontSector;			// On-screen message font for sector range messages.
	QFont								m_FontSystem;			// On-screen message font for solar system range messages.
	QFont								m_FontTitle;			// On-screen story title font.
	QFont								m_FontUniverse;			// On-screen message font for universe range messages.
	int									m_HUDIntensity;			// 0..10
	QHash<QUrl, AuImageDownload>		m_ImageDownloads;
	QMutex								m_ImageDownloadsLock;
	bool								m_LimitMessages;		// true to limit number of messages accumulated by each comm channel
	int									m_MessageLimit;			// number of messages to limit each comm channel
	bool								m_MouseGrab;
	int									m_MouseLookSensitivity;	// 1..500
	quint8								m_PopupNotify;			// Popup a dialog when a new chat message contains avatar's name.
	double								m_RoundingAngle;
	int									m_RoadType;				// 0=road, 1=slab.
	float								m_RoadWidth;
	bool								m_ShowNet;				// true to show network activity indicator in scene
	bool								m_ShowThreads;			// true to show worker thread queue indicator in scene
	bool								m_ShowWindowSize;		// true to show size changes in the view
	float								m_SlabWidth;
	AuStarMapData						m_StarMap;				// Known empire for nav mission orders and star map widget.
	bool								m_StoryDialog;			// True to use the story dialog window.
	mutable QReadWriteLock				m_rwStarMap;			// Guards m_StarMap.
	bool								m_ViewActionButtons;	// true to show/use view action buttons (right side black buttons)
	int									m_ViewButtonSize;		// 0..100 pixels
	int									m_ViewButtonStyle;		// 0=custom image
	QString								m_ViewButtonStyleImage;	// Custom view button style image, if style is 0.
	int									m_ViewButtonTransparency;	// 0..100 percent
	QList<int>							m_ViewButtonsHidden;	// AuViewButton::VIEWBUTTON's disabled for display on the screen.
	int									m_WordWrapCharacters;	// number of characters per line at which to break
	bool								m_WordWrapState;		// true to force wordwrapping

	AuAtomChatList						m_AtomChat;				// Atom chat message queue and recent memory.
	QMutex								m_muAtomChat;			// Guards m_AtomChat

private:
	mutable QMutex						m_BdDesignListLock;
	QHash<quint32, AuBdDesignListEntry>	m_BdDesignList;			// List of designs available for download from the library.

private:
	mutable QMutex						m_BdExchangeLock;
	QHash<quint32, AuBdDesignExchangeEntryClient>	m_BdExchange;	// List of published designs.

private:
	mutable QMutex						m_ItDesignListLock;
	QHash<quint32, AuItDesignListEntry>	m_ItDesignList;			// List of designs available for download from the library.

private:
	mutable QMutex						m_ScDesignListLock;
	QHash<quint32, AuScDesignListEntry>	m_ScDesignList;			// List of designs available for download from the library.

private:
	mutable QMutex						m_ScExchangeLock;
	QHash<quint32, AuScDesignExchangeEntryClient>	m_ScExchange;	// List of published designs.

private:
	mutable QMutex						m_SceneControlLock;
	QList<QByteArray>					m_ControlPackets;

private:
	mutable QMutex						m_SceneSyncLock;
	QList<AuSyncItem>					m_Sync;

private:
	QMutex								m_RatingsLock;
	QList<AuExchangeRatingRec>			m_BdRatings;
	QList<AuExchangeRatingRec>			m_ScRatings;

private:
	QMutex								m_EtheriumLock;

private:
	mutable QReadWriteLock				m_rwScripts;			// Guards m_Scripts.
	AuScripts							m_Scripts;				// Story scripts

	mutable QReadWriteLock				m_rwStories;			// Guards m_Stories.
	AuStories							m_Stories;				// Stories that are open

private:
	QReadWriteLock						m_JobOffersLock;		// Controls m_JobOffers.
	AuJobOffers							m_JobOffers;			// Job offers that are posted. These come in from the entire farm.

public:
	AuClientGlobal(const QString& appName, const QString& settingsAppName, const QString& company, bool isSolo, DgData* data,
			AuEther* etherium=0, QSettings::Format format=QSettings::NativeFormat);

	virtual ~AuClientGlobal();

	virtual void AtomAdded(const AuId& id);

	virtual void AtomChanged(const AuId& id);

	virtual void AtomDeleted(const AuId& id);

	virtual void AtomDeparted(const AuId& id);

	// Adds a bid from the sender id on the named channel.
	// Revokes all previous bids by the sender on the channel.
	// NOTE: Does not verify the comm packet actually represents a bid. Caller does that.
	virtual void BidPlaced(const AuCommPacket* msgBid);

	virtual void BidRevoked(const AuId& idFrom, const QString& channelName);

	inline bool ChannelLimitMessages() const
	{
		return (m_LimitMessages && m_MessageLimit > 0) ? true : false;
	}

	virtual void ClearEtherium();

	virtual void DecayActiveBids();

	virtual size_t DeleteEtheriumTrashAtom();

	// Adds/replaces a design in the list of Bd designs available for loading from the library.
	void DesignerAddBdDesignListEntry(const AuBdReport& report, quint8 state, const QImage& thumb);

	void DesignerAddBdExchangeEntry(const AuBdDesignExchangeEntry& libraryEntry, const QImage& thumb);

	// Adds/replaces a design in the list of It designs available for loading from the library.
	void DesignerAddItDesignListEntry(const AuItReport& report, quint8 state, const QImage& thumb);

	// Adds/replaces a design in the list of Sc designs available for loading from the library.
	void DesignerAddScDesignListEntry(const AuScReport& report, quint8 state, const QImage& thumb);

	void DesignerAddScExchangeEntry(const AuScDesignExchangeEntry& libraryEntry, const QImage& thumb);

	// Clears the library of published Bd designs.
	void DesignerClearBdExchange();

	// Clears the list of Bd designs available for loading from the library.
	void DesignerClearBdList();

	// Clears the list of It designs available for loading from the library.
	void DesignerClearItList();

	// Clears the library of published Sc designs.
	void DesignerClearScExchange();

	// Clears the list of Sc designs available for loading from the library.
	void DesignerClearScList();

	inline bool DesignerGridFollowsWork() const
	{
		return m_DesignerGridFollowsWork;
	}

	inline double DesignerGridRadius() const
	{
		return m_DesignerGridRadius;
	}

	inline double DesignerGridSpacing() const
	{
		return m_DesignerGridSpacing;
	}

	inline double DesignerGridSpacingNoM() const
	{
		return m_DesignerGridSpacingNoM;
	}

	inline double DesignerGridSpacingOm() const
	{
		return m_DesignerGridSpacingOm;
	}

	inline double DesignerGridSpacingFt() const
	{
		return m_DesignerGridSpacing * FeetPerMeter();
	}

	inline double DesignerSelectionAngleTolerance() const
	{
		return m_DesignerSelectionAngleTolerance;
	}

	inline void DesignerSetGridFollowsWork(bool gridFollowsWork)
	{
		m_DesignerGridFollowsWork = gridFollowsWork;
		Settings().writeEntry("Designer/GridFollowsWork", gridFollowsWork);
	}

	inline void DesignerSetGridRadius(double gridRadius)
	{
		m_DesignerGridRadius = gridRadius;
	}

	// 'gridSpacing' is the distance between the grid lines.
	// 'gridSpacingOm' is the grid's order of magnitude. It is the number of grid increments for next brighter grid line.
	// 'gridSpacingNoM' is the grid's next order of magnitude. It is the number of grid increments for brightest grid lines.
	inline void DesignerSetGridSpacing(double gridSpacing, quint8 gridOm, quint8 gridNoM)
	{
		m_DesignerGridSpacing = gridSpacing;
		m_DesignerGridSpacingNoM = gridNoM;
		m_DesignerGridSpacingOm = gridOm;
		Settings().writeEntry("Designer/GridSpacing", gridSpacing);
		Settings().writeEntry("Designer/GridOm", gridOm);
		Settings().writeEntry("Designer/GridNoM", gridNoM);
	}

	inline void DesignerSetSelectionAngleTolerance(float angleTol)
	{
		if(angleTol < 0)
			angleTol = 0;
		m_DesignerSelectionAngleTolerance = angleTol;
		Settings().writeEntry("Designer/SelectionAngleTolerance", angleTol);
	}

	inline void DesignerSetShowBacks(bool show)
	{
		m_DesignerShowBacks = show;
		Settings().writeEntry("Designer/ShowBacks", show);
	}

	inline void DesignerSetShowEdges(bool show)
	{
		m_DesignerShowEdges = show;
		Settings().writeEntry("Designer/ShowEdges", show);
	}

	inline void DesignerSetShowFaces(bool show)
	{
		m_DesignerShowFaces = show;
		Settings().writeEntry("Designer/ShowFaces", show);
	}

	inline void DesignerSetShowGrid(bool show)
	{
		m_DesignerShowGrid = show;
		Settings().writeEntry("Designer/ShowGrid", show);
	}

	inline void DesignerSetShowGround(bool show)
	{
		m_DesignerShowGround = show;
		Settings().writeEntry("Designer/ShowGround", show);
	}

	inline void DesignerSetShowSkyBox(bool show)
	{
		m_DesignerShowSkyBox = show;
		Settings().writeEntry("Designer/ShowSkyBox", show);
	}

	inline void DesignerSetSnapToEdges(bool snap)
	{
		m_DesignerSnapToEdges = snap;
		Settings().writeEntry("Designer/SnapToEdges", snap);
	}

	inline void DesignerSetSnapToGrid(bool snap)
	{
		m_DesignerSnapToGrid = snap;
		Settings().writeEntry("Designer/SnapToGrid", snap);
	}

	inline void DesignerSetSnapToVertices(bool snap)
	{
		m_DesignerSnapToVertices = snap;
		Settings().writeEntry("Designer/SnapToVertices", snap);
	}

	inline void DesignerSetUnitOfMeasure(UNIT_OF_MEASURE uom)
	{
		m_DesignerUnitOfMeasure = uom;
		Settings().writeEntry("Designer/UnitOfMeasure", uom);
	}

	inline bool DesignerShowBacks() const
	{
		return m_DesignerShowBacks;
	}

	inline bool DesignerShowEdges() const
	{
		return m_DesignerShowEdges;
	}

	inline bool DesignerShowFaces() const
	{
		return m_DesignerShowFaces;
	}

	inline bool DesignerShowGrid() const
	{
		return m_DesignerShowGrid;
	}

	inline bool DesignerShowGround() const
	{
		return m_DesignerShowGround;
	}

	inline bool DesignerShowSkyBox() const
	{
		return m_DesignerShowSkyBox;
	}

	inline bool DesignerSnapToEdges() const
	{
		return m_DesignerSnapToEdges;
	}

	inline bool DesignerSnapToGrid() const
	{
		return m_DesignerSnapToGrid;
	}

	inline bool DesignerSnapToVertices() const
	{
		return m_DesignerSnapToVertices;
	}

	inline bool DesignerUnitIsMetric() const
	{
		return m_DesignerUnitOfMeasure == UOM_Meters ||
			m_DesignerUnitOfMeasure == UOM_Centimeters ||
			m_DesignerUnitOfMeasure == UOM_Millimeters;
	}

	inline UNIT_OF_MEASURE DesignerUnitOfMeasure() const
	{
		return m_DesignerUnitOfMeasure;
	}

	// Return the factor that is multiplied by values entered, to convert them to our internal unit of feet.
	inline double DesignerUnitScale() const
	{
		return UnitScaleToFeet(m_DesignerUnitOfMeasure);
	}

	// Return the unit of measure symbol.
	inline QString DesignerUnitSymbol() const
	{
		return UnitSymbol(m_DesignerUnitOfMeasure);
	}

	inline int DossierDoubleClickStatus() const
	{
		return m_DossierDoubleClickStatus;
	}

	void DossierSetDoubleClickStatus(int dcs);

	// Converts 'c' to output as coordinate in unit of measure. e.g. (1.1, 2.1, 3.1)m
	// Rounds to .001.
	// Shows unit marks.
	QString FormatCoordinate(const AuPoint& c);

	// Converts 'c.z' to output as length in unit of measure. e.g. 4.3m
	// Rounds to .001.
	// Shows unit marks.
	QString FormatLength(double l);

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

	// Called in solo mode after base data has been initialized, to set the starting point for various next id values.
	void InitNextIds();

	inline int PopupNotify() const
	{
		return m_PopupNotify;
	}

	void RestoreSolitaire(QDataStream& in);

	void SaveSolitaire(QDataStream& out);

	inline bool ShowNet() const
	{
		return m_ShowNet;
	}

	inline bool ShowStoryDialog() const
	{
		return m_StoryDialog;
	}

	inline bool ShowThreads() const
	{
		return m_ShowThreads;
	}

	inline bool ShowWindowSize() const
	{
		return m_ShowWindowSize;
	}

	inline int ChannelMessageLimit() const
	{
		return m_MessageLimit;
	}

	virtual void DecayAtomChat();

	// Overridden in the client to get images for picture frames off the internet.
	// Loads the image from local disk cache first, if it's there.
	virtual QImage DownloadImage(const QUrl& pictureURL);

	inline const QFont& FontClose() const
	{
		return m_FontClose;
	}

	inline const QFont& FontGalaxy() const
	{
		return m_FontGalaxy;
	}

	inline const QFont& FontHail() const
	{
		return m_FontHail;
	}

	inline const QFont& FontSector() const
	{
		return m_FontSector;
	}

	inline const QFont& FontSystem() const
	{
		return m_FontSystem;
	}

	inline const QFont& FontTitle() const
	{
		return m_FontTitle;
	}

	inline const QFont& FontUniverse() const
	{
		return m_FontUniverse;
	}

	inline int HUDIntensity() const
	{
		return m_HUDIntensity;
	}

	virtual bool IsStoryActive(quint32 storyInstance);

	void LoadEther(QDataStream& in);

	void LoadScripts();

	virtual AuAtomChatList* LockAtomChat()
	{
		m_muAtomChat.lock();
		return &m_AtomChat;
	}

	inline QHash<quint32, AuBdDesignListEntry>& LockBdDesignList()
	{
		m_BdDesignListLock.lock();
		return m_BdDesignList;
	}

	inline QHash<quint32, AuBdDesignExchangeEntryClient>& LockBdDesignExchange()
	{
		m_BdExchangeLock.lock();
		return m_BdExchange;
	}

	virtual AuEther& LockEtheriumDelete();

	inline QHash<QUrl, AuImageDownload>& LockImageDownloads()
	{
		m_ImageDownloadsLock.lock();
		return m_ImageDownloads;
	}

	inline QHash<quint32, AuItDesignListEntry>& LockItDesignList()
	{
		m_ItDesignListLock.lock();
		return m_ItDesignList;
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

	inline QHash<quint32, AuScDesignListEntry>& LockScDesignList()
	{
		m_ScDesignListLock.lock();
		return m_ScDesignList;
	}

	inline QHash<quint32, AuScDesignExchangeEntryClient>& LockScDesignExchange()
	{
		m_ScExchangeLock.lock();
		return m_ScExchange;
	}

	virtual const AuScripts* LockScripts()
	{
		m_rwScripts.lockForRead();
		return &m_Scripts;
	}

	inline const AuStarMapData& LockStarMapForRead() const
	{
		m_rwStarMap.lockForRead();
		return m_StarMap;
	}

	inline AuStarMapData& LockStarMapForWrite()
	{
		m_rwStarMap.lockForWrite();
		return m_StarMap;
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

	inline bool MouseGrab() const
	{
		return m_MouseGrab;
	}

	inline int MouseLookSensitivity() const
	{
		return m_MouseLookSensitivity;
	}

	virtual AuId NextAtomId(unsigned int howMany=1);

	virtual quint32 NextBuildingDesignId(unsigned int howMany=1);

	virtual quint32 NextEmpireId();

	virtual quint32 NextItemDesignId(unsigned int howMany=1);

	virtual quint32 NextMailMessageId();

	virtual quint32 NextStoryId(unsigned int howmany=1);

	virtual quint32 NextSpaceShipDesignId(unsigned int howMany=1);

	// Queues a channel to be opened to another atom.
	virtual void OpenChannel(const QString& channelName, AuCommStdChannel::SCOPE scope, const AuId& idFrom, const QString& fromName,
			const AuId& fromSector, GALAXY fromGalaxy, const AuId& idTo, bool connectBeyondSolarSystem);

	virtual void RateBlueprintBd(const AuExchangeRatingRec& rating);

	virtual void RateBlueprintSc(const AuExchangeRatingRec& rating);

	virtual void RemoveStory(quint32 storyInstance);

	virtual void ReloadStory(quint32 storyInstance);

	inline double RoundingAngle() const
	{
		return m_RoundingAngle;
	}

	inline int RoadType() const
	{
		return m_RoadType;
	}

	inline float RoadWidthInFeet() const
	{
		return m_RoadWidth * FeetPerMeter();
	}

	inline float RoadWidthInMeters() const
	{
		return m_RoadWidth;
	}

	// Returns true if the exchange has an entry for 'designId' with the same 'whenMod' last modified time.
	bool BdExchangeHasEntry(quint32 designId, const AuTime& whenMod) const;
	bool ScExchangeHasEntry(quint32 designId, const AuTime& whenMod) const;

	// Prepares exchange for a new table of contents.
	// 'designIds' contains the designs that are supposed to be in my exchange right now, with their last modified times.
	// Loads cached entries if available.
	// Removes entries with different last modified time.
	// Appends ids to 'designsNeeded' of designs that must be requested from the server.
	void BdExchangeToC(const QHash<quint32, AuTime>& designIds, QList<quint32>& designsNeeded);
	void ScExchangeToC(const QHash<quint32, AuTime>& designIds, QList<quint32>& designsNeeded);

	inline float SlabWidthInFeet() const
	{
		return m_SlabWidth * FeetPerMeter();
	}

	inline float SlabWidthInMeters() const
	{
		return m_SlabWidth;
	}

	virtual void SendAtomChat(const AuCommPacket& msg);

	inline void SetChannelLimitMessages(bool limit)
	{
		m_LimitMessages = limit;
		Settings().writeEntry("Settings/ChannelLimitMessages", limit);
	}

	inline void SetPopupNotify(int notify)
	{
		m_PopupNotify = notify;
		Settings().writeEntry("Settings/PopupNotify", notify);
	}

	inline void SetShowNet(bool limit)
	{
		m_ShowNet = limit;
		Settings().writeEntry("Settings/ShowNet", limit);
	}

	inline void SetShowThreads(bool limit)
	{
		m_ShowThreads = limit;
		Settings().writeEntry("Settings/ShowThreads", limit);
	}

	inline void SetShowWindowSize(bool showWindowSize)
	{
		m_ShowWindowSize = showWindowSize;
		Settings().writeEntry("Settings/ShowWindowSize", showWindowSize);
	}

	inline void SetStoryDialog(bool useStoryDialog)
	{
		m_StoryDialog = useStoryDialog;
		Settings().writeEntry("Settings/StoryDialog", useStoryDialog);
	}

	inline void SetChannelMessageLimit(int limit)
	{
		if(limit > 0)
		{
			m_MessageLimit = limit;
			Settings().writeEntry("Settings/ChannelMessageLimit", limit);
		}
	}

	inline void SetFontClose(const QFont& font)
	{
		Q_ASSERT(font.pixelSize());
		m_FontClose = font;
		Settings().writeEntry("Settings/FontClose", m_FontClose);
	}

	inline void SetFontGalaxy(const QFont& font)
	{
		Q_ASSERT(font.pixelSize());
		m_FontGalaxy = font;
		Settings().writeEntry("Settings/FontGalaxy", m_FontGalaxy);
	}

	inline void SetFontHail(const QFont& font)
	{
		Q_ASSERT(font.pixelSize());
		m_FontHail = font;
		Settings().writeEntry("Settings/FontHail", m_FontHail);
	}

	inline void SetFontSector(const QFont& font)
	{
		Q_ASSERT(font.pixelSize());
		m_FontSector = font;
		Settings().writeEntry("Settings/FontSector", m_FontSector);
	}

	inline void SetFontSystem(const QFont& font)
	{
		Q_ASSERT(font.pixelSize());
		m_FontSystem = font;
		Settings().writeEntry("Settings/FontSystem", m_FontSystem);
	}

	inline void SetFontTitle(const QFont& font)
	{
		Q_ASSERT(font.pixelSize());
		m_FontTitle = font;
		Settings().writeEntry("Settings/FontTitle", m_FontTitle);
	}

	inline void SetFontUniverse(const QFont& font)
	{
		Q_ASSERT(font.pixelSize());
		m_FontUniverse = font;
		Settings().writeEntry("Settings/FontUniverse", m_FontUniverse);
	}

	inline void SetHUDIntensity(int hi)
	{
		m_HUDIntensity = hi;
		if(m_HUDIntensity < 0)
			m_HUDIntensity = 0;
		else if(m_HUDIntensity > 10)
			m_HUDIntensity = 10;
		Settings().writeEntry("Settings/HUDIntensity", m_HUDIntensity);
	}

	inline void SetMouseGrab(bool grab)
	{
		m_MouseGrab = grab;
		Settings().writeEntry("Settings/MouseGrab", m_MouseGrab);
	}

	inline void SetMouseLookSensitivity(int mls)
	{
		m_MouseLookSensitivity = mls;
		if(m_MouseLookSensitivity < 1)
			m_MouseLookSensitivity = 1;
		else if(m_MouseLookSensitivity > 500)
			m_MouseLookSensitivity = 500;
		Settings().writeEntry("Settings/MouseLookSensitivity", m_MouseLookSensitivity);
	}

	inline void SetRoundingAngle(double angleInDegrees)
	{
		m_RoundingAngle = angleInDegrees;
		if(m_RoundingAngle < 0)
			m_RoundingAngle = 0;
		else if(m_RoundingAngle > 90)
			m_RoundingAngle = 90;
		Settings().writeEntry("Settings/RoundingAngle", m_RoundingAngle);
	}

	inline void SetRoadType(int type)
	{
		m_RoadType = type;
		if(m_RoadType < 0)
			m_RoadType = 0;
		else if(m_RoadType > 1)
			m_RoadType = 1;
		Settings().writeEntry("Settings/RoadType", m_RoadType);
	}

	inline void SetRoadWidth(float widthInMeters)
	{
		m_RoadWidth = widthInMeters;
		if(m_RoadWidth < 8)
			m_RoadWidth = 8;
		else if(m_RoadWidth > 80)
			m_RoadWidth = 80;
		Settings().writeEntry("Settings/RoadWidth", m_RoadWidth);
	}

	inline void SetSlabWidth(float widthInMeters)
	{
		m_SlabWidth = widthInMeters;
		if(m_SlabWidth < 8)
			m_SlabWidth = 8;
		else if(m_SlabWidth > 80)
			m_SlabWidth = 80;
		Settings().writeEntry("Settings/SlabWidth", m_SlabWidth);
	}

	inline void SetViewButtonHidden(int vb, bool isHidden)
	{
		if(isHidden)
		{
			if(!m_ViewButtonsHidden.contains(vb))
				m_ViewButtonsHidden.append(vb);
		}
		else
		{
			m_ViewButtonsHidden.removeAll(vb);
		}

		Settings().writeEntry("Settings/ViewButtonsHidden", m_ViewButtonsHidden);
	}

	inline void SetViewButtonsHidden(const QList<int>& hiddenButtons)
	{
		m_ViewButtonsHidden = hiddenButtons;
		Settings().writeEntry("Settings/ViewButtonsHidden", m_ViewButtonsHidden);
	}

	inline void SetViewActionButtons(bool visible)
	{
		m_ViewActionButtons = visible;
		Settings().writeEntry("Settings/ViewActionButtons", m_ViewActionButtons);
	}

	inline void SetViewButtonSize(int hi)
	{
		m_ViewButtonSize = hi;
		if(m_ViewButtonSize < 0)
			m_ViewButtonSize = 0;
		else if(m_ViewButtonSize > 100)
			m_ViewButtonSize = 100;
		Settings().writeEntry("Settings/ViewButtonSize", m_ViewButtonSize);
	}

	inline void SetViewButtonStyle(int style)
	{
		m_ViewButtonStyle = style;
		if(m_ViewButtonStyle < 1 || m_ViewButtonStyle >= ViewButtonStyleCount())
			m_ViewButtonStyle = 1;
		Settings().writeEntry("Settings/ViewButtonStyle", m_ViewButtonStyle);
	}

	inline void SetViewButtonStyleImage(const QString& imageName)
	{
		if(imageName.isEmpty())
			return;

		m_ViewButtonStyleImage = imageName;
		Settings().writeEntry("Settings/ViewButtonStyleImage", m_ViewButtonStyleImage);

		m_ViewButtonStyle = 0;
		Settings().writeEntry("Settings/ViewButtonStyle", m_ViewButtonStyle);
	}

	inline void SetViewButtonTransparency(int hi)
	{
		m_ViewButtonTransparency = hi;
		if(m_ViewButtonTransparency < 0)
			m_ViewButtonTransparency = 0;
		else if(m_ViewButtonTransparency > 100)
			m_ViewButtonTransparency = 100;
		Settings().writeEntry("Settings/ViewButtonTransparency", m_ViewButtonTransparency);
	}

	inline void SetWordWrapCharacters(int chars)
	{
		if(chars >= 40)
		{
			m_WordWrapCharacters = chars;
			Settings().writeEntry("Settings/WordWrapCharacters", chars);
		}
	}

	inline void SetWordWrapState(bool state)
	{
		m_WordWrapState = state;
		Settings().writeEntry("Settings/WordWrap", state);
	}

	// Adds/replaces one sector into the global star map.
	void StarMapAddSector(const AuId& id, const AuStarMapSector& sector);

	// Returns the empire id associated with the current star map data.
	quint32 StarMapEmpire() const;

	// Sets the empire id of the owner of the current star map.
	// Clears the current star map when the owner empire changes.
	bool StarMapSetEmpire(quint32 empireId);

	// Returns true if the global star map contains a sector with 'id'.
	bool StarMapHasSector(const AuId& id) const;

	// Clears the star map in preparation for building new data for 'empireId'.
	void StarMapInit(quint32 empireId);

	// Returns true if the current star map is empty of sectors.
	bool StarMapIsEmpty() const;

	// Lists ids, names and empire ids of cities in a system.
	void StarMapListAirportsAtPlanet(const AuId& systemId, const AuId& planetId, AuIdList& cityIds, QStringList& cityNames,
			QList<quint32>& cityEmpires) const;

	// Lists ids, names and empire ids of airports in a system.
	void StarMapListAirportsInSystem(const AuId& systemId, AuIdList& cityIds, QStringList& cityNames,
			QList<quint32>& cityEmpires) const;

	// Lists ids, names and empire ids of cities in a system.
	void StarMapListCitiesAtPlanet(const AuId& systemId, const AuId& planetId, AuIdList& cityIds, QStringList& cityNames,
			QList<quint32>& cityEmpires) const;

	// Lists ids, names and empire ids of cities in a system.
	void StarMapListCitiesInSystem(const AuId& systemId, AuIdList& cityIds, QStringList& cityNames,
			QList<quint32>& cityEmpires) const;

	// Adds/replaces all sectors from 'newMap' into the global star map.
	void StarMapUpdate(const AuStarMapData& newMap);

	virtual void StoryAdded(quint32 storyInstance);

	virtual QString StoryAuthor(quint32 storyInstance);

	virtual QString StoryBrief(quint32 storyInstance);

	virtual void StoryChanged(quint32 storyInstance);

	virtual void StoryDeleted(quint32 storyInstance);

	// Call periodically to release defunct stories.
	void StoryMaintenance();

	virtual QString StoryTitle(quint32 storyInstance);

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

	virtual int TestEtheriumLock(int timeout);

	virtual bool TestEtheriumTrashLock();

	virtual void UnlockAtomChat()
	{
		m_muAtomChat.unlock();
	}

	virtual void UnlockBdDesignList()
	{
		m_BdDesignListLock.unlock();
	}

	virtual void UnlockBdDesignExchange()
	{
		m_BdExchangeLock.unlock();
	}

	virtual void UnlockEtheriumDelete();

	virtual void UnlockImageDownloads()
	{
		m_ImageDownloadsLock.unlock();
	}

	virtual void UnlockItDesignList()
	{
		m_ItDesignListLock.unlock();
	}

	virtual void UnlockJobOffers()
	{
		m_JobOffersLock.unlock();
	}

	virtual void UnlockScDesignList()
	{
		m_ScDesignListLock.unlock();
	}

	virtual void UnlockScDesignExchange()
	{
		m_ScExchangeLock.unlock();
	}

	virtual void UnlockScripts()
	{
		m_rwScripts.unlock();
	}

	inline void UnlockStarMap() const
	{
		m_rwStarMap.unlock();
	}

	virtual void UnlockStories()
	{
		m_rwStories.unlock();
	}

	virtual void UnlockStory(quint32 storyInstance);

	inline bool ViewButtonHidden(int vb) const
	{
		return m_ViewButtonsHidden.contains(vb);
	}

	inline const QList<int>& ViewButtonsHidden() const
	{
		return m_ViewButtonsHidden;
	}

	inline float ViewButtonAlpha() const
	{
		if(m_ViewButtonTransparency >= 100)
			return 0.f;
		else if(m_ViewButtonTransparency <= 0)
			return 1.f;
		return (100.f - m_ViewButtonTransparency)/100.f;
	}

	inline QString ViewButtonImageName() const
	{
		return ViewButtonImageName(m_ViewButtonStyle);
	}

	static QString ViewButtonImageName(int vbs);

	inline bool ViewActionButtons() const
	{
		return m_ViewActionButtons;
	}

	inline int ViewButtonSize() const
	{
		return m_ViewButtonSize;
	}

	// 0 is the custom image style.
	inline int ViewButtonStyle() const
	{
		return m_ViewButtonStyle;
	}

	static int ViewButtonStyleCount();

	static QString ViewButtonStyleName(int vbs);

	inline int ViewButtonTransparency() const
	{
		return m_ViewButtonTransparency;
	}

	inline int WordWrapCharacters() const
	{
		return m_WordWrapCharacters;
	}

	inline bool WordWrapState() const
	{
		return m_WordWrapState;
	}

	// Control packet queue.
	void ControlPacket(const QByteArray& ba);
	void TakeControlQueue(QList<QByteArray>& controls);

	// Scene sync queue.
	void SyncAtomAdded(const DaAtom* atom);
	void SyncAtomChanged(const DaAtom* atom);
	unsigned int SyncAtomCount() const;
	void SyncAtomDeparted(const AuId& atomId);
	void SyncAtomParentChanged(const DaAtom* atom);
	void SyncAtomsCleared();
	void TakeSyncQueue(QList<AuSyncItem>& sync);

	virtual void AddTicker(const AuId& atomId, quint8 atomType, const AuId& systemId);

	virtual void UpdateOrAddTicker(const AuId& atomId, quint8 atomType, const AuId& systemId);

	virtual void AddTocker(const AuId& atomId, quint8 atomType, const AuId& systemId, unsigned int desiredCycleMs);

	virtual void UpdateOrAddTocker(const AuId& atomId, quint8 atomType, const AuId& systemId, unsigned int desiredCycleMs);
};

#define g_AuClient		((AuClientGlobal&)(*gp_AuGlobal))

#endif // AuClientGlobal_H
