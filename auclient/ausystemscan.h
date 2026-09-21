//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausystemscan.h,v 1.22 2022/05/04 21:24:02 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSystemScan_H
#define AuSystemScan_H

#include "auclientlib.h"
#include "audef.h"
#include "audialog.h"
#include "audockwidget.h"
#include "auether.h"
#include "auglobal.h"
#include "auid.h"
#include "ausensorblip.h"
#include <QWidget>

class AuIconButton;
class AuPoint;
class AuSystemWidget;
class DaAtom;
class DaSolarSystem;
class QLabel;
class QShowEvent;
class QSpinBox;
class QSplitter;
class QTextBrowser;
class QTimer;
class QTreeWidget;
class QTreeWidgetItem;

class AUCLIENT_DLL AuSystemScan : public QWidget
{
	Q_OBJECT;
private:
	QHash<AuId, QString>				m_AsteroidDesc;			// Descriptions of asteroids, for report.
	AuIdList							m_AsteroidIds;			// So we can watch for changes.
	QHash<AuId, QTreeWidgetItem*>		m_BodyItems;
	AuEther								m_Ether;
	int									m_ScanEx;
	QString								m_SectorName;
	bool								m_ShowCurrentSystem;
	DaSolarSystem*						m_System;
	QTimer*								m_Timer;
	QString								m_Worms;

	// Widgets
	QTreeWidgetItem*					m_AtomItem;
	QTreeWidget*						m_Bodies;
	QTextBrowser*						m_BodyInfo;
	QSplitter*							m_BodySplitter;
	AuIconButton*						m_LandAdd;
	QLabel*								m_MinQLabel;
	QSpinBox*							m_MinQToShow;
	QSplitter*							m_ScanSplitter;
	AuSystemWidget*						m_SystemPic;
	QLabel*								m_Title;
	AuIconButton*						m_ViewReset;

public:
	AuSystemScan(bool showCurrentSystem, QWidget* parent=0);
	virtual ~AuSystemScan();

	void SetSystem(const AuId& sectorId, const QString& sectorName, const AuId& systemId, const QString& systemName, GALAXY galaxy,
			bool isCoreBlackHole, HABITABILITY genHab, quint32 genSeed, const AuPoint& systemGalaxyLoc, float starLight,
			int scanEx, const DaAtomList& asteroids);

	void SetSystem(const DaSolarSystem* system, quint32 empireId, AuSensorBlip::TYPE viewerBlip, POLITICAL_STANCE viewerStance,
			const AuPoint& viewerLoc);

	virtual QSize sizeHint() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void ListAtomRecursive(QTreeWidgetItem* atomItem, const DaAtom* atom);
	QString ReportSystemWormholes(const DaSolarSystem* system);
	QString ReportWormholes(const AuId& sectorId, const AuId& systemId);
	void RestoreGeometry();
	void SaveGeometry();
	virtual void showEvent(QShowEvent* event);

protected slots:
	void BodyClicked(AuId bodyId);
	void BodySelected(QTreeWidgetItem* current, QTreeWidgetItem* previous);
	void BodySplitterMoved(int pos, int index);
	void LandAdd();
	void ScanSplitterMoved(int pos, int index);
	void TimerUpdate();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuSystemScanDialog : public AuDialog
{
	Q_OBJECT;
public:
	// 'scanEx' 2=scanned, 1=explored, 0=observed
	AuSystemScanDialog(const AuId& sectorId, const QString& sectorName, const AuId& systemId, const QString& systemName,
			GALAXY galaxy, bool isCoreBlackHole, HABITABILITY genHab, quint32 genSeed, const AuPoint& systemGalaxyLoc,
			float starLight, int scanEx, QWidget* parent=0);
	virtual ~AuSystemScanDialog();

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuLocatorSystemScanDock : public AuDockWidget
{
public:
	AuSystemScan*						m_Content;

public:
	AuLocatorSystemScanDock(QWidget* parent) : AuDockWidget(":data/RangeSystem.png", tr("<b>System Map</b>"), tr("System Map"),
			parent)
	{
		SetWikiTopic("System Survey");
		m_Content = new AuSystemScan(true, this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/LocatorSystemMapAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/LocatorSystemMapAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuSystemScan_H
