//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscimportstldlg.h,v 1.2 2022/02/19 18:22:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScImportSTLDlg_H
#define AuScImportSTLDlg_H

#include "audialog.h"

class QCheckBox;
class QComboBox;
class QPushButton;

class AuScImportSTLDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QPushButton*						m_FileName;
	QComboBox*							m_UnitsOfMeasure;

public:
	AuScImportSTLDlg(QWidget* parent=0);
	virtual ~AuScImportSTLDlg();

	QString FileName() const;

	// Returns the scale factor needed to convert from incoming values to feet.
	double ScaleToFeet() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void ChooseFile();
	void DoImport();
};

#endif // AuScImportSTLDlg_H
