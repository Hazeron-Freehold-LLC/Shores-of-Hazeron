//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auempirewait.h,v 1.7 2022/02/19 17:55:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuEmpireWait_H
#define AuEmpireWait_H

#include "auclientlib.h"
#include "audialog.h"

class QLabel;

class AUCLIENT_DLL AuEmpireWait : public AuDialog
{
private:
	Q_OBJECT;
	int				m_EmpiresKnown;
	QLabel*			m_lbProgress;

public:
	AuEmpireWait(QWidget* parent=0);
	virtual ~AuEmpireWait();

public slots:
	void TimerUpdate();
};

#endif // AuEmpireWait_H
