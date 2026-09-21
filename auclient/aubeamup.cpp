//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubeamup.cpp,v 1.2 2022/02/19 18:07:21 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubeamup.h"
//
#include <auglobal.h>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVariant>

AuBeamUp::AuBeamUp(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Request Transport"));
	setAuDlgIcon(QIcon(":data/MsgTransport.png"));

	QLabel* lb1 = new QLabel(tr("How many to transport?"));

	m_BeamCountSpin = new QSpinBox(this);
	m_BeamCountSpin->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_BeamCountSpin->selectAll();
	m_BeamCountSpin->setMinimum(1);
	m_BeamCountSpin->setMaximum(255);
	m_BeamCountSpin->setValue(1);
	m_BeamCountSpin->setToolTip(tr("<html>Enter the number of transporter beams to energize.</html>"));

	QPushButton* okButton = new QPushButton(tr("Request Transporter Beam"), this);
	okButton->setAutoDefault(true);
	okButton->setDefault(true);
	connect(okButton, SIGNAL(clicked()), SLOT(accept()));
	okButton->setToolTip(tr("<html>Request transporter beam. "
				"The request is transmitted on the current comm channel.</html>"));

	// Layout the controls.
	QGridLayout* lay = new QGridLayout();
	int row = 0;
	lay->addWidget(lb1, row, 0);
	lay->addWidget(m_BeamCountSpin, row, 1);
	row++;
	lay->addWidget(okButton, row, 1);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuBeamUp::~AuBeamUp()
{
}

unsigned int AuBeamUp::BeamCount() const
{
	// Returns the final value the user entered if the dialog is accepted.
	return m_BeamCountSpin->value();
}

void AuBeamUp::showEvent(QShowEvent* event)
{
	AuDialog::showEvent(event);

	int beamCount = g_Au.Settings().readNumEntry(QString("Widget/AuBeamUp/BeamCount"), 1);
	if(beamCount >= 1 && beamCount <= 255)
		m_BeamCountSpin->setValue(beamCount);
}

void AuBeamUp::hideEvent(QHideEvent* event)
{
	AuDialog::hideEvent(event);

	g_Au.Settings().writeEntry(QString("Widget/AuBeamUp/BeamCount"), m_BeamCountSpin->value());
}
