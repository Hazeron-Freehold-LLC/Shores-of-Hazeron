//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscexportstldlg.h,v 1.2 2022/02/19 18:22:36 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScExportSTLDlg_H
#define AuScExportSTLDlg_H

#include "audialog.h"

class QCheckBox;
class QComboBox;
class QPushButton;

class AuScExportSTLDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QPushButton*						m_FileName;
	QComboBox*							m_UnitsOfMeasure;

public:
	AuScExportSTLDlg(QWidget* parent=0);
	virtual ~AuScExportSTLDlg();

	QString FileName() const;

	// Returns the scale factor needed to convert from outgoing values to feet.
	double ScaleToFeet() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void ChooseFile();
	void DoExport();
};

#endif // AuScExportSTLDlg_H
