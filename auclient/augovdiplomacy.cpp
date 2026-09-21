//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovdiplomacy.cpp,v 1.14 2022/05/04 21:26:46 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augovdiplomacy.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auempirewait.h"
#include "auframe.h"
#include "autablewidgetitem.h"
#include "auview.h"
#include "auwarcriteriadlg.h"
#include "daperson.h"
#include "dgdata.h"
#include <QComboBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSlider>
#include <QSplitter>
#include <QTextBrowser>

#define d_NoteLimit						5000					// Character length limit on diplomatic notes.

void AuGovDiplomacyDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleGovernDiplomacyUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDossierWidget::AuDossierWidget(QWidget* parent) : QTableWidget(parent)
{
	setColumnCount(2);
	QTableWidgetItem *colItem = new QTableWidgetItem();
	setHorizontalHeaderItem(0, colItem);
	QTableWidgetItem *colItem1 = new QTableWidgetItem();
	setHorizontalHeaderItem(1, colItem1);
	setFocusPolicy(Qt::NoFocus);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setDragEnabled(false);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setShowGrid(false);
	setGridStyle(Qt::NoPen);
	setCornerButtonEnabled(false);
	setRowCount(0);
	horizontalHeaderItem(0)->setText(tr("Date"));
	horizontalHeaderItem(1)->setText(tr("Subject"));
	horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	verticalHeader()->hide();
	setToolTip(tr("Dossier"));
}

void AuDossierWidget::mouseReleaseEvent(QMouseEvent* event)
{
	QTableWidget::mouseReleaseEvent(event);
	if(event->button() == Qt::RightButton)
	{
		QItemSelectionModel* ism = selectionModel();
		if(ism && !ism->selectedIndexes().isEmpty())
		{
			QMenu menu(this);
			QAction* setNew = menu.addAction(QIcon(AuDocument::StateIcon(AuDocument::DOC_New)), tr("New"));
			connect(setNew, SIGNAL(triggered()), this, SIGNAL(New()));
			QAction* setOpen = menu.addAction(QIcon(AuDocument::StateIcon(AuDocument::DOC_Open)), tr("Open"));
			connect(setOpen, SIGNAL(triggered()), this, SIGNAL(Open()));
			QAction* setPend = menu.addAction(QIcon(AuDocument::StateIcon(AuDocument::DOC_Pending)), tr("Pend"));
			connect(setPend, SIGNAL(triggered()), this, SIGNAL(Pend()));
			QAction* setClose = menu.addAction(QIcon(AuDocument::StateIcon(AuDocument::DOC_Closed)), tr("Close"));
			connect(setClose, SIGNAL(triggered()), this, SIGNAL(Close()));
			QAction* setDelete = menu.addAction(QIcon(AuDocument::StateIcon(AuDocument::DOC_Deleted)), tr("Delete..."));
			connect(setDelete, SIGNAL(triggered()), this, SIGNAL(Delete()));
			menu.exec(QCursor::pos());
		}
	}
}

void AuDossierWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
	QTableWidget::mouseDoubleClickEvent(event);
	if(event->button() == Qt::LeftButton)
		emit DoubleClick();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGovDiplomacy::AuGovDiplomacy(QWidget* parent) : QWidget(parent)
{
	QLabel* lbdefstance = new QLabel(tr("Contact Stance"));

	cbDefStance = new QComboBox;
	cbDefStance->addItem(tr("Friend"));
	cbDefStance->addItem(tr("Neutral"));
	cbDefStance->addItem(tr("Enemy"));
	cbDefStance->setToolTip(tr("<html><b>Contact Stance</b><br>"
				"When an unknown empire is encountered, the contact stances of both empires are compared. "
				"The final contact stance is the least friendly of the two. "
				"It becomes the initial political stance of each empire toward the other.<br>"
				"A dossier on the new empire is started with a document showing the initial contact stance.<br>"
				"<i>Tips</i><br>"
				"%1 When encountering an empire that shares a common overlord with your empire, "
				"contact stance will at be at least neutral.<br>"
				"%1 Only governors with appropriate rights can change this policy.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(cbDefStance, SIGNAL(currentIndexChanged(int)), auApp, SLOT(GovSetDefaultPoliticalStance(int)));

	QPushButton* pbreq = new QPushButton(tr("Request Empires"));
	pbreq->setToolTip(tr("<html><b>Request Empires</b><br>"
				"Requests data about all empires. "
				"This is needed to see the political stance of other empires toward your empire.</html>"));
	connect(pbreq, SIGNAL(clicked(bool)), SLOT(RequestEmpireData()));

	QLabel* lbstance = new QLabel(tr("Known Empires"));
	lbstance->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
	lbstance->setToolTip(tr("<html><b>Political Stances</b><br>"
				"Political stance determines how members of your empire perceive "
				"members of another empire. Political stance also determines the "
				"rewards or penalties for harming units of another empire.</html>"));

	{
		tableEmpires = new QTableWidget(this);
		tableEmpires->setColumnCount(5);
		QTableWidgetItem *colItem = new QTableWidgetItem();
		tableEmpires->setHorizontalHeaderItem(0, colItem);
		QTableWidgetItem *colItem1 = new QTableWidgetItem();
		tableEmpires->setHorizontalHeaderItem(1, colItem1);
		QTableWidgetItem *colItem2 = new QTableWidgetItem();
		tableEmpires->setHorizontalHeaderItem(2, colItem2);
		QTableWidgetItem *colItem3 = new QTableWidgetItem();
		tableEmpires->setHorizontalHeaderItem(3, colItem3);
		QTableWidgetItem *colItem4 = new QTableWidgetItem();
		tableEmpires->setHorizontalHeaderItem(4, colItem4);
		tableEmpires->setFocusPolicy(Qt::NoFocus);
		tableEmpires->setEditTriggers(QAbstractItemView::NoEditTriggers);
		tableEmpires->setDragEnabled(false);
		tableEmpires->setAlternatingRowColors(true);
		tableEmpires->setSelectionMode(QAbstractItemView::SingleSelection);
		tableEmpires->setSelectionBehavior(QAbstractItemView::SelectRows);
		tableEmpires->setShowGrid(false);
		tableEmpires->setGridStyle(Qt::NoPen);
		tableEmpires->setSortingEnabled(true);
		tableEmpires->setCornerButtonEnabled(false);
		tableEmpires->setRowCount(0);
		tableEmpires->horizontalHeaderItem(0)->setText(tr("Name"));
		tableEmpires->horizontalHeaderItem(1)->setText(tr("Dossier"));
		tableEmpires->horizontalHeaderItem(2)->setText(tr("Our Stance Toward Them"));
		tableEmpires->horizontalHeaderItem(3)->setText(tr("Their Stance Toward Us"));
		tableEmpires->horizontalHeaderItem(4)->setText(tr("First Contact"));
		tableEmpires->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
		tableEmpires->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
		tableEmpires->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
		tableEmpires->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Interactive);
		tableEmpires->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Interactive);
		tableEmpires->verticalHeader()->hide();
		connect(tableEmpires, SIGNAL(itemSelectionChanged()), SLOT(EmpireSelectionChanged()));
	}

	m_Dossier = new QTextBrowser();
	m_Dossier->setReadOnly(true);
	//m_Dossier->setOpenExternalLinks(true);

	tableDossier = new AuDossierWidget(this);
	connect(tableDossier, SIGNAL(itemSelectionChanged()), SLOT(DossierSelectionChanged()));
	connect(tableDossier, SIGNAL(New()), SLOT(NewDoc()));
	connect(tableDossier, SIGNAL(Open()), SLOT(OpenDoc()));
	connect(tableDossier, SIGNAL(Pend()), SLOT(PendDoc()));
	connect(tableDossier, SIGNAL(Close()), SLOT(CloseDoc()));
	connect(tableDossier, SIGNAL(Delete()), SLOT(DeleteDoc()));
	connect(tableDossier, SIGNAL(DoubleClick()), SLOT(DoubleClickDoc()));

	// Assemble the splitters.
	m_DosSplitter = new QSplitter;
	m_DosSplitter->setOrientation(Qt::Horizontal);
	m_DosSplitter->addWidget(tableDossier);
	m_DosSplitter->addWidget(m_Dossier);
	m_DosSplitter->setCollapsible(0, false);

	m_Splitter = new QSplitter;
	m_Splitter->setOrientation(Qt::Vertical);
	m_Splitter->addWidget(tableEmpires);
	m_Splitter->addWidget(m_DosSplitter);
	m_Splitter->setCollapsible(0, false);
	m_Splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	connect(m_Splitter, SIGNAL(splitterMoved(int, int)), SLOT(SplitterMoved(int, int)));

	pbenemy = new QPushButton(tr("Enemy"));
	pbenemy->setToolTip(tr("<html><b>Enemy</b><br>"
				"Set the political stance toward the selected empire to 'Enemy'.<br>"
				"<i>Tips</i><br>"
				"%1 Only governors with appropriate rights can change political stance toward known empires policy.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(pbenemy, SIGNAL(clicked(bool)), SLOT(SetEnemy()));

	pbfriend = new QPushButton(tr("Friend"));
	pbfriend->setToolTip(tr("<html><b>Friend</b><br>"
				"Set the political stance toward the selected empire to 'Friend'.<br>"
				"<i>Tips</i><br>"
				"%1 Only governors with appropriate rights can change political stance toward known empires policy.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(pbfriend, SIGNAL(clicked(bool)), SLOT(SetFriend()));

	pbneutral = new QPushButton(tr("Neutral"));
	pbneutral->setToolTip(tr("<html><b>Neutral</b><br>"
				"Set the political stance toward the selected empire to 'Neutral'.<br>"
				"<i>Tips</i><br>"
				"%1 Only governors with appropriate rights can change political stance toward known empires policy.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(pbneutral, SIGNAL(clicked(bool)), SLOT(SetNeutral()));

	pbally = new QPushButton(tr("Ally"));
	pbally->setToolTip(tr("<html><b>Ally</b><br>"
				"Set the political stance toward the selected empire to 'Ally'. "
				"A tribute up to 9% may be set. "
				"Our capital will pay tribute to their capital once per day, if a tribute is set.<br>"
				"<i>Tips</i><br>"
				"%1 Only governors with appropriate rights can change political stance toward known empires policy.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(pbally, SIGNAL(clicked(bool)), SLOT(SetAlly()));

	pbvassal = new QPushButton(tr("Vassal..."));
	pbvassal->setToolTip(tr("<html><b>Vassal...</b><br>"
				"Set the political stance toward the selected empire to 'Vassal'. "
				"A tribute of 10% to 100% must be set. "
				"Our capital will pay tribute to their capital once per day.<br>"
				"<i>Tips</i><br>"
				"%1 Only governors with appropriate rights can change political stance toward known empires policy.<br>"
				"%1 The sworn empire may not have a political stance of 'enemy' toward the vassal.<br>"
				"%1 Vassal stance is automatically broken if the sworn empire downgrades their stance toward the vassal. "
				"The vassal stance changes to match the downgraded stance toward the vassal.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(pbvassal, SIGNAL(clicked(bool)), SLOT(SetVassal()));

	m_WarCriteria = new QPushButton(QPixmap(":data/DocWar.png"), tr("War Policy..."));
	m_WarCriteria->setToolTip(tr("<html><b>War Policy...</b><br>"
				"War policy determines what conditions cause our empire's political stance toward a neutral empire to "
				"automatically change to enemy.<br>"
				"<i>Tips</i><br>"
				"%1 Only governors with appropriate rights can change war policy.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(m_WarCriteria, SIGNAL(clicked(bool)), SLOT(WarCriteria()));

	m_DispatchCourier = new QPushButton(QPixmap(":data/ParcXLogo.png"), tr("Dispatch Courier..."));
	m_DispatchCourier->setToolTip(tr("<html><b>Dispatch Courier...</b><br>"
				"Dispatches a courier with a diplomatic message for the selected empire. "
				"The message appears as a new document in the other empire's dossier on your empire. "
				"The message is also added to your dossier on the other empire.<br>"
				"<i>Tips</i><br>"
				"%1 Only governors can send diplomatic messages.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(m_DispatchCourier, SIGNAL(clicked(bool)), SLOT(DispatchCourier()));

	m_AddNote = new QPushButton(QPixmap(":data/note12x12.png"), tr("Add Note..."));
	m_AddNote->setToolTip(tr("<html><b>Add Note...</b><br>"
				"Adds a note to the dossier on the selected empire.<br>"
				"<i>Tips</i><br>"
				"%1 Only governors with appropriate rights can manage the dossier of known empires.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(m_AddNote, SIGNAL(clicked(bool)), SLOT(AddNote()));

	m_NewDoc = new QPushButton(QPixmap(AuDocument::StateIcon(AuDocument::DOC_New)), tr("New"));
	m_NewDoc->setToolTip(tr("<html><b>New</b><br>"
				"Sets the status to <i>New</i> on the selected documents in the dossier.<br>"
				"<i>Tips</i><br>"
				"%1 All documents added to the dossier begin with New status.<br>"
				"%1 New documents are a factor when checking automatic war policy.<br>"
				"%1 Only governors with appropriate rights can manage the dossier of known empires.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(m_NewDoc, SIGNAL(clicked(bool)), SLOT(NewDoc()));

	m_OpenDoc = new QPushButton(QPixmap(AuDocument::StateIcon(AuDocument::DOC_Open)), tr("Open"));
	m_OpenDoc->setToolTip(tr("<html><b>Open</b><br>"
				"Opens the selected documents in the dossier.<br>"
				"<i>Tips</i><br>"
				"%1 Open documents are a factor when checking automatic war policy.<br>"
				"%1 Only governors with appropriate rights can manage the dossier of known empires.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(m_OpenDoc, SIGNAL(clicked(bool)), SLOT(OpenDoc()));

	m_CloseDoc = new QPushButton(QPixmap(AuDocument::StateIcon(AuDocument::DOC_Closed)), tr("Close"));
	m_CloseDoc->setToolTip(tr("<html><b>Close</b><br>"
				"Closes the selected documents in the dossier.<br>"
				"<i>Tips</i><br>"
				"%1 Closed documents are ignored when checking automatic war policy.<br>"
				"%1 Only governors with appropriate rights can manage the dossier of known empires.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(m_CloseDoc, SIGNAL(clicked(bool)), SLOT(CloseDoc()));

	m_PendDoc = new QPushButton(QPixmap(AuDocument::StateIcon(AuDocument::DOC_Pending)), tr("Pend"));
	m_PendDoc->setToolTip(tr("<html><b>Pend</b><br>"
				"Pends the selected documents in the dossier.<br>"
				"<i>Tips</i><br>"
				"%1 Pending documents are ignored when checking automatic war policy.<br>"
				"%1 Only governors with appropriate rights can manage the dossier of known empires.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(m_PendDoc, SIGNAL(clicked(bool)), SLOT(PendDoc()));

	m_DeleteDoc = new QPushButton(QPixmap(AuDocument::StateIcon(AuDocument::DOC_Deleted)), tr("Delete..."));
	m_DeleteDoc->setToolTip(tr("<html><b>Delete...</b><br>"
				"Deletes the selected documents from the dossier.<br>"
				"<i>Tips</i><br>"
				"%1 Deleted documents are permanently removed from the dossier.<br>"
				"%1 Only governors with appropriate rights can manage the dossier of known empires.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(m_DeleteDoc, SIGNAL(clicked(bool)), SLOT(DeleteDoc()));

	QComboBox* cbDblClickDoc = new QComboBox;
	cbDblClickDoc->addItem(QIcon(AuDocument::StateIcon(AuDocument::DOC_New)), tr("New"), AuDocument::DOC_New);
	cbDblClickDoc->addItem(QIcon(AuDocument::StateIcon(AuDocument::DOC_Open)), tr("Open"), AuDocument::DOC_Open);
	cbDblClickDoc->addItem(QIcon(AuDocument::StateIcon(AuDocument::DOC_Pending)), tr("Pend"), AuDocument::DOC_Pending);
	cbDblClickDoc->addItem(QIcon(AuDocument::StateIcon(AuDocument::DOC_Closed)), tr("Close"), AuDocument::DOC_Closed);
	cbDblClickDoc->addItem(QIcon(AuDocument::StateIcon(AuDocument::DOC_Deleted)), tr("Delete..."), AuDocument::DOC_Deleted);
	cbDblClickDoc->setToolTip(tr("<html><b>Double-click Status</b><br>"
				"Status to apply to dossier documents when double-clicked using the left mouse button."
				"</html>"));
	cbDblClickDoc->setCurrentIndex(cbDblClickDoc->findData(g_AuClient.DossierDoubleClickStatus()));
	connect(cbDblClickDoc, SIGNAL(currentIndexChanged(int)), SLOT(SetDoubleClickStatus(int)));

	QVBoxLayout* vlay = new QVBoxLayout;
	vlay->addWidget(pbally);
	vlay->addWidget(pbvassal);
	vlay->addWidget(pbfriend);
	vlay->addWidget(pbneutral);
	vlay->addWidget(pbenemy);
	vlay->addStretch();
	vlay->addWidget(lbdefstance);
	vlay->addWidget(cbDefStance);
	vlay->addWidget(m_WarCriteria);
	vlay->addStretch();
	vlay->addWidget(m_DispatchCourier);
	vlay->addWidget(m_AddNote);
	vlay->addStretch();
	vlay->addWidget(m_NewDoc);
	vlay->addWidget(m_OpenDoc);
	vlay->addWidget(m_PendDoc);
	vlay->addWidget(m_CloseDoc);
	vlay->addWidget(m_DeleteDoc);
	vlay->addStretch();
	vlay->addWidget(cbDblClickDoc);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbstance, row,1, 1,2);
	lay->addWidget(pbreq, row,2);
	row++;
	lay->addLayout(vlay, row,0);
	lay->addWidget(m_Splitter, row,1, 1,2);
	row++;
	lay->setColumnStretch(1, 10);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);

	QByteArray splitterstate;
	g_Au.Settings().readEntry("Widget/GovDiplomacy/Splitter", splitterstate);
	m_Splitter->restoreState(splitterstate);

	connect(this, SIGNAL(DossierAddNote(quint32, QString)), auApp, SLOT(GovDossierAddNote(quint32, QString)));
	connect(this, SIGNAL(DossierDispatchCourier(quint32, QString)), auApp, SLOT(GovDossierDispatchCourier(quint32, QString)));
	connect(this, SIGNAL(DossierDocState(quint32, AuTime, quint8, quint16, quint8)),
			auApp, SLOT(GovDossierDocState(quint32, AuTime, quint8, quint16, quint8)));
	connect(this, SIGNAL(SetWarCriteria(AuWarCriteria)), auApp, SLOT(GovSetWarCriteria(AuWarCriteria)));
	connect(this, SIGNAL(SetStance(quint32, quint8, quint8)), auApp, SLOT(SetPoliticalStance(quint32, quint8, quint8)));
}

void AuGovDiplomacy::SplitterMoved(int /*pos*/, int /*index*/)
{
	g_Au.Settings().writeEntry("Widget/GovDiplomacy/Splitter", m_Splitter->saveState());
}

void AuGovDiplomacy::EmpireDataComplete()
{
	UpdateEmpireRecord();
	emit EmpireDataCompleteSignal();
}

void AuGovDiplomacy::UpdateEmpireRecord()
{
	// If this widget is not visible, just clear the empire id to 0.
	// setVisible calls us to really update the page when we become visible.
	if(!isVisible())
	{
		//qDebug("AuGovDiplomacy::UpdateEmpireRecord page is not visible. Postponing update until page becomes visible.");
		m_Empire.m_id = 0;
		return;
	}

	//qDebug("AuGovDiplomacy::UpdateEmpireRecord page is visible. Update contents.");
	// Get new empire record data
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
		m_Empire.m_id = person->EmpireId();
	pView->UnlockSceneAtom();

	// Acquire a copy of the empire record.
	const DgData& gd = g_Au.LockDataForRead();
	gd.Empire(m_Empire);
	g_Au.UnlockData();

	int defaultStance = m_Empire.m_DefaultStance;

	cbDefStance->blockSignals(true);
	cbDefStance->setCurrentIndex(defaultStance);
	cbDefStance->blockSignals(false);

	// Get selected empire to restore it when we're done.
	quint32 selectedEmpire;
	QTableWidgetItem* selectedEmpireItem = tableEmpires->item(tableEmpires->currentRow(), 0);
	if(selectedEmpireItem)
		selectedEmpire = selectedEmpireItem->data(Qt::UserRole).toUInt();
	else
		selectedEmpire = 0;
	selectedEmpireItem = 0;

	// Remove current empire list
	int iCount = tableEmpires->rowCount();
	for(int i=0; i<iCount; i++)
		tableEmpires->removeRow(0);

	QImage flag;
	int iRow = 0;
	tableEmpires->setSortingEnabled(false);
	unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::const_iterator itEmpire;
	for(itEmpire = m_Empire.m_Stance.begin(); itEmpire != m_Empire.m_Stance.end(); ++itEmpire)
	{
		// That other empire's stance toward me.
		AuTime whenSet;
		int tribute = 0;
		bool hasEmpire;
		int citizenOnLine;

		const DgData& gd = g_Au.LockDataForRead();
		DgEmpireHash::const_iterator ei = gd.m_gEmpires.find((*itEmpire).second.m_id);
		if(ei != gd.m_gEmpires.end())
		{
			hasEmpire = true;
			whenSet = (*ei).second.WhenSet(m_Empire.m_id);
			tribute = (*ei).second.TributePercent(m_Empire.m_id);
			flag = (*ei).second.m_flag;
			if((*ei).second.IsPirate())
				citizenOnLine = 3;
			else
				citizenOnLine = (*ei).second.IsCitizenOnLine() ? 1 : 0;
		}
		else
		{
			hasEmpire = false;
			citizenOnLine = 2;	// unknown
		}
		g_Au.UnlockData();

		tableEmpires->insertRow(iRow);
		int iColumn = 0;
		QTableWidgetItem *itemName;
		if(hasEmpire)
			itemName = new QTableWidgetItem(QIcon(QPixmap::fromImage(flag)), (*itEmpire).second.m_name);
		else
			itemName = new QTableWidgetItem((*itEmpire).second.m_name);
		QVariant var;
		var.setValue((*itEmpire).second.m_id);
		itemName->setData(Qt::UserRole, var);
		switch(citizenOnLine)
		{
		case 0:
			itemName->setForeground(QBrush(Qt::lightGray));
			itemName->setToolTip(tr("<b>Noncombatant</b><br>Empire has no avatar on line. "
						"Empire cannot attack or be attacked."));
			break;
		case 1:
			itemName->setForeground(QBrush(Qt::cyan));
			itemName->setToolTip(tr("<b>Playing</b><br>Empire has an avatar on line. "
						"Empire can attack and be attacked."));
			break;
		case 2:
			itemName->setForeground(QBrush(Qt::yellow));
			itemName->setToolTip(tr("<b>Unknown</b><br>Not known if any avatar is on line. "
						"Unknown if empire can attack or be attacked. "
						"<i>Request Empires</i> may get more information."));
			break;
		case 3:
			itemName->setForeground(QBrush(Qt::white));
			itemName->setToolTip(tr("<b>Pirate</b><br>Empire has no avatars; it only has NPCs. "
						"Empire can attack and be attacked."));
			break;
		}
		tableEmpires->setItem(iRow, iColumn++, itemName);

		if((*itEmpire).second.m_id == selectedEmpire)
			selectedEmpireItem = itemName;

		AuDocument::DOC_STATE nState = (*itEmpire).second.m_Dossier.NotableState();
		QTableWidgetItem* itemDossier = new AuTableWidgetSortedItem(nState, QPixmap(AuDocument::StateIcon(nState)),
				AuDocument::StateName(nState));
		if(nState < AuDocument::DOC_Count)
			itemDossier->setToolTip(tr("The highest level document status in the dossier is <b>%1</b>.")
					.arg(AuDocument::StateName(nState)));
		else
			itemDossier->setToolTip(tr("Dossier contains no documents."));
		tableEmpires->setItem(iRow, iColumn++, itemDossier);

		// My stance toward that other empire.
		QString sStance = "";
		if((*itEmpire).second.m_stance == POL_Friend)
			sStance = tr("Friend\n%1").arg((*itEmpire).second.m_whenSet.UTC().toLocalTime().toString());
		else if((*itEmpire).second.m_stance == POL_Enemy)
			sStance = tr("Enemy\n%1").arg((*itEmpire).second.m_whenSet.UTC().toLocalTime().toString());
		else if((*itEmpire).second.m_stance == POL_Vassal)
		{
			if((*itEmpire).second.m_tribute >= 10)
				sStance = tr("Vassal %2%\n%1")
					.arg((*itEmpire).second.m_whenSet.UTC().toLocalTime().toString()).arg((*itEmpire).second.m_tribute);
			else if((*itEmpire).second.m_tribute)
				sStance = tr("Ally %2%\n%1")
					.arg((*itEmpire).second.m_whenSet.UTC().toLocalTime().toString()).arg((*itEmpire).second.m_tribute);
			else
				sStance = tr("Ally\n%1")
					.arg((*itEmpire).second.m_whenSet.UTC().toLocalTime().toString());
		}
		else
			sStance = tr("Neutral\n%1")
				.arg((*itEmpire).second.m_whenSet.UTC().toLocalTime().toString());
		QTableWidgetItem *itemStanceType = new QTableWidgetItem(sStance);
		itemStanceType->setForeground(QBrush(PoliticalStanceColor((POLITICAL_STANCE)(*itEmpire).second.m_stance, false)));
		tableEmpires->setItem(iRow, iColumn++, itemStanceType);

		QColor stanceColor;
		if(hasEmpire)
		{
			POLITICAL_STANCE stanceBack = PoliticalStance((*itEmpire).second.m_id, m_Empire.m_id); 
			if(stanceBack == POL_Friend)
				sStance = tr("Friend\n%1").arg(whenSet.UTC().toLocalTime().toString());
			else if(stanceBack == POL_Enemy)
				sStance = tr("Enemy\n%1").arg(whenSet.UTC().toLocalTime().toString());
			else if(stanceBack == POL_Vassal)
			{
				if(tribute >= 10)
					sStance = tr("Vassal %2%\n%1").arg(whenSet.UTC().toLocalTime().toString()).arg(tribute);
				else if(tribute)
					sStance = tr("Ally %2%\n%1").arg(whenSet.UTC().toLocalTime().toString()).arg(tribute);
				else
					sStance = tr("Ally\n%1").arg(whenSet.UTC().toLocalTime().toString());
			}
			else
				sStance = tr("Neutral\n%1").arg(whenSet.UTC().toLocalTime().toString());
			stanceColor = PoliticalStanceColor(stanceBack, false);
		}
		else
		{
			sStance = "Data Not Available";
			stanceColor.setRgb(100,100,100);
		}
		QTableWidgetItem *itemStanceBack = new QTableWidgetItem(sStance);
		itemStanceBack->setForeground(QBrush(stanceColor));
		tableEmpires->setItem(iRow, iColumn++, itemStanceBack);

		// First contact date.
		QTableWidgetItem *itemMet = new AuTableWidgetSortedItem((*itEmpire).second.m_whenMet.MSecsSinceEpoch(), QIcon(),
				(*itEmpire).second.m_whenMet.UTC().toLocalTime().toString());
		itemMet->setForeground(QBrush(stanceColor));
		tableEmpires->setItem(iRow, iColumn++, itemMet);

		iRow++;
	}
	tableEmpires->setSortingEnabled(true);

	// If the table had no rows to begin with, resize the resizeable columns to the fresh data.
	if(!iCount)
	{
		tableEmpires->resizeColumnToContents(1);
		tableEmpires->resizeColumnToContents(2);
		tableEmpires->resizeColumnToContents(3);
		tableEmpires->resizeColumnToContents(4);
	}

	if(selectedEmpireItem)
		tableEmpires->setCurrentItem(selectedEmpireItem);

	tableEmpires->resizeRowsToContents();

	UpdateDossier();

	EnableControls();
}

void AuGovDiplomacy::EmpireSelectionChanged()
{
	UpdateDossier();
	EnableControls();
}

void AuGovDiplomacy::DossierSelectionChanged()
{
	UpdateDocument();
	EnableControls();
}

void AuGovDiplomacy::EnableControls()
{
	bool empireSelected = tableEmpires->currentRow() != -1;

	bool docSelected = false;
	QItemSelectionModel* ism = tableDossier->selectionModel();
	if(ism && !ism->selectedIndexes().isEmpty())
		docSelected = true;

	AuView* pView = auApp->Frame()->View();
	AuId personId = pView->SceneAtomId();
	quint32 personEmpireId = pView->SceneAtomEmpire();
	const DgData& gd = g_Au.LockDataForRead();
	bool canManageStance = gd.EmpireCitizenCanSetPoliticalStance(personEmpireId, personId);
	bool enableWarCrit = gd.EmpireCitizenCanSetWarCriteria(personEmpireId, personId);
	bool canManageDossier = gd.EmpireCitizenCanManageDossier(personEmpireId, personId);
	bool isGov = gd.IsEmpireGovernor(personEmpireId, personId);
	g_Au.UnlockData();

	m_WarCriteria->setEnabled(enableWarCrit);

	m_DispatchCourier->setEnabled(isGov && empireSelected);
	m_AddNote->setEnabled(canManageDossier && empireSelected);

	bool enableDoc = canManageDossier && docSelected;
	m_OpenDoc->setEnabled(enableDoc);
	m_CloseDoc->setEnabled(enableDoc);
	m_DeleteDoc->setEnabled(enableDoc);
	m_NewDoc->setEnabled(enableDoc);
	m_PendDoc->setEnabled(enableDoc);

	cbDefStance->setEnabled(canManageStance);

	bool enableStance = canManageStance && empireSelected;
	pbally->setEnabled(enableStance);
	pbenemy->setEnabled(enableStance);
	pbfriend->setEnabled(enableStance);
	pbneutral->setEnabled(enableStance);
	pbvassal->setEnabled(enableStance);
}

void AuGovDiplomacy::SetEnemy()
{
	quint32	towardEmpire = tableEmpires->item(tableEmpires->currentRow(), 0)->data(Qt::UserRole).toUInt();

	if(towardEmpire)
	{
		emit SetStance(towardEmpire, POL_Enemy, 0);
	}
}

void AuGovDiplomacy::SetAlly()
{
	quint32	towardEmpire = tableEmpires->item(tableEmpires->currentRow(), 0)->data(Qt::UserRole).toUInt();
	if(towardEmpire)
	{
		bool ok;
		QString tribute = QInputDialog::getText(this, g_Au.ApplicationName(),
				tr("Enter the percentage of tribute you would like to contribute, from\n"
					"0% to 9%. The capital city sends this percentage of its bank funds\n"
					"to the sworn empire once each real day."),
				QLineEdit::Normal, "0%", &ok);
		tribute = tribute.trimmed();	// Strip leading/trailing whitespace.
		while(tribute.right(1) == "%")
			tribute = tribute.left(tribute.count()-1).trimmed();	// Strip % sign and remaining whitespace.
		int tributePercent = QVariant(tribute).toInt();
		if(ok && tributePercent >= 0 && tributePercent <= 9)
		{
			emit SetStance(towardEmpire, POL_Vassal, tributePercent);
		}
	}
}

void AuGovDiplomacy::SetVassal()
{
	quint32	towardEmpire = tableEmpires->item(tableEmpires->currentRow(), 0)->data(Qt::UserRole).toUInt();
	if(towardEmpire)
	{
		bool ok;
		QString tribute = QInputDialog::getText(this, g_Au.ApplicationName(),
				tr("Enter the percentage of tribute you must contribute, from 10% to\n"
					"100%. The capital city sends this percentage of its bank funds to\n"
					"the sworn empire once each real day."),
				QLineEdit::Normal, "50%", &ok);
		tribute = tribute.trimmed();	// Strip leading/trailing whitespace.
		while(tribute.right(1) == "%")
			tribute = tribute.left(tribute.count()-1).trimmed();	// Strip % sign and remaining whitespace.
		int tributePercent = QVariant(tribute).toInt();
		if(ok && tributePercent >= 10 && tributePercent <= 100)
		{
			emit SetStance(towardEmpire, POL_Vassal, tributePercent);
		}
	}
}

void AuGovDiplomacy::SetFriend()
{
	quint32	towardEmpire = tableEmpires->item(tableEmpires->currentRow(), 0)->data(Qt::UserRole).toUInt();

	if(towardEmpire)
	{
		emit SetStance(towardEmpire, POL_Friend, 0);
	}
}

void AuGovDiplomacy::SetNeutral()
{
	quint32	towardEmpire = tableEmpires->item(tableEmpires->currentRow(), 0)->data(Qt::UserRole).toUInt();

	if(towardEmpire)
	{
		emit SetStance(towardEmpire, POL_Neutral, 0);
	}
}

void AuGovDiplomacy::RequestEmpireData()
{
	auApp->RequestEmpiresAll();
	AuFrame* pFrame = auApp->Frame();
	AuEmpireWait* waitDlg = new AuEmpireWait(this);
	connect(this, SIGNAL(EmpireDataCompleteSignal()), waitDlg, SLOT(accept()), Qt::QueuedConnection);
	pFrame->ExecDialog(*waitDlg);
}

void AuGovDiplomacy::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	if(!m_Empire.m_id)
		UpdateEmpireRecord();
}

// Dossier
void AuGovDiplomacy::UpdateDossier()
{
	// Remove current empire list
	int iCount = tableDossier->rowCount();
	for(int i=0; i<iCount; i++)
		tableDossier->removeRow(0);

	QTableWidgetItem* selectedEmpireItem = tableEmpires->item(tableEmpires->currentRow(), 0);
	if(selectedEmpireItem)
	{
		quint32 selectedEmpire = selectedEmpireItem->data(Qt::UserRole).toUInt();

		unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::const_iterator ki = m_Empire.m_Stance.find(selectedEmpire);
		if(ki != m_Empire.m_Stance.cend())
		{
			quint16 docIndex = 0;
			for(AuDossier::const_iterator di = (*ki).second.m_Dossier.cbegin(); di != (*ki).second.m_Dossier.cend();
					di++, docIndex++)
			{
				int row = tableDossier->rowCount();
				tableDossier->insertRow(row);

				// CreateTime is how dossier documents are uniquely identified.
				QVariant var;
				var.setValue((*di)->CreateTime());

				int column = 0;
				QTableWidgetItem* itemDate = new QTableWidgetItem(QIcon(QPixmap((*di)->ListIcon()).scaled(20,20, Qt::KeepAspectRatio, Qt::SmoothTransformation)),
						(*di)->CreateTime().UTC().toLocalTime().toString());
				itemDate->setData(Qt::UserRole, var);
				itemDate->setData(Qt::UserRole+1, (*di)->Type());
				itemDate->setData(Qt::UserRole+2, docIndex);
				tableDossier->setItem(row, column++, itemDate);

				QTableWidgetItem* itemName = new QTableWidgetItem(QIcon((*di)->StateIcon()), (*di)->Subject());
				itemName->setData(Qt::UserRole, var);
				tableDossier->setItem(row, column++, itemName);
			}

			// If the table had no rows to begin with, resize the resizeable columns to the fresh data.
			if(!iCount)
				tableDossier->resizeColumnToContents(0);
		}
	}

	m_Dossier->setHtml("");
}

// Dossier
void AuGovDiplomacy::UpdateDocument()
{
	// Html of selected dossier document.
	QString dos;

	QTableWidgetItem* selectedEmpireItem = tableEmpires->item(tableEmpires->currentRow(), 0);
	if(selectedEmpireItem)
	{
		quint32 selectedEmpire = selectedEmpireItem->data(Qt::UserRole).toUInt();
		QString empireName = selectedEmpireItem->text();

		QTableWidgetItem* selectedDossierItem = tableDossier->item(tableDossier->currentRow(), 0);
		if(selectedDossierItem)
		{
			//AuTime docTime = selectedDossierItem->data(Qt::UserRole).value<AuTime>();
			//quint8 docType = selectedDossierItem->data(Qt::UserRole+1).toUInt();
			quint16 docIndex = selectedDossierItem->data(Qt::UserRole+2).toUInt();

			unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::const_iterator ki = m_Empire.m_Stance.find(selectedEmpire);
			if(ki != m_Empire.m_Stance.cend())
			{
				if(docIndex < (*ki).second.m_Dossier.size())
				{
					AuDossier::const_iterator di = (*ki).second.m_Dossier.cbegin() + docIndex;
					dos = QString("<img height=\"32\" src=\"qrc%1\">%2")
						.arg((*di)->ListIcon())
						.arg((*di)->Html(empireName));
				}
			}
		}

		dos = QString("<html>%1</html>").arg(dos);
	}

	m_Dossier->setHtml(dos);
}

void AuGovDiplomacy::AddNote()
{
	QTableWidgetItem* selectedEmpireItem = tableEmpires->item(tableEmpires->currentRow(), 0);
	if(!selectedEmpireItem)
		return;

	quint32	selectedEmpire = selectedEmpireItem->data(Qt::UserRole).toUInt();
	QString empireName = selectedEmpireItem->text();

	QString docNote;
	bool ok;
	do
	{
		QString prompt;
		if(docNote.length() >= d_NoteLimit)
			prompt = tr("Enter a diplomatic note to be added to the dossier for %1.<br><br>The text entered is %2 characters. "
					"It must be less than %3 characters.").arg(empireName).arg(docNote.length()).arg(d_NoteLimit);
		else
			prompt = tr("Enter a diplomatic note to be added to the dossier for %1.").arg(empireName);
		docNote = QInputDialog::getMultiLineText(this, g_Au.ApplicationName(), prompt, docNote, &ok).trimmed();
		if(!ok || docNote.isEmpty())
			return;
	}
	while(docNote.length() >= d_NoteLimit);

	docNote.replace("\r\n", "\n");	// Does anybody put CRLF in text strings anymore?
	docNote.replace("\r", "\n");	// How about just CR?
	docNote.replace("\n", "<br>");	// A single LF is the standard.

	emit DossierAddNote(selectedEmpire, docNote);
}

void AuGovDiplomacy::NewDoc()
{
	QTableWidgetItem* selectedEmpireItem = tableEmpires->item(tableEmpires->currentRow(), 0);
	if(!selectedEmpireItem)
		return;

	quint32	selectedEmpire = selectedEmpireItem->data(Qt::UserRole).toUInt();

	quint8 docState = AuDocument::DOC_New;

	QItemSelectionModel* ism = tableDossier->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		for(QModelIndexList::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
			if((*si).isValid())
			{
				QTableWidgetItem* selectedDossierItem = tableDossier->item((*si).row(), 0);

				AuTime docTime = selectedDossierItem->data(Qt::UserRole).value<AuTime>();
				quint8 docType = selectedDossierItem->data(Qt::UserRole+1).toUInt();
				quint16 docIndex = selectedDossierItem->data(Qt::UserRole+2).toUInt();

				emit DossierDocState(selectedEmpire, docTime, docType, docIndex, docState);
			}
	}
}

void AuGovDiplomacy::OpenDoc()
{
	QTableWidgetItem* selectedEmpireItem = tableEmpires->item(tableEmpires->currentRow(), 0);
	if(!selectedEmpireItem)
		return;

	quint32	selectedEmpire = selectedEmpireItem->data(Qt::UserRole).toUInt();

	quint8 docState = AuDocument::DOC_Open;

	QItemSelectionModel* ism = tableDossier->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		for(QModelIndexList::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
			if((*si).isValid())
			{
				QTableWidgetItem* selectedDossierItem = tableDossier->item((*si).row(), 0);

				AuTime docTime = selectedDossierItem->data(Qt::UserRole).value<AuTime>();
				quint8 docType = selectedDossierItem->data(Qt::UserRole+1).toUInt();
				quint16 docIndex = selectedDossierItem->data(Qt::UserRole+2).toUInt();

				emit DossierDocState(selectedEmpire, docTime, docType, docIndex, docState);
			}
	}
}

void AuGovDiplomacy::CloseDoc()
{
	QTableWidgetItem* selectedEmpireItem = tableEmpires->item(tableEmpires->currentRow(), 0);
	if(!selectedEmpireItem)
		return;

	quint32	selectedEmpire = selectedEmpireItem->data(Qt::UserRole).toUInt();

	quint8 docState = AuDocument::DOC_Closed;

	QItemSelectionModel* ism = tableDossier->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		for(QModelIndexList::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
			if((*si).isValid())
			{
				QTableWidgetItem* selectedDossierItem = tableDossier->item((*si).row(), 0);

				AuTime docTime = selectedDossierItem->data(Qt::UserRole).value<AuTime>();
				quint8 docType = selectedDossierItem->data(Qt::UserRole+1).toUInt();
				quint16 docIndex = selectedDossierItem->data(Qt::UserRole+2).toUInt();

				emit DossierDocState(selectedEmpire, docTime, docType, docIndex, docState);
			}
	}
}

void AuGovDiplomacy::PendDoc()
{
	QTableWidgetItem* selectedEmpireItem = tableEmpires->item(tableEmpires->currentRow(), 0);
	if(!selectedEmpireItem)
		return;

	quint32	selectedEmpire = selectedEmpireItem->data(Qt::UserRole).toUInt();

	quint8 docState = AuDocument::DOC_Pending;

	QItemSelectionModel* ism = tableDossier->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		for(QModelIndexList::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
			if((*si).isValid())
			{
				QTableWidgetItem* selectedDossierItem = tableDossier->item((*si).row(), 0);

				AuTime docTime = selectedDossierItem->data(Qt::UserRole).value<AuTime>();
				quint8 docType = selectedDossierItem->data(Qt::UserRole+1).toUInt();
				quint16 docIndex = selectedDossierItem->data(Qt::UserRole+2).toUInt();

				emit DossierDocState(selectedEmpire, docTime, docType, docIndex, docState);
			}
	}
}

void AuGovDiplomacy::DeleteDoc()
{
	QTableWidgetItem* selectedEmpireItem = tableEmpires->item(tableEmpires->currentRow(), 0);
	if(!selectedEmpireItem)
		return;

	QString smsg = tr("<html>Confirm delete of documents from the dossier.</html>");
	QMessageBox mb(g_Au.ApplicationName(), smsg,
			QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, auApp->Frame());
	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	quint32	selectedEmpire = selectedEmpireItem->data(Qt::UserRole).toUInt();

	quint8 docState = AuDocument::DOC_Deleted;

	QItemSelectionModel* ism = tableDossier->selectionModel();
	if(ism)
	{
		// Delete these in reverse order so the indexes don't change as we go.
		QModelIndexList sel = ism->selectedIndexes();
		for(QModelIndexList::const_iterator si = sel.cend()-1; si != sel.cbegin()-1; si--)
			if((*si).isValid())
			{
				QTableWidgetItem* selectedDossierItem = tableDossier->item((*si).row(), 0);

				AuTime docTime = selectedDossierItem->data(Qt::UserRole).value<AuTime>();
				quint8 docType = selectedDossierItem->data(Qt::UserRole+1).toUInt();
				quint16 docIndex = selectedDossierItem->data(Qt::UserRole+2).toUInt();

				emit DossierDocState(selectedEmpire, docTime, docType, docIndex, docState);
			}
	}
}

void AuGovDiplomacy::DispatchCourier()
{
	QTableWidgetItem* selectedEmpireItem = tableEmpires->item(tableEmpires->currentRow(), 0);
	if(!selectedEmpireItem)
		return;

	quint32	selectedEmpire = selectedEmpireItem->data(Qt::UserRole).toUInt();
	QString empireName = selectedEmpireItem->text();

	QString docNote;
	bool ok;
	do
	{
		QString prompt;
		if(docNote.length() >= d_NoteLimit)
			prompt = tr("Enter a diplomatic message for %1. "
					"It will be delivered by a trusted courier. "
					"<br><br>The text entered is %2 characters. "
					"It must be less than %3 characters.").arg(empireName).arg(docNote.length()).arg(d_NoteLimit);
		else
			prompt = tr("Enter a diplomatic message for %1. "
					"It will be delivered by a trusted courier.").arg(empireName);
		docNote = QInputDialog::getMultiLineText(this, g_Au.ApplicationName(), prompt, docNote, &ok).trimmed();
		if(!ok || docNote.isEmpty())
			return;
	}
	while(docNote.length() >= d_NoteLimit);

	docNote.replace("\r\n", "\n");	// Does anybody put CRLF in text strings anymore?
	docNote.replace("\r", "\n");	// How about just CR?
	docNote.replace("\n", "<br>");	// A single LF is the standard.

	emit DossierDispatchCourier(selectedEmpire, docNote);
}

void AuGovDiplomacy::WarCriteria()
{
	AuFrame* pFrame = auApp->Frame();
	AuWarCriteriaDlg warCrit(m_Empire.m_WarCriteria, this);
	int result = pFrame->ExecDialog(warCrit);
	if(result != QDialog::Accepted)
		return;

	emit SetWarCriteria(warCrit.Criteria());
}

void AuGovDiplomacy::DoubleClickDoc()
{
	qDebug() << "AuGovDiplomacy::DoubleClickDoc";
	switch((AuDocument::DOC_STATE)g_AuClient.DossierDoubleClickStatus())
	{
	case AuDocument::DOC_New:
		NewDoc();
		break;
	case AuDocument::DOC_Open:
		OpenDoc();
		break;
	case AuDocument::DOC_Pending:
		PendDoc();
		break;
	case AuDocument::DOC_Closed:
		CloseDoc();
		break;
	case AuDocument::DOC_Deleted:
		DeleteDoc();
		break;
	case AuDocument::DOC_Count:
		break;
	}
}

void AuGovDiplomacy::SetDoubleClickStatus(int index)
{
	// 'index' happens to match the document state enum.
	qDebug() << "AuGovDiplomacy::SetDoubleClickStatus" << index << AuDocument::StateName((AuDocument::DOC_STATE)index);
	g_AuClient.DossierSetDoubleClickStatus(index);
}

QSize AuGovDiplomacy::sizeHint() const
{
	return QSize(1000,800);
}

