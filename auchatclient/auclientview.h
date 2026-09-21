//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auclientview.h,v 1.44 2022/05/04 21:32:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuClientView_H
#define AuClientView_H

#include "auatomrenderinfo.h"
#include "aucommpacket.h"
#include "auglmodel.h"
#include "auglmodelmaker.h"
#include "auselectedunit.h"
#include "auview.h"
#include "dadesignbase.h"
#include <QPoint>

class AuClientView;
class AuDialog;
class AuToolButtonExpandValue;
class DaGalaxy;
class DaWorldLL;
class DiControlInputArray;
class DtGL;
class DtTarget;
class QImage;
class QFocusEvent;
class QKeyEvent;
class QMouseEvent;
class QTextBrowser;
class QTimer;
class QToolBar;
class QWheelEvent;

#define CHAT_SOUNDS 4

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuViewButton
{
public:
	// NOTE: Do not change the values of these enums. They are stored in the client's settings.
	enum VIEWBUTTON
	{
		VB_Invalid,
		VB_Agent,
		VB_Architect,
		VB_Bio,
		VB_Cargo,
		VB_UNUSED2,
		VB_Comm,
		VB_Exit,
		VB_FullScreen,
		VB_Government,
		VB_Help,
		VB_HideWindows,
		VB_Gear,
		VB_Locator,
		VB_Mail,
		VB_UNUSED4,
		VB_Programmer,
		VB_Recall,
		VB_Settings,
		VB_Spacecraft,
		VB_UnitOrders,
		VB_About,
		VB_Avatar1,
		VB_Avatar2,
		VB_Avatar3,
		VB_Avatar4,
		VB_Avatar5,
		VB_Avatar6,
		VB_SpacecraftAccount,
		VB_SpacecraftBounties,
		VB_SpacecraftDamage,
		VB_SpacecraftMission,
		VB_SpacecraftPolicy,
		VB_SpacecraftRoster,
		VB_CargoHold,
		VB_CargoOrdnance,
		VB_CargoTransfer,
		VB_LocatorCompass,
		VB_LocatorGPS,
		VB_LocatorStarMap,
		VB_LocatorWorldMap,
		VB_GovernAnnouncements,
		VB_GovernCities,
		VB_GovernCitizens,
		VB_GovernCommerce,
		VB_GovernDiplomacy,
		VB_GovernPolicy,
		VB_UNUSED3,
		VB_GovernTaxes,
		VB_Forum,
		VB_Wiki,
		VB_WarDimensionStandings,
		VB_UNUSED6,
		VB_Avatars,
		VB_CargoManifest,
		VB_SpacecraftDesignExchange,
		VB_HazeronHome,
		VB_Minion,
		VB_PlayersOnline,
		VB_Videos,
		VB_BdDesignExchange,
		VB_Building,
		VB_GovernPatents,
		VB_UNUSED1,
		VB_SpacecraftShipments,
		VB_GearHandCraft,
		VB_LocatorSystemScan,
		VB_UNUSED5,
		VB_COUNT						// LAST ITEM
	};

private:
	QAction*							m_Action;
	AuToolButtonExpandValue*			m_Button;

public:
	bool								m_Enabled;
	VIEWBUTTON							m_Id;
	VIEWBUTTON							m_Parent;				// For sub buttons.
	AuPoint								m_ScreenPos;			// GL unit coord, as if screen is -1 to 1 in both X and Y axes.
	QString								m_Text;					// Overrides default text.
	GLuint								m_Texture;
	QString								m_Tip;					// Overrides default tip.

public:
	inline AuViewButton(VIEWBUTTON buttonId)
	{
		m_Action = 0;
		m_Button = 0;
		m_Enabled = true;
		m_Id = buttonId;
		m_Parent = VB_Invalid;
		m_Texture = 0;
	}

	inline AuViewButton(VIEWBUTTON buttonId, const QString& tip)
	{
		m_Action = 0;
		m_Button = 0;
		m_Enabled = true;
		m_Id = buttonId;
		m_Parent = VB_Invalid;
		m_Texture = 0;
		m_Tip = tip;
	}

	inline AuViewButton(VIEWBUTTON buttonId, const QString& text, const QString& tip)
	{
		m_Action = 0;
		m_Button = 0;
		m_Enabled = true;
		m_Id = buttonId;
		m_Parent = VB_Invalid;
		m_Text = text;
		m_Texture = 0;
		m_Tip = tip;
	}

	inline AuViewButton(const AuViewButton& c)
	{
		m_Action = c.m_Action;
		m_Button = c.m_Button;
		m_Enabled = c.m_Enabled;
		m_Id = c.m_Id;
		m_Parent = c.m_Parent;
		m_ScreenPos = c.m_ScreenPos;
		m_Text = c.m_Text;
		m_Texture = c.m_Texture;
		m_Tip = c.m_Tip;
	}

public:	// Methods
	void AddToToolBar(QToolBar* tb, AuClientView* view);

	// Returns the tool tip that appears on the expand button, if HasSubs is true.
	QString ExpandTip() const;

	bool HasSubs() const;

	inline QString Image() const
	{
		return Image(m_Id);
	}

	static QString Image(VIEWBUTTON vb);

	// True if this button is hidden right now because it is not available to the user.
	// 'hasEmpire' is true if the scene atom has an empire, false if they are a free avatar.
	bool IsHidden(SCENE_CONTEXT sceneContext, bool hasEmpire) const;

	inline bool IsSub() const
	{
		return m_Parent != VB_Invalid;
	}

	// True if this button can be painted at all.
	bool IsVisible() const;

	// Name used as the button text.
	inline QString Name() const
	{
		if(!m_Text.isEmpty())
			return m_Text;
		return Name(m_Id);
	}

	static QString Name(VIEWBUTTON vb);

	// Name in settings is longer to group sub items.
	inline QString SettingsName() const
	{
		return SettingsName(m_Id);
	}

	static QString SettingsName(VIEWBUTTON vb);

	void SetVisible(bool visible);

	inline QString TextureName() const
	{
		return QString("VB%1").arg(m_Id);
	}

	inline QString Tip() const
	{
		if(!m_Tip.isEmpty())
			return Tip(m_Tip);
		return Tip(TipBase(m_Id));
	}

	// Returns the base tip, with %2 where the key command should be plugged in.
	static QString TipBase(VIEWBUTTON vb);

	// Replaces %2 in tipBase with the current key command binding and returns the result.
	QString Tip(const QString& tipBase) const;
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// AuRenderMsg is used to track AuCommPackets as they arrive.

#define s_TitleFadeTime					15						// Time before title narrative fades.
#define s_MessageFadeTime				10						// Time before title narrative fades.

class AuRenderMsg
{
private:
	QString								m_Channel;
	AuElapsedTimer						m_DisplayTime;			// Time this message was first rendered on the screen.
	QString								m_FromName;
	AuCommStdChannel::RANGE				m_Range;				// Actual sender range, as determined by the server.
	AuCommStdChannel::SCOPE				m_Scope;				// Message transmission scope.
	POLITICAL_STANCE					m_StanceTowardTarget;	// Only for SpotTargetOrTerrain messages.
	bool								m_TargetIsCivilian;		// For SpotTargetOrTerrain messages.
	QString								m_Text;
	bool								m_ToneSounded;
	AuCommPacket::TYPE					m_Type;

public:
	inline AuRenderMsg(const AuCommPacket* msg)
	{
		m_Channel = msg->ChannelName();
		m_FromName = msg->FromName();
		m_Range = msg->ReceptionRange();;
		m_Scope = msg->Scope();

		const AuCommSpotTargetOrTerrain* msgSpot = dynamic_cast<const AuCommSpotTargetOrTerrain*>(msg);
		if(msgSpot)
		{
			m_StanceTowardTarget = msgSpot->StanceTowardTarget();
			m_TargetIsCivilian = msgSpot->IsCivilian();
		}
		else
		{
			m_StanceTowardTarget = POL_Neutral;
			m_TargetIsCivilian = false;
		}

		m_Text = msg->Text();
		m_ToneSounded = false;
		m_Type = msg->Type();
	}

	inline AuRenderMsg()
	{
		m_StanceTowardTarget = POL_Neutral;
		m_Range = AuCommStdChannel::Close;
		m_Scope = AuCommStdChannel::Everyone;
		m_TargetIsCivilian = false;
		m_ToneSounded = false;
		m_Type = AuCommPacket::CommTypes;	// This is the "invalid" value.
	}

	inline AuRenderMsg(const AuRenderMsg& c)
	{
		m_Channel = c.m_Channel;
		m_DisplayTime = c.m_DisplayTime;
		m_FromName = c.m_FromName;
		m_Range = c.m_Range;
		m_Scope = c.m_Scope;
		m_StanceTowardTarget = c.m_StanceTowardTarget;
		m_TargetIsCivilian = c.m_TargetIsCivilian;
		m_Text = c.m_Text;
		m_ToneSounded = c.m_ToneSounded;
		m_Type = c.m_Type;
	}

public:	// Operators
	inline const AuRenderMsg& operator=(const AuRenderMsg& c)
	{
		m_Channel = c.m_Channel;
		m_DisplayTime = c.m_DisplayTime;
		m_FromName = c.m_FromName;
		m_Range = c.m_Range;
		m_Scope = c.m_Scope;
		m_StanceTowardTarget = c.m_StanceTowardTarget;
		m_TargetIsCivilian = c.m_TargetIsCivilian;
		m_Text = c.m_Text;
		m_ToneSounded = c.m_ToneSounded;
		m_Type = c.m_Type;
		return *this;
	}

public:	// Methods
	inline const QString& ChannelName() const
	{
		return m_Channel;
	}

	QColor Color() const;

	inline bool DecayMessage() const
	{
		int secsUp = SecondsOnDisplay();
		if(!secsUp)
			return false;	// Message either hasn't been displayed yet or hasn't been displayed long enough to worry about decay.

		// Different types of messages decay at different rates.
		if(AuCommPacket::IsNarratorMessage(m_Type))
		{
			if(m_Type == AuCommPacket::NarrateTitle)
			{
				if(secsUp > s_TitleFadeTime)
					return true;	// Narrator titles decay after 10 seconds.
			}
			else
			{
				if(secsUp > s_MessageFadeTime)
					return true;	// Narrator messages decay after 30 seconds.
			}
		}
		else
		{
			if(secsUp > s_MessageFadeTime)
				return true;		// Regular chat messages decay after 10 seconds.
		}
		return false;	// This message has not completely decayed yet.
	}

	inline const QString& FromName() const
	{
		return m_FromName;
	}

	inline bool IsTargetCivilian() const
	{
		return m_TargetIsCivilian;
	}

	inline int MSecsOnDisplay() const
	{
		// If the display time is null, this message has not been displayed yet.
		if(!m_DisplayTime.isValid())
			return 0;
		return m_DisplayTime.elapsed();
	}

	QString OnScreenMessage() const;

	inline AuCommStdChannel::RANGE Range() const
	{
		return m_Range;
	}

	inline int SecondsOnDisplay() const
	{
		// If the display time is null, this message has not been displayed yet.
		if(!m_DisplayTime.isValid())
			return 0;
		return m_DisplayTime.elapsed()/d_MSecPerSec;
	}

	inline void SetDisplayed()
	{
		// Only set the displayed time the first time this function is called. Otherwise the message never decays.
		if(!m_DisplayTime.isValid())
			m_DisplayTime.start();
	}

	inline void SetText(const QString& text)
	{
		m_Text = text;
	}

	inline void SetType(AuCommPacket::TYPE type)
	{
		m_Type = type;
	}

	inline POLITICAL_STANCE StanceTowardTarget() const
	{
		return m_StanceTowardTarget;
	}

	inline const QString& Text() const
	{
		return m_Text;
	}

	// Returns a SOUND_EFFECT
	int ToneToPlay();

	inline AuCommPacket::TYPE Type() const
	{
		return m_Type;
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuClientView : public QWidget, public AuView
{
	Q_OBJECT;
	QTextBrowser*						m_Browser;
	QList<AuViewButton>					m_ViewButtons;			// Buttons on the tool bar.

private:
	bool								m_bWaitingForScene;		// True while waiting for scene atom.
    AuId								m_idSceneAtom;			// The atom that provides the proper scene for m_pViewAtom.
	AuId								m_idWaitScene;			// Scene atom for which we are waiting.
	AuGLModelMaker						m_MM;					// Model maker.
	AuGLModelCache						m_ModelCache;
	unsigned int						m_ModalDlgCount;		// non-zero while a modal dialog is up. Incremented/decremented
																// for each dialog exec()'d.
	AuAtomRenderInfo					m_RenderInfo;			// Values needed by atoms when rendering them.
	SCENE_CONTEXT						m_SceneContext;			// Current scene context.
	QTimer*								m_SceneTimer;			// Scene animation timer.

    // Selection
	AuSelectedUnitArray	 				m_Selected;				// Information about currently selected atoms.
	AuSelectedUnitArray	 				m_Squad[10];			// Squad selection sets.

	// Communications
	mutable QMutex						m_MessageLock;
	QVector<AuRenderMsg>				m_Messages;				// Recent comm messages.

private:
	mutable QReadWriteLock				m_SceneInfoLock;
	quint8								m_AgentBits;			// Agent bits from scene atom
    AuId								m_idSceneSector;		// The sector atom that currently contains the scene atom.
    AuId								m_idSceneSolarSystem;	// The solar system atom that currently contains the scene atom.
	bool								m_InCity;				// true while the scene atom is in a city and not in a spacecraft
	bool								m_IsShipCommand;		// true if person is in chain-of-command of the ship they are aboard
	AuRosterEntry::TYPE					m_SceneAtomBerth;
	quint32								m_SceneAtomEmpire;		// Empire of the scene atom.
	int									m_SceneAtomEnvType;		// construction window: 0=space, 1=desert, 2=plants, 3=water, 4=moon
    AuId								m_SceneAtomInShip;		// Ship id of the ship that contains the scene atom.
	QString								m_SceneAtomName;		// Name of the scene atom.
	AuId								m_SceneAtomOwnerId;		// The owner id of a private ship officered by the scene atom, if
																// it is found in the environment. Otherwise, the scene atom id.
    AuId								m_SceneAtomScopeShip;	// Ship id of the scene atom used for crew scope chat messages.
	bool								m_SceneAtomTransition;	// result of last call to scene atom's IsInTransition function.
	quint32								m_SceneAtomWorldBuildingCount;	// Current building count on the world.
	quint32								m_SceneAtomWorldBuildingLimit;	// Limit to building volume on the world, in cubic meters.
	quint32								m_SceneAtomWorldBuildingVolume;	// Current building volume on the world, in cubic meters.
    GALAXY								m_SceneGalaxy;			// The galaxy that currently contains the scene atom.

private:
	// The view keeps its own copy of the atoms in the scene.
	QReadWriteLock						m_SceneLock;			// Locked while accessing the scene atom pointer in m_SceneEther.
	AuEther								m_SceneEther;			// Ether of this scene.

private:
	// Unit orders sprite.
	AuId								m_OrderId;				// Atom that m_OrderLoc is relative to.
	AuPoint								m_OrderLoc;				// Location of go to order.
	AuTime								m_OrderTime;			// Time goto order was made.

public:
	AuClientView(QWidget *parent=0);
	~AuClientView();

	virtual void ChatReceived(AuCommPacket* msg);

	inline void AddMessage(const AuRenderMsg& rm)
	{
		m_MessageLock.lock();
		m_Messages.append(rm);
		m_MessageLock.unlock();
	}

	// Returns true if person is in the chain-of-command of the ship they are aboard.
	virtual bool IsPersonInCommand() const
	{
		m_SceneInfoLock.lockForRead();
		bool v = m_IsShipCommand;
		m_SceneInfoLock.unlock();
		return v;
	}

	virtual void LocalLogMessage(const QString& text);

	virtual quint8 PersonAgentBits() const
	{
		m_SceneInfoLock.lockForRead();
		quint8 v = m_AgentBits;
		m_SceneInfoLock.unlock();
		return v;
	}

	// Returns true if the selection actually changes.
	virtual bool SelectUnit(const AuId& id, const QString& name, int rank, const QString& rankAbbr, const QString& rankName,
			NPC_ROLE npcRole, bool selectIt);

	void SelectUnitToggle(const AuId& id, const QString& name, int rank, const QString& rankAbbr, const QString& rankName,
			NPC_ROLE npcRole);

	virtual const AuSelectedUnitArray& SelectedUnits() const
	{
		return m_Selected;
	}

	virtual void SetSelectedBd(const AuId& idBd);

	void SquadCreate(int auquadIndex);

	// So far, 'orderText' is only needed for the go to room order. It is expected to be a room name then.
	virtual void SquadIssueOrder(CITIZEN_ORDER order, const QString& orderText=QString(""));

	virtual void SquadIssuePosture(CITIZEN_POSTURE posture);

	void SquadSelect(int auquadIndex, bool addToCurrentSelection);

	void SquadSendOrder(AuCommPacket::TYPE msgType, const QString& msgText) const;

	// These LockScene functions only lock the atom mutex.
	virtual void LockSceneAtom(DaPerson** sceneAtom);
	void LockSceneAtomForWrite(DaPerson** sceneAtom);
	virtual void UnlockSceneAtom();

	virtual const AuId& SceneAtomId() const
	{
		return m_idSceneAtom;
	}

	inline bool SceneAtomIsOfficer() const
	{
		m_SceneInfoLock.lockForRead();
		bool v = !m_SceneAtomScopeShip.IsNull() && AuRosterEntry::IsOfficer(m_SceneAtomBerth);
		m_SceneInfoLock.unlock();
		return v;
	}

	virtual quint32 SceneAtomEmpire() const
	{
		m_SceneInfoLock.lockForRead();
		quint32 v = m_SceneAtomEmpire;
		m_SceneInfoLock.unlock();
		return v;
	}

	virtual int SceneAtomEnvType() const
	{
		m_SceneInfoLock.lockForRead();
		int v = m_SceneAtomEnvType;
		m_SceneInfoLock.unlock();
		return v;
	}

	virtual QImage SceneAtomImage() const;

	inline bool SceneAtomInTransition() const
	{
		m_SceneInfoLock.lockForRead();
		bool v = m_SceneAtomTransition;
		m_SceneInfoLock.unlock();
		return v;
	}

	virtual QString SceneAtomName() const
	{
		m_SceneInfoLock.lockForRead();
		QString v(m_SceneAtomName);
		m_SceneInfoLock.unlock();
		return v;
	}

	// If the scene atom is an officer of a private ship, returns the owner id of that ship if it is found in the environment.
	// Otherwise, returns the scene atom id.
	virtual AuId SceneAtomOwnerId() const
	{
		m_SceneInfoLock.lockForRead();
		AuId v(m_SceneAtomOwnerId);
		m_SceneInfoLock.unlock();
		return v;
	}

	// Returns the id of the scene atom's spawn ship.
	virtual AuId SceneAtomScopeShip() const
	{
		m_SceneInfoLock.lockForRead();
		AuId v(m_SceneAtomScopeShip);
		m_SceneInfoLock.unlock();
		return v;
	}

	virtual GALAXY SceneGalaxy() const
	{
		m_SceneInfoLock.lockForRead();
		GALAXY v = m_SceneGalaxy;
		m_SceneInfoLock.unlock();
		return v;
	}

	// Returns the id of the ship that contains the scene atom, if any.
	virtual AuId SceneAtomInShip() const
	{
		m_SceneInfoLock.lockForRead();
		AuId v(m_SceneAtomInShip);
		m_SceneInfoLock.unlock();
		return v;
	}

	virtual quint32 SceneAtomWorldBuildingCount() const
	{
		m_SceneInfoLock.lockForRead();
		quint32 v = m_SceneAtomWorldBuildingCount;
		m_SceneInfoLock.unlock();
		return v;
	}

	virtual quint32 SceneAtomWorldBuildingVolumeLimit() const
	{
		m_SceneInfoLock.lockForRead();
		quint32 v = m_SceneAtomWorldBuildingLimit;
		m_SceneInfoLock.unlock();
		return v;
	}

	virtual quint32 SceneAtomWorldBuildingVolume() const
	{
		m_SceneInfoLock.lockForRead();
		quint32 v = m_SceneAtomWorldBuildingVolume;
		m_SceneInfoLock.unlock();
		return v;
	}

	virtual AuId SceneSector() const
	{
		m_SceneInfoLock.lockForRead();
		AuId v(m_idSceneSector);
		m_SceneInfoLock.unlock();
		return v;
	}

	virtual AuId SceneSolarSystem() const
	{
		m_SceneInfoLock.lockForRead();
		AuId v(m_idSceneSolarSystem);
		m_SceneInfoLock.unlock();
		return v;
	}

	void SetSceneAtom(const AuId& idScene);

	virtual SCENE_CONTEXT SceneContext() const
	{
		return m_SceneContext;
	}

protected slots:
	void AnimateScene();

public slots:
	void ActionClicked(QAction* action);

	void AvatarOnLine(AuId id);

	virtual void DeselectAllUnits();

	void PageAgent();

	void SaveWhereabouts();

	void SetNumberOfMessages(int iNumber);

	void SpawnRecall();

	void SpawnTie();

	void TabKeyPressed(Qt::KeyboardModifiers mods);

	void SpotTargetOrTerrain(bool logOnly);

	void ToggleFullScreen();

	// 'value' is a AuViewButton::VIEWBUTTON
	void ViewIconButtonClicked(int value);

	// 'value' is a AuViewButton::VIEWBUTTON
	void ViewIconButtonExpanded(int value);

public:
	virtual QString DesignAnalysis();

	inline bool IsModalDlgUp() const
	{
		return m_ModalDlgCount ? true : false;
	}

	virtual void SetModalDlgUp(bool isUp)
	{
		// Increment the modal dialog counter based on whether this is an up or a down notification.
		if(isUp)
		{
			// Increment count of modal dialogs that are up.
			m_ModalDlgCount++;
		}
		else
		{
			// Decrement count of modal dialogs that are up.
			Q_ASSERT(m_ModalDlgCount);	// Fails if you call SetModalDlgUp(false) more times than SetModalDlgUp(true).
			if(m_ModalDlgCount)
				m_ModalDlgCount--;
		}
	}

signals:
	void ChatReleased();	// emitted when keyboard grab released for chat

	void SceneAtomChanged();

	// 'sceneContext' is a SCENE_CONTEXT value.
	void SceneContextChanged(int sceneContext);

	void SpotTargetOrTerrain(QString targetDesc, int stanceTowardTarget, bool targetIsCivilian, bool logOnly);

	void ToggleScreenReady();

private:
	void CheckDossier();

	void RenderCommMessages();

	bool SelectUnitToggle(DaPerson* sceneAtom, DaAtom *pSelect);

	// emits the SceneContextChanged signal if the view context actually changes.
	void SetSceneContext(SCENE_CONTEXT viewContext);

	// 'sceneAtom' may be null.
	void UpdateSceneInfo(DaPerson* sceneAtom);

	void UpdateViewButtonVisibility();

	void ViewButtonClicked(AuViewButton::VIEWBUTTON button);

	void SyncScene();

	// Sync functions.
	void AtomAdded(DaAtom* atom);
	void AtomChanged(DaAtom* atom);
	void AtomDeparted(const AuId& atomId);
	void AtomParentChanged(const DaAtom* atom);
	void AtomsCleared();
};

#endif // AuClientView_H
