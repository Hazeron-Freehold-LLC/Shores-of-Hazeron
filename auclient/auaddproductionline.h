//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auaddproductionline.h,v 1.11 2022/02/19 17:49:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAddProductionLine_H
#define AuAddProductionLine_H

#include "auclientlib.h"
#include "audialog.h"
#include "auid.h"

class QComboBox;
class QLabel;
class QPushButton;
class QSpinBox;

class AUCLIENT_DLL AuAddProductionLine : public AuDialog
{
	Q_OBJECT;
	AuId								m_CapitolOpId;

	// Widgets
	QLabel*								lbMinQ;
	QLabel*								lbStockLimit;
	QLabel*								lbWorkshops;
	QSpinBox*							m_MinQ;
	QComboBox*							m_ProcCombo;
	QLabel*								m_ProcLabel;
	QSpinBox*							m_StockLimit;
	QSpinBox*							m_Workshops;
	QPushButton*						pbshop1;
	QPushButton*						pbstock0;
	QPushButton*						pbstock1;
	QPushButton*						pbstock10;
	QPushButton*						pbstock100;
	QPushButton*						pbstock1000;

public:
	AuAddProductionLine(const AuId& capitolOpId, quint32 maxWorkshops, QWidget* parent=0);
	virtual ~AuAddProductionLine();

	quint8 MinQ() const;

	quint16 MPID() const;

	// 0=no limit
	quint32 StockLimit() const;

	quint32 Workshops() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void ListProcesses();
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void MinQChanged(int value);
	void ProcessChanged(int index);
	void Shop1();
	void StockLimitChanged(int value);
	void Stock0();
	void Stock1();
	void Stock10();
	void Stock100();
	void Stock1000();
};

#endif // AuAddProductionLine_H
