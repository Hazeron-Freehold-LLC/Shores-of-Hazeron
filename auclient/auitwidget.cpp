//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auitwidget.cpp,v 1.2 2022/02/19 18:16:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auitwidget.h"
//
#include "auclientglobal.h"
#include "audesign.h"
#include <QHeaderView>

AuItWidget::AuItWidget(const AuId& architectId, bool publishMode, QWidget* parent) : QTableWidget(0,3, parent)
{
	m_ArchitectId = architectId;
	m_CurrentId = 0;
	m_IsPublishMode = publishMode;

	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setShowGrid(false);

	QStringList headerLabels;
	headerLabels << tr("Blueprint");
	headerLabels << tr("Name");
	headerLabels << tr("State");
	setHorizontalHeaderLabels(headerLabels);

	setSortingEnabled(true);
	sortByColumn(1, Qt::AscendingOrder);

	QHeaderView* hhdr = horizontalHeader();
	hhdr->setSectionResizeMode(0, QHeaderView::Interactive);
	hhdr->setSectionResizeMode(1, QHeaderView::Stretch);
	hhdr->setSectionResizeMode(2, QHeaderView::Interactive);

	verticalHeader()->hide();

	RefreshContents();
}

AuItWidget::~AuItWidget()
{
}

void AuItWidget::RefreshContents()
{
	//qDebug() << "AuItWidget::RefreshContents" << m_CurrentId;
	// Do this quietly.
	blockSignals(true);

	// We will sort the contents when done. Preserve the sort column and disable sorting so it doesn't interfere with the fill.
	int sortSection = horizontalHeader()->sortIndicatorSection();
	setSortingEnabled(false);

	// Remove current contents.
	clearContents();
	while(rowCount())
		removeRow(0);

	// Add known library designs.
	const QHash<quint32, AuBdDesignListEntry>& designs = g_AuClient.LockBdDesignList();
	if(m_IsPublishMode)
	{
		// Show only the architect's final designs when choosing to publish one.
		for(QHash<quint32, AuBdDesignListEntry>::const_iterator di = designs.cbegin(); di != designs.cend(); di++)
			if((*di).ArchitectId() == m_ArchitectId &&
					((*di).State() == AuDesign::Final || (*di).State() == AuDesign::FinalAssembly))
				AddDesign(di.key(), *di);
	}
	else
	{
		// Show all the architect's designs when not choosing to publish one.
		for(QHash<quint32, AuBdDesignListEntry>::const_iterator di = designs.cbegin(); di != designs.cend(); di++)
			if((*di).ArchitectId() == m_ArchitectId)
				AddDesign(di.key(), *di);
	}
	g_AuClient.UnlockBdDesignList();

	// Restore sorting and sort the result.
	setSortingEnabled(true);
	sortItems(sortSection);

	// Reselect the current design.
	if(m_CurrentId)
		SetCurrent(m_CurrentId);

	// Resume normal signal emissions.
	blockSignals(false);
}

void AuItWidget::AddDesign(quint32 designId, const AuBdDesignListEntry& design)
{
	//qDebug() << "AuItWidget::AddDesign" << designId << design.Name() << design.StateName();
	insertRow(0);

	QIcon icon;
	if(design.State() == AuDesign::Final)
		icon = QIcon(":data/Inventory.png");
	else
		icon = QIcon(":data/c_ItDesignDrawing.png");

	int col=0;
	QTableWidgetItem* twid = new QTableWidgetItem(icon, QVariant(designId).toString());
	twid->setData(Qt::UserRole, designId);						// Design id stored on id column.
	QString t = QString("%1").arg(designId, (int)10, (int)10, QLatin1Char('0'));		// Sorting role needs leading zeros.
	twid->setData(Qt::UserRole+1, t);							// Sorting role
	setItem(0,col++, twid);

	QTableWidgetItem* twname = new QTableWidgetItem(design.Name());
	twname->setData(Qt::UserRole, design.ReportText());			// Report text is stored on name column.
	twname->setData(Qt::UserRole+1, design.Name());				// Sorting role
	setItem(0,col++, twname);

	QTableWidgetItem* twstate = new QTableWidgetItem(design.StateName());
	twstate->setData(Qt::UserRole, design.State());
	twstate->setData(Qt::UserRole+1, design.StateName());		// Sorting role
	setItem(0,col++, twstate);
}

int AuItWidget::CurrentRow() const
{
	QItemSelectionModel* ism = selectionModel();
	if(!ism)
		return -1;

	QModelIndexList sel = ism->selectedIndexes();
	if(sel.empty())
		return -1;

	QModelIndexList::const_iterator si = sel.cbegin();
	if(!(*si).isValid())
		return -1;

	return (*si).row();
}

quint32 AuItWidget::CurrentDesignId() const
{
	int row = CurrentRow();
	if(row < 0)
		return 0;

	QTableWidgetItem* twid = item(row, 0);
	if(!twid)
		return 0;

	return twid->data(Qt::UserRole).toUInt();
}

QString AuItWidget::CurrentDesignName() const
{
	int row = CurrentRow();
	if(row < 0)
		return QString();

	QTableWidgetItem* twname = item(row, 1);
	if(!twname)
		return QString();

	return twname->text();
}

QString AuItWidget::CurrentDesignReport() const
{
	int row = CurrentRow();
	if(row < 0)
		return QString();

	QTableWidgetItem* twname = item(row, 1);
	if(!twname)
		return QString();

	return twname->data(Qt::UserRole).toString();
}

quint8 AuItWidget::CurrentDesignState() const
{
	int row = CurrentRow();
	if(row < 0)
		return AuDesign::Deleted;

	QTableWidgetItem* twstate = item(row, 2);
	if(!twstate)
		return AuDesign::Deleted;

	return twstate->data(Qt::UserRole).toUInt();
}

void AuItWidget::SetCurrent(quint32 designId)
{
	//qDebug() << "AuItWidget::SetCurrent" << designId;
	// Remember id in case the row isn't here yet. Refresh may not be done yet.
	m_CurrentId = designId;

	// Select the row if it's here.
	for(int row=0; row<rowCount(); row++)
	{
		QTableWidgetItem* twid = item(row, 0);
		if(twid && twid->data(Qt::UserRole) != designId)
			continue;
		selectRow(row);
		return;
	}
}

void AuItWidget::SelectedDesigns(QHash<quint32, QString>& designs) const
{
	QItemSelectionModel* ism = selectionModel();
	if(!ism)
		return;

	QModelIndexList sel = ism->selectedIndexes();
	if(sel.empty())
		return;

	for(QModelIndexList::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		int row = (*si).row();
		QTableWidgetItem* twid = item(row, 0);
		if(!twid)
			continue;
		QTableWidgetItem* twname = item(row, 1);
		if(!twname)
			continue;
		quint32 designId = twid->data(Qt::UserRole).toUInt();
		QString designName = twname->text();
		designs.insert(designId, designName);
	}
}

void AuItWidget::SelectedDesigns(QHash<quint32, QString>& designs, quint8 designState) const
{
	QItemSelectionModel* ism = selectionModel();
	if(!ism)
		return;

	QModelIndexList sel = ism->selectedIndexes();
	if(sel.empty())
		return;

	for(QModelIndexList::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
	{
		int row = (*si).row();
		QTableWidgetItem* twid = item(row, 0);
		if(!twid)
			continue;
		QTableWidgetItem* twname = item(row, 1);
		if(!twname)
			continue;
		QTableWidgetItem* twstate = item(row, 2);
		if(!twstate)
			continue;
		quint8 state = twstate->data(Qt::UserRole).toUInt();
		if(state != designState)
			continue;
		quint32 designId = twid->data(Qt::UserRole).toUInt();
		QString designName = twname->text();
		designs.insert(designId, designName);
	}
}
