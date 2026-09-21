//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscmakenavlightdlg.h,v 1.3 2022/02/19 18:22:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScMakeNavLightDlg_H
#define AuScMakeNavLightDlg_H

#include "audialog.h"

class AuLightColorWidget;

class AuScMakeNavLightDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	AuLightColorWidget*					m_Color;

public:
	AuScMakeNavLightDlg(QWidget* parent=0);
	virtual ~AuScMakeNavLightDlg();

	QColor Color() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DoOk();
};

#endif // AuScMakeNavLightDlg_H
