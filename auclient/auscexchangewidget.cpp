//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscexchangewidget.cpp,v 1.8 2022/02/19 18:16:43 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auscexchangewidget.h"
//
#include "auclientglobal.h"
#include "audesign.h"
#include "auexchangerating.h"
#include "autablewidgetitem.h"
#include "dgdata.h"
#include <QHeaderView>

AuScExchangeWidget::AuScExchangeWidget(QWidget* parent) : QTableWidget(0,11, parent)
{
	m_CurrentId = 0;

	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setShowGrid(false);

	QStringList headerLabels;
	headerLabels << tr("Blueprint");
	headerLabels << tr("Name");
	headerLabels << tr("Rating");
	headerLabels << tr("Structure");
	headerLabels << tr("Q");
	headerLabels << tr("Size");
	headerLabels << tr("State");
	headerLabels << tr("Architect");
	headerLabels << tr("Empire");
	headerLabels << tr("Terms");
	headerLabels << tr("Published");
	setHorizontalHeaderLabels(headerLabels);

	setSortingEnabled(true);
	sortByColumn(1, Qt::AscendingOrder);

	QHeaderView* hhdr = horizontalHeader();
	hhdr->setSectionResizeMode(0, QHeaderView::Interactive);
	hhdr->setSectionResizeMode(1, QHeaderView::Stretch);
	hhdr->setSectionResizeMode(2, QHeaderView::Interactive);
	hhdr->setSectionResizeMode(3, QHeaderView::Interactive);
	hhdr->setSectionResizeMode(4, QHeaderView::Interactive);
	hhdr->setSectionResizeMode(5, QHeaderView::Interactive);
	hhdr->setSectionResizeMode(6, QHeaderView::Interactive);
	hhdr->setSectionResizeMode(7, QHeaderView::Interactive);
	hhdr->setSectionResizeMode(8, QHeaderView::Interactive);
	hhdr->setSectionResizeMode(9, QHeaderView::Interactive);
	hhdr->setSectionResizeMode(10, QHeaderView::Interactive);

	verticalHeader()->hide();

	RefreshContents();
}

AuScExchangeWidget::~AuScExchangeWidget()
{
}

void AuScExchangeWidget::RefreshContents()
{
	//qDebug() << "AuScExchangeWidget::RefreshContents" << m_CurrentId;
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
	QHash<quint32, AuScDesignExchangeEntryClient>& designs = g_AuClient.LockScDesignExchange();
	for(QHash<quint32, AuScDesignExchangeEntryClient>::const_iterator di = designs.cbegin(); di != designs.cend(); di++)
		AddDesign(di.key(), *di);
	g_AuClient.UnlockScDesignExchange();

	// Restore sorting and sort the result.
	setSortingEnabled(true);
	sortItems(sortSection);

	// Reselect the current design.
	if(m_CurrentId)
		SetCurrent(m_CurrentId);

	// Resume normal signal emissions.
	blockSignals(false);

	if(CurrentDesignId() != m_CurrentId)
		emit itemSelectionChanged();
}

void AuScExchangeWidget::AddDesign(quint32 designId, const AuScDesignExchangeEntryClient& design)
{
	insertRow(0);

	QIcon icon(":data/Exchange.png");

	int col=0;
	QTableWidgetItem* twid = new QTableWidgetItem(icon, QVariant(designId).toString());
	twid->setData(Qt::UserRole, designId);						// Design id stored on id column.
	setItem(0,col++, twid);

	QTableWidgetItem* twname = new QTableWidgetItem(design.DesignName());
	twname->setData(Qt::UserRole, design.ReportText());			// Report text is stored on name column.
	setItem(0,col++, twname);

	// Rating
	QString srating;
	QIcon rateIcon;
	if(design.Rating() >= 1)
	{
		srating = AuExchangeBlueprintRating::RatingAverageString(design.Rating());
		rateIcon = QIcon(AuExchangeBlueprintRating::RatingAverageIconName(design.Rating()));
	}
	QTableWidgetItem* twrat = new QTableWidgetItem(rateIcon, srating);
	if(!srating.isEmpty())
		twrat->setToolTip(tr("<html>"
					"Rated %1 stars out of %L2 reviews."
					"</html>")
				.arg(srating)
				.arg(design.Raters()));
	else
		twrat->setToolTip(tr("<html>"
					"This blueprint has not been rated."
					"</html>"));
	setItem(0,col++, twrat);

	// Structure
	SPACECRAFT_STRUCTURE scStruct = design.DesignStructure();
	QTableWidgetItem* twstruct = new QTableWidgetItem(QIcon(SpacecraftStructureIcon(scStruct)), SpacecraftStructureName(scStruct));
	setItem(0,col++, twstruct);

	QTableWidgetItem* twtl = new QTableWidgetItem(QString("Q%1").arg(design.DesignQuality()));
	setItem(0,col++, twtl);

	// Size
	QString svol;
	if(design.DesignState() == AuDesign::Final)
	{
		if(design.FaceCount() || design.TriangleCount())
			svol = QString("%L1m%2 %L3 faces %L4 tris")
				.arg(design.HullVolume())
				.arg(g_Au.CubedSymbol())
				.arg(design.FaceCount())
				.arg(design.TriangleCount());
		else
			svol = QString("%L1m%2")
				.arg(design.HullVolume())
				.arg(g_Au.CubedSymbol());
	}
	else
	{
		if(design.FaceCount() || design.TriangleCount())
			svol = QString("%L1 faces %L2 tris")
				.arg(design.FaceCount())
				.arg(design.TriangleCount());
	}
	QTableWidgetItem* twvol = new QTableWidgetItem(svol);
	setItem(0,col++, twvol);

	QTableWidgetItem* twstate = new QTableWidgetItem(AuDesign::StateName((AuDesign::STATE)design.DesignState()));
	twstate->setData(Qt::UserRole, design.DesignState());
	setItem(0,col++, twstate);

	QString sarch = QString("%1 %2").arg(design.DesignArchitectName()).arg(design.DesignArchitectId().StringId());
	QTableWidgetItem* twarch = new QTableWidgetItem(sarch);
	setItem(0,col++, twarch);

	QString empireName;
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireName(design.PublisherEmpire(), empireName);
	g_Au.UnlockData();
	QTableWidgetItem* twemp = new QTableWidgetItem(empireName);
	setItem(0,col++, twemp);

	// Abbreviate terms.
	QString terms;
	// Price
	if(design.Price())
		terms = tr("%L1%2").arg(design.Price()).arg(g_Au.CurrencySymbol());
	else
		terms = tr("Free");
	// License
	if(design.CanCopy())
		terms += tr(" C");
	else if(design.CanView())
		terms += tr(" V");
	else
		terms += tr(" M");

	// Stance Restriction
	switch(design.MinStanceToBuyer())
	{
	case POL_Vassal:
	case POL_Enemy:
		terms += tr("A");
		break;
	case POL_Neutral:
		terms += tr("N");
		break;
	case POL_Friend:
		terms += tr("F");
		break;
	case POL_Empire:
		terms += tr("E");
		break;
	}

	if(design.DesignState() == AuDesign::Final)
	{
		// Limit
		// UseOnce only applies to designs that are final.
		if(design.UseOnce())
			terms += tr("1");
		else
			terms += tr("U");
	}

	QTableWidgetItem* twterms = new QTableWidgetItem(terms);
	setItem(0,col++, twterms);

	QString date;
	if(design.WhenPublished() != AuTime())
		date = design.WhenPublished().UTC().toLocalTime().toString("yyyy/MM/dd");
	else
		date = tr("Before 2018/09/15");
	AuTableWidgetSortedItem* twdate = new AuTableWidgetSortedItem(design.WhenPublished().MSecsSinceEpoch(), QIcon(), date);
	twdate->setData(Qt::UserRole, design.WhenPublished().UTC());
	setItem(0,col++, twdate);
}

int AuScExchangeWidget::CurrentRow() const
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

quint32 AuScExchangeWidget::CurrentDesignId() const
{
	int row = CurrentRow();
	if(row < 0)
		return 0;

	QTableWidgetItem* twid = item(row, 0);
	if(!twid)
		return 0;

	return twid->data(Qt::UserRole).toUInt();
}

QString AuScExchangeWidget::CurrentDesignName() const
{
	int row = CurrentRow();
	if(row < 0)
		return QString();

	QTableWidgetItem* twname = item(row, 1);
	if(!twname)
		return QString();

	return twname->text();
}

QString AuScExchangeWidget::CurrentDesignReport() const
{
	int row = CurrentRow();
	if(row < 0)
		return QString();

	QTableWidgetItem* twname = item(row, 1);
	if(!twname)
		return QString();

	return twname->data(Qt::UserRole).toString();
}

quint8 AuScExchangeWidget::CurrentDesignState() const
{
	int row = CurrentRow();
	if(row < 0)
		return AuDesign::Deleted;

	QTableWidgetItem* twstate = item(row, 6);
	if(!twstate)
		return AuDesign::Deleted;

	return twstate->data(Qt::UserRole).toUInt();
}

void AuScExchangeWidget::SetCurrent(quint32 designId)
{
	//qDebug() << "AuScExchangeWidget::SetCurrent" << designId;
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

void AuScExchangeWidget::SelectedDesigns(QHash<quint32, QString>& designs) const
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
		QTableWidgetItem* twstate = item(row, 6);
		if(!twstate)
			continue;
		quint32 designId = twid->data(Qt::UserRole).toUInt();
		QString designName = twname->text();
		designs.insert(designId, designName);
	}
}
