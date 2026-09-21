//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscvoidpropsdlg.cpp,v 1.3 2022/02/19 18:26:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscvoidpropsdlg.h"
//
#include "auclientglobal.h"
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AuScVoidPropsDlg::AuScVoidPropsDlg(QWidget* parent) : AuDialog(parent)
{
	m_Mode = UpZ;

	int lastG = g_Au.Settings().readNumEntry(QString("Widget/VoidPropsDlg/GravityType"), G_Natural);
	if(lastG < G_Natural || lastG >= G_Count)
		lastG = G_Natural;
	int lastHold = g_Au.Settings().readNumEntry(QString("Widget/VoidPropsDlg/HoldAccess"), HOLD_Inaccessible);
	if(lastHold < HOLD_Inaccessible || lastHold >= HOLD_Count)
		lastHold = HOLD_Inaccessible;
	bool lastCrew = g_Au.Settings().readBoolEntry(QString("Widget/VoidPropsDlg/PersonnelCrew"), true);
	bool lastOfficers = g_Au.Settings().readBoolEntry(QString("Widget/VoidPropsDlg/PersonnelOfficers"), true);
	bool lastPassengers = g_Au.Settings().readBoolEntry(QString("Widget/VoidPropsDlg/PersonnelPassengers"), true);
	bool lastTroops = g_Au.Settings().readBoolEntry(QString("Widget/VoidPropsDlg/PersonnelTroops"), true);

	setAuDlgTitle(tr("Room Void Properties"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	// Personnel Access
	QLabel* lbpers = new QLabel(tr("Configure personnel access restrictions in the room void. "
				"The captain is authorized access to all room voids."), this);
	lbpers->setWordWrap(true);

	m_PersonnelOfficers = new QCheckBox(tr("Officers"), this);
	m_PersonnelOfficers->setChecked(lastOfficers);
	m_PersonnelOfficers->setToolTip(tr("<html>When checked, officers are authorized to be in the room void. "
				"When unchecked, officers are not authorized to be in the room void.</html>"));
	connect(m_PersonnelOfficers, SIGNAL(stateChanged(int)), SLOT(OfficersChanged(int)));

	m_PersonnelCrew = new QCheckBox(tr("Crew"), this);
	m_PersonnelCrew->setChecked(lastCrew);
	m_PersonnelCrew->setToolTip(tr("<html>When checked, crew are authorized to be in the room void. "
				"When unchecked, crew are not authorized to be in the room void.</html>"));
	connect(m_PersonnelCrew, SIGNAL(stateChanged(int)), SLOT(CrewChanged(int)));

	m_PersonnelTroops = new QCheckBox(tr("Troops"), this);
	m_PersonnelTroops->setChecked(lastTroops);
	m_PersonnelTroops->setToolTip(tr("<html>When checked, troops are authorized to be in the room void. "
				"When unchecked, troops are not authorized to be in the room void.</html>"));
	connect(m_PersonnelTroops, SIGNAL(stateChanged(int)), SLOT(TroopsChanged(int)));

	m_PersonnelPassengers = new QCheckBox(tr("Passengers"), this);
	m_PersonnelPassengers->setChecked(lastPassengers);
	m_PersonnelPassengers->setToolTip(tr("<html>When checked, passengers are authorized to be in the room void. "
				"When unchecked, passengers are not authorized to be in the room void.</html>"));
	connect(m_PersonnelPassengers, SIGNAL(stateChanged(int)), SLOT(PassengersChanged(int)));

	// Hold Access
	QLabel* lbhold = new QLabel(tr("Configure hold accessibility while in the room void. "), this);
	lbhold->setWordWrap(true);

	m_HoldAccess = new QComboBox(this);
	for(int h=HOLD_Inaccessible; h<HOLD_Count; h++)
		m_HoldAccess->addItem(HoldAccessName((HOLD_ACCESS)h), h);
	m_HoldAccess->setCurrentIndex(m_HoldAccess->findData(lastHold));
	connect(m_HoldAccess, SIGNAL(currentIndexChanged(int)), SLOT(EnableControls()));

	// Gravity
	QLabel* lbgrav = new QLabel(tr("Configure gravity in the room void. "), this);
	lbgrav->setWordWrap(true);

	m_GravityType = new QComboBox(this);
	for(int g=G_Natural; g<G_Count; g++)
		m_GravityType->addItem(GravityName((GRAVITY)g), g);
	m_GravityType->setCurrentIndex(m_GravityType->findData(lastG));
	connect(m_GravityType, SIGNAL(currentIndexChanged(int)), SLOT(EnableControls()));

	m_PickDir = new QPushButton(tr("Pick Up Direction"), this);
	m_PickDir->setToolTip(tr("<html>"
				"The room void up direction away from gravity will be determined by entering two 3D locations. "
				"The cursor can be used to snap to existing geometry. "
				"</html>"));
	connect(m_PickDir, SIGNAL(clicked()), SLOT(DoPickVector()));

	QPushButton* pbupz = new QPushButton(tr("Real +Z is Up"), this);
	pbupz->setToolTip(tr("<html>"
				"The room void up direction will be in the direction of the real positive Z axis, (0,0,1). "
				"This is the true Z axis of the design. "
				"The current orientation of the grid plane is ignored. "
				"</html>"));
	connect(pbupz, SIGNAL(clicked()), SLOT(DoUpZ()));

	QPushButton* pbupgrid = new QPushButton(tr("Grid +Z is Up"), this);
	pbupgrid->setAutoDefault(true);
	pbupgrid->setDefault(true);
	pbupgrid->setToolTip(tr("<html>"
				"The room void up direction will be in the direction of the grid positive Z axis, (0,0,1). "
				"Orient the grid before choosing this option. "
				"</html>"));
	connect(pbupgrid, SIGNAL(clicked()), SLOT(DoUpZGrid()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(m_PickDir);
	blay->addWidget(pbupz);
	blay->addWidget(pbupgrid);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbpers);
	lay->addWidget(m_PersonnelOfficers);
	lay->addWidget(m_PersonnelCrew);
	lay->addWidget(m_PersonnelTroops);
	lay->addWidget(m_PersonnelPassengers);
	lay->addWidget(lbhold);
	lay->addWidget(m_HoldAccess);
	lay->addWidget(lbgrav);
	lay->addWidget(m_GravityType);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScVoidPropsDlg::~AuScVoidPropsDlg()
{
}

void AuScVoidPropsDlg::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
		EnableControls();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuScVoidPropsDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/VoidPropsDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/VoidPropsDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/VoidPropsDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/VoidPropsDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScVoidPropsDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/VoidPropsDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/VoidPropsDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/VoidPropsDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/VoidPropsDlg/SizeY",sz.height());
}

void AuScVoidPropsDlg::DoPickVector()
{
	g_Au.Settings().writeEntry("Widget/VoidPropsDlg/GravityType", GravityType());
	g_Au.Settings().writeEntry("Widget/VoidPropsDlg/HoldAccess", HoldAccess());

	m_Mode = PickVector;
	accept();
}

void AuScVoidPropsDlg::DoUpZ()
{
	g_Au.Settings().writeEntry("Widget/VoidPropsDlg/GravityType", GravityType());
	g_Au.Settings().writeEntry("Widget/VoidPropsDlg/HoldAccess", HoldAccess());

	m_Mode = UpZ;
	accept();
}

void AuScVoidPropsDlg::DoUpZGrid()
{
	g_Au.Settings().writeEntry("Widget/VoidPropsDlg/GravityType", GravityType());
	g_Au.Settings().writeEntry("Widget/VoidPropsDlg/HoldAccess", HoldAccess());

	m_Mode = UpZGrid;
	accept();
}

GRAVITY AuScVoidPropsDlg::GravityType() const
{
	int gi = m_GravityType->currentIndex();
	if(gi < 0)
		return G_Natural;

	GRAVITY grav = (GRAVITY)m_GravityType->itemData(gi).toInt();
	return grav;
}

HOLD_ACCESS AuScVoidPropsDlg::HoldAccess() const
{
	int hi = m_HoldAccess->currentIndex();
	if(hi < 0)
		return HOLD_Inaccessible;

	HOLD_ACCESS holdAccess = (HOLD_ACCESS)m_HoldAccess->itemData(hi).toInt();
	return holdAccess;
}

void AuScVoidPropsDlg::EnableControls()
{
	m_PickDir->setEnabled(GravityType() == G_Artificial);
}

void AuScVoidPropsDlg::CrewChanged(int state)
{
	g_Au.Settings().writeEntry(QString("Widget/VoidPropsDlg/PersonnelCrew"), state == Qt::Checked);
}

void AuScVoidPropsDlg::OfficersChanged(int state)
{
	g_Au.Settings().writeEntry(QString("Widget/VoidPropsDlg/PersonnelOfficers"), state == Qt::Checked);
}

void AuScVoidPropsDlg::PassengersChanged(int state)
{
	g_Au.Settings().writeEntry(QString("Widget/VoidPropsDlg/PersonnelPassengers"), state == Qt::Checked);
}

void AuScVoidPropsDlg::TroopsChanged(int state)
{
	g_Au.Settings().writeEntry(QString("Widget/VoidPropsDlg/PersonnelTroops"), state == Qt::Checked);
}

quint8 AuScVoidPropsDlg::PersonnelAccess() const
{
	quint8 access = 0;
	if(m_PersonnelOfficers->isChecked())
		access |= PERSONNEL_Command;
	if(m_PersonnelCrew->isChecked())
		access |= PERSONNEL_Crew;
	if(m_PersonnelTroops->isChecked())
		access |= PERSONNEL_Troops;
	if(m_PersonnelPassengers->isChecked())
		access |= PERSONNEL_Passengers;
	return access;
}
