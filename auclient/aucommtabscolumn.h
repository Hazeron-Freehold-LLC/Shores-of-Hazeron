//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommtabscolumn.h,v 1.6 2022/02/19 17:55:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCommTabsColumn_H
#define AuCommTabsColumn_H

#include "auclientlib.h"
#include <autabcolumn.h>
#include <QTimer>

class AuComm;

class AUCLIENT_DLL AuCommTabsColumn : public AuTabColumn
{
	Q_OBJECT;
private:
	QTimer*								m_timer;

public:
	AuCommTabsColumn(AuComm* parent=0);
	~AuCommTabsColumn();

public slots:
	void Animate();
};

#endif // AuCommTabsColumn_H
