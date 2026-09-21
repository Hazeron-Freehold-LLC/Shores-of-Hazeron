//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscaddstopdlg.h,v 1.2 2022/02/19 18:22:02 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScAddStopDlg_H
#define AuScAddStopDlg_H

#include "audialog.h"

class QLineEdit;

class AuScAddStopDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QLineEdit*							m_StopName;

public:
	AuScAddStopDlg(QWidget* parent=0);
	virtual ~AuScAddStopDlg();

	QString StopName() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuScAddStopDlg_H
