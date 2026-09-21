//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldstore.h,v 1.8 2022/02/19 17:52:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBldStore_H
#define AuBldStore_H

#include "auclientlib.h"
#include "auid.h"
#include "auitem.h"
#include "dbcommodity.h"	// for CommoditySubKey
#include <QWidget>

class AuItem;
class DaBd;
class QComboBox;
class QLabel;
class QListWidget;
class QPushButton;
class QSpinBox;
class QTextBrowser;
class QTimer;

class AUCLIENT_DLL AuBldStore : public QWidget
{
	Q_OBJECT;
private:
	double								m_Account;				// Person's current account balance.
	QStringList							m_AllCargo;				// To monitor for changes.
	AuItem*								m_BuyItem;				// Item to buy
	AuId								m_CapitolOpId;			// Id of building when operated from a capitol building.
	QString								m_EmpireOfOrigin;
	COMMODITYFILTER						m_Filter;
	AuItemHash							m_Inv;					// Person's current inventory, to monitor for changes.
	bool								m_IsOpen;				// true if this store is open for business.
	quint32								m_Lounges;
	unsigned int						m_MaxCount;				// Max count available in m_BuyItem.
	float								m_NominalPrice;			// Nominal price of the item offered.
	AuItem*								m_OfferItem;			// Item offered in trade.
	quint32								m_Stores;
	QTimer*								m_Timer;
	float								m_UnitPrice;			// Unit price.

	// Widgets
	QLabel*								m_AccountBalance;
	QListWidget*						m_Cargo;
	QTextBrowser*						m_CargoDesc;
	QSpinBox*							m_BuyCountSpin;			// Quantity to buy
	QComboBox*							m_OfferCombo;
	QSpinBox*							m_OfferCountSpin;		// Offer count
	QPushButton*						m_OkButton;
	QLabel*								m_OkIcon;
	QLabel*								m_Price;
	QPushButton*						m_SellButton;
	QLabel*								m_UnitPriceInfoLabel;
	QLabel*								m_UnitPriceLabel;
	QLabel*								m_NominalPercent;

public:
	AuBldStore(QWidget* parent=0);
	virtual ~AuBldStore();

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
	virtual void showEvent(QShowEvent* event);

protected slots:
	void CargoSelected();
	void EnableControls();
	void Filter(COMMODITYFILTER);
	void OkPressed();
	void QuantityToBuyChanged(int qtyToBuy);
	void SellPressed();
	void UpdateContents();

protected:
	void ListCargo(DaBd* bdOp, quint32 buyerEmpireId, QListWidget* lw);
};

#endif // AuBldStore_H
