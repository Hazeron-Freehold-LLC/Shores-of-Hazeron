//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auglslerr.cpp,v 1.2 2022/02/19 18:29:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auglslerr.h"
//
#include <QIcon>
#include <QLabel>
#include <QTextEdit>
#include <QHBoxLayout>

AuGLSLErr::AuGLSLErr(const QString& title, const QString& errMsg, QWidget* parent) : QDialog(parent)
{
	setWindowTitle(title);
	setWindowIcon(QIcon(":data/Damage.png"));

	QLabel* lbico = new QLabel();
	lbico->setPixmap(QPixmap(":data/Damage.png").scaled(32,32));

	QTextEdit* teErr = new QTextEdit;
	teErr->setReadOnly(true);
	teErr->setText(errMsg);

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addWidget(lbico);
	lay->addWidget(teErr);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuGLSLErr::~AuGLSLErr()
{
}
