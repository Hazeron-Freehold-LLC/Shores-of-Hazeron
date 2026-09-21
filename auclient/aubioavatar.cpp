//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubioavatar.cpp,v 1.19 2022/04/20 19:11:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubioavatar.h"
//
#include "auapp.h"
#include "auframe.h"
#include "aupushbutton.h"
#include "auselfie.h"
#include "auskill.h"
#include "autoolbutton.h"
#include "auview.h"
#include "daperson.h"
#include "dgdata.h"
#include <QGridLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>

AuBioAvatar::AuBioAvatar(QWidget *parent) : QWidget(parent)
{
	m_timer = 0;

	AuFrame* pFrame = auApp->Frame();
	const DgData& gd = g_Au.LockDataForRead();
	bool isAgent = gd.ClientPlayerIsAgent();
	g_Au.UnlockData();

	lbname = new QLabel(this);
	lbname->setWordWrap(true);

	lbimg = new QLabel(this);

	AuIconButton* pbrequestavatars = new AuIconButton(QIcon(":data/c_VacSuitHelmet.png"), this);
	pbrequestavatars->setToolTip(tr("<html><b>Refresh Avatars</b>"
				"<p>The list of avatars can change while you are on line. "
				"This button refreshes the list of avatars with an updated list from the server.</p>"
				"<i>Things That Change Your Avatar List</i><br>"
				"%1 When you give birth to an avatar, the new avatar is added to your account if an empty slot is available.<br>"
				"%1 When a female citizen gives birth to a demiavatar that you fathered, the new avatar is added to your account "
				"if an empty slot is available.<br>"
				"%1 When a demiavatar dies, it is removed from the owner's account."
				"</html>").arg(g_Au.MiddleDotSymbol()));
	connect(pbrequestavatars, SIGNAL(clicked()), auApp, SLOT(RequestAvatars()));

	AuIconButton* pbselfie = new AuIconButton(QIcon(":data/c_Camera.png"), this);
	pbselfie->setToolTip(tr("<b>Selfie...</b><br>"
					"Sends the current avatar image on this window to the server. "
					"The image is shown to you for confirmation before sending. "
					"The selfie will appear on the current avatar's web page."));
	connect(pbselfie, SIGNAL(clicked()), SLOT(GrabSelfie()));

	pbrelease = new QPushButton(tr("Release Berth..."));
	pbrelease->setIcon(QIcon(":data/dp_Berth0.png"));
	pbrelease->setVisible(false);
	pbrelease->setToolTip(tr("<b>Release My Berth</b><br>"
				"Releases your berth. You will no longer occupy any berth on any ship's roster.<br>"
				"You do not need to be aboard a ship to release your berth."));
	connect(pbrelease, SIGNAL(clicked()), SLOT(ReleaseSlot()));

	QPushButton* pbspawnrecall = new QPushButton(tr("Recall Home"));
	pbspawnrecall->setIcon(QIcon(":data/SpawnRecall.png"));
	if(isAgent)
		pbspawnrecall->setToolTip(tr("<html><b>Recall Home</b><br>"
					"Teleports your avatar to your home location, in this order of precedence.<br>"
					"  1. <b>Spacecraft</b> where you hold a berth. "
					"You recall to your berth or standing at the helm if the ship has no berths. "
					"<b>E to exit a berth</b>. "
					"A berth provides a breathable environment. "
					"Ship life support must be functioning to recall aboard.<br>"
					"  2. <b>Home city</b>, if that city is friendly to your empire. "
					"You appear near where you stood when you declared the city your home.<br>"
					"  3. <b>Capital city</b> of your empire, if that city is controlled by your empire.<br>"
					"  4. <b>Randomly selected city</b> of your empire.<br>"
					"  5. <b>Empty world</b> that is habitable.<br>"
					"A <i>period of vulnerability</i> ensues while waiting to recall, during which you can be seen and "
					"injured.<br>"
					"<b>Cancel recall</b> by pressing the Recall Home button while waiting to recall.<br>"
					"<b><u>Equipment and money are lost if you recall to a different dimension. "
					"</b></u>"
					"Sea chest contents are not lost; your sea chest is different in each zone: conflict or neutral.<br>"
					"<b>Agent Power</b> When agent power is enabled, you do not lose your stuff."
					"</html>"));
	else
		pbspawnrecall->setToolTip(tr("<html><b>Recall Home</b><br>"
					"Teleports your avatar to your home location, in this order of precedence.<br>"
					"  1. <b>Spacecraft</b> where you hold a berth. "
					"You recall to your berth or standing at the helm if the ship has no berths. "
					"<b>E to exit a berth</b>. "
					"A berth provides a breathable environment. "
					"Ship life support must be functioning to recall aboard.<br>"
					"  2. <b>Home city</b>, if that city is friendly to your empire. "
					"You appear near where you stood when you declared the city your home.<br>"
					"  3. <b>Capital city</b> of your empire, if that city is controlled by your empire.<br>"
					"  4. <b>Randomly selected city</b> of your empire.<br>"
					"  5. <b>Empty world</b> that is habitable.<br>"
					"A <i>period of vulnerability</i> ensues while waiting to recall, during which you can be seen and "
					"injured.<br>"
					"<b>Cancel recall</b> by pressing the Recall Home button while waiting to recall.<br>"
					"<b><u>Equipment and money are lost if you recall to a different dimension. "
					"</b></u>"
					"Sea chest contents are not lost; your sea chest is different in each zone: conflict or neutral."
					"</html>"));
	connect(pbspawnrecall, SIGNAL(clicked()), pFrame, SLOT(SpawnRecall()));

	pbrename = new QPushButton(tr("Rename Avatar..."));
	pbrename->setIcon(QIcon(":data/Bio.png"));
	pbrename->setVisible(false);
	pbrename->setToolTip(tr("<html><b>Rename Avatar</b><br>"
				"Assigns a different name to your avatar.<br>"
				"%1 This button is visible while your avatar is still a child."
				"</html>").arg(g_Au.MiddleDotSymbol()));
	connect(pbrename, SIGNAL(clicked()), SLOT(RenameAvatar()));

	lbcredit = new QLabel(this);
	lbcredit->setWordWrap(true);

	lbreputation = new QLabel(this);
	lbreputation->setWordWrap(true);

	lbwd = new QLabel(this);
	lbwd->setWordWrap(true);

	lbspawn = new QLabel(this);
	//lbspawn->setAlignment(Qt::AlignLeft|Qt::AlignTop);
	lbspawn->setWordWrap(true);

	lbflag = new QLabel(this);
	lbflag->setAlignment(Qt::AlignLeft|Qt::AlignTop);

	lbempirename = new QLabel(this);
	lbempirename->setWordWrap(true);

	lbemperorname = new QLabel(this);
	lbemperorname->setWordWrap(true);

	// Hair
	QMap<QString, AuHairStyle::STYLE> hairStyles;
	for(int s=AuHairStyle::Unkempt+1; s<AuHairStyle::STYLE_Count; s++)
	{
		AuHairStyle::STYLE style = (AuHairStyle::STYLE)s;
		hairStyles.insert(AuHairStyle::Name(style), style);
	}
	QMap<QString, AuHairStyle::SHAVE> shaveStyles;
	for(int s=AuHairStyle::Unshaven+1; s<AuHairStyle::SHAVE_Count; s++)
	{
		AuHairStyle::SHAVE shave = (AuHairStyle::SHAVE)s;
		shaveStyles.insert(AuHairStyle::ShaveName(shave), shave);
	}

	m_Haircut = new QComboBox;
	m_Haircut->setVisible(false);
	m_Haircut->setToolTip(tr("<html>"
				"<p>"
				"<b>Hair Style</b>"
				"<br>"
				"Hair styles affect head hair only. "
				"Hair on other body parts is not styled. "
				"</p>"
				"<p>"
				"<i>Buzz</i>, <i>Close</i>, <i>Straight Out</i>, and <i>Unkempt</i> show every follicle. "
				"Other styles may hide hair. "
				"</p>"
				"</html>"));
	for(int s=0; s<=AuHairStyle::Unkempt; s++)
		m_Haircut->addItem(AuHairStyle::Name((AuHairStyle::STYLE)s), s);
	for(QMap<QString, AuHairStyle::STYLE>::const_iterator hi = hairStyles.cbegin(); hi != hairStyles.cend(); hi++)
		m_Haircut->addItem(hi.key(), *hi);
	connect(m_Haircut, SIGNAL(currentIndexChanged(int)), SLOT(HaircutChanged(int)));

	m_Shave = new QComboBox;
	m_Shave->setVisible(false);
	m_Shave->setToolTip(tr("<html>"
				"<p>"
				"<b>Shave</b>"
				"<br>"
				"Shaves affect head hair only. "
				"Hair on other body parts is not shaved. "
				"</p>"
				"<p>"
				"<i>Close</i>, <i>Straight Out</i>, <i>Stubble</i>, and <i>Unshaven</i> show every follicle. "
				"Other shaves may hide hair. "
				"</p>"
				"</html>"));
	for(int s=0; s<=AuHairStyle::Unshaven; s++)
		m_Shave->addItem(AuHairStyle::ShaveName((AuHairStyle::SHAVE)s), s);
	for(QMap<QString, AuHairStyle::SHAVE>::const_iterator si = shaveStyles.cbegin(); si != shaveStyles.cend(); si++)
		m_Shave->addItem(si.key(), *si);
	connect(m_Shave, SIGNAL(currentIndexChanged(int)), SLOT(ShaveChanged(int)));

	// Create widgets.
	QGridLayout* avlay = new QGridLayout;
	avlay->addWidget(pbrequestavatars, 0,0);
	avlay->addWidget(pbselfie, 1,0);
	for(int ichr=0; ichr<6; ichr++)
	{
		m_Avatars[ichr] = new AuPushButtonValue("", ichr, this);
		connect(m_Avatars[ichr], SIGNAL(ClickedValue(int)), SLOT(SwitchToAvatar(int)));
		m_Avatars[ichr]->hide();
		avlay->addWidget(m_Avatars[ichr], ichr,1);
	}
	avlay->setRowStretch(6,1);
	avlay->addWidget(m_Haircut, 7,1);
	avlay->addWidget(m_Shave, 8,1);

	QHBoxLayout* homelay = new QHBoxLayout;
	homelay->addWidget(pbrename);
	homelay->addWidget(pbrelease);
	homelay->addWidget(pbspawnrecall);

	// Layout the dialog.
	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbflag, row,0);
	lay->addWidget(lbimg, row,1, 9,1);
	lay->addLayout(avlay, row,2, 9,1);
	row++;
	lay->addWidget(lbname, row,0);
	row++;
	lay->addWidget(lbempirename, row,0);
	row++;
	lay->addWidget(lbemperorname, row,0);
	row++;
	lay->addWidget(lbreputation, row,0);
	row++;
	lay->addWidget(lbcredit, row,0);
	row++;
	lay->addWidget(lbwd, row,0);
	row++;
	lay->addWidget(lbspawn, row,0);
	row++;
	lay->setRowStretch(row, 10);
	row++;
	lay->addLayout(homelay, row,0, 1,3);
	row++;
	lay->setColumnStretch(0, 1);
	lay->setMargin(4);
	lay->setSpacing(2);
	setLayout(lay);
}

AuBioAvatar::~AuBioAvatar()
{
}

QSize AuBioAvatar::sizeHint() const
{
	return QSize(600,300);
}

void AuBioAvatar::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_timer)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_timer->isActive())
		m_timer->start(1000);
}

void AuBioAvatar::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be recreated during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	// Get the scene atom so we can show their information.
	AuFrame* pFrame = auApp->Frame();
	AuView* view = pFrame->View();
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(!person)
	{
		view->UnlockSceneAtom();
		return;
	}

	QImage viewImage(view->SceneAtomImage());
	if(!viewImage.isNull())
	{
		QImage image(256,256, QImage::Format_RGB32);
		image.fill(QColor(64,64,64));
		QPainter p(&image);
		p.drawImage(QRect(0,0,256,256), viewImage);
		lbimg->setPixmap(QPixmap::fromImage(image));
	}

	// Get the known ids and names of their characters.
	// First, hide the ones that need to be invisible.
	const DgData& gd = g_Au.LockDataForRead();
	for(int ichr=0; ichr<6; ichr++)
	{
		AuId unitId = gd.ClientUnitId(ichr);
		if(unitId.IsNull())
			m_Avatars[ichr]->hide();
	}
	// Second, show the ones that need to be visible.
	for(int ichr=0; ichr<6; ichr++)
	{
		AuId unitId = gd.ClientUnitId(ichr);
		if(!unitId.IsNull())
		{
			QString name(gd.ClientUnitName(unitId));
			m_Avatars[ichr]->show();
			m_Avatars[ichr]->setText(name);
			if(unitId == gd.ClientUnitId())
			{
				m_Avatars[ichr]->setEnabled(false);
				m_Avatars[ichr]->setToolTip(tr("<b>%1</b> <i>%2</i> is Current").arg(name).arg(unitId.StringId()));
			}
			else
			{
				m_Avatars[ichr]->setEnabled(true);
				m_Avatars[ichr]->setToolTip(tr("Switch to <b>%1</b> <i>%2</i>").arg(name).arg(unitId.StringId()));
			}
		}
	}

	QString swd = tr("Wormholes Explored <b>%L1</b>").arg(person->WormholesExplored());

	QString name;
	QString semp;
	QString semper;
	QString scredit;
	QString sss;
	QPixmap pxflag;
	if(person->EmpireId())
	{
		// Get the person's fully decorated name from the model.
		name = QString("<big><b>%1</b></big> <i>%2</i>").arg(person->TitleName()).arg(person->Id().StringId());

		scredit = tr("Credit Limit <b>%L1%2</b>").arg(person->CreditLimit(), 0,'f',0).arg(g_Au.CurrencySymbol());

		sss = tr("Reputation <b>%L1</b>").arg(person->Reputation());

		// Get information about the person's empire of allegiance.
		DgEmpire emp;
		emp.m_id = person->Allegiance();
		gd.Empire(emp);

		semp = tr("Empire <b>") + emp.HtmlName() + "</b>";

		AuId emperorId(emp.EmperorId());
		if(emperorId.IsNull())
			semper = "Emperor <b>.</b>";
		else
			semper = emp.CitizenTitle(emperorId) + " <b>" + emp.CitizenName(emperorId) + "</b>";

		pxflag = QPixmap::fromImage(emp.m_flag);
	}
	else
	{
		name = QString("<big><b>%1</b></big> <i>%2</i>").arg(person->Name()).arg(person->Id().StringId());
	}

	g_Au.UnlockData();

	QString spawnInfo;
	bool hasBerth = !person->SpawnShipName().isEmpty();
	if(hasBerth)
	{
		if(!spawnInfo.isEmpty())
			spawnInfo += "<br>";
		spawnInfo += tr("Berthed Aboard <b>%1</b>").arg(person->SpawnShipName());

		pbrelease->setText(tr("Release Berth on %1").arg(person->SpawnShipName()));
	}
	if(!person->SpawnCityName().isEmpty())
	{
		if(!spawnInfo.isEmpty())
			spawnInfo += "<br>";
		if(!person->SpawnCitySystemName().isEmpty())
			spawnInfo += tr("Home <b>%1, %2</b>").arg(person->SpawnCityName()).arg(person->SpawnCitySystemName());
		else
			spawnInfo += tr("Home <b>%1</b>").arg(person->SpawnCityName());
	}

	bool isYoung = !person->IsFullGrown();
	AuHairStyle::STYLE haircut = person->Haircut();
	AuHairStyle::SHAVE shave = person->Shave();

	int dnaVersion = 0;
	AuDNABase* dna = person->DNAnew();
	if(dna)
	{
		dnaVersion = dna->Version();
		delete dna;
	}

	view->UnlockSceneAtom();

	lbname->setText(name);
	pbrename->setVisible(isYoung);
	pbrelease->setVisible(hasBerth);
	lbcredit->setText(scredit);
	lbreputation->setText(sss);
	lbwd->setText(swd);
	lbflag->setPixmap(pxflag);
	lbempirename->setText(semp);
	lbemperorname->setText(semper);
	lbspawn->setText(spawnInfo);

	m_Haircut->setVisible(dnaVersion == 4);
	m_Haircut->blockSignals(true);
	m_Haircut->setCurrentIndex(m_Haircut->findData(haircut));
	m_Haircut->blockSignals(false);

	m_Shave->setVisible(dnaVersion == 4);
	m_Shave->blockSignals(true);
	m_Shave->setCurrentIndex(m_Shave->findData(shave));
	m_Shave->blockSignals(false);
}

void AuBioAvatar::ReleaseSlot()
{
	QString smsg = tr("<html>You are about to release your berth. You will no longer hold a berth on any ship's roster.<br><br>"
			"Are you sure you want to release your berth?");
	QMessageBox mb(g_Au.ApplicationName(), smsg, QMessageBox::Question, QMessageBox::Yes, QMessageBox::Cancel,
			QMessageBox::NoButton, this);
	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result == QMessageBox::Yes)
		auApp->ReleaseBerth();
}

void AuBioAvatar::SwitchToAvatar(int index)
{
	const DgData& gd = g_Au.LockDataForRead();
	AuId switchTo = gd.ClientUnitId(index);
	g_Au.UnlockData();

	// Set up to wait for a different scene atom to view.
	AuFrame* pFrame = auApp->Frame();
	pFrame->SwitchToAvatar(switchTo);
}

void AuBioAvatar::GrabSelfie()
{
	AuView* pView = auApp->Frame()->View();
	QImage avatarImage = pView->SceneAtomImage();
	QString avatarName = pView->SceneAtomName();

	if(avatarImage.isNull())
		return;

	AuSelfie dlg(avatarName, avatarImage, auApp->Frame());
	auApp->Frame()->ExecDialog(dlg);
}

void AuBioAvatar::RenameAvatar()
{
	bool ok;
	QString name = QInputDialog::getText(this, g_Au.ApplicationName(),
			tr("Enter a name for your avatar."),
			QLineEdit::Normal, "", &ok).trimmed();
	if(!ok)
		return;

	if(!IsNameValid(name))
	{
		QMessageBox::critical(this, g_Au.ApplicationName(),
				tr("The avatar name is invalid. A valid name must be less than %1 characters "
					"and it must not contain certain reserved characters such as | and newline.")
				.arg(MaxNameLength()+1),
				QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		return;
	}

	auApp->AvatarRename(name);
}

void AuBioAvatar::HaircutChanged(int index)
{
	AuHairStyle::STYLE haircut = (AuHairStyle::STYLE)m_Haircut->itemData(index).toInt();
	AuHairStyle::SHAVE shave = (AuHairStyle::SHAVE)m_Shave->currentData().toInt();
	auApp->SetHairStyle(haircut, shave);
}

void AuBioAvatar::ShaveChanged(int index)
{
	AuHairStyle::STYLE haircut = (AuHairStyle::STYLE)m_Haircut->currentData().toInt();
	AuHairStyle::SHAVE shave = (AuHairStyle::SHAVE)m_Shave->itemData(index).toInt();
	auApp->SetHairStyle(haircut, shave);
}

