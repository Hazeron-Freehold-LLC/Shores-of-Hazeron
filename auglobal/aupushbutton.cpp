//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aupushbutton.cpp,v 1.9 2022/02/19 18:33:28 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aupushbutton.h"
//
#include "auglobal.h"
#include <QColorDialog>
#include <QMouseEvent>
#include <QPainter>
#include <QToolButton>

AuHueSelector::AuHueSelector(QWidget* parent) : QWidget(parent)
{
	m_Hue = 170;			// Default blue
	m_Saturation = 127;		// Default blue

	//setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

QSize AuHueSelector::sizeHint() const
{
	return QSize(256,16);
}

void AuHueSelector::SetHue(quint8 hue)
{
	m_Hue = hue;
	update();

	if(updatesEnabled())
		emit HueChanged(m_Hue);
}

void AuHueSelector::SetSaturation(quint8 sat)
{
	m_Saturation = sat;
	update();
}

void AuHueSelector::mousePressEvent(QMouseEvent* ev)
{
	if(ev->button() == Qt::LeftButton && width() > 1)
		SetHue(ev->pos().x() * 255 / (width() - 1));
	QWidget::mousePressEvent(ev);
}

void AuHueSelector::paintEvent(QPaintEvent* ev)
{
	QWidget::paintEvent(ev);

	QPainter p(this);

	float sat = (float)m_Saturation / 255.f;

	// Horizontal orientation.
	int leftPixel = 0;
	int rightPixel = width() - 1;
	int pixelWidth = rightPixel - leftPixel + 1;
	if(pixelWidth)
	{
		for(int xpos=leftPixel; xpos<=rightPixel; xpos++)
		{
			float hue = (float)xpos / (float)pixelWidth;
			QColor col;
			col.setHsvF(hue, sat, 1);
			p.setPen(QPen(col, 0.f));
			p.drawLine(xpos,0, xpos,height()-1);
		}

		int curPixel = leftPixel + Hue() * pixelWidth / 255;
		p.setPen(QPen(Qt::black, 0.f));
		p.drawLine(curPixel,0, curPixel,height()-1);
	}
}

QColor AuHueSelector::Color() const
{
	float sat = (float)m_Saturation / 255.f;
	float hue = (float)m_Hue / 255.f;
	QColor col;
	col.setHsvF(hue, sat, 1);
	return col;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuColorButton::AuColorButton(QWidget* parent) : QPushButton(parent)
{
	m_Color = QColor(255,255,255);

	//setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	connect(this, SIGNAL(clicked()), this, SLOT(PickColor()));
}

QSize AuColorButton::sizeHint() const
{
	return QSize(32,16);
}

void AuColorButton::SetColor(QColor color)
{
	m_Color = color;
	update();

	if(updatesEnabled())
		emit ColorChanged(m_Color);
}

void AuColorButton::SetCustomColors(const QList<QColor>& customColors)
{
	m_CustomColors = customColors;
}

void AuColorButton::paintEvent(QPaintEvent* ev)
{
	QWidget::paintEvent(ev);

	QPainter p(this);
	p.fillRect(rect(), m_Color);
}

QColor AuColorButton::Color() const
{
	return m_Color;
}

void AuColorButton::PickColor()
{
	// Configure custom color choices. This is done with a static function.
	int ccnt = QColorDialog::customCount();
	int cidx = 0;
	for(QList<QColor>::const_iterator ci = m_CustomColors.cbegin(); ci != m_CustomColors.cend() && cidx < ccnt; ci++, cidx++)
		QColorDialog::setCustomColor(cidx, *ci);
	QColor color = QColorDialog::getColor(m_Color, this, tr("Choose Color"),
			g_Au.UseNativeColorDialog() ? QColorDialog::ColorDialogOptions() : QColorDialog::DontUseNativeDialog);
	if(!color.isValid())
		return;
	SetColor(color);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuPushButton::mouseDoubleClickEvent(QMouseEvent* me)
{
	QPushButton::mouseDoubleClickEvent(me);
	emit DoubleClicked();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuPushButtonExpand::AuPushButtonExpand(QWidget* parent) : QWidget(parent)
{
	m_MainButton = new AuPushButton(this);
	connect(m_MainButton, SIGNAL(clicked()), SLOT(BaseButtonClicked()));
	connect(m_MainButton, SIGNAL(DoubleClicked()), SLOT(BaseButtonDoubleClicked()));
	m_MainButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QPixmap px = QPixmap(":data/hazExpand.png").scaled(8,8);
	m_ExpandButton = new QToolButton(this);
	m_ExpandButton->setIcon(QIcon(px));
	m_ExpandButton->setIconSize(QSize(8,8));
	m_ExpandButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	connect(m_ExpandButton, SIGNAL(clicked()), SLOT(BaseExpandClicked()));

	QGridLayout* lay = new QGridLayout(this);
	lay->addWidget(m_MainButton, 1,0, 2,2);
	lay->addWidget(m_ExpandButton, 0,1, 2,2);
	lay->setColumnStretch(0, 1);
	lay->setColumnMinimumWidth(1, 8);
	lay->setColumnMinimumWidth(2, 8);
	lay->setRowMinimumHeight(0, 8);
	lay->setRowMinimumHeight(1, 8);
	lay->setRowStretch(2, 1);
	lay->setMargin(0);
	lay->setSpacing(0);
	setLayout(lay);
}

void AuPushButtonExpand::SetText(const QString& text)
{
	m_MainButton->setText(text);
}

void AuPushButtonExpand::SetIconSize(const QSize& size)
{
	int szexpand;
	if(size.width() < size.height())
		szexpand = size.width();
	else
		szexpand = size.height();
	szexpand /= 4;
	if(szexpand < 4)
		szexpand = 4;
	m_ExpandButton->setIconSize(QSize(szexpand, szexpand));
}

void AuPushButtonExpand::SetToolTip(const QString& tip)
{
	m_MainButton->setToolTip(tip);
}

void AuPushButtonExpand::SetExpandToolTip(const QString& tip)
{
	m_ExpandButton->setToolTip(tip);
}

void AuPushButtonExpand::setVisible(bool visible)
{
	QWidget::setVisible(visible);
	//m_MainButton->setVisible(visible);
	//m_MenuButton->setVisible(visible);
}

void AuPushButtonExpand::BaseButtonClicked()
{
	emit Clicked();
}

void AuPushButtonExpand::BaseButtonDoubleClicked()
{
	emit DoubleClicked();
}

void AuPushButtonExpand::BaseExpandClicked()
{
	emit ExpandClicked();
}

QString AuPushButtonExpand::Text() const
{
	return m_MainButton->text();
}

void AuPushButtonExpand::SetStyleSheet(const QString& styleSheet)
{
	m_MainButton->setStyleSheet(styleSheet);
}

void AuPushButtonExpand::SetFocusPolicy(Qt::FocusPolicy policy)
{
	m_MainButton->setFocusPolicy(policy);
	m_ExpandButton->setFocusPolicy(policy);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuPushButtonExpandValue::AuPushButtonExpandValue(int value, QWidget* parent) : AuPushButtonExpand(parent)
{
	m_Value = value;
}

void AuPushButtonExpandValue::BaseButtonClicked()
{
	emit Clicked(m_Value);
}

void AuPushButtonExpandValue::BaseButtonDoubleClicked()
{
	emit DoubleClicked(m_Value);
}

void AuPushButtonExpandValue::BaseExpandClicked()
{
	emit ExpandClicked(m_Value);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

