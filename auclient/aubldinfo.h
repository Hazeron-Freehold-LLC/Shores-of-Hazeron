//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldinfo.h,v 1.6 2022/02/19 17:52:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBldInfo_H
#define AuBldInfo_H

#include "auclientlib.h"
#include <QWidget>

class QLabel;

class AUCLIENT_DLL AuBldInfo : public QWidget
{
	Q_OBJECT;
public:
	AuBldInfo(bool atCapitol, QWidget* parent=0);
	virtual ~AuBldInfo();

protected:
	virtual void hideEvent(QHideEvent* event);
	virtual void showEvent(QShowEvent* event);
};

#endif // AuBldInfo_H
