//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auplayersonline.cpp,v 1.14 2022/05/04 22:12:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auplayersonline.h"
//
#include "auagentteleport.h"
#include "auapp.h"
#include "auframe.h"
#include "austylesheet.h"
#include "autoolbutton.h"
#include "auview.h"
#include "dgdata.h"
#include <QGridLayout>
#include <QHeaderView>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>

void AuPlayersOnlineDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_TogglePlayersOnlineUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuPlayersOnlineTable::AuPlayersOnlineTable(QWidget* parent) : QTableWidget(0,2, parent)
{
	verticalHeader()->hide();
	verticalHeader()->setDefaultSectionSize(18);
	verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

	horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);

	QStringList headerLabels;
	headerLabels << tr("👤");
	headerLabels << tr("🏳️");
	setHorizontalHeaderLabels(headerLabels);

	setEditTriggers(QAbstractItemView::NoEditTriggers);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setShowGrid(false);
	setSortingEnabled(true);
	sortByColumn(0, Qt::AscendingOrder);
}

void AuPlayersOnlineTable::mousePressEvent(QMouseEvent* event)
{
	QTableWidget::mousePressEvent(event);
	m_MousePressItem = itemAt(event->pos());
}

void AuPlayersOnlineTable::mouseReleaseEvent(QMouseEvent* event)
{
	QTableWidget::mouseReleaseEvent(event);
	if(event->button() == Qt::RightButton)
	{
		QTableWidgetItem* item = itemAt(event->pos());
		if(item && item == m_MousePressItem)
			emit ItemRightClicked(item);
	}
}

QSize AuPlayersOnlineTable::sizeHint() const
{
	return QSize(500,600);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuPlayersOnlineWidget::AuPlayersOnlineWidget(QWidget* parent) : QWidget(parent)
{
	m_AFK = false;
	m_Timer = 0;

	m_PlayersLabel = new QLabel(tr("Avatars Online"), this);
	//m_PlayersLabel->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_Players = new AuPlayersOnlineTable(this);
	connect(m_Players, SIGNAL(ItemRightClicked(QTableWidgetItem*)), SLOT(PlayerRightClicked(QTableWidgetItem*)));
	m_Players->setToolTip(tr("<html>"
				"A tone sounds when someone on your contact list appears on this list or when they turn off their AFK setting. "
				"That tone can be configured on the settings window. "
				"</html>"));

	m_AFKCheck = new AuIconButton(":data/ComputerEye.png", this);
	m_AFKCheck->setToolTip(tr("<html>"
				"<p>"
				"Check this box to indicate to others that you are away from your keyboard. "
				"</p>"
				"<p>"
				"When checked, comm channels will alert you before transmitting your messages. "
				"</p>"
				"</html>"));
	connect(m_AFKCheck, SIGNAL(clicked()), SLOT(AFKClicked()));

	QHBoxLayout* blay = new QHBoxLayout();
	blay->addWidget(m_PlayersLabel);
	blay->addWidget(m_AFKCheck);
	blay->setMargin(0);
	blay->setSpacing(2);

	// Layout the widgets.
	QVBoxLayout* lay = new QVBoxLayout();
	lay->addWidget(m_Players);
	lay->addLayout(blay);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);

	for(int col=0; col<m_Players->columnCount(); col++)
	{
		if(col == 0)
			continue;	// skip the automatically sized column
		int colwid = g_Au.Settings().readNumEntry(QString("Widget/PlayersOnline/ColumnWid%1").arg(col),-123);
		if(colwid != -123)
			m_Players->setColumnWidth(col, colwid);
	}
}

AuPlayersOnlineWidget::~AuPlayersOnlineWidget()
{
}

void AuPlayersOnlineWidget::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to request player online status periodically.
	if(!m_Timer)
	{
		m_Timer = new QTimer(this);
		connect(m_Timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_Timer->isActive())
		m_Timer->start(10000);	// Server data is only updated every 3 minutes.

	// Post an initial request for online status, to populate our newly visible dialog.
	UpdateContents();
}

void AuPlayersOnlineWidget::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Kill the timer. It will be recreated during the next showEvent.
	if(m_Timer && m_Timer->isActive())
		m_Timer->stop();

	// Save the sizes of the table widget's columns.
	for(int col=0; col<m_Players->columnCount(); col++)
		g_Au.Settings().writeEntry(QString("Widget/PlayersOnline/ColumnWid%1").arg(col), m_Players->columnWidth(col));
}

void AuPlayersOnlineWidget::PlayersOnlineStatus(AuPlayerOnlineStatusVector players)
{
	// Id of currently selected avatar.
	AuId curId;
	QTableWidgetItem* curItem = m_Players->currentItem();
	if(curItem && curItem->isSelected())
	{
		QTableWidgetItem* item = m_Players->item(curItem->row(), 0);
		curId = AuId(item->data(Qt::UserRole).toUInt());
	}

	// Refresh the contents of the table widget.
	m_Players->clearContents();
	while(m_Players->rowCount())
		m_Players->removeRow(0);

	const DgData& gd = g_Au.LockDataForRead();
	bool isArchitect = gd.ClientPlayerIsArchitect();
	g_Au.UnlockData();

	QHash<AuId, QString> playersOn;
	int playersOnCount = 0;
	m_Players->setSortingEnabled(false);
	curItem = 0;
	for(AuPlayerOnlineStatusVector::const_iterator posi = players.constBegin(); posi != players.constEnd(); posi++)
	{
		if(!isArchitect && (*posi).IsIncognito() && (*posi).m_Id == AuId::Haxus())
			continue;	// Skip Haxus when incognito, except to architects.
		if((*posi).IsAnimal() || (*posi).IsEntity())
			continue;	// Animals and entities do not show up on the list.
		bool alreadyOn = m_PlayersOn.contains((*posi).m_Id);
		QIcon icon;
		if((*posi).IsAFK())
		{
			icon = QIcon(":data/ComputerEye.png");
			if(m_PlayersOn.contains((*posi).m_Id))
				m_PlayersOn.remove((*posi).m_Id);	// So we don't get the departing state when they go AFK.
		}
		else if((*posi).IsAgent() && !(*posi).IsIncognito())
		{
			icon = QIcon(":data/SoHLogo32.png");
			playersOn.insert((*posi).m_Id, (*posi).m_Name);
			if(!alreadyOn)
				AvatarOnLine((*posi).m_Id);
		}
		else if((*posi).IsChatClient())
		{
			// Note that regular non-agent players do not receive this status bit.
			icon = QIcon(":data/Microphone.png");
			playersOn.insert((*posi).m_Id, (*posi).m_Name);
			if(!alreadyOn)
				AvatarOnLine((*posi).m_Id);
		}
		else
		{
			// Regular player.
			icon = QIcon(":data/c_VacSuitHelmet.png");
			playersOn.insert((*posi).m_Id, (*posi).m_Name);
			if(!alreadyOn)
				AvatarOnLine((*posi).m_Id);
		}

		// The citizen is not in the table so insert a new record.
		playersOnCount++;
		m_Players->insertRow(0);

		// Player Name
		int col=0;
		QTableWidgetItem* twname = new QTableWidgetItem(icon, (*posi).m_Name);
		twname->setData(Qt::UserRole, (*posi).m_Id.GetID());
		if(!alreadyOn && !(*posi).IsAFK())
		{
			twname->setForeground(QColor(255,255,255));
			QFont f = twname->font();
			f.setBold(true);
			twname->setFont(f);
		}
		m_Players->setItem(0,col++, twname);
		if((*posi).IsAgent() && !(*posi).IsIncognito())
		{
			if((*posi).m_Id == AuId::Haxus() || (*posi).m_Id == AuId("V"))
			{
				if((*posi).IsAFK())
					twname->setToolTip(tr("%1 is away from his keyboard, contemplating improvements to the game. "
								"%1 is the architect of <i>%2</i>. "
								"<br>"
								"Right-click for options.").arg((*posi).m_Name).arg(g_Au.ApplicationName()));
				else if(!alreadyOn)
					twname->setToolTip(tr("%1 arrived. "
								"<br>"
								"%1 is the architect of <i>%2</i>. "
								"Ask %1 for help if assistance is needed. "
								"<br>"
								"Right-click for options.").arg((*posi).m_Name).arg(g_Au.ApplicationName()));
				else
					twname->setToolTip(tr("%1 is present, working hard to improve the game. "
								"<br>"
								"%1 is the architect of <i>%2</i>. "
								"Ask %1 for help if assistance is needed. "
								"<br>"
								"Right-click for options.").arg((*posi).m_Name).arg(g_Au.ApplicationName()));
			}
			else
			{
				if((*posi).IsAFK())
					twname->setToolTip(tr("%1 is away from their keyboard. "
								"%1 is an agent of <i>%2</i>. "
								"<br>"
								"Right-click for options.").arg((*posi).m_Name).arg(g_Au.ApplicationName()));
				else if(!alreadyOn)
					twname->setToolTip(tr("%1 arrived. "
								"<br>"
								"%1 is an agent of <i>%2 of Hazeron</i>. "
								"Ask %1 for help if assistance is needed. "
								"<br>"
								"Right-click for options.").arg((*posi).m_Name).arg(g_Au.ApplicationName()));
				else
					twname->setToolTip(tr("%1 is present. "
								"<br>"
								"%1 is an agent of <i>%2 of Hazeron</i>. "
								"Ask %1 for help if assistance is needed. "
								"<br>"
								"Right-click for options.").arg((*posi).m_Name).arg(g_Au.ApplicationName()));
			}
		}
		else
		{
			if((*posi).IsAFK())
				twname->setToolTip(tr("%1 is away from their keyboard.").arg((*posi).m_Name));
			else if(!alreadyOn)
				twname->setToolTip(tr("%1 arrived.").arg((*posi).m_Name));
			else
				twname->setToolTip(tr("%1 is present.").arg((*posi).m_Name));
		}

		// Empire
		QString empireName;
		QImage empireFlag;
		if((*posi).m_EmpireId)
		{
			const DgData& gd = g_Au.LockDataForRead();
			bool hasEmpire = gd.EmpireName((*posi).m_EmpireId, empireName);
			gd.EmpireFlag((*posi).m_EmpireId, empireFlag);
			g_Au.UnlockData();
			if(!hasEmpire)
				auApp->EmpireRequested((*posi).m_EmpireId);	// Request the empire record from the server.
		}
		QIcon flagIcon(QPixmap::fromImage(empireFlag));

		QTableWidgetItem* twempire = new QTableWidgetItem(flagIcon, empireName);
		twempire->setData(Qt::UserRole, (*posi).m_EmpireId);
		if(!alreadyOn && !(*posi).IsAFK())
		{
			twempire->setForeground(QColor(255,255,255));
			QFont f = twempire->font();
			f.setBold(true);
			twempire->setFont(f);
		}
		m_Players->setItem(0,col++, twempire);

		if(!curId.IsNull() && (*posi).m_Id == curId)
			curItem = twname;
	}

	// Show players who just departed.
	for(QHash<AuId, QString>::const_iterator di = m_PlayersOn.cbegin(); di != m_PlayersOn.cend(); di++)
	{
		if(playersOn.contains(di.key()))
			continue;

		QIcon icon(":data/MsgTransport.png");

		m_Players->insertRow(0);

		// Player Name
		int col=0;
		QTableWidgetItem* twname = new QTableWidgetItem(icon, *di);
		twname->setData(Qt::UserRole, di.key().GetID());
		twname->setForeground(AuStyleSheet::HazeronOrange());
		QFont f = twname->font();
		f.setItalic(true);
		twname->setFont(f);

		m_Players->setItem(0,col++, twname);
		twname->setToolTip(tr("%1 is departing.").arg(*di));

		if(!curId.IsNull() && di.key() == curId)
			curItem = twname;
	}
	m_PlayersOn = playersOn;				// Remember for next time through.
	m_Players->setSortingEnabled(true);
	if(curItem)
		m_Players->setCurrentItem(curItem);

	QString playtext;
	if(playersOnCount == 0)
		playtext = tr("No Avatar Online");
	else if(playersOnCount == 1)
		playtext = tr("One Avatar Online");
	else
		playtext = tr("%L1 Avatars Online").arg(playersOnCount);
	m_PlayersLabel->setText(playtext);
}

void AuPlayersOnlineWidget::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be restarted during the next showEvent.
		if(m_Timer && m_Timer->isActive())
			m_Timer->stop();
		return;
	}
	auApp->RequestPlayersOnline();
}

void AuPlayersOnlineWidget::AFKClicked()
{
	m_AFK = !m_AFK;
	m_AFKCheck->setDown(m_AFK);
	auApp->SetAFK(m_AFK);
}

bool AuPlayersOnlineWidget::IsAFK() const
{
	return m_AFKCheck->isDown();
}

void AuPlayersOnlineWidget::PlayerRightClicked(QTableWidgetItem* item)
{
	if(!item)
		return;

	const DgData& gd = g_Au.LockDataForRead();
	bool isProgrammer = gd.ClientPlayerIsProgrammer();
	AuId personId = gd.ClientUnitId();
	g_Au.UnlockData();

	// Our player's id and name are in column 0.
	QTableWidgetItem* item0 = m_Players->item(item->row(), 0);
	QString curName = item0->text();
	AuId gotoId = AuId(item0->data(Qt::UserRole).toUInt());
	bool contactIsMe = gotoId == personId;

	QMenu menu;

	if(!contactIsMe && isProgrammer)
	{
		QAction* goToAvatar = menu.addAction(QIcon(":data/MsgTransport.png"), tr("Teleport to %1...").arg(curName));
		connect(goToAvatar, SIGNAL(triggered()), SLOT(GoToAvatar()));
	}

	QAction* openWebPage = menu.addAction(QIcon(":data/c_VacSuitHelmet.png"), tr("View Web Page for %1...").arg(curName));

	if(!contactIsMe)
	{
		QAction *openchannel = menu.addAction(QIcon(":data/CommDirect.png"), tr("Open Channel to %1...")
				.arg(curName));
		connect(openchannel, SIGNAL(triggered()), SLOT(OpenChannelToAvatar()));
	}

	QAction* addBuddy = menu.addAction(QIcon(":data/ContactNew.png"), tr("Add %1 to Contact List").arg(curName));
	connect(addBuddy, SIGNAL(triggered()), SLOT(AddBuddy()));

	QAction* addSquelch = menu.addAction(QIcon(":data/ContactBlocked.png"), tr("Add %1 to Block List").arg(curName));
	connect(addSquelch, SIGNAL(triggered()), SLOT(AddSquelch()));

	QAction* action = menu.exec(QCursor::pos());
	if(!action)
		return;	// Menu canceled.

	if(action == openWebPage)
	{
		QUrl url(QString("https://www.hazeron.com/EmpireStandings/p%1.php").arg(gotoId.StringId()));
		QDesktopServices::openUrl(url);
	}
}

void AuPlayersOnlineWidget::GoToAvatar()
{
	QTableWidgetItem* curItem = m_Players->currentItem();
	if(!curItem)
		return;

	QTableWidgetItem* item = m_Players->item(curItem->row(), 0);
	AuId curId = AuId(item->data(Qt::UserRole).toUInt());
	if(curId.IsNull())
		return;
	QString curName = item->text();
	QMessageBox mb(g_Au.ApplicationName(),
			tr("This action will teleport you to %1.\n\n"
				"After 3 minutes, if %1 is not found, you will be redirected to the capital "
				"city of a randomly selected empire.").arg(curName),
		QMessageBox::Question, QMessageBox::Ok, QMessageBox::Cancel, QMessageBox::NoButton, this);
	if(auApp->Frame()->ExecMessageBox(mb) != QMessageBox::Ok)
		return;

	AuAgentTeleport::GoToDestination(curId, false, false);
}

void AuPlayersOnlineWidget::AddBuddy()
{
	QTableWidgetItem* curItem = m_Players->currentItem();
	if(!curItem)
		return;

	QTableWidgetItem* item = m_Players->item(curItem->row(), 0);
	AuId curId = AuId(item->data(Qt::UserRole).toUInt());
	if(curId.IsNull())
		return;
	QString curName = item->text();

	emit AddBuddy(curId, curName);
}

void AuPlayersOnlineWidget::AddSquelch()
{
	QTableWidgetItem* curItem = m_Players->currentItem();
	if(!curItem)
		return;

	QTableWidgetItem* item = m_Players->item(curItem->row(), 0);
	AuId curId = AuId(item->data(Qt::UserRole).toUInt());
	if(curId.IsNull())
		return;
	QString curName = item->text();

	emit AddSquelch(curId, curName);
}

void AuPlayersOnlineWidget::OpenChannelToAvatar()
{
	QTableWidgetItem* curItem = m_Players->currentItem();
	if(!curItem)
		return;

	QTableWidgetItem* item = m_Players->item(curItem->row(), 0);
	AuId curId = AuId(item->data(Qt::UserRole).toUInt());
	if(curId.IsNull())
		return;
	emit OpenChannelToAvatar(curId);
}
