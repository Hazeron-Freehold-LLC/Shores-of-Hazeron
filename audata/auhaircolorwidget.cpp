//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auhaircolorwidget.cpp,v 1.5 2022/02/19 18:20:09 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auhaircolorwidget.h"
//
#include "audna.h"
#include "aupushbutton.h"
#include <QComboBox>

AuHairColorWidget::AuHairColorWidget(QWidget* parent) : QWidget(parent)
{
    m_Color = new AuColorButton(this);
    connect(m_Color, SIGNAL(ColorChanged(QColor)), SLOT(ColorChanged(QColor)));

	m_ColorPresets = new QComboBox(this);
    for(int c=0; c<AuChromoFur::Colors(); c++)
        m_ColorPresets->addItem(AuChromoFur::ColorName(c), AuChromoFur::Color(c));
    m_ColorPresets->setToolTip(tr("<html>"
                "A selection of hair colors. "
                "</html>"));
    connect(m_ColorPresets, SIGNAL(currentIndexChanged(int)), SLOT(ColorPresetChanged(int)));

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addWidget(m_Color);
	lay->addWidget(m_ColorPresets);
	setLayout(lay);
}

AuHairColorWidget::~AuHairColorWidget()
{
}

QColor AuHairColorWidget::HairColor() const
{
	return m_Color->Color();
}

void AuHairColorWidget::SetHairColor(const QColor& color)
{
	m_Color->SetColor(color);
}

void AuHairColorWidget::ColorChanged(QColor color)
{
    // Color button changed.
    // Select matching light color, if any.
    int selIdx = m_ColorPresets->findData(QVariant(color));
    m_ColorPresets->blockSignals(true);
    m_ColorPresets->setCurrentIndex(selIdx);
    m_ColorPresets->blockSignals(false);
	emit HairColorChanged(color);
}

void AuHairColorWidget::ColorPresetChanged(int index)
{
	// Color preset selected.
	// Change the color in the color button.
    QColor color = m_ColorPresets->itemData(index).value<QColor>();
    m_Color->blockSignals(true);
    m_Color->SetColor(color);
    m_Color->blockSignals(false);
	emit HairColorChanged(color);
}

