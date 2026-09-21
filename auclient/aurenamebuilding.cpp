//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aurenamebuilding.cpp,v 1.6 2022/02/19 18:16:36 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aurenamebuilding.h"
//
#include "auapp.h"
#include "auglobal.h"
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AuRenameBuilding::AuRenameBuilding(const QString& curName, const AuId& capitolOpId, QWidget* parent) : AuDialog(parent)
{
	m_CapitolOpId = capitolOpId;
	m_OriginalName = curName;
	QIcon icon(":data/RenameBuilding.png");
	setAuDlgIcon(icon);
	setAuDlgTitle("Change Building Name");

	// Create name controls.
	QLabel* lbtext = new QLabel(this);
	if(curName.isEmpty())
		lbtext->setText(tr("The building is unnamed. Enter a name for the building."));
	else
		lbtext->setText(tr("The building is already named <b>%1</b>. Enter a new name for the building.").arg(curName));
	lbtext->setWordWrap(true);

	m_NameEdit = new QLineEdit(curName, this);
	m_NameEdit->setFocus();
	m_NameEdit->setToolTip(tr("<html><b>Name</b><br>"
				"Enter a new name for the building.</html>"));
	connect(m_NameEdit, SIGNAL(textEdited(const QString&)), SLOT(NameChanged()));

	QPushButton* pbrandomname = new QPushButton(this);
	pbrandomname->setText(tr("Suggest a Name"));
	pbrandomname->setToolTip(tr("<html>"
				"Suggests a name for the building."
				"</html>"));
	connect(pbrandomname, SIGNAL(clicked(bool)), SLOT(RandomName()));

	// Select a picture based on what is getting named.
	QPixmap img = QPixmap(":data/RenameBuilding.png").scaled(64,64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QLabel* lbimg = 0;
	if(!img.isNull())
	{
		lbimg = new QLabel(this);
		lbimg->setPixmap(img);
	}

	m_Ok = new QPushButton(tr("Rename Building"));
	m_Ok->setAutoDefault(true);
	m_Ok->setDefault(true);
	connect(m_Ok, SIGNAL(clicked()), SLOT(OkPressed()));
	m_Ok->setToolTip(tr("<html>"
				"Changes the name of the current building to the name entered. "
				"</html>"));

	m_ClearName = new QPushButton(tr("Clear Building Name"));
	connect(m_ClearName, SIGNAL(clicked()), SLOT(ClearNamePressed()));
	m_ClearName->setToolTip(tr("<html>"
				"Clears the name of the current building. "
				"The building will return to being unnamed."
				"</html>"));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbtext, row,0, 1,2);
	row++;
	lay->addWidget(m_NameEdit, row,0, 1,1);
	lay->addWidget(pbrandomname, row,1, 1,1);
	row++;
	lay->addWidget(m_Ok, row,0, 1,2);
	row++;
	lay->addWidget(m_ClearName, row,0, 1,2);
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

AuRenameBuilding::~AuRenameBuilding()
{
}

// returns enabled state of the ok button
void AuRenameBuilding::EnableControls()
{
	bool okenabled = IsNameOk();
	m_Ok->setEnabled(okenabled);
}

bool AuRenameBuilding::IsNameOk() const
{
	Q_ASSERT(m_NameEdit);
	QString name = m_NameEdit->text().simplified();
	if(!name.isEmpty() && !IsNameValid(name))
		return false;						// Name must not be invalid if it is not blank.
	return true;
}

void AuRenameBuilding::NameChanged()
{
	EnableControls();
}

void AuRenameBuilding::RandomName()
{
	// Generate random names until we get one that's ok.
	do
	{
		QString name = g_Au.Dice().RandomBuildingName();
		m_NameEdit->setText(name);
	}
	while(!IsNameOk());
	EnableControls();
}

void AuRenameBuilding::OkPressed()
{
	// Make a final check for valid name(s).
	if(!IsNameOk())
		return;
	QString name = m_NameEdit->text().simplified();
	if(name == m_OriginalName)
		return;
	auApp->BuildingRename(name, m_CapitolOpId);
	accept();
}

void AuRenameBuilding::ClearNamePressed()
{
	if(m_OriginalName.isEmpty())
		return;
	auApp->BuildingRename(QString(), m_CapitolOpId);
	accept();
}

