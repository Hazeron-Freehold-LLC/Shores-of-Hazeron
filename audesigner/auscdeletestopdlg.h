//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscdeletestopdlg.h,v 1.2 2022/02/19 18:22:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScDeleteStopDlg_H
#define AuScDeleteStopDlg_H

#include "audialog.h"

class QComboBox;

class AuScDeleteStopDlg : public AuDialog
{
	Q_OBJECT;
	// Widgets
	QComboBox*							m_Stops;

public:
	AuScDeleteStopDlg(const QStringList& deletableStops, QWidget* parent=0);
	virtual ~AuScDeleteStopDlg();

	QString StopName() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuScDeleteStopDlg_H
