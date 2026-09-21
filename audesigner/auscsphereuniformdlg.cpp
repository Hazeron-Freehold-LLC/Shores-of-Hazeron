//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscsphereuniformdlg.cpp,v 1.3 2022/02/19 18:25:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscsphereuniformdlg.h"
//
#include "auglobal.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>

AuScSphereUniformDlg::AuScSphereUniformDlg(quint8 partType, QWidget* parent) : AuDialog(parent)
{
	m_PartType = partType;

	setAuDlgTitle(tr("Sphere Uniform"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	QLabel* lbeq = new QLabel(tr("Enter the number of steps around the equator."), this);
	lbeq->setWordWrap(true);

	m_EqStepsSpin = new QSpinBox(this);
	m_EqStepsSpin->setRange(3, 1000);
	m_EqStepsSpin->setValue(g_Au.Settings().readNumEntry(QString("Widget/SphereUniform/EquatorSteps%1").arg(m_PartType), 10));

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>"
				"Creates a sphere."
				"</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(OkPressed()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbeq);
	lay->addWidget(m_EqStepsSpin);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScSphereUniformDlg::~AuScSphereUniformDlg()
{
}

void AuScSphereUniformDlg::setVisible(bool visible)
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

void AuScSphereUniformDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/SphereUniform/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/SphereUniform/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/SphereUniform/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/SphereUniform/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScSphereUniformDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/SphereUniform/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/SphereUniform/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/SphereUniform/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/SphereUniform/SizeY",sz.height());
}

void AuScSphereUniformDlg::OkPressed()
{
	g_Au.Settings().writeEntry(QString("Widget/SphereUniform/EquatorSteps%1").arg(m_PartType), m_EqStepsSpin->value());
	accept();
}

quint16 AuScSphereUniformDlg::EquatorSteps() const
{
	return m_EqStepsSpin->value();
}
