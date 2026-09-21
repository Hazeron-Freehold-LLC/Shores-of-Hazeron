//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldshed.cpp,v 1.6 2022/02/19 18:09:51 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubldshed.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "dabd.h"
#include "daperson.h"
#include "daworldll.h"
#include "dgdata.h"
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QTextBrowser>
#include <QTimer>
#include <QVBoxLayout>

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBldShed::AuBldShed(QWidget* parent) : QWidget(parent)
{
	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>"
				"Warehouses, storage tanks and sheds fill up by importing commodities manufactured at buildings. "
				"Storage tanks store a specific type of fuel, determined by the blueprint. "
				"Sheds store tools, spare parts, construction materials and miscellaneous items in small quantities. "
				"Warehouses specialize in storing one specific kind of commodity. "
				"</p>"
				"<p>"
				"Commodities at warehouses and storage tanks are available to manufacturing and construction processes. "
				"</p>"
				"<p>"
				"When space traders ask to buy cargo, brokers sell commodities that are stored in warehouses and storage tanks. "
				"</p>"
				"</html"), this);
	lbinfo->setWordWrap(true);

	// Build the layout.
	QVBoxLayout* lay = new QVBoxLayout();
	lay->addWidget(lbinfo);
	lay->addStretch();
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuBldShed::~AuBldShed()
{
}

void AuBldShed::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Show all my widgets.
	QVBoxLayout* lay = (QVBoxLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->show();
}

void AuBldShed::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Hide my widgets now or they force the parent AuLabor dialog to be too big even though this page isn't visible.
	QVBoxLayout* lay = (QVBoxLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->hide();
}
