//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audesignerparts.cpp,v 1.17 2022/02/19 18:25:13 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audesignerparts.h"
//
#include "aucombobox.h"
#include "audesignerapp.h"
#include "audesignerframe.h"
#include "audesignerview.h"
#include "auscaddstate.h"
#include "auscaddstopdlg.h"
#include "auscchangestopdlg.h"
#include "auscdeletestate.h"
#include "auscdeletestopdlg.h"
#include "auscparts.h"
#include "auscrenamestopdlg.h"
#include "auslider.h"
#include "autablewidgetitem.h"
#include "dadesigner.h"
#include "daperson.h"
#include "dgdata.h"
#include <QHeaderView>
#include <QIcon>
#include <QInputDialog>
#include <QMenu>
#include <QTimer>
#include <QVBoxLayout>

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuDesignerPartsDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleDesignPartsUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

AuDesignerPartsDock::AuDesignerPartsDock(QWidget* parent) :
	AuDockWidget(":data/SoHDesignerIcon.png", tr("<b>Parts</b>"), tr("Parts"), parent)
{
	m_Parts = new AuDesignerParts(this);
	setWidget(m_Parts);
	connect(m_Parts, SIGNAL(GoToSelection()), SIGNAL(GoToSelection()));
	connect(m_Parts, SIGNAL(PartsSelectionChanged(AuScIdSet)), SIGNAL(PartsSelectionChanged(AuScIdSet)));

	int dockSides = g_Au.Settings().readNumEntry("Dock/DesignerPartsAreas", (int)allowedAreas());
	setAllowedAreas((Qt::DockWidgetAreas)dockSides);
}

void AuDesignerPartsDock::SetAllowedAreas(Qt::DockWidgetAreas areas)
{
	setAllowedAreas(areas);
	g_Au.Settings().writeEntry("Dock/DesignerPartsAreas", (int)allowedAreas());
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDesignerPartsTable::AuDesignerPartsTable(QWidget* parent) : QTableWidget(0,8, parent)
{
	m_ButtonsPressed = 0;

	//setAlternatingRowColors(true);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setShowGrid(false);
	setSortingEnabled(true);
	setToolTip(tr("Parts that make up the design. "
				"Right-click parts for options."));
	sortByColumn(1, Qt::AscendingOrder);

	//horizontalHeader()->hide();
	int col=0;
	horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Stretch);
	horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Interactive);
	horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Interactive);

	QStringList headerLabels;
	headerLabels << tr("Id");		// Identity number
	headerLabels << tr("Name");		// Name and type icon
	headerLabels << tr("Group");	// Group name
	headerLabels << tr("V");		// Visibility
	headerLabels << tr("L");		// Lock
	headerLabels << tr("O");		// Omit
	headerLabels << tr("S");		// Seal
	headerLabels << tr("State");	// Open/Closed state for doors, etc.
	setHorizontalHeaderLabels(headerLabels);

	verticalHeader()->hide();
	verticalHeader()->setDefaultSectionSize(18);
	verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
}

void AuDesignerPartsTable::mouseMoveEvent(QMouseEvent* e)
{
	QTableWidget::mouseMoveEvent(e);
}

void AuDesignerPartsTable::mouseDoubleClickEvent(QMouseEvent* e)
{
	QTableWidget::mouseDoubleClickEvent(e);
	emit MouseDoubleClicked();
}

void AuDesignerPartsTable::mousePressEvent(QMouseEvent* e)
{
	QTableWidget::mousePressEvent(e);
	m_ButtonsPressed = e->buttons();
	m_PressPos = e->pos();
}

void AuDesignerPartsTable::mouseReleaseEvent(QMouseEvent* e)
{
	QTableWidget::mouseReleaseEvent(e);
	if(m_ButtonsPressed & Qt::RightButton)
		emit MouseRightClicked();
	else if(rect().contains(e->pos()))
		emit MouseReleasedInside();
	else
		emit MouseReleasedOutside();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDesignerParts::AuDesignerParts(QWidget* parent) : QWidget(parent)
{
	m_timer = 0;
	m_Parts = new AuScParts;
	m_PartSel = new AuScSelection;

	m_PartsTable = new AuDesignerPartsTable(this);
	connect(m_PartsTable, SIGNAL(itemSelectionChanged()), SLOT(SelectionChanged()));
	connect(m_PartsTable, SIGNAL(itemClicked(QTableWidgetItem*)), SLOT(PartClicked(QTableWidgetItem*)));
	connect(m_PartsTable, SIGNAL(MouseRightClicked()), SLOT(PartsRightClicked()));

	QVBoxLayout* lay = new QVBoxLayout();
	lay->addWidget(m_PartsTable);
	lay->setMargin(4);
	lay->setSpacing(2);
	setLayout(lay);
}

AuDesignerParts::~AuDesignerParts()
{
	delete m_Parts;
	delete m_PartSel;
}

void AuDesignerParts::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_timer)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_timer->isActive())
		m_timer->start(1000);

	RestoreSettings();
	UpdateContents();
}

void AuDesignerParts::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Kill the timer. It will be recreated during the next showEvent.
	if(m_timer && m_timer->isActive())
		m_timer->stop();

	SaveSettings();
}

void AuDesignerParts::UpdateContents()
{
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	if(!pFrame)
		return;
	AuDesignerView* pView = pFrame->DesignerView();
	if(!pView)
		return;

	if(!isVisible())
	{
		// Kill the timer. It will be recreated during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	// List of the ids that are supposed to be in the table so we can remove the unwanted ones at the end.
	QList<quint16> intable;

	//qDebug() << "AuDesignerParts::UpdateContents";
	m_PartsTable->setSortingEnabled(false);
	m_PartsTable->blockSignals(true);

	const AuScSelection& selected = pView->DesignerPartObjectSelection();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		DaDesigner* de = person->FindParentDesignerInEnv();
		if(de)
		{
			const AuScParts* parts = de->LockPartsForRead();
			if(parts)
			{
				// Get the list of part ids that we should have in our table.
				intable = parts->keys();

				for(AuScParts::const_iterator pi = parts->cbegin(); pi != parts->cend(); pi++)
				{
					quint16 partId = pi.key();
					bool isSelected = selected.contains(partId);
					bool wasSelected = m_PartSel->contains(partId);

					bool hasPart;
					AuScParts::iterator mpi = m_Parts->find(partId);
					if(mpi == m_Parts->cend())
					{
						// Grab a clone of the part for future updates.
						hasPart = false;
						m_Parts->insert(partId, (*pi)->Clone());
					}
					else
					{
						// Do nothing if the part is identical.
						if(*(*mpi) == *(*pi))
						{
							// Update row selection state.
							if(isSelected != wasSelected)
								SetPartSelected(partId, isSelected);
							continue;
						}

						// Update my list with a clone of the new different part.
						delete *mpi;
						m_Parts->erase(mpi);
						m_Parts->insert(pi.key(), (*pi)->Clone());
						hasPart = true;
					}

					// Get information about the part.
					QIcon typeIcon = QIcon((*pi)->TypeIcon());
					bool canAddState = (*pi)->CanAddState();
					bool canDeleteState = (*pi)->CanDeleteState();
					bool canChangeStopName = (*pi)->CanChangeStopName();
					bool canChangeOpacity = (*pi)->CanChangeOpacity();
					bool canChangeState = (*pi)->CanChangeState();
					QString typeName = (*pi)->TypeName();
					QString partName = (*pi)->FullName(*parts);
					QString groupName = (*pi)->GroupName();
					/*This icon is useless. It tells me nothing and makes the column wider.
					QString groupIcon;
					if(!groupName.isEmpty())
					{
						if(groupName == AuScPart::GroupNameHull())
							groupIcon = ":data/ScHull.png";
						else if(groupName == AuScPart::GroupNameFencePost())
							groupIcon = ":data/ScFencePosts.png";
						else if(groupName == AuScPart::GroupNameFenceSection())
							groupIcon = ":data/ScFenceWood.png";
						else
						{
							int cids = g_Au.m_CommodityArray.size();
							QByteArray ba(groupName.toUtf8());
							int cksum = qChecksum(ba.data(), ba.size());
							int cid = cksum % cids;							// 0..n
							groupIcon = g_Au.m_CommodityArray[cid].BitmapName();
							//quint16 groupImgs = 60;						// The number of ScGroupNN.png images that exist.
							//quint16 groupNum = (cksum % groupImgs) + 1;	// 1..n
							//groupIcon = QString(":data/ScGroup%1.png").arg(groupNum);
						}
					}
					*/
					bool partLocked = (*pi)->IsLocked();
					bool partHidden = (*pi)->IsHidden();
					bool partOmitted = (*pi)->IsOmitted();
					bool partSealed = (*pi)->IsSealed();
					AuId partArchId = (*pi)->ArchId();
					QString partArchName = (*pi)->ArchName();
					quint8 partState = (*pi)->State();
					int iOpacity = (int)((*pi)->Opacity() * 100);
					bool canAddStop = (*pi)->CanAddStop();
					bool canChangeStop = (*pi)->CanChangeStop();
					bool canDeleteStop = (*pi)->CanDeleteStop();
					QMap<quint8, QString> partStates = (*pi)->StateNames();	// All possible state names.
					QString atStopName = (*pi)->AtStop();
					QStringList stopNames = (*pi)->StopNames();
					bool canChangeSeal = person->Is(partArchId);

					// Seal column tool tip says who the architect is.
					QString archTip;
					{
						QString archTipBase = tr("<p>"
								"Seal parts to prevent other architects from editing them in face mode. "
								"<b>Only the architect of a part can seal or unseal the part.</b>"
								"</p>"

								"<p>"
								"The architect of this part is <b>%1</b>."
								"</p>"

								"<p>"
								"Sealed parts can be used in part mode like other whole parts. "
								"They can be moved, stretched, rotated, copied to the clipboard and saved into files. "
								"Whole part editing operations are mostly disabled when a sealed part is selected. "
								"</p>"

								"<p>"
								"A sealed part can be used as a jig but it cannot be cut using a jig. "
								"</p>"

								"<p>"
								"Only the architect of a sealed part can enter face mode when the part is selected. "
								"This prevents individual faces of sealed parts from being modified or copied to the clipboard "
								"by other architects. "
								"</p>"

								"<p>"
								"The report for a design lists all architects whose parts are included in a design. "
								"</p>");
						if(partArchId.IsNull())
							archTip = archTipBase.arg(tr("Unknown"));
						else if(person->Is(partArchId))
							archTip = archTipBase.arg(tr("You"));
						else
							archTip = archTipBase.arg(QString("%1, <i>%2</i>").arg(partArchName).arg(partArchId.StringId()));
					}

					// Add the part to the table.
					AuComboBoxValue* cbState = 0;
					AuSliderValue* slOpacity = 0;
					if(hasPart)
					{
						// The part is already in the table so just update the contents.
						for(int row=0; row<m_PartsTable->rowCount(); row++)
						{
							int col=0;
							QTableWidgetItem* twid = m_PartsTable->item(row, col++);
							if(twid->data(Qt::UserRole).toUInt() == partId)
							{
								// Found the part's row in the table.
								// Update the fields.
								twid->setText(QString("%1").arg(partId));

								QTableWidgetItem* twname = m_PartsTable->item(row, col++);
								twname->setIcon(QIcon(typeIcon));
								twname->setText(partName);
								twname->setData(Qt::UserRole, canChangeStopName);
								twname->setData(Qt::UserRole+1, canAddState);
								twname->setData(Qt::UserRole+2, canDeleteState);
								twname->setData(Qt::UserRole+3, canAddStop);
								twname->setData(Qt::UserRole+4, canChangeStop);
								twname->setData(Qt::UserRole+5, canDeleteStop);
								twname->setData(Qt::UserRole+6, atStopName);
								twname->setData(Qt::UserRole+7, stopNames);
								twname->setData(Qt::UserRole+8, canChangeSeal);
								twname->setData(Qt::UserRole+9, groupName);

								QTableWidgetItem* twgroup = (QTableWidgetItem*)m_PartsTable->item(row, col++);
								//twgroup->setIcon(QIcon(QPixmap(groupIcon).scaled(12,12)));
								twgroup->setText(groupName);

								AuTableWidgetSortedItem* twvis = (AuTableWidgetSortedItem*)m_PartsTable->item(row, col++);
								twvis->setData(Qt::UserRole, partHidden);
								twvis->setData(Qt::UserRole+1, partHidden);	// sort
								if(partHidden)
									twvis->setIcon(QIcon(":data/ScHide.png"));
								else
									twvis->setIcon(QIcon(":data/ScShow.png"));

								AuTableWidgetSortedItem* twlock = (AuTableWidgetSortedItem*)m_PartsTable->item(row, col++);
								twlock->setData(Qt::UserRole, partLocked);
								twlock->setData(Qt::UserRole+1, partLocked);	// sort
								if(partLocked)
									twlock->setIcon(QIcon(":data/ScLock.png"));
								else
									twlock->setIcon(QIcon(":data/ScUnlock.png"));

								AuTableWidgetSortedItem* twomit = (AuTableWidgetSortedItem*)m_PartsTable->item(row, col++);
								twomit->setData(Qt::UserRole, partOmitted);
								twomit->setData(Qt::UserRole+1, partOmitted);	// sort
								if(partOmitted)
									twomit->setIcon(QIcon(":data/ScOmit.png"));
								else
									twomit->setIcon(QIcon(":data/ScInclude.png"));

								AuTableWidgetSortedItem* twseal = (AuTableWidgetSortedItem*)m_PartsTable->item(row, col++);
								twseal->setData(Qt::UserRole, partSealed);
								twseal->setData(Qt::UserRole+1, partSealed);	// sort
								if(partSealed)
									twseal->setIcon(QIcon(":data/ScSeal.png"));
								else
									twseal->setIcon(QIcon(":data/ScUnseal.png"));
								twseal->setToolTip(archTip);

								if(canChangeState)
								{
									QModelIndex idxState = m_PartsTable->model()->index(row, col++);
									cbState = dynamic_cast<AuComboBoxValue*>(m_PartsTable->indexWidget(idxState));
									if(cbState && cbState->count() != partStates.size())
									{
										cbState->blockSignals(true);
										cbState->clear();
										for(QMap<quint8, QString>::const_iterator psi = partStates.cbegin();
												psi != partStates.cend(); psi++)
											cbState->addItem(*psi, psi.key());
										cbState->blockSignals(false);
									}
								}
								else if(canChangeOpacity)
								{
									QModelIndex idxState = m_PartsTable->model()->index(row, col++);
									slOpacity = dynamic_cast<AuSliderValue*>(m_PartsTable->indexWidget(idxState));
								}
								else
									col++;

								if(isSelected != wasSelected)
								{
									twid->setSelected(isSelected);
									twname->setSelected(isSelected);
									twvis->setSelected(isSelected);
									twlock->setSelected(isSelected);
									twomit->setSelected(isSelected);
									twseal->setSelected(isSelected);
									twgroup->setSelected(isSelected);
								}
								break;
							}
						}
					}
					else
					{
						// The part is not in the table so insert a new record.
						m_PartsTable->insertRow(0);

						int col=0;
						AuTableWidgetIntegerItem* twid = new AuTableWidgetIntegerItem(partId);
						twid->setData(Qt::UserRole, partId);
						twid->setToolTip(tr("<html>"
									"Each part is identified by a unique number. "
									"Part id numbers cannot be changed. "
									"</html>"));
						m_PartsTable->setItem(0, col++, twid);

						QTableWidgetItem* twname = new QTableWidgetItem(QIcon(typeIcon), partName);
						twname->setData(Qt::UserRole, canChangeStopName);
						twname->setData(Qt::UserRole+1, canAddState);
						twname->setData(Qt::UserRole+2, canDeleteState);
						twname->setData(Qt::UserRole+3, canAddStop);
						twname->setData(Qt::UserRole+4, canChangeStop);
						twname->setData(Qt::UserRole+5, canDeleteStop);
						twname->setData(Qt::UserRole+6, atStopName);
						twname->setData(Qt::UserRole+7, stopNames);
						twname->setData(Qt::UserRole+8, canChangeSeal);
						twname->setData(Qt::UserRole+9, groupName);
						twname->setToolTip(tr("<html>"
									"Part name for most parts identifies the kind of part. "
									"This part is a %1."
									"</html>").arg(typeName));
						m_PartsTable->setItem(0, col++, twname);

						//QTableWidgetItem* twgroup = new QTableWidgetItem(QIcon(QPixmap(groupIcon).scaled(12,12)), groupName);
						QTableWidgetItem* twgroup = new QTableWidgetItem(groupName);
						twgroup->setToolTip(tr("<html>"
									"<p>"
									"Groups organize parts together. "
									"In building and spacecraft blueprints, groups define the rooms. "
									"In fence designs, two specific groups are required: <b>Post</b> and <b>Section</b>. "
									"</p>"
									"<p>"
									"See <i>Usage Tips</i> for detailed information about <b>Part Groups</b>. "
									"</p>"
									"</html>"));
						m_PartsTable->setItem(0, col++, twgroup);

						double sort = partHidden;
						AuTableWidgetSortedItem* twvis = new AuTableWidgetSortedItem(sort, QIcon(), "");
						if(partHidden)
							twvis->setIcon(QIcon(":data/ScHide.png"));
						else
							twvis->setIcon(QIcon(":data/ScShow.png"));
						twvis->setData(Qt::UserRole, partHidden);
						twvis->setToolTip(tr("<html>"
									"Change part visibility to reduce scene clutter while drawing. "
									"Visibility is used only during the design phase; "
									"it has no effect when a design is finalized. "
									"</html>"));
						m_PartsTable->setItem(0, col++, twvis);

						sort = partLocked;
						AuTableWidgetSortedItem* twlock = new AuTableWidgetSortedItem(sort, QIcon(), "");
						if(partLocked)
							twlock->setIcon(QIcon(":data/ScLock.png"));
						else
							twlock->setIcon(QIcon(":data/ScUnlock.png"));
						twlock->setData(Qt::UserRole, partLocked);
						twlock->setToolTip(tr("<html>"
									"Lock parts to protect them from accidental editing. "
									"Locking is used only during the design phase; it has no effect when a design is finalized. "
									"</html>"));
						m_PartsTable->setItem(0, col++, twlock);

						sort = partOmitted;
						AuTableWidgetSortedItem* twomit = new AuTableWidgetSortedItem(sort, QIcon(), "");
						if(partOmitted)
							twomit->setIcon(QIcon(":data/ScOmit.png"));
						else
							twomit->setIcon(QIcon(":data/ScInclude.png"));
						twomit->setData(Qt::UserRole, partOmitted);
						twomit->setToolTip(tr("<html>"
									"Omit parts to prevent them from being included when the design is finalized. "
									"Omitted parts are excluded from the design analysis. "
									"Some parts are always omitted; they cannot be included. "
									"</html>"));
						m_PartsTable->setItem(0, col++, twomit);

						sort = partSealed;
						AuTableWidgetSortedItem* twseal = new AuTableWidgetSortedItem(sort, QIcon(), "");
						if(partSealed)
							twseal->setIcon(QIcon(":data/ScSeal.png"));
						else
							twseal->setIcon(QIcon(":data/ScUnseal.png"));
						twseal->setData(Qt::UserRole, partSealed);
						twseal->setToolTip(archTip);
						m_PartsTable->setItem(0, col++, twseal);

						if(canChangeState)
						{
							cbState = new AuComboBoxValue(partId);
							for(QMap<quint8, QString>::const_iterator psi = partStates.cbegin(); psi != partStates.cend(); psi++)
								cbState->addItem(*psi, psi.key());
							cbState->setToolTip(tr("<html>"
										"Choose which state of the part will be viewed.<br>"
										"Some parts can be represented in different states. "
										"For example, a door may be open or closed. "
										"Right-click to add a state.<br>"
										"%1 In <i>Face Mode</i>, only the visible state is edited. "
										"Edit each state to create distinct appearances.<br>"
										"%1 In <i>Part Mode</i>, all states are edited at once "
										"so a part with multiple states can be moved easily. "
										"</html>")
									.arg(g_Au.MiddleDotSymbol()));
							cbState->setObjectName(QString("state%1").arg(partId));
							connect(cbState, SIGNAL(CurrentIndexChanged(int, int)), SLOT(StateChanged(int, int)));
							QModelIndex idxState = m_PartsTable->model()->index(0, col++);
							m_PartsTable->setIndexWidget(idxState, cbState);
						}
						else if(canChangeOpacity)
						{
							slOpacity = new AuSliderValue(Qt::Horizontal, partId);
							slOpacity->setRange(0, 100);
							slOpacity->setToolTip(tr("<html>"
										"Adjusts transparency from invisible through opaque, translucent for windows."
										"</html>"));
							slOpacity->setObjectName(QString("state%1").arg(partId));
							connect(slOpacity, SIGNAL(ValueChanged(int, int)), SLOT(OpacityChanged(int, int)));
							QModelIndex idxState = m_PartsTable->model()->index(0, col++);
							m_PartsTable->setIndexWidget(idxState, slOpacity);
						}
						else
							col++;

						twid->setSelected(isSelected);
						twname->setSelected(isSelected);
						twvis->setSelected(isSelected);
						twlock->setSelected(isSelected);
						twomit->setSelected(isSelected);
						twseal->setSelected(isSelected);
						twgroup->setSelected(isSelected);
					}

					// Update contents of field widgets.
					if(cbState)
					{
						cbState->blockSignals(true);
						cbState->setCurrentIndex(cbState->findData(partState));
						cbState->blockSignals(false);
					}
					if(slOpacity)
					{
						slOpacity->blockSignals(true);
						slOpacity->setValue(iOpacity);
						slOpacity->blockSignals(false);
					}
				}
			}
			de->UnlockParts();
		}
	}
	pView->UnlockSceneAtom();

	*m_PartSel = selected;

	// Remove rows that no longer belong in the table.
	AuScParts::iterator mpi = m_Parts->begin();
	while(mpi != m_Parts->end())
	{
		// If my part is not in the intable list, it needs to be removed.
		if(intable.contains(mpi.key()))
		{
			// My part is in the parts list so it remains in the table.
			mpi++;
		}
		else
		{
			// My part is no longer in the parts list so it must be removed from the table.
			for(int row=0; row<m_PartsTable->rowCount(); row++)
			{
				QTableWidgetItem* item = m_PartsTable->item(row, 0);
				if(item->data(Qt::UserRole).toUInt() == mpi.key())
				{
					// Found the part's row in the table. Remove it.
					m_PartsTable->removeRow(row);
					break;
				}
			}

			// Remove my part.
			delete *mpi;
			mpi = m_Parts->erase(mpi);
		}
	}

	m_PartsTable->setSortingEnabled(true);
	m_PartsTable->blockSignals(false);

	EnableControls();
}

void AuDesignerParts::EnableControls()
{
	/*
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;
		*/
}

void AuDesignerParts::RestoreSettings()
{
	// Restore the sizes of the table widget's columns 1 through 4, not 0 which is the automatically sized column.
	for(int col=0; col<m_PartsTable->columnCount(); col++)
	{
		if(col == 1)
			continue;	// skip the automatically sized column
		int colwid = g_Au.Settings().readNumEntry(QString("Widget/PartsTable/ColumnWid%1").arg(col),-123);
		if(colwid != -123)
			m_PartsTable->setColumnWidth(col, colwid);
	}
}

void AuDesignerParts::SaveSettings()
{
	// Save the sizes of the table widget's columns.
	for(int col=0; col<m_PartsTable->columnCount(); col++)
		g_Au.Settings().writeEntry(QString("Widget/PartsTable/ColumnWid%1").arg(col), m_PartsTable->columnWidth(col));
}

AuScIdSet AuDesignerParts::SelectedParts() const
{
	AuScIdSet selParts;
	QItemSelectionModel* ism = m_PartsTable->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		for(QModelIndexList::const_iterator si = sel.cbegin(); si != sel.cend(); si++)
			if((*si).isValid())
			{
				QTableWidgetItem* twid = m_PartsTable->item((*si).row(), 0);
				quint16 partId = twid->data(Qt::UserRole).toUInt();
				selParts.m_Ids.insert(partId);
			}
	}
	return selParts;
}

void AuDesignerParts::SelectionChanged()
{
	// Set the selection in the view to match my new selection state.
	emit PartsSelectionChanged(SelectedParts());
}

void AuDesignerParts::StateChanged(int index, int partId)
{
	AuFrame* frame = auApp->Frame();
	if(!frame)
		return;

	AuComboBoxValue* cbState = findChild<AuComboBoxValue*>(QString("state%1").arg(partId));
	if(!cbState)
		return;

	quint8 partState = cbState->itemData(index).toUInt();
	auApp->DesignerPartSetState(partId, partState);
}

void AuDesignerParts::OpacityChanged(int iOpacity, int partId)
{
	AuFrame* frame = auApp->Frame();
	if(!frame)
		return;

	float opacity = (float)iOpacity / 100.f;
	auApp->DesignerPartSetOpacity(partId, opacity);
}

void AuDesignerParts::PartClicked(QTableWidgetItem* item)
{
	AuFrame* frame = auApp->Frame();
	if(!frame)
		return;

	if(!item)
		return;

	AuScIdSet selIds(SelectedParts());

	if(item->column() == 3)
	{
		// Visibility
		bool partHidden = item->data(Qt::UserRole).toBool();
		auApp->DesignerPartsSetHidden(selIds, !partHidden);
	}
	else if(item->column() == 4)
	{
		// Lock
		bool partLocked = item->data(Qt::UserRole).toBool();
		auApp->DesignerPartsSetLocked(selIds, !partLocked);
	}
	else if(item->column() == 5)
	{
		// Omit
		bool partOmitted = item->data(Qt::UserRole).toBool();
		auApp->DesignerPartsSetOmitted(selIds, !partOmitted);
	}
	else if(item->column() == 6)
	{
		// Seal
		bool canChangeSeal = true;
		for(int row=0; row<m_PartsTable->rowCount(); row++)
		{
			QTableWidgetItem* twid = m_PartsTable->item(row, 0);
			quint16 partId = twid->data(Qt::UserRole).toUInt();
			if(selIds.contains(partId))
			{
				QTableWidgetItem* twname = m_PartsTable->item(row, 1);
				canChangeSeal = twname->data(Qt::UserRole+8).toBool();
				if(!canChangeSeal)
					break;
			}
		}

		if(canChangeSeal)
		{
			bool partSealed = item->data(Qt::UserRole).toBool();
			auApp->DesignerPartsSetSeal(selIds, !partSealed);
		}
	}
}

void AuDesignerParts::PartsRightClicked()
{
	AuDesignerFrame* frame = auDesignerApp->DesignerFrame();
	if(!frame)
		return;
	AuDesignerView* view = frame->DesignerView();
	if(!view)
		return;

	// Designer does nothing in preview mode.
	if(view->IsDesignPreview())
		return;

	//qDebug() << "AuDesignerParts::PartsRightClicked";
	AuScIdSet selIds(SelectedParts());
	if(selIds.m_Ids.empty())
		return;

	// Show a pop-up menu of options.
	QMenu menu(this);

	// Can only rename or add a state to one thing at a time.
	QAction* addState = 0;
	QAction* deleteState = 0;
	QAction* changeGroup = 0;
	QAction* changeStopName = 0;
	QAction* addStop = 0;
	QSet<QAction*> changeStops;
	QAction* deleteStop = 0;
	QAction* renameStop = 0;
	QString groupName;
	if(selIds.m_Ids.size() == 1)
	{
		// Find the item in the table.
		bool canAddState = false;
		bool canDeleteState = false;
		bool canChangeStopName = false;
		bool canAddStop = false;
		bool canChangeStop = false;
		bool canDeleteStop = false;
		QString atStopName;
		QStringList stopNames;
		for(int row=0; row<m_PartsTable->rowCount(); row++)
		{
			QTableWidgetItem* twid = m_PartsTable->item(row, 0);
			quint16 partId = twid->data(Qt::UserRole).toUInt();
			if(partId == *(selIds.m_Ids.begin()))
			{
				QTableWidgetItem* twname = m_PartsTable->item(row, 1);
				canChangeStopName = twname->data(Qt::UserRole).toBool();
				canAddState = twname->data(Qt::UserRole+1).toBool();
				canDeleteState = twname->data(Qt::UserRole+2).toBool();
				canAddStop = twname->data(Qt::UserRole+3).toBool();
				canChangeStop = twname->data(Qt::UserRole+4).toBool();
				canDeleteStop = twname->data(Qt::UserRole+5).toBool();
				atStopName = twname->data(Qt::UserRole+6).toString();
				stopNames = twname->data(Qt::UserRole+7).toStringList();
				groupName = twname->data(Qt::UserRole+9).toString();
				break;
			}
		}

		if(canAddStop || canChangeStop || canDeleteStop)
		{
			if(canChangeStop && !stopNames.empty())
			{
				QMenu* stopsMenu = menu.addMenu(QIcon(":data/ScLiftButtons.png"), tr("Stops"));

				for(QStringList::const_iterator si = stopNames.cbegin(); si != stopNames.cend(); si++)
				{
					if(*si == atStopName)
						changeStops.insert(stopsMenu->addAction(QIcon(":data/ScChangeStop.png"), *si));
					else
						changeStops.insert(stopsMenu->addAction(QIcon(":data/ScLiftStop.png"), *si));
				}
			}

			if(canAddStop)
				addStop = menu.addAction(QIcon(":data/ScAddStop.png"), tr("Add Stop..."));

			if(canDeleteStop)
			{
				renameStop = menu.addAction(QIcon(":data/ScRenameStop.png"), tr("Rename Stop..."));

				deleteStop = menu.addAction(QIcon(":data/ScDeleteStop.png"), tr("Delete Stop..."));
			}

			menu.addSeparator();
		}

		if(canAddState || canDeleteState || canChangeStopName)
		{
			if(canAddState)
				addState = menu.addAction(QIcon(":data/ScAddState.png"), tr("Add State..."));

			if(canDeleteState)
				deleteState = menu.addAction(QIcon(":data/ScDeleteState.png"), tr("Delete State..."));

			if(canChangeStopName)
				changeStopName = menu.addAction(QIcon(":data/ScRename.png"), tr("Change Stop Name..."));

			menu.addSeparator();
		}
	}

	QAction* gotoSel = 0;
	if(!selIds.m_Ids.empty())
	{
		changeGroup = menu.addAction(QIcon(":data/ScGroup.png"), tr("Change Group..."));

		gotoSel = menu.addAction(QIcon(":data/MsgTransport.png"), tr("Go To Selection..."));

		menu.addSeparator();
	}

	bool canChangeSeal = true;
	for(int row=0; row<m_PartsTable->rowCount(); row++)
	{
		QTableWidgetItem* twid = m_PartsTable->item(row, 0);
		quint16 partId = twid->data(Qt::UserRole).toUInt();
		if(selIds.contains(partId))
		{
			QTableWidgetItem* twname = m_PartsTable->item(row, 1);
			canChangeSeal = twname->data(Qt::UserRole+8).toBool();
			if(!canChangeSeal)
				break;
		}
	}

	QAction* showSel = menu.addAction(QIcon(":data/ScShow.png"), tr("Show"));

	QAction* hideSel = menu.addAction(QIcon(":data/ScHide.png"), tr("Hide"));

	menu.addSeparator();

	QAction* lockSel = menu.addAction(QIcon(":data/ScLock.png"), tr("Lock"));

	QAction* unlockSel = menu.addAction(QIcon(":data/ScUnlock.png"), tr("Unlock"));

	menu.addSeparator();

	QAction* includeSel = menu.addAction(QIcon(":data/ScInclude.png"), tr("Include"));

	QAction* omitSel = menu.addAction(QIcon(":data/ScOmit.png"), tr("Omit"));

	QAction* sealSel = 0;
	QAction* unsealSel = 0;
	if(canChangeSeal)
	{
		menu.addSeparator();

		sealSel = menu.addAction(QIcon(":data/ScSeal.png"), tr("Seal"));

		unsealSel = menu.addAction(QIcon(":data/ScUnseal.png"), tr("Unseal"));
	}

	QAction* action = menu.exec(QCursor::pos());
	if(!action)
		return;

	if(action == changeStopName)
	{
		bool ok;
		QString name = QInputDialog::getText(this, g_Au.ApplicationName(),
				tr("Enter a turbo lift stop name."), QLineEdit::Normal, "", &ok);
		if(ok)
			auApp->DesignerPartSetStopName(*selIds.m_Ids.begin(), name);
	}
	else if(action == gotoSel)
	{
		emit GoToSelection();
	}
	else if(action == showSel)
	{
		auApp->DesignerPartsSetHidden(selIds, false);
	}
	else if(action == hideSel)
	{
		auApp->DesignerPartsSetHidden(selIds, true);
	}
	else if(action == lockSel)
	{
		auApp->DesignerPartsSetLocked(selIds, true);
	}
	else if(action == unlockSel)
	{
		auApp->DesignerPartsSetLocked(selIds, false);
	}
	else if(action == includeSel)
	{
		auApp->DesignerPartsSetOmitted(selIds, false);
	}
	else if(action == omitSel)
	{
		auApp->DesignerPartsSetOmitted(selIds, true);
	}
	else if(action == sealSel)
	{
		auApp->DesignerPartsSetSeal(selIds, true);
	}
	else if(action == unsealSel)
	{
		auApp->DesignerPartsSetSeal(selIds, false);
	}
	else if(action == addState)
	{
		// Get the list of states that can be added.
		quint16 partId = *selIds.m_Ids.begin();
		if(partId)
		{
			QMap<quint8, QString> addableStates;
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				DaDesigner* de = person->FindParentDesignerInEnv();
				if(de)
				{
					const AuScParts* parts = de->LockPartsForRead();
					if(parts)
					{
						AuScParts::const_iterator pi = parts->find(partId);
						if(pi != parts->cend())
							addableStates = (*pi)->AddableStates();
					}
					de->UnlockParts();
				}
			}
			view->UnlockSceneAtom();

			if(!addableStates.empty())
			{
				// Find out what state they want added.
				AuScAddState dlg(addableStates, this);
				if(dlg.exec() == QDialog::Accepted)
				{
					quint8 state = dlg.State();
					auApp->DesignerPartAddState(partId, state);
				}
			}
		}
	}
	else if(action == deleteState)
	{
		// Get the list of states that can be added.
		quint16 partId = *selIds.m_Ids.begin();
		if(partId)
		{
			QMap<quint8, QString> deletableStates;
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				DaDesigner* de = person->FindParentDesignerInEnv();
				if(de)
				{
					const AuScParts* parts = de->LockPartsForRead();
					if(parts)
					{
						AuScParts::const_iterator pi = parts->find(partId);
						if(pi != parts->cend())
							deletableStates = (*pi)->DeletableStates();
					}
					de->UnlockParts();
				}
			}
			view->UnlockSceneAtom();

			if(!deletableStates.empty())
			{
				// Find out what state they want added.
				AuScDeleteState dlg(deletableStates, this);
				if(dlg.exec() == QDialog::Accepted)
				{
					quint8 state = dlg.State();
					auApp->DesignerPartDeleteState(partId, state);
				}
			}
		}
	}
	else if(action == addStop)
	{
		// Get the list of states that can be added.
		quint16 partId = *selIds.m_Ids.begin();
		if(partId)
		{
			// Find out what stop they want added.
			AuScAddStopDlg dlg(this);
			if(dlg.exec() == QDialog::Accepted)
			{
				QString stopName = dlg.StopName();
				auApp->DesignerPartAddStop(partId, stopName);
			}
		}
	}
	else if(changeStops.contains(action))
	{
		quint16 partId = *selIds.m_Ids.begin();
		if(partId)
		{
			QString stopName = action->text();
			auApp->DesignerPartSetStop(partId, stopName);
		}
	}
	else if(action == renameStop)
	{
		// Get the list of stop names.
		quint16 partId = *selIds.m_Ids.begin();
		if(partId)
		{
			QStringList deletableStops;
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				DaDesigner* de = person->FindParentDesignerInEnv();
				if(de)
				{
					const AuScParts* parts = de->LockPartsForRead();
					if(parts)
					{
						AuScParts::const_iterator pi = parts->find(partId);
						if(pi != parts->cend())
							deletableStops = (*pi)->DeletableStops();
					}
					de->UnlockParts();
				}
			}
			view->UnlockSceneAtom();

			if(!deletableStops.empty())
			{
				// Find out what state they want added.
				AuScRenameStopDlg dlg(deletableStops, this);
				if(dlg.exec() == QDialog::Accepted)
				{
					QString oldStopName = dlg.OldStopName();
					QString nuStopName = dlg.NewStopName();
					auApp->DesignerPartRenameStop(partId, oldStopName, nuStopName);
				}
			}
		}
	}
	else if(action == deleteStop)
	{
		// Get the list of stop names.
		quint16 partId = *selIds.m_Ids.begin();
		if(partId)
		{
			QStringList deletableStops;
			DaPerson* person;
			view->LockSceneAtom(&person);
			if(person)
			{
				DaDesigner* de = person->FindParentDesignerInEnv();
				if(de)
				{
					const AuScParts* parts = de->LockPartsForRead();
					if(parts)
					{
						AuScParts::const_iterator pi = parts->find(partId);
						if(pi != parts->cend())
							deletableStops = (*pi)->DeletableStops();
					}
					de->UnlockParts();
				}
			}
			view->UnlockSceneAtom();

			if(!deletableStops.empty())
			{
				// Find out what state they want added.
				AuScDeleteStopDlg dlg(deletableStops, this);
				if(dlg.exec() == QDialog::Accepted)
				{
					QString stopName = dlg.StopName();
					auApp->DesignerPartDeleteStop(partId, stopName);
				}
			}
		}
	}
	else if(action == changeGroup)
	{
		//qDebug() << "AuDesignerParts::PartsRightClicked() change group";
		// Get the list of all group names currently in use by the parts.
		QStringList groupNames;
		DaDesignBase::DESIGNMODE designMode = DaDesignBase::InvalidMode;
		BUILDING_STRUCTURE bdStruct = BD_Invalid;
		DaPerson* person;
		view->LockSceneAtom(&person);
		if(person)
		{
			DaDesigner* de = person->FindParentDesignerInEnv();
			if(de)
			{
				designMode = de->DesignMode();
				bdStruct = de->BuildingStructure();
				de->PartGroupNames(groupNames);
			}
		}
		view->UnlockSceneAtom();
		switch(designMode)
		{
		case DaDesignBase::InvalidMode:
		case DaDesignBase::DESIGN_Unknown:
		case DaDesignBase::DESIGN_Vehicle:
			break;
		case DaDesignBase::DESIGN_Item:
			break;
		case DaDesignBase::DESIGN_Spacecraft:
			if(groupNames.indexOf(AuScPart::GroupNameHull()) < 0)
				groupNames.append(AuScPart::GroupNameHull());
			break;
		case DaDesignBase::DESIGN_Building:
			switch(bdStruct)
			{
			case BD_Fence:
				if(groupNames.indexOf(AuScPart::GroupNameFencePost()) < 0)
					groupNames.append(AuScPart::GroupNameFencePost());
				if(groupNames.indexOf(AuScPart::GroupNameFenceSection()) < 0)
					groupNames.append(AuScPart::GroupNameFenceSection());
				break;
			case BD_Biodome:
			case BD_Bridge:
			case BD_Building:
			case BD_SeaPlatform:
			case BD_AerialPlatform:
			case BD_GroundPlatform:
			case BD_SeaSurfaceBuilding:
			case BD_SpaceElevator:
			case BD_RoadEnclosure:
			case BD_Statue:
			case BD_Tunnel:
			case BD_StreetLight:
				if(groupNames.indexOf(AuScPart::GroupNameHull()) < 0)
					groupNames.append(AuScPart::GroupNameHull());
				break;
			case BD_Invalid:
			case BD_Count:
				break;
			}
			break;
		}

		// Alphabetize the list of names.
		QMap<QString, bool> sortedNames;
		for(QStringList::const_iterator gi = groupNames.cbegin(); gi != groupNames.cend(); gi++)
			sortedNames.insert(*gi, true);
		groupNames = sortedNames.keys();

		// Present the choices to the user.
		bool ok = false;
		int current = groupNames.indexOf(groupName);
		QString name;
		switch(designMode)
		{
		case DaDesigner::InvalidMode:
		case DaDesigner::DESIGN_Unknown:
		case DaDesigner::DESIGN_Vehicle:
			break;
		case DaDesigner::DESIGN_Item:
			name = QInputDialog::getItem(this, g_Au.ApplicationName(),
					tr("<html>"
						"Enter a group name for the selected parts. "
						"<br>"
						"%1 Names already in use are shown. "
						"<br>"
						"%1 Enter a new name to create a new group. "
						"</html>").arg(AuGlobal::MiddleDotSymbol()),
					groupNames, current, true, &ok).trimmed();
			break;
		case DaDesigner::DESIGN_Spacecraft:
			name = QInputDialog::getItem(this, g_Au.ApplicationName(),
					tr("<html>"
						"Enter a group name for the selected parts. "
						"<br>"
						"%1 Group names become room names in the final blueprint, if the group contains a room void. "
						"<br>"
						"%1 Parts in the <b>Hull</b> group and in the blank group are considered hull parts. "
						"<br>"
						"%1 Names already in use are shown, plus the reserved name <b>Hull</b>. "
						"<br>"
						"%1 Enter a new name to create a new group. "
						"</html>").arg(AuGlobal::MiddleDotSymbol()),
					groupNames, current, true, &ok).trimmed();
			break;
		case DaDesigner::DESIGN_Building:
			switch(bdStruct)
			{
			case BD_Fence:
				name = QInputDialog::getItem(this, g_Au.ApplicationName(),
						tr("<html>"
							"Fence and wall groups separate their subassemblies. "
							"Enter a group name for the selected parts. "
							"<br>"
							"%1 Parts in the <b>Post</b> group will be used to create fence or wall posts. "
							"<br>"
							"%1 Parts in the <b>Section</b> group will be stretched to fit between post centers. "
							"<br>"
							"%1 Names already in use are shown, plus the reserved names <b>Post</b> and <b>Section</b>. "
							"<br>"
							"%1 Enter a new name to create a new group. "
							"<br>"
							"%1 Enter a blank name to remove parts from groups. "
							"</html>").arg(AuGlobal::MiddleDotSymbol()),
						groupNames, current, true, &ok).trimmed();
				break;
			case BD_Biodome:
			case BD_Bridge:
			case BD_Building:
			case BD_SeaPlatform:
			case BD_AerialPlatform:
			case BD_GroundPlatform:
			case BD_SeaSurfaceBuilding:
			case BD_SpaceElevator:
			case BD_RoadEnclosure:
			case BD_Statue:
			case BD_Tunnel:
			case BD_StreetLight:
				name = QInputDialog::getItem(this, g_Au.ApplicationName(),
						tr("<html>"
							"Enter a group name for the selected parts. "
							"<br>"
							"%1 Group names become room names in the final blueprint, if the group contains a room void. "
							"<br>"
							"%1 Parts in the <b>Hull</b> group and in the blank group are considered hull parts. "
							"<br>"
							"%1 Names already in use are shown, plus the reserved name <b>Hull</b>. "
							"<br>"
							"%1 Enter a new name to create a new group. "
							"</html>").arg(AuGlobal::MiddleDotSymbol()),
						groupNames, current, true, &ok).trimmed();
				break;
			case BD_Invalid:
			case BD_Count:
				break;
			}
			break;
		}
		if(!ok)
			return;

		// Configure the group on the selected parts.
		auApp->DesignerPartsSetGroup(selIds, name);
	}
}

void AuDesignerParts::SetPartSelected(quint16 partId, bool isSelected)
{
	for(int row=0; row<m_PartsTable->rowCount(); row++)
	{
		int col=0;
		QTableWidgetItem* twid = m_PartsTable->item(row, col++);
		if(twid->data(Qt::UserRole).toUInt() != partId)
			continue;

		// Found the part's row in the table.
		twid->setSelected(isSelected);

		QTableWidgetItem* twname = m_PartsTable->item(row, col++);
		twname->setSelected(isSelected);

		QTableWidgetItem* twvis = m_PartsTable->item(row, col++);
		twvis->setSelected(isSelected);

		QTableWidgetItem* twlock = m_PartsTable->item(row, col++);
		twlock->setSelected(isSelected);

		QTableWidgetItem* twomit = m_PartsTable->item(row, col++);
		twomit->setSelected(isSelected);

		QTableWidgetItem* twseal = m_PartsTable->item(row, col++);
		twseal->setSelected(isSelected);

		QTableWidgetItem* twgroup = m_PartsTable->item(row, col++);
		twgroup->setSelected(isSelected);

		break;
	}
}
