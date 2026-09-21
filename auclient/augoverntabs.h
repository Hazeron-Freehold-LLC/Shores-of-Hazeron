//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augoverntabs.h,v 1.6 2022/02/19 17:57:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGovernTabs_H
#define AuGovernTabs_H

#include "auclientlib.h"
#include "autabcolumn.h"

class AUCLIENT_DLL AuGovernTabs : public AuTabColumn
{
	Q_OBJECT;
public:
	AuGovernTabs(QWidget *parent=0);
	~AuGovernTabs();
};

#endif // AuGovernTabs_H
