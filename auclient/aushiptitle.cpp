//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aushiptitle.cpp,v 1.2 2022/02/19 18:16:49 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aushiptitle.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "daperson.h"
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AuShipTitle::AuShipTitle(QWidget* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":data/c_Patent.png"));
	setAuDlgTitle("Spacecraft Title of Ownership");

	QLabel* lblicon = new QLabel;
	lblicon->setPixmap(QPixmap(":data/c_Patent.png"));

	QString textConfirm = tr("<html>"
			"Choose media in your gear to store the title of ownership. "
			"Paper and blank disks can only store one title of ownership and they cannot be reused. "
			"</html>");
	QLabel* lbmedia = new QLabel(textConfirm, this);
	lbmedia->setWordWrap(true);

	m_GearMedia = new QComboBox(this);
	m_GearMedia->setToolTip(tr("<html>Shows media in your gear that can be used to store a title of ownership."
				"</html>"));
	connect(m_GearMedia, SIGNAL(currentIndexChanged(int)), SLOT(GearMediaChanged(int)));

	m_Ok = new QPushButton(tr("Ok"));
	m_Ok->setEnabled(false);
	connect(m_Ok, SIGNAL(clicked()), SLOT(OkPressed()));

	QHBoxLayout* nlay = new QHBoxLayout;
	nlay->addWidget(lblicon);
	nlay->addWidget(lbmedia);
	nlay->setMargin(0);
	nlay->setSpacing(2);

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(m_Ok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(nlay);
	lay->addWidget(m_GearMedia);
	lay->addLayout(blay);
	setAuDlgLayout(lay);
}

AuShipTitle::~AuShipTitle()
{
}

void AuShipTitle::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		ListMedia();
		RestoreGeometry();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuShipTitle::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/CopyShipTitle/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/CopyShipTitle/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/CopyShipTitle/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/CopyShipTitle/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuShipTitle::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/CopyShipTitle/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/CopyShipTitle/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/CopyShipTitle/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/CopyShipTitle/SizeY",sz.height());
}

void AuShipTitle::OkPressed()
{
	accept();
}

void AuShipTitle::ListMedia()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* view = pFrame->View();
	if(!view)
		return;

	m_GearMedia->blockSignals(true);

	DaPerson* sceneAtom;
	view->LockSceneAtom(&sceneAtom);
	if(sceneAtom)
	{
		// Add designs in my possession.
		const AuGear* inv = sceneAtom->LockGearForRead();
		AuGearPosList allGearPos;
		inv->ListAll(allGearPos);
		for(AuGearPosList::const_iterator gi = allGearPos.cbegin(); gi != allGearPos.cend(); gi++)
		{
			const AuItem* gitem = inv->ItemAt(*gi);
			if(gitem->CommodityId() == CID_BlankDisk)
			{
				m_GearMedia->addItem(QIcon(gitem->BitmapName()), gitem->ListDesc(), QVariant::fromValue(*gi));
				continue;
			}
			else if(gitem->CommodityId() == CID_Paper && gitem->Inscription().isEmpty())
			{
				m_GearMedia->addItem(QIcon(gitem->BitmapName()), gitem->ListDesc(), QVariant::fromValue(*gi));
				continue;
			}
		}
		sceneAtom->UnlockGear();
	}
	view->UnlockSceneAtom();

	m_GearMedia->setCurrentIndex(-1);
	m_GearMedia->blockSignals(false);
}

void AuShipTitle::GearMediaChanged(int index)
{
	m_Media = m_GearMedia->itemData(index).value<AuGearPos>();
	EnableControls();
}

void AuShipTitle::EnableControls()
{
	bool enableOk = true;

	if(!m_Media.IsValid())
		enableOk = false;

	m_Ok->setEnabled(enableOk);
}
