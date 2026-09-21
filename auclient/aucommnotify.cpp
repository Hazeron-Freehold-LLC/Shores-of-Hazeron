//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommnotify.cpp,v 1.3 2022/02/19 18:10:32 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucommnotify.h"
//
#include "aucommpacket.h"
#include "auglobal.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>

AuCommNotify::AuCommNotify(const QString& avatarName, const AuCommPacket& msg, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Comm Notification"));
	setAuDlgIcon(QIcon(":data/Comm.png"));

	AuTime when = AuTime::Now();

	QLabel* lb = new QLabel(tr("<html>"
					"<p>"
					"This message mentions %5. "
					"</p>"
					"Channel: <b>%1</b> "
					"<br>"
					"When: <b>%2</b> "
					"<br>"
					"From: <b>%3</b> <i>%4</i> "
					"</html>")
				.arg(msg.ChannelName())
				.arg(when.UTC().toLocalTime().toString())
				.arg(msg.FromName())
				.arg(msg.FromId().StringId())
				.arg(avatarName),
				this);
	lb->setWordWrap(true);

	QLabel* lbico = new QLabel(this);
	if(!msg.Icon().isEmpty())
		lbico->setPixmap(QPixmap(msg.Icon()).scaled(32,32, Qt::KeepAspectRatio, Qt::SmoothTransformation));

	QLabel* lbico2 = new QLabel(this);
	if(!msg.Icon2().isEmpty())
		lbico2->setPixmap(QPixmap(msg.Icon2()).scaled(32,32, Qt::KeepAspectRatio, Qt::SmoothTransformation));

	QTextEdit* tetext = new QTextEdit(msg.Text(), this);
	tetext->setReadOnly(true);

	QHBoxLayout* icolay = new QHBoxLayout;
	icolay->addWidget(lbico);
	icolay->addWidget(lbico2);
	icolay->setMargin(0);
	icolay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lb);
	lay->addLayout(icolay);
	lay->addWidget(tetext);
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuCommNotify::~AuCommNotify()
{
}

void AuCommNotify::setVisible(bool visible)
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

void AuCommNotify::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/CommNotify/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/CommNotify/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/CommNotify/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/CommNotify/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuCommNotify::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/CommNotify/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/CommNotify/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/CommNotify/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/CommNotify/SizeY",sz.height());
}
