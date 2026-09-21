//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubeamup.h,v 1.6 2022/02/19 17:50:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBeamUp_H
#define AuBeamUp_H

#include "auclientlib.h"
#include <audialog.h>

class QSpinBox;

class AUCLIENT_DLL AuBeamUp : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QSpinBox*							m_BeamCountSpin;		// Number to transport.

public:
	AuBeamUp(QWidget* parent=0);
	virtual ~AuBeamUp();

	// Returns the value in the QSpinBox.
	unsigned int BeamCount() const;

protected:
	virtual void hideEvent(QHideEvent* event);
	virtual void showEvent(QShowEvent* event);
};

#endif // AuBeamUp_H
