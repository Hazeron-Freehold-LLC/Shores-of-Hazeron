//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldmanufacture.h,v 1.14 2022/05/05 20:56:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBldManufacture_H
#define AuBldManufacture_H

#include "auclientlib.h"
#include "audialog.h"
#include "auid.h"
#include "auknownpatents.h"
#include "dbcommodity.h"
#include <QWidget>

class DaBd;
class DaWorldLL;
class QGroupBox;
class QLabel;
class QListWidget;
class QPushButton;
class QScrollArea;
class QSpinBox;
class QTableWidget;
class QTimer;
class QToolButton;
class QVBoxLayout;

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuBdWorkshopSettings : public AuDialog
{
	Q_OBJECT;

	// Widgets
	QSpinBox*							m_MinQ;
	QSpinBox*							m_StockLimit;
	QSpinBox*							m_Workshops;

public:
	AuBdWorkshopSettings(quint8 curMinQ, quint8 procMinQ, quint32 curWorkshops, quint32 maxWorkshops, quint32 curStockLimit,
			QWidget* parent=0);
	virtual ~AuBdWorkshopSettings();

	quint8 MinQ() const;

	// 0=no limit
	quint32 StockLimit() const;

	quint32 Workshops() const;

public slots:
	virtual void setVisible(bool visible);
	void Shop1();
	void Stock0();
	void Stock1();
	void Stock10();
	void Stock100();
	void Stock1000();

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuBdManufacturePanel : public QWidget
{
	Q_OBJECT;
private:
	AuId								m_CapitolOpId;			// Id of building operated when person is in a capitol building.
	bool								m_InCoC;				// True if the scene atom is in the chain of command.
	quint32								m_ShopsEnabled;			// shops/citizen workers enabled
	quint32								m_ShopsMax;
	quint32								m_StockLimit;
	int									m_MPID;					// manufacturing process id
	quint8								m_CurMinQuality;
	quint8								m_ProcMinQ;

	// Widget
	QListWidget*						m_Components;
	QToolButton*						m_DeleteProcess;
	QPushButton*						m_Fetch;
	QLabel*								m_LastOutput;
	QToolButton*						m_LowerPriority;
	QLabel*								m_MakeWhatIcon;
	QLabel*								m_MakeWhatName;
	QToolButton*						m_MaxPriority;
	QToolButton*						m_MinPriority;
	QTableWidget*						m_Modifiers;
	QToolButton*						m_RaisePriority;
	QPushButton*						m_Run;
	QLabel*								m_Status;
	QLabel*								m_StatusIcon;
	QPushButton*						m_Workshops;

public:
	AuBdManufacturePanel(QWidget* parent=0);
	virtual ~AuBdManufacturePanel();

	inline quint16 ManufacturingProcessId() const
	{
		return m_MPID;
	}

	// 'inCoC' is true if the scene atom is in the chain of command of my building.
	void SetChainOfCommand(bool inCoC);

	void SetShopCount(quint32 shops);

	void UpdateContents(const DaBd* bd, quint16 panelIndex, const AuId& capitolOpId);

protected slots:
	void DeleteProcess();
	void FetchPushed();
	void GoPushed();
	void LowerPriority();
	void MaxPriority();
	void MinPriority();
	void RaisePriority();
	void WorkshopsClicked();

signals:
	void CitizenEnabled(quint16 mpid, quint32 enabled, AuId capitolOpId);
	void DeleteProcess(quint16 mpid, AuId capitolOpId);
	void Fetch(quint16 mpid, AuId capitolOpId);
	void Go(quint16 mpid, AuId capitolOpId);
	void MinQSet(quint16 mpid, quint8 minQ, AuId capitolOpId);
	void PriorityLowered(quint16 mpid, AuId capitolOpId);
	void PriorityMax(quint16 mpid, AuId capitolOpId);
	void PriorityMin(quint16 mpid, AuId capitolOpId);
	void PriorityRaised(quint16 mpid, AuId capitolOpId);
	void StockLimitSet(quint16 mpid, quint32 stockLimit, AuId capitolOpId);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuBldManufacture : public QWidget
{
	Q_OBJECT;
private:
	AuId								m_CapitolOpId;			// Id of building operated when person is in a capitol building.
	quint32								m_ShopsCount;
	QTimer*								m_timer;

	// Widgets
	QToolButton*						m_AddProcess;
	QToolButton*						m_ClearProcesses;
	QToolButton*						m_ProcessQ;
	QVBoxLayout*						m_PanelLay;
	QList<AuBdManufacturePanel*>		m_Panels;
	QScrollArea*						m_PanelScroller;
	QToolButton*						m_PatentsGet;
	QToolButton*						m_PatentsRecord;
	QToolButton*						m_PatentsTree;
	QLabel*								m_ShopsLabel;

public:
	AuBldManufacture(QWidget* parent=0);
	virtual ~AuBldManufacture();

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
	void AddProcess();
	void ClearProcesses();
	void GetPatent();
	void PatentsTree();
	void ProcessQ();
	void RecordPatent();
	void UpdateContents();

signals:
	void AddProcess(quint16 mpid, quint8 minQ, quint32 workshops, quint32 stockLimit, AuId capitolOpId);
};

#endif // AuBldManufacture_H
