//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommopenchannel.cpp,v 1.2 2022/02/19 18:10:33 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucommopenchannel.h"
//
#include <dgdata.h>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AuCommOpenChannel::AuCommOpenChannel(const QString& defChannelName,
		const QStringList& reservedNames, QWidget* parent) :
	AuDialog(parent), m_ReservedNames(reservedNames)
{
	setAuDlgTitle("Open Channel");

	QLabel* lb = new QLabel(tr("Enter the name of the channel to open."));

	QLineEdit* le = new QLineEdit(this);
	le->setMaxLength(MaxNameLength());
	le->setObjectName("lechannel");
	le->setText(defChannelName);
	connect(le, SIGNAL(textChanged(const QString&)), SLOT(EnableControls()));

	QPushButton* pbok = new QPushButton(tr("Open Channel"));
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setObjectName("pbok");
	connect(pbok, SIGNAL(clicked()), SLOT(OpenChannel()));
	pbok->setToolTip(tr("Press this button to open the named channel."));

	QGridLayout* lay = new QGridLayout();
	int row = 0;	// Using this makes inserting and rearranging rows easier.
	lay->addWidget(lb, row, 0);
	row++;
	lay->addWidget(le, row, 0);
	row++;
	lay->addWidget(pbok, row, 0);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);

	EnableControls();
}

AuCommOpenChannel::~AuCommOpenChannel()
{
}

void AuCommOpenChannel::EnableControls()
{
	bool enableok = false;
	QLineEdit* le = findChild<QLineEdit*>("lechannel");
	if(le)
	{
		QString channelName = le->text().trimmed();
		if(!channelName.isEmpty() && !m_ReservedNames.contains(channelName) && IsNameValid(channelName))
			enableok = true;
	}

	QPushButton* pbok = findChild<QPushButton*>("pbok");
	if(pbok)
		pbok->setEnabled(enableok);
}

void AuCommOpenChannel::OpenChannel()
{
	//qDebug("AuCommOpenChannel::OpenChannel");
	QLineEdit* le = findChild<QLineEdit*>("lechannel");
	if(le)
	{
		// Get name so it can be retrieved after the widget is gone.
		QString channelName = le->text().trimmed();
		if(!channelName.isEmpty() && !m_ReservedNames.contains(channelName) && IsNameValid(channelName))
		{
			m_ChannelName = channelName;
			accept();
		}
	}
}
