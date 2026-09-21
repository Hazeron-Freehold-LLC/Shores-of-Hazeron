//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubuybd.cpp,v 1.4 2022/02/19 18:09:54 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubuybd.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "daperson.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>

AuBuyBd::AuBuyBd(quint32 designId, const QString& designName, const QString& terms, QWidget* parent) :
	AuDialog(parent)
{
	setAuDlgTitle(tr("Buy Building Blueprint"));
	setAuDlgIcon(QIcon(":data/Exchange.png"));

	QLabel* lbterms = new QLabel(terms, this);
	lbterms->setWordWrap(true);

	QString textConfirm = tr("<html>"
			"Choose media in your gear to store the blueprint of <b>%1 %2</b>."
			"</html>").arg(designId).arg(designName);
	QLabel* lbmedia = new QLabel(textConfirm, this);
	lbmedia->setWordWrap(true);

	m_GearMedia = new QComboBox(this);
	m_GearMedia->setToolTip(tr("<html>Shows media in your gear that can be used to store a blueprint."
				"</html>"));
	connect(m_GearMedia, SIGNAL(currentIndexChanged(int)), SLOT(GearMediaChanged(int)));

	m_Ok = new QPushButton(tr("Buy"), this);
	m_Ok->setAutoDefault(true);
	m_Ok->setDefault(true);
	m_Ok->setEnabled(false);
	m_Ok->setToolTip(tr("<html>"
				"Confirms purchase of a blueprint onto the selected media."
				"</html>"));
	connect(m_Ok, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(m_GearMedia, 1);
	blay->addWidget(m_Ok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbterms);
	lay->addWidget(lbmedia);
	lay->addLayout(blay);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuBuyBd::~AuBuyBd()
{
}

void AuBuyBd::setVisible(bool visible)
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

void AuBuyBd::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/BuyBd/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/BuyBd/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/BuyBd/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/BuyBd/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuBuyBd::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/BuyBd/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/BuyBd/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/BuyBd/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/BuyBd/SizeY",sz.height());
}

void AuBuyBd::ListMedia()
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
			const AuItemStorageMedia* media = dynamic_cast<const AuItemStorageMedia*>(gitem);
			if(media)
			{
				if(media->StorageSpaceAvailable())
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

void AuBuyBd::GearMediaChanged(int index)
{
	m_Media = m_GearMedia->itemData(index).value<AuGearPos>();
	m_MediaName = m_GearMedia->itemText(index);
	EnableControls();
}

void AuBuyBd::EnableControls()
{
	bool enableOk = true;

	if(!m_Media.IsValid())
		enableOk = false;

	m_Ok->setEnabled(enableOk);
}
