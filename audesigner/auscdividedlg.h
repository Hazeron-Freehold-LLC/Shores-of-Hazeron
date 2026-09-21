//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscdividedlg.h,v 1.2 2022/02/19 18:22:32 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScDivideDlg_H
#define AuScDivideDlg_H

#include "audialog.h"

class QSpinBox;

class AuScDivideDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QSpinBox*							m_DivisionsSpin;

public:
	AuScDivideDlg(QWidget* parent=0);
	virtual ~AuScDivideDlg();

	quint8 Divisions() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DoDivisionsEntered();
};

#endif // AuScDivideDlg_H
