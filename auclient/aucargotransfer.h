//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucargotransfer.h,v 1.11 2022/05/04 21:19:25 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCargoTransfer_H
#define AuCargoTransfer_H

#include "auclientlib.h"
#include "aucommodityfilter.h"
#include "audockwidget.h"
#include "auglobal.h"
#include "auid.h"
#include <QWidget>

class AuItem;
class AuItemHash;
class DaVessel;
class QComboBox;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;
class QTextBrowser;
class QTimer;

class AUCLIENT_DLL AuCargoTransfer : public QWidget
{
	Q_OBJECT;
private:
	COMMODITYFILTER				m_GearFilter;
	double						m_MyMoneyAmt;
	AuId						m_MyShipId;
	double						m_OtherMoneyAmt;
	AuId						m_OtherShipId;
	QTimer*						m_timer;			// For updates.

	// Widgets
	AuCommodityFilter*			m_Filter;
	QListWidget*				m_MyCargo;
	//QTextBrowser*				m_MyCargoDesc;
	QTableWidget*				m_MyCitizens;
	QLabel*						m_MyDesc;
	QPushButton*				m_MyItems;
	QPushButton*				m_MyMoney;
	QLabel*						m_MyName;
	QListWidget*				m_OtherCargo;
	//QTextBrowser*				m_OtherCargoDesc;
	QTableWidget*				m_OtherCitizens;
	QLabel*						m_OtherDesc;
	QPushButton*				m_OtherItems;
	QPushButton*				m_OtherMoney;
	QLabel*						m_OtherName;
	QComboBox*					m_OtherShips;
	QPushButton*				m_SendCargo;
	QPushButton*				m_SendCitizens;
	QPushButton*				m_SendSomeCargo;
	QPushButton*				m_TakeCargo;
	QPushButton*				m_TakeCitizens;
	QPushButton*				m_TakeSomeCargo;

public:
	AuCargoTransfer(QWidget* parent=0);
	virtual ~AuCargoTransfer();

	void TransferCitizensAvailable(const QByteArray& ba);

	virtual QSize sizeHint() const;

protected:
	void ListCargo(QListWidget* list, const DaVessel* ship);
	void ListCargoItem(QListWidget* list, const AuItem& item, AuItemHash& cargoInList);
	void ListCitizens(QTableWidget* table, const DaVessel* ship);
	virtual void showEvent(QShowEvent* event);

protected slots:
	void Filter(COMMODITYFILTER);
	void MyCargoSelected();
	void OtherCargoSelected();
	void RequestMyCitizens();
	void RequestOtherCitizens();
	void SendCargo();
	void SendCargoItem(QListWidgetItem* item);
	void SendCitizen(QTableWidgetItem* item);
	void SendCitizens();
	void SendItems();
	void SendMoney();
	void SendSomeCargo();
	void TakeCargo();
	void TakeCargoItem(QListWidgetItem* item);
	void TakeCitizen(QTableWidgetItem* item);
	void TakeCitizens();
	void TakeItems();
	void TakeMoney();
	void TakeSomeCargo();
	void UpdateContents();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuCargoTransferDock : public AuDockWidget
{
public:
	AuCargoTransfer*					m_Content;

public:
	AuCargoTransferDock(QWidget* parent) : AuDockWidget(":data/c_Spacecraft.png", tr("<b>Cargo Transfer</b>"),
			tr("Transfer"), parent)
	{
		m_Content = new AuCargoTransfer(this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/CargoTransferAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/CargoTransferAreas", (int)allowedAreas());
	}

	inline void TransferCitizensAvailable(const QByteArray& ba)
	{
		m_Content->TransferCitizensAvailable(ba);
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuCargoTransfer_H
