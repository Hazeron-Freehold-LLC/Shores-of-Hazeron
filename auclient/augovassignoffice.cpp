//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovassignoffice.cpp,v 1.11 2022/02/19 18:10:54 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augovassignoffice.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "augovchooseplace.h"
#include "auframe.h"
#include "auview.h"
#include "daperson.h"
#include "dasector.h"
#include "dgdata.h"
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QScrollArea>
#include <QTreeWidget>
#include <QVBoxLayout>

// Null assigner indicates 'view' mode.
AuGovAssignOffice::AuGovAssignOffice(quint32 empireId, const AuId& assigner, const AuId& assignee, QWidget* parent) :
	AuDialog(parent)
{
	Q_ASSERT(!assignee.IsNull());
	m_Assignee = assignee;

	m_Empire.m_id = empireId;
	const DgData& gd = g_Au.LockDataForRead();
	gd.Empire(m_Empire);
	g_Au.UnlockData();

	bool isEditMode = true;
	if(assigner.IsNull())
	{
		// When assigner is null, we are viewing settings on an existing office.
		// Initialize my settings with the office of the assignee.
		setAuDlgTitle(tr("View Office of %1").arg(m_Empire.CitizenNameWithTitle(assignee)));
		m_Office = m_Empire.CitizenOffice(assignee);
		isEditMode = false;
	}
	else if(assigner == assignee)
	{
		// When assigner == assignee, we are changing settings on an existing office.
		// Initialize my settings with the office of the assignee.
		setAuDlgTitle(tr("Change Office of %1").arg(m_Empire.CitizenNameWithTitle(assignee)));
		m_Office = m_Empire.CitizenOffice(assignee);
	}
	else
	{
		// When assigner != assignee we are creating a new office assignment.
		// Initialize my settings with the office of the assigner.
		setAuDlgTitle(tr("Assign Office to %1").arg(m_Empire.CitizenName(assignee)));
		m_Office = m_Empire.CitizenOffice(assigner);
		m_Office.SetBossId(assigner);
	}
	setAuDlgIcon(QIcon(":data/CommGovernment.png"));

	QLabel* lbTitle = new QLabel(tr("Title of Office"), this);

	QComboBox* cbTitle = new QComboBox(this);
	cbTitle->setEditable(true);
	cbTitle->addItem(tr("Arch Bishop"));
	cbTitle->addItem(tr("Baron"));
	cbTitle->addItem(tr("Baroness"));
	cbTitle->addItem(tr("Bishop"));
	cbTitle->addItem(tr("Cardinal"));
	cbTitle->addItem(tr("Congressman"));
	cbTitle->addItem(tr("Congresswoman"));
	cbTitle->addItem(tr("Count"));
	cbTitle->addItem(tr("Countess"));
	cbTitle->addItem(tr("Crown Prince"));
	cbTitle->addItem(tr("Crown Princess"));
	cbTitle->addItem(tr("Czar"));
	cbTitle->addItem(tr("Deacon"));
	cbTitle->addItem(tr("Deaconess"));
	cbTitle->addItem(tr("Duchess"));
	cbTitle->addItem(tr("Duke"));
	cbTitle->addItem(tr("Emperor"));
	cbTitle->addItem(tr("Empress"));
	cbTitle->addItem(tr("Governor"));
	cbTitle->addItem(tr("King"));
	cbTitle->addItem(tr("Marquesa"));
	cbTitle->addItem(tr("Marquis"));
	cbTitle->addItem(tr("Mayor"));
	cbTitle->addItem(tr("Minister"));
	cbTitle->addItem(tr("Pope"));
	cbTitle->addItem(tr("Prefect"));
	cbTitle->addItem(tr("President"));
	cbTitle->addItem(tr("Priest"));
	cbTitle->addItem(tr("Priestess"));
	cbTitle->addItem(tr("Prime Minister"));
	cbTitle->addItem(tr("Prince"));
	cbTitle->addItem(tr("Princess"));
	cbTitle->addItem(tr("Queen"));
	cbTitle->addItem(tr("Senator"));
	cbTitle->addItem(tr("Shaman"));
	cbTitle->addItem(tr("Vice President"));
	cbTitle->addItem(tr("Viceroy"));
	cbTitle->setEditText(m_Office.TitleOfOffice());
	cbTitle->setEnabled(isEditMode);
	connect(cbTitle, SIGNAL(editTextChanged(const QString&)), SLOT(TitleOfOfficeChanged(const QString&)));

	QCheckBox* ckCanAcceptCitySurrender = new QCheckBox(tr("Can accept city surrender."), this);
	ckCanAcceptCitySurrender->setCheckState(m_Office.m_CanAcceptCitySurrender ? Qt::Checked : Qt::Unchecked);
	ckCanAcceptCitySurrender->setEnabled(isEditMode);
	connect(ckCanAcceptCitySurrender, SIGNAL(clicked()), SLOT(ToggleCanAcceptCitySurrender()));

	QCheckBox* ckCanDelegatePermissions = new QCheckBox(tr("Can assign and change government offices, including emperor. "), this);
	ckCanDelegatePermissions->setCheckState(m_Office.m_CanDelegatePermissions ? Qt::Checked : Qt::Unchecked);
	ckCanDelegatePermissions->setEnabled(isEditMode);
	connect(ckCanDelegatePermissions, SIGNAL(clicked()), SLOT(ToggleCanDelegatePermissions()));

	QCheckBox* ckCanInvite = new QCheckBox(tr("Can invite citizens to the empire."), this);
	ckCanInvite->setCheckState(m_Office.m_CanInvite ? Qt::Checked : Qt::Unchecked);
	ckCanInvite->setEnabled(isEditMode);
	connect(ckCanInvite, SIGNAL(clicked()), SLOT(ToggleCanInvite()));

	QCheckBox* ckCanKick = new QCheckBox(tr("Can kick citizens from the empire."), this);
	ckCanKick->setCheckState(m_Office.m_CanKick ? Qt::Checked : Qt::Unchecked);
	ckCanKick->setEnabled(isEditMode);
	connect(ckCanKick, SIGNAL(clicked()), SLOT(ToggleCanKick()));

	QCheckBox* ckCanSetColorTheme = new QCheckBox(tr("Can change color theme."), this);
	ckCanSetColorTheme->setCheckState(m_Office.m_CanSetColorTheme ? Qt::Checked : Qt::Unchecked);
	ckCanSetColorTheme->setEnabled(isEditMode);
	connect(ckCanSetColorTheme, SIGNAL(clicked()), SLOT(ToggleCanSetColorTheme()));

	QCheckBox* ckCanManageDossier = new QCheckBox(tr("Can change dossier on known empires."), this);
	ckCanManageDossier->setCheckState(m_Office.m_CanManageDossier ? Qt::Checked : Qt::Unchecked);
	ckCanManageDossier->setEnabled(isEditMode);
	connect(ckCanManageDossier, SIGNAL(clicked()), SLOT(ToggleCanManageDossier()));

	QCheckBox* ckCanManageTrade = new QCheckBox(tr("Can change broker shipments and negotiate trade agreements at cities in their domain."), this);
	ckCanManageTrade->setCheckState(m_Office.m_CanManageTrade ? Qt::Checked : Qt::Unchecked);
	ckCanManageTrade->setEnabled(isEditMode);
	connect(ckCanManageTrade, SIGNAL(clicked()), SLOT(ToggleCanManageTrade()));

	QCheckBox* ckCanSetAnnouncements = new QCheckBox(tr("Can change announcements."), this);
	ckCanSetAnnouncements->setCheckState(m_Office.m_CanSetAnnouncements ? Qt::Checked : Qt::Unchecked);
	ckCanSetAnnouncements->setEnabled(isEditMode);
	connect(ckCanSetAnnouncements, SIGNAL(clicked()), SLOT(ToggleCanSetAnnouncements()));

	QCheckBox* ckCanSetCapital = new QCheckBox(tr("Can declare empire capital."), this);
	ckCanSetCapital->setCheckState(m_Office.m_CanSetCapital ? Qt::Checked : Qt::Unchecked);
	ckCanSetCapital->setEnabled(isEditMode);
	connect(ckCanSetCapital, SIGNAL(clicked()), SLOT(ToggleCanSetCapital()));

	QCheckBox* ckCanSetCommerce = new QCheckBox(tr("Can change commerce settings."), this);
	ckCanSetCommerce->setCheckState(m_Office.m_CanSetCommerce ? Qt::Checked : Qt::Unchecked);
	ckCanSetCommerce->setEnabled(isEditMode);
	connect(ckCanSetCommerce, SIGNAL(clicked()), SLOT(ToggleCanSetCommerce()));

	QCheckBox* ckCanSetContrailColor = new QCheckBox(tr("Can change contrail color."), this);
	ckCanSetContrailColor->setCheckState(m_Office.m_CanSetContrailColor ? Qt::Checked : Qt::Unchecked);
	ckCanSetContrailColor->setEnabled(isEditMode);
	connect(ckCanSetContrailColor, SIGNAL(clicked()), SLOT(ToggleCanSetContrailColor()));

	QCheckBox* ckCanSetEmpireFlag = new QCheckBox(tr("Can change empire flag."), this);
	ckCanSetEmpireFlag->setCheckState(m_Office.m_CanSetEmpireFlag ? Qt::Checked : Qt::Unchecked);
	ckCanSetEmpireFlag->setEnabled(isEditMode);
	connect(ckCanSetEmpireFlag, SIGNAL(clicked()), SLOT(ToggleCanSetEmpireFlag()));

	QCheckBox* ckCanSetJoinPass = new QCheckBox(tr("Can change join password."), this);
	ckCanSetJoinPass->setCheckState(m_Office.m_CanSetJoinPassword ? Qt::Checked : Qt::Unchecked);
	ckCanSetJoinPass->setEnabled(isEditMode);
	connect(ckCanSetJoinPass, SIGNAL(clicked()), SLOT(ToggleCanSetJoinPassword()));

	QCheckBox* ckCanSetPoliticalStance =
		new QCheckBox(tr("Can change default political stance and political stance toward known empires."), this);
	ckCanSetPoliticalStance->setCheckState(m_Office.m_CanSetPoliticalStance ? Qt::Checked : Qt::Unchecked);
	ckCanSetPoliticalStance->setEnabled(isEditMode);
	connect(ckCanSetPoliticalStance, SIGNAL(clicked()), SLOT(ToggleCanSetPoliticalStance()));

	QCheckBox* ckCanSetTax = new QCheckBox(tr("Can change taxes."), this);
	ckCanSetTax->setCheckState(m_Office.m_CanSetTax ? Qt::Checked : Qt::Unchecked);
	ckCanSetTax->setEnabled(isEditMode);
	connect(ckCanSetTax, SIGNAL(clicked()), SLOT(ToggleCanSetTax()));

	/*
	QCheckBox* ckCanSetTitle = new QCheckBox(tr("Can change their own title of office."), this);
	ckCanSetTitle->setCheckState(m_Office.m_CanSetTitle ? Qt::Checked : Qt::Unchecked);
	ckCanSetTitle->setEnabled(isEditMode);
	connect(ckCanSetTitle, SIGNAL(clicked()), SLOT(ToggleCanSetTitle()));
	*/

	QCheckBox* ckCanSetWarPolicy = new QCheckBox(tr("Can change war policy."), this);
	ckCanSetWarPolicy->setCheckState(m_Office.m_CanSetWarCriteria ? Qt::Checked : Qt::Unchecked);
	ckCanSetWarPolicy->setEnabled(isEditMode);
	connect(ckCanSetWarPolicy, SIGNAL(clicked()), SLOT(ToggleCanSetWarPolicy()));

	QCheckBox* ckCanUseDiplomacyChannel = new QCheckBox(tr("Can communicate on the Diplomacy comm channel."), this);
	ckCanUseDiplomacyChannel->setCheckState(m_Office.m_CanUseDiplomacyChannel ? Qt::Checked : Qt::Unchecked);
	ckCanUseDiplomacyChannel->setEnabled(isEditMode);
	connect(ckCanUseDiplomacyChannel, SIGNAL(clicked()), SLOT(ToggleCanUseDiplomacyChannel()));

	QCheckBox* ckCanSendEmpireMail = new QCheckBox(tr("Can send mail to empire members using Governance window."), this);
	ckCanSendEmpireMail->setCheckState(m_Office.m_CanSendEmpireMail ? Qt::Checked : Qt::Unchecked);
	ckCanSendEmpireMail->setEnabled(isEditMode);
	connect(ckCanSendEmpireMail, SIGNAL(clicked()), SLOT(ToggleCanSendEmpireMail()));

	QCheckBox* ckCanAccessGovtAccount = new QCheckBox(tr("Can access government account at banks in their domain."), this);
	ckCanAccessGovtAccount->setCheckState(m_Office.m_CanAccessGovtAccount ? Qt::Checked : Qt::Unchecked);
	ckCanAccessGovtAccount->setEnabled(isEditMode);
	connect(ckCanAccessGovtAccount, SIGNAL(clicked()), SLOT(ToggleCanAccessGovtAccount()));

	QCheckBox* ckCanBuild = new QCheckBox(tr("Can construct buildings and roads at cities in their domain."), this);
	ckCanBuild->setCheckState(m_Office.m_CanBuild ? Qt::Checked : Qt::Unchecked);
	ckCanBuild->setEnabled(isEditMode);
	connect(ckCanBuild, SIGNAL(clicked()), SLOT(ToggleCanBuild()));

	QCheckBox* ckCanLoadOfficers = new QCheckBox(tr("Can load officers from worlds in their domain."), this);
	ckCanLoadOfficers->setCheckState(m_Office.m_CanLoadOfficers ? Qt::Checked : Qt::Unchecked);
	ckCanLoadOfficers->setEnabled(isEditMode);
	connect(ckCanLoadOfficers, SIGNAL(clicked()), SLOT(ToggleCanLoadOfficers()));

	QCheckBox* ckCanSetMfg = new QCheckBox(tr("Can change manufacturing settings at cities in their domain."), this);
	ckCanSetMfg->setCheckState(m_Office.m_CanSetMfg ? Qt::Checked : Qt::Unchecked);
	ckCanSetMfg->setEnabled(isEditMode);
	connect(ckCanSetMfg, SIGNAL(clicked()), SLOT(ToggleCanSetMfg()));

	QCheckBox* ckCanSetMfgSpacecraft = new QCheckBox(tr("Can change spacecraft manufacturing settings at cities in their domain."), this);
	ckCanSetMfgSpacecraft->setCheckState(m_Office.m_CanSetMfgSpacecraft ? Qt::Checked : Qt::Unchecked);
	ckCanSetMfgSpacecraft->setEnabled(isEditMode);
	connect(ckCanSetMfgSpacecraft, SIGNAL(clicked()), SLOT(ToggleCanSetMfgSpacecraft()));

	QCheckBox* ckCanSetPlaceNames = new QCheckBox(tr("Can change names of places in their domain."), this);
	ckCanSetPlaceNames->setCheckState(m_Office.m_CanSetPlaceNames ? Qt::Checked : Qt::Unchecked);
	ckCanSetPlaceNames->setEnabled(isEditMode);
	connect(ckCanSetPlaceNames, SIGNAL(clicked()), SLOT(ToggleCanSetPlaceNames()));

	QCheckBox* ckCanSetResearch = new QCheckBox(tr("Can change research settings at Universities in their domain."), this);
	ckCanSetResearch->setCheckState(m_Office.m_CanSetResearch ? Qt::Checked : Qt::Unchecked);
	ckCanSetResearch->setEnabled(isEditMode);
	connect(ckCanSetResearch, SIGNAL(clicked()), SLOT(ToggleCanSetResearch()));

	QCheckBox* ckCanSetGalaxyCapital = new QCheckBox(tr("Can declare galaxy capital in galaxies in their domain."), this);
	ckCanSetGalaxyCapital->setCheckState(m_Office.m_CanSetGalaxyCapital ? Qt::Checked : Qt::Unchecked);
	ckCanSetGalaxyCapital->setEnabled(isEditMode);
	connect(ckCanSetGalaxyCapital, SIGNAL(clicked()), SLOT(ToggleCanSetGalaxyCapital()));

	QCheckBox* ckCanSetSectorCapital = new QCheckBox(tr("Can declare sector capital in sectors in their domain."), this);
	ckCanSetSectorCapital->setCheckState(m_Office.m_CanSetSectorCapital ? Qt::Checked : Qt::Unchecked);
	ckCanSetSectorCapital->setEnabled(isEditMode);
	connect(ckCanSetSectorCapital, SIGNAL(clicked()), SLOT(ToggleCanSetSectorCapital()));

	QCheckBox* ckRxFleetMessages = new QCheckBox(tr("Can communicate on the Fleet comm channel and "
				"receives fleet mail messages from officers from their domain."), this);
	ckRxFleetMessages->setCheckState(m_Office.m_RxFleetMessages ? Qt::Checked : Qt::Unchecked);
	ckRxFleetMessages->setEnabled(isEditMode);
	connect(ckRxFleetMessages, SIGNAL(clicked()), SLOT(ToggleRxFleetMessages()));

	QCheckBox* ckRxSensorScans = new QCheckBox(tr("Receives sensor scan report mail messages from spacecraft commanded by officers from their domain."), this);
	ckRxSensorScans->setCheckState(m_Office.m_RxSensorScans ? Qt::Checked : Qt::Unchecked);
	ckRxSensorScans->setEnabled(isEditMode);
	connect(ckRxSensorScans, SIGNAL(clicked()), SLOT(ToggleRxSensorScans()));

	QLabel* lbDomain = new QLabel(tr("Governance Domain"));

	m_DomainList = new QListWidget(this);
	m_DomainList->setToolTip(tr("Places in the governor's domain."));
	m_DomainList->setMinimumHeight(10);
	m_DomainList->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_DomainList->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_DomainList->setSelectionMode(QAbstractItemView::ExtendedSelection);
	connect(m_DomainList, SIGNAL(itemSelectionChanged()), SLOT(DomainSelectionChanged()));

	// List their governance domain.
	for(AuOfficeDomain::const_iterator di = m_Office.Domain().cbegin(); di != m_Office.Domain().cend(); di++)
	{
		QListWidgetItem* lwi = new QListWidgetItem(QIcon((*di)->Icon()), (*di)->Name(), m_DomainList);
		lwi->setData(Qt::UserRole, (*di)->AtomId().GetID());
	}

	QVBoxLayout* dombutlay = 0;
	if(isEditMode)
	{
		m_AddButton = new QPushButton(tr("Add Place"), this);
		m_AddButton->setToolTip(tr("<html>Adds a place to the governance domain.</html>"));
		connect(m_AddButton, SIGNAL(clicked()), SLOT(AddDomain()));

		m_RemoveButton = new QPushButton(tr("Remove Place"), this);
		m_RemoveButton->setToolTip(tr("<html>Removes the selected place from the governance domain.</html>"));
		m_RemoveButton->setEnabled(false);
		connect(m_RemoveButton, SIGNAL(clicked()), SLOT(RemoveDomain()));

		dombutlay = new QVBoxLayout;
		dombutlay->addWidget(m_AddButton);
		dombutlay->addWidget(m_RemoveButton);
		dombutlay->addStretch();
	}
	else
	{
		m_AddButton = 0;
		m_RemoveButton = 0;
	}


	QHBoxLayout* domlay = new QHBoxLayout;
	domlay->addWidget(m_DomainList);
	if(dombutlay)
		domlay->addLayout(dombutlay);

	QHBoxLayout* laytit = new QHBoxLayout;
	laytit->addWidget(lbTitle);
	laytit->addWidget(cbTitle);

	// Empire context settings.
	QLabel* lbemp = new QLabel(tr("These rights apply regardless of the office holder's location."));
	QWidget* wemp = new QWidget();
	{
		QVBoxLayout* lay = new QVBoxLayout;
		lay->addLayout(laytit);
		lay->addWidget(ckCanAcceptCitySurrender);
		lay->addWidget(ckCanDelegatePermissions);
		lay->addWidget(ckCanSetAnnouncements);
		lay->addWidget(ckCanSetColorTheme);
		lay->addWidget(ckCanSetCommerce);
		lay->addWidget(ckCanSetContrailColor);
		lay->addWidget(ckCanSetPoliticalStance);
		lay->addWidget(ckCanManageDossier);
		lay->addWidget(ckCanSetEmpireFlag);
		lay->addWidget(ckCanSetJoinPass);
		lay->addWidget(ckCanSetTax);
		//lay->addWidget(ckCanSetTitle);
		lay->addWidget(ckCanSetWarPolicy);
		lay->addWidget(ckCanUseDiplomacyChannel);
		lay->addWidget(ckCanSetCapital);
		lay->addWidget(ckCanInvite);
		lay->addWidget(ckCanKick);
		lay->addWidget(ckCanSendEmpireMail);
		lay->setMargin(2);
		lay->setSpacing(2);
		wemp->setLayout(lay);
	}
	QScrollArea* empscroll = new QScrollArea;
	empscroll->setWidget(wemp);

	// Domain context settings.
	QLabel* lbdomain = new QLabel(tr("These rights apply at places in the office holder's domain."));
	lbdomain->setWordWrap(true);
	QWidget* wdomain = new QWidget();
	{
		QVBoxLayout* lay = new QVBoxLayout;
		lay->addWidget(ckCanAccessGovtAccount);
		lay->addWidget(ckCanBuild);
		lay->addWidget(ckCanLoadOfficers);
		lay->addWidget(ckCanManageTrade);
		lay->addWidget(ckCanSetMfg);
		lay->addWidget(ckCanSetPlaceNames);
		lay->addWidget(ckCanSetResearch);
		lay->addWidget(ckCanSetMfgSpacecraft);
		lay->addWidget(ckCanSetGalaxyCapital);
		lay->addWidget(ckCanSetSectorCapital);
		lay->addWidget(ckRxFleetMessages);
		lay->addWidget(ckRxSensorScans);
		lay->addWidget(lbDomain);
		lay->addLayout(domlay);
		lay->setMargin(2);
		lay->setSpacing(2);
		wdomain->setLayout(lay);
	}
	QScrollArea* domainscroll = new QScrollArea;
	domainscroll->setWidget(wdomain);

	QHBoxLayout* layok = 0;
	if(isEditMode)
	{
		m_OkButton = new QPushButton(tr("Ok"), this);
		m_OkButton->setAutoDefault(true);
		m_OkButton->setDefault(true);
		m_OkButton->setToolTip(tr("<html>Accepts the changes to the office rights.</html>"));
		connect(m_OkButton, SIGNAL(clicked()), SLOT(AssignOffice()));

		layok = new QHBoxLayout;
		layok->addStretch();
		layok->addWidget(m_OkButton);
	}
	else
		m_OkButton = 0;

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbemp);
	lay->addWidget(empscroll);
	lay->addWidget(lbdomain);
	lay->addWidget(domainscroll);
	if(layok)
		lay->addLayout(layok);

	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuGovAssignOffice::~AuGovAssignOffice()
{
}

void AuGovAssignOffice::setVisible(bool visible)
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

void AuGovAssignOffice::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/AuGovAssignOffice/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/AuGovAssignOffice/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/AuGovAssignOffice/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/AuGovAssignOffice/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuGovAssignOffice::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/AuGovAssignOffice/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/AuGovAssignOffice/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/AuGovAssignOffice/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/AuGovAssignOffice/SizeY",sz.height());
}

void AuGovAssignOffice::AssignOffice()
{
	auApp->AssignOffice(m_Assignee, m_Empire.CitizenName(m_Assignee), m_Office);
	accept();
}

void AuGovAssignOffice::TitleOfOfficeChanged(const QString& title)
{
	m_Office.m_Title = title.trimmed();
	if(m_OkButton)
		m_OkButton->setEnabled(IsNameValid(m_Office.m_Title));
}

void AuGovAssignOffice::ToggleCanAcceptCitySurrender()
{
	m_Office.m_CanAcceptCitySurrender = !m_Office.m_CanAcceptCitySurrender;
}

void AuGovAssignOffice::ToggleCanAccessGovtAccount()
{
	m_Office.m_CanAccessGovtAccount = !m_Office.m_CanAccessGovtAccount;
}

void AuGovAssignOffice::ToggleCanBuild()
{
	m_Office.m_CanBuild = !m_Office.m_CanBuild;
}

void AuGovAssignOffice::ToggleCanDelegatePermissions()
{
	m_Office.m_CanDelegatePermissions = !m_Office.m_CanDelegatePermissions;
}

void AuGovAssignOffice::ToggleCanInvite()
{
	m_Office.m_CanInvite = !m_Office.m_CanInvite;
}

void AuGovAssignOffice::ToggleCanKick()
{
	m_Office.m_CanKick = !m_Office.m_CanKick;
}

void AuGovAssignOffice::ToggleCanLoadOfficers()
{
	m_Office.m_CanLoadOfficers = !m_Office.m_CanLoadOfficers;
}

void AuGovAssignOffice::ToggleCanSetColorTheme()
{
	m_Office.m_CanSetColorTheme = !m_Office.m_CanSetColorTheme;
}

void AuGovAssignOffice::ToggleCanManageDossier()
{
	m_Office.m_CanManageDossier = !m_Office.m_CanManageDossier;
}

void AuGovAssignOffice::ToggleCanManageTrade()
{
	m_Office.m_CanManageTrade = !m_Office.m_CanManageTrade;
}

void AuGovAssignOffice::ToggleCanSetAnnouncements()
{
	m_Office.m_CanSetAnnouncements = !m_Office.m_CanSetAnnouncements;
}

void AuGovAssignOffice::ToggleCanSetWarPolicy()
{
	m_Office.m_CanSetWarCriteria = !m_Office.m_CanSetWarCriteria;
}

void AuGovAssignOffice::ToggleCanSetCapital()
{
	m_Office.m_CanSetCapital = !m_Office.m_CanSetCapital;
}

void AuGovAssignOffice::ToggleCanSetCommerce()
{
	m_Office.m_CanSetCommerce = !m_Office.m_CanSetCommerce;
}

void AuGovAssignOffice::ToggleCanSetContrailColor()
{
	m_Office.m_CanSetContrailColor = !m_Office.m_CanSetContrailColor;
}

void AuGovAssignOffice::ToggleCanSetEmpireFlag()
{
	m_Office.m_CanSetEmpireFlag = !m_Office.m_CanSetEmpireFlag;
}

void AuGovAssignOffice::ToggleCanSetJoinPassword()
{
	m_Office.m_CanSetJoinPassword = !m_Office.m_CanSetJoinPassword;
}

void AuGovAssignOffice::ToggleCanSetMfg()
{
	m_Office.m_CanSetMfg = !m_Office.m_CanSetMfg;
}

void AuGovAssignOffice::ToggleCanSetMfgSpacecraft()
{
	m_Office.m_CanSetMfgSpacecraft = !m_Office.m_CanSetMfgSpacecraft;
}

void AuGovAssignOffice::ToggleCanSetPlaceNames()
{
	m_Office.m_CanSetPlaceNames = !m_Office.m_CanSetPlaceNames;
}

void AuGovAssignOffice::ToggleCanSetPoliticalStance()
{
	m_Office.m_CanSetPoliticalStance = !m_Office.m_CanSetPoliticalStance;
}

void AuGovAssignOffice::ToggleCanSetResearch()
{
	m_Office.m_CanSetResearch = !m_Office.m_CanSetResearch;
}

void AuGovAssignOffice::ToggleCanSetGalaxyCapital()
{
	m_Office.m_CanSetGalaxyCapital = !m_Office.m_CanSetGalaxyCapital;
}

void AuGovAssignOffice::ToggleCanSetSectorCapital()
{
	m_Office.m_CanSetSectorCapital = !m_Office.m_CanSetSectorCapital;
}

void AuGovAssignOffice::ToggleCanSetTax()
{
	m_Office.m_CanSetTax = !m_Office.m_CanSetTax;
}

void AuGovAssignOffice::ToggleCanSetTitle()
{
	m_Office.m_CanSetTitle = !m_Office.m_CanSetTitle;
}

void AuGovAssignOffice::ToggleCanUseDiplomacyChannel()
{
	m_Office.m_CanUseDiplomacyChannel = !m_Office.m_CanUseDiplomacyChannel;
}

void AuGovAssignOffice::ToggleCanSendEmpireMail()
{
	m_Office.m_CanSendEmpireMail = !m_Office.m_CanSendEmpireMail;
}

void AuGovAssignOffice::ToggleRxFleetMessages()
{
	m_Office.m_RxFleetMessages = !m_Office.m_RxFleetMessages;
}

void AuGovAssignOffice::ToggleRxSensorScans()
{
	m_Office.m_RxSensorScans = !m_Office.m_RxSensorScans;
}

void AuGovAssignOffice::AddDomain()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;

	AuGovChoosePlace dlg(m_Empire, m_Office, pFrame);
	int result = pFrame->ExecDialog(dlg);
	if(result == QDialog::Accepted && dlg.Place() && !m_Office.IsPlaceInDomain(dlg.Place()->AtomId()))
	{
		m_Office.AddDomain(*dlg.Place());

		QListWidgetItem* lwi = new QListWidgetItem(QIcon(dlg.Place()->Icon()), dlg.Place()->Name(), m_DomainList);
		lwi->setData(Qt::UserRole, dlg.Place()->AtomId().GetID());
	}
}

void AuGovAssignOffice::RemoveDomain()
{
	QList<QListWidgetItem*> sel = m_DomainList->selectedItems();
	if(sel.count())
	{
		for(QList<QListWidgetItem*>::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
		{
			m_Office.RevokeDomain(AuId((*si)->data(Qt::UserRole).toUInt()));
			delete m_DomainList->takeItem(m_DomainList->row(*si));
		}
	}
}

void AuGovAssignOffice::DomainSelectionChanged()
{
	if(!m_RemoveButton)
		return;

	QList<QListWidgetItem*> sel = m_DomainList->selectedItems();
	if(sel.count() > 0)
	{
		// An item is selected.
		m_RemoveButton->setEnabled(true);
	}
	else
	{
		// No items are selected.
		m_RemoveButton->setEnabled(false);
	}
}
