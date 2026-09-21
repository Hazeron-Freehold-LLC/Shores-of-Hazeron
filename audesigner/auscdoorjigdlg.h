//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscdoorjigdlg.h,v 1.2 2022/02/19 18:22:33 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScDoorJigDlg_H
#define AuScDoorJigDlg_H

#include "audialog.h"

class QLineEdit;

class AuScDoorJigDlg : public AuDialog
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
	AuScDoorJigDlg(QWidget* parent=0);
	virtual ~AuScDoorJigDlg();

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

#endif // AuScDoorJigDlg_H
