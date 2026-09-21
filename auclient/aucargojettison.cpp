//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucargojettison.cpp,v 1.13 2022/02/19 18:10:02 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucargojettison.h"
//
#include "auapp.h"
#include "aucommpacket.h"
#include "auframe.h"
#include "auglobal.h"
#include "auview.h"
#include "daatom.h"
#include "daperson.h"
#include "dgdata.h"
#include <QComboBox>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QTextBrowser>
#include <QTimer>

void AuCargoJettisonDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleCargoManifestUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuCargoJettison::AuCargoJettison(QWidget* parent) : QWidget(parent)
{
	m_CargoFilter = FILTER_All;
	m_MaxCount = 0;
	m_timer = 0;

	QGridLayout* lay = new QGridLayout();
	int row = 0;	// Using this makes inserting and rearranging rows easier.

	// Cargo Space Status
	QLabel* lbct = new QLabel(tr("Cargo Hold"), this);
	lbct->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_FillCargo = new QProgressBar(this);
	m_FillCargo->setFormat(tr("%v of %m"));

	lay->addWidget(lbct, row,0);
	lay->addWidget(m_FillCargo, row,1);
	row++;

	// Fuel Cell Status
	QLabel* lbft = new QLabel(tr("Fuel Cells"), this);
	lbft->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_FillFuel = new QProgressBar(this);
	m_FillFuel->setFormat(tr("%v of %m"));

	lay->addWidget(lbft, row,0);
	lay->addWidget(m_FillFuel, row,1);
	row++;

	m_CargoFilterWidget = new AuCommodityFilter;
	connect(m_CargoFilterWidget, SIGNAL(FilterChanged(COMMODITYFILTER)), SLOT(FilterCargo(COMMODITYFILTER)));
	lay->addWidget(m_CargoFilterWidget, row,0, 1,2);
	row++;

	// Cargo aboard
	m_CargoList = new QListWidget(this);
	m_CargoList->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_CargoList->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_CargoList->setSelectionMode(QAbstractItemView::SingleSelection);
	m_CargoList->setSortingEnabled(true);
	connect(m_CargoList, SIGNAL(itemSelectionChanged()), SLOT(CargoSelected()));
	connect(m_CargoList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(CargoDoubleClicked(QListWidgetItem*)));

	m_CargoDesc = new QTextBrowser(this);
	m_CargoDesc->setReadOnly(true);

	QHBoxLayout* cargolay = new QHBoxLayout;
	cargolay->addWidget(m_CargoList);
	cargolay->addWidget(m_CargoDesc);

	lay->addLayout(cargolay, row,0, 1,2);
	row++;

	QGridLayout* dolay = new QGridLayout;
	{
		int dorow=0;

		QLabel* lbcombine = new QLabel(tr("Combine With"), this);
		lbcombine->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_CombineWith = new QComboBox(this);
		connect(m_CombineWith, SIGNAL(currentIndexChanged(int)), SLOT(EnableControls()));

		m_CombineButton = new QPushButton(tr("Combine Cargo..."), this);
		m_CombineButton->setIcon(QIcon(":data/CombineCargo.png"));
		m_CombineButton->setToolTip(tr("Combines the selected cargo with the cargo shown in the combo box. "
					"<br>"
					"The the higher Q cargo is merged with the lower Q cargo. "
					"The combined result is the lower Q of the two. "
					"<br>"
					"This is useful for building bundles of ammunition or lenses for bay weapons. "));
		m_CombineButton->setAutoDefault(true);
		m_CombineButton->setDefault(true);
		connect(m_CombineButton, SIGNAL(clicked()), SLOT(CombinePressed()));

		QLabel* lbcount = new QLabel(tr("Count To Jettison"), this);
		lbcount->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		m_CountEdit = new QLineEdit(this);
		m_CountEdit->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
		connect(m_CountEdit, SIGNAL(textChanged(const QString&)), SLOT(EnableControls()));

		m_JettisonButton = new QPushButton(tr("Jettison Cargo..."), this);
		m_JettisonButton->setIcon(QIcon(":data/CommJettison.png"));
		m_JettisonButton->setToolTip(tr("Jettison the selected cargo. "
					"<br>"
					"This can also be done by double clicking an item."));
		m_JettisonButton->setAutoDefault(true);
		m_JettisonButton->setDefault(true);
		connect(m_JettisonButton, SIGNAL(clicked()), SLOT(JettisonPressed()));

		m_OpenTEUButton = new QPushButton(tr("Open TEU Container..."), this);
		m_OpenTEUButton->setIcon(QIcon(":data/c_Container.png"));
		m_OpenTEUButton->setToolTip(tr("Opens the selected TEU container shipment. "
					"The entire contents of the shipment is emptied into the hold. "
					"The empty containers are jettisoned. "));
		m_OpenTEUButton->setAutoDefault(true);
		m_OpenTEUButton->setDefault(true);
		connect(m_OpenTEUButton, SIGNAL(clicked()), SLOT(OpenTEUPressed()));

		dolay->addWidget(m_OpenTEUButton, dorow,2);
		dorow++;
		dolay->addWidget(lbcombine, dorow,0);
		dolay->addWidget(m_CombineWith, dorow,1);
		dolay->addWidget(m_CombineButton, dorow,2);
		dorow++;
		dolay->addWidget(lbcount, dorow,0);
		dolay->addWidget(m_CountEdit, dorow,1);
		dolay->addWidget(m_JettisonButton, dorow,2);
		dorow++;
		dolay->setColumnStretch(1,1);
	}

	lay->addLayout(dolay, row,0, 1,2);
	row++;
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuCargoJettison::~AuCargoJettison()
{
}

void AuCargoJettison::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_timer)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_timer->isActive())
		m_timer->start(500);
}

void AuCargoJettison::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Kill the timer. It will be recreated during the next showEvent.
	if(m_timer && m_timer->isActive())
		m_timer->stop();
}

void AuCargoJettison::EnableControls()
{
	// Offer count
	unsigned int count = QVariant(m_CountEdit->text()).toUInt();

	// Enable/disable the jettison button.
	bool enableOk = false;
	if(m_MaxCount && count && count <= m_MaxCount)
		enableOk = true;
	m_JettisonButton->setEnabled(enableOk);
	m_CountEdit->setEnabled(enableOk);

	// Enable/disable the combine button
	bool combineOk = false;
	bool openOk = false;
	QItemSelectionModel* ism = m_CargoList->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		if(!sel.isEmpty())
		{
			QModelIndex idxsel = sel.first();
			if(idxsel.isValid())
			{
				QListWidgetItem* cur = (QListWidgetItem*)idxsel.internalPointer();
				QByteArray code = cur->data(Qt::UserRole).toByteArray();
				AuItem* item = AuItem::Unpack(code);
				if(item)
				{
					openOk = item->Type() == AuItem::IT_TEUContainer;
					combineOk = item->CanCombine() && m_CombineWith->count();
					delete item;
				}
			}
		}
	}
	m_CombineButton->setEnabled(combineOk);
	m_CombineWith->setEnabled(combineOk);
	m_OpenTEUButton->setEnabled(openOk);
}

void AuCargoJettison::JettisonPressed()
{
	if(!m_CargoList || !m_CargoList->isVisible())
		return;

	// Check for selected cargo item first.
	QByteArray code;
	QItemSelectionModel* ism = m_CargoList->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		if(!sel.isEmpty())
		{
			QModelIndex idxsel = sel.first();
			if(idxsel.isValid())
			{
				QListWidgetItem* cur = (QListWidgetItem*)idxsel.internalPointer();
				code = cur->data(Qt::UserRole).toByteArray();
			}
		}
	}

	if(!code.isEmpty())
	{
		AuItem* item = AuItem::Unpack(code);
		if(item)
		{
			unsigned int count = QVariant(m_CountEdit->text()).toUInt();
			if(count && count <= item->Count())
			{
				item->SetCount(count);
				QString smsg;
				const AuItemTEUContainer* teuItem = dynamic_cast<const AuItemTEUContainer*>(item);
				if(teuItem && teuItem->IsInsured())
					smsg = tr("Jettison %1. "
							"The shipment is insured by the %2. "
							"Are you sure?").arg(item->ListDesc()).arg(InsurerName());
				else
					smsg = tr("Jettison %1.").arg(item->ListDesc());
				QMessageBox mb(g_Au.ApplicationName(), smsg, QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel,
						QMessageBox::NoButton, this);
				int result = auApp->Frame()->ExecMessageBox(mb);
				if(result == QMessageBox::Ok)
					JettisonCargo(*item);
			}
			delete item;
		}
	}
}

void AuCargoJettison::CargoSelected()
{
	if(!m_CargoList || !m_CargoList->isVisible())
		return;

	QString cargotext;
	m_MaxCount = 0;
	QItemSelectionModel* ism = m_CargoList->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		if(!sel.isEmpty())
		{
			QModelIndex idxsel = sel.first();
			if(idxsel.isValid())
			{
				QListWidgetItem* cur = (QListWidgetItem*)idxsel.internalPointer();
				QByteArray code = cur->data(Qt::UserRole).toByteArray();
				AuItem* item = AuItem::Unpack(code);
				if(item)
				{
					QSize packSize = item->BitmapSizeToFit(100);
					m_MaxCount = item->Count();
					cargotext = QString("<html>"
							"<img align=\"right\" width=\"%3\" height=\"%4\" src=\"qrc%1\">%2"
							"</html>")
						.arg(item->BitmapName())
						.arg(item->HtmlDesc())
						.arg(packSize.width())
						.arg(packSize.height());

					// Fill the combine combo box with items that can be combined with the item.
					m_CombineWith->blockSignals(true);
					m_CombineWith->clear();
					if(item->CanCombine())
					{
						for(int it = 0; it < m_CargoList->count(); it++)
						{
							QListWidgetItem* comb = m_CargoList->item(it);
							if(comb == cur)
								continue;
							QByteArray combcode = comb->data(Qt::UserRole).toByteArray();
							AuItem* combitem = AuItem::Unpack(combcode);
							if(combitem)
							{
								if(combitem->CommodityId() == item->CommodityId() && combitem->CanCombine())
								{
									QString text = combitem->ListDesc();
									m_CombineWith->addItem(QIcon(combitem->BitmapName()), text, combcode);
								}
								delete combitem;
							}
						}
					}
					delete item;
					m_CombineWith->blockSignals(false);
				}
			}
		}
	}
	m_CargoDesc->setHtml(cargotext);

	// Jettison count
	m_CountEdit->setText(QVariant(m_MaxCount).toString());

	EnableControls();
}

// Returns false if the list widget is not filled because we are waiting for info from the server.
void AuCargoJettison::ListCargo(DaVessel* ship)
{
	int cargoTotal = ship->CargoCapacityVolume();
	int cargoUsed = ship->CargoTotalVolume();
	m_FillCargo->setRange(0, cargoTotal);
	m_FillCargo->setValue(cargoUsed);

	int fuelTotal = (int)ship->FuelCapacity();
	int fuelUsed = (int)ship->FuelQuantity();
	m_FillFuel->setRange(0, fuelTotal);
	m_FillFuel->setValue(fuelUsed);

	if(!m_CargoList->isVisible())
		return;

	// Insure we have the empire and specie data loaded for all items in cargo, not including items in containers.
	const AuItemHash& cr = ship->LockCargoForRead();
	if(auApp->QueryEmpireAndSpecie(cr))
	{
		ship->UnlockCargo();
		m_CargoList->clear();
		return;
	}

	// Remember the currently selected item.
	QByteArray selCode;
	QItemSelectionModel* ism = m_CargoList->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		if(!sel.isEmpty())
		{
			QModelIndex idxsel = sel.first();
			if(idxsel.isValid())
			{
				QListWidgetItem* cur = (QListWidgetItem*)idxsel.internalPointer();
				selCode = cur->data(Qt::UserRole).toByteArray();
			}
		}
	}

	// Get a list of the commodities that fit into the current filter.
	QMap<QString, AuCargoListItem> cargoItems;
	QList<COMMODITYID> cidFilter;
	g_Au.m_CommodityArray.ListFilterCommodities(cidFilter, m_CargoFilter);

	for(AuItemHash::const_iterator cit = cr.begin(); cit != cr.end(); cit++)
	{
		// Apply filter to list.
		if(!cidFilter.contains((*cit).second->CommodityId()))
			continue;

		QString text = (*cit).second->ListDesc();
		QByteArray code = (*cit).second->Pack();

		AuCargoListItem item;
		item.m_Code = code;
		item.m_IconName = (*cit).second->BitmapName();
		item.m_ToolTip = (*cit).second->HtmlToolTip();
		cargoItems.insert(text, item);
	}
	ship->UnlockCargo();

	if(m_CargoFilter == FILTER_All || m_CargoFilter == FILTER_Fuel)
	{
		// List Hydrogen fuel as cargo.
		int fuelQty = (int)ship->FuelQuantity();
		if(fuelQty)
		{
			AuItem* fuelItem = AuItem::Create(CID_Hydrogen, ship->FuelQuality(), ship->EmpireId(), fuelQty);
			if(fuelItem)
			{
				QString text = QObject::tr("%1 in fuel cells").arg(fuelItem->ListDesc());
				QByteArray code = fuelItem->Pack();

				AuCargoListItem item;
				item.m_Code = code;
				item.m_IconName = fuelItem->BitmapName();
				item.m_ToolTip = fuelItem->HtmlToolTip();
				cargoItems.insert(text, item);

				delete fuelItem;
			}
		}
	}

	if(cargoItems != m_CargoItems)
	{
		m_CargoItems = cargoItems;
		m_CargoList->blockSignals(true);
		m_CargoList->clear();
		QListWidgetItem* selectedItem = 0;
		for(QMap<QString, AuCargoListItem>::const_iterator cli = cargoItems.cbegin(); cli != cargoItems.cend(); cli++)
		{
			QPixmap px32 = QPixmap((*cli).m_IconName).scaled(32,32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			QIcon icon(px32);

			QListWidgetItem* lwi = new QListWidgetItem(icon, cli.key(), m_CargoList);
			lwi->setData(Qt::UserRole, (*cli).m_Code);
			lwi->setToolTip((*cli).m_ToolTip);
			if((*cli).m_Code == selCode)
				selectedItem = lwi;
		}
		if(selectedItem)
		{
			m_CargoList->scrollToItem(selectedItem);
			selectedItem->setSelected(true);
		}
		m_CargoList->blockSignals(false);
	}
}

void AuCargoJettison::FilterCargo(COMMODITYFILTER filter)
{
	m_CargoFilter = filter;
	UpdateContents();
}

void AuCargoJettison::CargoDoubleClicked(QListWidgetItem* item)
{
	if(item)
		JettisonPressed();
}

void AuCargoJettison::UpdateContents()
{
	// Lock the root and find the scene atom.
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		// List ship's cargo items.
		// Don't list cargo items if the person is not an officer of the ship.
		// Don't list cargo items if the ship is a subsidized station.
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship)
		{
			if(pView->IsPersonInCommand())
				m_CargoList->show();
			else
				m_CargoList->hide();
			m_FillCargo->show();
			m_FillFuel->show();
			ListCargo(ship);	// List the ship's cargo but not the contents of containers, which aren't allowed in cargo.
		}
		else
		{
			m_CargoList->hide();
			m_FillCargo->hide();
			m_FillFuel->hide();
		}
	}
	pView->UnlockSceneAtom();

	EnableControls();
}

void AuCargoJettison::JettisonCargo(const AuItem& item)
{
	if(!item.Count())
		return;

	QString empireNameOfOrigin;
	if(item.EmpireOfOrigin())
	{
		const DgData& gd = g_Au.LockDataForRead();
		gd.EmpireName(item.EmpireOfOrigin(), empireNameOfOrigin);
		g_Au.UnlockData();
	}

	AuView* view = auApp->Frame()->View();
	AuCommJettison msg(item, empireNameOfOrigin,
			AuCommStdChannel::IntercomChannelName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
			view->SceneGalaxy(), AuCommStdChannel::Close, AuCommStdChannel::Intercom);
	auApp->SendChatMessage(msg);
}

void AuCargoJettison::CombinePressed()
{
	if(!m_CargoList->isVisible())
		return;
	if(m_CombineWith->currentIndex() < 0)
		return;

	// Check for selected cargo item first.
	QByteArray code;
	QItemSelectionModel* ism = m_CargoList->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		if(!sel.isEmpty())
		{
			QModelIndex idxsel = sel.first();
			if(idxsel.isValid())
			{
				QListWidgetItem* cur = (QListWidgetItem*)idxsel.internalPointer();
				code = cur->data(Qt::UserRole).toByteArray();
			}
		}
	}

	// Get combine item code.
	QByteArray combcode = m_CombineWith->itemData(m_CombineWith->currentIndex()).toByteArray();
	if(combcode == code)
		return;	// wut?
	AuItem* item = AuItem::Unpack(code);
	if(!item)
		return;	// wut?
	AuItem* combitem = AuItem::Unpack(combcode);
	if(!combitem)
	{
		delete item;
		return;	// wut?
	}
	if(!item->CanCombine() || !combitem->CanCombine())
	{
		delete item;
		delete combitem;
		return;	// wut?
	}

	QString smsg;
	if(item->Quality() > combitem->Quality())
		smsg = tr("Combine %1 into %2.").arg(item->ListDesc()).arg(combitem->ListDesc());
	else
		smsg = tr("Combine %2 into %1.").arg(item->ListDesc()).arg(combitem->ListDesc());
	QMessageBox mb(g_Au.ApplicationName(), smsg, QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel,
			QMessageBox::NoButton, this);
	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Ok)
		return;

	QString empireNameOfOrigin;
	QString combEmpireNameOfOrigin;
	const DgData& gd = g_Au.LockDataForRead();
	if(item->EmpireOfOrigin())
		gd.EmpireName(item->EmpireOfOrigin(), empireNameOfOrigin);
	if(combitem->EmpireOfOrigin())
		gd.EmpireName(combitem->EmpireOfOrigin(), combEmpireNameOfOrigin);
	g_Au.UnlockData();

	AuView* view = auApp->Frame()->View();
	AuCommCombine msg(*item, empireNameOfOrigin, *combitem, combEmpireNameOfOrigin,
			AuCommStdChannel::IntercomChannelName(), view->SceneAtomId(), view->SceneAtomName(), view->SceneSector(),
			view->SceneGalaxy(), AuCommStdChannel::Close, AuCommStdChannel::Intercom);
	auApp->SendChatMessage(msg);

	delete item;
	delete combitem;
}

void AuCargoJettison::OpenTEUPressed()
{
	if(!m_CargoList->isVisible())
		return;

	// Check for selected cargo item first.
	QByteArray code;
	QItemSelectionModel* ism = m_CargoList->selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		if(!sel.isEmpty())
		{
			QModelIndex idxsel = sel.first();
			if(idxsel.isValid())
			{
				QListWidgetItem* cur = (QListWidgetItem*)idxsel.internalPointer();
				code = cur->data(Qt::UserRole).toByteArray();
			}
		}
	}

	if(!code.isEmpty())
	{
		AuItem* item = AuItem::Unpack(code);
		if(item)
		{
			const AuItemTEUContainer* teuItem = dynamic_cast<const AuItemTEUContainer*>(item);
			if(teuItem)
			{
				QString smsg;
				if(teuItem->IsInsured())
					smsg = tr("Open %1. "
							"The shipment is insured by the %2. "
							"Are you sure?").arg(item->ListDesc()).arg(InsurerName());
				else
					smsg = tr("Open %1.").arg(item->ListDesc());
				QMessageBox mb(g_Au.ApplicationName(), smsg, QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel,
						QMessageBox::NoButton, this);
				AuFrame* frame = auApp->Frame();
				int result = frame->ExecMessageBox(mb);
				if(result == QMessageBox::Ok)
				{
					AuView* view = frame->View();
					AuCommOpenTEU msg(*item, AuCommStdChannel::IntercomChannelName(), view->SceneAtomId(),
							view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(), AuCommStdChannel::Close,
							AuCommStdChannel::Intercom);
					auApp->SendChatMessage(msg);
				}
			}
			delete item;
		}
	}
}

