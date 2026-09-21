//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscmakerotatingbeacondlg.h,v 1.3 2022/02/19 18:22:42 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScMakeRotatingBeaconDlg_H
#define AuScMakeRotatingBeaconDlg_H

#include "audialog.h"

class AuLightColorWidget;

class AuScMakeRotatingBeaconDlg : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	AuLightColorWidget*					m_Color;

public:
	AuScMakeRotatingBeaconDlg(QWidget* parent=0);
	virtual ~AuScMakeRotatingBeaconDlg();

	QColor Color() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();

protected slots:
	void DefaultColor();
	void DoOk();
};

#endif // AuScMakeRotatingBeaconDlg_H
