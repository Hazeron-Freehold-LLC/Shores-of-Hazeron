//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auaccesshold.h,v 1.12 2022/05/04 21:20:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAccessHold_H
#define AuAccessHold_H

#include "auclientlib.h"
#include "aucommodityfilter.h"
#include "audockwidget.h"
#include "auglobal.h"
#include "auitem.h"
#include <QWidget>

class QLabel;
class QListWidget;
class QListWidgetItem;
class QProgressBar;
class QPushButton;
class QTextBrowser;
class QTimer;

class AUCLIENT_DLL AuAccessHold : public QWidget
{
	Q_OBJECT;
public:
	enum MODE
	{
		Invalid,
		Armory,
		Bar,
		Feeder,
		FuelCoupling,
		Hold,
		Pantry,
		Pharmacy,
		Wardrobe,
		Garage,
		SeaChest,
	};


private:
	AuItemHash		m_CargoShown;
	AuItemHash		m_GearShown;
	COMMODITYFILTER	m_GearFilter;
	MODE			m_Mode;
	QTimer*			m_timer;

	QTextBrowser*		cargodesc;
	AuCommodityFilter*	cfilt;
	QTextBrowser*		invdesc;
	QLabel*				lbcargo;
	QLabel*				lbct;
	QLabel*				lbnot;
	QListWidget*		lwcargo;
	QListWidget*		lwGear;
	QProgressBar*		m_FillCargo;
	QPushButton*		pbput;
	QPushButton*		pbtake;

public:
	AuAccessHold(QWidget* parent=0);
	~AuAccessHold();

	virtual QSize sizeHint() const;

protected:
	virtual void hideEvent(QHideEvent* event);
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void showEvent(QShowEvent* event);

protected slots:
	void CargoDoubleClicked(QListWidgetItem* item);
	void CargoSelected();
	void EnableControls();
	void FilterGear(COMMODITYFILTER);
	void GearDoubleClicked(QListWidgetItem* item);
	void GearSelected();
	void PutIntoCargo();
	void TakeFromCargo();
	void UpdateContents();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuCargoHoldDock : public AuDockWidget
{
public:
	AuAccessHold*						m_Content;

public:
	AuCargoHoldDock(QWidget* parent) : AuDockWidget(":data/Shelf.png", tr("<b>Cargo Hold</b>"),
			tr("Hold"), parent)
	{
		m_Content = new AuAccessHold(this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/CargoHoldAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/CargoHoldAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuAccessHold_H
