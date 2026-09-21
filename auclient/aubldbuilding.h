//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldbuilding.h,v 1.9 2022/02/19 17:51:26 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBldBuilding_H
#define AuBldBuilding_H

#include "auclientlib.h"
#include "auid.h"
#include "dbcommodity.h"
#include <QWidget>

class AuCommodityFilter;
class AuIconButton;
class AuItem;
class DaBd;
class QLabel;
class QListWidget;
class QPushButton;
class QSpinBox;
class QTextBrowser;
class QTimer;

class AUCLIENT_DLL AuBldBuilding : public QWidget
{
	Q_OBJECT;
private:
	QString								m_AssignedName;
	QStringList							m_AllCargo;				// To monitor for changes.
	AuItem*								m_BuyItem;				// Item to buy
	AuId								m_CapitolOpId;			// Id of building when operated from a capitol building.
	bool								m_InCoC;				// true if in chain of command of person.
	QString								m_EmpireOfOrigin;
	COMMODITYFILTER						m_Filter;
	QString								m_InfoText;
	unsigned int						m_MaxCount;				// Max count available in m_BuyItem.
	QTimer*								m_Timer;

	// Widgets
	QSpinBox*							m_BuyCountSpin;			// Quantity to take
	QListWidget*						m_Cargo;
	QTextBrowser*						m_CargoDesc;
	AuCommodityFilter*					m_CommodityFilter;
	QLabel*								m_CountLabel;
	QLabel*								m_Icon;
	QTextBrowser*						m_Info;
	QPushButton*						m_OkButton;
	AuIconButton*						m_Rename;
	QPushButton*						m_TrashButton;

public:
	AuBldBuilding(QWidget* parent=0);
	virtual ~AuBldBuilding();

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
	void Rename();
	void TrashPressed();
	void UpdateContents();

protected:
	void ListCargo(DaBd* bdOp, QListWidget* lw);
};

#endif // AuBldBuilding_H
