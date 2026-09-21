//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aurateblueprint.cpp,v 1.3 2022/02/19 18:16:36 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aurateblueprint.h"
//
#include "auglobal.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QTextEdit>
#include <QToolButton>

AuRateBlueprint::AuRateBlueprint(quint32 designId, const QString& designName, const QString& designReport, QWidget* parent) :
	AuDialog(parent)
{
	QLabel* lbname = new QLabel(tr("<html>"
				"<p>"
				"<big><b>%1</b></big>"
				"<br>"
				"<i>%2</i>"
				"</p>"
				"<p>"
				"A blueprint can be rated and reviewed by each avatar only once. "
				"Rating a blueprint replaces any previous rating and review by the avatar. "
				"</p>"
				"</html>").arg(designName).arg(designId), this);
	lbname->setWordWrap(true);

	m_Rating = 0;
	setAuDlgTitle(tr("Rate Blueprint"));
	setAuDlgIcon(QIcon(":data/OrderTakeCommand.png"));

	m_Rate1 = new QToolButton(this);
	m_Rate1->setIcon(QIcon(":data/desired10x10.png"));
	m_Rate1->setAutoRaise(true);
	connect(m_Rate1, SIGNAL(clicked(bool)), SLOT(Rate1Clicked()));

	m_Rate2 = new QToolButton(this);
	m_Rate2->setIcon(QIcon(":data/desired10x10.png"));
	m_Rate2->setAutoRaise(true);
	connect(m_Rate2, SIGNAL(clicked(bool)), SLOT(Rate2Clicked()));

	m_Rate3 = new QToolButton(this);
	m_Rate3->setIcon(QIcon(":data/desired10x10.png"));
	m_Rate3->setAutoRaise(true);
	connect(m_Rate3, SIGNAL(clicked(bool)), SLOT(Rate3Clicked()));

	m_Rate4 = new QToolButton(this);
	m_Rate4->setIcon(QIcon(":data/desired10x10.png"));
	m_Rate4->setAutoRaise(true);
	connect(m_Rate4, SIGNAL(clicked(bool)), SLOT(Rate4Clicked()));

	m_Rate5 = new QToolButton(this);
	m_Rate5->setIcon(QIcon(":data/desired10x10.png"));
	m_Rate5->setAutoRaise(true);
	connect(m_Rate5, SIGNAL(clicked(bool)), SLOT(Rate5Clicked()));

	m_Comment = new QTextEdit(this);
	m_Comment->setToolTip(tr("<html>"
				"Write a review about the blueprint. "
				"A review is optional. "
				"</html>"));

	m_DesignReport = new QTextBrowser;
	m_DesignReport->setReadOnly(true);
	m_DesignReport->setHtml(designReport);

	m_Ok = new QPushButton("Ok", this);
	m_Ok->setAutoDefault(true);
	m_Ok->setDefault(true);
	m_Ok->setEnabled(false);
	connect(m_Ok, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(m_Ok);

	QHBoxLayout* rlay = new QHBoxLayout;
	rlay->addStretch();
	rlay->addWidget(m_Rate1);
	rlay->addWidget(m_Rate2);
	rlay->addWidget(m_Rate3);
	rlay->addWidget(m_Rate4);
	rlay->addWidget(m_Rate5);
	rlay->addStretch();

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbname);
	lay->addLayout(rlay);
	lay->addWidget(m_Comment);
	lay->addLayout(blay);
	lay->addWidget(m_DesignReport);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuRateBlueprint::~AuRateBlueprint()
{
}

void AuRateBlueprint::setVisible(bool visible)
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

void AuRateBlueprint::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/RateBlueprint/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/RateBlueprint/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/RateBlueprint/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/RateBlueprint/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuRateBlueprint::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/RateBlueprint/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/RateBlueprint/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/RateBlueprint/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/RateBlueprint/SizeY",sz.height());
}

QString AuRateBlueprint::Comment() const
{
	return m_Comment->toPlainText();
}

void AuRateBlueprint::Rate1Clicked()
{
	m_Rating = 1;
	ShowRating();
	m_Ok->setEnabled(true);
}

void AuRateBlueprint::Rate2Clicked()
{
	m_Rating = 2;
	ShowRating();
	m_Ok->setEnabled(true);
}

void AuRateBlueprint::Rate3Clicked()
{
	m_Rating = 3;
	ShowRating();
	m_Ok->setEnabled(true);
}

void AuRateBlueprint::Rate4Clicked()
{
	m_Rating = 4;
	ShowRating();
	m_Ok->setEnabled(true);
}

void AuRateBlueprint::Rate5Clicked()
{
	m_Rating = 5;
	ShowRating();
	m_Ok->setEnabled(true);
}

void AuRateBlueprint::ShowRating()
{
	// Put stars on the buttons to show the rating.
	QIcon icoStar(":data/OrderTakeCommand.png");
	QIcon icoDot(":data/desired10x10.png");
	switch(m_Rating)
	{
	case 1:
		m_Rate1->setIcon(icoStar);
		m_Rate2->setIcon(icoDot);
		m_Rate3->setIcon(icoDot);
		m_Rate4->setIcon(icoDot);
		m_Rate5->setIcon(icoDot);
		break;
	case 2:
		m_Rate1->setIcon(icoStar);
		m_Rate2->setIcon(icoStar);
		m_Rate3->setIcon(icoDot);
		m_Rate4->setIcon(icoDot);
		m_Rate5->setIcon(icoDot);
		break;
	case 3:
		m_Rate1->setIcon(icoStar);
		m_Rate2->setIcon(icoStar);
		m_Rate3->setIcon(icoStar);
		m_Rate4->setIcon(icoDot);
		m_Rate5->setIcon(icoDot);
		break;
	case 4:
		m_Rate1->setIcon(icoStar);
		m_Rate2->setIcon(icoStar);
		m_Rate3->setIcon(icoStar);
		m_Rate4->setIcon(icoStar);
		m_Rate5->setIcon(icoDot);
		break;
	case 5:
		m_Rate1->setIcon(icoStar);
		m_Rate2->setIcon(icoStar);
		m_Rate3->setIcon(icoStar);
		m_Rate4->setIcon(icoStar);
		m_Rate5->setIcon(icoStar);
		break;
	default:
		m_Rate1->setIcon(icoDot);
		m_Rate2->setIcon(icoDot);
		m_Rate3->setIcon(icoDot);
		m_Rate4->setIcon(icoDot);
		m_Rate5->setIcon(icoDot);
		break;
	}
}
