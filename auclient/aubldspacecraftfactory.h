//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldspacecraftfactory.h,v 1.6 2022/02/19 17:52:29 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBldSpacecraftFactory_H
#define AuBldSpacecraftFactory_H

#include "auclientlib.h"
#include "auglobell.h"
#include "auid.h"
#include <QWidget>

class DaBd;
class DeManufacturingProcess;
class QCheckBox;
class QComboBox;
class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QSpinBox;
class QTableWidget;
class QTimer;
class QToolButton;

class AUCLIENT_DLL AuBldSpacecraftFactory : public QWidget
{
	Q_OBJECT;
private:
	AuId								m_CapitolOpId;			// Id of building operated when person is in a capitol building.
	bool								m_CitizenOk;			// is citizen worker enabled?
	quint8								m_CompanyMaxQCur;
	bool								m_CompanyOkCur;
	bool								m_InCoC;				// True if scene atom is in my chain of command.
	bool								m_InitCombos;
	QTimer*								m_timer;
	quint8								m_CurMinQuality;
	quint32								m_PersonEmpire;
	quint8								m_QRangeMax;
	quint8								m_QRangeMin;

	// Widgets
	QPushButton*						m_BuyCompany;
	QPushButton*						m_Cancel;
	QPushButton*						m_Commission;
	QSpinBox*							m_CompanyMaxQ;
	QCheckBox*							m_CompanyOk;
	QListWidget*						m_CompanyQueue;
	QListWidget*						m_Components;
	QPushButton*						m_Delete;
	QCheckBox*							m_EnableWorker;
	QPushButton*						m_Fetch;
	QLabel*								m_LastOutput;
	QPushButton*						m_MakeWhat;
	QSpinBox*							m_MinQ;
	QTableWidget*						m_Modifiers;
	QToolButton*						m_PatentsGet;
	QToolButton*						m_PatentsRecord;
	QPushButton*						m_Run;
	QLabel*								m_Running;
	QLabel*								m_RunningIcon;
	QLabel*								m_Status;
	QLabel*								m_StatusIcon;

public:
	AuBldSpacecraftFactory(QWidget* parent=0);
	virtual ~AuBldSpacecraftFactory();

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
	void BuyOne();
	void CancelPushed();
	void ChooseDefaultProcess();
	void CitizenEnableChanged(int state);
	void Commission();
	void CompanyEnableChanged(int state);
	void CompanyMaxQChanged(int index);
	void Delete();
	void FetchPushed();
	void GetPatent();
	void GoPushed();
	void MinQChanged(int index);
	void QueueSelChanged(QListWidgetItem* current, QListWidgetItem* previous);
	void RecordPatent();
	void UpdateContents();

private:
	void UpdateContents(DeManufacturingProcess* mp, DaBd* scf);
};

#endif // AuBldSpacecraftFactory_H
