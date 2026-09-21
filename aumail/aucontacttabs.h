//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucontacttabs.h,v 1.4 2022/02/19 18:36:30 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AUCONTACTTABS_H
#define AUCONTACTTABS_H

#include "aumaillib.h"
#include <QTabBar>

class AUMAIL_DLL AuContactTabs : public QTabBar
{
	Q_OBJECT;
public:
	AuContactTabs(QWidget* parent=0);
};

#endif // AUCONTACTTABS_H
