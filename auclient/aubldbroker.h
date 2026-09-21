//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldbroker.h,v 1.8 2022/02/19 17:51:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBldBroker_H
#define AuBldBroker_H

#include "auclientlib.h"
#include "auid.h"
#include "aushipment.h"
#include <QWidget>

class QListWidget;
class QListWidgetItem;
class QPushButton;
class QSplitter;
class QTimer;

class AUCLIENT_DLL AuBldBroker : public QWidget
{
	Q_OBJECT;
private:
	AuId								m_CapitolOpId;			// Id of building when operated from a capitol building.
	int									m_ShipmentOffices;		// Offices used by current shipments.
	int									m_TradeOffices;			// Offices used by current trade agreements.

	// Widgets
	QPushButton*						m_AcceptTradeAgreement;
	QPushButton*						m_CounterTradeAgreement;
	QPushButton*						m_CreateShipment;
	QPushButton*						m_CreateTradeAgreement;
	QPushButton*						m_DeleteShipment;
	QPushButton*						m_DeleteTradeAgreement;
	QPushButton*						m_RefreshShipments;		// and trade agreements
	QListWidget*						m_Shipments;
	QListWidget*						m_ShipmentsEnroute;
	QSplitter*							m_Splitter;
	QListWidget*						m_TradeAgreements;

public:
	AuBldBroker(QWidget* parent=0);
	virtual ~AuBldBroker();

	// Clear information needed when this building is operated from a capitol building.
	void ClearCapitolOp()
	{
		m_CapitolOpId = AuId(0);
	}

	// Set information needed when this building is operated from a capitol building.
	void SetCapitolOp(const AuId& capitolOpId)
	{
		m_CapitolOpId = capitolOpId;
	}

protected:
	virtual void hideEvent(QHideEvent* event);
	void RequestShipments(bool sayResult);	// and trade agreements
	virtual void showEvent(QShowEvent* event);

public slots:
	void Shipment(AuShipment shipment);
	void ShipmentsClear();
	void TradeAgreement(AuTradeAgreement ta);
	void TradeAgreementsClear();

protected slots:
	void AcceptTradeAgreement();
	void CounterTradeAgreement();
	void CreateShipment();
	void CreateTradeAgreement();
	void DeleteShipment();
	void DeleteTradeAgreement();
	void EnableControls();
	void RefreshShipments();	// and trade agreements
	void ShipmentChanged(QListWidgetItem*, QListWidgetItem*);
	void SplitterMoved(int pos, int index);
	void TradeAgreementChanged(QListWidgetItem*, QListWidgetItem*);
};

#endif // AuBldBroker_H
