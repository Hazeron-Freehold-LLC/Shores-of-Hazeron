//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aulightcolorwidget.cpp,v 1.3 2022/02/19 18:33:23 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aulightcolorwidget.h"
//
#include "augllight.h"
#include "aupushbutton.h"
#include <QComboBox>

AuLightColorWidget::AuLightColorWidget(QWidget* parent) : QWidget(parent)
{
    m_Color = new AuColorButton(this);
    connect(m_Color, SIGNAL(ColorChanged(QColor)), SLOT(ColorChanged(QColor)));

	m_ColorPresets = new QComboBox(this);
    for(int c=0; c<AuGLLight::Count(); c++)
        m_ColorPresets->addItem(AuGLLight::Name(c), AuGLLight::Color(c));
    m_ColorPresets->setToolTip(tr("<html>"
                "A selection of light colors. "
                "</html>"));
    connect(m_ColorPresets, SIGNAL(currentIndexChanged(int)), SLOT(ColorPresetChanged(int)));

	QHBoxLayout* lay = new QHBoxLayout;
	lay->addWidget(m_Color);
	lay->addWidget(m_ColorPresets);
	setLayout(lay);
}

AuLightColorWidget::~AuLightColorWidget()
{
}

QColor AuLightColorWidget::LightColor() const
{
	return m_Color->Color();
}

void AuLightColorWidget::SetLightColor(const QColor& color)
{
	m_Color->SetColor(color);
}

void AuLightColorWidget::ColorChanged(QColor color)
{
    // Color button changed.
    // Select matching light color, if any.
    int selIdx = m_ColorPresets->findData(QVariant(color));
    m_ColorPresets->blockSignals(true);
    m_ColorPresets->setCurrentIndex(selIdx);
    m_ColorPresets->blockSignals(false);
	emit LightColorChanged(color);
}

void AuLightColorWidget::ColorPresetChanged(int index)
{
	// Color preset selected.
	// Change the color in the color button.
    QColor color = m_ColorPresets->itemData(index).value<QColor>();
    m_Color->blockSignals(true);
    m_Color->SetColor(color);
    m_Color->blockSignals(false);
	emit LightColorChanged(color);
}

