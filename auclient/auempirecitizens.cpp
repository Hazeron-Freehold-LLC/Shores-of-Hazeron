//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auempirecitizens.cpp,v 1.12 2022/05/04 21:28:10 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auempirecitizens.h"
//
#include "auapp.h"
#include "auframe.h"
#include "auglobal.h"
#include "augovassignoffice.h"
#include "augovchooseboss.h"
#include "aumailnew.h"
#include "auview.h"
#include "dagalaxy.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dauniverse.h"
#include "dgdata.h"
#include <QComboBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QTreeWidget>

void AuGovCitizensDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleGovernCitizensUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuEmpireCitizens::AuEmpireCitizens(QWidget* parent) : QWidget(parent)
{
	m_idPersonEmpire = 0;
	m_EmpireMailOk = false;
	m_bPersonHasDebt = false;

	pbSendMail = new QPushButton(this);
	pbSendMail->setText(tr("Send Mail"));
	pbSendMail->setToolTip(tr("<html><b>Send Mail</b><br>"
		"Sends a mail message to the selected person.</html>"));
	connect(pbSendMail, SIGNAL(clicked()), SLOT(SendMail()));

	pbSendEmpireMail = new QPushButton(this);
	pbSendEmpireMail->setText(tr("Send Empire Mail"));
	pbSendEmpireMail->setToolTip(tr("<html><b>Send Empire Mail</b><br>"
				"Sends a mail message to every person in your empire.</html>"));
	connect(pbSendEmpireMail, SIGNAL(clicked()), SLOT(SendEmpireMail()));

	pbSendGovernmentMail = new QPushButton(this);
	pbSendGovernmentMail->setText(tr("Send Government Mail"));
	pbSendGovernmentMail->setToolTip(tr("<html><b>Send Government Mail</b><br>"
				"Sends a mail message to every person in your empire's government.</html>"));
	connect(pbSendGovernmentMail, SIGNAL(clicked()), SLOT(SendGovernmentMail()));

	pbAssignPosition = new QPushButton(this);
	pbAssignPosition->setText(tr("Assign Office"));
	pbAssignPosition->setToolTip(tr("<html><b>Assign Office</b><br>"
				"Assigns a government office to the selected person.<br>"
				"<i>Tips</i><br>"
				"%1 Offices can only be assigned to people who do not already hold an office.<br>"
				"%1 Only governors with appropriate rights can assign offices.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(pbAssignPosition, SIGNAL(clicked()), SLOT(AssignPosition()));

	pbChangePosition = new QPushButton(this);
	pbChangePosition->setText(tr("Modify Office"));
	pbChangePosition->setToolTip(tr("<html><b>Modify Office</b><br>"
				"Changes the person's government office settings.<br>"
				"<i>Tips</i><br>"
				"%1 Offices can only be change on people who are in your chain of command.<br>"
				"%1 Only governors with appropriate rights can change offices.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(pbChangePosition, SIGNAL(clicked()), SLOT(ChangePosition()));

	pbViewPosition = new QPushButton(this);
	pbViewPosition->setText(tr("View Office"));
	pbViewPosition->setToolTip(tr("<html><b>View Office</b><br>"
				"Views the person's government office.<br>"
				"<i>Tips</i><br>"
				"%1 All citizens can view the offices of any governors.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(pbViewPosition, SIGNAL(clicked()), SLOT(ViewPosition()));

	pbChangeBoss = new QPushButton(this);
	pbChangeBoss->setText(tr("Change Government Boss"));
	pbChangeBoss->setToolTip(tr("<html><b>Change Government Boss</b><br>"
				"Changes the person's government boss in the chain of command.<br>"
				"<i>Tips</i><br>"
				"%1 Offices can only be change on people who are in your chain of command.<br>"
				"%1 Only governors with appropriate rights can change offices.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(pbChangeBoss, SIGNAL(clicked()), SLOT(ChangeBoss()));

	pbResignPosition = new QPushButton(this);
	pbResignPosition->setText(tr("Resign Office"));
	pbResignPosition->setToolTip(tr("<html><b>Resign Office</b><br>"
				"Resigns your office completely."
				"</html>"));
	connect(pbResignPosition, SIGNAL(clicked()), SLOT(ResignPosition()));

	pbRevokePosition = new QPushButton(this);
	pbRevokePosition->setText(tr("Revoke Office"));
	pbRevokePosition->setToolTip(tr("<html><b>Revoke Office</b><br>"
				"Revokes a person's office completely.<br>"
				"<i>Tips</i><br>"
				"%1 Only governors with appropriate rights can revoke offices.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(pbRevokePosition, SIGNAL(clicked()), SLOT(RevokePosition()));

	pbLeaveEmpire = new QPushButton(this);
	pbLeaveEmpire->setText(tr("Renouce Citizenship"));
	pbLeaveEmpire->setToolTip(tr("<html><p><b>Renouce Citizenship</b><br>"
				"Renounces your citizenship in your current empire. "
				"You will not be a member of any empire until you establish a new empire or join an existing empire. "
				"</p>"
				"<p>"
				"You will lose your berth held aboard a spacecraft unless it is a passenger berth. "
				"</p>"
				"<p>"
				"If you founded any cities, surrender them to your new empire before renouncing citizenship. "
				"</p>"
				"<p>"
				"<b>You cannot renounce your citizenship while you owe a debt to the banks.</b>"
				"</p>"
				"</html>"));
	connect(pbLeaveEmpire, SIGNAL(clicked()), SLOT(CitizenLeaveEmpire()));

	// Determine when the next election is scheduled.
	QDateTime electionSched;
	{
		// Calculate election day to be the next Saturday after today.
		QDate electionDay(QDate::currentDate());
		int dow = electionDay.dayOfWeek();						// 1=Monday, 7=Sunday
		int daysToSaturday = 6 - dow;
		if(daysToSaturday <= 0)
			daysToSaturday += 7;
		electionDay = electionDay.addDays(daysToSaturday);

		// Set election time to be 00:00Z.
		QTime electionTime(0, 0);

		// Build scheduled date/time in UTC.
		electionSched.setDate(electionDay);
		electionSched.setTime(electionTime);
		electionSched.setTimeSpec(Qt::UTC);
	}

	pbKick = new QPushButton(this);
	pbKick->setText(tr("Kick From Empire"));
	pbKick->setToolTip(tr("<html><b>Kick From Empire</b><br>"
				"Kicks the selected person from your empire.<br>"
				"<i>Tips</i><br>"
				"%1 Only governors with appropriate rights can kick citizens from the empire.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(pbKick, SIGNAL(clicked()), SLOT(Dismiss()));

	m_CitizensTree = new QTreeWidget(this);
	m_CitizensTree->setColumnCount(2);
	QStringList citizensHdr;
	citizensHdr << tr("Citizens");
	citizensHdr << tr("Id");
	m_CitizensTree->setHeaderLabels(citizensHdr);
	m_CitizensTree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	m_CitizensTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	connect(m_CitizensTree, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), SLOT(EnableControls()));

	gridLayout = new QGridLayout;
	gridLayout->setSpacing(2);
	gridLayout->setMargin(2);
	int row=0;
	gridLayout->addWidget(pbSendMail, row++,1);
	gridLayout->addWidget(pbSendEmpireMail, row++,1);
	gridLayout->addWidget(pbSendGovernmentMail, row++,1);
	row++;
	gridLayout->addWidget(pbAssignPosition, row++,1);
	gridLayout->addWidget(pbChangePosition, row++,1);
	gridLayout->addWidget(pbViewPosition, row++,1);
	gridLayout->addWidget(pbChangeBoss, row++,1);
	gridLayout->addWidget(pbRevokePosition, row++,1);
	gridLayout->addWidget(pbResignPosition, row++,1);
	row++;
	gridLayout->addWidget(pbLeaveEmpire, row++,1);
	gridLayout->addWidget(pbKick, row++,1);

	gridLayout->addWidget(m_CitizensTree, 0,0, row, 1);
	setLayout(gridLayout);
}

void AuEmpireCitizens::UpdateEmpireRecord()
{
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		m_idPerson = person->Id();
		m_idPersonEmpire = person->EmpireId();
		m_sPersonName = person->Name();
		m_bPersonHasDebt = person->LoanBalance() > 0;
		const DaSector* sector = person->FindParentSector();
		if(sector)
			m_idSector = sector->Id();
	}
	pView->UnlockSceneAtom();

	m_Empire.m_id = m_idPersonEmpire;
	const DgData& gd = g_Au.LockDataForRead();
	gd.Empire(m_Empire);
	g_Au.UnlockData();

	m_CitizensTree->clear();
	if(isVisible())
		FillCitizensTree(m_CitizensTree);

	EnableControls();
}

void AuEmpireCitizens::EnableControls()
{
	bool canSendGovMail = m_Empire.CitizenCanSendEmpireMail(m_idPerson);
	pbSendEmpireMail->setVisible(canSendGovMail);
	pbSendGovernmentMail->setVisible(canSendGovMail);
	pbLeaveEmpire->setEnabled(!m_bPersonHasDebt);

	const QTreeWidgetItem* citizen = m_CitizensTree->currentItem();
	if(citizen)
	{
		AuId idSelCitizen = AuId(citizen->data(0, Qt::UserRole).toUInt());

		bool canAssignPos = false;
		bool canChangePos = false;
		if(m_Empire.CitizenCanDelegate(m_idPerson))
		{
			if(m_Empire.IsGovernor(idSelCitizen))
				canChangePos = m_Empire.CitizenInChainOfCommand(m_idPerson, idSelCitizen);
			else
				canAssignPos = true;
		}

		// Anyone can view the permissions and domain of all government officials.
		bool canViewPos = m_Empire.IsGovernor(idSelCitizen);

		// A citizen is selected.
		pbAssignPosition->setEnabled(canAssignPos);
		pbChangePosition->setEnabled(canChangePos);
		pbViewPosition->setEnabled(canViewPos);
		pbChangeBoss->setEnabled(canChangePos);
		pbRevokePosition->setEnabled(canChangePos);

		// President and vice president can kick anyone from their empire.
		pbKick->setEnabled(m_Empire.CitizenCanKickCitizens(m_idPerson));

		// Can always send mail to fellow empire members.
		pbSendMail->setEnabled(true);
	}
	else
	{
		// No citizen is selected.
		pbAssignPosition->setEnabled(false);
		pbChangePosition->setEnabled(false);
		pbViewPosition->setEnabled(false);
		pbChangeBoss->setEnabled(false);
		pbRevokePosition->setEnabled(false);

		pbKick->setEnabled(false);
		pbSendMail->setEnabled(false);
	}

	pbResignPosition->setEnabled(m_Empire.IsGovernor(m_idPerson));
}

void AuEmpireCitizens::SendMail()
{
	const QTreeWidgetItem* citizen = m_CitizensTree->currentItem();
	if(!citizen)
		return;
	AuId idSelCitizen = AuId(citizen->data(0, Qt::UserRole).toUInt());
	if(idSelCitizen.IsNull())
		return;

	unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator itCitizen = m_Empire.m_Citizens.find(idSelCitizen);
	if(itCitizen == m_Empire.m_Citizens.end())
		return;

	QList<AuContact> qlContacts;
	AuContact contact(idSelCitizen, (*itCitizen).second.m_name);
	qlContacts.append(contact);

	AuMailNew* mail = new AuMailNew(this, m_idPerson, &qlContacts, (*itCitizen).second.m_name);
	mail->show();
}

void AuEmpireCitizens::SendEmpireMail()
{
	// Make a list of the recipients.
	AuContact contact;
	QList<AuContact> qlContacts;
	for(unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Empire.m_Citizens.begin(); ci != m_Empire.m_Citizens.end(); ci++)
	{
		contact = AuContact((*ci).first, (*ci).second.Name());
		qlContacts.append(contact);
	}

	AuMailNew* mail = new AuMailNew(this, m_idPerson, &qlContacts, tr("Empire"));
	mail->show();
}

void AuEmpireCitizens::SendGovernmentMail()
{
	// Make a list of the recipients.
	AuContact contact;
	QList<AuContact> qlContacts;
	for(unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Empire.m_Citizens.begin(); ci != m_Empire.m_Citizens.end(); ci++)
	{
		if(!(*ci).second.HasOffice())
			continue;
		contact = AuContact((*ci).first, (*ci).second.Name());
		qlContacts.append(contact);
	}

	AuMailNew* mail = new AuMailNew(this, m_idPerson, &qlContacts, tr("Empire"));
	mail->show();
}

void AuEmpireCitizens::AssignPosition()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;

	const QTreeWidgetItem* citizen = m_CitizensTree->currentItem();
	if(!citizen)
		return;
	AuId idSelCitizen = AuId(citizen->data(0, Qt::UserRole).toUInt());
	if(idSelCitizen.IsNull())
		return;

	AuGovAssignOffice dlg(m_idPersonEmpire, m_idPerson, idSelCitizen, pFrame);
	auApp->Frame()->ExecDialog(dlg);
}

void AuEmpireCitizens::ChangePosition()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;

	const QTreeWidgetItem* citizen = m_CitizensTree->currentItem();
	if(!citizen)
		return;
	AuId idSelCitizen = AuId(citizen->data(0, Qt::UserRole).toUInt());
	if(idSelCitizen.IsNull())
		return;

	AuGovAssignOffice dlg(m_idPersonEmpire, idSelCitizen, idSelCitizen, pFrame);
	auApp->Frame()->ExecDialog(dlg);
}

void AuEmpireCitizens::ViewPosition()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;

	const QTreeWidgetItem* citizen = m_CitizensTree->currentItem();
	if(!citizen)
		return;
	AuId idSelCitizen = AuId(citizen->data(0, Qt::UserRole).toUInt());
	if(idSelCitizen.IsNull())
		return;

	// Null assigner is used to trigger "view" mode.
	AuGovAssignOffice dlg(m_idPersonEmpire, AuId(0), idSelCitizen, pFrame);
	auApp->Frame()->ExecDialog(dlg);
}

void AuEmpireCitizens::ChangeBoss()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;

	const QTreeWidgetItem* citizen = m_CitizensTree->currentItem();
	if(!citizen)
		return;
	AuId idSelCitizen = AuId(citizen->data(0, Qt::UserRole).toUInt());
	if(idSelCitizen.IsNull())
		return;

	AuGovChooseBoss dlg(m_Empire, idSelCitizen, pFrame);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
		auApp->GovSetBoss(idSelCitizen, dlg.Boss());
}

void AuEmpireCitizens::Dismiss()
{
	const QTreeWidgetItem* citizen = m_CitizensTree->currentItem();
	if(!citizen)
		return;
	AuId idSelCitizen = AuId(citizen->data(0, Qt::UserRole).toUInt());
	if(idSelCitizen.IsNull())
		return;

	unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator itCitizen = m_Empire.m_Citizens.find(idSelCitizen);
	if(itCitizen != m_Empire.m_Citizens.end())
	{
		QString sTitle = tr("Kick From Empire");
		int iResult = QMessageBox::warning(this, sTitle, tr("<html>%1 will be kicked from your empire. "
					"%1 will not be killed or forced to respawn.<br><br>"
					"Are you sure you want to kick %1 from your empire?</html>").arg((*itCitizen).second.m_name),
				QMessageBox::Yes, QMessageBox::No);
		if(iResult != QMessageBox::Yes)
			return;
		auApp->KickOrLeaveEmpire((*itCitizen).second.m_id, (*itCitizen).second.m_name, m_Empire.m_id);
	}
}

void AuEmpireCitizens::CitizenLeaveEmpire()
{
	QString smsg = tr("<html>"
			"Confirm renunciation of citizenship. "
			"<br>"
			"%1 You will no longer be a member of any empire. "
			"<br>"
			"%1 You will lose your berth unless it is a passenger berth. "
			"</html>").arg(g_Au.MiddleDotSymbol());

	QMessageBox mb(g_Au.ApplicationName(), smsg, QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel,
			QMessageBox::NoButton, auApp->Frame());
	if(mb.exec() != QMessageBox::Ok)
		return;

	AuView* pView = auApp->Frame()->View();
	AuId sceneId;
	QString sceneName;
	quint32 sceneEmpire = 0;
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		sceneId = person->Id();
		sceneName = person->Name();
		sceneEmpire = person->EmpireId();
	}
	pView->UnlockSceneAtom();
	auApp->KickOrLeaveEmpire(sceneId, sceneName, sceneEmpire);
}

void AuEmpireCitizens::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	UpdateEmpireRecord();
}

void AuEmpireCitizens::ResignPosition()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;

	const DgData& gd = g_Au.LockDataForRead();
	bool isGov = gd.IsEmpireGovernor(m_Empire.m_id, m_idPerson);
	g_Au.UnlockData();

	if(!isGov)
		return;

	QString smsg = tr("Your office will be resigned.");
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Critical, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, auApp->Frame());
	if(auApp->Frame()->ExecMessageBox(mb) == QMessageBox::Ok)
		auApp->ResignOffice();
}

void AuEmpireCitizens::RevokePosition()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;

	const QTreeWidgetItem* citizen = m_CitizensTree->currentItem();
	if(!citizen)
		return;
	AuId idSelCitizen = AuId(citizen->data(0, Qt::UserRole).toUInt());
	if(idSelCitizen.IsNull())
		return;

	unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator itCitizen = m_Empire.m_Citizens.find(idSelCitizen);
	if(itCitizen != m_Empire.m_Citizens.end() && (*itCitizen).second.HasOffice())
	{
		// Found the citizen and they hold an office.
		QString smsg = tr("The office held by %1 will be revoked.").arg((*itCitizen).second.NameWithTitle());
		QMessageBox mb(g_Au.ApplicationName(), smsg,
				QMessageBox::Critical, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, auApp->Frame());
		if(auApp->Frame()->ExecMessageBox(mb) == QMessageBox::Ok)
			auApp->RevokeOffice((*itCitizen).first, (*itCitizen).second.Name());
	}
}

void AuEmpireCitizens::FillCitizensTree(QTreeWidget* tree)
{
	// Add items for the government.
	AuId emperorId(m_Empire.EmperorId());
	QSet<AuId> alreadyListed;
	if(!emperorId.IsNull())
	{
		QTreeWidgetItem* emperor = new QTreeWidgetItem(tree);
		emperor->setExpanded(true);
		emperor->setIcon(0, QIcon(":data/CommGovernment.png"));
		emperor->setText(0, m_Empire.CitizenNameWithTitle(emperorId));
		emperor->setData(0, Qt::UserRole, emperorId.GetID());
		emperor->setText(1, emperorId.StringId());
		alreadyListed.insert(emperorId);

		AddSubordinates(emperor, alreadyListed);

		// List governors with broken trees. They are the ones who are left.
		QMultiMap<QString, AuId> govStragglers;
		for(unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Empire.m_Citizens.cbegin();
				ci != m_Empire.m_Citizens.cend(); ci++)
			if(!alreadyListed.contains((*ci).first) && (*ci).second.HasOffice())
				govStragglers.insert((*ci).second.NameWithTitle(), (*ci).first);
		for(QMultiMap<QString, AuId>::const_iterator si = govStragglers.cbegin(); si != govStragglers.cend(); si++)
		{
			QTreeWidgetItem* citizen = new QTreeWidgetItem(emperor);
			citizen->setIcon(0, QIcon(":data/CommGovernment.png"));
			citizen->setText(0, si.key());
			citizen->setData(0, Qt::UserRole, (*si).GetID());
			citizen->setText(1, (*si).StringId());

			alreadyListed.insert(*si);
		}
	}

	// Add entries for remaining citizens.
	QMultiMap<QString, AuId> citizens;
	for(unordered_map<AuId, AuCitizen, AuIdHash>::const_iterator ci = m_Empire.m_Citizens.cbegin();
			ci != m_Empire.m_Citizens.cend(); ci++)
		if(!alreadyListed.contains((*ci).first))
			citizens.insert((*ci).second.Name(), (*ci).first);
	for(QMultiMap<QString, AuId>::const_iterator si = citizens.cbegin(); si != citizens.cend(); si++)
	{
		QTreeWidgetItem* citizen = new QTreeWidgetItem(tree);
		citizen->setIcon(0, QIcon(":data/Passenger.png"));
		citizen->setText(0, si.key());
		citizen->setData(0, Qt::UserRole, (*si).GetID());
		citizen->setText(1, (*si).StringId());
	}

	//m_CitizensTree->resizeColumnToContents(0);
	//m_CitizensTree->resizeColumnToContents(1);
}

// Recursive
void AuEmpireCitizens::AddSubordinates(QTreeWidgetItem* boss, QSet<AuId>& alreadyListed)
{
	// Get the list of names and ids of all citizens who are subordinate to the boss.
	AuId bossId = AuId(boss->data(0, Qt::UserRole).toUInt());
	QMultiMap<QString, AuId> subs;
	m_Empire.Subordinates(bossId, subs);

	// Create a tree entry under boss for each citizen.
	for(QMultiMap<QString, AuId>::const_iterator si = subs.cbegin(); si != subs.cend(); si++)
	{
		if(alreadyListed.contains(*si))
			continue;

		QTreeWidgetItem* sub = new QTreeWidgetItem(boss);
		sub->setExpanded(true);
		sub->setIcon(0, QIcon(":data/CommGovernment.png"));
		sub->setText(0, si.key());
		sub->setData(0, Qt::UserRole, (*si).GetID());
		sub->setText(1, (*si).StringId());
		alreadyListed.insert(*si);

		// Add the citizen's subordinates, if any.
		AddSubordinates(sub, alreadyListed);
	}
}

QSize AuEmpireCitizens::sizeHint() const
{
	return QSize(600,800);
}
