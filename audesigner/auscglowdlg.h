//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscglowdlg.h,v 1.2 2022/02/19 18:22:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScGlowDlg_H
#define AuScGlowDlg_H

#include "audialog.h"

class AuColorButton;

class AuScGlowDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	AuColorButton*						m_Color;

public:
	AuScGlowDlg(QWidget* parent=0);
	virtual ~AuScGlowDlg();

	QColor Color() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DoGlow();
	void DoNoGlow();
};

#endif // AuScGlowDlg_H
