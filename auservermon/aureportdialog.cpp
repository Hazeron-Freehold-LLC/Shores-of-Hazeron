//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aureportdialog.cpp,v 1.10 2022/02/19 18:40:47 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aureportdialog.h"
//
#include "auservermonapp.h"
#include <QIcon>
#include <QTextBrowser>
#include <QVBoxLayout>

AuReportDialog::AuReportDialog(const QString& text, QWidget* parent) : QDialog(parent)
{
	setWindowIcon(QIcon(":data/ServerMon.png"));
	setWindowTitle(auServerMonApp->Name());

	QTextBrowser* tb = new QTextBrowser;
	tb->setHtml(text);

	QVBoxLayout* mainLay = new QVBoxLayout;
	mainLay->addWidget(tb);
	mainLay->setSpacing(2);
	mainLay->setMargin(4);
	setLayout(mainLay);
}

AuReportDialog::AuReportDialog(const QStringList& textLines, QWidget* parent) : QDialog(parent)
{
	setWindowIcon(QIcon(":data/ServerMon.png"));
	setWindowTitle(auServerMonApp->Name());

	QString text = "<html>";
	for(QStringList::const_iterator li = textLines.cbegin(); li != textLines.cend(); li++)
		text += *li + "<br>";
	text += "</html>";

	QTextBrowser* tb = new QTextBrowser;
	tb->setHtml(text);

	QVBoxLayout* mainLay = new QVBoxLayout;
	mainLay->addWidget(tb);
	mainLay->setSpacing(2);
	mainLay->setMargin(4);
	setLayout(mainLay);
}

AuReportDialog::~AuReportDialog()
{
}

void AuReportDialog::RestoreGeometry()
{
	int posX = auServerMonApp->Settings().value("ReportDialog/PosX", -123).toInt();
	int posY = auServerMonApp->Settings().value("ReportDialog/PosY", -123).toInt();
	QSize szdef = sizeHint();
	int sizeX = auServerMonApp->Settings().value("ReportDialog/SizeX", szdef.width()).toInt();
	int sizeY = auServerMonApp->Settings().value("ReportDialog/SizeY", szdef.height()).toInt();
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuReportDialog::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	auServerMonApp->Settings().setValue("ReportDialog/PosX", ps.x());
	auServerMonApp->Settings().setValue("ReportDialog/PosY", ps.y());
	auServerMonApp->Settings().setValue("ReportDialog/SizeX", sz.width());
	auServerMonApp->Settings().setValue("ReportDialog/SizeY", sz.height());
}

void AuReportDialog::setVisible(bool visible)
{
	if(visible)
	{
		QDialog::setVisible(visible);
		RestoreGeometry();
	}
	else
	{
		SaveGeometry();
		QDialog::setVisible(visible);
	}
}

