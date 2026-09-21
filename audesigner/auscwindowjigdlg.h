//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscwindowjigdlg.h,v 1.2 2022/02/19 18:22:52 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScWindowJigDlg_H
#define AuScWindowJigDlg_H

#include "audialog.h"

class QLineEdit;

class AuScWindowJigDlg : public AuDialog
{
	Q_OBJECT;
public:
	enum MODE
	{
		HeightEntered,
		PickHeight,
		PickVector
	};

private:
	MODE								m_Mode;					// 0=height entered, 1=pick height, 2=pick vector

	// Widgets
	QLineEdit*							m_HeightEdit;

public:
	AuScWindowJigDlg(QWidget* parent=0);
	virtual ~AuScWindowJigDlg();

	double Height() const;

	// Valid on accept.
	MODE Mode() const
	{
		return m_Mode;
	}

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DoHeightEntered();
	void DoPickHeight();
	void DoPickVector();
};

#endif // AuScWindowJigDlg_H
