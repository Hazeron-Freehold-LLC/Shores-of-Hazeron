//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audebugmsgbits.cpp,v 1.8 2022/02/19 18:40:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audebugmsgbits.h"
//
#include "audef.h"
#include "auservermonapp.h"
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVariant>
#include <QGridLayout>

AuDebugMsgBits::AuDebugMsgBits(QWidget* parent) : QDialog(parent)
{
	setWindowTitle("Configure Log Messages");

	// Restore previous values.
	quint32 defaultMsgBits = LOG_Log|LOG_Debug|LOG_Warning|LOG_Error|LOG_LoadBalance|LOG_Admin;
	quint32 debugMsgBits = auServerMonApp->Settings().value("DebugMsgBits/Bits", defaultMsgBits).toUInt();

	QLabel* lbInfo = new QLabel(tr("Check boxes show previous dialog settings. "
				"They do not represent the current state of any server."), this);
	lbInfo->setWordWrap(true);

	QVBoxLayout* cblay = new QVBoxLayout;
	for(int bit=0; bit<32; bit++)
	{
		QString logMsgTypeName = LogMessageTypeName(bit);
		QCheckBox* cbBit = new QCheckBox(logMsgTypeName, this);
		if(debugMsgBits & (1<<bit))
			cbBit->setChecked(true);
		m_CheckBoxes.append(cbBit);
		cblay->addWidget(cbBit);
	}

	QPushButton* pbdefm = new QPushButton("Master Defaults");
	pbdefm->setToolTip("<html>Configures check boxes to match default settings on the master server.</html>");
	connect(pbdefm, SIGNAL(clicked()), SLOT(DefaultMaster()));

	QPushButton* pbdefg = new QPushButton("Manager Defaults");
	pbdefg->setToolTip("<html>Configures check boxes to match default settings on a manager server.</html>");
	connect(pbdefg, SIGNAL(clicked()), SLOT(DefaultManager()));

	QPushButton* pbdefw = new QPushButton("Worker Defaults");
	pbdefw->setToolTip("<html>Configures check boxes to match default settings on a worker server.</html>");
	connect(pbdefw, SIGNAL(clicked()), SLOT(DefaultWorker()));

	QPushButton* pbok = new QPushButton("Ok");
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip("<html>Configures log messages using these check box settings.</html>");
	connect(pbok, SIGNAL(clicked()), SLOT(Ok()));

	QVBoxLayout* dlay = new QVBoxLayout;
	dlay->addWidget(pbdefm);
	dlay->addWidget(pbdefg);
	dlay->addWidget(pbdefw);
	dlay->addStretch();
	dlay->addWidget(pbok);

	QHBoxLayout* elay = new QHBoxLayout;
	elay->addLayout(cblay);
	elay->addLayout(dlay);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbInfo);
	lay->addLayout(elay);
	setLayout(lay);
}

AuDebugMsgBits::~AuDebugMsgBits()
{
}

void AuDebugMsgBits::setVisible(bool visible)
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

void AuDebugMsgBits::RestoreGeometry()
{
	int posX = auServerMonApp->Settings().value("DebugMsgBits/PosX", -123).toInt();
	int posY = auServerMonApp->Settings().value("DebugMsgBits/PosY", -123).toInt();
	QSize szdef = sizeHint();
	int sizeX = auServerMonApp->Settings().value("DebugMsgBits/SizeX", szdef.width()).toInt();
	int sizeY = auServerMonApp->Settings().value("DebugMsgBits/SizeY", szdef.height()).toInt();
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuDebugMsgBits::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	auServerMonApp->Settings().setValue("DebugMsgBits/PosX", ps.x());
	auServerMonApp->Settings().setValue("DebugMsgBits/PosY", ps.y());
	auServerMonApp->Settings().setValue("DebugMsgBits/SizeX", sz.width());
	auServerMonApp->Settings().setValue("DebugMsgBits/SizeY", sz.height());
}

quint32 AuDebugMsgBits::DebugMsgBits() const
{
	quint32 debugMsgBits = 0;
	int bit = 0;
	for(QList<QCheckBox*>::const_iterator cbi = m_CheckBoxes.cbegin(); cbi != m_CheckBoxes.cend(); cbi++, bit++)
		if((*cbi)->isChecked())
			debugMsgBits |= 1<<bit;
	return debugMsgBits;
}

void AuDebugMsgBits::DefaultMaster()
{
	quint32 debugMsgBits = LOG_Log|LOG_Debug|LOG_Warning|LOG_Error|LOG_LoadBalance|LOG_Admin|LOG_Status;
	int bit = 0;
	for(QList<QCheckBox*>::const_iterator cbi = m_CheckBoxes.cbegin(); cbi != m_CheckBoxes.cend(); cbi++, bit++)
	{
		int bitval = 1<<bit;
		(*cbi)->setChecked(debugMsgBits&bitval);
	}
}

void AuDebugMsgBits::DefaultManager()
{
	quint32 debugMsgBits = LOG_Log|LOG_Debug|LOG_Warning|LOG_Error|LOG_LoadBalance|LOG_Admin|LOG_Status;
	int bit = 0;
	for(QList<QCheckBox*>::const_iterator cbi = m_CheckBoxes.cbegin(); cbi != m_CheckBoxes.cend(); cbi++, bit++)
	{
		int bitval = 1<<bit;
		(*cbi)->setChecked(debugMsgBits&bitval);
	}
}

void AuDebugMsgBits::DefaultWorker()
{
	quint32 debugMsgBits = LOG_Log|LOG_Debug|LOG_Warning|LOG_Error|LOG_LoadBalance|LOG_Admin;
	int bit = 0;
	for(QList<QCheckBox*>::const_iterator cbi = m_CheckBoxes.cbegin(); cbi != m_CheckBoxes.cend(); cbi++, bit++)
	{
		int bitval = 1<<bit;
		(*cbi)->setChecked(debugMsgBits&bitval);
	}
}

void AuDebugMsgBits::Ok()
{
	quint32 debugBits = DebugMsgBits();
	auServerMonApp->Settings().setValue("DebugMsgBits/Bits", debugBits);
	accept();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
