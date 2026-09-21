//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscconsolemoduledlg.h,v 1.2 2022/02/19 18:22:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScConsoleModuleDlg_H
#define AuScConsoleModuleDlg_H

#include "audialog.h"
#include "dbcommodity.h"

class QComboBox;

class AuScConsoleModuleDlg : public AuDialog
{
	Q_OBJECT;

	// Widgets
	QComboBox*							m_Modules;
	QComboBox*							m_WhichEffect;

public:
	// 'isSc' is true for spacecraft designs, false for building designs.
	AuScConsoleModuleDlg(SYSTEM system, bool isSc, QWidget* parent=0);
	virtual ~AuScConsoleModuleDlg();

	COMMODITYID Module() const;

	int WhichEffect() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuScConsoleModuleDlg_H
