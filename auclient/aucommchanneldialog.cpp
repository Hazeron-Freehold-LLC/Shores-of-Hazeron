//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommchanneldialog.cpp,v 1.4 2022/02/19 18:10:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucommchanneldialog.h"
//
#include "auapp.h"
#include "aucommchannel.h"
#include "auglobal.h"
#include "auframe.h"
#include <QHBoxLayout>
#include <QMessageBox>
#include <QToolButton>

AuCommChannelDialog::AuCommChannelDialog(QWidget* parent) : AuDialog(parent)
{
	QToolButton* closeButton = new QToolButton(this);
	closeButton->setIcon(QIcon(":data/CommClose.png"));
	closeButton->setAutoRaise(true);
	closeButton->setToolTip(tr("<html>"
				"<b>Close Channel</b>"
				"<p>"
				"Closes this window. "
				"The comm channel is closed. "
				"</p>"
				"</html>"));
	connect(closeButton, SIGNAL(clicked(bool)), SLOT(CloseChannel()));

	QToolButton* restoreButton = new QToolButton(this);
	restoreButton->setIcon(QIcon(":data/CommChannel.png"));
	restoreButton->setAutoRaise(true);
	restoreButton->setToolTip(tr("<html>"
				"<b>Return to Comm</b>"
				"<p>"
				"Closes this window. "
				"Restores the comm channel tab to the comm window. "
				"The channel remains open. "
				"</p>"
				"<p>"
				"The X button on the title bar of this window does the same thing. "
				"</p>"
				"</html>"));
	connect(restoreButton, SIGNAL(clicked(bool)), SLOT(RestoreChannel()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(closeButton);
	blay->addWidget(restoreButton);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(blay);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuCommChannelDialog::~AuCommChannelDialog()
{
}

void AuCommChannelDialog::setVisible(bool visible)
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

void AuCommChannelDialog::RestoreGeometry()
{
	if(!m_Channel)
		return;
	QString name(m_Channel->objectName());
	int posX = g_Au.Settings().readNumEntry(QString("Widget/Channel/%1/PosX").arg(name), -123);
	int posY = g_Au.Settings().readNumEntry(QString("Widget/Channel/%1/PosY").arg(name), -123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry(QString("Widget/Channel/%1/SizeX").arg(name), szdef.width());
	int sizeY = g_Au.Settings().readNumEntry(QString("Widget/Channel/%1/SizeY").arg(name), szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuCommChannelDialog::SaveGeometry()
{
	if(!m_Channel)
		return;
	QString name(m_Channel->objectName());
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry(QString("Widget/Channel/%1/PosX").arg(name), ps.x());
	g_Au.Settings().writeEntry(QString("Widget/Channel/%1/PosY").arg(name), ps.y());
	g_Au.Settings().writeEntry(QString("Widget/Channel/%1/SizeX").arg(name), sz.width());
	g_Au.Settings().writeEntry(QString("Widget/Channel/%1/SizeY").arg(name), sz.height());
}

void AuCommChannelDialog::SetChannelWidget(AuCommChannel* channel)
{
	m_Channel = channel;
	QString name(channel->objectName());
	setAuDlgTitle(name);
	setAuDlgIcon(AuCommStdChannel::ChannelIcon(name, channel->Scope()));
	QVBoxLayout* audlglay = (QVBoxLayout*)layout();			// This is the audlg layout.
	QVBoxLayout* lay = (QVBoxLayout*)audlglay->itemAt(1);	// This is my layout, set with setAuDlgLayout.
	lay->addWidget(channel);
}

void AuCommChannelDialog::keyPressEvent(QKeyEvent* event)
{
	if(event->key() != Qt::Key_Escape)
		QWidget::keyPressEvent(event);
}

void AuCommChannelDialog::closeEvent(QCloseEvent* e)
{
	Q_UNUSED(e);
	if(m_Channel)
		RestoreChannel();				// Window Xed? Must signal something. Most benign action is to restore the channel tab.
}

void AuCommChannelDialog::CloseChannel()
{
	SaveGeometry();
	emit CloseChannel(m_Channel->objectName());
	m_Channel->deleteLater();
	m_Channel = 0;						// Prevent close event from signaling again.
	accept();
}

void AuCommChannelDialog::RestoreChannel()
{
	SaveGeometry();
	emit RestoreChannel(m_Channel);
	m_Channel = 0;						// Prevent close event from signaling again.
	accept();
}
