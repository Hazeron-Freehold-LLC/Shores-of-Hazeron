//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommstdchannels.cpp,v 1.3 2022/02/19 18:10:35 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucommstdchannels.h"
//
#include <dgdata.h>
#include <QGridLayout>
#include <QIcon>
#include <QListWidget>
#include <QPushButton>

AuCommStdChannels::AuCommStdChannels(const QStringList& openChannels, QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle("Open Standard Channel");

	QListWidget* lw = new QListWidget(this);
	lw->setEditTriggers(QAbstractItemView::NoEditTriggers);
	lw->setObjectName("lwchannels");
	lw->setSelectionBehavior(QAbstractItemView::SelectRows);
	lw->setSelectionMode(QAbstractItemView::SingleSelection);
	lw->setSortingEnabled(true);
	connect(lw, SIGNAL(itemSelectionChanged()), SLOT(EnableControls()));
	connect(lw, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(OpenChannel()));

	const DgData& gd = g_Au.LockDataForRead();
	bool isAgent = gd.ClientPlayerIsAgent();	// also true if programmer or architect
	bool isProgrammer = gd.ClientPlayerIsProgrammer();	// also true if architect
	bool isArchitect = gd.ClientPlayerIsArchitect();
	bool isMinion = gd.ClientPlayerIsMinion();
	g_Au.UnlockData();

	QStringList names;
	AuCommStdChannel::ListStandardChannels(names, isMinion);
	if(isAgent)
		AuCommStdChannel::ListAgentChannels(names);				// Append agent channel(s) to the list.
	if(isProgrammer)
		AuCommStdChannel::ListProgrammerChannels(names);		// Append programmer channel(s) to the list.
	if(isArchitect)
		AuCommStdChannel::ListArchitectChannels(names);			// Append architect channel(s) to the list.
	for(int ic=0; ic<names.count(); ic++)
	{
		if(openChannels.contains(names[ic]))
			continue;

		AuCommStdChannel::SCOPE scope = AuCommStdChannel::Scope(names[ic]);
		QIcon icon = QIcon(QPixmap::fromImage(QImage(AuCommStdChannel::ChannelIconName(names[ic], scope)).scaled(16,16,
						Qt::KeepAspectRatio, Qt::SmoothTransformation)));
		QListWidgetItem* lwi = new QListWidgetItem(icon, names[ic], lw);

		QString desc = QString("<b>%1</b><br>%2").arg(names[ic]).arg(AuCommStdChannel::ChannelDescription(names[ic], scope));
		lwi->setToolTip(desc);
	}

	QPushButton* pbok = new QPushButton(tr("Open Channel"));
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setEnabled(false);
	pbok->setObjectName("pbok");
	connect(pbok, SIGNAL(clicked()), SLOT(OpenChannel()));
	pbok->setToolTip(tr("<b>Open Channel</b><br>Opens the standard channel that is selected."));

	QGridLayout* lay = new QGridLayout();
	int row = 0;	// Using this makes inserting and rearranging rows easier.
	lay->addWidget(lw, row, 0);
	row++;
	lay->addWidget(pbok, row, 0);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuCommStdChannels::~AuCommStdChannels()
{
}

void AuCommStdChannels::EnableControls()
{
	//qDebug("AuCommStdChannels::EnableControls");
	bool enableok = false;
	QListWidget* lw = findChild<QListWidget*>("lwchannels");
	if(lw)
	{
		QItemSelectionModel* ism = lw->selectionModel();
		if(ism && !ism->selection().isEmpty())
			enableok = true;
	}

	QPushButton* pbok = findChild<QPushButton*>("pbok");
	if(pbok)
		pbok->setEnabled(enableok);
}

void AuCommStdChannels::OpenChannel()
{
	//qDebug("AuCommStdChannels::OpenChannel");
	QListWidget* lw = findChild<QListWidget*>("lwchannels");
	if(lw)
	{
		// Get selection values so they can be retrieved after the widget is gone.
		QItemSelectionModel* ism = lw->selectionModel();
		if(ism)
		{
			QModelIndexList sel = ism->selectedIndexes();
			if(!sel.isEmpty())
			{
				QModelIndex idxsel = sel.first();
				if(idxsel.isValid())
				{
					QListWidgetItem* cur = (QListWidgetItem*)idxsel.internalPointer();
					m_ChannelName = cur->text();
					//qDebug("AuCommStdChannels::OpenChannel = " + m_ChannelName);
					accept();
				}
			}
		}
	}
}
