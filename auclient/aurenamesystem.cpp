//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aurenamesystem.cpp,v 1.5 2022/02/19 18:16:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aurenamesystem.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "dgdata.h"
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AuRenameSystem::AuRenameSystem(QWidget* parent) : AuDialog(parent)
{
	QIcon icon(":/data/RangeSystem.png");
	setAuDlgIcon(icon);
	setAuDlgTitle("Change Solar System Name");

	// Get the current name of the system.
	QString systemName;
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		DaSolarSystem* system = person->FindParentSolarSystem();
		if(system)
			systemName = system->Name();
	}
	pView->UnlockSceneAtom();

	// Create system name controls.
	QLabel* lbtextsystem = new QLabel(this);
	lbtextsystem->setText(tr("<html>Enter a new name for the solar system. The current name is <i>%1</i>.<br><br>"
				"NOTE: Solar system name changes propagate to all planets in the system.</html>").arg(systemName));
	lbtextsystem->setWordWrap(true);

	m_SystemNameEdit = new QLineEdit(this);
	m_SystemNameEdit->setFocus();
	m_SystemNameEdit->setToolTip(tr("<html><b>Solar System Name</b><br>"
				"Enter a new name for the solar system. The current name is <i>%1</i>.</html>").arg(systemName));
	connect(m_SystemNameEdit, SIGNAL(textEdited(const QString&)), SLOT(SystemNameChanged()));

	QPushButton* pbrandomsystem = new QPushButton(this);
	pbrandomsystem->setText(tr("Suggest a Name"));
	pbrandomsystem->setToolTip(tr("Suggests a name for the solar system."));
	connect(pbrandomsystem, SIGNAL(clicked(bool)), SLOT(RandomSystemName()));

	// Select a picture based on what is getting named.
	QPixmap img = QPixmap(":data/WormholeSpace.png").scaled(100,100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QLabel* lbimg = 0;
	if(!img.isNull())
	{
		lbimg = new QLabel(this);
		lbimg->setPixmap(img);
	}

	m_Ok = new QPushButton(tr("Rename Current Solar System"));
	m_Ok->setAutoDefault(true);
	m_Ok->setDefault(true);
	connect(m_Ok, SIGNAL(clicked()), SLOT(OkPressed()));
	m_Ok->setToolTip(tr("Changes the name of the current solar system to the name entered."));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbtextsystem, row,0, 1,2);
	row++;
	lay->addWidget(m_SystemNameEdit, row,0, 1,1);
	lay->addWidget(pbrandomsystem, row,1, 1,1);
	row++;
	lay->addWidget(m_Ok, row,0, 1,2);
	row++;
	lay->setRowStretch(row, 1);
	row++;
	if(lbimg)
	{
		QVBoxLayout* vlay = new QVBoxLayout;
		vlay->addWidget(lbimg);
		vlay->addStretch(1);
		lay->addLayout(vlay, 0,2, row,1);
	}
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);

	EnableControls();
}

AuRenameSystem::~AuRenameSystem()
{
}

// returns enabled state of the ok button
void AuRenameSystem::EnableControls()
{
	bool okenabled = IsSystemNameOk();
	m_Ok->setEnabled(okenabled);
}

bool AuRenameSystem::IsSystemNameOk() const
{
	Q_ASSERT(m_SystemNameEdit);

	QString name = m_SystemNameEdit->text().simplified();
	if(!IsNameValid(name))
		return false;						// Name must not be invalid.
	return true;
}

void AuRenameSystem::SystemNameChanged()
{
	EnableControls();
}

void AuRenameSystem::RandomSystemName()
{
	// Generate random system names until we get one that's ok.
	do
	{
		QString name = g_Au.Dice().RandomSystemName();
		m_SystemNameEdit->setText(name);
	}
	while(!IsSystemNameOk());

	EnableControls();
}

void AuRenameSystem::OkPressed()
{
	// Make a final check for valid name(s).
	if(!IsSystemNameOk())
		return;

	m_SystemName = m_SystemNameEdit->text().simplified();
	auApp->SystemRename(m_SystemName);
	accept();
}
