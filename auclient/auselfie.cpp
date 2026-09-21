//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auselfie.cpp,v 1.4 2022/02/19 18:16:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auselfie.h"
//
#include "auapp.h"
#include "auglobal.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

AuSelfie::AuSelfie(const QString& selfieName, const QImage& selfieImg, QWidget* parent) : AuDialog(parent), m_Selfie(selfieImg)
{
	setAuDlgTitle(tr("Selfie of %1").arg(selfieName));
	setAuDlgIcon(QIcon(":data/it_Camera.png"));

	QLabel* lbselfie = new QLabel(this);
	lbselfie->setAlignment(Qt::AlignCenter);
	QPixmap pix = QPixmap::fromImage(m_Selfie);
	lbselfie->setPixmap(pix);

	QPushButton* pbok = new QPushButton(tr("Send Selfie to Server"));
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	connect(pbok, SIGNAL(clicked()), SLOT(SendSelfie()));
	pbok->setToolTip(tr("Sends the selfie picture to the server."));

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbselfie);
	lay->addWidget(pbok);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuSelfie::~AuSelfie()
{
}

void AuSelfie::setVisible(bool visible)
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

void AuSelfie::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/AuSelfie/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/AuSelfie/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/AuSelfie/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/AuSelfie/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuSelfie::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/AuSelfie/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/AuSelfie/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/AuSelfie/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/AuSelfie/SizeY",sz.height());
}

void AuSelfie::SendSelfie()
{
	auApp->PostSelfie(m_Selfie);
}

