//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscpostdlg.cpp,v 1.3 2022/02/19 18:25:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscpostdlg.h"
//
#include "auglobal.h"
#include "auscparts.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>

AuScPostDlg::AuScPostDlg(quint8 partType, QWidget* parent) : AuDialog(parent)
{
	m_PartType = partType;

	setAuDlgTitle(tr("Post"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	QLabel* lbinfo = new QLabel(tr("Choose the body position of the post's occupant."), this);
	lbinfo->setWordWrap(true);

	m_Sit = new QRadioButton(tr("Sit %1 Occupant sits; can turn and look, like a swivel chair.").arg(g_Au.MiddleDotSymbol()), this);

	m_SitStationary = new QRadioButton(tr("Sit Stationary %1 Occupant cannot turn but can twist at the waist and look, "
				"like a bucket seat.").arg(g_Au.MiddleDotSymbol()), this);

	m_Stand = new QRadioButton(tr("Stand %1 Occupant stands; can turn and look.").arg(g_Au.MiddleDotSymbol()), this);
	m_Stand->setChecked(true);

	m_StandStationary = new QRadioButton(tr("Stand Stationary %1 Occupant stands; cannot turn but can twist at the waist and look.")
				.arg(g_Au.MiddleDotSymbol()), this);

	m_PickLocation = new QPushButton(tr("Pick Occupant Location"), this);
	m_PickLocation->setAutoDefault(true);
	m_PickLocation->setDefault(true);
	m_PickLocation->setToolTip(tr("<html>"
				"Accepts the choice of occupant position and requests the occupant's 3D location."
				"</html>"));
	connect(m_PickLocation, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(m_PickLocation);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbinfo);
	lay->addWidget(m_Sit);
	lay->addWidget(m_SitStationary);
	lay->addWidget(m_Stand);
	lay->addWidget(m_StandStationary);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScPostDlg::~AuScPostDlg()
{
}

void AuScPostDlg::setVisible(bool visible)
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

void AuScPostDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/PostDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/PostDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/PostDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/PostDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScPostDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/PostDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/PostDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/PostDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/PostDlg/SizeY",sz.height());
}

quint8 AuScPostDlg::Position() const
{
	if(m_Sit->isChecked())
		return AuScPost::Occupant_Sit;

	if(m_SitStationary->isChecked())
		return AuScPost::Occupant_SitStationary;

	if(m_Stand->isChecked())
		return AuScPost::Occupant_Stand;

	if(m_StandStationary->isChecked())
		return AuScPost::Occupant_StandStationary;

	return AuScPost::Occupant_Stand;
}
