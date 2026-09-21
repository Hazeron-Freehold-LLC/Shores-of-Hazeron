//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscrenamestopdlg.h,v 1.2 2022/02/19 18:22:46 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScRenameStopDlg_H
#define AuScRenameStopDlg_H

#include "audialog.h"

class QComboBox;
class QLineEdit;

class AuScRenameStopDlg : public AuDialog
{
	Q_OBJECT;
	// Widgets
	QLineEdit*							m_NewName;
	QComboBox*							m_Stops;

public:
	AuScRenameStopDlg(const QStringList& deletableStops, QWidget* parent=0);
	virtual ~AuScRenameStopDlg();

	QString NewStopName() const;

	QString OldStopName() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuScRenameStopDlg_H
