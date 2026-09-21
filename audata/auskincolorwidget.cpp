//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auskincolorwidget.cpp,v 1.4 2022/02/19 18:20:20 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auskincolorwidget.h"
//
#include "audna.h"
#include "aupushbutton.h"
#include <QComboBox>

AuSkinColorWidget::AuSkinColorWidget(QWidget* parent) : QWidget(parent)
{
    m_Color = new AuColorButton(this);
    connect(m_Color, SIGNAL(ColorChanged(QColor)), SLOT(ColorChanged(QColor)));

	m_ColorPresets = new QComboBox(this);
    for(int c=0; c<AuChromoSkin::Colors(); c++)
        m_ColorPresets->addItem(AuChromoSkin::ColorName(c), AuChromoSkin::Color(c));
    m_ColorPresets->setToolTip(tr("<html>"
                "A selection of skin colors. "
                "</html>"));
    connect(m_ColorPresets, SIGNAL(currentIndexChanged(int)), SLOT(ColorPresetChanged(int)));

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addWidget(m_Color);
	lay->addWidget(m_ColorPresets);
	setLayout(lay);
}

AuSkinColorWidget::~AuSkinColorWidget()
{
}

QColor AuSkinColorWidget::SkinColor() const
{
	return m_Color->Color();
}

void AuSkinColorWidget::SetSkinColor(const QColor& color)
{
	m_Color->SetColor(color);
}

void AuSkinColorWidget::ColorChanged(QColor color)
{
    // Color button changed.
    // Select matching light color, if any.
    int selIdx = m_ColorPresets->findData(QVariant(color));
    m_ColorPresets->blockSignals(true);
    m_ColorPresets->setCurrentIndex(selIdx);
    m_ColorPresets->blockSignals(false);
	emit SkinColorChanged(color);
}

void AuSkinColorWidget::ColorPresetChanged(int index)
{
	// Color preset selected.
	// Change the color in the color button.
    QColor color = m_ColorPresets->itemData(index).value<QColor>();
    m_Color->blockSignals(true);
    m_Color->SetColor(color);
    m_Color->blockSignals(false);
	emit SkinColorChanged(color);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

