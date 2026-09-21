//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aushipmentcreate.h,v 1.8 2022/02/19 18:00:54 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuShipmentCreate_H
#define AuShipmentCreate_H

#include "auclientlib.h"
#include "audialog.h"
#include "aushipment.h"
#include <QTreeWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QTreeWidgetItem;

class AUCLIENT_DLL AuCityTree : public QTreeWidget
{
	Q_OBJECT;
	QTreeWidgetItem*					m_MousePressItem;		// Item under the mouse press.

public:
	AuCityTree(QWidget* parent=0);
	virtual ~AuCityTree()
	{
	}

signals:
	void ItemRightClicked(QTreeWidgetItem*);

protected:
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuShipmentCreate : public AuDialog
{
	Q_OBJECT;
	quint32								m_ShipperEmpire;
	GALAXY								m_ShipperGalaxy;
	AuId								m_ShipperId;
	bool								m_ShipperIsStation;
	AuId								m_ShipperSystemId;
	AuPoint								m_ShipperSystemLoc;

	// Widgets
	QLineEdit*							m_Amount;
	QComboBox*							m_Category;
	QRadioButton*						m_CategoryRadio;
	AuCityTree*							m_Cities;
	QComboBox*							m_Commodity;
	QRadioButton*						m_CommodityRadio;
	QCheckBox*							m_ContinuousCheck;
	QCheckBox*							m_EmpireCities;
	QCheckBox*							m_FriendCities;
	QComboBox*							m_MinQ;
	QCheckBox*							m_NeutralCities;
	QPushButton*						m_Ok;
	QSpinBox*							m_Repeat;
	QLabel*								m_TEUs;

public:
	// 'fromStation' is true if shipper is a DaVessel, otherwise it is a broker at a DaBd.
	AuShipmentCreate(const AuId& shipperId, quint32 shipperEmpire, bool shipperIsStation, const AuId& shipperSystemId,
			GALAXY shipperGalaxy, const AuPoint& shipperSystemLoc, QWidget* parent=0);
	virtual ~AuShipmentCreate();

	quint32 Amount() const;

	COMMODITYFILTER Category() const;

	COMMODITYID CommodityId() const;

	AuId DestCityId() const;

	QString DestDesc() const;

	float DestDistPc() const;

	AuId DestSectorId() const;

	AuId DestSystemId() const;

	quint8 MinQ() const;

	quint8 Repeat() const;

	// Returns 0 for commodity, 1 for category
	int SendWhat() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void EnableControls();
	void ListCategories();
	void ListCities();
	void ListCommodities();
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void AmountChanged(const QString& text);
	void CategoryChanged(int idx);
	void CityChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
	void CommodityChanged(int idx);
	void ContinuousChanged(int state);
	void EmpireChanged(int state);
	void FriendChanged(int state);
	void MinQChanged(int idx);
	void NeutralChanged(int state);
	void RadioToggled(bool checked);
	void RepeatChanged(int v);
	void RequestEmpireData();
	void ShipIt();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuTradeAgreementCreate : public AuDialog
{
	Q_OBJECT;
	quint32								m_ShipperEmpire;
	GALAXY								m_ShipperGalaxy;
	AuId								m_ShipperId;
	AuId								m_ShipperSystemId;
	AuPoint								m_ShipperSystemLoc;
	COMMODITYID							m_OrigCID;
	COMMODITYID							m_DestCID;
	COMMODITYFILTER						m_OrigFilter;
	COMMODITYFILTER						m_DestFilter;
	AuId								m_DestCity;

	// Widgets
	AuCityTree*							m_Cities;
	QCheckBox*							m_ContinuousCheck;
	QCheckBox*							m_EmpireCities;
	QCheckBox*							m_FriendCities;
	QCheckBox*							m_NeutralCities;
	QPushButton*						m_Ok;
	QSpinBox*							m_Repeat;

	QLabel*								m_GetTEUs;
	QLineEdit*							m_GetAmount;
	QComboBox*							m_GetCategory;
	QRadioButton*						m_GetCategoryRadio;
	QComboBox*							m_GetCommodity;
	QRadioButton*						m_GetCommodityRadio;
	QComboBox*							m_GetMinQ;

	QLabel*								m_SendTEUs;
	QLineEdit*							m_SendAmount;
	QComboBox*							m_SendCategory;
	QRadioButton*						m_SendCategoryRadio;
	QComboBox*							m_SendCommodity;
	QRadioButton*						m_SendCommodityRadio;
	QComboBox*							m_SendMinQ;

public:
	// Constructor for creating a new trade proposal.
	AuTradeAgreementCreate(const AuId& shipperId, quint32 shipperEmpire,
			const AuId& shipperSystemId, GALAXY shipperGalaxy, const AuPoint& shipperSystemLoc, QWidget* parent=0);
	// Constructor for countering a trade proposal.
	AuTradeAgreementCreate(const AuTradeAgreement& proposal, const AuId& shipperId, quint32 shipperEmpire,
			const AuId& shipperSystemId, GALAXY shipperGalaxy, const AuPoint& shipperSystemLoc, QWidget* parent=0);
	virtual ~AuTradeAgreementCreate();

	AuId DestCityId() const;
	QString DestDesc() const;
	float DestDistPc() const;
	quint32 DestEmpireId() const;
	AuId DestSectorId() const;
	AuId DestSystemId() const;

	quint32 GetAmount() const;
	COMMODITYFILTER GetCategory() const;
	COMMODITYID GetCommodityId() const;
	quint8 GetMinQ() const;
	int GetWhat() const;	// Returns 0 for commodity, 1 for category

	quint8 Repeat() const;

	quint32 SendAmount() const;
	COMMODITYFILTER SendCategory() const;
	COMMODITYID SendCommodityId() const;
	quint8 SendMinQ() const;
	int SendWhat() const;	// Returns 0 for commodity, 1 for category

public slots:
	virtual void setVisible(bool visible);

protected:
	void EnableControls();
	void ListCategories(QComboBox* cb, COMMODITYFILTER filter);
	void ListCities();
	void ListCommodities(QComboBox* cb, COMMODITYID cid);
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void CityChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
	void ContinuousChanged(int state);
	void EmpireChanged(int state);
	void FriendChanged(int state);
	void NeutralChanged(int state);
	void RepeatChanged(int v);
	void RequestEmpireData();
	void ShipIt();

	void GetAmountChanged(const QString& text);
	void GetCategoryChanged(int idx);
	void GetCommodityChanged(int idx);
	void GetMinQChanged(int idx);
	void GetRadioToggled(bool checked);

	void SendAmountChanged(const QString& text);
	void SendCategoryChanged(int idx);
	void SendCommodityChanged(int idx);
	void SendMinQChanged(int idx);
	void SendRadioToggled(bool checked);
};

#endif // AuShipmentCreate_H
