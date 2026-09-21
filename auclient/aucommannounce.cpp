//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommannounce.cpp,v 1.7 2022/02/19 18:10:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucommannounce.h"
//
#include "auapp.h"
#include "aucommpacket.h"
#include <auglobal.h>
#include "auframe.h"
#include "auview.h"
#include "daperson.h"
#include "dasolarsystem.h"
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AuCommAnnounce::AuCommAnnounce(const QString& answerWho, QWidget* parent) : AuDialog(parent)
{
	setObjectName("CommAnnounce");
	if(answerWho.isEmpty())
		setAuDlgTitle(tr("Announce"));
	else
		setAuDlgTitle(tr("Reply to Hail"));
	setAuDlgIcon(QIcon(":data/CommAnnounce.png"));

	// Collect pieces of information for building a standard reply.
	QStringList identifiers;
	QStringList identifierIcons;
	QStringList loca;
	QStringList locaIcons;
	QStringList repairs;
	QStringList repairIcons;
	QStringList postures;
	QStringList postureIcons;
	{
		AuFrame* pFrame = auApp->Frame();
		if(!pFrame)
			return;
		AuView* view = pFrame->View();
		if(!view)
			return;

		DaPerson* person;
		view->LockSceneAtom(&person);
		if(person)
		{
			const DaAtom* system = person->FindEnvironmentAtom();
			if(system)
			{
				m_SystemId = system->Id();

				const DaSector* sector = system->FindParentSector();
				if(sector)
				{
					const DaVessel* ship = person->FindParentVesselInEnv();
					const DaSentient* officer;
					if(ship)
						officer = ship->FindOfficer();
					else
						officer = 0;

					// Build list of identifiers.
					QString decName = person->TitleName();
					if(!answerWho.isEmpty())
					{
						identifiers << tr("Hello %1.").arg(answerWho);
						identifierIcons << QString(":data/RangeClose.png");
					}
					identifiers << tr("Hello.");
					identifierIcons << QString(":data/RangeClose.png");
					identifiers << tr("%1 here.").arg(person->Name());
					identifierIcons << QString(":data/GovSelf.png");
					identifiers << tr("%1 here.").arg(decName);
					identifierIcons << QString(":data/GovSelf.png");
					if(ship)
					{
						identifiers << tr("%1 here.").arg(ship->NameWithEmpire());
						identifierIcons << QString(":data/FleetShip.png");
						identifiers << tr("%1 aboard %2 here.").arg(person->Name()).arg(ship->NameWithEmpire());
						identifierIcons << QString(":data/FleetShip.png");
						identifiers << tr("%1 aboard %2 here.").arg(decName).arg(ship->NameWithEmpire());
						identifierIcons << QString(":data/FleetShip.png");
					}

					// Say where I am.
					loca << person->CommMessageLocation();
					locaIcons << QString(":data/GovSelf.png");
					if(ship)
					{
						loca << ship->CommMessageLocation();
						locaIcons << QString(":data/FleetShip.png");
					}

					// Add repair estimate onto hail reply.
					if(ship && ship->RepairSecondsRemaining())
					{
						repairs << tr("Repairing damage, %1 seconds until finished.").arg(ship->RepairSecondsRemaining());
						repairIcons << QString(":data/FleetShip.png");
					}

					// Postures
					postures << tr("I want to kill.");
					postureIcons << QString(":data/PostureBerserk.png");
					postures << tr("I am watching for threats.");
					postureIcons << QString(":data/PostureOffensive.png");
					postures << tr("I am defending my area.");
					postureIcons << QString(":data/PostureDefendArea.png");
					postures << tr("I am holding fire unless attacked.");
					postureIcons << QString(":data/PostureDefendSelf.png");
					postures << tr("I am holding fire.");
					postureIcons << QString(":data/PostureHoldFire.png");
					if(officer)
					{
						switch(officer->Posture())
						{
						case POSTURE_Offensive:
							postures << tr("Our posture is offensive; we are watching for threats.");
							postureIcons << QString(":data/Officer.png");
							break;
						case POSTURE_DefendSelf:
							postures << tr("Our posture is self defense; we are holding fire unless attacked.");
							postureIcons << QString(":data/Officer.png");
							break;
						case POSTURE_DefendArea:
							postures << tr("Our posture is defensive; we are defending our area.");
							postureIcons << QString(":data/Officer.png");
							break;
						case POSTURE_Berserk:
							postures << tr("Our posture is berserk; we want to kill.");
							postureIcons << QString(":data/Officer.png");
							break;
						case POSTURE_HoldFire:
							postures << tr("Our posture is hold fire; we are defenseless.");
							postureIcons << QString(":data/Officer.png");
							break;
						}
					}
				}
			}
		}
		view->UnlockSceneAtom();
	}

	QLabel* lbid;
	if(!identifiers.isEmpty())
	{
		lbid = new QLabel(tr("Identity"));
		lbid->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

		m_Identifiers = new QComboBox(this);
		m_Identifiers->setToolTip(tr("<html>Identity<br>"
					"If you are aboard a ship, identity options are listed with an icon of a spacecraft.</html>"));
		m_Identifiers->addItem(tr(""));
		QStringList::const_iterator si = identifiers.constBegin();
		QStringList::const_iterator sii = identifierIcons.constBegin();
		for(; si != identifiers.constEnd() && sii != identifierIcons.constEnd(); si++, sii++)
			m_Identifiers->addItem(QIcon(*sii), *si);
		m_Identifiers->setCurrentIndex(m_Identifiers->count()-1);
	}
	else
	{
		lbid = 0;
		m_Identifiers = 0;
	}

	QLabel* lbloc;
	if(!loca.isEmpty())
	{
		lbloc = new QLabel(tr("Location"));
		lbloc->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

		m_Loca = new QComboBox(this);
		m_Loca->setToolTip(tr("<html>Location<br>"
					"If you are aboard a ship, its location is listed with an icon of a spacecraft.</html>"));
		m_Loca->addItem(tr(""));
		QStringList::const_iterator si = loca.constBegin();
		QStringList::const_iterator sii = locaIcons.constBegin();
		for(; si != loca.constEnd() && sii != locaIcons.constEnd(); si++, sii++)
			m_Loca->addItem(QIcon(*sii), *si);
		m_Loca->setCurrentIndex(0);
	}
	else
	{
		lbloc = 0;
		m_Loca = 0;
	}

	QLabel* lbrep;
	if(!repairs.isEmpty())
	{
		lbrep = new QLabel(tr("Repairs"));
		lbrep->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

		m_Repairs = new QComboBox(this);
		m_Repairs->setToolTip(tr("<html>Repair Time</html>"));
		m_Repairs->addItem(tr(""));
		QStringList::const_iterator si = repairs.constBegin();
		QStringList::const_iterator sii = repairIcons.constBegin();
		for(; si != repairs.constEnd() && sii != repairIcons.constEnd(); si++, sii++)
			m_Repairs->addItem(QIcon(*sii), *si);
		m_Repairs->setCurrentIndex(0);
	}
	else
	{
		lbrep = 0;
		m_Repairs = 0;
	}

	QLabel* lbpos;
	if(!postures.isEmpty())
	{
		lbpos = new QLabel(tr("Posture"));
		lbpos->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

		m_Postures = new QComboBox(this);
		m_Postures->setToolTip(tr("<html>Posture<br>"
					"If the ship has an officer, its posture is listed with an icon of an officer.</html>"));
		m_Postures->addItem(tr(""));
		QStringList::const_iterator si = postures.constBegin();
		QStringList::const_iterator sii = postureIcons.constBegin();
		for(; si != postures.constEnd() && sii != postureIcons.constEnd(); si++, sii++)
			m_Postures->addItem(QIcon(*sii), *si);
		m_Postures->setCurrentIndex(0);
	}
	else
	{
		lbpos = 0;
		m_Postures = 0;
	}

	m_Comments = new QLineEdit(this);
	m_Comments->setMaxLength(AuCommChat::MaxChatText());
	m_Comments->setToolTip(tr("<html>Additional Announcements</html>"));

	m_Announce = new QPushButton(tr("Send Announcement"), this);
	m_Announce->setToolTip(tr("<html>Transmits the announcement. "
				"The announcement is transmitted on the current comm channel.</html>"));
	connect(m_Announce, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(m_Announce);

	// Layout the controls.
	QGridLayout* lay = new QGridLayout();
	int row = 0;
	if(m_Identifiers)
	{
		lay->addWidget(lbid, row,0);
		lay->addWidget(m_Identifiers, row,1);
		row++;
	}
	if(m_Loca)
	{
		lay->addWidget(lbloc, row,0);
		lay->addWidget(m_Loca, row,1);
		row++;
	}
	if(m_Repairs)
	{
		lay->addWidget(lbrep, row,0);
		lay->addWidget(m_Repairs, row,1);
		row++;
	}
	if(m_Postures)
	{
		lay->addWidget(lbpos, row,0);
		lay->addWidget(m_Postures, row,1);
		row++;
	}
	lay->addWidget(m_Comments, row,1);
	row++;
	lay->addLayout(blay, row,0, 1,2);
	row++;
	lay->setColumnStretch(1, 10);
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuCommAnnounce::~AuCommAnnounce()
{
}

void AuCommAnnounce::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/CommAnnounce/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/CommAnnounce/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/CommAnnounce/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/CommAnnounce/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);

	if(m_Identifiers)
	{
		int identifier = g_Au.Settings().readNumEntry("Widget/CommAnnounce/Identity", m_Identifiers->currentIndex());
		if(identifier >= 0 && identifier < m_Identifiers->count())
			m_Identifiers->setCurrentIndex(identifier);
	}
}

void AuCommAnnounce::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/CommAnnounce/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/CommAnnounce/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/CommAnnounce/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/CommAnnounce/SizeY",sz.height());

	if(m_Identifiers)
		g_Au.Settings().writeEntry("Widget/CommAnnounce/Identity", m_Identifiers->currentIndex());
}

void AuCommAnnounce::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

QString AuCommAnnounce::MessageText() const
{
	// Collect information from the widgets.
	QString id;
	if(m_Identifiers)
		id = m_Identifiers->currentText();

	QString loc;
	if(m_Loca)
		loc = m_Loca->currentText();

	QString repair;
	if(m_Repairs)
		repair = m_Repairs->currentText();

	QString posture;
	if(m_Postures)
		posture = m_Postures->currentText();

	QString comment = m_Comments->text().trimmed();

	// Assemble the announcement message text.
	QString text;
	{
		if(!id.isEmpty())
			text = id;

		if(!loc.isEmpty())
		{
			if(!text.isEmpty())
				text += " ";
			text += loc;
		}

		if(!repair.isEmpty())
		{
			if(!text.isEmpty())
				text += " ";
			text += repair;
		}

		if(!posture.isEmpty())
		{
			if(!text.isEmpty())
				text += " ";
			text += posture;
		}

		if(!comment.isEmpty())
		{
			if(!text.isEmpty())
				text += " ";
			text += comment;
		}
	}

	return text;
}
