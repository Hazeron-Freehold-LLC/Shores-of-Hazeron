//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscchangestopdlg.h,v 1.2 2022/02/19 18:22:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScChangeStopDlg_H
#define AuScChangeStopDlg_H

#include "audialog.h"

class QComboBox;

class AuScChangeStopDlg : public AuDialog
{
	Q_OBJECT;
	// Widgets
	QComboBox*							m_Stops;

public:
	AuScChangeStopDlg(const QStringList& deletableStops, QWidget* parent=0);
	virtual ~AuScChangeStopDlg();

	QString StopName() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuScChangeStopDlg_H
