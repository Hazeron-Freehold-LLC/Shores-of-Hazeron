//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augameclonepokui.cpp,v 1.13 2022/02/19 17:39:33 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augameclonepokui.h"
//
#include "augameclonepok.h"
#include "auglobal.h"
#include "aupushbutton.h"
#include "daperson.h"
#include <QComboBox>
#include <QHeaderView>
#include <QLabel>
#include <QLCDNumber>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSlider>
#include <QTextBrowser>
#include <QVBoxLayout>

AuGameClonepokGearTree::AuGameClonepokGearTree(QWidget* parent) : QTreeView(parent)
{
	m_ButtonsPressed = 0;

	//setAlternatingRowColors(true);
	setAllColumnsShowFocus(true);
	setSelectionMode(QAbstractItemView::NoSelection);
	setSortingEnabled(true);
	setIconSize(QSize(72,72));
	//setTextElideMode(Qt::ElideNone);
	setWordWrap(true);

	//header()->setStretchLastSection(false);
	//header()->hide();
}

void AuGameClonepokGearTree::mouseMoveEvent(QMouseEvent* e)
{
	QTreeView::mouseMoveEvent(e);
	//QPoint vec = e->pos() - m_PressPos;
	//if(vec.manhattanLength() > 3)
		//setCursor(Qt::ClosedHandCursor);
}

void AuGameClonepokGearTree::mouseDoubleClickEvent(QMouseEvent* e)
{
	QTreeView::mouseDoubleClickEvent(e);
	emit MouseDoubleClicked();
}

void AuGameClonepokGearTree::mousePressEvent(QMouseEvent* e)
{
	QTreeView::mousePressEvent(e);
	m_ButtonsPressed = e->buttons();
	m_PressPos = e->pos();
}

void AuGameClonepokGearTree::mouseReleaseEvent(QMouseEvent* e)
{
	QTreeView::mouseReleaseEvent(e);
	if(m_ButtonsPressed & Qt::RightButton)
		emit MouseRightClicked();
	else if(rect().contains(e->pos()))
		emit MouseReleasedInside();
	else
		emit MouseReleasedOutside();
	//setCursor(Qt::ArrowCursor);
}

void AuGameClonepokGearTree::ConfigureSort()
{
	sortByColumn(0, Qt::AscendingOrder);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGameClonepokGearModel::AuGameClonepokGearModel()
{
}

void AuGameClonepokGearModel::AddGearPos(const AuGearPos& gpos, const AuGear& gear)
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

void AuGameClonepokGearModel::AddGearPos(const AuGearPos& gpos, const AuGear& gear, QList<AuItemContentId>& path,
		QStandardItem* parentItem, const QModelIndex& parentIdx)
{
	if(path.size() <= 1)
	{
		// Add a new item.
		const AuItem* item = gear.ItemAt(gpos);
		if(!item)
			return;

		QString itemName(item->ListName());
		QSize psz(item->PackSize());
		int icowid = 12 + psz.width() * 3;
		int icoht = 12 + psz.height() * 3;	// max height of a pack item is 20, therefore max icoht is 72.
		QIcon pico(QPixmap(item->BitmapName()).scaled(icowid, icoht));

		QList<QStandardItem*> rowItems;

		// Gear column.
		// Item icon and name, with informative tooltip.
		QStandardItem* col0Item = new QStandardItem(pico, itemName);
		col0Item->setToolTip(item->HtmlToolTip());
		col0Item->setData(QVariant::fromValue(gpos), Qt::UserRole+1);
		col0Item->setData(item->Pack(), Qt::UserRole+2);
		rowItems.append(col0Item);

		// Slot column.
		if(gpos.IsTopLevel())
		{
			// Set data used for sorting.
			col0Item->setData(QString("%1%2").arg(AuGearSlot::SlotTypeSortName(gpos.Slot())).arg(gpos.Which()), Qt::UserRole);

			// Top level items have an icon representing the slot type.
			const int ticosz = 20;
			QIcon tico(QPixmap(AuGearSlot::SlotTypeIcon(gpos.Slot())).scaled(ticosz, ticosz));
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
		}
		else
		{
			// Set data used for sorting.
			col0Item->setData(itemName, Qt::UserRole);

			// Dummy item in the gear slot type column.
			QStandardItem* col1Item = new QStandardItem();
			rowItems.append(col1Item);
		}

		// Stack check box, indicating items to be included in the stack.
		if(item->Content() && !item->Content()->empty())
		{
			// Non-empty containers cannot be stacked.
			QStandardItem* colItem = new QStandardItem();
			rowItems.append(colItem);
		}
		else
		{
			QStandardItem* colItem = new QStandardItem(QIcon(":data/checkbox10x10.png"), "");
			rowItems.append(colItem);
		}

		// Type
		{
			QStandardItem* colItem = new QStandardItem();
			rowItems.append(colItem);
		}

		// Element
		{
			QStandardItem* colItem = new QStandardItem();
			rowItems.append(colItem);
		}

		// Notes
		{
			QStandardItem* colItem = new QStandardItem();
			rowItems.append(colItem);
		}

		// HP
		{
			QStandardItem* colItem = new QStandardItem();
			rowItems.append(colItem);
		}

		// Move cost
		{
			QStandardItem* colItem = new QStandardItem();
			rowItems.append(colItem);
		}

		// Attack cost
		{
			QStandardItem* colItem = new QStandardItem();
			rowItems.append(colItem);
		}

		// Weakness
		{
			QStandardItem* colItem = new QStandardItem();
			rowItems.append(colItem);
		}

		// Resistance
		{
			QStandardItem* colItem = new QStandardItem();
			rowItems.append(colItem);
		}

		if(gpos.IsTopLevel())
		{
			// Add the new row at the top-level.
			insertRow(rowCount(), rowItems);

			if(rowCount() == 1)
			{
				int col=0;
				setHeaderData(col++, Qt::Horizontal, tr("Gear"));				// Gear icon and name
				setHeaderData(col++, Qt::Horizontal, tr("Slot"));				// Gear slot for top level items
				setHeaderData(col++, Qt::Horizontal, tr("Stack"));				// Check box
				setHeaderData(col++, Qt::Horizontal, tr("Type"));				// Tile Type
				setHeaderData(col++, Qt::Horizontal, tr("Element"));			// Elemental Type(s)
				setHeaderData(col++, Qt::Horizontal, tr("Notes"));				// Text
				setHeaderData(col++, Qt::Horizontal, tr("HP"));					// 0-250
				setHeaderData(col++, Qt::Horizontal, tr("Move"));				// Element type, 0-5
				setHeaderData(col++, Qt::Horizontal, tr("Attack"));				// Elemental Types, 0-5 and Damage HP
				setHeaderData(col++, Qt::Horizontal, tr("Weakness"));			// Elemental types, 0-5 of each.
				setHeaderData(col++, Qt::Horizontal, tr("Resistance"));			// Elemental Types, 0-5 of each.
			}
		}
		else
		{
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

bool AuGameClonepokGearModel::RemoveGearPos(const AuGearPos& gpos, const QModelIndex& parentIdx)
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

void AuGameClonepokGearModel::Sort()
{
	setSortRole(Qt::UserRole);
	sort(0);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGameClonepokSetup::AuGameClonepokSetup(const DaPerson& person, const AuGameClonepok& game, QWidget* parent) : AuDialog(parent)
{
	Q_UNUSED(person);
	// Restore my previous settings.
	qint32 ante = g_Au.Settings().readNumEntry("Game Table/Clonepok/Treasure", 0);
	if(ante < 0)
		ante = 0;
	else if(ante > 100000000)
		ante = 100000000;

	setAuDlgIcon(QIcon(":data/GameClonepokIcon.png"));
	setAuDlgTitle(tr("<b>Clonēpok Setup</b>"));

	m_MinCash = new QLabel(this);
	m_MinCash->setWordWrap(true);

	m_Ok = new QPushButton(tr("Start Game"), this);
	m_Ok->setToolTip(tr("<html>Starts the game with the settings shown."
				"</html>"));
	m_Ok->setAutoDefault(true);
	m_Ok->setDefault(true);
	connect(m_Ok, SIGNAL(clicked()), SLOT(StartGame()));

	// Add a box to set the ante.
	QLabel* lbante = new QLabel(QObject::tr("Treasure Ante %1").arg(g_Au.CurrencySymbol()), this);

	m_Ante = new QLineEdit(this);
	m_Ante->setText(QVariant(ante).toString());
	m_Ante->setToolTip(tr("<html>Money each player must contribute to the treasure pile to start the game. "
				"Treasure is optional and it can be zero. "
				"Money is paid into the treasure pile when the game starts. "
				"Winner takes all the treasure.</html>"));
	connect(m_Ante, SIGNAL(textChanged(const QString&)), SLOT(SettingsChanged()));

	// Add a box to set the stack size.
	QLabel* lbstack = new QLabel(QObject::tr("Stack Size"), this);

	m_Stack = new QComboBox(this);
	m_Stack->setToolTip(tr("<html>Number of playable tiles each player must possess to start the game. "
				"Tiles correspond to gear carried by each player. "
				"One tile is created for each gear item. "
				"Tiles are not created from non-empty containers but their contents can become tiles.</html>"));
	m_Stack->addItem(tr("60 tiles"), 60);
	m_Stack->addItem(tr("50 tiles"), 50);
	m_Stack->addItem(tr("40 tiles"), 40);
	m_Stack->addItem(tr("30 tiles"), 30);
	m_Stack->addItem(tr("20 tiles"), 20);
	m_Stack->addItem(tr("10 tiles"), 10);
	m_Stack->setCurrentText(tr("60 tiles"));
	connect(m_Stack, SIGNAL(currentIndexChanged(int)), SLOT(SettingsChanged()));

	/*
	{
		// List the none option.
		m_SampleKits->addItem(tr("Winner Gets No DNA Sample"), QVariant::fromValue(AuGearPos()));

		// List sample kits held in the player's gear. This player is the dealer.
		AuGearPosList kitpos;
		const AuGear* gear = person.LockGearForRead();
		gear->ListItems(CID_DNASampleKit, kitpos);
		for(AuGearPosList::const_iterator gi = kitpos.cbegin(); gi != kitpos.cend(); gi++)
		{
			const AuItem* item = gear->ItemAt(*gi);
			if(item)
				m_SampleKits->addItem(QIcon(item->BitmapName()), tr("%1, Gear").arg(item->ListDesc()), QVariant::fromValue(*gi));
		}
		person.UnlockGear();

		if(person.IsInSpacecraft())
		{
			// List sample kits in the hold of the spacecraft.
			const DaVessel* ship = person.FindParentVesselInEnv();
			if(ship)
			{
				const AuItemHash& cr = ship->LockCargoForRead();
				for(AuItemHash::const_iterator it = cr.begin(); it != cr.end(); it++)
				{
					if((*it).second->CommodityId() == CID_DNASampleKit)
						m_SampleKits->addItem(QIcon((*it).second->BitmapName()), tr("%1, Cargo").arg((*it).second->ListDesc()),
								QVariant::fromValue((*it).second->Pack()));
				}
				ship->UnlockCargo();
			}
		}
	}
	*/

	// List the players in the game.
	// A kick button is added for everyone but the dealer.
	QGridLayout* layPlayers = new QGridLayout;
	int row=0;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
	{
		m_PlayerIds[player] = game.PlayerId(player);
		if(!m_PlayerIds[player].IsNull())
		{
			QLabel* lbhead = new QLabel(this);
			lbhead->setPixmap(QPixmap(":data/Passenger.png"));
			layPlayers->addWidget(lbhead, row,0);

			m_PlayerNames[player] = new QLabel(game.PlayerName(player), this);
			layPlayers->addWidget(m_PlayerNames[player], row,1);

			if(player != 0)
			{
				// Allow dealer to kick any of the players who were automatically enrolled because they are sitting at the table.
				m_PlayerKick[player] = new AuPushButtonValue(player, this);
				m_PlayerKick[player]->setIcon(QIcon(":data/CommKick.png"));
				m_PlayerKick[player]->setToolTip(tr("Kicks the player from the game."));
				connect(m_PlayerKick[player], SIGNAL(ClickedValue(int)), SLOT(KickPlayer(int)));
				layPlayers->addWidget(m_PlayerKick[player], row,2);
			}
			else
				m_PlayerKick[player] = 0;		// No kick button for the dealer.
			layPlayers->setColumnStretch(3,1);	// Scrunch them over to the left.
			row++;
		}
		else
		{
			// No player in that slot.
			m_PlayerNames[player] = 0;
			m_PlayerKick[player] = 0;
		}
	}

	// Create the layout of the settings dialog.
	QGridLayout* lay = new QGridLayout;
	row=0;
	lay->addLayout(layPlayers, row,0, 1,2);
	row++;

	// Add game controls.
	lay->addWidget(lbstack, row,0);
	lay->addWidget(m_Stack, row,1);
	row++;

	lay->addWidget(lbante, row,0);
	lay->addWidget(m_Ante, row,1);
	row++;

	lay->addWidget(m_MinCash, row,0, 1,2);
	row++;

	lay->setRowStretch(row, 1);			// Make a stretchy row to crunch the names to the top when there's extra space.
	row++;

	// Add the Start Game button.
	lay->addWidget(m_Ok, row,0, 1,2);

	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);

	SettingsChanged();
}

AuGameClonepokSetup::~AuGameClonepokSetup()
{
}

void AuGameClonepokSetup::SettingsChanged()
{
	bool okToStart = true;
	int players = 0;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
		if(!m_PlayerIds[player].IsNull())
			players++;
	m_Ok->setEnabled(players);
	if(!players)
		okToStart = false;		// Can't start a game with no players.

	quint32 ante = QVariant(m_Ante->text()).toUInt();
	if(ante > 100000000)
		okToStart = false;	// Treasure limited to 100M.

	quint8 stack = m_Stack->currentData().toUInt();
	if(stack < 10 || stack > 60)
		okToStart = false;

	quint8 basicClones = stack / 10;

	quint8 treasureTiles = basicClones;

	m_Ok->setEnabled(okToStart);

	// Players need enough money to ante up.
	quint32 moneyToStart = ante;
	if(moneyToStart)
		m_MinCash->setText(tr("<html><p>Each player must possess %L1 playable tiles to start. "
					"At least %L2 of the tiles must be Basic clones. "
					"%L3 tiles will be added to the treasure pile.</p>"
					"<p>Each player must have %L4%5 to start. "
					"Money is paid into the treasure pile when the game starts.</p>"
					"<p><b>Players with less money or tiles will be booted when the game starts.</b></p>"
					"</html>")
				.arg(stack)
				.arg(basicClones)
				.arg(treasureTiles)
				.arg(moneyToStart)
				.arg(g_Au.CurrencySymbol()));
	else
		m_MinCash->setText(tr("<html><p>Each player must possess %L1 playable tiles to start. "
					"At least %L2 of the tiles must be Basic clones. "
					"%L3 tiles will be added to the treasure pile.</p>"
					"<p><b>Players with less tiles will be booted when the game starts.</b></p>"
					"</html>")
				.arg(stack)
				.arg(basicClones)
				.arg(treasureTiles));
}

void AuGameClonepokSetup::StartGame()
{
	// Collect and validate my settings from the widgets.
	int players = 0;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
		if(!m_PlayerIds[player].IsNull())
			players++;
	if(!players)
		return;		// Can't start a game with no players.

	quint32 ante = QVariant(m_Ante->text()).toUInt();
	if(ante > 100000000)
		ante = 100000000;	// Treasure limited to 10M.

	quint8 stack = m_Stack->currentData().toUInt();
	quint8 basicClones = stack / 10;
	quint8 treasureTiles = basicClones;

	// Pack my settings into a blob the game will recognize at the other side.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << ante;
	out << basicClones;
	out << stack;
	out << treasureTiles;

	for(int player=0; player<MAX_GAME_PLAYERS; player++)
		out << m_PlayerIds[player];

	// Emit a signal that ausceneclient can put into a packet to the server with my game settings.
	emit Accepted(AuGameClonepok::GameType(), ba);
	accept();

	// Save these settings for next time.
	g_Au.Settings().writeEntry("Game Table/Clonepok/Treasure", ante);
}

void AuGameClonepokSetup::KickPlayer(int playerIndex)
{
	Q_ASSERT(playerIndex >= 1 && playerIndex < MAX_GAME_PLAYERS);
	m_PlayerIds[playerIndex] = AuId(0);
	m_PlayerNames[playerIndex]->hide();
	m_PlayerKick[playerIndex]->hide();
}

void AuGameClonepokSetup::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuGameClonepokSetup::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/GameClonepokSetup/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/GameClonepokSetup/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/GameClonepokSetup/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/GameClonepokSetup/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuGameClonepokSetup::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/GameClonepokSetup/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/GameClonepokSetup/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/GameClonepokSetup/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/GameClonepokSetup/SizeY",sz.height());
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGameClonepokMap::AuGameClonepokMap(const DaPerson& person, const AuGameClonepok& game, QWidget* parent) : QWidget(parent),
	m_TilesInPlay(game.TilesInPlay())
{
	m_Map = QImage(":data/GameClonepokBoard.png");

	setMinimumSize(m_Map.size());
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
}

AuGameClonepokMap::~AuGameClonepokMap()
{
}

QSize AuGameClonepokMap::sizeHint() const
{
	return m_Map.size();
}

void AuGameClonepokMap::paintEvent(QPaintEvent* pe)
{
	if(rect().width() == 0 || rect().height() == 0)
	{
		QWidget::paintEvent(pe);
		return;
	}

	QPainter p(this);
	p.drawImage(rect(), m_Map);
	p.end();

	/*
	// Scaling factors.
	float sx = rect().width() / m_Map.width();
	float sy = rect().height() / m_Map.height();

	// Draw units.
	QImage unit0(":data/BlipAnimalEmpire.png");
	QImage unit1(":data/BlipAnimalEnemy.png");
	QImage unit2(":data/BlipAnimalFriend.png");
	QImage unit3(":data/BlipAnimalNeutral.png");
	for(AuGameClonepokUnitList::const_iterator ui = m_Units.cbegin(); ui != m_Units.cend(); ui++)
	{
		float px = (float)(*ui).m_MapLoc.x() * sx - unit0.width()/2;
		float py = (float)(*ui).m_MapLoc.y() * sy - unit0.height()/2;
		p.drawImage(px,py, unit0);
	}
	*/
	
	QWidget::paintEvent(pe);
}

void AuGameClonepokMap::mouseReleaseEvent(QMouseEvent* e)
{
	QWidget::mouseReleaseEvent(e);

	//emit HordeMoved(quint8 toSpace, QSet<quint8> selectedUnits);
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGameClonepokTurn::AuGameClonepokTurn(const DaPerson& person, const AuGameClonepok& game, QWidget* parent) : AuDialog(parent)
{
	m_TurnState = game.TurnState();

	setObjectName(game.TurnUIName());
	setAuDlgIcon(QIcon(":data/GameClonepokIcon.png"));

	if(m_TurnState == AuGameClonepok::StackBuilding)
	{
		setAuDlgTitle(tr("<b>Clonēpok Stack Building</b>"));

		// Tell them what to do.
		QLabel* lbturn = new QLabel(QObject::tr("<html><p>Create a stack of %1 tiles from your gear items.</p>"
					"<p>%2 treasure tiles will be taken at random from the stack at the end of the landfall phase. "
					"Corresponding gear is taken from your gear. "
					"<b>You are kicked from the game if you do not possess the items at that time.</b></p>"
					"<p>The winner of each treasure tile receives the associated gear item. "
					"It is placed at the winner's feet if they have no room to carry it.</p>"
					"</html>")
				.arg(game.StackSize())
				.arg(game.TreasureTileAnte()), this);
		lbturn->setWordWrap(true);

		// List of items in person's inventory.
		m_GearTree = new AuGameClonepokGearTree(this);
		m_GearTree->setModel(&m_GearModel);
		m_GearTree->setToolTip(tr("<html><b>Gear</b><br>"
					"Gear carried or worn by you.<br>"
					"<i>Tips</i><br>"
					"%1 Select one or more items.<br>"
					"%1 Right-click items to make tiles from them.<br>"
					"%1 Double-click items to make a tiles from them.<br>"
					"%1 A tile cannot be created from a container that is not empty but items inside the container can be used."
					"</html>").arg(g_Au.MiddleDotSymbol()));
		connect(m_GearTree, SIGNAL(MouseRightClicked()), SLOT(GearTreeRightClicked()));
		connect(m_GearTree, SIGNAL(MouseDoubleClicked()), SLOT(GearTreeDoubleClicked()));
		connect(m_GearTree->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
				SLOT(GearSelectionChanged(const QItemSelection&, const QItemSelection&)));

		// List of tiles.
		//DHH

		// Quit button.
		QPushButton* quit = new QPushButton(tr("Forfeit"), this);
		quit->setToolTip(tr("<html>Quits the game. "
					"Money and treasure remains in the treasure pile.</html>"));
		connect(quit, SIGNAL(clicked()), SLOT(QuitGame()));

		// Build the layout.
		QGridLayout* lay = new QGridLayout;
		int row=0;
		lay->addWidget(lbturn, row,0, 1,2);
		row++;
		lay->addWidget(m_GearTree, row,0, 1,2);
		row++;
		lay->addWidget(quit, row++,0, 1,1);
		lay->setMargin(2);
		lay->setSpacing(2);
		setAuDlgLayout(lay);

		// Dialog is complete. Fill the gear tree.
		{
			const AuGear* gear = person.LockGearForRead();
			if(gear)
			{
				AuGearPosList newGearPos;
				gear->ListAll(newGearPos);

				//AuGear oldGear(m_Gear);
				m_Gear = *gear;

				// Sync the model.
				for(AuGearPosList::const_iterator ni = newGearPos.cbegin(); ni != newGearPos.cend(); ni++)
					m_GearModel.AddGearPos(*ni, m_Gear);	// Create a new model item.

				if(m_GearModel.rowCount())
				{
					m_GearTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
					m_GearTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
					m_GearTree->ConfigureSort();
					m_GearModel.Sort();
				}
			}
			person.UnlockGear();
		}
	}
	else if(m_TurnState == AuGameClonepok::Landfall)
	{
		setAuDlgTitle(tr("<b>Clonēpok Landfall</b>"));

		// Tell them what to do.
		QLabel* lbturn = new QLabel(QObject::tr("<html><p>Choose a Basic clone to make landfall.</p>"
					"</html>"), this);
		lbturn->setWordWrap(true);

		// List of Basic clone tiles in the person's stack.
		//DHH

		// Quit button.
		QPushButton* quit = new QPushButton(tr("Forfeit"), this);
		quit->setToolTip(tr("<html>Quits the game. "
					"Money and treasure remains in the treasure pile.</html>"));
		connect(quit, SIGNAL(clicked()), SLOT(QuitGame()));

		// Build the layout.
		QGridLayout* lay = new QGridLayout;
		int row=0;
		lay->addWidget(lbturn, row,0, 1,2);
		row++;
		lay->addWidget(quit, row++,0, 1,1);
		lay->setMargin(2);
		lay->setSpacing(2);
		setAuDlgLayout(lay);
	}
	else
	{
		setAuDlgTitle(tr("<b>Clonēpok Turn</b>"));

		// Tell them what to do.
		QLabel* lbturn = new QLabel(QObject::tr("<html><p>Several actions are available to you.<br>"
					"%1 Choose a Basic clone to make landfall.</p>"
					"</html>").arg(g_Au.MiddleDotSymbol()), this);
		lbturn->setWordWrap(true);

		// Game board.
		m_Map = new AuGameClonepokMap(person, game, this);
		m_Map->setToolTip(tr("<html><b>A.</b> Form a horde by clicking on one or more creatures. "
					"They do not have to be in the same map space.<br>"
					"<b>B.</b> Right-click a map space to move the horde. "
					"Creatures can only move one space. "
					"Aquatic creatures can only move into coastal terrain.<br>"
					"Moving a horde ends your turn.</html>"));
		connect(m_Map, SIGNAL(HordeMoved(quint8, QSet<quint8>)), SLOT(MoveHorde(quint8, QSet<quint8>)));

		// Quit button.
		QPushButton* quit = new QPushButton(tr("Forfeit"), this);
		quit->setToolTip(tr("<html>Quits the game. "
					"Money and treasure remains in the treasure pile.</html>"));
		connect(quit, SIGNAL(clicked()), SLOT(QuitGame()));

		// Build the layout.
		QGridLayout* lay = new QGridLayout;
		int row=0;
		lay->addWidget(lbturn, row,0, 1,2);
		row++;
		lay->addWidget(m_Map, row,0, 1,2);
		row++;
		lay->addWidget(quit, row++,0, 1,1);
		lay->setMargin(2);
		lay->setSpacing(2);
		setAuDlgLayout(lay);
	}

	/*
	// List all the DNA samples in the person's possession.
	m_DNASamples = new QComboBox(this);
	m_DNASamples->setToolTip(tr("<html>Creature DNA to clone. "
				"Cloning a DNA sample uses it, for item wear purposes; it could wear out. "
				"Item wear is affected by its quality. "
				"</html>"));

	AuGearPosList samplepos;
	const AuGear* gear = person.LockGearForRead();
	gear->ListItems(CID_DNASample, samplepos);
	for(AuGearPosList::const_iterator gi = samplepos.cbegin(); gi != samplepos.cend(); gi++)
	{
		const AuItem* item = gear->ItemAt(*gi);
		if(item && item->DNAIsViable())
			m_DNASamples->addItem(QIcon(item->BitmapName()), item->ListName(), QVariant::fromValue(item->DNA()));
	}
	person.UnlockGear();

	// Button to create a clone.
	m_CreateClone = new QPushButton(tr("Create Clone"), this);
	m_CreateClone->setToolTip(tr("<html>Creates a clone of the DNA sample. "
				"The cost will be deducted from your personal account. "
				"The clone appears on the coastal terrain space in front of your seat.<br>"
				"Creating a clone ends your turn.</html>"));
	m_CreateClone->setEnabled(cloneCount > 0 && cloneHp > 0 && m_DNASamples->count());
	connect(m_CreateClone, SIGNAL(clicked()), SLOT(CreateClone()));
	*/
}

AuGameClonepokTurn::~AuGameClonepokTurn()
{
}

void AuGameClonepokTurn::CreateClone()
{
	DhDNA dna = m_DNASamples->currentData().value<DhDNA>();

	// Pack my turn data into a blob the game will recognize at the other side.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)1;	// Create clone
	out << dna;

	// Emit a signal that ausceneclient can put into a packet to the server with my game settings.
	emit Accepted(AuGameClonepok::GameType(), ba);
	accept();
}

void AuGameClonepokTurn::MoveHorde(quint8 toSpace, QSet<quint8> unitIds)
{
	// Pack my turn data into a blob the game will recognize at the other side.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)2;	// Move horde
	out << toSpace;
	out << unitIds;

	// Emit a signal that ausceneclient can put into a packet to the server with my game settings.
	emit Accepted(AuGameClonepok::GameType(), ba);
	accept();
}

void AuGameClonepokTurn::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuGameClonepokTurn::RestoreGeometry()
{
	QString settingsName;
	if(m_TurnState == AuGameClonepok::StackBuilding)
		settingsName = "StackBuilding";
	else if(m_TurnState == AuGameClonepok::Landfall)
		settingsName = "Landfall";
	else
		settingsName = "Turn";

	int posX = g_Au.Settings().readNumEntry(QString("Widget/GameClonepok%1/PosX").arg(settingsName),-123);
	int posY = g_Au.Settings().readNumEntry(QString("Widget/GameClonepok%1/PosY").arg(settingsName),-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry(QString("Widget/GameClonepok%1/SizeX").arg(settingsName),szdef.width());
	int sizeY = g_Au.Settings().readNumEntry(QString("Widget/GameClonepok%1/SizeY").arg(settingsName),szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuGameClonepokTurn::SaveGeometry()
{
	QString settingsName;
	if(m_TurnState == AuGameClonepok::StackBuilding)
		settingsName = "StackBuilding";
	else if(m_TurnState == AuGameClonepok::Landfall)
		settingsName = "Landfall";
	else
		settingsName = "Turn";

	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry(QString("Widget/GameClonepok%1/PosX").arg(settingsName),ps.x());
	g_Au.Settings().writeEntry(QString("Widget/GameClonepok%1/PosY").arg(settingsName),ps.y());
	g_Au.Settings().writeEntry(QString("Widget/GameClonepok%1/SizeX").arg(settingsName),sz.width());
	g_Au.Settings().writeEntry(QString("Widget/GameClonepok%1/SizeY").arg(settingsName),sz.height());
}

void AuGameClonepokTurn::QuitGame()
{
	emit Quit();
	reject();
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGameClonepokFinish::AuGameClonepokFinish(const QByteArray& gameResult, QWidget* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":data/GameClonepokIcon.png"));
	setAuDlgTitle(tr("<b>Clonēpok Over</b>"));

	// Game result variables.
	quint32 randomNumber;			// The final random number.
	double share;					// Each winner's share of the kitty.
	QStringList winnerNames;		// Names of the winner.
	QStringList playerNames;		// Names of the players.
	QList<quint32> playerNumbers;	// Numbers entered by the players.

	// Deserialize the game result from the blob.
	quint8 listCount;
	quint8 digits;
	QString name;
	quint32 number;
	QDataStream in(gameResult);
	in.setVersion(QDataStream::Qt_4_1);
	in >> randomNumber;
	in >> digits;
	in >> share;
	in >> listCount;				// winner list
	for(; listCount; listCount--)
	{
		in >> name;
		winnerNames.append(name);
	}
	in >> listCount;				// players list
	for(; listCount; listCount--)
	{
		in >> name;
		in >> number;
		playerNames.append(name);
		playerNumbers.append(number);
	}

	// Create a little html report of the results.
	QString result("<html>");

	if(winnerNames.count() == 1)
	{
		// There was only one winner.
		result += tr("<img src=\"qrc:data/c_Money.png\"> %1 <i>WINS</i> %2")
			.arg(winnerNames.first())
			.arg(QString("%L1%2").arg(share).arg(g_Au.CurrencySymbol()));
	}
	else
	{
		// There was more than one winner.
		result += tr("<img src=\"qrc:data/CommCrew.png\"> <i>TIE</i> Between %1 Players!<br><br>").arg(winnerNames.count());

		for(QList<QString>::const_iterator wni = winnerNames.constBegin(); wni != winnerNames.constEnd(); wni++)
			result += QString("%1<br>").arg(*wni);
		result += "<br>";

		result += tr("<img src=\"qrc:data/c_Money.png\"> Each Player Wins %1")
			.arg(QString("%L1%2").arg(share).arg(g_Au.CurrencySymbol()));
	}

	// Reveal the final random number.
	result += "<table><tbody>";
	result += tr("<tr><td></td><td><img src=\"qrc:data/Game.png\"> <b>%1</b></td></tr>").arg(randomNumber, digits, 10, QChar('0'));

	// List the players
	for(int player=0; player<playerNames.count(); player++)
		result += tr("<tr><td><img src=\"qrc:data/Passenger.png\"> %1</td>"
				"<td><img src=\"qrc:data/ToggleMail.png\"> %2</td></tr>").arg(playerNames[player])
			.arg(QString("%1").arg(playerNumbers[player], digits, 10, QChar('0')));
	result += "</tbody></table>";

	// Terminate the report.
	result += "</html>";

	// Create the one and only widget.
	QTextBrowser* tb = new QTextBrowser;
	tb->setReadOnly(true);
	tb->setText(result);

	// Create the overall layout for the dialog.
	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(tb);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuGameClonepokFinish::~AuGameClonepokFinish()
{
}

void AuGameClonepokFinish::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuGameClonepokFinish::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/GameClonepokFinish/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/GameClonepokFinish/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/GameClonepokFinish/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/GameClonepokFinish/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuGameClonepokFinish::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/GameClonepokFinish/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/GameClonepokFinish/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/GameClonepokFinish/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/GameClonepokFinish/SizeY",sz.height());
}
