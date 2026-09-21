//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: AuMissionDiskUi.cpp,v 1.4 2022/02/19 18:16:22 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "AuMissionDiskUi.h"
//
#include "auapp.h"
#include <auglobal.h>
#include "auframe.h"
#include "auview.h"
#include <daperson.h>
#include <QComboBox>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QPushButton>

AuMissionDiskUi::AuMissionDiskUi(bool isNew, bool listShipMission, QWidget* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":/data/c_MissionDisk.png"));
	setAuDlgTitle("Load Mission");

	QLabel* lbl;
	if(listShipMission)
		lbl = new QLabel(tr("Select a mission to load from gear or your ship.<br>"
					"The mission will be sent to the recipient."));
	else if(isNew)
		lbl = new QLabel(tr("Select a mission disk to load.<br>"
					"The mission will replace your ship's mission.<br>"
					"The new mission will be inactive upon loading."));
	else
		lbl = new QLabel(tr("Select a mission disk to load.<br>"
					"The mission will be added to the end of your ship's mission."));

	m_MissionDisks = new QComboBox;
	{
		// List the mission disks in the scene atom's possession.
		AuView* view = auApp->Frame()->View();
		DaPerson* person;
		view->LockSceneAtom(&person);
		if(person)
		{
			if(listShipMission)
			{
				// Add an entry enabling selection of the ship's mission.
				const DaVessel* ship = person->FindParentVesselInEnv();
				if(ship && !ship->MissionIsEmpty() && ship->IsCrewOrTroop(person->Id()))
				{
					// Current ship has a mission and the person is a crew member.
					QVariant var;
					var.setValue(ship->LockMission());
					ship->UnlockMission();
					m_MissionDisks->addItem(QIcon(":data/GovSpacecraft.png"), ship->Name(), var);
				}
			}

			const AuGear* inv = person->LockGearForRead();
			AuGearPosList allGearPos;
			inv->ListAll(allGearPos);
			for(AuGearPosList::const_iterator gi = allGearPos.cbegin(); gi != allGearPos.cend(); gi++)
			{
				const AuItem* gitem = inv->ItemAt(*gi);
				const AuItemMission* itemm = dynamic_cast<const AuItemMission*>(gitem);
				if(itemm)
				{
					// Add the mission disk to the combo box.
					// The mission data is attached as a variant value.
					AuMission mission = itemm->Mission();
					QString missionName = itemm->MissionName();
					QVariant var;
					var.setValue(mission);
					m_MissionDisks->addItem(QIcon(":data/c_MissionDisk.png"), missionName, var);
					continue;
				}

				const AuItemStorageMedia* media = dynamic_cast<const AuItemStorageMedia*>(gitem);
				if(media)
				{
					for(AuMissionList::const_iterator mi = media->Missions().cbegin(); mi != media->Missions().cend(); mi++)
					{
						// The mission data is attached as a variant value.
						QVariant var;
						var.setValue(*mi);
						m_MissionDisks->addItem(QIcon(":data/c_MissionDisk.png"), mi.key(), var);
					}
					continue;
				}
			}
			person->UnlockGear();
		}
		view->UnlockSceneAtom();
	}
	if(listShipMission)
		m_MissionDisks->setToolTip(tr("<html>Missions in your gear and the mission of the ship you are aboard.</html>"));
	else
		m_MissionDisks->setToolTip(tr("<html>Missions in your gear.</html>"));
	connect(m_MissionDisks, SIGNAL(currentIndexChanged(int)), SLOT(MissionChanged(int)));

	m_Ok = new QPushButton(tr("Ok"));
	m_Ok->setEnabled((m_MissionDisks->currentIndex() >= 0) ? true:false);
	connect(m_Ok, SIGNAL(clicked()), SLOT(OkPressed()));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbl, row++,0);
	lay->addWidget(m_MissionDisks, row++,0);
	lay->addWidget(m_Ok, row++,0);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuMissionDiskUi::~AuMissionDiskUi()
{
}

void AuMissionDiskUi::OkPressed()
{
	int curIdx = m_MissionDisks->currentIndex();
	if(curIdx >= 0)
	{
		m_Mission = m_MissionDisks->itemData(curIdx).value<AuMission>();
		m_MissionName = m_MissionDisks->itemText(curIdx);
		accept();
	}
}

void AuMissionDiskUi::MissionChanged(int index)
{
	m_Ok->setEnabled((index < 0)?false:true);
}
