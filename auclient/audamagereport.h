//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audamagereport.h,v 1.11 2022/05/04 21:10:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDamageReport_H
#define AuDamageReport_H

#include "auclientlib.h"
#include "audamage.h"
#include "audockwidget.h"
#include "auglobal.h"

class AuIconButton;
class QLabel;
class QProgressBar;
class QScrollArea;
class QVBoxLayout;
class QTimer;

class AUCLIENT_DLL AuDamageReportSystem : public QWidget
{
	Q_OBJECT;
private:
	// Widgets
	QProgressBar*						m_Bar;
	QLabel*								m_Icon;
	QLabel*								m_Name;
	QLabel*								m_Parts;
	QLabel*								m_Percent;
	QLabel*								m_PercentIco;

public:
	AuDamageReportSystem(QWidget* parent=0);
	~AuDamageReportSystem();

	void Update(const AuSysId& sysId, COMMODITYID curModule, const QString& sysName, const AuDamage& damage, quint32 maxHits,
			quint32 maxComponents);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuDamageReport : public QWidget
{
	Q_OBJECT;
private:
	bool								m_LastAllOrDamaged;
	AuDamage							m_LastDamage;				// Returned by DaVessel::DamageReport
	QTimer*								m_timer;

	// Widgets
	AuIconButton*						m_AllOrDamaged;
	QLabel*								m_Conditions;
	QLabel*								m_Name;
	QVBoxLayout*						m_SystemLay;
	QScrollArea*						m_SystemScroller;
	QList<AuDamageReportSystem*>		m_SystemWidgets;

public:
	AuDamageReport(QWidget* parent=0);
	~AuDamageReport();

	virtual QSize sizeHint() const;

protected:
	virtual void hideEvent(QHideEvent* event);
	virtual void showEvent(QShowEvent* event);

protected slots:
	void AllOrDamaged();
	void UpdateContents();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuSpacecraftDamageDock : public AuDockWidget
{
public:
	AuDamageReport*						m_DamageReport;

public:
	AuSpacecraftDamageDock(QWidget* parent) : AuDockWidget(":data/MsgDamageReport.png", tr("<b>Spacecraft Damage</b>"),
			tr("Damage"), parent)
	{
		m_DamageReport = new AuDamageReport(this);
		setWidget(m_DamageReport);

		int dockSides = g_Au.Settings().readNumEntry("Dock/SpacecraftDamageAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/SpacecraftDamageAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuDamageReport_H
