//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucargojettison.h,v 1.11 2022/02/19 17:53:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCargoJettison_H
#define AuCargoJettison_H

#include "auclientlib.h"
#include "aucommodityfilter.h"
#include "audockwidget.h"
#include "auglobal.h"
#include <QMap>
#include <QWidget>

class AuCommChannel;
class AuItem;
class DaVessel;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QTextBrowser;
class QProgressBar;
class QPushButton;

class AUCLIENT_DLL AuCargoListItem
{
public:
	QByteArray		m_Code;
	QString			m_IconName;
	QString			m_ToolTip;

	bool operator==(const AuCargoListItem& c) const
	{
		return m_Code == c.m_Code;
	}
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuCargoJettison : public QWidget
{
	Q_OBJECT;
private:
	COMMODITYFILTER		m_CargoFilter;
	quint32				m_MaxCount;
	QTimer*				m_timer;
	QMap<QString, AuCargoListItem>	m_CargoItems;

	// Widgets
	QTextBrowser*		m_CargoDesc;
	QListWidget*		m_CargoList;
	AuCommodityFilter*	m_CargoFilterWidget;
	QPushButton*		m_CombineButton;
	QComboBox*			m_CombineWith;
	QLineEdit*			m_CountEdit;
	QProgressBar*		m_FillCargo;
	QProgressBar*		m_FillFuel;
	QPushButton*		m_JettisonButton;
	QPushButton*		m_OpenTEUButton;

public:
	AuCargoJettison(QWidget* parent=0);
	virtual ~AuCargoJettison();

protected slots:
	void CargoDoubleClicked(QListWidgetItem* item);
	void CargoSelected();
	void CombinePressed();
	void EnableControls();
	void FilterCargo(COMMODITYFILTER);
	void JettisonPressed();
	void OpenTEUPressed();
	void UpdateContents();

protected:
	virtual void hideEvent(QHideEvent* event);
	void JettisonCargo(const AuItem& item);
	void ListCargo(DaVessel* ship);
	void OpenTEUs(const AuItem& item);
	virtual void showEvent(QShowEvent* event);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuCargoJettisonDock : public AuDockWidget
{
public:
	AuCargoJettison*					m_Content;

public:
	AuCargoJettisonDock(QWidget* parent) : AuDockWidget(":data/c_Crate.png", tr("<b>Cargo Manifest</b>"),
			tr("Manifest"), parent)
	{
		m_Content = new AuCargoJettison(this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/CargoManifestAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/CargoManifestAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuCargoJettison_H
