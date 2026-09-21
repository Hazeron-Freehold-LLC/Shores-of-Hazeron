//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausetoverlay.h,v 1.2 2022/02/19 17:47:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSetOverlay_H
#define AuSetOverlay_H

#include <QWidget>

class AuSetOverlay : public QWidget
{
	Q_OBJECT;
public:
	AuSetOverlay(QWidget *parent=0);
	~AuSetOverlay();

protected slots:
	void ViewButtonCheckChanged(int checkState, int vb);
};

#endif // AuSetOverlay_H
