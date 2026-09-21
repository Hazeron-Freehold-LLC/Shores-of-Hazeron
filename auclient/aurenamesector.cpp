//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aurenamesector.cpp,v 1.5 2022/02/19 18:16:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aurenamesector.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "daperson.h"
#include "dasector.h"
#include "dgdata.h"
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

AuRenameSector::AuRenameSector(QWidget* parent) : AuDialog(parent)
{
	QIcon icon(":/data/RangeSector.png");
	setAuDlgIcon(icon);
	setAuDlgTitle("Change Sector Name");

	// Get the current name of the sector.
	QString sectorName;
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		const DaSector* sector = person->FindParentSector();
		if(sector && sector->HasName())
			sectorName = sector->SectorName();
	}
	pView->UnlockSceneAtom();

	// Create sector name controls.
	QLabel* lbtextsector = new QLabel(this);
	if(sectorName.isEmpty())
		lbtextsector->setText(tr("<html>Enter a name for the sector. The sector is currently unnamed.</html>"));
	else
		lbtextsector->setText(tr("<html>Enter a new name for the sector. The current name is <i>%1</i>.</html>").arg(sectorName));
	lbtextsector->setWordWrap(true);

	m_SectorNameEdit = new QLineEdit(this);
	m_SectorNameEdit->setFocus();
	m_SectorNameEdit->setToolTip(tr("<html><b>Sector Name</b><br>"
				"Enter a new name for the sector. The current name is <i>%1</i>.</html>").arg(sectorName));
	connect(m_SectorNameEdit, SIGNAL(textEdited(const QString&)), SLOT(SectorNameChanged()));

	QPushButton* pbrandomsector = new QPushButton(this);
	pbrandomsector->setText(tr("Suggest a Name"));
	pbrandomsector->setToolTip(tr("Suggests a name for the sector."));
	connect(pbrandomsector, SIGNAL(clicked(bool)), SLOT(RandomSectorName()));

	// Select a picture based on what is getting named.
	QPixmap img = QPixmap(":data/RangeSector.png").scaled(100,100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QLabel* lbimg = 0;
	if(!img.isNull())
	{
		lbimg = new QLabel(this);
		lbimg->setPixmap(img);
	}

	m_Ok = new QPushButton(tr("Rename Current Sector"));
	m_Ok->setAutoDefault(true);
	m_Ok->setDefault(true);
	connect(m_Ok, SIGNAL(clicked()), SLOT(OkPressed()));
	m_Ok->setToolTip(tr("Changes the name of the current sector to the name entered."));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbtextsector, row,0, 1,2);
	row++;
	lay->addWidget(m_SectorNameEdit, row,0, 1,1);
	lay->addWidget(pbrandomsector, row,1, 1,1);
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

AuRenameSector::~AuRenameSector()
{
}

// returns enabled state of the ok button
void AuRenameSector::EnableControls()
{
	bool okenabled = IsSectorNameOk();
	m_Ok->setEnabled(okenabled);
}

bool AuRenameSector::IsSectorNameOk() const
{
	Q_ASSERT(m_SectorNameEdit);

	QString name = m_SectorNameEdit->text().simplified();
	if(!IsNameValid(name))
		return false;						// Name must not be invalid.
	return true;
}

void AuRenameSector::SectorNameChanged()
{
	EnableControls();
}

void AuRenameSector::RandomSectorName()
{
	// Generate random sector names until we get one that's ok.
	do
	{
		QString name = g_Au.Dice().RandomSectorName();
		m_SectorNameEdit->setText(name);
	}
	while(!IsSectorNameOk());

	EnableControls();
}

void AuRenameSector::OkPressed()
{
	// Make a final check for valid name(s).
	if(!IsSectorNameOk())
		return;

	m_SectorName = m_SectorNameEdit->text().simplified();
	auApp->SectorRename(m_SectorName);
	accept();
}
