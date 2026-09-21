//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auservicemisc.h,v 1.7 2022/02/19 18:00:46 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuServiceMisc_H
#define AuServiceMisc_H

#include "auclientlib.h"
#include <QWidget>

class AuServiceDialog;

class AUCLIENT_DLL AuServiceMisc : public QWidget
{
	Q_OBJECT;
public:
	AuServiceMisc(AuServiceDialog* parent=0);

protected slots:
	void RequestExterminator();
	void RequestFuel();
	void RequestHouseKeeping();
};

#endif // AuServiceMisc_H
