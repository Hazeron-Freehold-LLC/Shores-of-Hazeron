//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovpolicy.h,v 1.10 2022/02/19 17:57:11 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGovPolicy_H
#define AuGovPolicy_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"
#include "auid.h"
#include "dgempire.h"
#include <QStandardItemModel>
#include <QWidget>

class AuColorButton;
class AuHueSelector;
class AuContrailSample;
class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QSlider;
class QToolButton;

class AUCLIENT_DLL AuGovPolicy : public QWidget
{
	Q_OBJECT;
	DgEmpire			m_Empire;

	// Widgets
	AuHueSelector*		hueContrail;
	QToolButton*		m_ChangeFlag;
	QPushButton*		m_JoinPass;
	AuContrailSample*	sampleContrail;
	QSlider*			satContrail;
	AuColorButton*		m_ThemeExteriorPrimary;
	AuColorButton*		m_ThemeExteriorSecondary;
	AuColorButton*		m_ThemeExteriorTertiary;
	AuColorButton*		m_ThemeInteriorPrimary;
	AuColorButton*		m_ThemeInteriorSecondary;
	AuColorButton*		m_ThemeInteriorTertiary;

public:
	AuGovPolicy(QWidget* parent=0);

	void UpdateEmpireRecord();

protected slots:
	void ChangeFlag();
	void EnableControls();
	void SetContrailHue(quint8 hue);
	void SetContrailSaturation(int sat);
	void SetJoinPassword();
	void SetThemeColorExteriorPrimary(QColor color);
	void SetThemeColorExteriorSecondary(QColor color);
	void SetThemeColorExteriorTertiary(QColor color);
	void SetThemeColorInteriorPrimary(QColor color);
	void SetThemeColorInteriorSecondary(QColor color);
	void SetThemeColorInteriorTertiary(QColor color);

protected:
	virtual void showEvent(QShowEvent* event);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuGovPolicyDock : public AuDockWidget
{
public:
	AuGovPolicy*					m_Content;

public:
	AuGovPolicyDock(QWidget* parent) : AuDockWidget(":data/CommGovernment.png", tr("<b>Policy</b>"), tr("Govern Policy"), parent)
	{
		m_Content = new AuGovPolicy(this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/GovernPolicyAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/GovernPolicyAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuGovPolicy_H
