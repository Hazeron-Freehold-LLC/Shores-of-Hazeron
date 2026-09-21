//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auabout.h,v 1.6 2022/02/19 17:48:52 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuAbout_H
#define AuAbout_H

#include "auclientlib.h"
#include "audialog.h"

class AUCLIENT_DLL AuAbout : public AuDialog
{
	Q_OBJECT;
public:
	AuAbout(QWidget* parent=0);
	virtual ~AuAbout();

	virtual QSize sizeHint() const;

protected slots:
	void AboutQt();
};

#endif // AuAbout_H
