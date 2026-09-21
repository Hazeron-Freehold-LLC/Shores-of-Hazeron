//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: AuMissionNameUi.cpp,v 1.3 2022/02/19 18:16:23 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "AuMissionNameUi.h"
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

AuMissionNameUi::AuMissionNameUi(QWidget* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":/data/c_MissionDisk.png"));
	setAuDlgTitle("Mission Name");

	QLabel* lbl = new QLabel(tr("Please enter a name for the mission."));

	QLabel* lblicon = new QLabel;
	lblicon->setPixmap(QPixmap(":/data/c_MissionDisk.png"));

	m_NameEdit = new QLineEdit(this);
	m_NameEdit->setMaxLength(MaxNameLength());
	connect(m_NameEdit, SIGNAL(textChanged(const QString&)), SLOT(NameChanged(const QString&)));

	QString textConfirm = tr("<html>"
			"Choose media in your gear to store the mission. "
			"Paper and blank disks can only store one mission and they cannot be reused. "
			"</html>");
	QLabel* lbmedia = new QLabel(textConfirm, this);
	lbmedia->setWordWrap(true);

	m_GearMedia = new QComboBox(this);
	m_GearMedia->setToolTip(tr("<html>Shows media in your gear that can be used to store a mission."
				"</html>"));
	connect(m_GearMedia, SIGNAL(currentIndexChanged(int)), SLOT(GearMediaChanged(int)));

	m_Ok = new QPushButton(tr("Ok"));
	m_Ok->setEnabled(false);
	connect(m_Ok, SIGNAL(clicked()), SLOT(OkPressed()));

	QHBoxLayout* nlay = new QHBoxLayout;
	nlay->addWidget(lblicon);
	nlay->addWidget(m_NameEdit);
	nlay->setMargin(0);
	nlay->setSpacing(2);

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(m_Ok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbl);
	lay->addLayout(nlay);
	lay->addWidget(lbmedia);
	lay->addWidget(m_GearMedia);
	lay->addLayout(blay);
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuMissionNameUi::~AuMissionNameUi()
{
}

void AuMissionNameUi::setVisible(bool visible)
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

void AuMissionNameUi::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/StoreMission/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/StoreMission/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/StoreMission/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/StoreMission/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuMissionNameUi::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/StoreMission/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/StoreMission/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/StoreMission/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/StoreMission/SizeY",sz.height());
}

void AuMissionNameUi::OkPressed()
{
	m_Name = m_Name.trimmed();
	accept();
}

void AuMissionNameUi::NameChanged(const QString& name)
{
	m_Name = name;	// Don't trim yet.
	EnableControls();
}

void AuMissionNameUi::ListMedia()
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

void AuMissionNameUi::GearMediaChanged(int index)
{
	m_Media = m_GearMedia->itemData(index).value<AuGearPos>();
	EnableControls();
}

void AuMissionNameUi::EnableControls()
{
	bool enableOk = true;

	if(!IsNameValid(m_Name.trimmed()))
		enableOk = false;
	if(!m_Media.IsValid())
		enableOk = false;

	m_Ok->setEnabled(enableOk);
}
