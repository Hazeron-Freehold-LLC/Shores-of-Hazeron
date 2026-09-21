//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audiplomaticmessage.cpp,v 1.2 2022/02/19 18:10:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audiplomaticmessage.h"
//
#include "aucommpacket.h"
#include "auglobal.h"
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AuDiplomaticMessage::AuDiplomaticMessage(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Diplomatic Message"));
	setAuDlgIcon(QIcon(":data/CommDiplomacy.png"));

	QLabel* lbmsg = new QLabel(tr("<html>Enter a diplomatic message to be forwarded to the city's chain of command via mail.<br>"
				"%1 Message length is limited to %2 characters.<br>"
				"%1 All cities that receive this message will forward it to their chain of command. "
				"Use range or a channel to limit which cities receive the message, to reduce spam.<br>"
				"%1 Each city can forward only one diplomatic message per five minutes.<br>"
				"%1 Enemy cities forward diplomatic messages even though they do not respond to you.<br>"
				"%1 The courier will mail you a delivery receipt when your diplomatic message is delivered to someone."
				"</html>").arg(g_Au.MiddleDotSymbol()).arg(AuCommDiplomaticMessage::MaxChatText()), this);
	lbmsg->setWordWrap(true);

	m_MessageEdit = new QLineEdit(this);
	m_MessageEdit->setMaxLength(AuCommDiplomaticMessage::MaxChatText());

	QPushButton* pbok = new QPushButton(QIcon(":data/CommDiplomacy.png"), tr("Send Diplomatic Message"));
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));
	pbok->setToolTip(tr("Sends the diplomatic message."));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbmsg, row,0, 1,2);
	row++;
	lay->addWidget(m_MessageEdit, row,0, 1,2);
	row++;
	lay->addWidget(pbok, row,1);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuDiplomaticMessage::~AuDiplomaticMessage()
{
}

void AuDiplomaticMessage::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuDiplomaticMessage::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/AuDiplomaticMessage/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/AuDiplomaticMessage/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/AuDiplomaticMessage/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/AuDiplomaticMessage/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuDiplomaticMessage::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/AuDiplomaticMessage/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/AuDiplomaticMessage/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/AuDiplomaticMessage/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/AuDiplomaticMessage/SizeY",sz.height());
}

QString AuDiplomaticMessage::MessageText() const
{
	return m_MessageEdit->text().trimmed();
}
