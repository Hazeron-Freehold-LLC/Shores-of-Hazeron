//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auagentteleport.cpp,v 1.14 2022/02/19 18:04:49 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auagentteleport.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auempirewait.h"
#include "auframe.h"
#include "auglobal.h"
#include "augotogalaxyloc.h"
#include "auview.h"
#include "daatomlist.h"
#include "dagalaxy.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "davehicle.h"
#include "davessel.h"
#include "dgdata.h"
#include <QComboBox>
#include <QGridLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QToolButton>

AuAgentTeleport::AuAgentTeleport(QWidget* parent) : QWidget(parent)
{
	m_DataValid = false;
	m_IsManualGoTo = false;	// true when user enters an id to goto

	const DgData& gd = g_Au.LockDataForRead();
	bool isProgrammer = gd.ClientPlayerIsProgrammer();
	bool isArchitect = gd.ClientPlayerIsArchitect();
	g_Au.UnlockData();

	QPushButton* pbrefreshempires = new QPushButton("Request Full Empires List");
	connect(pbrefreshempires, SIGNAL(clicked()), SLOT(RefreshEmpires()));
	pbrefreshempires->setToolTip("<html>Requests the server to send you the complete list of empires and avatars. "
			"This may take a minute or two.</html>");

	QLabel* lbacct = new QLabel("Account");

	QLabel* lbchars = new QLabel("Avatars");

	QLabel* lbcities = new QLabel("Cities");

	m_Account = new QComboBox;
	connect(m_Account, SIGNAL(activated(int)), SLOT(AccountSelected(int)));

	m_Avatars = new QComboBox;
	connect(m_Avatars, SIGNAL(activated(int)), SLOT(AvatarSelected(int)));

	m_Cities = new QComboBox;
	connect(m_Cities, SIGNAL(activated(int)), SLOT(CitySelected(int)));

	QLabel* lbscene;
	QToolButton* refreshScene;
	if(isProgrammer)
	{
		lbscene = new QLabel("Scene Atoms");
		m_SceneAtoms = new QComboBox;
		connect(m_SceneAtoms, SIGNAL(activated(int)), SLOT(SceneAtomSelected(int)));

		refreshScene = new QToolButton(this);
		refreshScene->setIcon(QIcon(":data/SensorScan.png"));
		refreshScene->setToolTip(tr("<b>Refresh Scene Atoms</b><br>"
					"Refreshes scene atoms list based on current scene."));
		refreshScene->setAutoRaise(true);
		connect(refreshScene, SIGNAL(clicked(bool)), SLOT(RefreshSceneAtoms()));
	}
	else
	{
		lbscene = 0;
		refreshScene = 0;
		m_SceneAtoms = 0;
	}

	QLabel* lbatom = 0;
	leatom = 0;
	QToolButton* clearId = 0;
	if(isProgrammer)
	{
		lbatom = new QLabel("Atom Id");

		leatom = new QLineEdit;
		leatom->setObjectName("leatom");
		connect(leatom, SIGNAL(textChanged(const QString&)), SLOT(IdChanged(const QString&)));

		clearId = new QToolButton(this);
		clearId->setIcon(QIcon(":data/Sweep.png"));
		clearId->setToolTip(tr("<b>Clear Atom Id</b><br>"
					"Clears text that has been entered into the atom id box."));
		clearId->setAutoRaise(true);
		connect(clearId, SIGNAL(clicked(bool)), SLOT(ClearAtomId()));
	}

	m_GoTo = new QPushButton("Go to Destination...");
	m_GoTo->setToolTip(tr("<html>Teleport to the last selected avatar or city.<br>"
			"%1 Does not bring the mount, vehicle or spacecraft you are aboard along with you.<br>"
			"%1 You arrive invisible."
			"</html>").arg(g_Au.MiddleDotSymbol()));
	connect(m_GoTo, SIGNAL(clicked()), SLOT(GoToDestination()));

	m_GoToWithRide = new QPushButton("Go to Destination With My Ride...");
	m_GoToWithRide->setToolTip("<html>Teleport to the last selected avatar or city. "
			"Brings the mount, vehicle or spacecraft you are aboard along with you.</html>");
	connect(m_GoToWithRide, SIGNAL(clicked()), SLOT(GoToDestinationWithRide()));

	m_Summon = new QPushButton("Summon Avatar");
	m_Summon->setToolTip("<html>Teleport the selected avatar to your current location.<br>"
			"%1 Does not bring the mount, vehicle or spacecraft they are aboard along with them.<br>"
			"%1 Agent will arrive invisible, regardless of the state of their agent powers."
			"</html>");
	connect(m_Summon, SIGNAL(clicked()), SLOT(SummonPerson()));

	m_SummonWithRide = new QPushButton("Summon Avatar With Their Ride");
	m_SummonWithRide->setToolTip("<html>Teleport the selected avatar to your current location. "
			"Brings the mount, vehicle or spacecraft they are aboard along with them.</html>");
	connect(m_SummonWithRide, SIGNAL(clicked()), SLOT(SummonPersonWithRide()));

	// This isn't really useful any more. It is easier done using the server manager program.
	if(isArchitect && true)
	{
		m_LimboRescue = new QPushButton("Limbo Rescue...");
		m_LimboRescue->setToolTip(tr("<html>Teleport to the selected avatar.<br>"
				"%1 Attempts to rescue them from limbo if they can't be found after 3 minutes.<br>"
				"%1 Does not bring the mount, vehicle or spacecraft you are aboard along with you.</html>")
			.arg(g_Au.MiddleDotSymbol()));
		connect(m_LimboRescue, SIGNAL(clicked()), SLOT(LimboRescue()));
	}
	else
		m_LimboRescue = 0;

	m_GoToLongLat = new QPushButton("Go to Longitude/Latitude...");
	m_GoToLongLat->setToolTip(tr("<html>Teleport to a specific longitude/latitude on the current world.<br>"
			"Does not bring the mount, vehicle or spacecraft you are aboard along with you.</html>"));
	connect(m_GoToLongLat, SIGNAL(clicked()), SLOT(GoToLongLat()));

	m_GoToLongLatWithRide = new QPushButton("Go to Longitude/Latitude With My Ride...");
	m_GoToLongLatWithRide->setToolTip(tr("<html>Teleport to a specific longitude/latitude on the current world. "
			"Brings the mount, vehicle or spacecraft you are aboard along with you.</html>"));
	connect(m_GoToLongLatWithRide, SIGNAL(clicked()), SLOT(GoToLongLatWithRide()));

	m_GoToGalaxy = new QPushButton("Go to Galaxy Location...");
	m_GoToGalaxy->setToolTip(tr("<html>Teleport to a specific coordinate in a galaxy.<br>"
			"Does not bring the mount, vehicle or spacecraft you are aboard along with you.</html>"));
	connect(m_GoToGalaxy, SIGNAL(clicked()), SLOT(GoToGalaxy()));

	m_GoToGalaxyWithRide = new QPushButton("Go to Galaxy Location With My Ride...");
	m_GoToGalaxyWithRide->setToolTip(tr("<html>Teleport to a specific coordinate in a galaxy. "
			"Brings the mount, vehicle or spacecraft you are aboard along with you.</html>"));
	connect(m_GoToGalaxyWithRide, SIGNAL(clicked()), SLOT(GoToGalaxyWithRide()));

	// Layout of buttons.
	QGridLayout* laygo = new QGridLayout;
	int row=0;
	laygo->addWidget(m_GoTo, row,0);
	laygo->addWidget(m_GoToWithRide, row,1);
	row++;
	laygo->addWidget(m_Summon, row,0);
	laygo->addWidget(m_SummonWithRide, row,1);
	row++;
	laygo->addWidget(m_GoToLongLat, row,0);
	laygo->addWidget(m_GoToLongLatWithRide, row,1);
	row++;
	laygo->addWidget(m_GoToGalaxy, row,0);
	laygo->addWidget(m_GoToGalaxyWithRide, row,1);
	row++;
	if(m_LimboRescue)
	{
		laygo->addWidget(m_LimboRescue, row,0);
		row++;
	}

	QHBoxLayout* atlay;
	if(clearId && leatom)
	{
		atlay = new QHBoxLayout;
		atlay->addWidget(clearId);
		atlay->addWidget(leatom);
	}
	else
		atlay = 0;

	QHBoxLayout* sclay;
	if(refreshScene && m_SceneAtoms)
	{
		sclay = new QHBoxLayout;
		sclay->addWidget(refreshScene);
		sclay->addWidget(m_SceneAtoms);
	}
	else
		sclay = 0;

	QHBoxLayout* relay;
	relay = new QHBoxLayout;
	relay->addStretch();
	relay->addWidget(pbrefreshempires);

	// Final layout
	QGridLayout* lay = new QGridLayout;
	row=0;
	lay->addLayout(relay, row,1);
	row++;
	lay->addWidget(lbacct, row,0);
	lay->addWidget(m_Account, row,1);
	row++;
	lay->addWidget(lbchars, row,0);
	lay->addWidget(m_Avatars, row,1);
	row++;
	lay->addWidget(lbcities, row,0);
	lay->addWidget(m_Cities, row,1);
	row++;

	if(lbscene && sclay)
	{
		lay->addWidget(lbscene, row,0);
		lay->addLayout(sclay, row,1);
		row++;
	}

	if(lbatom && atlay)
	{
		lay->addWidget(lbatom, row,0);
		lay->addLayout(atlay, row,1);
		row++;
	}

	lay->addLayout(laygo, row,0, 1,2);
	row++;

	lay->setRowStretch(row,1);
	row++;

	setLayout(lay);
}

AuAgentTeleport::~AuAgentTeleport()
{
}

void AuAgentTeleport::LimboRescue()
{
	if(m_GoToId.IsNull())
		return;
	QMessageBox mb(g_Au.ApplicationName(),
			tr("This action will teleport you to the destination atom.\n\n"
				"Limbo rescue locates the destination person; "
				"it then checks to see if they are in a vehicle; "
				"it then checks to see if the person or vehicle is in a spacecraft; "
				"then it agent teleports the spacecraft, vehicle, or person to a city. "
				"The city chosen is their capital city, a randomly selected city in their empire, or the capital "
				"city of a randomly selected empire, in that order.\n\n"
				"The agent teleport sends you to them when they are found, either right away or after the limbo rescue "
				"succeeds.\n\n"
				"After 3 minutes, if the destination atom is not found, the agent teleport is redirected to the capital "
				"city of a randomly selected empire. "
				"The same happens to the atom rescued from limbo if the chosen city isn't found in 3 minutes."),
		QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
	if(auApp->Frame()->ExecMessageBox(mb) != QMessageBox::Ok)
		return;
	GoToDestination(m_GoToId, false, true);
	emit AgentActionComplete();
}

void AuAgentTeleport::GoToLongLat()
{
	QString locDef = g_Au.Settings().readEntry("Widget/Admin/TeleportLongLat", "");

	bool ok;
	QString slonglat = QInputDialog::getText(this, tr("Longitude/Latitude Coordinates"),
			tr("<html>Enter the destination longitude/latitude coordinates on the current world.</html>"),
			QLineEdit::Normal, locDef, &ok);
	if(!ok)
		return;
	AuGlobeLLF ll;
	if(!ll.ParseLongitudeLatitude(slonglat))
		return;

	g_Au.Settings().writeEntry("Widget/Admin/TeleportLongLat", slonglat);

	auApp->AgentGoToLongLat(ll, false);

	emit AgentActionComplete();
}

void AuAgentTeleport::GoToLongLatWithRide()
{
	QString locDef = g_Au.Settings().readEntry("Widget/Admin/TeleportLongLat", "");

	bool ok;
	QString slonglat = QInputDialog::getText(this, tr("Longitude/Latitude Coordinates"),
			tr("<html>Enter the destination longitude/latitude coordinates on the current world.</html>"),
			QLineEdit::Normal, locDef, &ok);
	if(!ok)
		return;
	AuGlobeLLF ll;
	if(!ll.ParseLongitudeLatitude(slonglat))
		return;

	g_Au.Settings().writeEntry("Widget/Admin/TeleportLongLat", slonglat);

	auApp->AgentGoToLongLat(ll, true);

	emit AgentActionComplete();
}

void AuAgentTeleport::GoToDestination()
{
	if(m_GoToId.IsNull())
		return;
	QMessageBox mb(g_Au.ApplicationName(),
			tr("This action will teleport you to the destination atom.\n\n"
				"After 3 minutes, if the destination atom is not found, the agent teleport is redirected to the capital "
				"city of a randomly selected empire."),
		QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
	if(auApp->Frame()->ExecMessageBox(mb) != QMessageBox::Ok)
		return;
	GoToDestination(m_GoToId, false, false);
	emit AgentActionComplete();
}

void AuAgentTeleport::GoToDestinationWithRide()
{
	if(m_GoToId.IsNull())
		return;
	QMessageBox mb(g_Au.ApplicationName(),
			tr("This action will teleport you to the destination atom. "
				"The vehicle, mount, or spacecraft you are aboard goes with you.\n\n"
				"After 3 minutes, if the destination atom is not found, the agent teleport is redirected to the capital "
				"city of a randomly selected empire."),
		QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
	if(auApp->Frame()->ExecMessageBox(mb) != QMessageBox::Ok)
		return;
	GoToDestination(m_GoToId, true, false);
	emit AgentActionComplete();
}

//static
void AuAgentTeleport::GoToDestination(const AuId& goToId, bool rideToo, bool limboRescue)
{
	auApp->AgentGoToDestination(goToId, rideToo, limboRescue);
}

void AuAgentTeleport::SummonPerson()
{
	if(m_SummonId.IsNull())
		return;
	SummonPerson(m_SummonId, false);
	emit AgentActionComplete();
}

void AuAgentTeleport::SummonPersonWithRide()
{
	if(m_SummonId.IsNull())
		return;
	SummonPerson(m_SummonId, true);
	emit AgentActionComplete();
}

void AuAgentTeleport::SummonPerson(const AuId& summonId, bool rideToo)
{
	if(summonId.IsNull())
		return;
	auApp->AgentSummonPerson(summonId, rideToo);
}

// 'destType' is 0=avatar, 1=city, 2=unknown
void AuAgentTeleport::SetGoToDestination(const AuId& id, const QString& name, int destType)
{
	switch(destType)
	{
	case 0:
		// Summon commands are only valid when a avatar is selected.
		m_GoTo->setText("Go to Avatar " + name + "...");
		m_GoToWithRide->setText("Go to Avatar " + name + " With My Ride...");
		if(m_LimboRescue)
		{
			m_LimboRescue->setText("Limbo Rescue " + name + "...");
			m_LimboRescue->setEnabled(true);
		}
		m_Summon->setText("Summon " + name);
		m_SummonWithRide->setText("Summon " + name + " With Their Ride");
		m_Summon->setEnabled(true);
		m_SummonWithRide->setEnabled(true);
		m_SummonId = id;
		m_IsManualGoTo = false;
		break;
	case 1:
		// Cannot summon cities.
		m_GoTo->setText("Go to City " + name + "...");
		m_GoToWithRide->setText("Go to City " + name + " With My Ride...");
		if(m_LimboRescue)
			m_LimboRescue->setEnabled(false);
		m_Summon->setEnabled(false);
		m_SummonWithRide->setEnabled(false);
		m_SummonId = AuId(0);
		m_IsManualGoTo = false;
		break;
	default:
		// Cannot summon manually entered atom ids, unless you're an architect. What if it was a solar system?
		m_GoTo->setText("Go to " + name + "...");
		m_GoToWithRide->setText("Go to " + name + " With My Ride...");
		if(m_LimboRescue)
		{
			m_LimboRescue->setText("Limbo Rescue " + name + "...");
			m_LimboRescue->setEnabled(true);

			// This is an architect because only architects have the limbo rescue button.
			m_Summon->setText("Summon " + name);
			m_SummonWithRide->setText("Summon " + name + " With Their Ride");
			m_Summon->setEnabled(true);
			m_SummonWithRide->setEnabled(true);
			m_SummonId = id;
		}
		else
		{
			// This is not an architect.
			m_Summon->setEnabled(false);
			m_SummonWithRide->setEnabled(false);
			m_SummonId = AuId(0);
		}
		m_IsManualGoTo = true;
		break;
	}
	m_GoToId = id;
}

void AuAgentTeleport::AccountSelected(int index)
{
	QString charName = m_Account->itemText(index);
	AuId charId = AuId(m_Account->itemData(index).toUInt());
	SetGoToDestination(charId, charName, 0);
}

void AuAgentTeleport::AvatarSelected(int index)
{
	QString charName = m_Avatars->itemText(index);
	AuId charId = AuId(m_Avatars->itemData(index).toUInt());
	SetGoToDestination(charId, charName, 0);
}

void AuAgentTeleport::AvatarSelected(AuId charId, QString charName)
{
	// Set the destination regardless if the avatar is in the list. This happens when a avatar is selected on the
	// players list page but their empire data has not been queried.
	SetGoToDestination(charId, charName, 0);

	// Select the avatar in the combo box.
	int charIdx = m_Avatars->findData(charId.GetID());
	if(charIdx >= 0)
	{
		m_Avatars->blockSignals(true);
		m_Avatars->setCurrentIndex(charIdx);
		m_Avatars->blockSignals(false);
	}
}

void AuAgentTeleport::CitySelected(int index)
{
	QString cityName = m_Cities->itemText(index);
	AuId cityId = AuId(m_Cities->itemData(index).toUInt());
	SetGoToDestination(cityId, cityName, 1);
}

void AuAgentTeleport::SceneAtomSelected(int index)
{
	QString atomName = m_SceneAtoms->itemText(index);
	AuId atomId = AuId(m_SceneAtoms->itemData(index).toUInt());
	SetGoToDestination(atomId, atomName, 2);
}

void AuAgentTeleport::IdChanged(const QString& idText)
{
	// The user can enter a number or a string id.
	quint32 idVal = QVariant(idText).toUInt();
	AuId gotoId;
	QString gotoName;
	if(idVal)
	{
		gotoName = "Numeric Atom Id";
		gotoId = AuId(idVal);
	}
	else
	{
		gotoName = "String Atom Id";
		gotoId = AuId(idText.toUpper());
	}

	SetGoToDestination(gotoId, gotoName, 2);
}

void AuAgentTeleport::setVisible(bool visible)
{
	QWidget::setVisible(visible);

	if(visible)
		VisibilityRestored();
}

void AuAgentTeleport::VisibilityRestored()
{
	if(!m_DataValid)
		RefreshData();
}

void AuAgentTeleport::EmpireDataInvalid()
{
	m_DataValid = false;
	if(isVisible())
		RefreshData();
}

void AuAgentTeleport::RefreshSceneAtoms()
{
	if(!m_SceneAtoms)
		return;

	// Get info about current selection in combo.
	int curIdx = m_SceneAtoms->currentIndex();
	AuId atomId;
	if(curIdx >= 0)
		atomId = AuId(m_SceneAtoms->itemData(curIdx).toUInt());

	// Fill the combo with fresh data.
	m_SceneAtoms->blockSignals(true);
	m_SceneAtoms->clear();

	AuFrame* pFrame = auApp->Frame();
	AuView* view = pFrame->View();
	AuId sceneAtomId = view->SceneAtomId();
	AuEther& ether = g_Au.LockEtheriumDelete();
	DaPerson* sceneAtom = dynamic_cast<DaPerson*>(ether.FindNormalAtom(sceneAtomId));
	if(sceneAtom)
	{
		m_SpawnCity = sceneAtom->SpawnCity();
		m_SpawnCityName = sceneAtom->SpawnCityName();
		m_SpawnShip = sceneAtom->SpawnShip();
		m_SpawnShipName = sceneAtom->SpawnShipName();

		DaAtomList sceneAtoms;
		sceneAtom->ListSceneAtoms(sceneAtoms);

		// Sort scene atoms by name.
		QMultiMap<QString, DaAtom*> sortedAtoms;
		for(DaAtomList::const_iterator ai = sceneAtoms.begin(); ai != sceneAtoms.end(); ai++)
		{
			if(dynamic_cast<DaGalaxy*>(*ai) ||
					dynamic_cast<DaSector*>(*ai) ||
					dynamic_cast<DaSolarSystem*>(*ai))
				continue;

			QString sortName = (*ai)->Name().toUpper();
			sortedAtoms.insert(sortName, *ai);
		}

		// Now list them in the combo box in alphabetical order.
		for(QMultiMap<QString, DaAtom*>::const_iterator si = sortedAtoms.cbegin(); si != sortedAtoms.cend(); si++)
			m_SceneAtoms->addItem((*si)->Name() + ", " + (*si)->Description(), QVariant((*si)->Id().GetID()));
	}
	g_Au.UnlockEtheriumDelete();

	// Reselect current item, or none at all.
	if(!atomId.IsNull())
		curIdx = m_SceneAtoms->findData(atomId.GetID());
	else
		curIdx = -1;
	m_SceneAtoms->setCurrentIndex(curIdx);
	m_SceneAtoms->blockSignals(false);
}

void AuAgentTeleport::RefreshData()
{
	// Avatars
	{
		// Get info about current selection in combo.
		int curIdx = m_Avatars->currentIndex();
		AuId charId;
		if(curIdx >= 0)
			charId = AuId(m_Avatars->itemData(curIdx).toUInt());

		// Fill the combo with fresh data.
		m_Avatars->blockSignals(true);
		m_Avatars->clear();
		const DgData& gd = g_Au.LockDataForRead();
		gd.ListAllAvatars(*m_Avatars);
		g_Au.UnlockData();

		// Reselect current item, or none at all.
		if(!charId.IsNull())
			curIdx = m_Avatars->findData(charId.GetID());
		else
			curIdx = -1;
		m_Avatars->setCurrentIndex(curIdx);
		m_Avatars->blockSignals(false);
	}

	// Cities
	{
		// Get info about current selection in combo.
		int curIdx = m_Cities->currentIndex();
		AuId cityId;
		if(curIdx >= 0)
			cityId = AuId(m_Cities->itemData(curIdx).toUInt());

		// Fill the combo with fresh data.
		m_Cities->blockSignals(true);
		m_Cities->clear();
		const DgData& gd = g_Au.LockDataForRead();
		gd.ListAllCities(*m_Cities);
		g_Au.UnlockData();

		// Reselect current item, or none at all.
		if(!cityId.IsNull())
			curIdx = m_Cities->findData(cityId.GetID());
		else
			curIdx = -1;
		m_Cities->setCurrentIndex(curIdx);
		m_Cities->blockSignals(false);
	}

	// Scene atoms
	RefreshSceneAtoms();

	// Account
	{
		QMultiMap<QString, AuId> acct;
		for(int c=0; c<6; c++)
		{
			const DgData& gd = g_Au.LockDataForRead();
			AuId avatarId = gd.ClientUnitId(c);
			QString avatarName = gd.ClientUnitName(avatarId);
			g_Au.UnlockData();
			if(!avatarId.IsNull() && !avatarName.isEmpty())
				acct.insert(QString("%1, %2").arg(avatarName).arg(avatarId.StringId()), avatarId);
		}
		if(!m_SpawnCity.IsNull() && !m_SpawnCityName.isEmpty())
			acct.insert(QString("Home: %1").arg(m_SpawnCityName), m_SpawnCity);
		if(!m_SpawnShip.IsNull() && !m_SpawnShipName.isEmpty())
			acct.insert(QString("Berth: %1").arg(m_SpawnShipName), m_SpawnShip);

		// Get info about current selection in combo.
		int curIdx = m_Account->currentIndex();
		AuId acctId;
		if(curIdx >= 0)
			acctId = AuId(m_Account->itemData(curIdx).toUInt());

		// Fill the combo with fresh data.
		m_Account->blockSignals(true);
		m_Account->clear();
		for(QMultiMap<QString, AuId>::const_iterator ai = acct.cbegin(); ai != acct.cend(); ai++)
			m_Account->addItem(ai.key(), QVariant((*ai).GetID()));	// Add item with record id.

		// Reselect current item, or none at all.
		if(!acctId.IsNull())
			curIdx = m_Account->findData(acctId.GetID());
		else
			curIdx = -1;
		m_Account->setCurrentIndex(curIdx);
		m_Account->blockSignals(false);
	}

	m_DataValid = true;
}

void AuAgentTeleport::RefreshEmpires()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;

	auApp->RequestEmpiresAll();

	emit EmpireDataRequested();

	AuEmpireWait waitDlg(this);
	connect(this, SIGNAL(EmpireDataComplete()), &waitDlg, SLOT(accept()));
	pFrame->ExecDialog(waitDlg);

	EmpireDataInvalid();
}

void AuAgentTeleport::ClearAtomId()
{
	leatom->setText("");
}

void AuAgentTeleport::GoToGalaxy()
{
	AuGoToGalaxyLoc dlg(this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	GALAXY galaxy = dlg.Galaxy();
	AuPoint coord = dlg.Coordinate();

	auApp->AgentGoToGalaxy(galaxy, coord, false);

	emit AgentActionComplete();
}

void AuAgentTeleport::GoToGalaxyWithRide()
{
	AuGoToGalaxyLoc dlg(this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	GALAXY galaxy = dlg.Galaxy();
	AuPoint coord = dlg.Coordinate();

	auApp->AgentGoToGalaxy(galaxy, coord, true);

	emit AgentActionComplete();
}

