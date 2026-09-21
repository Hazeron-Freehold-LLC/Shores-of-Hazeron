//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auspacecraftshipments.h,v 1.8 2022/05/04 21:55:32 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSpacecraftShipments_H
#define AuSpacecraftShipments_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"
#include "aushipment.h"
#include <QWidget>

class QListWidget;
class QListWidgetItem;
class QPushButton;
class QSplitter;

class AUCLIENT_DLL AuSpacecraftShipments : public QWidget
{
	Q_OBJECT;
private:
	// Widgets
	QPushButton*						m_CreateShipment;
	QPushButton*						m_DeleteShipment;
	QPushButton*						m_RefreshShipments;
	QListWidget*						m_Shipments;
	QListWidget*						m_ShipmentsEnroute;
	QSplitter*							m_Splitter;

public:
	AuSpacecraftShipments(QWidget* parent);
	virtual ~AuSpacecraftShipments();

	virtual QSize sizeHint() const;

protected:
	void RequestShipments(bool sayResult);
	virtual void showEvent(QShowEvent* event);

public slots:
	void Shipment(AuShipment shipment);
	void ShipmentsClear();

protected slots:
	void CreateShipment();
	void DeleteShipment();
	void EnableControls();
	void RefreshShipments();
	void ShipmentChanged(QListWidgetItem*, QListWidgetItem*);
	void SplitterMoved(int pos, int index);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuSpacecraftShipmentsDock : public AuDockWidget
{
public:
	AuSpacecraftShipments*					m_Content;

public:
	AuSpacecraftShipmentsDock(QWidget* parent) : AuDockWidget(":data/c_Container.png", tr("<b>Spacecraft Shipments</b>"),
			tr("Shipments"), parent)
	{
		m_Content = new AuSpacecraftShipments(this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/SpacecraftShipmentsAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/SpacecraftShipmentsAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuSpacecraftShipments_H
