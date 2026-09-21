//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscimport3dsdlg.h,v 1.2 2022/02/19 18:22:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScImport3dsDlg_H
#define AuScImport3dsDlg_H

#include "audialog.h"

class QCheckBox;
class QComboBox;
class QPushButton;

class AuScImport3dsDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QPushButton*						m_FileName;
	QCheckBox*							m_TextureToo;
	QPushButton*						m_TextureFileName;
	QComboBox*							m_UnitsOfMeasure;

public:
	AuScImport3dsDlg(QWidget* parent=0);
	virtual ~AuScImport3dsDlg();

	QString FileName() const;

	// Returns the scale factor needed to convert from incoming values to feet.
	double ScaleToFeet() const;

	QString TextureFileName() const;

	bool TextureToo() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void ChooseFile();
	void ChooseTextureFile();
	void DoImport();
};

#endif // AuScImport3dsDlg_H
