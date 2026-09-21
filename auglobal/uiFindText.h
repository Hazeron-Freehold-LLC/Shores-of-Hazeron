//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: uiFindText.h,v 1.4 2022/02/19 18:32:49 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

/********************************************************************************
** Form generated from reading ui file 'aufindtextPM4492.ui'
**
** Created: Wed Dec 23 10:21:06 2009
**      by: Qt User Interface Compiler version 4.5.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef AUFINDTEXTPM4492_H
#define AUFINDTEXTPM4492_H

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

class AUGLOBAL_DLL Ui_FindText
{
public:
	QGridLayout *gridLayout;
	QLabel *label;
	QLineEdit *leFind;
	QSpacerItem *horizontalSpacer;
	QPushButton *pbFindNext;
	QPushButton *pbClose;

	void setupUi(QWidget *FindText)
	{
		if (FindText->objectName().isEmpty())
			FindText->setObjectName(QString::fromUtf8("FindText"));
		FindText->resize(269, 67);
		gridLayout = new QGridLayout(FindText);
		gridLayout->setMargin(6);
		gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
		label = new QLabel(FindText);
		label->setObjectName(QString::fromUtf8("label"));
		QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
		sizePolicy.setHorizontalStretch(0);
		sizePolicy.setVerticalStretch(0);
		sizePolicy.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
		label->setSizePolicy(sizePolicy);

		gridLayout->addWidget(label, 0, 0, 1, 1);

		leFind = new QLineEdit(FindText);
		leFind->setObjectName(QString::fromUtf8("leFind"));

		gridLayout->addWidget(leFind, 0, 1, 1, 3);

		horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

		gridLayout->addItem(horizontalSpacer, 1, 0, 1, 2);

		pbFindNext = new QPushButton(FindText);
		pbFindNext->setObjectName(QString::fromUtf8("pbFindNext"));

		gridLayout->addWidget(pbFindNext, 1, 2, 1, 1);

		pbClose = new QPushButton(FindText);
		pbClose->setObjectName(QString::fromUtf8("pbClose"));

		gridLayout->addWidget(pbClose, 1, 3, 1, 1);


		retranslateUi(FindText);

		QMetaObject::connectSlotsByName(FindText);
	} // setupUi

	void retranslateUi(QWidget *FindText)
	{
		FindText->setWindowTitle(QApplication::translate("FindText", "Find", 0, -1));
		label->setText(QApplication::translate("FindText", "Find:", 0, -1));
		pbFindNext->setText(QApplication::translate("FindText", "Find Next", 0, -1));
		pbClose->setText(QApplication::translate("FindText", "Close", 0, -1));
		Q_UNUSED(FindText);
	} // retranslateUi

};

namespace Ui {
	class FindText: public Ui_FindText {};
} // namespace Ui

QT_END_NAMESPACE

#endif // AUFINDTEXTPM4492_H

