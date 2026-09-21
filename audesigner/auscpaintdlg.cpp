//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscpaintdlg.cpp,v 1.4 2022/02/19 18:25:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscpaintdlg.h"
//
#include "auglobal.h"
#include "aupushbutton.h"
#include "auscface.h"
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QPushButton>
#include <QRadioButton>

AuScPaintDlg::AuScPaintDlg(const QList<QColor>& paintInUse, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Paint"));
	setAuDlgIcon(QIcon(":data/SoHDesignerIcon.png"));

	bool useTheme = g_Au.Settings().readBoolEntry("Widget/PaintDlg/UseTheme", false);
	int theme = g_Au.Settings().readNumEntry("Widget/PaintDlg/Theme", AuScFace::THEME_ExteriorPrimary);

	m_RadioColor = new QRadioButton(tr("Paint using a color. "), this);
	m_RadioColor->setChecked(!useTheme);

	QColor col = g_Au.Settings().readColorEntry(QString("Widget/PaintDlg/Color"), QColor(255,255,255));
	m_Color = new AuColorButton(this);
	m_Color->SetColor(col);
	m_Color->SetCustomColors(paintInUse);

	QLabel* lbtrans = new QLabel(tr("Choose the transparency of the paint."), this);
	lbtrans->setWordWrap(true);

	QLabel* lbmin = new QLabel(tr("Opaque"), this);
	lbmin->setAlignment(Qt::AlignVCenter|Qt::AlignRight);

	QLabel* lbmax = new QLabel(tr("Invisible"), this);
	lbmax->setAlignment(Qt::AlignVCenter|Qt::AlignLeft);

	int opacity = g_Au.Settings().readNumEntry("Widget/PaintDlg/Opacity", 204);

	m_TransparencySlider = new QSlider(Qt::Horizontal, this);
	m_TransparencySlider->setRange(0,255);
	m_TransparencySlider->setValue(255 - opacity);
	m_TransparencySlider->setToolTip(tr("<html><b>Paint Transparency</b><br>"
				"Sets the transparency of the paint. "
				"Opaque paint completely obscures the underlying texture. "
				"A small amount of transparency allows some of the underlying texture to show through. "
				"This can retain useful detail, depending on the texture. "
				"A large amount of transparency causes only a slight tint change to the underlying texture. "
				"The lowest setting removes paint entirely. "
				"</html>"));
	connect(m_TransparencySlider, SIGNAL(valueChanged(int)), SLOT(TransparencyChanged(int)));

	int tv = 100 - opacity * 100 / 255;
	m_TransparencyValue = new QLabel(QString("%1%").arg(tv), this);

	QPushButton* pbnopaint = new QPushButton(tr("Bare"), this);
	pbnopaint->setToolTip(tr("<html>"
				"Removes paint from the selected faces."
				"</html>"));
	connect(pbnopaint, SIGNAL(clicked()), SLOT(DoNoPaint()));

	QPushButton* pbpaint = new QPushButton(tr("Paint"), this);
	pbpaint->setAutoDefault(true);
	pbpaint->setDefault(true);
	pbpaint->setToolTip(tr("<html>"
				"Applies paint to the selected faces."
				"</html>"));
	connect(pbpaint, SIGNAL(clicked()), SLOT(DoPaint()));

	m_RadioTheme = new QRadioButton(tr("Paint using an empire theme color. "), this);
	m_RadioTheme->setChecked(useTheme);

	m_ThemeCombo = new QComboBox(this);
	m_ThemeCombo->addItem("Exterior Primary Color", AuScFace::THEME_ExteriorPrimary);
	m_ThemeCombo->addItem("Exterior Secondary Color", AuScFace::THEME_ExteriorSecondary);
	m_ThemeCombo->addItem("Exterior Tertiary Color", AuScFace::THEME_ExteriorTertiary);
	m_ThemeCombo->addItem("Interior Primary Color", AuScFace::THEME_InteriorPrimary);
	m_ThemeCombo->addItem("Interior Secondary Color", AuScFace::THEME_InteriorSecondary);
	m_ThemeCombo->addItem("Interior Tertiary Color", AuScFace::THEME_InteriorTertiary);
	m_ThemeCombo->setCurrentIndex(m_ThemeCombo->findData(theme));

	QHBoxLayout* slblay = new QHBoxLayout;
	slblay->addWidget(lbmin);
	slblay->addStretch();
	slblay->addWidget(lbmax);

	QHBoxLayout* slay = new QHBoxLayout;
	slay->addWidget(m_TransparencySlider);
	slay->addWidget(m_TransparencyValue);

	QGridLayout* rb1lay = new QGridLayout;
	int row=0;
	rb1lay->addWidget(m_RadioColor, row,0, 1,2);
	row++;
	rb1lay->addWidget(m_Color, row,1); 
	row++;
	rb1lay->setColumnMinimumWidth(0, 10);

	QGridLayout* rb2lay = new QGridLayout;
	row=0;
	rb2lay->addWidget(m_RadioTheme, row,0, 1,2);
	row++;
	rb2lay->addWidget(m_ThemeCombo, row,1);
	row++;
	rb2lay->setColumnMinimumWidth(0, 10);

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbnopaint);
	blay->addWidget(pbpaint);
	blay->setMargin(0);
	blay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(rb1lay);
	lay->addLayout(rb2lay);
	lay->addWidget(lbtrans);
	lay->addLayout(slblay);
	lay->addLayout(slay);
	lay->addLayout(blay);
	lay->addStretch();
	lay->setMargin(4);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuScPaintDlg::~AuScPaintDlg()
{
}

void AuScPaintDlg::setVisible(bool visible)
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

void AuScPaintDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/PaintDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/PaintDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/PaintDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/PaintDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuScPaintDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/PaintDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/PaintDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/PaintDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/PaintDlg/SizeY",sz.height());
}

void AuScPaintDlg::DoNoPaint()
{
	// Set up to return 0,0,0,0 paint.
	m_RadioTheme->setChecked(true);

	m_TransparencySlider->blockSignals(true);
	m_TransparencySlider->setValue(255);
	m_TransparencySlider->blockSignals(false);

	accept();
}

void AuScPaintDlg::DoPaint()
{
	bool useTheme = UseTheme();
	g_Au.Settings().writeEntry("Widget/PaintDlg/UseTheme", useTheme);

	int theme = Theme();
	g_Au.Settings().writeEntry("Widget/PaintDlg/Theme", theme);

	QColor color = m_Color->Color();
	g_Au.Settings().writeEntry("Widget/PaintDlg/Color", color);

	quint8 alpha = Opacity();
	g_Au.Settings().writeEntry("Widget/PaintDlg/Opacity", alpha);

	accept();
}

QColor AuScPaintDlg::Paint() const
{
	QColor color = m_Color->Color();

	// Prevent colors that are too dark.
	qreal h,s,v;
	color.getHsvF(&h, &s, &v);
	if(v < .05f)
		color.setHsvF(h, s, .05f);

	quint8 alpha = Opacity();
	color.setAlpha(alpha);

	return color;
}

quint8 AuScPaintDlg::Theme() const
{
	quint8 theme = m_ThemeCombo->itemData(m_ThemeCombo->currentIndex()).toUInt();
	return theme;
}

quint8 AuScPaintDlg::Opacity() const
{
	return 255 - m_TransparencySlider->value();
}

bool AuScPaintDlg::UseTheme() const
{
	return m_RadioTheme->isChecked();
}

void AuScPaintDlg::TransparencyChanged(int value)
{
	int tv = value * 100 / 255;
	m_TransparencyValue->setText(QString("%1%").arg(tv));
}
