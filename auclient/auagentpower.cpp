//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auagentpower.cpp,v 1.24 2022/02/19 18:03:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auagentpower.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "aufilever.h"
#include "auglobal.h"
#include "auframe.h"
#include "austory.h"
#include "auview.h"
#include "dabd.h"
#include "daperson.h"
#include "daworld.h"
#include "dgdata.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>

AuAgentPower::AuAgentPower(QWidget* parent) : QWidget(parent)
{
	quint8 agentBits = auApp->Frame()->View()->PersonAgentBits();
	const DgData& gd = g_Au.LockDataForRead();
	bool isAgent = gd.ClientPlayerIsAgent();	// also true if programmer or architect
	bool isProgrammer = gd.ClientPlayerIsProgrammer();	// also true if architect
	bool isArchitect = gd.ClientPlayerIsArchitect();
	bool isMinion = gd.ClientPlayerIsMinion();
	g_Au.UnlockData();

	QString title;
	QPixmap img;
	if(isArchitect)
	{
		title = "Architect";
		img = QPixmap(":data/Architect.png").scaled(64,64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}
	else if(isProgrammer)
	{
		title = "Programmer";
		img = QPixmap(":data/Programmer.png").scaled(64,64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}
	else if(isMinion)
	{
		title = "Minion";
		img = QPixmap(":data/Minion.png").scaled(64,64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}
	else
	{
		title = "Agent";
		img = QPixmap(":data/Agent.png").scaled(64,64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}

	QLabel* lbimg = new QLabel(this);
	lbimg->setPixmap(img);

	m_MyServer = new QLabel("My Server", this);

	// Special agent powers.
	if(!isMinion)
	{
		m_ckAgent = new QCheckBox(tr("%2 Powers").arg(title), this);
		if(agentBits & AGENT_Enable)
			m_ckAgent->setChecked(true);
		m_ckAgent->setToolTip(tr("<html>Enables or disables %2 powers. "
					"<br>"
					"%1 Disable %2 powers to resume playing as a normal player.<br>"
					"%1 %2 powers are not needed to use the buttons on any tabs of this window.<br>"
					"%1 When %2 powers apply to buttons, the tootip usually explains with a bold <b>Agent Power</b> note.<br>"
					"%1 %2 powers only affect certain activities implicit to being an agent.<br>"
					"%1 <b>City Inventory</b> Every city will respond to the <i>Inventory of City</i> request.<br>"
					"%1 <b>Fast Recall</b> Recall home takes only a few seconds.<br>"
					"%1 <b>Safe Recall</b> Recall to a different zone does not lose your gear.<br>"
					"%1 <b>Fast Survey</b> Your crew will complete survey scans instantly while you are aboard.<br>"
					"%1 <b>Command</b> Ships, cities and other things will consider you to be in their chain of command for some things."
					"<br>"
					"%1 <b>Ship Command</b> You are in the chain of command of every ship you are aboard."
					"<br>"
					"%1 <b>Kick</b> Kick officers from all ships and captains from fleet ships.<br>"
					"%1 <b>Kick On Line Stowaways</b> Kick stowaways also kicks on-line stow aways.<br>"
					"%1 <b>Static Displays</b> Manipulate all static displayed items.<br>"
					"%1 <b>Manufacture</b> Control all manufacturing processes.<br>"
					"%1 <b>No Capture</b> You will not influence flags at town squares or bases.<br>"
					"%1 <b>No Drop</b> You will not drop an item when you die.<br>"
					"%1 <b>No Indigenous</b> You will not trigger encounters with indigenous, which can establish local worker DNA.<br>"
					"%1 <b>No Target</b> You will not be targeted by AI operating turrets.<br>"
					"%1 <b>See/Show Paths</b> Hold a gem in hand to see AI walk paths in ships and buildings. "
					"Other avatars in your ship/building see them too. "
					"</html>")
				.arg(g_Au.MiddleDotSymbol()).arg(title));
		connect(m_ckAgent, SIGNAL(clicked()), SLOT(ToggleAgentBits()));
	}
	else
		m_ckAgent = 0;

	if(!isMinion)
	{
		m_ckInvincible = new QCheckBox(tr("Invincible"), this);
		if(agentBits & AGENT_Invincible)
			m_ckInvincible->setChecked(true);
		m_ckInvincible->setToolTip(tr("<html>You cannot be harmed by anything, including the environment. "
					"Since you cannot be harmed, your personal equipment will not be damaged and you cannot be poisoned, diseased, "
					"implanted, etc.<br>"
					"%1 <b>No Capture</b> You will not influence flags at town squares or bases.</html>")
				.arg(g_Au.MiddleDotSymbol()));
		connect(m_ckInvincible, SIGNAL(clicked()), SLOT(ToggleAgentBits()));
	}
	else
		m_ckInvincible = 0;

	if(!isMinion)
	{
		m_ckInvisible = new QCheckBox(tr("Invisible"), this);
		if(agentBits & AGENT_Invisible)
			m_ckInvisible->setChecked(true);
		m_ckInvisible->setToolTip(tr("<html>You are not visible in anyone's scene.<br>"
					"%1 Invisibility overrides any other appearance setting; you are not visible.<br>"
					"%1 <b>No Capture</b> You will not influence flags at town squares or bases.<br>"
					"%1 <b>No Spot</b> You cannot be spotted by anyone using the ` or ~ keys.<br>"
					"%1 <b>No Stare</b> AI citizens will not stare at you.<br>"
					"%1 <b>No Target</b> You cannot be targeted by AI operating turrets or wild animals.<br>"
					"%1 <b>No War</b> Killing units will not make your empires become mutual enemies.</html>")
				.arg(g_Au.MiddleDotSymbol()));
		connect(m_ckInvisible, SIGNAL(clicked()), SLOT(ToggleAgentBits()));
	}
	else
		m_ckInvisible = 0;

	if(isProgrammer && !isMinion)
	{
		m_ckAnimal = new QCheckBox(tr("Animal"), this);
		if(agentBits & AGENT_Animal)
			m_ckAnimal->setChecked(true);
		m_ckAnimal->setToolTip(tr("<html>Your spot description will give the animal description of your DNA.<br>"
					"%1 <b>Animal Name</b> Your name will show as your specie name in chat, when spotted, and other places.<br>"
					"%1 <b>Anonymous</b> Comm messages will not reveal your id.<br>"
					"%1 <b>No Capture</b> You will not influence flags at town squares or bases.<br>"
					"%1 <b>No Equipment</b> Your helmet, clothing and item in hand will not be shown.<br>"
					"%1 <b>No Sensors</b> You do not show up on transporter sensors.<br>"
					"%1 <b>No War</b> Killing units will not make your empires become mutual enemies."
					"<br>"
					"%1 <b>Hushed</b> Comm channels will warn before sending your messages. "
					"</html>")
				.arg(g_Au.MiddleDotSymbol()));
		connect(m_ckAnimal, SIGNAL(clicked()), SLOT(ToggleAgentBits()));

		m_ckEntity = new QCheckBox(tr("Entity"), this);
		if(agentBits & AGENT_Entity)
			m_ckEntity->setChecked(true);
		m_ckEntity->setToolTip(tr("<html>You appear as a glowing bodiless entity of light the color of your empire contrails. "
					"The color updates live when the contrail color changes.<br>"
					"%1 <b>Anonymous</b> Comm messages will not reveal your id.<br>"
					"%1 <b>Entity Name</b> Your name will show as <i>Entity</i> in chat and other places.<br>"
					"%1 <b>No Capture</b> You will not influence flags at town squares or bases.<br>"
					"%1 <b>No Damage</b> You cannot take damage except when bleeding out during unconsiousness.<br>"
					"%1 <b>No Equipment</b> Your helmet, clothing and item in hand will not be shown.<br>"
					"%1 <b>No Gravity</b> Your movement is not affected by gravity.<br>"
					"%1 <b>No Obstacles</b> Your movement is not obstructed by terrain, spacecraft walls or equipment.<br>"
					"%1 <b>No Sensors</b> You do not show up on transporter sensors.<br>"
					"%1 <b>No Spot</b> You cannot be spotted by anyone using the ` or ~ keys.<br>"
					"%1 <b>No Target</b> You cannot be targeted by AI operating turrets or wild animals.<br>"
					"%1 <b>No War</b> Killing units will not make your empires become mutual enemies."
					"<br>"
					"%1 <b>Hushed</b> Comm channels will warn before sending your messages. "
					"</html>")
				.arg(g_Au.MiddleDotSymbol()));
		connect(m_ckEntity, SIGNAL(clicked()), SLOT(ToggleAgentBits()));
	}
	else
	{
		m_ckAnimal = 0;
		m_ckEntity = 0;
	}

	if(isAgent && !isMinion)
	{
		m_ckIncognito = new QCheckBox(tr("Incognito"), this);
		if(agentBits & AGENT_Incognito)
			m_ckIncognito->setChecked(true);
		m_ckIncognito->setToolTip(tr("<html>Hides the fact that you are an agent:<br>"
					"%1 <b>Incognito</b> Your name is not highlighted on the player status web page or players online window. "
					"Haxus is not listed at all. "
					"<br>"
					"%1 <b>No Item Empire</b> Items you make are not identified with any empire. "
					"<br>"
					"%1 <b>Hushed</b> Comm channels will warn before sending your messages. "
					"</html>")
				.arg(g_Au.MiddleDotSymbol()));
		connect(m_ckIncognito, SIGNAL(clicked()), SLOT(ToggleAgentBits()));
	}
	else
	{
		m_ckIncognito = 0;
	}

	QPushButton* pbmakeitem = 0;
	if(isProgrammer || isMinion)
	{
		pbmakeitem = new QPushButton("Make Item or Vehicle...");
		connect(pbmakeitem, SIGNAL(clicked()), SLOT(MakeItem()));
		pbmakeitem->setToolTip(QString("<html>Creates an item or vehicle at the agent's location.</html>"));
	}

	QPushButton* pbcancelstory = 0;
	QPushButton* pbchgmydna = 0;
	QPushButton* pbresetstory = 0;
	QPushButton* pbsubmitstory = 0;
	if(isProgrammer)
	{
		pbchgmydna = new QPushButton("My DNA...");
		connect(pbchgmydna, SIGNAL(clicked()), SLOT(ChangeMyDNA()));

		pbresetstory = new QPushButton("Reset Story...");
		connect(pbresetstory, SIGNAL(clicked()), SLOT(ResetStory()));
		pbresetstory->setToolTip(QString("<html>Resets a specific story as if you never started it.</html>"));

		pbsubmitstory = new QPushButton("Submit Story Script...");
		connect(pbsubmitstory, SIGNAL(clicked()), SLOT(SubmitStory()));
		pbsubmitstory->setToolTip(QString("<html>Submits a story script onto an avatar, as a custom story. "
					"An associated story job will start on the avatar, when any autostart condition is true. "
					"</html>"));

		pbcancelstory = new QPushButton("Cancel Story Script...");
		connect(pbcancelstory, SIGNAL(clicked()), SLOT(CancelStory()));
		pbcancelstory->setToolTip(QString("<html>Cancels a custom story script that was submitted for an avatar. "
					"Any associated story job is canceled. "
					"</html>"));
	}

	// Construct layout.
	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbimg, row,0, 1,1);
	lay->addWidget(m_MyServer, row,1, 1,7);
	row++;

	if(m_ckAgent)
	{
		lay->addWidget(m_ckAgent, row,0, 1,8);
		row++;
	}

	if(m_ckInvincible || m_ckInvisible || m_ckAnimal || m_ckEntity)
	{
		if(m_ckInvincible)
			lay->addWidget(m_ckInvincible, row,0, 1,2);
		if(m_ckInvisible)
			lay->addWidget(m_ckInvisible, row,2, 1,2);
		if(m_ckAnimal)
			lay->addWidget(m_ckAnimal, row,4, 1,2);
		if(m_ckEntity)
			lay->addWidget(m_ckEntity, row,6, 1,2);
		if(m_ckIncognito)
			lay->addWidget(m_ckIncognito, row,8, 1,2);
		row++;
	}

	if(pbmakeitem)
	{
		lay->addWidget(pbmakeitem, row,2, 1,8);
		row++;
	}

	lay->setRowMinimumHeight(row++, 10);

	if(pbchgmydna)
	{
		lay->addWidget(pbchgmydna, row,2, 1,8);
		row++;
	}

	lay->setRowMinimumHeight(row++, 10);

	if(pbresetstory)
	{
		lay->addWidget(pbresetstory, row,2, 1,8);
		row++;
	}

	if(pbsubmitstory && pbcancelstory)
	{
		lay->addWidget(pbsubmitstory, row,2, 1,4);
		lay->addWidget(pbcancelstory, row,6, 1,4);
		row++;
	}

	lay->setRowStretch(row++, 1);

	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuAgentPower::~AuAgentPower()
{
}

void AuAgentPower::setVisible(bool visible)
{
	QWidget::setVisible(visible);

	if(visible)
		VisibilityRestored();
}

void AuAgentPower::VisibilityRestored()
{
	QString unit;
	if(g_Au.ScenePort() == 16760)
		unit = "s1";
	else if(g_Au.ScenePort() == 16764)
		unit = "s2";
	else if(g_Au.ScenePort() == 16767)
		unit = "s3";
	else if(g_Au.ScenePort() == 16770)
		unit = "s4";
	else
		unit = "s?";
	QString myServer = QString("Scene Server %1 : %2 %3").arg(g_Au.SceneServer()).arg(g_Au.ScenePort()).arg(unit);
	m_MyServer->setText(myServer);

	AuId agentId = auApp->Frame()->View()->SceneAtomId();
	if(agentId.IsNull())
	{
		if(m_ckAgent)
			m_ckAgent->setEnabled(false);
		if(m_ckAnimal)
			m_ckAnimal->setEnabled(false);
		if(m_ckEntity)
			m_ckEntity->setEnabled(false);
		if(m_ckIncognito)
			m_ckIncognito->setEnabled(false);
		if(m_ckInvincible)
			m_ckInvincible->setEnabled(false);
		if(m_ckInvisible)
			m_ckInvisible->setEnabled(false);
	}
	else
	{
		quint8 agentBits = auApp->Frame()->View()->PersonAgentBits();

		if(m_ckAgent)
		{
			m_ckAgent->blockSignals(true);
			m_ckAgent->setChecked(agentBits & AGENT_Enable);
			m_ckAgent->blockSignals(false);
			m_ckAgent->setEnabled(true);
		}

		if(m_ckAnimal)
		{
			m_ckAnimal->blockSignals(true);
			m_ckAnimal->setChecked(agentBits & AGENT_Animal);
			m_ckAnimal->blockSignals(false);
			m_ckAnimal->setEnabled(true);
		}

		if(m_ckEntity)
		{
			m_ckEntity->blockSignals(true);
			m_ckEntity->setChecked(agentBits & AGENT_Entity);
			m_ckEntity->blockSignals(false);
			m_ckEntity->setEnabled(true);
		}

		if(m_ckIncognito)
		{
			m_ckIncognito->blockSignals(true);
			m_ckIncognito->setChecked(agentBits & AGENT_Incognito);
			m_ckIncognito->blockSignals(false);
			m_ckIncognito->setEnabled(true);
		}

		if(m_ckInvincible)
		{
			m_ckInvincible->blockSignals(true);
			m_ckInvincible->setChecked(agentBits & AGENT_Invincible);
			m_ckInvincible->blockSignals(false);
			m_ckInvincible->setEnabled(true);
		}

		if(m_ckInvisible)
		{
			m_ckInvisible->blockSignals(true);
			m_ckInvisible->setChecked(agentBits & AGENT_Invisible);
			m_ckInvisible->blockSignals(false);
			m_ckInvisible->setEnabled(true);
		}
	}
}

void AuAgentPower::ToggleAgentBits()
{
	quint8 bits = 0;
	if(m_ckAgent && m_ckAgent->isChecked())
		bits |= AGENT_Enable;
	if(m_ckAnimal && m_ckAnimal->isChecked())
		bits |= AGENT_Animal;
	if(m_ckEntity && m_ckEntity->isChecked())
		bits |= AGENT_Entity;
	if(m_ckInvincible && m_ckInvincible->isChecked())
		bits |= AGENT_Invincible;
	if(m_ckInvisible && m_ckInvisible->isChecked())
		bits |= AGENT_Invisible;
	if(m_ckIncognito && m_ckIncognito->isChecked())
		bits |= AGENT_Incognito;

	auApp->AgentSetBits(bits);
}

void AuAgentPower::MakeItem()
{
	AuAgentMakeItem makeItem(this);
	if(makeItem.exec() == QDialog::Accepted)
	{
		AuAgentMakeItem::MAKEWHAT makeWhat = makeItem.MakeWhat();
		COMMODITYID cid = makeItem.CommodityId();
		quint8 quality = makeItem.Quality();
		quint32 count = makeItem.Count();
		quint8 galaxy = makeItem.Galaxy();
		AuPoint galaxyCoord = makeItem.GalaxyCoord();
		auApp->AgentMakeItem(cid, quality, count, galaxy, galaxyCoord, makeWhat);
	}
}

void AuAgentPower::SubmitStory()
{
	// Get default avatar id and name from the scene atom's mount or the scene atom.
	AuId avatarId;
	QString avatarName;
	AuFrame* pFrame = auApp->Frame();
	AuView* view = pFrame->View();
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		const DaPerson* mount = dynamic_cast<const DaPerson*>(person->Parent());
		if(mount)
		{
			avatarId = mount->Id();
			avatarName = mount->Name();
		}
		else
		{
			avatarId = person->Id();
			avatarName = person->Name();
		}
	}
	view->UnlockSceneAtom();

	// Query the user for the avatar id.
	bool ok;
	QString sid = QInputDialog::getText(this, tr("Submit Story for %1").arg(avatarName),
			tr("<html>Enter the atom id of the avatar to receive the submitted story.<br>"
				"The default id is for <b>%1</b>.<br>"
				"It is determined by the avatar you are riding or you if not riding anyone.</html>")
			.arg(avatarName),
			QLineEdit::Normal, avatarId.StringId(), &ok);
	// Bail if they canceled.
	if(!ok)
		return;

	// Bail if they gave us a bogus id.
	avatarId = AuId(sid);
	if(avatarId.IsNull())
		return;

	// Load last known file folder.
	QString lastFile = g_Au.Settings().readEntry("Widget/Admin/ScriptFile", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();
	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

	QString file = QFileDialog::getOpenFileName(this, QObject::tr("Select Script File"), lastFile, "", 0,
			g_Au.UseNativeFileDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
	if(file.isEmpty())
		return;

	// Store last known file folder.
	g_Au.Settings().writeEntry("Widget/Admin/ScriptFile", file);

	AuScript script;	// Construct with 0 script id.
	if(!script.Load(file))
	{
		QString text = tr("Script compiler reported errors.");
		for(QStringList::const_iterator si = script.CompilerOutput().cbegin(); si != script.CompilerOutput().cend(); si++)
			text += tr("<br> %1").arg(*si);

		QMessageBox mb(g_Au.ApplicationName(), text,
				QMessageBox::Critical, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton, auApp->Frame());
		auApp->Frame()->ExecMessageBox(mb);
	}
	else
	{
		// Send the script packet to the server.
		auApp->StorySubmitScript(avatarId, script);
	}
}

void AuAgentPower::CancelStory()
{
	// Get default avatar id and name from the scene atom's mount or the scene atom.
	AuId avatarId;
	QString avatarName;
	AuFrame* pFrame = auApp->Frame();
	AuView* view = pFrame->View();
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		const DaPerson* mount = dynamic_cast<const DaPerson*>(person->Parent());
		if(mount)
		{
			avatarId = mount->Id();
			avatarName = mount->Name();
		}
		else
		{
			avatarId = person->Id();
			avatarName = person->Name();
		}
	}
	view->UnlockSceneAtom();

	// Query the user for the avatar id.
	bool ok;
	QString sid = QInputDialog::getText(this, tr("Clear Story for %1").arg(avatarName),
			tr("<html>Enter the atom id of the avatar to clear the submitted story.<br>"
				"The default id is for <b>%1</b>.<br>"
				"It is determined by the avatar you are riding or you if not riding anyone.</html>")
			.arg(avatarName),
			QLineEdit::Normal, avatarId.StringId(), &ok);
	// Bail if they canceled.
	if(!ok)
		return;

	// Bail if they gave us a bogus id.
	avatarId = AuId(sid);
	if(avatarId.IsNull())
		return;

	auApp->StoryCancelSubmission(avatarId);
}

void AuAgentPower::ResetStory()
{
	// Get default avatar id and name from the scene atom's mount or the scene atom.
	AuId avatarId;
	QString avatarName;
	AuFrame* pFrame = auApp->Frame();
	AuView* view = pFrame->View();
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		const DaPerson* mount = dynamic_cast<const DaPerson*>(person->Parent());
		if(mount)
		{
			avatarId = mount->Id();
			avatarName = mount->Name();
		}
		else
		{
			avatarId = person->Id();
			avatarName = person->Name();
		}
	}
	view->UnlockSceneAtom();

	// Query the user for the avatar id.
	bool ok;
	QString sid = QInputDialog::getText(this, tr("Reset Story for %1").arg(avatarName),
			tr("<html>Enter the atom id of the avatar whose story will be reset.<br>"
				"The default id is for <b>%1</b>.<br>"
				"It is determined by the avatar you are riding or you if not riding anyone.</html>")
			.arg(avatarName),
			QLineEdit::Normal, avatarId.StringId(), &ok);
	// Bail if they canceled.
	if(!ok)
		return;

	// Bail if they gave us a bogus id.
	avatarId = AuId(sid);
	if(avatarId.IsNull())
		return;

	// Get the default script id to reset.
	quint32 scriptId = g_Au.Settings().readNumEntry("Widget/Admin/ResetStory", 0);

	// Query the user for the script id.
	QStringList stories;
	stories << "1. Veil of Targoss";
	stories << "2. Space Program";
	stories << "3. Getting Started";
	stories << "4. Rocket Training";
	stories << "5. Caroler.";
	stories << "6. Relic";
	stories << "7. Dangerous Exchange";
	stories << "8. Beacon";
	stories << "9. Reach for the Stars";
	stories << "10. Undertow";
	int scriptIndex = scriptId - 1;
	QString script = QInputDialog::getItem(this, tr("Reset Story"),
			tr("<html>Enter the script id of the story to reset. "
				"The avatar will no longer have a record of starting or finishing the story.</html>"), stories,
			scriptIndex, false, &ok);
	// Bail if they canceled.
	if(!ok)
		return;

	// Bail if they gave us a bogus id.
	scriptId = stories.indexOf(script) + 1;
	if(!scriptId)
		return;

	// Save the new default script id to reset.
	g_Au.Settings().writeEntry("Widget/Admin/ResetStory", (int)scriptId);

	auApp->StoryReset(avatarId, scriptId);
}

void AuAgentPower::ChangeMyDNA()
{
	// Locate target atom for default DNA.
	AuView* view = auApp->Frame()->View();
	if(!view->TargetDNA())
		return;
	AuAgentPowerSetDNA setDNADlg(*view->TargetDNA(), this);			// Default is DNA of whatever was targeted last.
	if(auApp->Frame()->ExecDialog(setDNADlg) == QDialog::Accepted)
		auApp->ChangeMyDNA(setDNADlg.DNA());
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuAgentMakeItem::AuAgentMakeItem(QWidget* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":data/c_Crate.png"));
	setAuDlgTitle("<b>Choose Commodity</b>");

	m_MakeWhat = Nothing;

	COMMODITYID lastCID = (COMMODITYID)g_Au.Settings().readNumEntry("Widget/AdminMakeItem/CID", CID_Invalid);
	if(lastCID <= CID_Invalid || lastCID >= CID_Count)
		lastCID = CID_Flag;
	quint8 lastQ = g_Au.Settings().readNumEntry("Widget/AdminMakeItem/Quality", 1);
	if(lastQ < 1)
		lastQ = 1;
	quint32 lastCount = g_Au.Settings().readNumEntry("Widget/AdminMakeItem/Count", 1);
	if(lastCount < 1)
		lastCount = 1;
	quint8 lastGalaxy = g_Au.Settings().readNumEntry("Widget/AdminMakeItem/Galaxy", ShoresOfHazeron);
	QString lastGalaxyCoord = g_Au.Settings().readEntry("Widget/AdminMakeItem/GalaxyCoord", "");

	QLabel* lbinfo = new QLabel("<html>"
			"Made in empire of new items will be set to the agent's empire, unless agent is incognito. "
			"When agent is incognito, made in empire will be blank. "
			"</html>", this);
	lbinfo->setWordWrap(true);

	// Combo box lists all possible commodities, including vehicles.
	QMap<QString, COMMODITYID> sortedCIDs;	// commodities sorted alphabetically
	for(DbCommodityArray::const_iterator cit = g_Au.m_CommodityArray.constBegin(); cit != g_Au.m_CommodityArray.constEnd(); cit++)
		if((*cit).m_id != CID_Invalid && (*cit).m_id != CID_Money && !(*cit).IsObsolete())
			sortedCIDs.insert((*cit).m_name, (*cit).m_id);

	m_Commodity = new QComboBox(this);
	for(QMap<QString, COMMODITYID>::const_iterator sci = sortedCIDs.constBegin(); sci != sortedCIDs.constEnd(); sci++)
		m_Commodity->addItem(QIcon(g_Au.CommodityBitmapName(*sci)), sci.key(), QVariant(*sci));
	m_Commodity->setCurrentIndex(m_Commodity->findData(QVariant(lastCID)));
	m_Commodity->setToolTip(tr("<html>Item or vehicle to make. Vehicles are created fully fueled, engine running, "
				"with you in the driver's seat.</html>"));

	// Spin box for quality.
	m_Quality = new QSpinBox(this);
	m_Quality->setRange(1,255);
	m_Quality->setValue(lastQ);
	m_Quality->setToolTip(tr("<html>Quality to make.</html>"));
	connect(m_Quality, SIGNAL(valueChanged(int)), SLOT(MakeQualityChanged(int)));

	// Count to make.
	m_Count = new QLineEdit(this);
	m_Count->setText(QVariant(lastCount).toString());
	m_Count->setToolTip(tr("<html>Count to make.<br>"
				"%1 When count is more than one, items are added to the current ship's hold or to the current city's inventory.<br>"
				"%1 Limit is 10000 when adding to ship's hold or city's inventory; limit is 10 everywhere else.<br>"
				"%1 Count is ignored when making a vehicle.</html>")
			.arg(g_Au.MiddleDotSymbol()));

	// Combo box for galaxy, for preons.
	m_Galaxy = new QComboBox(this);
	for(int galaxy=AndromedaRising; galaxy<GALAXY_Count; galaxy++)
		m_Galaxy->insertItem(0, GalaxyName((GALAXY)galaxy), QVariant(galaxy));
	m_Galaxy->setCurrentIndex(m_Galaxy->findData(QVariant(lastGalaxy)));
	m_Galaxy->setToolTip(tr("<html>Destination galaxy for preons.</html>"));

	// Galaxy coordinate for preons
	m_GalaxyCoord = new QLineEdit(this);
	m_GalaxyCoord->setText(lastGalaxyCoord);
	m_GalaxyCoord->setToolTip(tr("<html>Destination galaxy coordinate for preons.<br>"
				"%1 Coord is in pc. e.g. Center of sector (214,2,1) is 2140,20,10.<br>"
				"%1 Systems are always +4.9 to -4.9 of their sector coordinate, in .1 increments.<br>"
				"%1 A stargate opened using the preon finds the closest system to the coordinate.</html>")
			.arg(g_Au.MiddleDotSymbol()));

	QPushButton* pbitems = new QPushButton(tr("Make Multiple Items"), this);
	pbitems->setToolTip(tr("<html>Makes a number of the items specified and adds them to your inventory."
				"</html>"));
	connect(pbitems, SIGNAL(clicked()), SLOT(MakeItems()));

	QPushButton* pbcityinv = new QPushButton(tr("Make City Inventory"), this);
	pbcityinv->setToolTip(tr("<html>Makes a number of the items specified and adds them to the inventory of your current city."
				"</html>"));
	connect(pbcityinv, SIGNAL(clicked()), SLOT(MakeCityInventory()));

	QPushButton* pbshipcargo = new QPushButton(tr("Make Ship Cargo"), this);
	pbshipcargo->setToolTip(tr("<html>Makes a number of the items specified and adds them to the inventory of your current ship."
				"</html>"));
	connect(pbshipcargo, SIGNAL(clicked()), SLOT(MakeShipCargo()));

	QPushButton* pbone = new QPushButton(tr("Make One Item/Vehicle"), this);
	pbone->setAutoDefault(true);
	pbone->setDefault(true);
	pbone->setToolTip(tr("<html>Makes one of the item or vehicle specified. "
				"The value in the count control is ignored."
				"</html>"));
	connect(pbone, SIGNAL(clicked()), SLOT(MakeOne()));

	QPushButton* pbutil = new QPushButton(tr("Utility Pouch"), this);
	pbutil->setToolTip(tr("<html>Makes a utility pouch filled with tools. "
				"The value in the count control is ignored."
				"</html>"));
	connect(pbutil, SIGNAL(clicked()), SLOT(MakeUtilityPouch()));

	QPushButton* pbknap = new QPushButton(tr("Knapsack"), this);
	pbknap->setToolTip(tr("<html>Makes a knapsack filled with a few basic items. "
				"The value in the count control is ignored."
				"</html>"));
	connect(pbknap, SIGNAL(clicked()), SLOT(MakeKnapsack()));

	QPushButton* pbpack = new QPushButton(tr("Backpack"), this);
	pbpack->setToolTip(tr("<html>Makes a backpack filled with a good set of adventure gear. "
				"The value in the count control is ignored."
				"</html>"));
	connect(pbpack, SIGNAL(clicked()), SLOT(MakeBackpack()));

	QPushButton* pbgrav = new QPushButton(tr("Grav Pack"), this);
	pbgrav->setToolTip(tr("<html>Makes a grav pack filled with Haxus' personal selection of gear. "
				"The value in the count control is ignored."
				"</html>"));
	connect(pbgrav, SIGNAL(clicked()), SLOT(MakeGravPack()));

	// Construct final overall layout.
	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbinfo, row,0, 1,4);
	row++;
	lay->addWidget(m_Commodity, row,0);
	lay->addWidget(m_Quality, row,1, 1,2);
	lay->addWidget(m_Count, row,3);
	row++;
	lay->addWidget(m_Galaxy, row,0, 1,2);
	lay->addWidget(m_GalaxyCoord, row,2, 1,2);
	row++;
	lay->addWidget(pbone, row,0, 1,2);
	lay->addWidget(pbitems, row,2, 1,2);
	row++;
	lay->addWidget(pbcityinv, row,0, 1,2);
	lay->addWidget(pbshipcargo, row,2, 1,2);
	row++;
	lay->addWidget(pbutil, row,0, 1,1);
	lay->addWidget(pbknap, row,1, 1,1);
	lay->addWidget(pbpack, row,2, 1,1);
	lay->addWidget(pbgrav, row,3, 1,1);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuAgentMakeItem::~AuAgentMakeItem()
{
}

COMMODITYID AuAgentMakeItem::CommodityId() const
{
	COMMODITYID cid = (COMMODITYID)(m_Commodity->itemData(m_Commodity->currentIndex()).toInt());
	g_Au.Settings().writeEntry("Widget/AdminMakeItem/CID", cid);
	return cid;
}

quint8 AuAgentMakeItem::Quality() const
{
	quint8 quality = (quint8)m_Quality->value();
	g_Au.Settings().writeEntry("Widget/AdminMakeItem/Quality", quality);
	return quality;
}

quint32 AuAgentMakeItem::Count() const
{
	// When only one was made, don't update the last stored count value.
	if(m_MakeWhat != MultipleItems && m_MakeWhat != CityInventory && m_MakeWhat != ShipCargo)
		return 1;

	quint32 count = QVariant(m_Count->text()).toUInt();
	g_Au.Settings().writeEntry("Widget/AdminMakeItem/Count", count);
	return count;
}

quint8 AuAgentMakeItem::Galaxy() const
{
	quint8 galaxy = (quint8)(m_Galaxy->itemData(m_Galaxy->currentIndex()).toUInt());
	g_Au.Settings().writeEntry("Widget/AdminMakeItem/Galaxy", galaxy);
	return galaxy;
}

AuPoint AuAgentMakeItem::GalaxyCoord() const
{
	AuPoint coord(m_GalaxyCoord->text());
	g_Au.Settings().writeEntry("Widget/AdminMakeItem/GalaxyCoord", m_GalaxyCoord->text());
	return coord;
}

void AuAgentMakeItem::MakeItems()
{
	m_MakeWhat = MultipleItems;
	accept();
}

void AuAgentMakeItem::MakeCityInventory()
{
	m_MakeWhat = CityInventory;
	accept();
}

void AuAgentMakeItem::MakeShipCargo()
{
	m_MakeWhat = ShipCargo;
	accept();
}

void AuAgentMakeItem::MakeOne()
{
	m_MakeWhat = OneItem;
	accept();
}

void AuAgentMakeItem::MakeUtilityPouch()
{
	m_MakeWhat = UtilityPouch;
	accept();
}

void AuAgentMakeItem::MakeKnapsack()
{
	m_MakeWhat = Knapsack;
	accept();
}

void AuAgentMakeItem::MakeBackpack()
{
	m_MakeWhat = Backpack;
	accept();
}

void AuAgentMakeItem::MakeGravPack()
{
	m_MakeWhat = GravPack;
	accept();
}

void AuAgentMakeItem::MakeQualityChanged(int index)
{
	Q_UNUSED(index);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuAgentPowerSetDNA::AuAgentPowerSetDNA(const AuDNABase& defaultDNA, QWidget* parent) : AuDialog(parent)
{
	m_DNA = defaultDNA.Clone();

	setAuDlgIcon(QIcon(":data/NameSpecie.png"));
	setAuDlgTitle("<b>My DNA</b>");

	// Construct the widgets.
	m_DNADesc = new QLabel(this);	// Text updated in DNAChanged.
	m_DNADesc->setToolTip(tr("<html>Description of DNA that is selected. "
				"The default DNA is the last creature that was spotted in the scene.</html>"));
	//m_DNADesc->setWordWrap(true);

	QPushButton* pbsave = new QPushButton(tr("Save DNA To File..."), this);
	pbsave->setToolTip(tr("Saves DNA to an avatar (.chr) file. "
				"The DNA that is shown is the DNA that is saved to the file."));
	connect(pbsave, SIGNAL(clicked()), SLOT(SaveDNAToFile()));

	QPushButton* pbload = new QPushButton(tr("Load DNA From File..."), this);
	pbload->setToolTip(tr("Loads DNA from an avatar (.chr) file."));
	connect(pbload, SIGNAL(clicked()), SLOT(LoadDNAFromFile()));

	QPushButton* pbrandomrat = new QPushButton(tr("Random Rat DNA"), this);
	pbrandomrat->setToolTip(tr("Creates random rat DNA."));
	connect(pbrandomrat, SIGNAL(clicked()), SLOT(RandomRatDNA()));

	QPushButton* pbrandom = new QPushButton(tr("Random DNA"), this);
	pbrandom->setToolTip(tr("Creates random DNA."));
	connect(pbrandom, SIGNAL(clicked()), SLOT(RandomDNA()));

	QPushButton* pbworker = new QPushButton(tr("Worker DNA"), this);
	pbworker->setToolTip(tr("Gets the worker DNA from the current city."));
	connect(pbworker, SIGNAL(clicked()), SLOT(WorkerDNA()));

	QPushButton* pbok = new QPushButton(tr("Set My DNA"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));

	// Construct final overall layout.
	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(m_DNADesc, row,0, 1,2);
	row++;
	lay->addWidget(pbsave, row,0);
	row++;
	lay->addWidget(pbload, row,0);
	lay->addWidget(pbworker, row,1);
	row++;
	lay->addWidget(pbrandomrat, row,1);
	row++;
	lay->addWidget(pbrandom, row,1);
	row++;
	row++;
	lay->addWidget(pbok, row,1);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);

	// Update contents of DNA descriptive text box.
	DNAChanged();
}

AuAgentPowerSetDNA::~AuAgentPowerSetDNA()
{
	delete m_DNA;
}

void AuAgentPowerSetDNA::DNAChanged()
{
	// Update the DNA descriptive text widget.
	Q_ASSERT(m_DNADesc);
	const DgData& gd = g_Au.LockDataForRead();
	QString specie = gd.SpeciesName(m_DNA->Species(), false);	// Returns blank if specie not named.
	g_Au.UnlockData();
	QString desc;
	if(specie.isEmpty())
		desc = QString("<html><b>Unnamed Specie</b><br>%1<br></html>")
			.arg(m_DNA->SpeciesDescription());
	else
		desc = QString("<html><b>%1</b><br>%2<br></html>")
			.arg(specie)
			.arg(m_DNA->SpeciesDescription());
	m_DNADesc->setText(desc);
}

// Copied from ausplash.cpp AuSplash::SaveCharacter.
void AuAgentPowerSetDNA::SaveDNAToFile()
{
	if(m_DNA->Version() == 4)
	{
		QString lastFile = g_Au.Settings().readEntry("Settings/LastDNAFile", "");
		if(lastFile.isEmpty())
			lastFile = QDir::currentPath();

		QFileInfo lastFileInfo(lastFile);
		if(!lastFileInfo.exists())
			lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

		QString saveFilter;
		saveFilter += QString("Hazeron DNA (*.dna)");
		saveFilter += QString(";;All Files (*.*)");

		QString saveFile = QFileDialog::getSaveFileName(this, tr("Hazeron DNA File"), lastFile, saveFilter, 0,
				QFileDialog::DontUseNativeDialog);
		if(saveFile.isEmpty())
			return;

		// Insure the file has an appropriate suffix.
		QString suffix(".dna");
		if(saveFile.right(suffix.length()).compare(suffix, Qt::CaseInsensitive) != 0)
			saveFile += suffix;

		g_Au.Settings().writeEntry("Settings/LastDNAFile", saveFile);

		int result = ((AuDNA*)m_DNA)->SaveFile(saveFile);
		if(result == 1)
		{
			QString smsg = QObject::tr("Unable to open file for writing.\n\n%1").arg(saveFile);
			QMessageBox::warning(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok);
		}
	}
	else
	{
		// Load last known file folder.
		QString lastFile = g_Au.Settings().readEntry("/Settings/LastChrFile", "");
		if(lastFile.isEmpty())
			lastFile = QDir::currentPath();
		QFileInfo lastFileInfo(lastFile);
		if(!lastFileInfo.exists())
			lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

		// Ask the user for a file name.
		QString sFilter;
		sFilter.append("Avatar File (*.chr)");
		QString sFile = QFileDialog::getSaveFileName(this, "Save As...", lastFile, sFilter, 0,
				g_Au.UseNativeFileDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
		if(!sFile.isEmpty())
		{
			// Open/create the file.
			QFile file(sFile);
			if(file.open(QFile::WriteOnly))
			{
				// Store the file data.
				QByteArray baDNA;
				baDNA.resize(sizeof(DhDNA_));
				DhDNA* dna3 = (DhDNA*)m_DNA;
				DhDNA_ dna(*dna3);
				memcpy(baDNA.data(), &dna, sizeof(DhDNA_));

				QDataStream ds(&file);
				ds.setVersion(QDataStream::Qt_4_1);

				// File Type and Version Bytes
				const quint8 chrFileType = CHR_FILE_TYPE;	// Not 1 or 0 so we can identify old .chr files.
				const quint8 chrFileVersion = CHR_FILE_VER;	// First version stored in a .chr file.
				ds << chrFileType;
				ds << chrFileVersion;

				// Character Data
				quint8 gender = 1;
				bool isLefty = false;
				ds << baDNA;
				ds << gender;
				ds << isLefty;

				file.close();
			}
		}
	}
}

// Copied from ausplash.cpp AuSplash::LoadCharacter.
void AuAgentPowerSetDNA::LoadDNAFromFile()
{
	// Load last known file folder.
	QString lastFile = g_Au.Settings().readEntry("/Settings/LastChrFile", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();
	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

	QFileDialog fileDialog(this, tr("Load Avatar File..."), lastFile);
	fileDialog.setFileMode(QFileDialog::ExistingFile);
	if(!g_Au.UseNativeFileDialog())
		fileDialog.setOptions(QFileDialog::DontUseNativeDialog);
	QStringList sFilters;
	sFilters.append("Avatar File (*.chr)");
	fileDialog.setNameFilters(sFilters);
	fileDialog.exec();
	if(!fileDialog.selectedFiles().isEmpty())
	{
		// Store last known file folder.
		QString sFile = fileDialog.selectedFiles().first();
		g_Au.Settings().writeEntry("/Settings/LastChrFile", sFile);

		QFile file(sFile);
		if(file.open(QFile::ReadOnly))
		{
			QDataStream ds(&file);
			ds.setVersion(QDataStream::Qt_4_1);

			DhDNA_ dna;
			bool bMale = false;
			bool bLefty = false;

			// File Type Byte
			quint8 chrFileType;
			ds >> chrFileType;
			if(chrFileType == CHR_FILE_TYPE)
			{
				// Version Byte
				quint8 chrFileVersion;
				ds >> chrFileVersion;

				// Character Data
				QByteArray baDNA;
				ds >> baDNA;
				dna = DhDNA_(baDNA);

				ds >> bMale;
				ds >> bLefty;
			}
			else
			{
				// Old 'Jacob' format file.
				file.seek(0);		// Restore file pos back to beginning.
				QByteArray baFileData = file.read(sizeof(DhDNA_));
				dna = DhDNA_(baFileData);
				QByteArray ba = file.read(sizeof(bool)*2);
				QDataStream ds(&ba, QIODevice::ReadOnly);
				ds.setVersion(QDataStream::Qt_4_1);
				ds >> bMale;
				ds >> bLefty;

				// Update size values for DNA changes that doubled length and thickness values. D.H.H. 12/22/2010
				if(dna.CalfLength*2 > 31)
					dna.CalfLength = 31;
				else
					dna.CalfLength *= 2;

				if(dna.CalfThickness*2 > 31)
					dna.CalfThickness = 31;
				else
					dna.CalfThickness *= 2;

				if(dna.ForeArmLength*2 > 31)
					dna.ForeArmLength = 31;
				else
					dna.ForeArmLength *= 2;

				if(dna.ForeArmThickness*2 > 31)
					dna.ForeArmThickness = 31;
				else
					dna.ForeArmThickness *= 2;

				if(dna.FootLength*2 > 31)
					dna.FootLength = 31;
				else
					dna.FootLength *= 2;

				if(dna.HandLength*2 > 31)
					dna.HandLength = 31;
				else
					dna.HandLength *= 2;

				if(dna.OverallSize*2 > 31)
					dna.OverallSize = 31;
				else
					dna.OverallSize *= 2;

				if(dna.ThighLength*2 > 31)
					dna.ThighLength = 31;
				else
					dna.ThighLength *= 2;

				if(dna.ThighThickness*2 > 31)
					dna.ThighThickness = 31;
				else
					dna.ThighThickness *= 2;

				if(dna.TorsoSegmentLength*2 > 31)
					dna.TorsoSegmentLength = 31;
				else
					dna.TorsoSegmentLength *= 2;

				if(dna.TorsoThickness*2 > 31)
					dna.TorsoThickness = 31;
				else
					dna.TorsoThickness *= 2;

				if(dna.UpperArmLength*2 > 31)
					dna.UpperArmLength = 31;
				else
					dna.UpperArmLength *= 2;

				if(dna.UpperArmThickness*2 > 31)
					dna.UpperArmThickness = 31;
				else
					dna.UpperArmThickness *= 2;
			}

			file.close();

			// Use the DNA as the current default.
			if(m_DNA)
				delete m_DNA;
			m_DNA = new DhDNA(dna);
			DNAChanged();
		}
	}
}

void AuAgentPowerSetDNA::RandomDNA()
{
	if(m_DNA)
		delete m_DNA;
	DhDNA dna;
	dna.Randomize(0, true, TOD_Any, g_Au.Dice(), true);
	m_DNA = dna.Clone();
	DNAChanged();
}

void AuAgentPowerSetDNA::RandomRatDNA()
{
	if(m_DNA)
		delete m_DNA;
	DhDNA dna;
	dna.RandomVermin(g_Au.Dice());
	m_DNA = dna.Clone();
	DNAChanged();
}

void AuAgentPowerSetDNA::WorkerDNA()
{
	bool didChange = false;
	AuView* view = auApp->Frame()->View();
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		DaWorld* planet = person->FindParentPlanetInEnv();
		if(planet)
		{
			if(!person->BdId().IsNull())
			{
				DaBd* bd = planet->FindBd(person->BdId());
				if(bd)
				{
					AuSpecies species(bd->ResidentSpecies());
					if(!species.IsNull())
					{
						if(m_DNA)
							delete m_DNA;
						m_DNA = bd->ResidentSpecies().CreateDNA();
						didChange = true;
					}
				}
			}
		}
	}
	view->UnlockSceneAtom();

	if(didChange)
		DNAChanged();
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
