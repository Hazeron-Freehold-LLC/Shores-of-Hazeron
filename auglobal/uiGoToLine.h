//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: uiGoToLine.h,v 1.4 2022/02/19 18:32:53 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

/********************************************************************************
** Form generated from reading ui file 'augotolinePl4492.ui'
**
** Created: Wed Dec 23 10:20:37 2009
**      by: Qt User Interface Compiler version 4.5.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef AUGOTOLINEPL4492_H
#define AUGOTOLINEPL4492_H

#include "augloballib.h"
#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>
#include <QWidget>

QT_BEGIN_NAMESPACE

class AUGLOBAL_DLL Ui_GoToLine
{
public:
	QGridLayout *gridLayout;
	QLabel *label;
	QLineEdit *leGoToLine;
	QSpacerItem *horizontalSpacer;
	QPushButton *pbGoToLine;
	QPushButton *pbClose;

	void setupUi(QWidget *GoToLine)
	{
		if (GoToLine->objectName().isEmpty())
			GoToLine->setObjectName(QString::fromUtf8("GoToLine"));
		GoToLine->setEnabled(true);
		GoToLine->resize(233, 61);
		GoToLine->setMaximumSize(QSize(16777215, 16777215));
		gridLayout = new QGridLayout(GoToLine);
		gridLayout->setMargin(6);
		gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
		label = new QLabel(GoToLine);
		label->setObjectName(QString::fromUtf8("label"));
		QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
		sizePolicy.setHorizontalStretch(0);
		sizePolicy.setVerticalStretch(0);
		sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
		label->setSizePolicy(sizePolicy);

		gridLayout->addWidget(label, 0, 0, 1, 1);

		leGoToLine = new QLineEdit(GoToLine);
		leGoToLine->setObjectName(QString::fromUtf8("leGoToLine"));

		gridLayout->addWidget(leGoToLine, 0, 1, 1, 3);

		horizontalSpacer = new QSpacerItem(61, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

		gridLayout->addItem(horizontalSpacer, 1, 0, 1, 2);

		pbGoToLine = new QPushButton(GoToLine);
		pbGoToLine->setObjectName(QString::fromUtf8("pbGoToLine"));

		gridLayout->addWidget(pbGoToLine, 1, 2, 1, 1);

		pbClose = new QPushButton(GoToLine);
		pbClose->setObjectName(QString::fromUtf8("pbClose"));

		gridLayout->addWidget(pbClose, 1, 3, 1, 1);


		retranslateUi(GoToLine);

		QMetaObject::connectSlotsByName(GoToLine);
	} // setupUi

	void retranslateUi(QWidget *GoToLine)
	{
		GoToLine->setWindowTitle(QApplication::translate("GoToLine", "Go To Line", 0, -1));
		label->setText(QApplication::translate("GoToLine", "Line number", 0, -1));
		pbGoToLine->setText(QApplication::translate("GoToLine", "Go To", 0, -1));
		pbClose->setText(QApplication::translate("GoToLine", "Cancel", 0, -1));
		Q_UNUSED(GoToLine);
	} // retranslateUi

};

namespace Ui {
	class GoToLine: public Ui_GoToLine {};
} // namespace Ui

QT_END_NAMESPACE

#endif // AUGOTOLINEPL4492_H

