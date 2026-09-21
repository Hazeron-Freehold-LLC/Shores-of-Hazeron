//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aurepairdamage.h,v 1.7 2022/02/19 18:00:07 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuRepairDamage_H
#define AuRepairDamage_H

#include "auclientlib.h"
#include "dbcommodity.h"
#include <QList>
#include <QWidget>

class AuServiceDialog;
class AuDamage;
class QLabel;
class QProgressBar;
class QPushButton;
class QScrollArea;
class QSpinBox;
class QVBoxLayout;

class AUCLIENT_DLL AuRepairDamageSystem : public QWidget
{
	Q_OBJECT;
private:
	AuSysId								m_SysId;
	QString								m_SysName;

	// Widgets
	QProgressBar*						m_Bar;
	QPushButton*						m_Estimate;
	QLabel*								m_Icon;
	QLabel*								m_Name;
	QLabel*								m_Parts;
	QLabel*								m_Percent;
	QLabel*								m_PercentIco;

public:
	AuRepairDamageSystem(QWidget* parent=0);
	~AuRepairDamageSystem();

	void Update(const AuSysId& sysId, const QString& sysName, const AuDamage& damage, quint32 maxHits);

protected slots:
	void Estimate();

signals:
	void Estimate(AuSysId sysId, QString systemName);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuRepairDamage : public QWidget
{
	Q_OBJECT;
private:
	// Widgets
	QSpinBox*							m_MinQ;
	QPushButton*						m_RepairButton;
	QVBoxLayout*						m_SystemLay;
	QScrollArea*						m_SystemScroller;
	QList<AuRepairDamageSystem*>		m_SystemWidgets;

public:
	AuRepairDamage(AuServiceDialog* parent=0);

	quint8 MinQ() const;

protected slots:
	void EstimateAll();
	void EstimateSystem(AuSysId sysId, QString systemName);
	void UpdateContents();
};

#endif // AuRepairDamage_H
