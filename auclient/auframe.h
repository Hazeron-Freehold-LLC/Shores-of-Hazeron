//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auframe.h,v 1.31 2022/04/15 21:00:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuFrame_H
#define AuFrame_H

#include "auclientlib.h"
#include "aucommstdchannel.h"
#include "audockwidget.h"
#include "auexchangerating.h"
#include "auid.h"
#include "auspecies.h"
#include <QMainWindow>

class AuCommPacket;
class AuDialog;
class AuGlobeLLF;
class AuPoint;
class AuView;
class DaSector;
class DzSpecies;
class QMessageBox;

class AUCLIENT_DLL AuFrameDock
{
	AuDockWidget*						m_DockBar;
	bool								m_WasVisible;			// Last known visibility state.

public:
	inline AuFrameDock(AuDockWidget* dock)
	{
		m_DockBar = dock;
		m_WasVisible = false;
	}

	inline AuFrameDock(const AuFrameDock& c)
	{
		m_DockBar = c.m_DockBar;
		m_WasVisible = c.m_WasVisible;
	}

public:
	// Closes the dock bar.
	inline void CloseDockBar()
	{
		if(m_DockBar)
			m_DockBar->close();
	}

	// Hides the dock bar if it is currently visible.
	inline void HideDockBar()
	{
		if(m_DockBar && m_DockBar->isVisible())
			m_DockBar->hide();
	}

	// Returns true if this dock bar is currently visible.
	inline bool IsDockBarVisible() const
	{
		return m_DockBar && m_DockBar->isVisible();
	}

	// Restores the last known visibility state from settings.
	void RestoreVisibilityState();

	// Saves the last known visibility state in settings.
	void SaveVisibilityState() const;

	// Shows the dock bar if the last know visibility state is true.
	inline void ShowWasVisible()
	{
		if(m_DockBar && !m_DockBar->isVisible() && m_WasVisible)
			m_DockBar->show();
	}

	void UpdateTitleShortcut()
	{
		if(m_DockBar)
			m_DockBar->UpdateTitleShortcut();
	}

	// Updates my last known visibility state.
	inline void UpdateVisibilityState()
	{
		m_WasVisible = IsDockBarVisible();
	}
};

class AUCLIENT_DLL AuFrameDockList
{
public:
	QList<AuFrameDock>					m_DockWidgets;

public:
	inline bool AnyVisible() const
	{
		for(QList<AuFrameDock>::const_iterator bi = m_DockWidgets.cbegin(); bi != m_DockWidgets.cend(); bi++)
			if((*bi).IsDockBarVisible())
				return true;
		return false;
	}

	// Closes all the dock bars.
	inline void CloseAll()
	{
		for(QList<AuFrameDock>::iterator bi = m_DockWidgets.begin(); bi != m_DockWidgets.end(); bi++)
			(*bi).CloseDockBar();
	}

	// Hides all the dock bars that are visible.
	inline void HideAll()
	{
		for(QList<AuFrameDock>::iterator bi = m_DockWidgets.begin(); bi != m_DockWidgets.end(); bi++)
			(*bi).HideDockBar();
	}

	inline void RestoreVisibilityStates()
	{
		for(QList<AuFrameDock>::iterator bi = m_DockWidgets.begin(); bi != m_DockWidgets.end(); bi++)
			(*bi).RestoreVisibilityState();
	}

	inline void SaveVisibilityStates() const
	{
		for(QList<AuFrameDock>::const_iterator bi = m_DockWidgets.cbegin(); bi != m_DockWidgets.cend(); bi++)
			(*bi).SaveVisibilityState();
	}

	// Shows all the dock bars that used to be visible.
	inline void ShowWereVisible()
	{
		for(QList<AuFrameDock>::iterator bi = m_DockWidgets.begin(); bi != m_DockWidgets.end(); bi++)
			(*bi).ShowWasVisible();
	}

	// Called when controls have changed to update the display of the keyboard shortcut for showing/hiding the dockbar.
	inline void UpdateTitleShortcuts()
	{
		for(QList<AuFrameDock>::iterator bi = m_DockWidgets.begin(); bi != m_DockWidgets.end(); bi++)
			(*bi).UpdateTitleShortcut();
	}

	inline void UpdateVisibilityStates()
	{
		for(QList<AuFrameDock>::iterator bi = m_DockWidgets.begin(); bi != m_DockWidgets.end(); bi++)
			(*bi).UpdateVisibilityState();
	}
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuFrame : public QMainWindow
{
	Q_OBJECT;
public:
	enum CONTROLINPUT
	{
		CI_Title = 0,						// Used for items that form titles in the setting UI.
		CI_ExitGame,
		CI_FullScreenToggle,
		CI_About,
		CI_ToggleAgentUI,
		CI_ToggleAllUIs,
		CI_ToggleBuildUI_Deprecated,		// Old city construction window
		CI_ToggleBuildingUI,
		CI_ToggleCargoUI,
		CI_ToggleCharacterUI,
		CI_ToggleCommUI,
		CI_ToggleConstructionUI,
		CI_ToggleDesignPartsUI,
		CI_ToggleGovernUI,
		CI_ToggleGearUI,
		CI_ToggleCityUI_Deprecated,			// Old city building tabs
		CI_ToggleLocatorUI,
		CI_ToggleMailUI,
		CI_ToggleSpacecraftUI,
		CI_ToggleOrdersUI,
		CI_ToggleSettingsUI,
		CI_ToggleSpacecraftAccountUI,
		CI_ToggleSpacecraftBountiesUI,
		CI_ToggleSpacecraftDamageUI,
		CI_ToggleSpacecraftMissionUI,
		CI_ToggleSpacecraftPolicyUI,
		CI_ToggleSpacecraftRosterUI,
		CI_ToggleCargoHoldUI,
		CI_ToggleCargoOrdnanceUI,
		CI_ToggleCargoTransferUI,
		CI_ToggleLocatorCompassUI,
		CI_ToggleLocatorGPSUI,
		CI_ToggleLocatorStarMapUI,
		CI_ToggleLocatorWorldMapUI,
		CI_ToggleGovernAnnouncementsUI,
		CI_ToggleGovernCitiesUI,
		CI_ToggleGovernCitizensUI,
		CI_ToggleGovernCommerceUI,
		CI_ToggleGovernDiplomacyUI,
		CI_ToggleGovernPatentsUI,
		CI_ToggleGovernPolicyUI,
		CI_ToggleGovernRewardsUI_Deprecated,
		CI_ToggleGovernTaxesUI,
		CI_ToggleCargoManifestUI,
		CI_ToggleDesignTextureWrapUI,
		CI_ToggleScExchangeUI,
		CI_TogglePlayersOnlineUI,
		CI_ToggleBdExchangeUI,
		CI_ToggleSpacecraftShipmentsUI,
		CI_ToggleHandCraftUI,
		CI_ToggleLocatorSystemScanUI,
		CI_ToggleSpacecraftSpecsUI,
		CI_LASTITEM = 132,					// THIS ENUM MUST NOT EXCEED 132 DUE TO DiControlInput USE OF DaPerson::CONTROLINPUT
	};

public:
	AuFrame(QWidget *parent=0);
	virtual ~AuFrame();

	virtual void ChannelOpened(const QString& channelName, AuCommStdChannel::SCOPE scope, const QString& narrDesc) = 0;

	virtual void ChatReceived(AuCommPacket& msg) = 0;

	virtual void ClearNetCache() = 0;

	virtual QString ControlBinding(CONTROLINPUT control) const = 0;

	virtual void DesignerExited();

	virtual void DisableAFK() = 0;

	// Use this function to display all modal dialogs to insure proper mouse and keyboard ungrab/grab behavior by the view.
	// The return value is the int returned from dlg.exec().
	// For example:
	//     MyDialog dlg;
	//     auApp->Frame()->ExecDialog(dlg);
	int ExecDialog(AuDialog& dlg);

	// Use this function to display all message boxes to insure proper mouse and keyboard ungrab/grab behavior by the view.
	// The return value is the int returned from mb.exec().
	int ExecMessageBox(QMessageBox& mb);

	// Returns true if m_EmpireStateChanged is currently 2 and resets it to 0 if so.
	// Returns false if m_EmipreStateChanged is not 2 and does nothing else.
	virtual bool GetAndResetEmpireDataReceived() = 0;

	virtual bool IsAFK() const = 0;

	bool IsCommChannelOpen(const QString& channelName) const;

	virtual void ListOpenChannels(QStringList& channelNames) const = 0;

	virtual void OperateTurboLift(const AuId& shipId, const AuId& personId, quint16 liftId) = 0;

	virtual void ReplaceSpecies(const DzSpecies& species) = 0;

	virtual void SendChat(AuCommPacket& msg) = 0;

	// Initializes the star map with data from the sector atoms we have locally.
	// Do not call this until the scene atom's empire record is available locally.
	virtual void StarMapInit(quint32 empireId) = 0;

	virtual void StopThreads() = 0;

	virtual AuView* View() const = 0;

public slots:
	void BlueprintRating(quint32 designId, QString designName, AuExchangeBlueprintRating rating);

	virtual void CloseSettingsUi() = 0;

	virtual void GameOver(quint8 gameType, QByteArray gameResult);

	virtual void GameSelect(quint8 gameType, bool gameSelected);

	virtual void GameTurn(AuId personsTurn);

	virtual QIODevice* NetCacheData(const QUrl& url) = 0;

	// Opens browser at, "https://www.hazeron.com"
	void ShowHomePage();

	// Opens wiki with browser at, "https://www.hazeron.com/wiki"
	void ShowWiki();

	// Opens wiki 'topic' with browser.
	// e.g. 'topic' "Designer" opens "https://www.hazeron.com/wiki/index.php/Designer"
	void ShowWikiTopic(const QString& topic);

	virtual void StarMapAddSector(const DaSector* sector, quint32 empireId) = 0;

	// Displays the Steamworks cheater UI, for the Steam evaluation team.
	void Steamworks();

	virtual void SwitchToAvatar(AuId avatarId) = 0;

	virtual void TransferCitizensAvailable(QByteArray ba) = 0;

	// Scene atom wants to enter a cartesian coordinate at a transporter console.
	virtual void TransporterCoordUI(const AuPoint& personLoc) = 0;

	// Scene atom wants to enter a longitude latitude coordinate at a transporter console.
	virtual void TransporterLongLatUI(const AuGlobeLLF& ll) = 0;

signals:
	void StoryChoicePickedCleanup(quint32 storyInstance, qint8 value, AuId offeredBy);
};

#endif // AuFrame_H
