//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscpolishdlg.cpp,v 1.4 2022/02/19 18:25:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscpolishdlg.h"
//
#include "auglobal.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QPushButton>

AuScPolishDlg::AuScPolishDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Polish"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	float shine = g_Au.Settings().readNumEntry("Widget/PolishDlg/Shininess", 50.);
	if(shine < 0)
		shine = 0;

	int reflectivity = g_Au.Settings().readNumEntry("Widget/PolishDlg/Reflectivity", 100);
	if(reflectivity < 0)
		reflectivity = 0;
	else if(reflectivity > 100)
		reflectivity = 100;

	QLabel* lbshine = new QLabel(tr("Enter the shininess of the surface."), this);
	lbshine->setWordWrap(true);

	m_ShineEdit = new QLineEdit(this);
	m_ShineEdit->setText(QVariant(shine).toString());
	m_ShineEdit->setToolTip(tr("<html><b>Shininess</b><br>"
				"<p>"
				"Shininess controls the scatter of reflected light, to simulate surface smoothness. "
				"This is an exponent in the specular lighting equation. "
				"</p>"

				"<p>"
				"Shininess must be 0 or more. "
				"Decimal values are ok. "
				"Higher shininess produces small intense specular highlights. "
				"Lower shininess produces large diffuse specular highlights. "
				"Try numbers up to 1000, or more. "
				"</p>"
				"</html>"));

	QLabel* lbreflect = new QLabel(tr("Enter the reflectivity of the surface."), this);
	lbreflect->setWordWrap(true);

	QLabel* lbmin = new QLabel(tr("None"), this);
	lbmin->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

	QLabel* lbmax = new QLabel(tr("Full"), this);
	lbmax->setAlignment(Qt::AlignVCenter|Qt::AlignLeft);

	m_ReflectSlider = new QSlider(Qt::Horizontal, this);
	m_ReflectSlider->setRange(0,100);
	m_ReflectSlider->setValue(reflectivity);
	m_ReflectSlider->setToolTip(tr("<html><b>Reflectivity</b><br>"
				"<p>"
				"Reflectivity controls how much of the specular light get reflected. "
				"This is a linear factor in the specular lighting equation. "
				"</p>"

				"<p>"
				"Full reflectivity produces the brightest light reflected by specular highlights. "
				"Lower reflectivity reduces the amount of light reflected by specular highlights. "
				"</p>"
				"</html>"));
	connect(m_ReflectSlider, SIGNAL(valueChanged(int)), SLOT(ReflectChanged(int)));

	m_ReflectValue = new QLabel(QString("%1%").arg(reflectivity), this);

	QPushButton* pbmatte = new QPushButton(tr("None"), this);
	pbmatte->setToolTip(tr("<html>"
				"Removes polish from the selected faces."
				"</html>"));
	connect(pbmatte, SIGNAL(clicked()), SLOT(DoMatte()));

	QPushButton* pbpolish = new QPushButton(tr("Polish"), this);
	pbpolish->setAutoDefault(true);
	pbpolish->setDefault(true);
	pbpolish->setToolTip(tr("<html>"
				"Applies polish to the selected faces."
				"</html>"));
	connect(pbpolish, SIGNAL(clicked()), SLOT(DoPolish()));

	QHBoxLayout* slblay = new QHBoxLayout;
	slblay->addWidget(lbmin);
	slblay->addStretch();
	slblay->addWidget(lbmax);

	QHBoxLayout* slay = new QHBoxLayout;
	slay->addWidget(m_ReflectSlider);
	slay->addWidget(m_ReflectValue);

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbmatte);
	blay->addWidget(pbpolish);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbshine);
	lay->addWidget(m_ShineEdit);
	lay->addWidget(lbreflect);
	lay->addLayout(slblay);
	lay->addLayout(slay);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScPolishDlg::~AuScPolishDlg()
{
}

void AuScPolishDlg::setVisible(bool visible)
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

void AuScPolishDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/PolishDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/PolishDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/PolishDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/PolishDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScPolishDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/PolishDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/PolishDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/PolishDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/PolishDlg/SizeY",sz.height());
}

void AuScPolishDlg::DoMatte()
{
	m_ShineEdit->setText("0");

	m_ReflectSlider->blockSignals(true);
	m_ReflectSlider->setValue(0);
	m_ReflectSlider->blockSignals(false);

	m_ReflectValue->setText("0%");

	accept();
}

void AuScPolishDlg::DoPolish()
{
	g_Au.Settings().writeEntry("Widget/PolishDlg/Shininess", Shine());
	g_Au.Settings().writeEntry("Widget/PolishDlg/Reflectivity", m_ReflectSlider->value());

	accept();
}

float AuScPolishDlg::Reflectivity() const
{
	int val = m_ReflectSlider->value();
	if(val == 100)
		return 1;
	return (float)val/100;
}

float AuScPolishDlg::Shine() const
{
	float s = QVariant(m_ShineEdit->text()).toFloat();

	if(s < 0)
		s = 0;

	return s;
}

void AuScPolishDlg::ReflectChanged(int value)
{
	m_ReflectValue->setText(QString("%1%").arg(value));
}
