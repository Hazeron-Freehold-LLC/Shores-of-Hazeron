//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovself.h,v 1.6 2022/02/19 17:57:15 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AUGOVSELF_H
#define AUGOVSELF_H

#include "auclientlib.h"
#include <QWidget>

class AUCLIENT_DLL AuGovSelf : public QWidget
{
	Q_OBJECT;
public:
	AuGovSelf(const QString& name, QWidget* parent=0);

protected slots:
	void SetActiveTrans(int value);
	void SetInactiveTrans(int value);
};

#endif // AUGOVSELF_H
