//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscdecaljigdlg.h,v 1.2 2022/02/19 18:22:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScDecalJigDlg_H
#define AuScDecalJigDlg_H

#include "audialog.h"

class QLineEdit;

class AuScDecalJigDlg : public AuDialog
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
	AuScDecalJigDlg(QWidget* parent=0);
	virtual ~AuScDecalJigDlg();

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

#endif // AuScDecalJigDlg_H
