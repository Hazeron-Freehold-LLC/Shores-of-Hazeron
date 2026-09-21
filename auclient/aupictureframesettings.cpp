//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aupictureframesettings.cpp,v 1.3 2022/02/19 18:16:33 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aupictureframesettings.h"
//
#include "auglobal.h"
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

AuPictureFrameSettings::AuPictureFrameSettings(const QString& url, bool hideURL, bool rotatePicture, QWidget* parent) :
	AuDialog(parent)
{
	setAuDlgTitle(tr("Picture Frame Settings"));
	setAuDlgIcon(QIcon(":data/c_PictureFrame.png"));

	QLabel* lbURL = new QLabel(tr("URL of Picture"));

	m_URLEdit = new QLineEdit(url);

	m_HideURLCheck = new QCheckBox(tr("Hide URL"));
	m_HideURLCheck->setChecked(hideURL);

	m_RotateCheck = new QCheckBox(tr("Rotate Picture"));
	m_RotateCheck->setChecked(rotatePicture);

	QPushButton* pbok = new QPushButton("Ok");
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("Accepts the changes to the picture frame."));
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbURL, row,0);
	row++;
	lay->addWidget(m_URLEdit, row,0);
	row++;
	lay->addWidget(m_HideURLCheck, row,0);
	row++;
	lay->addWidget(m_RotateCheck, row,0);
	row++;
	lay->addWidget(pbok, row,0);
	row++;
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuPictureFrameSettings::~AuPictureFrameSettings()
{
}

void AuPictureFrameSettings::setVisible(bool visible)
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

void AuPictureFrameSettings::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/PictureSettings/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/PictureSettings/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/PictureSettings/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/PictureSettings/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuPictureFrameSettings::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/PictureSettings/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/PictureSettings/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/PictureSettings/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/PictureSettings/SizeY",sz.height());
}

QString AuPictureFrameSettings::PictureURL() const
{
	return m_URLEdit->text();
}

bool AuPictureFrameSettings::HideURL() const
{
	return m_HideURLCheck->isChecked();
}

bool AuPictureFrameSettings::RotatePicture() const
{
	return m_RotateCheck->isChecked();
}
