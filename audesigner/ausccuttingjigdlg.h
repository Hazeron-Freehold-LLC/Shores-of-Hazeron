//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausccuttingjigdlg.h,v 1.2 2022/02/19 18:22:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScCuttingJigDlg_H
#define AuScCuttingJigDlg_H

#include "audialog.h"

class QLineEdit;

class AuScCuttingJigDlg : public AuDialog
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
	AuScCuttingJigDlg(QWidget* parent=0);
	virtual ~AuScCuttingJigDlg();

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

#endif // AuScCuttingJigDlg_H
