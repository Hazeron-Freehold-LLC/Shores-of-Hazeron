//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augearwidget.cpp,v 1.25 2022/05/06 21:17:03 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augearwidget.h"
//
#include "auapp.h"
#include "aucommchannel.h"
#include "audnabase.h"
#include "auflowerpotsettings.h"
#include "auitemdesc.h"
#include "aulaborhandcraft.h"
#include "auframe.h"
#include "aupictureframesettings.h"
#include "austoragemedia.h"
#include "autoolbutton.h"
#include "auview.h"
#include "daperson.h"
#include "dgdata.h"
#include <QInputDialog>
#include <QLineEdit>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QTimer>

//static unsigned int s_ShowSpecies = 0;							// Last count of species listed as debug output.

AuGearTree::AuGearTree(QWidget* parent) : QTreeView(parent)
{
	m_ButtonsPressed = 0;

	//setAlternatingRowColors(true);
	setAllColumnsShowFocus(true);
	setSortingEnabled(true);
	setIconSize(QSize(72,72));
	//setTextElideMode(Qt::ElideNone);
	setWordWrap(true);

	header()->setStretchLastSection(false);
	header()->hide();
}

void AuGearTree::mouseMoveEvent(QMouseEvent* e)
{
	QTreeView::mouseMoveEvent(e);
	QPoint vec = e->pos() - m_PressPos;
	if(vec.manhattanLength() > 3)
		setCursor(Qt::ClosedHandCursor);
}

void AuGearTree::mouseDoubleClickEvent(QMouseEvent* e)
{
	QTreeView::mouseDoubleClickEvent(e);
	emit MouseDoubleClicked();
}

void AuGearTree::mousePressEvent(QMouseEvent* e)
{
	QTreeView::mousePressEvent(e);
	m_ButtonsPressed = e->buttons();
	m_PressPos = e->pos();
}

void AuGearTree::mouseReleaseEvent(QMouseEvent* e)
{
	QTreeView::mouseReleaseEvent(e);
	if(m_ButtonsPressed & Qt::RightButton)
		emit MouseRightClicked();
	else if(rect().contains(e->pos()))
		emit MouseReleasedInside();
	else
		emit MouseReleasedOutside();
	setCursor(Qt::ArrowCursor);
}

void AuGearTree::ConfigureSort()
{
	sortByColumn(0, Qt::AscendingOrder);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGearModel::AuGearModel()
{
}

void AuGearModel::SetMergeFlags(const QModelIndex& parentIdx)
{
	for(int row=0; row<rowCount(parentIdx); row++)
	{
		// Set the merge flag on the row item.
		QModelIndex idx = index(row,0, parentIdx);
		setData(idx, 1, Qt::UserRole+3);

		// Recurse
		//SetMergeFlags(idx);
	}
}

// Look for the model item under 'parentIdx' that corresponds to the item in 'gearSlot' and clear its merge flag.
void AuGearModel::ClearMergeFlag(const AuGearSlot& gearSlot, const AuGearPos& nuGearPos, const QModelIndex& parentIdx)
{
	for(int row=0; row<rowCount(parentIdx); row++)
	{
		// Skip rows where the flag has already been cleared.
		QModelIndex idx = index(row,0, parentIdx);
		if(data(idx, Qt::UserRole+3).toInt() == 0)
			continue;

		// Compare the encoded item data.
		AuItem* item = AuItem::Unpack(data(idx, Qt::UserRole+2).toByteArray());
		if(!item)
			continue;

		bool looksLike = gearSlot.ItemLooksLike(*item);
		delete item;
		if(!looksLike)
			continue;

		// Update the gear pos on the item.
		setData(idx, QVariant::fromValue(nuGearPos), Qt::UserRole+1);

		// Clear the merge flag on the matching row item.
		setData(idx, 0, Qt::UserRole+3);
		return;
	}
	//qDebug() << "error: AuGearModel::ClearMergeFlag" << gearSlot.SlotTypeName() << "item not found" << gearSlot.Item().ListDesc();
}

// Look for the model item under 'parentIdx' that corresponds to the item in 'gearSlot' and clear its merge flag.
void AuGearModel::RemoveFlaggedItems(const QModelIndex& parentIdx)
{
	int row = 0;
	while(row < rowCount(parentIdx))
	{
		QModelIndex idx = index(row,0, parentIdx);
		if(data(idx, Qt::UserRole+3).toInt() == 1)
			removeRow(row, parentIdx);	// Remove rows where the flag is still set.
		else
			row++;						// Skip rows where the flag has already been cleared.
	}
}

void AuGearModel::AddGearPos(const AuGearPos& gpos, const AuGear& gear)
{
	if(gpos.IsTopLevel())
	{
		QList<AuItemContentId> path;
		AddGearPos(gpos, gear, path, 0, QModelIndex());
	}
	else
	{
		QList<AuItemContentId> path(gpos.ContentPath());
		AuGearPos topPos(gpos.Slot(), gpos.Which());
		for(int row=0; row<rowCount(QModelIndex()); row++)
		{
			QModelIndex idx = index(row,0, QModelIndex());

			QStandardItem* col0Item = itemFromIndex(idx);
			AuGearPos gearPos = col0Item->data(Qt::UserRole+1).value<AuGearPos>();
			if(gearPos == topPos)
			{
				AddGearPos(gpos, gear, path, col0Item, idx);
				break;
			}
		}
	}
}

void AuGearModel::AddGearPos(const AuGearPos& gpos, const AuGear& gear, QList<AuItemContentId>& path, QStandardItem* parentItem,
		const QModelIndex& parentIdx)
{
	if(path.size() <= 1)
	{
		// Add a new item.
		const AuItem* item = gear.ItemAt(gpos);
		if(!item)
			return;

		QString itemName(item->ListName());
		QSize psz(item->BitmapSize(12, 3));	// max height of a pack item is 20, therefore max icoht is 72.
		int icowid = psz.width();
		int icoht = psz.height();
		QIcon pico(QPixmap(item->BitmapName()).scaled(icowid, icoht, Qt::KeepAspectRatio, Qt::SmoothTransformation));

		QList<QStandardItem*> rowItems;

		QStandardItem* col0Item = new QStandardItem(pico, itemName);
		col0Item->setToolTip(item->HtmlToolTip());
		col0Item->setData(QVariant::fromValue(gpos), Qt::UserRole+1);
		col0Item->setData(item->Pack(), Qt::UserRole+2);
		rowItems.append(col0Item);

		// Top level items have an icon representing the slot type.
		if(gpos.IsTopLevel())
		{
			// Sorting of top level items.
			// It bugs me when I see Helmet, Clothing, Armor in the list. I always want to switch the Amor and the Clothing.
			if(gpos.Slot() == AuGearSlot::BodyWear)
			{
				// Force armor to sort before clothing.
				if(item->IsBodyArmor())
					col0Item->setData(QString("%1A%2").arg(AuGearSlot::SlotTypeSortName(gpos.Slot())).arg(gpos.Which()),
							Qt::UserRole);
				else
					col0Item->setData(QString("%1C%2").arg(AuGearSlot::SlotTypeSortName(gpos.Slot())).arg(gpos.Which()),
							Qt::UserRole);
			}
			else
				col0Item->setData(QString("%1%2").arg(AuGearSlot::SlotTypeSortName(gpos.Slot())).arg(gpos.Which()), Qt::UserRole);

			const int ticosz = 20;
			QIcon tico(QPixmap(AuGearSlot::SlotTypeIcon(gpos.Slot())).scaled(ticosz, ticosz, Qt::KeepAspectRatio, Qt::SmoothTransformation));
			QStandardItem* col1Item = new QStandardItem(tico, "");
			rowItems.append(col1Item);
			switch(gpos.Slot())
			{
			case AuGearSlot::Invalid:
			case AuGearSlot::Type_Count:
				break;
			case AuGearSlot::Hand:
				col1Item->setToolTip(tr("<html>%1 is carried in your %2.</html>")
						.arg(itemName)
						.arg(AuGearSlot::SlotTypeName(gpos.Slot())));
				break;
			case AuGearSlot::Head:
			case AuGearSlot::Face:
			case AuGearSlot::Neck:
			case AuGearSlot::Limb:
			case AuGearSlot::Digit:
				col1Item->setToolTip(tr("<html>%1 is worn on your %2.</html>")
						.arg(itemName)
						.arg(AuGearSlot::SlotTypeName(gpos.Slot())));
				break;
			case AuGearSlot::Body:
				col1Item->setToolTip(tr("<html>%1 is carried on your %2.</html>")
						.arg(itemName)
						.arg(AuGearSlot::SlotTypeName(gpos.Slot())));
				break;
			case AuGearSlot::BodyWear:
				col1Item->setToolTip(tr("<html>%1 is worn on your body.</html>")
						.arg(itemName));
				break;
			case AuGearSlot::Waist:
				col1Item->setToolTip(tr("<html>%1 is carried at your %2.</html>")
						.arg(itemName)
						.arg(AuGearSlot::SlotTypeName(gpos.Slot())));
				break;
			}

			// Add the new row at the top-level.
			insertRow(rowCount(), rowItems);
		}
		else
		{
			col0Item->setData(itemName, Qt::UserRole);

			// Add the new row to it's parent item.
			parentItem->insertRow(parentItem->rowCount(), rowItems);
		}
		return;
	}

	// Search for the next hop in the path under the parent index.
	AuItemContentId contentId = path.takeFirst();
	Q_ASSERT(!path.isEmpty());

	// Search for the next hop in the path under the parent index.
	AuGearPos parentPos = parentItem->data(Qt::UserRole+1).value<AuGearPos>();
	AuGearPos contentPos(parentPos, contentId);
	for(int row=0; row<rowCount(parentIdx); row++)
	{
		QModelIndex idx = index(row,0, parentIdx);

		QStandardItem* col0Item = itemFromIndex(idx);
		AuGearPos gearPos = col0Item->data(Qt::UserRole+1).value<AuGearPos>();
		if(gearPos == contentPos)
		{
			// Found the next hop in the path subordinate to the parent item.
			AddGearPos(gpos, gear, path, col0Item, idx);
			break;
		}
	}
}

void AuGearModel::ListExpandedGearPos(const AuGearTree& tree, AuGearPosList& expanded, const QModelIndex& parentIdx)
{
	for(int row=0; row<rowCount(parentIdx); row++)
	{
		QModelIndex idx = index(row,0, parentIdx);
		if(tree.isExpanded(idx))
		{
			QStandardItem* item = itemFromIndex(idx);
			AuGearPos gearPos = item->data(Qt::UserRole+1).value<AuGearPos>();
			expanded.append(gearPos);

			// Recurse
			ListExpandedGearPos(tree, expanded, idx);
		}
	}
}

bool AuGearModel::RemoveGearPos(const AuGearPos& gpos, const QModelIndex& parentIdx)
{
	for(int row=0; row<rowCount(parentIdx); row++)
	{
		QModelIndex idx = index(row,0, parentIdx);

		QStandardItem* item = itemFromIndex(idx);
		AuGearPos gearPos = item->data(Qt::UserRole+1).value<AuGearPos>();
		if(gearPos == gpos)
		{
			removeRow(row, parentIdx);
			return true;
		}

		// Recurse
		if(RemoveGearPos(gpos, idx))
			return true;
	}
	return false;
}

bool AuGearModel::UpdateGearPos(const AuGearPos& gpos, const AuGear& gear, const QModelIndex& parentIdx)
{
	//const AuItem* item = gear.ItemAt(gpos);
	//if(item)
		//qDebug() << "AuGearModel::UpdateGearPos" << gpos << item->ListDesc();
	//else
		//qDebug() << "AuGearModel::UpdateGearPos" << gpos << "item NOT FOUND";

	for(int row=0; row<rowCount(parentIdx); row++)
	{
		QModelIndex col0Idx = index(row,0, parentIdx);

		QStandardItem* col0Item = itemFromIndex(col0Idx);

		AuGearPos gearPos = col0Item->data(Qt::UserRole+1).value<AuGearPos>();
		if(gearPos == gpos)
		{
			// Found the gear position. Update the contents.
			const AuItem* item = gear.ItemAt(gpos);
			if(!item)
				return false;

			//qDebug() << "AuGearModel::UpdateGearPos" << gpos << "updating item" << item->ListName();
			QString itemName(item->ListName());
			QSize psz(item->BitmapSize(12, 3));	// max height of a pack item is 20, therefore max icoht is 72.
			int icowid = psz.width();
			int icoht = psz.height();
			QIcon pico(QPixmap(item->BitmapName()).scaled(icowid, icoht, Qt::KeepAspectRatio, Qt::SmoothTransformation));

			col0Item->setText(itemName);
			col0Item->setIcon(pico);
			col0Item->setToolTip(item->HtmlToolTip());
			col0Item->setData(QVariant::fromValue(gpos), Qt::UserRole+1);
			col0Item->setData(item->Pack(), Qt::UserRole+2);

			// Top level items have an icon representing the slot type.
			if(gpos.IsTopLevel())
			{

				// It bugs me when I see Helmet, Clothing, Armor in the list. I always want to switch the Amor and the Clothing.
				if(gpos.Slot() == AuGearSlot::BodyWear)
				{
					// Force armor to sort before clothing.
					if(item->IsBodyArmor())
						col0Item->setData(QString("%1A%2").arg(AuGearSlot::SlotTypeSortName(gpos.Slot())).arg(gpos.Which()),
								Qt::UserRole);
					else
						col0Item->setData(QString("%1C%2").arg(AuGearSlot::SlotTypeSortName(gpos.Slot())).arg(gpos.Which()),
								Qt::UserRole);
				}
				else
					col0Item->setData(QString("%1%2").arg(AuGearSlot::SlotTypeSortName(gpos.Slot())).arg(gpos.Which()),
							Qt::UserRole);

				QModelIndex col1Idx = index(row,1, parentIdx);
				QStandardItem* col1Item = itemFromIndex(col1Idx);

				const int ticosz = 20;
				QIcon tico(QPixmap(AuGearSlot::SlotTypeIcon(gpos.Slot())).scaled(ticosz, ticosz, Qt::KeepAspectRatio, Qt::SmoothTransformation));
				col1Item->setIcon(tico);

				switch(gpos.Slot())
				{
				case AuGearSlot::Invalid:
				case AuGearSlot::Type_Count:
					break;
				case AuGearSlot::Hand:
					col1Item->setToolTip(tr("<html>%1 is carried in your %2.</html>")
							.arg(itemName)
							.arg(AuGearSlot::SlotTypeName(gpos.Slot())));
					break;
				case AuGearSlot::Head:
				case AuGearSlot::Face:
				case AuGearSlot::Neck:
				case AuGearSlot::Limb:
				case AuGearSlot::Digit:
					col1Item->setToolTip(tr("<html>%1 is worn on your %2.</html>")
							.arg(itemName)
							.arg(AuGearSlot::SlotTypeName(gpos.Slot())));
					break;
				case AuGearSlot::Body:
					col1Item->setToolTip(tr("<html>%1 is carried on your %2.</html>")
							.arg(itemName)
							.arg(AuGearSlot::SlotTypeName(gpos.Slot())));
					break;
				case AuGearSlot::BodyWear:
					col1Item->setToolTip(tr("<html>%1 is worn on your body.</html>")
							.arg(itemName));
					break;
				case AuGearSlot::Waist:
					col1Item->setToolTip(tr("<html>%1 is carried at your %2.</html>")
							.arg(itemName)
							.arg(AuGearSlot::SlotTypeName(gpos.Slot())));
					break;
				}
			}
			else
				col0Item->setData(itemName, Qt::UserRole);
			return true;
		}

		// Recurse
		if(UpdateGearPos(gpos, gear, col0Idx))
			return true;
	}
	return false;
}

void AuGearModel::Sort()
{
	setSortRole(Qt::UserRole);
	sort(0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuGearDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleGearUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGearWidget::AuGearWidget(QWidget* parent) : QWidget(parent)
{
	m_HandCraftDlg = 0;
	m_HasBody = false;
	m_HasBodyWear = false;
	m_HasDigit = false;
	m_HasEmptyBody = false;
	m_HasEmptyBodyWear = false;
	m_HasEmptyDigit = false;
	m_HasEmptyFace = false;
	m_HasEmptyHand = false;
	m_HasEmptyHead = false;
	m_HasEmptyLimb = false;
	m_HasEmptyNeck = false;
	m_HasEmptyWaist = false;
	m_HasFace = false;
	m_HasHand = false;
	m_HasHead = false;
	m_HasLimb = false;
	m_HasNeck = false;
	m_HasWaist = false;
	m_SceneAtomDNA = 0;
	m_SceneAtomHasAcid = false;
	m_SceneAtomHasFire = false;
	m_StorageMediaDlg = 0;
	m_Timer = 0;

	// Buttons
	m_HandCraft = new AuIconButton(QIcon(":data/it_HandCraft.png"), this);
	m_HandCraft->setToolTip(tr("<html>"
				"<b>Handcraft...</b>"
				"<p>"
				"Shows the handcraft window, for making things by hand. "
				"Hides the handcraft window if it is already visible. "
				"</p>"
				"<p>"
				"<b>If you are a free avatar, handcraft a flag to start an empire.</b>"
				"</p>"
				"</html>"));
	connect(m_HandCraft, SIGNAL(clicked()), SLOT(HandCraftClicked()));
	QSize craftIconSize(40,40);
	m_HandCraft->setIconSize(craftIconSize);

	m_Decontaminate = new AuIconButton(QIcon(":data/Decontaminate.png"), this);
	m_Decontaminate->hide();
	m_Decontaminate->setToolTip(tr("<html><b>Decontaminate</b><br>"
				"Removes acid contamination from your body using the item."
				"</html>"));
	connect(m_Decontaminate, SIGNAL(clicked()), SLOT(DecontaminateClicked()));

	m_Drink = new AuIconButton(QIcon(":data/c_Wine.png"), this);
	m_Drink->hide();
	m_Drink->setToolTip(tr("<html><b>Drink</b><br>"
				"Drinks a liquid item."
				"</html>"));
	connect(m_Drink, SIGNAL(clicked()), SLOT(DrinkClicked()));

	m_Eat = new AuIconButton(QIcon(":data/c_Vegetable.png"), this);
	m_Eat->hide();
	m_Eat->setToolTip(tr("<html><b>Eat</b><br>"
				"Eats a food item, if you are hungry."
				"</html>"));
	connect(m_Eat, SIGNAL(clicked()), SLOT(EatClicked()));

	m_Extinguish = new AuIconButton(QIcon(":data/Damage.png"), this);
	m_Extinguish->hide();
	m_Extinguish->setToolTip(tr("<html><b>Extinguish</b><br>"
				"Removes fire from your body using the item."
				"</html>"));
	connect(m_Extinguish, SIGNAL(clicked()), SLOT(ExtinguishClicked()));

	m_Light = new AuIconButton(QIcon(":data/Light.png"), this);
	m_Light->hide();
	m_Light->setToolTip(tr("<html><b>Light On/Off</b><br>"
				"Switches on/off the light emitted by the item."
				"</html>"));
	connect(m_Light, SIGNAL(clicked()), SLOT(LightClicked()));

	m_CollectBounty = new AuIconButton(QIcon(":data/JobBountyHunter.png"), this);
	m_CollectBounty->hide();
	m_CollectBounty->setToolTip(tr("<html><b>Collect Bounty</b>"
				"<p>"
				"Collects the bounty on the item. "
				"If the fugitive can be located, a message indicates success or failure and the bounty is removed from the item. "
				"</p>"
				"<p>"
				"If the fugitive cannot be located, "
				"<u>there is no response</u> and the <u>bounty is not removed</u> from the item. "
				"Try again later to collect the bounty. "
				"</p>"
				"<p>"
				"Bounty cannot be collect on your own head. "
				"</p>"
				"</html>"));
	connect(m_CollectBounty, SIGNAL(clicked()), SLOT(CollectBountyClicked()));

	m_Sound = new AuIconButton(QIcon(":data/Speaker.png"), this);
	m_Sound->hide();
	m_Sound->setToolTip(tr("<html><b>Sound On/Off</b><br>"
				"Switches on/off the sound emitted by the item."
				"</html>"));
	connect(m_Sound, SIGNAL(clicked()), SLOT(SoundClicked()));

	m_StorageMedia = new AuIconButton(QIcon(":data/c_Computer.png"), this);
	m_StorageMedia->hide();
	m_StorageMedia->setToolTip(tr("<html><b>Manage Contents of Storage Media</b><br>"
				"Shows a list of data stored on media. "
				"Offers controls for managing the data on the storage media. "
				"</html>"));
	connect(m_StorageMedia, SIGNAL(clicked()), SLOT(StorageMediaClicked()));

	m_Visor = new AuIconButton(QIcon(":data/Visor.png"), this);
	m_Visor->hide();
	m_Visor->setToolTip(tr("<html><b>Visor Open/Close</b><br>"
				"Opens or closes the visor on an environment helmet. "
				"When closed and worn, the helmet provides breathable air in hostile environments, "
				"if its environment seal is intact. "
				"When closed and worn, your body recovers stamina more slowly than when it is open."
				"</html>"));
	connect(m_Visor, SIGNAL(clicked()), SLOT(VisorClicked()));

	m_Arm = new AuIconButton(QIcon(":data/CommDetonate.png"), this);
	m_Arm->hide();
	m_Arm->setToolTip(tr("<html><b>Arm and Drop</b><br>"
				"Arms a remotely detonated fuse on the item then drops it at your location. "
				"Use the <b>Detonate</b> button on the <b>Hail</b> comm channel to detonate the fuse."
				"</html>"));
	connect(m_Arm, SIGNAL(clicked()), SLOT(ArmClicked()));

	m_Fuse = new AuIconButton(QIcon(":data/c_Dynamite.png"), this);
	m_Fuse->hide();
	m_Fuse->setToolTip(tr("<html><b>Fuse and Drop</b><br>"
				"Initiates a fuse on the item then drops it at your location. "
				"The fuse detonates a few seconds after it is initiated."
				"</html>"));
	connect(m_Fuse, SIGNAL(clicked()), SLOT(FuseClicked()));

	m_StarmapLoc = new AuIconButton(QIcon(":data/RangeGalaxy.png"), this);
	m_StarmapLoc->hide();
	m_StarmapLoc->setToolTip(tr("<html><b>Locate on Star Map</b><br>"
				"Displays on the star map the location referenced by the item. "
				"The star map window must be open already; it is not opened automatically."
				"</html>"));
	connect(m_StarmapLoc, SIGNAL(clicked()), SLOT(StarmapLocClicked()));

	m_Heal = new AuIconButton(QIcon(":data/Heal.png"), this);
	m_Heal->hide();
	m_Heal->setToolTip(tr("<html><b>Heal</b><br>"
				"Uses the item on your body for its curative properties."
				"</html>"));
	connect(m_Heal, SIGNAL(clicked()), SLOT(UseClicked()));

	m_Claim = new AuIconButton(QIcon(":data/c_Patent.png"), this);
	m_Claim->hide();
	m_Claim->setToolTip(tr("<html><b>Claim Ownership</b><br>"
				"Uses the item to claim ownership of the property it identifies."
				"</html>"));
	connect(m_Claim, SIGNAL(clicked()), SLOT(UseClicked()));

	/*
	m_Use = new AuIconButton(QIcon(":data/Jobs.png"), this);
	m_Use->hide();
	m_Use->setToolTip(tr("<html><b>Use</b><br>"
				"Uses the item. "
				"When applicable, the item is used on yourself. "
				"Not all items do anything when used. "
				"Some items do nothing except under certain conditions. "
				"e.g. Using a medical kit heals your body, but only if you are injured."
				"</html>"));
	connect(m_Use, SIGNAL(clicked()), SLOT(UseClicked()));
	*/

	m_SetMessage = new AuIconButton(QIcon(":data/SensorScan.png"), this);
	m_SetMessage->hide();
	m_SetMessage->setToolTip(tr("<html><b>Set Message...</b><br>"
				"Configures the message that is broadcast by the item."
				"</html>"));
	connect(m_SetMessage, SIGNAL(clicked()), SLOT(SetMessageClicked()));

	m_Inspect = new AuIconButton(QIcon(":data/Magnifier.png"), this);
	m_Inspect->hide();
	m_Inspect->setToolTip(tr("<html><b>Inspect...</b><br>"
				"Displays information about the item. "
				"A model is displayed for examination, depicting the item as it appears in the environment."
				"</html>"));
	connect(m_Inspect, SIGNAL(clicked()), SLOT(InspectClicked()));

	m_Drop = new AuIconButton(QIcon(":data/DropItem.png"), this);
	m_Drop->hide();
	m_Drop->setToolTip(tr("<html><b>Drop</b><br>"
				"Drops the item. "
				"The item is dropped at your location."
				"</html>"));
	connect(m_Drop, SIGNAL(clicked()), SLOT(DropClicked()));

	m_Inscribe = new AuIconButton(QIcon(":data/it_InscribeItem.png"), this);
	m_Inscribe->hide();
	m_Inscribe->setToolTip(tr("<html><b>Inscribe...</b><br>"
				"Sets the inscription on the item. "
				"Inscriptions are permanent; they can never be changed or removed."
				"</html>"));
	connect(m_Inscribe, SIGNAL(clicked()), SLOT(InscribeClicked()));

	m_NameSpecies = new AuIconButton(QIcon(":data/NameSpecie.png"), this);
	m_NameSpecies->hide();
	m_NameSpecies->setToolTip(tr("<html><b>Name Species...</b><br>"
				"Name the species of the DNA contained in the item."
				"</html>"));
	connect(m_NameSpecies, SIGNAL(clicked()), SLOT(NameSpeciesClicked()));

	m_Settings = new AuIconButton(QIcon(":data/Settings.png"), this);
	m_Settings->hide();
	m_Settings->setToolTip(tr("<html><b>Settings...</b><br>"
				"Configures settings specific to the item."
				"</html>"));
	connect(m_Settings, SIGNAL(clicked()), SLOT(SettingsClicked()));

	// Controls to display gear and other info.
	m_GearTree = new AuGearTree(this);
	m_GearTree->setModel(&m_GearModel);
	m_GearTree->setToolTip(tr("<html><b>Gear</b><br>"
				"Gear carried or worn by you.<br>"
				"<i>Tips</i><br>"
				"%1 Double-click item to use it.<br>"
				"%1 Right-click item for menu.<br>"
				"%1 Drag item to a container to place it inside.<br>"
				"%1 Drag item outside window to drop it.<br>"
				"%1 Drag ammo to a weapon to load the weapon.<br>"
				"%1 Drag seed to a pot to plant the seed."
				"<br>"
				"%1 Drag DNA sample kit to an item with DNA to get a sample or vice versa."
				"</html>").arg(g_Au.MiddleDotSymbol()));
	connect(m_GearTree, SIGNAL(pressed(const QModelIndex&)), SLOT(GearTreePressed(const QModelIndex&)));
	connect(m_GearTree, SIGNAL(MouseReleasedInside()), SLOT(GearTreeMove()));
	connect(m_GearTree, SIGNAL(MouseReleasedOutside()), SLOT(GearTreeDrop()));
	connect(m_GearTree, SIGNAL(MouseRightClicked()), SLOT(GearTreeRightClicked()));
	connect(m_GearTree, SIGNAL(MouseDoubleClicked()), SLOT(GearTreeDoubleClicked()));
	connect(m_GearTree->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
			SLOT(GearSelectionChanged(const QItemSelection&, const QItemSelection&)));

	m_Account = new QLineEdit(this);
	m_Account->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_Account->setReadOnly(true);
	m_Account->setToolTip(tr("<html><b>Account Balance</b><br>"
				"This is your personal account balance in <i>cronodollars</i> or simply <i>cronos</i>. "
				"The symbol for cronodollars is '%1'. Cronos are the universal currency of Hazeron.<br>"
				"The color will be red if you have an outstanding loan balance.</html>")
			.arg(g_Au.CurrencySymbol()));

	m_Armor = new QLineEdit(this);
	m_Armor->setReadOnly(true);
	m_Armor->setToolTip(tr("<html><b>Armor Factors</b><br>"
				"Armor factors derive from your skin and the gear you are wearing. "
				"Two numbers show the total effectiveness of armor, separated by a %. "
				"e.g. 5% 12hp.<br>"
				"%1 The first number is the percentage of damage that is <i>absorbed</i> by armor.<br>"
				"%1 The second number is the amount of damage that is <i>blocked</i> by armor.<br>"
				"%1 Hit points of damage are absorbed then blocked. "
				"e.g. 20hp strike vs 5% 12hp armor = 20hp - (.05 * 20hp) - 12hp = 7hp unblocked damage.<br>"
				"%1 Unblocked damage injures your physical body, reducing your overall health.<br>"
				"%1 Armor factors are shown for kinetic contact damage. "
				"Armor factors can vary considerably against other weapon effects and modes.<br>"
				"%1 Gear degrades 1 point for every 10hp of damage it reduces; "
				"hardened armor degrades only 1 point for every 20hp, when hit by the type of damage it is hardened against."
				"</html>")
			.arg(g_Au.MiddleDotSymbol()));

	m_Burden = new QLineEdit(this);
	m_Burden->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	m_Burden->setReadOnly(true);
	m_Burden->setToolTip(tr("<html><b>Burden</b><br>"
				"This is your burden in kilograms.<br>"
				"%1 Left number is the weight you are carrying. You will move more slowly as you carry more weight.<br>"
				"%1 Right number is the maximum weight you can carry. "
				"<b>You may not be able to move if you carry more weight than your maximum.</b></html>")
			.arg(g_Au.MiddleDotSymbol()));

	// Gear Slots to store top-level items.
	QSize slotIconSize(10,10);
	m_SlotBody = new AuIconButton(QIcon(AuGearSlot::SlotTypeIcon(AuGearSlot::Body)), this);
	m_SlotBody->setIconSize(slotIconSize);
	m_SlotBody->hide();
	m_SlotBody->setToolTip(tr("<html><b>%1</b><br>"
			"Carries the item on your body.<br>"
			"Your body can carry 1 item. "
			"This button is disabled if your body is already carrying an item."
			"</html>")
		.arg(AuGearSlot::SlotTypeName(AuGearSlot::Body)));
	connect(m_SlotBody, SIGNAL(clicked()), SLOT(SlotBodyClicked()));

	m_SlotBodyWear = new AuIconButton(QIcon(AuGearSlot::SlotTypeIcon(AuGearSlot::BodyWear)), this);
	m_SlotBodyWear->setIconSize(slotIconSize);
	m_SlotBodyWear->hide();
	m_SlotBodyWear->setToolTip(tr("<html><b>%1</b><br>"
			"Wears the armor or clothing on your body.<br>"
			"Your body can wear 1 armor and 1 clothing. "
			"This button is disabled if your body is already wearing armor and clothing."
			"</html>")
		.arg(AuGearSlot::SlotTypeName(AuGearSlot::BodyWear)));
	connect(m_SlotBodyWear, SIGNAL(clicked()), SLOT(SlotBodyWearClicked()));

	m_SlotDigit = new AuIconButton(QIcon(AuGearSlot::SlotTypeIcon(AuGearSlot::Digit)), this);
	m_SlotDigit->setIconSize(slotIconSize);
	m_SlotDigit->hide();
	m_SlotDigit->setToolTip(tr("<html><b>%1</b><br>"
			"Wears the item on an empty finger or toe.<br>"
			"Your digits can wear 4 items. "
			"This button is disabled if your digits are full."
			"</html>")
		.arg(AuGearSlot::SlotTypeName(AuGearSlot::Digit)));
	connect(m_SlotDigit, SIGNAL(clicked()), SLOT(SlotDigitClicked()));

	m_SlotFace = new AuIconButton(QIcon(AuGearSlot::SlotTypeIcon(AuGearSlot::Face)), this);
	m_SlotFace->setIconSize(slotIconSize);
	m_SlotFace->hide();
	m_SlotFace->setToolTip(tr("<html><b>%1</b><br>"
			"Wears the item on your face.<br>"
			"Your face can wear 1 item. "
			"This button is disabled if your face is already wearing an item."
			"</html>")
		.arg(AuGearSlot::SlotTypeName(AuGearSlot::Face)));
	connect(m_SlotFace, SIGNAL(clicked()), SLOT(SlotFaceClicked()));

	m_SlotHand = new AuIconButton(QIcon(AuGearSlot::SlotTypeIcon(AuGearSlot::Hand)), this);
	m_SlotHand->setIconSize(slotIconSize);
	m_SlotHand->hide();
	m_SlotHand->setToolTip(tr("<html><b>%1</b><br>"
			"Carries the item in your hand.<br>"
			"Your hands can carry 2 items. "
			"This button is disabled if your hands are full."
			"</html>")
		.arg(AuGearSlot::SlotTypeName(AuGearSlot::Hand)));
	connect(m_SlotHand, SIGNAL(clicked()), SLOT(SlotHandClicked()));

	m_SlotHead = new AuIconButton(QIcon(AuGearSlot::SlotTypeIcon(AuGearSlot::Head)), this);
	m_SlotHead->setIconSize(slotIconSize);
	m_SlotHead->hide();
	m_SlotHead->setToolTip(tr("<html><b>%1</b><br>"
			"Wears the hat or helmet on your head.<br>"
			"Your head can wear 1 hat and 1 helmet. "
			"This button is disabled if your head is already wearing a hat and a helmet."
			"</html>")
		.arg(AuGearSlot::SlotTypeName(AuGearSlot::Head)));
	connect(m_SlotHead, SIGNAL(clicked()), SLOT(SlotHeadClicked()));

	m_SlotLimb = new AuIconButton(QIcon(AuGearSlot::SlotTypeIcon(AuGearSlot::Limb)), this);
	m_SlotLimb->setIconSize(slotIconSize);
	m_SlotLimb->hide();
	m_SlotLimb->setToolTip(tr("<html><b>%1</b><br>"
			"Wears the item on an arm or leg.<br>"
			"Your limbs can wear 4 items. "
			"This button is disabled if your limbs are full."
			"</html>")
		.arg(AuGearSlot::SlotTypeName(AuGearSlot::Limb)));
	connect(m_SlotLimb, SIGNAL(clicked()), SLOT(SlotLimbClicked()));

	m_SlotNeck = new AuIconButton(QIcon(AuGearSlot::SlotTypeIcon(AuGearSlot::Neck)), this);
	m_SlotNeck->setIconSize(slotIconSize);
	m_SlotNeck->hide();
	m_SlotNeck->setToolTip(tr("<html><b>%1</b><br>"
			"Wears the item on your neck.<br>"
			"Your neck can wear 1 item. "
			"This button is disabled if your neck is already wearing an item."
			"</html>")
		.arg(AuGearSlot::SlotTypeName(AuGearSlot::Neck)));
	connect(m_SlotNeck, SIGNAL(clicked()), SLOT(SlotNeckClicked()));

	m_SlotWaist = new AuIconButton(QIcon(AuGearSlot::SlotTypeIcon(AuGearSlot::Waist)), this);
	m_SlotWaist->setIconSize(slotIconSize);
	m_SlotWaist->hide();
	m_SlotWaist->setToolTip(tr("<html><b>%1</b><br>"
			"Carries the item at your waist. "
			"Items larger than 15cu cannot be carried at the waist. "
			"Tools in containers at your waist are readily available for construction and manufacturing.<br>"
			"Your waist can carry 4 items. "
			"This button is disabled if your waist is full."
			"</html>")
		.arg(AuGearSlot::SlotTypeName(AuGearSlot::Waist)));
	connect(m_SlotWaist, SIGNAL(clicked()), SLOT(SlotWaistClicked()));

	QHBoxLayout* buttonRow = new QHBoxLayout();
	buttonRow->addWidget(m_Arm);
	buttonRow->addWidget(m_Claim);
	buttonRow->addWidget(m_CollectBounty);
	buttonRow->addWidget(m_Decontaminate);
	buttonRow->addWidget(m_Drink);
	buttonRow->addWidget(m_Eat);
	buttonRow->addWidget(m_Extinguish);
	buttonRow->addWidget(m_Fuse);
	buttonRow->addWidget(m_Heal);
	buttonRow->addWidget(m_Light);
	//buttonRow->addWidget(m_Use);
	if(m_NameSpecies)
		buttonRow->addWidget(m_NameSpecies);
	buttonRow->addWidget(m_SetMessage);
	buttonRow->addWidget(m_Settings);
	buttonRow->addWidget(m_Sound);
	buttonRow->addWidget(m_StarmapLoc);
	buttonRow->addWidget(m_StorageMedia);
	buttonRow->addWidget(m_Visor);
	buttonRow->addWidget(m_Inscribe);
	buttonRow->addWidget(m_Inspect);
	buttonRow->addStretch();
	buttonRow->addWidget(m_Drop);
	buttonRow->setSpacing(2);
	buttonRow->setMargin(0);

	QHBoxLayout* slotButtonRow = new QHBoxLayout();
	slotButtonRow->addStretch();
	slotButtonRow->addWidget(m_SlotHand);
	slotButtonRow->addWidget(m_SlotBody);
	slotButtonRow->addWidget(m_SlotHead);
	slotButtonRow->addWidget(m_SlotFace);
	slotButtonRow->addWidget(m_SlotBodyWear);
	slotButtonRow->addWidget(m_SlotWaist);
	slotButtonRow->addWidget(m_SlotNeck);
	slotButtonRow->addWidget(m_SlotLimb);
	slotButtonRow->addWidget(m_SlotDigit);
	slotButtonRow->setSpacing(2);
	slotButtonRow->setMargin(0);

	QGridLayout* blay = new QGridLayout();
	blay->addWidget(m_HandCraft, 0,0, 2,1);
	blay->setColumnMinimumWidth(0, 50);
	blay->addLayout(slotButtonRow, 0,1);
	blay->setRowMinimumHeight(0, 22);
	blay->addLayout(buttonRow, 1,1);
	blay->setRowMinimumHeight(1, 28);

	QGridLayout* lay = new QGridLayout();
	int row=0;
	lay->addWidget(m_Account, row,0, 1,2);
	row++;
	lay->addWidget(m_Armor, row,0, 1,1);
	lay->addWidget(m_Burden, row,1, 1,1);
	row++;
	lay->addLayout(blay, row,0, 1,2);
	row++;
	lay->addWidget(m_GearTree, row,0, 1,2);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuGearWidget::~AuGearWidget()
{
	if(m_SceneAtomDNA)
		delete m_SceneAtomDNA;
}

void AuGearWidget::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_Timer)
	{
		m_Timer = new QTimer(this);
		connect(m_Timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_Timer->isActive())
		m_Timer->start(500);

	UpdateContents();
}

void AuGearWidget::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Kill the timer. It will be recreated during the next showEvent.
	if(m_Timer && m_Timer->isActive())
		m_Timer->stop();
}

void AuGearWidget::EnableControls()
{
	if(m_Gear.IsPosValid(m_PressPos) && m_PressPos != m_SelPos)
	{
		// During drag operation.
		m_HandCraft->setEnabled(false);
		m_Drop->setEnabled(false);
		m_Inspect->setEnabled(false);

		m_Arm->setEnabled(false);
		m_Claim->setEnabled(false);
		m_CollectBounty->setEnabled(false);
		m_Decontaminate->setEnabled(false);
		m_Drink->setEnabled(false);
		m_Eat->setEnabled(false);
		m_Extinguish->setEnabled(false);
		m_Fuse->setEnabled(false);
		m_Heal->setEnabled(false);
		m_Inscribe->setEnabled(false);
		m_Light->setEnabled(false);
		if(m_NameSpecies)
			m_NameSpecies->setEnabled(false);
		m_SetMessage->setEnabled(false);
		m_Settings->setEnabled(false);
		m_Sound->setEnabled(false);
		m_StarmapLoc->setEnabled(false);
		m_StorageMedia->setEnabled(false);
		//m_Use->setEnabled(false);
		m_Visor->setEnabled(false);

		m_SlotBody->setEnabled(false);
		m_SlotBodyWear->setEnabled(false);
		m_SlotDigit->setEnabled(false);
		m_SlotFace->setEnabled(false);
		m_SlotHand->setEnabled(false);
		m_SlotHead->setEnabled(false);
		m_SlotLimb->setEnabled(false);
		m_SlotNeck->setEnabled(false);
		m_SlotWaist->setEnabled(false);
		return;
	}

	// Not dragging an item.
	m_HandCraft->setEnabled(true);

	const AuItem* selItem = m_Gear.ItemAt(m_SelPos);
	if(!selItem)
	{
		// No item is selected.
		m_Arm->hide();
		m_Claim->hide();
		m_CollectBounty->hide();
		m_Decontaminate->hide();
		m_Drink->hide();
		m_Drop->hide();
		m_Eat->hide();
		m_Extinguish->hide();
		m_Fuse->hide();
		m_Heal->hide();
		m_Inscribe->hide();
		m_Light->hide();
		if(m_NameSpecies)
			m_NameSpecies->hide();
		m_SetMessage->hide();
		m_Settings->hide();
		m_Inspect->hide();
		m_SlotBody->hide();
		m_SlotBodyWear->hide();
		m_SlotDigit->hide();
		m_SlotFace->hide();
		m_SlotHand->hide();
		m_SlotHead->hide();
		m_SlotLimb->hide();
		m_SlotNeck->hide();
		m_SlotWaist->hide();
		m_Sound->hide();
		m_StarmapLoc->hide();
		m_StorageMedia->hide();
		//m_Use->hide();
		m_Visor->hide();
		return;
	}

	// An item is selected.
	m_Drop->show();
	m_Drop->setEnabled(true);
	m_Inspect->show();
	m_Inspect->setEnabled(true);

	// Enable all the buttons. We'll hide the ones we don't want.
	m_Arm->setEnabled(true);
	m_Claim->setEnabled(true);
	m_CollectBounty->setEnabled(true);
	m_Decontaminate->setEnabled(true);
	m_Drink->setEnabled(true);
	m_Eat->setEnabled(true);
	m_Extinguish->setEnabled(true);
	m_Fuse->setEnabled(true);
	m_Heal->setEnabled(true);
	m_Inscribe->setEnabled(true);
	m_Light->setEnabled(true);
	if(m_NameSpecies)
		m_NameSpecies->setEnabled(true);
	m_SetMessage->setEnabled(true);
	m_Settings->setEnabled(true);
	m_Sound->setEnabled(true);
	m_StarmapLoc->setEnabled(true);
	m_StorageMedia->setEnabled(true);
	//m_Use->setEnabled(true);
	m_Visor->setEnabled(true);

	// Gear slots are enabled if the person's body has an empty slot of that type.
	m_SlotBody->setEnabled(m_HasEmptyBody);
	m_SlotBodyWear->setEnabled(m_HasEmptyBodyWear);
	m_SlotDigit->setEnabled(m_HasEmptyDigit);
	m_SlotFace->setEnabled(m_HasEmptyFace);
	m_SlotHand->setEnabled(m_HasEmptyHand);
	m_SlotHead->setEnabled(m_HasEmptyHead);
	m_SlotLimb->setEnabled(m_HasEmptyLimb);
	m_SlotNeck->setEnabled(m_HasEmptyNeck);
	m_SlotWaist->setEnabled(m_HasEmptyWaist);

	// A lot of the buttons are mutually exclusive of each other.
	bool showArm = false;
	bool showClaim = false;
	bool showCollectBounty = !selItem->Bounty().Id().IsNull();
	bool enableCollectBounty = selItem->Bounty().Id() != m_GearId;
	bool showDecontaminate = false;
	bool showDrink = false;
	bool showEat = false;
	bool showExtinguish = false;
	bool showFuse = false;
	bool showHeal = false;
	bool showLight = false;
	bool showSound = false;
	bool showStarmapLoc = false;
	bool showStorageMedia = false;
	//bool showUse = false;
	bool showVisor = false;
	if(selItem->CommodityId() == CID_Water && m_SceneAtomHasAcid)
		showDecontaminate = true;
	else if(selItem->CommodityId() == CID_Water && m_SceneAtomHasFire)
		showExtinguish = true;
	else if(m_SceneAtomDNA && selItem->IsFood(m_SceneAtomDNA->EcoRole()))
	{
		if(selItem->CommodityId() == CID_Water || selItem->CommodityId() == CID_Milk)
			showDrink = true;
		else
			showEat = true;
	}
	else if(selItem->IsIntoxicant())
		showDrink = true;
	else if(selItem->CommodityId() == CID_SpacecraftTitle && !selItem->IsTitleVoid())
		showClaim = true;
	else if(selItem->CommodityId() == CID_MedicalKit ||
			selItem->CommodityId() == CID_Antidote ||
			selItem->CommodityId() == CID_Antitoxin)
		showHeal = true;
	else if(selItem->IsLight())
		showLight = true;
	else if(selItem->CommodityId() == CID_Drum)
		showSound = true;
	else if(selItem->IsEnvironmentHelmet())
		showVisor = true;
	else if(selItem->IsDetonatorWeapon())
		showArm = true;
	else if(selItem->IsFuseWeapon())
		showFuse = true;
	else if(dynamic_cast<const AuItemGalaxyLoc*>(selItem))
		showStarmapLoc = true;
	else if(selItem->Type() == AuItem::IT_StorageMedia)
		showStorageMedia = true;
	//else
		//showUse = true;

	// Anything can be inscribed if it is not already inscribed.
	bool showInscribe = false;
	if(selItem->Inscription().isEmpty())
		showInscribe = true;

	// Things with DNA can be used to name the species.
	bool showNameSpecies = false;
	if(m_NameSpecies && m_SceneAtomDNA)
	{
		const AuSpecies species = selItem->DNASpecies();
		if(!species.IsNull())
		{
			// The item is valid and appears to have DNA.
			// Determine if the creature is a wild animal.
			bool isWildAnimal = !selItem->DNAIsSentient();

			// Prevent naming of other sentient races (people and citizens) besides the scene atom's own race.
			// Agent is allowed to name/rename anything, including sentient races.
			const DgData& gd = g_Au.LockDataForRead();
			bool isAgent = gd.ClientPlayerIsAgent();
			g_Au.UnlockData();
			showNameSpecies = isAgent || isWildAnimal || species == m_SceneAtomDNA->Species();
		}
	}

	// This item is a light that can also have a message assigned.
	bool showSetMessage = dynamic_cast<const AuItemStateMessage*>(selItem);

	bool showSettings = DbCommodity::IsPicture(selItem->CommodityId()) ||
		DbCommodity::IsFlowerPot(selItem->CommodityId());

	// Gear slots are visible if the item can go in a particular gear slot and the person's body has that kind of gear slot.
	bool showBody = m_HasBody && AuGearSlot::CanHoldItem(AuGearSlot::Body, *selItem);
	bool showBodyWear = m_HasBodyWear && AuGearSlot::CanHoldItem(AuGearSlot::BodyWear, *selItem);
	bool showDigit = m_HasDigit && AuGearSlot::CanHoldItem(AuGearSlot::Digit, *selItem);
	bool showFace = m_HasFace && AuGearSlot::CanHoldItem(AuGearSlot::Face, *selItem);
	bool showHand = m_HasHand && AuGearSlot::CanHoldItem(AuGearSlot::Hand, *selItem);
	bool showHead = m_HasHead && AuGearSlot::CanHoldItem(AuGearSlot::Head, *selItem);
	bool showLimb = m_HasLimb && AuGearSlot::CanHoldItem(AuGearSlot::Limb, *selItem);
	bool showNeck = m_HasNeck && AuGearSlot::CanHoldItem(AuGearSlot::Neck, *selItem);
	bool showWaist = m_HasWaist && AuGearSlot::CanHoldItem(AuGearSlot::Waist, *selItem);

	// Hide the buttons we don't want before showing the ones we want. Due to the effect on the width of the window.
	if(!showArm)
		m_Arm->hide();
	if(!showClaim)
		m_Claim->hide();
	if(!showCollectBounty)
		m_CollectBounty->hide();
	if(!showDecontaminate)
		m_Decontaminate->hide();
	if(!showDrink)
		m_Drink->hide();
	if(!showEat)
		m_Eat->hide();
	if(!showExtinguish)
		m_Extinguish->hide();
	if(!showFuse)
		m_Fuse->hide();
	if(!showHeal)
		m_Heal->hide();
	if(!showInscribe)
		m_Inscribe->hide();
	if(!showLight)
		m_Light->hide();
	if(!showNameSpecies && m_NameSpecies)
		m_NameSpecies->hide();
	if(!showSetMessage)
		m_SetMessage->hide();
	if(!showSettings)
		m_Settings->hide();
	if(!showSound)
		m_Sound->hide();
	if(!showStarmapLoc)
		m_StarmapLoc->hide();
	if(!showStorageMedia)
		m_StorageMedia->hide();
	//if(!showUse)
		//m_Use->hide();
	if(!showVisor)
		m_Visor->hide();

	if(!showBody)
		m_SlotBody->hide();
	if(!showBodyWear)
		m_SlotBodyWear->hide();
	if(!showDigit)
		m_SlotDigit->hide();
	if(!showFace)
		m_SlotFace->hide();
	if(!showHand)
		m_SlotHand->hide();
	if(!showHead)
		m_SlotHead->hide();
	if(!showLimb)
		m_SlotLimb->hide();
	if(!showNeck)
		m_SlotNeck->hide();
	if(!showWaist)
		m_SlotWaist->hide();

	// Show the buttons we decided we wanted.
	if(showArm)
		m_Arm->show();
	if(showClaim)
		m_Claim->show();
	if(showCollectBounty)
	{
		m_CollectBounty->show();
		m_CollectBounty->setEnabled(enableCollectBounty);
	}
	if(showDecontaminate)
		m_Decontaminate->show();
	if(showDrink)
		m_Drink->show();
	if(showEat)
		m_Eat->show();
	if(showExtinguish)
		m_Extinguish->show();
	if(showFuse)
		m_Fuse->show();
	if(showHeal)
		m_Heal->show();
	if(showInscribe)
		m_Inscribe->show();
	if(showLight)
		m_Light->show();
	if(showNameSpecies && m_NameSpecies)
		m_NameSpecies->show();
	if(showSetMessage)
		m_SetMessage->show();
	if(showSettings)
		m_Settings->show();
	if(showSound)
		m_Sound->show();
	if(showStarmapLoc)
		m_StarmapLoc->show();
	if(showStorageMedia)
		m_StorageMedia->show();
	//if(showUse)
		//m_Use->show();
	if(showVisor)
		m_Visor->show();

	if(showBody)
		m_SlotBody->show();
	if(showBodyWear)
		m_SlotBodyWear->show();
	if(showDigit)
		m_SlotDigit->show();
	if(showFace)
		m_SlotFace->show();
	if(showHand)
		m_SlotHand->show();
	if(showHead)
		m_SlotHead->show();
	if(showLimb)
		m_SlotLimb->show();
	if(showNeck)
		m_SlotNeck->show();
	if(showWaist)
		m_SlotWaist->show();
}

void AuGearWidget::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be recreated during the next showEvent.
		if(m_Timer && m_Timer->isActive())
			m_Timer->stop();
		return;
	}

	/*
 	{
 		const DgData& gd = g_Au.LockDataForRead();
 		if(s_ShowSpecies != gd.m_Species.size())
 		{
 			s_ShowSpecies = gd.m_Species.size();
 			qDebug() << "AuGearWidget::UpdateContents species" << gd.m_Species.size();
 			for(DzSpeciesHash::const_iterator si = gd.m_Species.cbegin(); si != gd.m_Species.cend(); si++)
 			{
 				quint16 ck = qChecksum((*si).first.Key().data(), (*si).first.Key().size());
 				qDebug() << " " << (*si).second.Name() << (*si).second.Species().Description() << ck << (*si).first.Key().size()
					<< sizeof(DhDNA_) << (*si).first.Key();
 				if((*si).second.Species().Description() == "Carnivorous Insectoid")
 				{
 					AuDNABase* dna = (*si).first.CreateDNA();
 					if(dna->Version() == 3)
 					{
 						qDebug() << "  dna3" << ((DhDNA*)dna)->ToString();
 					}
 					delete dna;
 				}
 			}
 		}
 		g_Au.UnlockData();
	}
	*/

	AuView* view = auApp->Frame()->View();
	DaPerson* person;
	view->LockSceneAtom(&person);
	if(person)
	{
		if(m_SceneAtomDNA)
			delete m_SceneAtomDNA;
		m_SceneAtomDNA = person->DNAnew();
		m_SceneAtomHasAcid = person->HasCondition(AuConAcid::ConditionType());
		m_SceneAtomHasFire = person->HasCondition(AuConFire::ConditionType());

		m_HasBody = true;
		m_HasBodyWear = true;
		m_HasDigit = true;
		m_HasFace = true;
		m_HasHand = true;
		m_HasHead = true;
		m_HasLimb = true;
		m_HasNeck = true;
		m_HasWaist = true;

		m_HasEmptyBody = m_Gear.ItemsInSlot(AuGearSlot::Body, 0) < 1;
		m_HasEmptyBodyWear = m_Gear.ItemsInSlot(AuGearSlot::BodyWear, 0) < 2;
		m_HasEmptyDigit = m_Gear.ItemsInSlot(AuGearSlot::Digit, 0) < 4;
		m_HasEmptyFace = m_Gear.ItemsInSlot(AuGearSlot::Face, 0) < 1;
		m_HasEmptyHand = m_Gear.ItemsInSlot(AuGearSlot::Hand, 0) < 2;
		m_HasEmptyHead = m_Gear.ItemsInSlot(AuGearSlot::Head, 0) < 1;
		m_HasEmptyLimb = m_Gear.ItemsInSlot(AuGearSlot::Limb, 0) < 4;
		m_HasEmptyNeck = m_Gear.ItemsInSlot(AuGearSlot::Neck, 0) < 1;
		m_HasEmptyWaist = m_Gear.ItemsInSlot(AuGearSlot::Waist, 0) < 4;

		// Account Balance
		QString sacct;
		if(person->LoanBalance())
		{
			m_Account->setStyleSheet(QString("color: rgb(%1, %2, %3)") .arg(255) .arg(64) .arg(64));
			sacct = QString("%L1%2 %L3%2 debt")
				.arg(person->AccountBalance(),0,'f',0).arg(g_Au.CurrencySymbol())
				.arg(person->LoanBalance(),0,'f',0);
		}
		else
		{
			m_Account->setStyleSheet("");
			sacct = QString("%L1%2").arg(person->AccountBalance(),0,'f',0).arg(g_Au.CurrencySymbol());
		}
		if(m_Account->text() != sacct)
			m_Account->setText(sacct);

		// Armor
		float baseAbsorb, baseBlock, auxAbsorb, auxBlock;
		person->CombatArmorEffectiveness(WM_Contact, WE_Generic, false, AuPoint(0,0), AuPoint(0,0), baseAbsorb, baseBlock,
				auxAbsorb, auxBlock);
		float absorb = baseAbsorb + auxAbsorb;
		float block = baseBlock + auxBlock;
		QString scombatvals = g_Au.FormatArmorFactors(absorb, block);
		if(m_Armor->text() != scombatvals)
			m_Armor->setText(scombatvals);

		// Burden
		//float massKg = person->Mass() * KilogramsPerPound();
		float wtcKg = person->Burden() * KilogramsPerPound();
		float wtmKg = person->BurdenMax() * KilogramsPerPound();
		if(wtcKg > wtmKg*.9f)
			m_Burden->setStyleSheet(QString("color: rgb(%1, %2, %3)") .arg(255) .arg(85) .arg(85));
		else if(wtcKg > wtmKg*.5f)
			m_Burden->setStyleSheet(QString("color: rgb(%1, %2, %3)") .arg(255) .arg(244) .arg(85));
		else
			m_Burden->setStyleSheet("");
		//QString sburden = QString("%L1kg %L2kg %L3kg")
			//.arg(massKg,0,'f',1)
			//.arg(wtcKg,0,'f',1)
			//.arg(wtmKg,0,'f',1);
		QString sburden = QString("%L1kg %L2kg")
			.arg(wtcKg,0,'f',1)
			.arg(wtmKg,0,'f',1);
		if(m_Burden->text() != sburden)
			m_Burden->setText(sburden);

		// Gear
		if(m_GearId != person->Id())
		{
			// Scene atom changed. Discard current gear model and tree entirely.
			m_Gear.Clear();
			m_GearModel.clear();
			m_GearId = person->Id();
		}
		const AuGear* gear = person->LockGearForRead();
		if(gear && !gear->operator==(m_Gear))
		{
			//qDebug() << "AuGearWidget::UpdateContents update old size" << m_Gear.size() << "new size" << gear->size();
			//qDebug() << "old" << m_Gear;
			//qDebug() << "new" << *gear;

			AuGearPosList oldGearPos;
			m_Gear.ListAll(oldGearPos);

			AuGearPosList newGearPos;
			gear->ListAll(newGearPos);

			//AuGear oldGear(m_Gear);
			m_Gear = *gear;

			// Sync the model.
			for(AuGearPosList::const_iterator ni = newGearPos.cbegin(); ni != newGearPos.cend(); ni++)
				if(oldGearPos.contains(*ni))
					m_GearModel.UpdateGearPos(*ni, m_Gear);	// Update existing model item.
				else
					m_GearModel.AddGearPos(*ni, m_Gear);	// Create a new model item.

			// Remove model entries that are in the old gear pos list but not in the new gear pos list.
			for(AuGearPosList::const_iterator oi = oldGearPos.cbegin(); oi != oldGearPos.cend(); oi++)
				if(!newGearPos.contains(*oi))
					m_GearModel.RemoveGearPos(*oi);			// Delete old pos from the model.

			if(m_GearModel.rowCount())
			{
				m_GearTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
				m_GearTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
				m_GearTree->ConfigureSort();
				m_GearModel.Sort();
			}

			// Insure we have the empire and species data loaded for all of our items, including items in containers.
			auApp->QueryEmpireAndSpecie(*gear);
		}
		person->UnlockGear();
	}
	view->UnlockSceneAtom();

	if(m_StorageMediaDlg)
		m_StorageMediaDlg->UpdateContent();

	EnableControls();
}

/*
void AuGearWidget::BuildContainerTree(QStandardItem* parentItem, const AuGearPos& gearPos, const AuItemContent& content)
{
	for(AuItemContent::const_iterator ci = content.cbegin(); ci != content.cend(); ci++)
	{
		QList<QStandardItem*> rowItems;

		QString itemName((*ci).second->ListName());
		QSize psz((*ci).second->BitmapSize(12, 3));	// max height of a pack item is 20, therefore max icoht is 72.
		int icowid = psz.width();
		int icoht = psz.height();
		QIcon pico(QPixmap((*ci).second->BitmapName()).scaled(icowid, icoht, Qt::KeepAspectRatio, Qt::SmoothTransformation));

		AuGearPos contentPos(gearPos, (*ci).first);	// This will build a path as we recurse into containers.

		rowItems.append(new QStandardItem(pico, itemName));
		rowItems.last()->setToolTip((*ci).second->HtmlToolTip());
		rowItems.last()->setData(QVariant::fromValue(contentPos), Qt::UserRole+1);
		rowItems.last()->setData((*ci).second->Pack(), Qt::UserRole+2);
		//qDebug() << "tree item" << (*ci).second->ListName() << "gearPos" << gearPos.Slot(), gearPos.Which() << (*ci).first;

		if((*ci).second->Content())
			BuildContainerTree(rowItems.first(), contentPos, *(*ci).second->Content());

		parentItem->insertRow(parentItem->rowCount(), rowItems);
	}
}
*/

void AuGearWidget::GearTreePressed(const QModelIndex& idx)
{
	QStandardItem* item = m_GearModel.itemFromIndex(idx);
	if(item)
		m_PressPos = item->data(Qt::UserRole+1).value<AuGearPos>();
	else
		m_PressPos = AuGearPos();
	/*
	qDebug() << "AuGearWidget::GearTreePressed" << m_PressPos.Index() << m_PressPos.SubIndex();
	const AuItem* gearItem = m_Gear.ItemAt(m_PressPos);
	if(gearItem)
		qDebug() << "AuGearWidget::GearTreePressed item" << gearItem->ListName();
	else
	{
		qint8 gearIndex = 0;
		for(AuGear::const_iterator gi = m_Gear.cbegin(); gi != m_Gear.cend(); gi++, gearIndex++)
			qDebug() << gearIndex << 255 << (*gi).Item().ListName();
	}
	*/

	EnableControls();
}

void AuGearWidget::GearSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	Q_UNUSED(deselected);
	QModelIndexList sel = selected.indexes();
	if(sel.isEmpty())
	{
		m_SelPos = AuGearPos();
		return;
	}

	QStandardItem* item = m_GearModel.itemFromIndex(sel.first());
	m_SelPos = item->data(Qt::UserRole+1).value<AuGearPos>();

	EnableControls();
}

void AuGearWidget::GearTreeRightClicked()
{
	//qDebug() << "AuGearWidget::GearTreeRightClicked" << m_SelPos;
	const AuItem* selItem = m_Gear.ItemAt(m_SelPos);
	if(!selItem)
		return;
	QString itemName = selItem->ListName();
	int itemState = selItem->State();

	// Build a menu of available options.
	QMenu menu(this);

	if(m_Arm->isVisible() && m_Arm->isEnabled())
		menu.addAction(m_Arm->icon(), tr("Arm for Detonation and Drop %1").arg(itemName), this, SLOT(ArmClicked()));

	if(m_Claim->isVisible() && m_Claim->isEnabled())
		menu.addAction(m_Claim->icon(), tr("Claim Ownership of %2 Using %1").arg(itemName).arg(selItem->PropertyName()),
				this, SLOT(UseClicked()));

	if(m_CollectBounty->isVisible() && m_CollectBounty->isEnabled())
		menu.addAction(m_CollectBounty->icon(), tr("Collect Bounty on %1").arg(itemName), this, SLOT(CollectBountyClicked()));

	if(m_Decontaminate->isVisible() && m_Decontaminate->isEnabled())
		menu.addAction(m_Decontaminate->icon(), tr("Decontaminate Using %1").arg(itemName), this, SLOT(DecontaminateClicked()));

	if(m_Drink->isVisible() && m_Drink->isEnabled())
		menu.addAction(m_Drink->icon(), tr("Drink %1").arg(itemName), this, SLOT(DrinkClicked()));

	if(m_Eat->isVisible() && m_Eat->isEnabled())
		menu.addAction(m_Eat->icon(), tr("Eat %1").arg(itemName), this, SLOT(EatClicked()));

	if(m_Extinguish->isVisible() && m_Extinguish->isEnabled())
		menu.addAction(m_Extinguish->icon(), tr("Extinguish Using %1").arg(itemName), this, SLOT(ExtinguishClicked()));

	if(m_Fuse->isVisible() && m_Fuse->isEnabled())
		menu.addAction(m_Fuse->icon(), tr("Initiate Fuse and Drop %1").arg(itemName), this, SLOT(FuseClicked()));

	if(m_Heal->isVisible() && m_Heal->isEnabled())
		menu.addAction(m_Heal->icon(), tr("Heal Using %1").arg(itemName), this, SLOT(UseClicked()));

	if(m_Light->isVisible() && m_Light->isEnabled())
		menu.addAction(m_Light->icon(), tr("Switch %1 Light %2").arg(itemName).arg(itemState?tr("Off"):tr("On")), this,
				SLOT(LightClicked()));

	//if(m_Use->isVisible() && m_Use->isEnabled())
		//menu.addAction(m_Use->icon(), tr("Use %1").arg(itemName), this, SLOT(UseClicked()));

	if(m_NameSpecies && m_NameSpecies->isVisible() && m_NameSpecies->isEnabled())
		menu.addAction(m_NameSpecies->icon(), tr("Name Species of %1 DNA...").arg(itemName), this, SLOT(NameSpeciesClicked()));

	if(m_SetMessage->isVisible() && m_SetMessage->isEnabled())
		menu.addAction(m_SetMessage->icon(), tr("Set %1 Message...").arg(itemName), this, SLOT(SetMessageClicked()));

	if(m_Settings->isVisible() && m_Settings->isEnabled())
		menu.addAction(m_Settings->icon(), tr("Configure %1...").arg(itemName), this, SLOT(SettingsClicked()));

	if(m_Sound->isVisible() && m_Sound->isEnabled())
		menu.addAction(m_Sound->icon(), tr("Switch %1 Sound %2").arg(itemName).arg(itemState?tr("Off"):tr("On")), this,
				SLOT(SoundClicked()));

	if(m_StarmapLoc->isVisible() && m_StarmapLoc->isEnabled())
		menu.addAction(m_StarmapLoc->icon(), tr("Locate %1 on Star Map").arg(itemName), this, SLOT(StarmapLocClicked()));

	if(m_StorageMedia->isVisible() && m_StorageMedia->isEnabled())
		menu.addAction(m_StorageMedia->icon(), tr("Manage Contents of %1...").arg(itemName), this, SLOT(StorageMediaClicked()));

	if(m_Visor->isVisible() && m_Visor->isEnabled())
		menu.addAction(m_Visor->icon(), tr("%2 %1 Visor").arg(itemName).arg(itemState?tr("Open"):tr("Close")), this,
				SLOT(VisorClicked()));

	if(m_Inscribe->isVisible() && m_Inscribe->isEnabled())
		menu.addAction(m_Inscribe->icon(), tr("Inscribe %1...").arg(itemName), this, SLOT(InscribeClicked()));

	if(m_Inspect->isVisible() && m_Inspect->isEnabled())
		menu.addAction(m_Inspect->icon(), tr("Inspect %1...").arg(itemName), this, SLOT(InspectClicked()));

	if(m_Drop->isVisible() && m_Drop->isEnabled())
		menu.addAction(m_Drop->icon(), tr("Drop %1").arg(itemName), this, SLOT(DropClicked()));

	menu.addSeparator();

	if(m_SlotBody->isVisible() && m_SlotBody->isEnabled())
		menu.addAction(m_SlotBody->icon(), tr("Carry %1 on Body").arg(itemName), this, SLOT(SlotBodyClicked()));

	if(m_SlotBodyWear->isVisible() && m_SlotBodyWear->isEnabled())
		menu.addAction(m_SlotBodyWear->icon(), tr("Wear %1 on Body").arg(itemName), this, SLOT(SlotBodyWearClicked()));

	if(m_SlotDigit->isVisible() && m_SlotDigit->isEnabled())
		menu.addAction(m_SlotDigit->icon(), tr("Wear %1 on Digit").arg(itemName), this, SLOT(SlotDigitClicked()));

	if(m_SlotFace->isVisible() && m_SlotFace->isEnabled())
		menu.addAction(m_SlotFace->icon(), tr("Wear %1 on Face").arg(itemName), this, SLOT(SlotFaceClicked()));

	if(m_SlotHand->isVisible() && m_SlotHand->isEnabled())
		menu.addAction(m_SlotHand->icon(), tr("Carry %1 in Hand").arg(itemName), this, SLOT(SlotHandClicked()));

	if(m_SlotHead->isVisible() && m_SlotHead->isEnabled())
		menu.addAction(m_SlotHead->icon(), tr("Wear %1 on Head").arg(itemName), this, SLOT(SlotHeadClicked()));

	if(m_SlotLimb->isVisible() && m_SlotLimb->isEnabled())
		menu.addAction(m_SlotLimb->icon(), tr("Wear %1 on Limb").arg(itemName), this, SLOT(SlotLimbClicked()));

	if(m_SlotNeck->isVisible() && m_SlotNeck->isEnabled())
		menu.addAction(m_SlotNeck->icon(), tr("Wear %1 on Neck").arg(itemName), this, SLOT(SlotNeckClicked()));

	if(m_SlotWaist->isVisible() && m_SlotWaist->isEnabled())
		menu.addAction(m_SlotWaist->icon(), tr("Carry %1 at Waist").arg(itemName), this, SLOT(SlotWaistClicked()));

	// Show the menu.
	QAction* action = menu.exec(QCursor::pos());
	if(!action)
		return;
}

void AuGearWidget::GearTreeDoubleClicked()
{
	if(m_Gear.IsPosValid(m_SelPos))
		emit UseGearItem(m_SelPos);
}

void AuGearWidget::GearTreeDrop()
{
	DropClicked();

	m_PressPos = AuGearPos();
	EnableControls();
}

void AuGearWidget::GearTreeMove()
{
	if(m_Gear.IsPosValid(m_PressPos) && m_Gear.IsPosValid(m_SelPos) && m_PressPos != m_SelPos)
	{
		/*
		qDebug() << "AuGearWidget::GearTreeReleased moving item" << m_PressPos.Index() << m_PressPos.SubIndex()
			<< "to item" << m_SelPos.Index() << m_SelPos.SubIndex();
		qDebug() << "AuGearWidget::GearTreeReleased" << m_PressPos.Index() << m_PressPos.SubIndex();
		const AuItem* srcItem = m_Gear.ItemAt(m_PressPos);
		const AuItem* destItem = m_Gear.ItemAt(m_SelPos);
		if(srcItem && destItem)
			qDebug() << "AuGearWidget::GearTreeReleased srcItem" << srcItem->ListName() << "destItem" << destItem->ListName();
		else
		{
			qint8 gearIndex = 0;
			for(AuGear::const_iterator gi = m_Gear.cbegin(); gi != m_Gear.cend(); gi++, gearIndex++)
				qDebug() << gearIndex << 255 << (*gi).Item().ListName();
		}
		*/
		emit MoveGearItemToItem(m_PressPos, m_SelPos);
	}

	m_PressPos = AuGearPos();
	EnableControls();
}

void AuGearWidget::SlotBodyClicked()
{
	if(m_Gear.IsPosValid(m_SelPos))
		MoveGearItemToSlot(m_SelPos, AuGearSlot::Body);
}

void AuGearWidget::SlotBodyWearClicked()
{
	if(m_Gear.IsPosValid(m_SelPos))
		MoveGearItemToSlot(m_SelPos, AuGearSlot::BodyWear);
}

void AuGearWidget::SlotDigitClicked()
{
	if(m_Gear.IsPosValid(m_SelPos))
		MoveGearItemToSlot(m_SelPos, AuGearSlot::Digit);
}

void AuGearWidget::SlotFaceClicked()
{
	if(m_Gear.IsPosValid(m_SelPos))
		MoveGearItemToSlot(m_SelPos, AuGearSlot::Face);
}

void AuGearWidget::SlotHandClicked()
{
	if(m_Gear.IsPosValid(m_SelPos))
		MoveGearItemToSlot(m_SelPos, AuGearSlot::Hand);
}

void AuGearWidget::SlotHeadClicked()
{
	if(m_Gear.IsPosValid(m_SelPos))
		MoveGearItemToSlot(m_SelPos, AuGearSlot::Head);
}

void AuGearWidget::SlotLimbClicked()
{
	if(m_Gear.IsPosValid(m_SelPos))
		MoveGearItemToSlot(m_SelPos, AuGearSlot::Limb);
}

void AuGearWidget::SlotNeckClicked()
{
	if(m_Gear.IsPosValid(m_SelPos))
		MoveGearItemToSlot(m_SelPos, AuGearSlot::Neck);
}

void AuGearWidget::SlotWaistClicked()
{
	if(m_Gear.IsPosValid(m_SelPos))
		MoveGearItemToSlot(m_SelPos, AuGearSlot::Waist);
}

void AuGearWidget::ArmClicked()
{
	if(m_Gear.IsPosValid(m_SelPos))
		emit UseGearItem(m_SelPos);
}

void AuGearWidget::CollectBountyClicked()
{
	if(m_Gear.IsPosValid(m_SelPos))
		emit RedeemGearItem(m_SelPos);
}

void AuGearWidget::DecontaminateClicked()
{
	if(m_Gear.IsPosValid(m_SelPos))
		emit UseGearItem(m_SelPos);
}

void AuGearWidget::DrinkClicked()
{
	if(m_Gear.IsPosValid(m_SelPos))
		emit UseGearItem(m_SelPos);
}

void AuGearWidget::DropClicked()
{
	if(m_Gear.IsPosValid(m_SelPos))
		emit DropGearItem(m_SelPos);
}

void AuGearWidget::EatClicked()
{
	if(m_Gear.IsPosValid(m_SelPos))
		emit UseGearItem(m_SelPos);
}

void AuGearWidget::ExtinguishClicked()
{
	if(m_Gear.IsPosValid(m_SelPos))
		emit UseGearItem(m_SelPos);
}

void AuGearWidget::FuseClicked()
{
	if(m_Gear.IsPosValid(m_SelPos))
		emit UseGearItem(m_SelPos);
}

void AuGearWidget::HandCraftClicked()
{
	emit ToggleHandCraftUi();
}

void AuGearWidget::InscribeClicked()
{
	const AuItem* gearItem = m_Gear.ItemAt(m_SelPos);
	if(!gearItem)
		return;

	const DgData& gd = g_Au.LockDataForRead();
	bool isProgrammer = gd.ClientPlayerIsProgrammer();
	g_Au.UnlockData();

	bool ok;
	QString inscription;
	if(isProgrammer)
		inscription = QInputDialog::getText(this, tr("Inscribe Item"),
				tr("<html>Enter an inscription for the %1. "
					"The inscription is permanent.<br><br>"
					"The inscription must be no more than 100 characters <br>"
					"and must not contain non-printable characters<br>"
					"or certain reserved characters such as &lt; &gt; &amp;&amp; and |.<br>"
					"<b>Programmer Power</b> Text is not checked for length or invalid characters; be careful."
					"</html>")
				.arg(gearItem->SayName()), QLineEdit::Normal, "", &ok).trimmed();
	else
		inscription = QInputDialog::getText(this, tr("Inscribe Item"),
				tr("<html>Enter an inscription for the %1. "
					"The inscription is permanent.<br><br>"
					"The inscription must be no more than 100 characters<br>"
					"and must not contain non-printable characters<br>"
					"or certain reserved characters such as &lt; &gt; &amp;&amp; and |.</html>")
				.arg(gearItem->SayName()), QLineEdit::Normal, "", &ok).trimmed();
	if(!ok)
		return;

	if(!inscription.isEmpty() && (IsTextValid(inscription, 100) || isProgrammer))
		emit InscribeGearItem(m_SelPos, inscription);
}

void AuGearWidget::InspectClicked()
{
	const AuItem* gearItem = m_Gear.ItemAt(m_SelPos);
	if(!gearItem)
		return;

	AuItemDesc* dlg = new AuItemDesc(*gearItem, this);
	connect(dlg, SIGNAL(finished(int)), dlg, SLOT(deleteLater()));
	dlg->show();
}

void AuGearWidget::LightClicked()
{
	if(m_Gear.IsPosValid(m_SelPos))
		emit UseGearItem(m_SelPos);
}

void AuGearWidget::NameSpeciesClicked()
{
	const AuItem* gearItem = m_Gear.ItemAt(m_SelPos);
	if(!gearItem)
		return;

	const AuSpecies& species = gearItem->DNASpecies();
	if(species.IsNull())
		return;
	AuView* view = auApp->Frame()->View();
	AuId sceneAtomId = view->SceneAtomId();
	if(sceneAtomId.IsNull())
		return;

	bool isWildAnimal = !gearItem->DNAIsSentient();

	// Scene atom is an admin or the DNA is from an animal or the DNA matches the scene atom. So, the
	// scene atom is allowed to name/rename the DNA. The NameDNA function checks for authority to rename
	// when the species dna is already named.
	AuCommChannel::NameSpecies(species, isWildAnimal, sceneAtomId, this);
}

void AuGearWidget::SetMessageClicked()
{
	const AuItem* gearItem = m_Gear.ItemAt(m_SelPos);
	if(!gearItem)
		return;

	bool ok;
	QString message = QInputDialog::getText(this, tr("Set Message"),
			tr("<html>Enter a message for the %1. "
				"The message can be changed at any time.<br><br>"
				"The message must be no more than 100 characters<br>"
				"and must not contain non-printable characters<br>"
				"or certain reserved characters such as &lt; &gt; &amp;&amp; and |.</html>")
			.arg(gearItem->SayName()), QLineEdit::Normal, gearItem->Message(), &ok).trimmed();
	if(!ok)
		return;

	if(IsTextValid(message, 100))
		emit SetMessage(m_SelPos, message);
}

void AuGearWidget::SettingsClicked()
{
	const AuItem* gearItem = m_Gear.ItemAt(m_SelPos);
	if(!gearItem)
		return;

	if(DbCommodity::IsPicture(gearItem->CommodityId()))
	{
		const AuItemPicture* pic = dynamic_cast<const AuItemPicture*>(gearItem);
		if(pic)
		{
			QString picURL;
			if(!pic->IsURLHidden())
				picURL = pic->PictureURL();
			AuPictureFrameSettings dlg(picURL, pic->IsURLHidden(), pic->RotatePicture(), auApp->Frame());
			int result = auApp->Frame()->ExecDialog(dlg);
			if(result == QDialog::Accepted)
			{
				QString url = dlg.PictureURL();
				bool rotate = dlg.RotatePicture();
				bool hideURL = dlg.HideURL();

				emit SetPictureFrame(m_SelPos, url, hideURL, rotate);
			}
		}
	}
	else if(DbCommodity::IsFlowerPot(gearItem->CommodityId()))
	{
		const AuItemFloraDNA* pot = dynamic_cast<const AuItemFloraDNA*>(gearItem);
		if(pot)
		{
			//qDebug() << "AuGearWidget::SettingsClicked pot" << pot->ListName() << "size" << pot->PottedSize();
			AuFlowerPotSettings dlg(pot->PottedSize(), auApp->Frame());
			int result = auApp->Frame()->ExecDialog(dlg);
			if(result == QDialog::Accepted)
			{
				quint8 plantSize = dlg.PlantSize();
				//qDebug() << "AuGearWidget::SettingsClicked pot changed size" << plantSize << "selPos" << m_SelPos;

				emit SetFlowerPot(m_SelPos, plantSize);
			}
		}
	}
}

void AuGearWidget::SoundClicked()
{
	if(m_Gear.IsPosValid(m_SelPos))
		emit UseGearItem(m_SelPos);
}

void AuGearWidget::StarmapLocClicked()
{
	const AuItem* gearItem = m_Gear.ItemAt(m_SelPos);
	if(!gearItem)
		return;

	const AuItemGalaxyLoc* gloc = dynamic_cast<const AuItemGalaxyLoc*>(gearItem);
	if(!gloc)
		return;

	emit ShowGalaxyLocOnStarMap(gloc->GalaxyName(), gloc->GalaxyLoc());
}

void AuGearWidget::UseClicked()
{
	if(m_Gear.IsPosValid(m_SelPos))
		emit UseGearItem(m_SelPos);
}

void AuGearWidget::VisorClicked()
{
	if(m_Gear.IsPosValid(m_SelPos))
		emit UseGearItem(m_SelPos);
}

void AuGearWidget::ReplaceSpecies(DzSpecies species)
{
	//qDebug() << "AuGearWidget::ReplaceSpecies" << species.m_DNA.SpeciesDescription();
	for(AuGear::const_iterator gi = m_Gear.cbegin(); gi != m_Gear.cend(); gi++)
		if((*gi).Item().DNASpecies() == species.Species())
		{
			m_Gear.Clear();
			m_GearModel.clear();
			//qDebug() << "AuGearWidget::ReplaceSpecies gear cleared";
			return;
		}
}

void AuGearWidget::StorageMediaClicked()
{
	if(!m_Gear.IsPosValid(m_SelPos))
		return;
	const AuItem* selItem = m_Gear.ItemAt(m_SelPos);
	if(!selItem)
		return;
	AuStorageMedia dlg(*selItem, m_SelPos, this);
	m_StorageMediaDlg = &dlg;
	auApp->Frame()->ExecDialog(dlg);
	m_StorageMediaDlg = 0;
}
