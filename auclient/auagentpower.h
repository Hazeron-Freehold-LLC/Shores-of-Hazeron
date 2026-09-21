//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auagentpower.h,v 1.7 2022/02/19 17:49:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAgentPower_H
#define AuAgentPower_H

#include "auclientlib.h"
#include "audialog.h"
#include "auid.h"
#include "dbcommodity.h"
#include <QWidget>

class AuDNABase;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QSpinBox;

class AUCLIENT_DLL AuAgentPower : public QWidget
{
	Q_OBJECT;
private:
	// Widgets
	QLabel*								m_MyServer;
	QCheckBox*							m_ckAgent;
	QCheckBox*							m_ckAnimal;
	QCheckBox*							m_ckEntity;
	QCheckBox*							m_ckIncognito;
	QCheckBox*							m_ckInvincible;
	QCheckBox*							m_ckInvisible;

public:
	AuAgentPower(QWidget* parent=0);
	virtual ~AuAgentPower();

public slots:
	virtual void setVisible(bool visible);
	void VisibilityRestored();

protected slots:
	void CancelStory();
	void ChangeMyDNA();
	void MakeItem();
	void ResetStory();
	void SubmitStory();
	void ToggleAgentBits();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuAgentMakeItem : public AuDialog
{
	Q_OBJECT;
public:
	enum MAKEWHAT
	{
		Nothing,
		OneItem,
		MultipleItems,
		CityInventory,
		ShipCargo,
		UtilityPouch,
		Knapsack,
		Backpack,
		GravPack,
	};

private:
	MAKEWHAT							m_MakeWhat;				// Set based on what button was pressed.

	// Widgets
	QComboBox*							m_Commodity;
	QLineEdit*							m_Count;
	QComboBox*							m_Galaxy;
	QLineEdit*							m_GalaxyCoord;
	QSpinBox*							m_Quality;

public:
	AuAgentMakeItem(QWidget* parent=0);
	virtual ~AuAgentMakeItem();

	COMMODITYID CommodityId() const;

	quint32 Count() const;

	AuPoint GalaxyCoord() const;

	// Return value is a GALAXY enum value.
	quint8 Galaxy() const;

	inline MAKEWHAT MakeWhat() const
	{
		return m_MakeWhat;
	}

	quint8 Quality() const;

public slots:
	void MakeBackpack();
	void MakeCityInventory();
	void MakeGravPack();
	void MakeItems();
	void MakeKnapsack();
	void MakeOne();
	void MakeShipCargo();
	void MakeUtilityPouch();

protected slots:
	void MakeQualityChanged(int index);
};

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuAgentPowerSetDNA : public AuDialog
{
	Q_OBJECT;
private:
	AuDNABase*							m_DNA;					// The DNA

	// Widgets
	QLabel*								m_DNADesc;

public:
	AuAgentPowerSetDNA(const AuDNABase& defaultDNA, QWidget* parent=0);
	virtual ~AuAgentPowerSetDNA();

	inline const AuDNABase& DNA() const
	{
		return *m_DNA;
	}

protected slots:
	void DNAChanged();
	void LoadDNAFromFile();
	void RandomDNA();
	void RandomRatDNA();
	void SaveDNAToFile();
	void WorkerDNA();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#endif // AuAgentPower_H
