//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aublueprintratingdlg.cpp,v 1.4 2022/02/19 18:09:53 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aublueprintratingdlg.h"
//
#include "auexchangerating.h"
#include "auglobal.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QScrollArea>

AuBlueprintRatingDlg::AuBlueprintRatingDlg(quint32 designId, const QString& designName, const AuExchangeBlueprintRating& rating,
		QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Blueprint Ratings and Reviews"));
	setAuDlgIcon(QIcon(":data/OrderReleaseCommand.png"));

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>"
				"<big><b>%1</b></big>"
				"<br>"
				"<i>%2</i>"
				"</p>"
				"</html>").arg(designName).arg(designId), this);
	lbinfo->setWordWrap(true);
	lbinfo->setAlignment(Qt::AlignLeft|Qt::AlignTop);

	QPixmap pxname(rating.RatingAverageIconName());
	QLabel* lbratico = new QLabel(this);
	lbratico->setPixmap(QPixmap(pxname));

	QLabel* lbrating;
	if(rating.Ratings() > 1)
		lbrating = new QLabel(tr("%1 Stars Average of %2 Reviews").arg(rating.RatingAverageString()).arg(rating.Ratings()), this);
	else
		lbrating = new QLabel(tr("%1 Stars from One Review").arg(rating.RatingAverageString()), this);
	lbrating->setWordWrap(true);
	lbrating->setAlignment(Qt::AlignLeft|Qt::AlignTop);

	QHBoxLayout* rlay = new QHBoxLayout;
	rlay->addWidget(lbratico);
	rlay->addWidget(lbrating, 1);

	QVBoxLayout* revlay = new QVBoxLayout;
	ListReviews(*revlay, rating);
	revlay->addStretch();

	QWidget* reviews = new QWidget;
	reviews->setLayout(revlay);

	QScrollArea* scroller = new QScrollArea(this);
	scroller->setWidget(reviews);
	scroller->setWidgetResizable(true);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbinfo);
	lay->addLayout(rlay);
	lay->addWidget(scroller);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuBlueprintRatingDlg::~AuBlueprintRatingDlg()
{
}

void AuBlueprintRatingDlg::setVisible(bool visible)
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

void AuBlueprintRatingDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/BlueprintRatingDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/BlueprintRatingDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/BlueprintRatingDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/BlueprintRatingDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuBlueprintRatingDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/BlueprintRatingDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/BlueprintRatingDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/BlueprintRatingDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/BlueprintRatingDlg/SizeY",sz.height());
}

void AuBlueprintRatingDlg::ListReviews(QVBoxLayout& lay, const AuExchangeBlueprintRating& rating)
{
	QMultiMap<AuTime, QHash<AuId, AuExchangeRating>::const_iterator> sorted;
	for(QHash<AuId, AuExchangeRating>::const_iterator ri = rating.Reviews().cbegin(); ri != rating.Reviews().cend(); ri++)
		sorted.insert((*ri).WhenRated(), ri);

	QPixmap pxline(QString(":data/RatingLine.png"));
	for(QMultiMap<AuTime, QHash<AuId, AuExchangeRating>::const_iterator>::const_iterator si = sorted.cbegin(); si != sorted.cend();
			si++)
	{
		QHash<AuId, AuExchangeRating>::const_iterator ri = *si;
		QPixmap pxstars = QPixmap(AuExchangeBlueprintRating::RatingAverageIconName((*ri).Rating())).scaled(16,16);
		QString comment = (*ri).Comment();
		comment.replace("\n", "<br>");
		QString when = (*ri).WhenRated().UTC().toLocalTime().toString();

		QLabel* lbline = new QLabel(this);
		lbline->setPixmap(pxline);
		lbline->setAlignment(Qt::AlignLeft|Qt::AlignTop);

		QLabel* lbstars = new QLabel(this);
		lbstars->setPixmap(pxstars);
		lbstars->setAlignment(Qt::AlignLeft|Qt::AlignTop);

		QLabel* lbrating = new QLabel(tr("<html>"
					"<small>%4</small><br>"
					"<big><b>%1</b></big> <small><i>%2</i></small>"
					"<div style=\"margin-left: 20px;\">"
					"%3"
					"</div>"
					"</html>")
				.arg((*ri).RaterName())
				.arg(ri.key().StringId())
				.arg(comment)
				.arg(when)
				, this);
		lbrating->setWordWrap(true);
		lbrating->setAlignment(Qt::AlignLeft|Qt::AlignTop);
		lbrating->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);

		QGridLayout* blay = new QGridLayout;
		blay->addWidget(lbline, 0,0, 1,2);
		blay->addWidget(lbstars, 1,0);
		blay->addWidget(lbrating, 1,1);
		blay->setColumnStretch(1,1);
		lay.addLayout(blay);
	}
}
