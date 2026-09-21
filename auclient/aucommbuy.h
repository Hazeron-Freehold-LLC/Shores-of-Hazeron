//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommbuy.h,v 1.8 2022/02/19 17:54:08 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCommBuy_H
#define AuCommBuy_H

#include "auclientlib.h"
#include "audialog.h"
#include "auitem.h"
#include <QMetaType>

class QComboBox;
class QLabel;
class QPushButton;
class QSpinBox;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuOfferItem
{
public:
	AuItem*								m_Item;
	QString								m_EmpireOfOrigin;		// Name, not id number.

	inline AuOfferItem(const AuItem& item, const QString& empireOfOrigin)
	{
		m_Item = item.Clone();
		m_EmpireOfOrigin = empireOfOrigin;
	}

	inline AuOfferItem()
	{
		m_Item = 0;
	}

	inline AuOfferItem(const AuOfferItem& c)
	{
		if(c.m_Item)
			m_Item = c.m_Item->Clone();
		else
			m_Item = 0;
		m_EmpireOfOrigin = c.m_EmpireOfOrigin;
	}

	inline ~AuOfferItem()
	{
		if(m_Item)
			delete m_Item;
	}
};

Q_DECLARE_METATYPE(AuOfferItem)	// Needed for QVariant and queued signal/slot connections. Must also call qRegisterMetaType.

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuCommBuy : public AuDialog
{
	Q_OBJECT;
private:
	double								m_Account;				// Person's current account balance.
	AuItem*								m_BuyItem;				// Item to buy
	QString								m_EmpireOfOrigin;
	unsigned int						m_MaxCount;				// Max count available in m_BuyItem.
	float								m_NominalPrice;			// Nominal price of the item offered.
	QString								m_OfferEmpireOfOrigin;	// Empire name of origin of offered CID. n/a when offering money.
	AuItem*								m_OfferItem;			// Item offered in trade.
	float								m_UnitPrice;			// Unit price.

	// Widgets
	QSpinBox*							m_BuyCountSpin;			// Quantity to buy
	QComboBox*							m_OfferCombo;
	QSpinBox*							m_OfferCountSpin;		// Offer count
	QPushButton*						m_OkButton;
	QLabel*								m_UnitPriceInfoLabel;
	QLabel*								m_UnitPriceLabel;
	QLabel*								m_NominalPercent;

public:
	// Use default constructor for a "general" bid. Commodity and quality are selected on this dialog.
	// Supply parameter values for a "specific" bid. Commodity and quality cannot be changed.
	AuCommBuy(const AuItem& item, const QString& empireNameOfOrigin="", unsigned int price=0, QWidget* parent=0);
	virtual ~AuCommBuy();

	const QString& EmpireNameOfOrigin() const
	{
		return m_EmpireOfOrigin;
	}

	const AuItem* Item()
	{
		return m_BuyItem;
	}

	const QString& OfferEmpireOfOrigin() const
	{
		return m_OfferEmpireOfOrigin;
	}

	const AuItem* OfferItem()
	{
		return m_OfferItem;
	}

protected slots:
	void EnableControls();
	void OkPressed();
	void QuantityToBuyChanged(int qtyToBuy);
};

#endif // AuCommBuy_H
