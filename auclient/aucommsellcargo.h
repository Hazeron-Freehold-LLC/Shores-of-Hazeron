//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommsellcargo.h,v 1.9 2022/02/19 17:54:49 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AUCOMMSELLCARGO_H
#define AUCOMMSELLCARGO_H

#include "auclientlib.h"
#include "aucommodityfilter.h"
#include <audialog.h>

class AuCommodityFilter;
class AuGear;
class AuItem;
class DaVessel;
class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QRadioButton;
class QTextBrowser;

class AUCLIENT_DLL AuCommSellCargo : public AuDialog
{
	Q_OBJECT;
private:
	COMMODITYFILTER	m_Filter;
	QString			m_EmpireOfOrigin;	// empire that made the item
	unsigned int	m_MaxCount;			// max quantity of currently selected item that is available for sale
	AuItem*			m_Item;				// The item offered for sale.
	unsigned int	m_Price;			// asking price, 0 for auctions
	bool			m_Shipment;
	QTimer*			m_timer;

	// Widgets
	QRadioButton*						m_AuctionRadio;
	QListWidget*						m_Cargo;
	QTextBrowser*						m_CargoDesc;
	AuCommodityFilter*					m_CommodityFilter;
	QLineEdit*							m_CountEdit;
	QLabel*								m_CurrencySymbolLabel;
	QRadioButton*						m_GiveRadio;
	QListWidget*						m_Inventory;
	QTextBrowser*						m_InventoryDesc;
	QPushButton*						m_OkButton;
	QPushButton*						m_OkTEUButton;
	QLineEdit*							m_PriceEdit;
	QLabel*								m_PriceLabel;
	QRadioButton*						m_SellRadio;
	QLabel*								m_TotalLabel;
	QLabel*								m_TotalPrice;

public:
	AuCommSellCargo(QWidget* parent=0);
	virtual ~AuCommSellCargo();

	const QString EmpireOfOrigin() const;

	bool IsAuction() const;

	bool IsGiveAway() const;

	bool IsSale() const;

	bool IsShipment() const;

	const AuItem* Item() const
	{
		return m_Item;
	}

	unsigned int Price() const;

public slots:
	virtual void setVisible(bool visible);

protected slots:
	void AuctionRadio();
	void CargoDoubleClicked(QListWidgetItem* item);
	void CargoSelected();
	void EnableControls();
	void Filter(COMMODITYFILTER);
	void GiveRadio();
	void InventoryDoubleClicked(QListWidgetItem* item);
	void InventorySelected();
	void OkPressed();
	void OkTEUPressed();
	void SellRadio();
	void UpdateContents();

protected:
	bool ListCargo(DaVessel* ship, QListWidget* lw);
	bool ListInventory(const AuGear& gear, QListWidget* lw);
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AUCOMMSELLCARGO_H
