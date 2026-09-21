//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitemdesc.cpp,v 1.8 2022/02/19 18:16:11 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auitemdesc.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auitem.h"
#include "auitemview.h"
#include <QSplitter>
#include <QTextBrowser>
#include <QVBoxLayout>

AuItemDesc::AuItemDesc(const AuItem& item, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(item.SayName());
	setAuDlgIcon(QIcon(item.BitmapName()));

	// Stretch image according to aspect ratio, in pack units, to fit into a 64x64 sized area.
	QSize packSize = item.BitmapSizeToFit(100);
	QString text = QString("<html>"
			"<img align=\"right\" width=\"%3\" height=\"%4\" src=\"qrc%1\">%2"
			"</html>").arg(item.BitmapName()).arg(item.HtmlDesc()).arg(packSize.width()).arg(packSize.height());
	QTextBrowser* desc = new QTextBrowser(this);
	desc->setHtml(text);
	desc->setReadOnly(true);

	if(!auApp->IsHazeronCommander())
	{
		AuItemView* itemView = new AuItemView(this);
		itemView->ViewItem(item);
		itemView->setToolTip(tr("Mouse Controls<br>"
					"%1 Wheel and Ctrl+Wheel to zoom<br>"
					"%1 Left button to spin<br>"
					"%1 Right button to pan.")
				.arg(g_Au.MiddleDotSymbol()));

		// Assemble the widgets into a splitter.
		m_Splitter = new QSplitter;
		m_Splitter->addWidget(desc);
		m_Splitter->addWidget(itemView);
		m_Splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
		connect(m_Splitter, SIGNAL(splitterMoved(int, int)), SLOT(SplitterMoved(int, int)));

		QVBoxLayout* lay = new QVBoxLayout;
		lay->addWidget(m_Splitter);
		lay->setMargin(2);
		lay->setSpacing(2);
		setAuDlgLayout(lay);

		QByteArray splitterstate;
		g_Au.Settings().readEntry("Widget/ItemView/Splitter", splitterstate);
		m_Splitter->restoreState(splitterstate);
	}
	else
	{
		// Assemble the widgets without a GL view.
		m_Splitter = 0;
		QVBoxLayout* lay = new QVBoxLayout;
		lay->addWidget(desc);
		lay->setMargin(2);
		lay->setSpacing(2);
		setAuDlgLayout(lay);
	}
}

AuItemDesc::~AuItemDesc()
{
	//qDebug() << "AuItemDesc::~AuItemDesc";
}

void AuItemDesc::SplitterMoved(int /*pos*/, int /*index*/)
{
	if(m_Splitter)
		g_Au.Settings().writeEntry("Widget/ItemView/Splitter", m_Splitter->saveState());
}

void AuItemDesc::setVisible(bool visible)
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

void AuItemDesc::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/AuItemDesc/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/AuItemDesc/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/AuItemDesc/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/AuItemDesc/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuItemDesc::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/AuItemDesc/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/AuItemDesc/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/AuItemDesc/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/AuItemDesc/SizeY",sz.height());
}
