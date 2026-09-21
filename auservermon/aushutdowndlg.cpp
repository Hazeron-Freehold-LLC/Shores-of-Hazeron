//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aushutdowndlg.cpp,v 1.6 2022/02/19 18:40:49 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aushutdowndlg.h"
//
#include "auservermonapp.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVariant>
#include <QGridLayout>

AuShutdownDlg::AuShutdownDlg(QWidget* parent) : QDialog(parent)
{
	// Restore previous values.
	QString warnMsg = auServerMonApp->Settings().value("ShutdownDlg/Warn",
			"ATTENTION! Servers are shutting down.").toString();
	QString haltMsg = auServerMonApp->Settings().value("ShutdownDlg/Halt",
			"<><><><><> Servers are shutting down now. <><><><><>").toString();
	int secs = auServerMonApp->Settings().value("ShutdownDlg/Seconds", 300).toInt();

	QLabel* lbWarn = new QLabel("Warning Message", this);

	m_leWarn = new QLineEdit(warnMsg, this);
	m_leWarn->setMinimumWidth(500);
	m_leWarn->setToolTip("<html>Enter a message to be broadcast periodically to warn of the pending shutdown.</html>");

	QLabel* lbHalt = new QLabel("Halt Message", this);

	m_leHalt = new QLineEdit(haltMsg, this);
	m_leHalt->setMinimumWidth(500);
	m_leHalt->setToolTip("<html>Enter a final message to be broadcast upon shutdown. "
			"When the target server is not the master server, this is the only message that is broadcast.</html>");

	QLabel* lbSecs = new QLabel("Seconds Before Shutdown", this);

	m_leSecs = new QLineEdit(QVariant(secs).toString(), this);
	m_leSecs->setToolTip("<html>Enter number of seconds before shutdown. 0 results in immediate shutdown. "
			"When the target server is not the master server, this value is ignored and shutdown is immediate.</html>");

	QPushButton* pbok = new QPushButton("Shutdown Server");
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip("<html>Shuts down the selected server(s).</html>");
	connect(pbok, SIGNAL(clicked()), SLOT(Shutdown()));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbWarn, row,0);
	lay->addWidget(m_leWarn, row,1);
	row++;
	lay->addWidget(lbHalt, row,0);
	lay->addWidget(m_leHalt, row,1);
	row++;
	lay->addWidget(lbSecs, row,0);
	lay->addWidget(m_leSecs, row,1);
	row++;
	lay->addWidget(pbok, row,1);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuShutdownDlg::~AuShutdownDlg()
{
}

void AuShutdownDlg::setVisible(bool visible)
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

void AuShutdownDlg::RestoreGeometry()
{
	int posX = auServerMonApp->Settings().value("ShutdownDlg/PosX", -123).toInt();
	int posY = auServerMonApp->Settings().value("ShutdownDlg/PosY", -123).toInt();
	QSize szdef = sizeHint();
	int sizeX = auServerMonApp->Settings().value("ShutdownDlg/SizeX", szdef.width()).toInt();
	int sizeY = auServerMonApp->Settings().value("ShutdownDlg/SizeY", szdef.height()).toInt();
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuShutdownDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	auServerMonApp->Settings().setValue("ShutdownDlg/PosX", ps.x());
	auServerMonApp->Settings().setValue("ShutdownDlg/PosY", ps.y());
	auServerMonApp->Settings().setValue("ShutdownDlg/SizeX", sz.width());
	auServerMonApp->Settings().setValue("ShutdownDlg/SizeY", sz.height());
}

void AuShutdownDlg::Shutdown()
{
	m_WarnMsg = m_leWarn->text();

	m_HaltMsg = m_leHalt->text();

	m_ShutdownSecs = QVariant(m_leSecs->text()).toInt();
	if(m_ShutdownSecs < 0)
		m_ShutdownSecs = 0;

	auServerMonApp->Settings().setValue("ShutdownDlg/Warn", m_WarnMsg);
	auServerMonApp->Settings().setValue("ShutdownDlg/Halt", m_HaltMsg);
	auServerMonApp->Settings().setValue("ShutdownDlg/Seconds", m_ShutdownSecs);

	accept();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuShutdownHaltDlg::AuShutdownHaltDlg(QWidget* parent) : QDialog(parent)
{
	// Restore previous values.
	QString haltMsg = auServerMonApp->Settings().value("ShutdownHaltDlg/Halt",
			"@@@@@@@@@@ SERVER SHUTDOWN HALTED! @@@@@@@@@@").toString();

	QLabel* lbHalt = new QLabel("Halt Message", this);

	m_leHalt = new QLineEdit(haltMsg, this);
	m_leHalt->setMinimumWidth(500);
	m_leHalt->setToolTip("<html>Enter a message to be broadcast to announce the shutdown was halted.</html>");

	QPushButton* pbok = new QPushButton("Halt Master Server Shutdown");
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip("<html>Halts shutdown sequence on the selected master server(s). Has no effect on other servers.</html>");
	connect(pbok, SIGNAL(clicked()), SLOT(HaltShutdown()));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbHalt, row,0);
	lay->addWidget(m_leHalt, row,1);
	row++;
	lay->addWidget(pbok, row,1);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuShutdownHaltDlg::~AuShutdownHaltDlg()
{
}

void AuShutdownHaltDlg::setVisible(bool visible)
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

void AuShutdownHaltDlg::RestoreGeometry()
{
	int posX = auServerMonApp->Settings().value("ShutdownHaltDlg/PosX", -123).toInt();
	int posY = auServerMonApp->Settings().value("ShutdownHaltDlg/PosY", -123).toInt();
	QSize szdef = sizeHint();
	int sizeX = auServerMonApp->Settings().value("ShutdownHaltDlg/SizeX", szdef.width()).toInt();
	int sizeY = auServerMonApp->Settings().value("ShutdownHaltDlg/SizeY", szdef.height()).toInt();
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuShutdownHaltDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	auServerMonApp->Settings().setValue("ShutdownHaltDlg/PosX", ps.x());
	auServerMonApp->Settings().setValue("ShutdownHaltDlg/PosY", ps.y());
	auServerMonApp->Settings().setValue("ShutdownHaltDlg/SizeX", sz.width());
	auServerMonApp->Settings().setValue("ShutdownHaltDlg/SizeY", sz.height());
}

void AuShutdownHaltDlg::HaltShutdown()
{
	m_HaltMsg = m_leHalt->text();

	auServerMonApp->Settings().setValue("ShutdownHaltDlg/Halt", m_HaltMsg);

	accept();
}

