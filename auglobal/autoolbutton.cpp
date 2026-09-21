//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: autoolbutton.cpp,v 1.6 2022/02/19 18:33:35 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "autoolbutton.h"
//
#include "audef.h"
#include <QDesktopServices>
#include <QGridLayout>
#include <QSpinBox>
#include <QToolButton>

AuToolButtonExpand::AuToolButtonExpand(QWidget* parent) : QWidget(parent)
{
	m_MainButton = new QToolButton(this);
	connect(m_MainButton, SIGNAL(clicked()), SLOT(BaseButtonClicked()));
	m_MainButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QPixmap px = QPixmap(":data/hazExpand.png").scaled(8,8);
	m_ExpandButton = new QToolButton(this);
	m_ExpandButton->setIcon(QIcon(px));
	m_ExpandButton->setIconSize(QSize(8,8));
	m_ExpandButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_ExpandButton->setToolTip(tr("..."));
	connect(m_ExpandButton, SIGNAL(clicked()), SLOT(BaseExpandClicked()));

	QGridLayout* lay = new QGridLayout(this);
	lay->addWidget(m_MainButton, 1,0, 2,2);
	lay->addWidget(m_ExpandButton, 0,1, 2,2);
	lay->setColumnStretch(0, 16);
	lay->setColumnStretch(1, 8);
	lay->setColumnStretch(2, 8);
	lay->setRowStretch(0, 8);
	lay->setRowStretch(1, 8);
	lay->setRowStretch(2, 16);
	lay->setMargin(0);
	lay->setSpacing(0);
	setLayout(lay);
}

void AuToolButtonExpand::SetIcon(const QIcon& icon)
{
	m_MainButton->setIcon(icon);
}

void AuToolButtonExpand::SetIconSize(const QSize& size)
{
	m_MainButton->setIconSize(size);
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

void AuToolButtonExpand::SetToolTip(const QString& tip)
{
	m_MainButton->setToolTip(tip);
}

void AuToolButtonExpand::SetExpandToolTip(const QString& tip)
{
	m_ExpandButton->setToolTip(tip);
}

void AuToolButtonExpand::setVisible(bool visible)
{
	QWidget::setVisible(visible);
	//m_MainButton->setVisible(visible);
	//m_MenuButton->setVisible(visible);
}

void AuToolButtonExpand::BaseButtonClicked()
{
	emit Clicked();
}

void AuToolButtonExpand::BaseExpandClicked()
{
	emit ExpandClicked();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuToolButtonExpandValue::AuToolButtonExpandValue(int value, QWidget* parent) : AuToolButtonExpand(parent)
{
	m_Value = value;
}

void AuToolButtonExpandValue::BaseButtonClicked()
{
	emit Clicked(m_Value);
}

void AuToolButtonExpandValue::BaseExpandClicked()
{
	emit ExpandClicked(m_Value);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIconButton::AuIconButton(const QIcon& icon, QWidget* parent) : QToolButton(parent)
{
	setAutoRaise(true);
	setIcon(icon);
}

AuIconButton::AuIconButton(const char* imageName, QWidget* parent) : QToolButton(parent)
{
	setAutoRaise(true);
	setIcon(QIcon(imageName));
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuIconButtonData::AuIconButtonData(const QIcon& icon, const QVariant& data, QWidget* parent) : QToolButton(parent), m_Data(data)
{
	setAutoRaise(true);
	setIcon(icon);
}

AuIconButtonData::AuIconButtonData(const char* imageName, const QVariant& data, QWidget* parent) : QToolButton(parent), m_Data(data)
{
	setAutoRaise(true);
	setIcon(QIcon(imageName));
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuToolButtonNumberValue::AuToolButtonNumberValue(int value, QWidget* parent) : QWidget(parent)
{
	m_Value = value;

	m_Button = new QToolButton;
	m_Button->setAutoRaise(true);
	connect(m_Button, SIGNAL(clicked()), SLOT(BaseClicked()));

	m_SpinBox = new QSpinBox;
	m_SpinBox->setAlignment(Qt::AlignRight);
	connect(m_SpinBox, SIGNAL(valueChanged(int)), SLOT(BaseNumberChanged(int)));

	QGridLayout* hlay = new QGridLayout(this);
	hlay->addWidget(m_Button, 0,0);
	hlay->setColumnStretch(0, 1);
	hlay->addWidget(m_SpinBox, 0,1);
	hlay->setMargin(0);
	hlay->setSpacing(2);
	setLayout(hlay);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuHelpButton::AuHelpButton(const QString& topicOrUrl, QWidget* parent) : AuIconButton(":data/hazWiki.png", parent)
{
	m_Help = topicOrUrl;
	connect(this, SIGNAL(clicked()), this, SLOT(ShowHelp()));
}

AuHelpButton::AuHelpButton(QWidget* parent) : AuIconButton(":data/hazWiki.png", parent)
{
	connect(this, SIGNAL(clicked()), this, SLOT(ShowHelp()));
}

void AuHelpButton::ShowHelp() const
{
	if(m_Help.isEmpty())
		return;
	QUrl url = UrlFromString(m_Help);
	if(url != QUrl())
		QDesktopServices::openUrl(url);
	else
	{
		QString wikiUrl = QString("https://www.hazeron.com/wiki/index.php/%1").arg(m_Help);
		url = UrlFromString(wikiUrl);
		if(url != QUrl())
			QDesktopServices::openUrl(url);
	}
}

