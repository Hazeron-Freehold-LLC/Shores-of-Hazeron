//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augotoline.h,v 1.4 2022/02/19 18:30:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AUGOTOLINE_H
#define AUGOTOLINE_H

#include "augloballib.h"
#include <QDialog>
#include "uiGoToLine.h"

class AUGLOBAL_DLL AuGoToLine : public QDialog, public Ui::GoToLine
{
	Q_OBJECT

public:
	AuGoToLine(QWidget *parent = 0, int iLineCount = 1);
	~AuGoToLine();
};

#endif //AUGOTOLINE_H

