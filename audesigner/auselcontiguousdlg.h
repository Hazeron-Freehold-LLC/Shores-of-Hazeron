//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auselcontiguousdlg.h,v 1.2 2022/02/19 18:22:52 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSelContiguousDlg_H
#define AuSelContiguousDlg_H

#include "audialog.h"

class QLineEdit;

class AuSelContiguousDlg : public AuDialog
{
	Q_OBJECT;

	// Widgets
	QLineEdit*							m_AngleEdit;

public:
	AuSelContiguousDlg(float angleTol, QWidget* parent=0);
	virtual ~AuSelContiguousDlg();

	float AngleTolerance() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuSelContiguousDlg_H
