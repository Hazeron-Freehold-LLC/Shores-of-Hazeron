//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augotoline.cpp,v 1.1 2022/02/19 18:33:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augotoline.h"
//

AuGoToLine::AuGoToLine(QWidget *parent, int iLineCount) : QDialog(parent)
{
	setupUi(this);
	label->setText(QString("Line number (1-%1)").arg(iLineCount));

	connect(pbGoToLine, SIGNAL(clicked()), this, SLOT(accept()));
	connect(pbClose, SIGNAL(clicked()), this, SLOT(reject()));
}

AuGoToLine::~AuGoToLine()
{

}
