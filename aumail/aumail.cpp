//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumail.cpp,v 1.127 2022/05/04 21:44:36 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aumail.h"
//
#include "aucontacts.h"
#include "auglobal.h"
#include "aulogincomm.h"
#include "aumailmsg.h"
#include "aumailnew.h"
#include "aumailsettings.h"
#include "aumailtreeview.h"
#include "aumission.h"
#include "dgdata.h"
#include "dtmailclient.h"
#include <QAction>
#include <QComboBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QProgressBar>
#include <QSplitter>
#include <QTextBrowser>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>

AuMail::AuMail(QWidget* parent) : QWidget(parent)
{
	m_ActivityCount = 0;
	m_idMessage = 0;
	m_timer = 0;
	m_UpdateIcons = false;

	setWindowIcon(QIcon(":data/Mail.png"));
	setWindowTitle(g_Au.ApplicationName());

	m_ActivityIndicator = new QLabel(this);
	m_ActivityIndicator->setPixmap(QPixmap(":data/MailInactive.png"));
	m_ActivityIndicator->setToolTip(tr("<html>Activity Indicator<br>"
				"Active when messages are being received."));

	m_PlayerCombo = new QComboBox(this);
	m_PlayerCombo->setToolTip(tr("Select Current Player"));
	connect(m_PlayerCombo, SIGNAL(currentIndexChanged(int)), SLOT(PersonChanged(int)));
	int curIdx = 0;
	const DgData& gd = g_Au.LockDataForRead();
	for(int iu=0; iu<6; iu++)
	{
		const AuId& id = gd.ClientUnitId(iu);
		if(!id.IsNull())
		{
			m_PlayerCombo->addItem(gd.ClientUnitName(id), id.GetID());
			if(id == gd.ClientUnitId())
				curIdx = iu;	// This is the character that the player logged in to.
		}
	}

	// need this for requesting contacts
	QString sAcctName = gd.GetAcctName();
	QByteArray sAcctPassword = gd.GetAcctPassCrypt();
	bool bEmpty = gd.GetContacts().isEmpty();

	g_Au.UnlockData();
	m_PlayerCombo->setCurrentIndex(curIdx);
	m_idPerson = AuId(m_PlayerCombo->itemData(curIdx).toUInt());

	if(bEmpty && !g_Au.IsSceneSolo())
	{
		// If it is empty, we may not have requested the contacts list yet.
		AuLoginComm login;
		login.RequestContacts(sAcctName,sAcctPassword);
	}

	QAction* pbsettings = new QAction(QIcon(":data/Settings.png"), tr("Settings..."), this);
	pbsettings->setToolTip(tr("Adjust mail settings."));
	connect(pbsettings, SIGNAL(triggered()), SLOT(MailSettings()));

	QAction* pbget = new QAction(QIcon(":/data/GetMail.png"), tr("Request New Messages"), this);
	pbget->setToolTip(tr("Request New Messages"));
	connect(pbget, SIGNAL(triggered()), SLOT(RequestMessages()));

	QAction* pbnew = new QAction(QIcon(":/data/MailMessage.png"), tr("Compose New Message..."), this);
	pbnew->setToolTip(tr("Compose New Message"));
	connect(pbnew, SIGNAL(triggered()), SLOT(NewMessage()));

	//////////////////////////////////////////////////////////////////////////

	QAction* pbdel = new QAction(QIcon(":data/DeleteMail.png"), tr("Delete..."), this);
	pbdel->setToolTip(tr("Deletes all selected messages. "
				"Deletes all messages from selected message senders; the sender is the folder that contains the mail messages."));
	connect(pbdel, SIGNAL(triggered()), SLOT(DeleteMessages()));

	QAction* pbreply = new QAction(QIcon(":data/MailReply.png"), tr("Reply..."), this);
	pbreply->setToolTip(tr("Reply to current message."));
	connect(pbreply, SIGNAL(triggered()), SLOT(ReplyToMessage()));

	QAction* pbfwd = new QAction(QIcon(":data/MailForward.png"), tr("Forward..."), this);
	pbfwd->setToolTip(tr("Forward the current message."));
	connect(pbfwd, SIGNAL(triggered()), SLOT(ForwardMessage()));

	//////////////////////////////////////////////////////////////////////////

	QAction* pbcontacts = new QAction(QIcon(":data/Contacts.png"), tr("Manage Contacts..."), this);
	pbcontacts->setToolTip(tr("Manage Contacts"));
	connect(pbcontacts, SIGNAL(triggered()), SLOT(ManageContacts()));

	QAction* pbaddbuddy = new QAction(QIcon(":data/ContactNew.png"), tr("Add Contact..."), this);
	pbaddbuddy->setToolTip(tr("Add current sender to Contact List"));
	connect(pbaddbuddy, SIGNAL(triggered()), SLOT(AddBuddy()));

	QAction* pbaddsquelch = new QAction(QIcon(":data/ContactBlocked.png"), tr("Block Sender..."), this);
	pbaddsquelch->setToolTip(tr("Block future messages from the current sender."));
	connect(pbaddsquelch, SIGNAL(triggered()), SLOT(AddSquelch()));

	m_Mail = new AuMailTreeView;
	m_Mail->setAlternatingRowColors(true);
	m_Mail->setModel(&m_Model);
	m_Mail->setObjectName(tr("mailtree"));
	connect(m_Mail, SIGNAL(ItemRightClicked(const QModelIndex&)), SLOT(MessageRightClicked(const QModelIndex&)));
	connect(m_Mail, SIGNAL(SelectionChanged()), SLOT(MessageSelectionChanged()));

	m_MessageText = new QTextBrowser;
	m_MessageText->setReadOnly(true);

	m_BatchProgress = new QProgressBar(this);
	m_BatchProgress->setFormat("Indexing Message %v of %m");
	m_BatchProgress->setTextVisible(true);
	m_BatchProgress->hide();

	m_Splitter = new QSplitter(this);
	m_Splitter->setOrientation(Qt::Vertical);
	m_Splitter->addWidget(m_Mail);
	m_Splitter->addWidget(m_MessageText);
	connect(m_Splitter, SIGNAL(splitterMoved(int, int)), SLOT(SplitterMoved(int, int)));

	QGridLayout* lay = new QGridLayout();

	QToolBar* buttonBar = new QToolBar(this);

	QWidget* spacer0 = new QWidget(this);
	spacer0->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

	QWidget* spacer1 = new QWidget(this);
	spacer1->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

	buttonBar->addWidget(m_ActivityIndicator);
	buttonBar->addWidget(m_PlayerCombo);
	buttonBar->addAction(pbsettings);
	buttonBar->addSeparator();
	buttonBar->addAction(pbget);
	buttonBar->addAction(pbnew);

	buttonBar->addWidget(spacer0);
	buttonBar->addSeparator();
	buttonBar->addAction(pbdel);
	buttonBar->addAction(pbreply);
	buttonBar->addAction(pbfwd);
	buttonBar->addSeparator();
	buttonBar->addWidget(spacer1);

	buttonBar->addAction(pbcontacts);
	buttonBar->addAction(pbaddbuddy);
	buttonBar->addAction(pbaddsquelch);

	buttonBar->setIconSize(QSize(16,16));
	buttonBar->setMovable(false);

	int row=0;
	lay->addWidget(buttonBar, row++, 0);
	lay->addWidget(m_BatchProgress, row++, 0);
	lay->addWidget(m_Splitter, row++, 0);
	//lay->setColumnMinimumWidth(1,4);
	//lay->setColumnStretch(0,1);
	lay->setMargin(4);
	lay->setSpacing(2);
	setLayout(lay);

	// The mail client thread is created now, so it can respond to the VisibilityChanged signal.
	m_tMailClient = new DtMailClient(this);
	connect(m_tMailClient, SIGNAL(CachedMessageReceived(AuMailMsg)), SLOT(CachedMessageReceived(AuMailMsg)), Qt::QueuedConnection);
	connect(m_tMailClient, SIGNAL(DeferFailed()), SLOT(ServerError()), Qt::QueuedConnection);
	connect(m_tMailClient, SIGNAL(MailBatchEnd()), SLOT(MailBatchEnd()), Qt::QueuedConnection);
	connect(m_tMailClient, SIGNAL(MailBatchStart(int)), SLOT(MailBatchStart(int)), Qt::QueuedConnection);
	connect(m_tMailClient, SIGNAL(LoginError()), SLOT(LoginError()), Qt::QueuedConnection);
	connect(m_tMailClient, SIGNAL(MessageDiscarded()), SLOT(MessageDiscarded()), Qt::QueuedConnection);
	connect(m_tMailClient, SIGNAL(NewMessageReceived(AuMailMsg)), SLOT(NewMessageReceived(AuMailMsg)), Qt::QueuedConnection);
	connect(m_tMailClient, SIGNAL(VersionError()), SLOT(VersionError()), Qt::QueuedConnection);
	connect(this, SIGNAL(PersonSelected(AuId)), m_tMailClient, SLOT(SelectPerson(AuId)), Qt::QueuedConnection);
	connect(this, SIGNAL(VisibilityChanged(bool)), m_tMailClient, SLOT(MailVisibilityChanged(bool)), Qt::QueuedConnection);
	m_tMailClient->start();
	emit PersonSelected(m_idPerson);
}

AuMail::~AuMail()
{
	if(m_tMailClient)
		StopThreads();
}

void AuMail::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_timer)
	{
		m_timer = new QTimer(this);
		connect(m_timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_timer->isActive())
		m_timer->start(2000);

	emit VisibilityChanged(true);
}

void AuMail::hideEvent(QHideEvent* event)
{
	SaveGeometry();

	if(m_timer && m_timer->isActive())
		m_timer->stop();

	QWidget::hideEvent(event);

	emit VisibilityChanged(false);
}

void AuMail::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be restarted during the next showEvent.
		if(m_timer && m_timer->isActive())
			m_timer->stop();
		return;
	}

	if(m_UpdateIcons)
	{
		UpdateModelIcons();		// Update the icons that show priority status.
		m_UpdateIcons = false;
	}

	// Show network activity.
	if(m_MsgActivity.isValid() && m_MsgActivity.elapsed() > 1000)
		m_ActivityIndicator->setPixmap(QPixmap(":data/MailInactive.png"));

	// We auto delete old mail every 20 minutes, not every 2 seconds.
	// Start/check the auto deletion timer.
	if(!m_DeleteTimer.isValid())
		m_DeleteTimer.start();
	else if(m_DeleteTimer.elapsed() > 20 * 60 * 1000)
	{
		DeleteOldMail();
		m_DeleteTimer.restart();
	}
}

void AuMail::NewMessage()
{
	AuMailNew* mail = new AuMailNew(this, m_idPerson);
	mail->show();
}

// Deletes old mail that is supposed to be automatically deleted, according to global settings.
void AuMail::DeleteOldMail()
{
	QList<quint32> oldMailIds;
	DtMailClient::DeleteOldMail(m_idPerson, oldMailIds);

	// Locate each mail id to be deleted in my data model.
	for(QList<quint32>::const_iterator omi = oldMailIds.constBegin(); omi != oldMailIds.constEnd(); omi++)
	{
		//qDebug() << "AuMail::DeleteOldMail looking for message" << *omi;
		QModelIndex mailIdx = FindMail(*omi, QModelIndex());	// Recursively search, beginning at the root.
		if(mailIdx != QModelIndex())
		{
			//qDebug() << "AuMail::DeleteOldMail deleting message" << *omi;
			DeleteMessage(mailIdx);								// Delete the mail message tree item and local file cache.
		}
	}
}

QModelIndex AuMail::FindMail(quint32 findMsgId, const QModelIndex& parIdx)
{
	int rows = m_Model.rowCount(parIdx);
	for(int row=0; row<rows; row++)
	{
		QModelIndex col0Idx = m_Model.index(row,0,parIdx);		// Index of col 0 under parent.
		NODE_TYPE ntype = (NODE_TYPE)m_Model.data(col0Idx, Qt::UserRole).toInt();
		if(ntype == NODE_MailMessage)
		{
			// Check to see if the message has the desired id.
			unsigned int mailMsgId = m_Model.data(col0Idx, Qt::UserRole+1).toUInt();
			if(mailMsgId == findMsgId)
				return col0Idx;
		}
		else
		{
			// Not a mail message node so it's a parent node. Recurse here.
			QModelIndex mailIdx = FindMail(findMsgId, col0Idx);
			if(mailIdx != QModelIndex())
				return mailIdx;									// Mail message was found subordinate to the node.
		}
	}

	// Mail message was not found.
	return QModelIndex();
}

// Deletes all local mail files that were sent to 'id'.
//static
void AuMail::DeleteLocalMail(const AuId& id)
{
	QString mailPath(g_Au.CacheMail());
	QDir mailDir(mailPath);
	if(mailDir.exists())
	{
		QStringList nameFilters;
		nameFilters << QString("%1.*.*.m").arg(id.StringId());
		QStringList delFiles = mailDir.entryList(nameFilters);
		for(QStringList::const_iterator dfi = delFiles.constBegin(); dfi != delFiles.constEnd(); dfi++)
			mailDir.remove(*dfi);
	}
}

void AuMail::DeleteMessages()
{
	// Loop on all selected messages.
	QItemSelectionModel* selection = m_Mail->selectionModel();
	while(selection && selection->hasSelection())
	{
		// Delete the first selected row.
		QModelIndexList selIdx = selection->selectedRows();
		if(!DeleteMessage(selIdx.first()))
			break;

		// Reacquire the selection model because of the change to the tree contents.
		selection = m_Mail->selectionModel();
	}
}

void AuMail::DeleteMessage()
{
	// Delete only the current message.
	QModelIndex itemIdx = m_Mail->currentIndex();
	DeleteMessage(itemIdx);
}

bool AuMail::DeleteMessage(const QModelIndex& idx)
{
	//qDebug("AuMail::DeleteMessage");
	bool messageDeleted = false;
	QModelIndex col0Idx = idx.sibling(idx.row(),0);
	NODE_TYPE ntype = (NODE_TYPE)m_Model.data(col0Idx, Qt::UserRole).toInt();
	if(ntype == NODE_MailMessage)
	{
		// Check for a selected message. The message id will be in the user data of column 1.
		unsigned int mailMsgId = m_Model.data(col0Idx, Qt::UserRole+1).toUInt();
		if(mailMsgId != 0)
		{
			// A single message is selected.
			//if(QMessageBox::question(this,
						//g_Au.ApplicationName(),
						//"The selected message\nwill be deleted.\n\n"
						//"Are you sure?", QMessageBox::Yes, QMessageBox::No,
						//QMessageBox::NoButton) == QMessageBox::Yes)
			{
				//qDebug("Delete message item id %d", mailMsgId);
				// Remove the selected item from the model.
				m_Model.removeRows(idx.row(),1,idx.parent());

				// Delete the locally cached mail file.
				QString mailPath(g_Au.CacheMail());
				QDir mailDir(mailPath);
				if(mailDir.exists())
				{
					QStringList nameFilters;
					nameFilters << QString("*.*.%1.m").arg(mailMsgId);
					QStringList delFiles = mailDir.entryList(nameFilters);
					for(QStringList::const_iterator dfi = delFiles.constBegin(); dfi != delFiles.constEnd(); dfi++)
						mailDir.remove(*dfi);
				}

				messageDeleted = true;
			}
		}
	}
	else
	{
		AuId containerSenderId(m_Model.data(col0Idx, Qt::UserRole+1).toUInt());
		if(!containerSenderId.IsNull())
		{
			// A top level item is selected, which represents all the mail from a particular sender id.
			if(QMessageBox::warning(this, g_Au.ApplicationName(),
						tr("%1\n\nAll messages from %1\nwill be deleted.\n\n"
						"Are you sure?").arg(m_Model.data(col0Idx).toString()), QMessageBox::Yes, QMessageBox::No,
						QMessageBox::NoButton) == QMessageBox::Yes)
			{
				// Remove the subitems from the parent item before removing the parent
				// item itself. I don't know if the parent would remove them for me if
				// I just removed the parent.
				m_Model.removeRows(0,m_Model.rowCount(idx),idx);
				m_Model.removeRows(idx.row(),1,idx.parent());

				// Delete the locally cached mail files.
				QString mailPath(g_Au.CacheMail());
				QDir mailDir(mailPath);
				if(mailDir.exists())
				{
					QStringList nameFilters;
					nameFilters << QString("%2.%1.*.m").arg(containerSenderId.StringId()).arg(m_idPerson.StringId());
					QStringList delFiles = mailDir.entryList(nameFilters);
					for(QStringList::const_iterator dfi = delFiles.constBegin(); dfi != delFiles.constEnd(); dfi++)
						mailDir.remove(*dfi);
				}

				messageDeleted = true;
			}
		}
	}

	// When a message is deleted, update the icons that show priority status.
	if(messageDeleted)
		m_UpdateIcons = true;

	return messageDeleted;
}

void AuMail::LoginError()
{
	QMessageBox::critical(this, g_Au.ApplicationName(), "Login incorrect.",
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
	close();
}

void AuMail::ServerError()
{
	QMessageBox::critical(this, g_Au.ApplicationName(), tr("Server error.\n\nThe mail "
			"server encountered an internal error.\n\nPlease try again later. Thank "
			"you for your patience."),
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
	close();
}

void AuMail::MessageDiscarded()
{
	//qDebug() << "AuMail::MessageDiscarded - Message discarded";
	m_MsgActivity.restart();
	m_ActivityCount = (m_ActivityCount + 1) % 4;
	m_ActivityIndicator->setPixmap(QPixmap(QString(":data/MailActive%1.png").arg(m_ActivityCount)));
}

void AuMail::NewMessageReceived(AuMailMsg msg)
{
	MessageReceived(msg);

	if(g_Au.MailNotificationMode() && msg.IsAlert())
		emit AlertMessageReceived(msg);
}

void AuMail::CachedMessageReceived(AuMailMsg msg)
{
	MessageReceived(msg);
}

void AuMail::MessageReceived(const AuMailMsg& msg)
{
	//qDebug() << "AuMail::MessageReceived - Message Received" << msg.m_id;
	m_MsgActivity.restart();
	m_ActivityCount = (m_ActivityCount + 1) % 4;
	m_ActivityIndicator->setPixmap(QPixmap(QString(":data/MailActive%1.png").arg(m_ActivityCount)));
	if(m_BatchProgress->isVisible())
		m_BatchProgress->setValue(m_BatchProgress->value()+1);

	//qDebug("AuMail::MessageReceived - From '%s' Subject '%s'",
			//(const char*)msg.m_nameFrom.toLatin1(),
			//(const char*)msg.m_subject.toLatin1());
	if(msg.m_idTo != m_idPerson)
	{
		// The current person was probably changed before the last mail request was completed.
		// Just ignore messages that are not addressed to the current person.
#if !defined(QT_NO_DEBUG)
		qDebug("AuMail::MessageReceived Ignored mail message to non-current person %d; current person is %d.",
				msg.m_idTo.GetID(), m_idPerson.GetID());
#endif
		return;
	}

	// Check for any items before adding so we know whether to resize column 0.
	bool hasRows = false;
	QModelIndex rootIdx;
	if(m_Model.rowCount(rootIdx))
		hasRows = true;

	// Search for a root level item with the same id as the sender.
	NODE_TYPE parNodeType = NODE_Individual;				// Node type for parent container that will hold the mail message.
	if(msg.HasHierarchyHeader())
	{
		// City reports are organized in a tree by galaxy, sector, solar system, world, then city name.
		// Dissect the sector, solar system, and world names from the report itself, which is the body text of the message.
		QStringList lines = msg.m_body.split("<br>");
		bool hasDate = false;
		if(lines[0].count() == 12 && lines[0].left(4) == "UTC:")	// e.g. UTC:01234567, always 8 hex digits
		{
			// 'date' format.
			// Remove the date line and accompanying blank line.
			hasDate = true;
			lines.removeFirst();
			lines.removeFirst();
		}

		QStringList firstLines = lines[0].split("\">");	// The end of the <div style in line 0 must break the first line.
		if(firstLines.count() > 1)
		{
			lines[0] = firstLines[0];
			lines.insert(1,firstLines[1]);
		}
		if(lines.count() < 3)
		{
			qWarning("AuMail::MessageReceived - Bad city message Id %d From '%s' Subject '%s'\n%s",
					msg.m_id,
					(const char*)msg.m_nameFrom.toLatin1(),
					(const char*)msg.m_subject.toLatin1(),
					(const char*)msg.m_body.toLatin1());
			return;
		}

		// Get names for hierarchy.
		QString universeName = tr("Empire");					// Root node for city reports hierarchy.
		QString galaxyName;
		GALAXY galaxy = (GALAXY)msg.m_fromGalaxy;				// The galaxy can be acquired directly from the message.
		if(galaxy >= 0 && galaxy < GALAXY_Count)
			galaxyName = GalaxyName(galaxy);	// This global function is in audef.h so I don't create a dependency on auatom here.
		else
			galaxyName = GalaxyName(ShoresOfHazeron);
		QString worldName;
		QString systemName;
		QString sectorName;

		// Format changed when stardate lines added.
		if(!hasDate)
		{
			// Old format without date or specific system name, when system names always matched world names.
			sectorName = lines[2].trimmed();
			worldName = lines[1].trimmed();
			int comma = worldName.indexOf(',');
			if(comma > 0)
				worldName = worldName.left(comma);
			int space = worldName.lastIndexOf(' ');
			systemName = worldName.left(space);
			if(systemName.right(6) == " Alpha")
				systemName = systemName.left(systemName.length()-6);
			else if(systemName.right(5) == " Beta")
				systemName = systemName.left(systemName.length()-5);
			else if(systemName.right(6) == " Gamma")
				systemName = systemName.left(systemName.length()-6);
		}
		else
		{
			// New format with date and specific system name, now that system names don't have to match world names.
			worldName = lines[1].trimmed();
			systemName = lines[2].trimmed();
			sectorName = lines[3].trimmed();
		}

		// Root item is universe.
		QModelIndex universeIdx = AddTreeItem(universeName, QIcon(":data/RangeUniverse.png"), NODE_Empire, rootIdx);

		// Item under universe is galaxy.
		QModelIndex galaxyIdx = AddTreeItem(galaxyName, QIcon(":data/RangeGalaxy.png"), NODE_Galaxy, universeIdx);

		// Item under galaxy is sector.
		QModelIndex sectorIdx = AddTreeItem(sectorName, QIcon(":data/Sector.png"), NODE_Sector, galaxyIdx);

		// Item under sector is solar system.
		QModelIndex systemIdx = AddTreeItem(systemName, QIcon(":data/SolarSystem.png"), NODE_System, sectorIdx,
				msg.m_fromSystemId, msg.m_fromSystem, AuId(0), "", msg.m_fromGalaxyX, msg.m_fromGalaxyY, msg.m_fromGalaxyZ);

		// Item under solar system is a world.
		// Root for the message sender will be the world item.
		rootIdx = AddTreeItem(worldName, QIcon(":data/World.png"), NODE_World, systemIdx, msg.m_fromSystemId, msg.m_fromSystem,
				msg.m_fromWorldId, msg.m_fromWorld, msg.m_fromGalaxyX, msg.m_fromGalaxyY, msg.m_fromGalaxyZ);

		// The parent node of my mail messages will be a city node.
		parNodeType = NODE_City;
	}
	else if(msg.IsOfficerMessage())
	{
		// Root item for the message sender is officers.
		rootIdx = AddTreeItem(tr("Officers"), QIcon(":data/Officer.png"), NODE_Officers, rootIdx);

		// The parent node of my mail messages will be an officer node.
		parNodeType = NODE_Officer;
	}
	else if(msg.IsShipLog())
	{
		// Root item for the message sender is ships.
		rootIdx = AddTreeItem(tr("Fleet"), QIcon(":data/FleetShip.png"), NODE_Fleet, rootIdx);

		// The parent node of my mail messages will be a spacecraft node.
		parNodeType = NODE_Spacecraft;
	}
	else if(msg.IsGovtOrigin())
	{
		// Root item for the message sender is govt.
		rootIdx = AddTreeItem(tr("Government"), QIcon(":data/MailGovtOrigin.png"), NODE_Government, rootIdx);

		// The parent node of my mail messages will be a department node.
		parNodeType = NODE_Department;
	}

	// Messages are collected into a container identified by sender.
	// Look for the container that contains all message from the message sender.
	bool foundParent = false;
	QModelIndex parIdx;
	int rootRows = m_Model.rowCount(rootIdx);
	int insertAt = rootRows;	// Insertion index when existing entry not found.
	for(int ir=0; ir<rootRows; ir++)
	{
		parIdx = m_Model.index(ir,0,rootIdx);	// Idx of ir under root.

		NODE_TYPE ntype = (NODE_TYPE)m_Model.data(parIdx, Qt::UserRole).toInt();
		AuId containerSenderId;
		if(ntype != NODE_MailMessage)
			containerSenderId = AuId(m_Model.data(parIdx, Qt::UserRole+1).toUInt());

		if(containerSenderId == msg.m_idFrom)
		{
			foundParent = true;
			break;
		}
		else
		{
			// Watch for the alphabetical order insert index.
			QVariant parName = m_Model.data(parIdx);
			if(insertAt == rootRows && parName.toString().localeAwareCompare(msg.m_nameFrom) > 0)
				insertAt = ir;
		}
	}

	// If no container item for sender yet; make one.
	if(!foundParent)
	{
		// The sender's container does not exist.
		// Create a new container item to categorize messages by sender.
		m_Model.insertRow(insertAt,rootIdx);
		parIdx = m_Model.index(insertAt,0,rootIdx);
		m_Model.insertColumns(0,2,parIdx);
		m_Model.setData(parIdx, msg.m_nameFrom);
		m_Model.setData(parIdx, QIcon(msg.OriginIcon()), Qt::DecorationRole);
		// For positive id of the sender folder.
		m_Model.setData(parIdx, parNodeType, Qt::UserRole);
		m_Model.setData(parIdx, msg.m_idFrom.GetID(), Qt::UserRole+1);
		m_Model.setData(parIdx, 0, Qt::UserRole+2);							// Priority when this is a mail message node.

		// Optional origin data, some or all of it may be 0.
		m_Model.setData(parIdx, msg.m_fromSystemId.GetID(), Qt::UserRole+3);
		m_Model.setData(parIdx, msg.m_fromSystem, Qt::UserRole+4);
		m_Model.setData(parIdx, msg.m_fromWorldId.GetID(), Qt::UserRole+5);
		m_Model.setData(parIdx, msg.m_fromWorld, Qt::UserRole+6);
		m_Model.setData(parIdx, msg.m_fromGalaxyX, Qt::UserRole+7);
		m_Model.setData(parIdx, msg.m_fromGalaxyY, Qt::UserRole+8);
		m_Model.setData(parIdx, msg.m_fromGalaxyZ, Qt::UserRole+9);
	}
	else
	{
		// The sender's container already exists.
		// Check to see if the message already exists in that container.
		QModelIndex itemIdx;
		int parRows = m_Model.rowCount(parIdx);
		for(int ir=0; ir<parRows; ir++)
		{
			itemIdx = m_Model.index(ir,0,parIdx);	// Index of col 0 under parent.

			NODE_TYPE ntype = (NODE_TYPE)m_Model.data(itemIdx, Qt::UserRole).toInt();
			if(ntype != NODE_MailMessage)
				continue;

			unsigned int mailMsgId = m_Model.data(itemIdx, Qt::UserRole+1).toUInt();
			if(mailMsgId == msg.m_id)
				return;										// Found existing item. Mail messages don't change so we're done.
		}
	}

	int priority = msg.Priority();
	if(priority)
		m_UpdateIcons = true;

	// We did not find an existing message to update so we must add a new one.
	QIcon pmType(msg.TypeIcon());
	m_Model.insertRow(0,parIdx);
	QModelIndex itemIdx = m_Model.index(0,0,parIdx);
	m_Model.setData(itemIdx, msg.m_subject);
	m_Model.setData(itemIdx, pmType, Qt::DecorationRole);

	// Store type and priority values on the message item object.
	m_Model.setData(itemIdx, NODE_MailMessage, Qt::UserRole);
	m_Model.setData(itemIdx, msg.m_id, Qt::UserRole+1);
	m_Model.setData(itemIdx, priority, Qt::UserRole+2);

	// Optional origin data, some or all of it may be 0.
	m_Model.setData(itemIdx, msg.m_fromSystemId.GetID(), Qt::UserRole+3);
	m_Model.setData(itemIdx, msg.m_fromSystem, Qt::UserRole+4);
	m_Model.setData(itemIdx, msg.m_fromWorldId.GetID(), Qt::UserRole+5);
	m_Model.setData(itemIdx, msg.m_fromWorld, Qt::UserRole+6);
	m_Model.setData(itemIdx, msg.m_fromGalaxyX, Qt::UserRole+7);
	m_Model.setData(itemIdx, msg.m_fromGalaxyY, Qt::UserRole+8);
	m_Model.setData(itemIdx, msg.m_fromGalaxyZ, Qt::UserRole+9);

	itemIdx = m_Model.index(0,1,parIdx);
	m_Model.setData(itemIdx, msg.m_tmSent.UTC().toLocalTime());

	if(!hasRows)
	{
		// First row added causes us to resize some columns.
		if(width() >= 250)
			m_Mail->setColumnWidth(0, width()-125);
	}
	else
	{
		m_Model.sort(1, Qt::DescendingOrder);
	}
}

void AuMail::VersionError()
{
	QMessageBox::critical(this, g_Au.ApplicationName(),
			tr("Version mismatch error. This software is not compatible with the mail server."),
			QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
	close();
}

void AuMail::MessageSelectionChanged()
{
	QString msgBody;	// Default is blank in case an invalid item is selected. i.e. a 'from' parent.

	QItemSelectionModel* selection = m_Mail->selectionModel();
	if(selection && selection->currentIndex() != QModelIndex())
	{
		QModelIndex idx = selection->currentIndex();
		QModelIndex itemIdx = idx.sibling(idx.row(),0);
		NODE_TYPE ntype = (NODE_TYPE)m_Model.data(itemIdx, Qt::UserRole).toInt();
		unsigned int mailMsgId = m_Model.data(itemIdx, Qt::UserRole+1).toUInt();
		if(ntype == NODE_MailMessage && mailMsgId != 0)
		{
			AuMailMsg msg;
			bool foundMsg = DtMailClient::LoadMessage(mailMsgId, msg);
			if(foundMsg)
			{
				m_idMessage = msg.m_id;
				msgBody = g_Au.DecodeUTCStrings(msg.m_body);
			}
		}
	}

	// Set the text whether or not we found a message; it may be blank.
	m_MessageText->setHtml(msgBody);
}

void AuMail::MessageRightClicked(const QModelIndex& idx)
{
	QModelIndex itemIdx = idx.sibling(idx.row(), 0);

	// Get a copy of the original mail message.
	bool foundMsg = false;
	AuMailMsg msg;
	quint32 mailMsgId = m_Model.data(itemIdx, Qt::UserRole+1).toUInt();	// Get the mail message id.
	if(mailMsgId)
		foundMsg = DtMailClient::LoadMessage(mailMsgId, msg);

	// Get information about the item that was clicked.
	NODE_TYPE ntype = (NODE_TYPE)m_Model.data(itemIdx, Qt::UserRole).toInt();
	AuId fromSystemId = AuId(m_Model.data(itemIdx, Qt::UserRole+3).toUInt());
	QString fromSystem = m_Model.data(itemIdx, Qt::UserRole+4).toString();
	AuId fromWorldId = AuId(m_Model.data(itemIdx, Qt::UserRole+5).toUInt());
	QString fromWorld = m_Model.data(itemIdx, Qt::UserRole+6).toString();
	float fromGalaxyX = m_Model.data(itemIdx, Qt::UserRole+7).toFloat();
	float fromGalaxyY = m_Model.data(itemIdx, Qt::UserRole+8).toFloat();
	float fromGalaxyZ = m_Model.data(itemIdx, Qt::UserRole+9).toFloat();
	//qDebug() << "AuMail::MessageRightClicked type" << ntype << "system" << fromSystemId << fromSystem
		//<< "world" << fromWorldId << fromWorld << "galaxyLoc" << fromGalaxyX << fromGalaxyY << fromGalaxyZ;

	// Create a menu of relevant actions.
	QMenu menu(this);

	AuId fromCityId;
	QString fromCity;
	QAction* aAddBuddy = 0;
	QAction* aSquelch = 0;
	QAction* aReply = 0;
	QAction* aForward = 0;
	QAction* aDelete = 0;
	if(ntype == NODE_MailMessage)
	{
		if(foundMsg)
		{
			if(msg.Type() == AuMailMsg::MSG_Mail || msg.Type() == AuMailMsg::MSG_CityDiplomaticMessage)
			{
				// Only mail from players can be used to add people to the buddy list or to reply.
				aAddBuddy = menu.addAction(QIcon(":data/ContactNew.png"), tr("Add Contact..."));
				aSquelch = menu.addAction(QIcon(":data/ContactBlocked.png"), tr("Block Sender..."));
				menu.addSeparator();
				aReply = menu.addAction(QIcon(":data/MailReply.png"), tr("Reply..."));
			}
			else if(msg.HasHierarchyHeader())
			{
				// Only city mail has a hierarchy header.
				fromCityId = msg.m_idFrom;
				fromCity = msg.m_nameFrom;
			}
		}

		// Enable forward and deletion of all types of mail messages.
		aForward = menu.addAction(QIcon(":data/MailForward.png"), tr("Forward..."));
		menu.addSeparator();
		aDelete = menu.addAction(QIcon(":data/DeleteMail.png"), tr("Delete..."));
	}
	else
	{
		AuId containerSenderId = AuId(m_Model.data(itemIdx, Qt::UserRole+1).toUInt());
		if(!containerSenderId.IsNull())
		{
			// Only mail from players can be used to add people to the buddy list.
			if(ntype == NODE_Individual)
			{
				aAddBuddy = menu.addAction(QIcon(":data/ContactNew.png"), tr("Add Contact..."));
				aSquelch = menu.addAction(QIcon(":data/ContactBlocked.png"), tr("Block Sender..."));
				menu.addSeparator();
			}
			else if(ntype == NODE_City)
			{
				fromCityId = containerSenderId;
				fromCity = m_Model.data(itemIdx).toString();
			}

			// Enable deletion of all types of mail senders.
			aDelete = menu.addAction(QIcon(":data/DeleteMail.png"), tr("Delete..."));
		}
	}

	// On the scene client, we can issue travel orders using the mail origin information.
	QAction* goDirect = 0;
	QAction* goDirectToCoord = 0;
	QAction* goLandAtCity = 0;
	QAction* goOrbit = 0;
	QAction* goOrbitCity = 0;
	QAction* goViaWormholes = 0;
	if(g_Au.IsSceneApp() &&
			(!fromSystemId.IsNull() || !fromWorldId.IsNull() || !fromCityId.IsNull() ||
			 fromGalaxyX != 0 || fromGalaxyY != 0 || fromGalaxyZ != 0))
	{
		// We have some kind of origin data to make mission orders with.
		menu.addSeparator();

		if(!fromSystemId.IsNull())
		{
			AuMissionOrderGoToSystemDirect order1(fromSystemId, fromSystem, fromGalaxyX, fromGalaxyY, fromGalaxyZ);
			goDirect = menu.addAction(QIcon(order1.ListNameIcon()), order1.ListName());

			AuMissionOrderGoToSystemViaWormholes order2(fromSystemId, fromSystem);
			goViaWormholes = menu.addAction(QIcon(order2.ListNameIcon()), order2.ListName());
		}

		if(fromGalaxyX != 0 || fromGalaxyY != 0 || fromGalaxyZ != 0)
		{
			AuMissionOrderGoToCoordinate order(AuMissionOrderGoToCoordinate::Galaxy, fromGalaxyX, fromGalaxyY, fromGalaxyZ);
			goDirectToCoord = menu.addAction(QIcon(order.ListNameIcon()), order.ListName());
		}

		if(!fromWorldId.IsNull())
		{
			AuMissionOrderGoToOrbit order(fromWorldId, fromWorld);
			goOrbit = menu.addAction(QIcon(order.ListNameIcon()), order.ListName());
		}

		if(!fromCityId.IsNull())
		{
			AuMissionOrderGoToOrbitCity order1(fromCityId, fromCity);
			goOrbitCity = menu.addAction(QIcon(order1.ListNameIcon()), order1.ListName());

			AuMissionOrderGoToCityAnywhere order2(fromCityId, fromCity);
			goLandAtCity = menu.addAction(QIcon(order2.ListNameIcon()), order2.ListName());
		}
	}

	if(menu.isEmpty())
		return;

	QAction* action = menu.exec(QCursor::pos());
	if(!action)
		return;

	if(action == aReply)
		ReplyToMessage();
	else if(action == aForward)
		ForwardMessage();
	else if(action == aDelete)
		DeleteMessages();
	else if(action == aAddBuddy)
		AddBuddy();
	else if(action == aSquelch)
		AddSquelch();
	else if(action == goDirect)
		emit GoDirectToSystem(fromSystemId, fromSystem, AuPoint(fromGalaxyX, fromGalaxyY, fromGalaxyZ));
	else if(action == goDirectToCoord)
		emit GoDirectToCoord(AuPoint(fromGalaxyX, fromGalaxyY, fromGalaxyZ));
	else if(action == goLandAtCity)
		emit GoLandAtCity(fromCityId, fromCity);
	else if(action == goOrbit)
		emit GoOrbit(fromWorldId, fromWorld);
	else if(action == goOrbitCity)
		emit GoOrbitCity(fromCityId, fromCity);
	else if(action == goViaWormholes)
		emit GoViaWormholes(fromSystemId, fromSystem);
}

QSize AuMail::sizeHint() const
{
	return QSize(600,800);
}

void AuMail::RequestMessages()
{
	//qDebug() << "AuMail::RequestMessages";
	if(m_tMailClient)
		m_tMailClient->RequestMessages();
}

void AuMail::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/Mail/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/Mail/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/Mail/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/Mail/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);

	if(m_Mail)
	{
		const int columnCount = 2;
		for(int col=0; col<columnCount; col++)
		{
			int colwid = g_Au.Settings().readNumEntry(QString("Widget/Mail/ColumnWid%1").arg(col),-123);
			if(colwid != -123)
				m_Mail->setColumnWidth(col, colwid);
		}
	}

	// Restore splitter state.
	QByteArray splitterState;
	g_Au.Settings().readEntry("Widget/Mail/Splitter", splitterState);
	if(!splitterState.isEmpty())
		m_Splitter->restoreState(splitterState);
}

void AuMail::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/Mail/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/Mail/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/Mail/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/Mail/SizeY",sz.height());

	// Save the sizes of the columns.
	if(m_Mail)
	{
		const int columnCount = 2;
		for(int col=0; col<columnCount; col++)
			g_Au.Settings().writeEntry(QString("Widget/Mail/ColumnWid%1").arg(col), m_Mail->columnWidth(col));
	}
}

void AuMail::SplitterMoved(int /*pos*/, int /*index*/)
{
	// Save splitter state.
	QByteArray splitterState = m_Splitter->saveState();
	g_Au.Settings().writeEntry("Widget/Mail/Splitter", splitterState);
}

void AuMail::setVisible(bool visible)
{
	QWidget::setVisible(visible);

	if(visible)
		RestoreGeometry();
}

void AuMail::PersonChanged(int index)
{
	if(index >= 0 && index < 6)
	{
		// Create columns in the model now.
		m_Model.clear();
		QModelIndex rootIdx;
		m_Model.insertColumns(0,2,rootIdx);
		m_Model.setHeaderData(0, Qt::Horizontal, tr("From"));
		m_Model.setHeaderData(1, Qt::Horizontal, tr("Date"));

		// Notify the mail thread that the person id has changed/been set.
		m_idPerson = AuId(m_PlayerCombo->itemData(index).toUInt());
		emit PersonSelected(m_idPerson);
	}
}

void AuMail::UpdateModelIcons()
{
	QModelIndex rootIdx;
	if(!m_Model.rowCount(rootIdx))
		return;
	// Look for the galaxy at the root.
	int rootRows = m_Model.rowCount(rootIdx);
	for(int ir=0; ir<rootRows; ir++)
	{
		// Look for sectors below the galaxy.
		QModelIndex universeIdx = m_Model.index(ir,0,rootIdx);
		if(universeIdx.data(Qt::UserRole).toInt() != NODE_Empire)
			continue;	// Not an empire node.

		int galaxies = m_Model.rowCount(universeIdx);
		for(int igal=0; igal<galaxies; igal++)
		{
			// Look for sectors below galaxies.
			QModelIndex galaxyIdx = m_Model.index(igal,0,universeIdx);
			int galpri = 0;	// Highest sector priority in the galaxy.
			int sectors = m_Model.rowCount(galaxyIdx);
			for(int isect=0; isect<sectors; isect++)
			{
				// Look for systems below sectors.
				QModelIndex sectorIdx = m_Model.index(isect,0,galaxyIdx);
				int sectorpri = 0;	// Highest system priority in the sector.
				int systems = m_Model.rowCount(sectorIdx);
				for(int is=0; is<systems; is++)
				{
					// Look for worlds below systems.
					QModelIndex systemIdx = m_Model.index(is,0,sectorIdx);
					int systempri = 0;	// Highest world priority in the system.
					int worlds = m_Model.rowCount(systemIdx);
					for(int iw=0; iw<worlds; iw++)
					{
						// Look for cities below worlds.
						QModelIndex worldIdx = m_Model.index(iw,0,systemIdx);
						int worldpri = 0;	// Highest city priority on the world.
						int cities = m_Model.rowCount(worldIdx);
						for(int ic=0; ic<cities; ic++)
						{
							// Look for the most recent message from the city to get its priority.
							QModelIndex cityIdx = m_Model.index(ic,0,worldIdx);
							int citypri = 0;	// Highest message priority from the city.
							QDateTime newestMsg;
							int messages = m_Model.rowCount(cityIdx);
							for(int im=0; im<messages; im++)
							{
								QModelIndex messageDateIdx = m_Model.index(im,1,cityIdx);
								QDateTime msgdate = m_Model.data(messageDateIdx).toDateTime();
								if(msgdate > newestMsg || newestMsg.isNull())
								{
									newestMsg = msgdate;
									QModelIndex messageIdx = m_Model.index(im,0,cityIdx);
									citypri = m_Model.data(messageIdx, Qt::UserRole+2).toInt();
								}
							}

							// Set city icon based on highest priority message.
							if(citypri == 6)
								m_Model.setData(cityIdx, QIcon(":data/MailCityDecayed.png"), Qt::DecorationRole);
							else if(citypri == 5)
								m_Model.setData(cityIdx, QIcon(":data/MailCityDiplomacy.png"), Qt::DecorationRole);
							else if(citypri == 4)
								m_Model.setData(cityIdx, QIcon(":data/MailCityOriginOcc.png"), Qt::DecorationRole);
							else if(citypri == 3)
								m_Model.setData(cityIdx, QIcon(":data/MailCityOriginDist.png"), Qt::DecorationRole);
							else if(citypri == 2)
								m_Model.setData(cityIdx, QIcon(":data/MailCityOriginInfo.png"), Qt::DecorationRole);
							else
								m_Model.setData(cityIdx, QIcon(":data/MailCityOrigin.png"), Qt::DecorationRole);
							if(citypri > worldpri)
								worldpri = citypri;
						}
						// Set world icon based on highest priority city.
						if(worldpri == 6)
							m_Model.setData(worldIdx, QIcon(":data/WorldDecayed.png"), Qt::DecorationRole);
						else if(worldpri == 5)
							m_Model.setData(worldIdx, QIcon(":data/WorldDiplomacy.png"), Qt::DecorationRole);
						else if(worldpri == 4)
							m_Model.setData(worldIdx, QIcon(":data/WorldOcc.png"), Qt::DecorationRole);
						else if(worldpri == 3)
							m_Model.setData(worldIdx, QIcon(":data/WorldDist.png"), Qt::DecorationRole);
						else if(worldpri == 2)
							m_Model.setData(worldIdx, QIcon(":data/WorldInfo.png"), Qt::DecorationRole);
						else
							m_Model.setData(worldIdx, QIcon(":data/World.png"), Qt::DecorationRole);
						if(worldpri > systempri)
							systempri = worldpri;
					}
					// Set system icon based on highest priority world.
					if(systempri == 6)
						m_Model.setData(systemIdx, QIcon(":data/SolarSystemDecayed.png"), Qt::DecorationRole);
					else if(systempri == 5)
						m_Model.setData(systemIdx, QIcon(":data/SolarSystemDiplomacy.png"), Qt::DecorationRole);
					else if(systempri == 4)
						m_Model.setData(systemIdx, QIcon(":data/SolarSystemOcc.png"), Qt::DecorationRole);
					else if(systempri == 3)
						m_Model.setData(systemIdx, QIcon(":data/SolarSystemDist.png"), Qt::DecorationRole);
					else if(systempri == 2)
						m_Model.setData(systemIdx, QIcon(":data/SolarSystemInfo.png"), Qt::DecorationRole);
					else
						m_Model.setData(systemIdx, QIcon(":data/SolarSystem.png"), Qt::DecorationRole);
					if(systempri > sectorpri)
						sectorpri = systempri;
				}
				// Set sector icon based on highest priority system.
				if(sectorpri == 6)
					m_Model.setData(sectorIdx, QIcon(":data/SectorDecayed.png"), Qt::DecorationRole);
				else if(sectorpri == 5)
					m_Model.setData(sectorIdx, QIcon(":data/SectorDiplomacy.png"), Qt::DecorationRole);
				else if(sectorpri == 4)
					m_Model.setData(sectorIdx, QIcon(":data/SectorOcc.png"), Qt::DecorationRole);
				else if(sectorpri == 3)
					m_Model.setData(sectorIdx, QIcon(":data/SectorDist.png"), Qt::DecorationRole);
				else if(sectorpri == 2)
					m_Model.setData(sectorIdx, QIcon(":data/SectorInfo.png"), Qt::DecorationRole);
				else
					m_Model.setData(sectorIdx, QIcon(":data/Sector.png"), Qt::DecorationRole);
				if(sectorpri > galpri)
					galpri = sectorpri;
			}
			// Set galaxy icon based on highest priority sector.
			if(galpri == 6)
				m_Model.setData(galaxyIdx, QIcon(":data/GalaxyDecayed.png"), Qt::DecorationRole);
			else if(galpri == 5)
				m_Model.setData(galaxyIdx, QIcon(":data/GalaxyDiplomacy.png"), Qt::DecorationRole);
			else if(galpri == 4)
				m_Model.setData(galaxyIdx, QIcon(":data/GalaxyOcc.png"), Qt::DecorationRole);
			else if(galpri == 3)
				m_Model.setData(galaxyIdx, QIcon(":data/GalaxyDist.png"), Qt::DecorationRole);
			else if(galpri == 2)
				m_Model.setData(galaxyIdx, QIcon(":data/GalaxyInfo.png"), Qt::DecorationRole);
			else
				m_Model.setData(galaxyIdx, QIcon(":data/RangeGalaxy.png"), Qt::DecorationRole);
		}
	}
}

QModelIndex AuMail::AddTreeItem(const QString& itemName, const QIcon& icon, NODE_TYPE type, const QModelIndex& parentIdx)
{
	QModelIndex itemIdx;
	int rootRows = m_Model.rowCount(parentIdx);
	int insertAt = rootRows;									// Insertion index when existing entry not found.
	for(int ir=0; ir<rootRows; ir++)
	{
		itemIdx = m_Model.index(ir,0,parentIdx);
		QString parName = m_Model.data(itemIdx).toString();
		if(itemIdx.data(Qt::UserRole).toInt() == type && parName == itemName)
			return itemIdx;

		// Watch for the alphabetical order insert index.
		if(insertAt == rootRows && parName.localeAwareCompare(itemName) > 0)
			insertAt = ir;
	}

	// Create a new root level item to categorize messages.
	m_Model.insertRow(insertAt,parentIdx);
	itemIdx = m_Model.index(insertAt,0,parentIdx);
	m_Model.insertColumns(0,2,itemIdx);
	m_Model.setData(itemIdx, itemName);
	m_Model.setData(itemIdx, icon, Qt::DecorationRole);
	m_Model.setData(itemIdx, type, Qt::UserRole);					// So UpdateModelIcons can tell the type of this node.
	m_Model.setData(itemIdx, 0, Qt::UserRole+1);					// Id of sender of messages contained within this node.
	m_Model.setData(itemIdx, 0, Qt::UserRole+2);					// Priority when this is a mail message node.
	return itemIdx;
}

QModelIndex AuMail::AddTreeItem(const QString& itemName, const QIcon& icon, NODE_TYPE type, const QModelIndex& parentIdx,
		const AuId& fromSystemId, const QString& fromSystem, const AuId& fromWorldId, const QString& fromWorld,
		float fromGalaxyX, float fromGalaxyY, float fromGalaxyZ)
{
	QModelIndex itemIdx;
	int rootRows = m_Model.rowCount(parentIdx);
	int insertAt = rootRows;	// Insertion index when existing entry not found.
	for(int ir=0; ir<rootRows; ir++)
	{
		itemIdx = m_Model.index(ir,0,parentIdx);
		QString parName = m_Model.data(itemIdx).toString();
		if(itemIdx.data(Qt::UserRole).toInt() == type && parName == itemName)
		{
			// Optional origin data, some or all of it may be 0.
			if(!fromSystemId.IsNull())
			{
				m_Model.setData(itemIdx, fromSystemId.GetID(), Qt::UserRole+3);
				m_Model.setData(itemIdx, fromSystem, Qt::UserRole+4);
			}
			if(!fromWorldId.IsNull())
			{
				m_Model.setData(itemIdx, fromWorldId.GetID(), Qt::UserRole+5);
				m_Model.setData(itemIdx, fromWorld, Qt::UserRole+6);
			}
			if(fromGalaxyX != 0 || fromGalaxyY != 0 || fromGalaxyZ != 0)
			{
				m_Model.setData(itemIdx, fromGalaxyX, Qt::UserRole+7);
				m_Model.setData(itemIdx, fromGalaxyY, Qt::UserRole+8);
				m_Model.setData(itemIdx, fromGalaxyZ, Qt::UserRole+9);
			}
			return itemIdx;
		}

		// Watch for the alphabetical order insert index.
		if(insertAt == rootRows && parName.localeAwareCompare(itemName) > 0)
			insertAt = ir;
	}

	// Create a new root level item to categorize messages.
	m_Model.insertRow(insertAt,parentIdx);
	itemIdx = m_Model.index(insertAt,0,parentIdx);
	m_Model.insertColumns(0,2,itemIdx);
	m_Model.setData(itemIdx, itemName);
	m_Model.setData(itemIdx, icon, Qt::DecorationRole);
	m_Model.setData(itemIdx, type, Qt::UserRole);					// So UpdateModelIcons can tell the type of this node.
	m_Model.setData(itemIdx, 0, Qt::UserRole+1);					// Id of sender of messages contained within this node.
	m_Model.setData(itemIdx, 0, Qt::UserRole+2);					// Priority when this is a mail message node.

	// Optional origin data, some or all of it may be 0.
	m_Model.setData(itemIdx, fromSystemId.GetID(), Qt::UserRole+3);
	m_Model.setData(itemIdx, fromSystem, Qt::UserRole+4);
	m_Model.setData(itemIdx, fromWorldId.GetID(), Qt::UserRole+5);
	m_Model.setData(itemIdx, fromWorld, Qt::UserRole+6);
	m_Model.setData(itemIdx, fromGalaxyX, Qt::UserRole+7);
	m_Model.setData(itemIdx, fromGalaxyY, Qt::UserRole+8);
	m_Model.setData(itemIdx, fromGalaxyZ, Qt::UserRole+9);
	return itemIdx;
}

void AuMail::ReplyToMessage()
{
	QModelIndex itemIdx = m_Mail->currentIndex();
	QModelIndex col0Idx = itemIdx.sibling(itemIdx.row(),0);
	NODE_TYPE ntype = (NODE_TYPE)m_Model.data(col0Idx, Qt::UserRole).toInt();
	if(ntype != NODE_MailMessage)
		return;

	// Get the mail message id.
	unsigned int mailMsgId = m_Model.data(col0Idx, Qt::UserRole+1).toUInt();
	if(mailMsgId == 0)
		return;

	// A single message is selected.
	AuMailMsg msg;
	bool foundMsg = DtMailClient::LoadMessage(mailMsgId, msg);
	if(foundMsg && (msg.Type() == AuMailMsg::MSG_Mail || msg.Type() == AuMailMsg::MSG_CityDiplomaticMessage))
	{
		m_idMessage = msg.m_id;
		QString subject, findRE = msg.m_subject.left(4);
		if(findRE == "RE: ")
			subject = msg.m_subject;	// don't put the RE: on if there already is one.
		else
			subject = QString("RE: ") + msg.m_subject;

		QList<AuContact> qlContacts;
		AuContact contact(msg.m_idFrom, msg.m_nameFrom);
		qlContacts.append(contact);
		AuMailNew* mail = new AuMailNew(this, m_idPerson, &qlContacts, msg.m_nameFrom, subject, "");
		mail->show();
	}
}

void AuMail::ForwardMessage()
{
	QModelIndex itemIdx = m_Mail->currentIndex();
	QModelIndex col0Idx = itemIdx.sibling(itemIdx.row(),0);
	NODE_TYPE ntype = (NODE_TYPE)m_Model.data(col0Idx, Qt::UserRole).toInt();
	if(ntype != NODE_MailMessage)
		return;

	// Get the mail message id.
	unsigned int mailMsgId = m_Model.data(col0Idx, Qt::UserRole+1).toUInt();
	if(mailMsgId == 0)
		return;

	// A single message is selected.
	AuMailMsg msg;
	bool foundMsg = DtMailClient::LoadMessage(mailMsgId, msg);
	if(foundMsg)
	{
		m_idMessage = msg.m_id;
		QString subject, body, findFW = msg.m_subject.left(4);
		QString msgBody = msg.m_body;	// Leave UTC: strings for decoding by final recipient. g_Au.DecodeUTCStrings(msg.m_body);
		if(findFW == "FW: ")
		{
			subject = msg.m_subject;	// don't put the FW: on if there already is one.
			body = msgBody;				// and only the first guy to forward gets the credit for starting it
		}
		else
		{
			subject = "FW: " + msg.m_subject;
			body = tr("<b>Forwarded From: %1</b><hr>").arg(msg.m_nameFrom) + msgBody;
		}

		QList<AuContact> qlContacts;
		AuContact contact(msg.m_idFrom, msg.m_nameFrom);
		qlContacts.append(contact);
		AuMailNew* mail = new AuMailNew(this, m_idPerson, &qlContacts, msg.m_nameFrom, subject, body);
		mail->show();
	}
}

void AuMail::AddBuddy()
{
	QModelIndex itemIdx = m_Mail->currentIndex();
	QModelIndex col0Idx = itemIdx.sibling(itemIdx.row(),0);
	NODE_TYPE ntype = (NODE_TYPE)m_Model.data(col0Idx, Qt::UserRole).toInt();
	AuId senderId;
	QString senderName;
	if(ntype == NODE_MailMessage)
	{
		// Get the mail message id.
		unsigned int mailMsgId = m_Model.data(col0Idx, Qt::UserRole+1).toUInt();
		if(mailMsgId == 0)
			return;

		AuMailMsg msg;
		bool foundMsg = DtMailClient::LoadMessage(mailMsgId, msg);
		if(foundMsg && (msg.Type() == AuMailMsg::MSG_Mail || msg.Type() == AuMailMsg::MSG_CityDiplomaticMessage))
		{
			senderId = msg.m_idFrom;
			senderName = msg.m_nameFrom;
		}
	}
	else if(ntype == NODE_Individual)
	{
		// Get the sender id and name from the container.
		senderId = AuId(m_Model.data(col0Idx, Qt::UserRole+1).toUInt());
		senderName = m_Model.data(col0Idx).toString();
	}

	if(senderId.IsNull() || senderName.isEmpty())
		return;

	// First check to see if this contact is already in the list, don't send the update if it exists
	const DgData& gd = g_Au.LockDataForRead();
	bool exists=false;
	for(QList<AuContact>::const_iterator it = gd.GetContacts().begin(); it != gd.GetContacts().end(); it++)
	{
		if((*it).m_id == senderId)
		{
			exists = true;	// don't care whether it is a buddy or squelchy. if in the list, don't add again.
			break;
		}
	}
	g_Au.UnlockData();

	if(!exists)	// not in the list, send it to the database
	{
		AuContact contact;
		contact.m_id = senderId;
		contact.m_name = senderName;
		contact.m_bIgnore = false;

		const DgData& gd = g_Au.LockDataForRead();
		QString sAcctName = gd.GetAcctName();
		QByteArray sAcctPassword = gd.GetAcctPassCrypt();
		g_Au.UnlockData();

		if(!g_Au.IsSceneSolo())
		{
			AuLoginComm login;
			login.UpdateContact(sAcctName, sAcctPassword, contact, false);
		}

		DgData& gdwrite = g_Au.LockDataForWrite();
		gdwrite.AddContact(contact);
		g_Au.UnlockData();

		QMessageBox::information(this, g_Au.ApplicationName(), tr("%1 was added to your contact list.").arg(senderName));
	}
	else
	{
		QMessageBox::warning(this, g_Au.ApplicationName(),
				tr("%1 is already in your list. They can only be listed once, as either a contact or a block.")
				.arg(senderName));
	}
}

void AuMail::AddSquelch()
{
	QModelIndex itemIdx = m_Mail->currentIndex();
	QModelIndex col0Idx = itemIdx.sibling(itemIdx.row(),0);
	NODE_TYPE ntype = (NODE_TYPE)m_Model.data(col0Idx, Qt::UserRole).toInt();
	AuId senderId;
	QString senderName;
	if(ntype == NODE_MailMessage)
	{
		// Get the mail message id.
		unsigned int mailMsgId = m_Model.data(col0Idx, Qt::UserRole+1).toUInt();
		if(mailMsgId == 0)
			return;

		AuMailMsg msg;
		bool foundMsg = DtMailClient::LoadMessage(mailMsgId, msg);
		if(foundMsg && (msg.Type() == AuMailMsg::MSG_Mail || msg.Type() == AuMailMsg::MSG_CityDiplomaticMessage))
		{
			senderId = msg.m_idFrom;
			senderName = msg.m_nameFrom;
		}
	}
	else if(ntype == NODE_Individual)
	{
		// Get the sender id and name from the container.
		senderId = AuId(m_Model.data(col0Idx, Qt::UserRole+1).toUInt());
		senderName = m_Model.data(col0Idx).toString();
	}

	if(senderId.IsNull() || senderName.isEmpty())
		return;

	// First check to see if this contact is already in the list, don't send the update if it exists
	const DgData& gd = g_Au.LockDataForRead();
	bool exists=false;
	for(QList<AuContact>::const_iterator it = gd.GetContacts().begin(); it != gd.GetContacts().end(); it++)
	{
		if((*it).m_id == senderId)
		{
			exists = true;	// don't care whether it is a buddy or squelchy. if in the list, don't add again.
			break;
		}
	}
	g_Au.UnlockData();

	if(!exists)	// not in the list, send it to the database
	{
		if(senderId == m_idPerson)
		{
			QMessageBox::warning(this, g_Au.ApplicationName(), tr("Don't block yourself."));
		}
		else
		{
			AuContact contact;
			contact.m_id = senderId;
			contact.m_name = senderName;
			contact.m_bIgnore = true;

			const DgData& gd = g_Au.LockDataForRead();
			QString sAcctName = gd.GetAcctName();
			QByteArray sAcctPassword = gd.GetAcctPassCrypt();
			g_Au.UnlockData();

			DgData& gdwrite = g_Au.LockDataForWrite();
			gdwrite.AddContact(contact);
			g_Au.UnlockData();

			if(!g_Au.IsSceneSolo())
			{
				AuLoginComm login;
				login.UpdateContact(sAcctName, sAcctPassword, contact, false);
				emit UpdateSquelch(contact.m_id, false);
			}

			QMessageBox::information(this, g_Au.ApplicationName(), tr("Messages from %1 will now be ignored.").arg(senderName));
		}
	}
	else
	{
		QMessageBox::warning(this, g_Au.ApplicationName(),
				tr("%1 is already in your list. They can only be listed once, as either a contact or a block.")
				.arg(senderName));
	}
}

void AuMail::ManageContacts()
{
	AuContacts cnt(m_idPerson, this);
	connect(&cnt, SIGNAL(UpdateSquelch(AuId, bool)), SIGNAL(UpdateSquelch(AuId, bool)));
	connect(&cnt, SIGNAL(UpdateSquelches(AuIdList, bool)), SIGNAL(UpdateSquelches(AuIdList, bool)));
	cnt.exec();
}

void AuMail::StopThreads()
{
	if(m_tMailClient)
	{
		m_tMailClient->deleteLater();
		m_tMailClient->Kill();

		AuElapsedTimer t;
		t.start();
		while(m_tMailClient->isRunning())
		{
			if(t.elapsed() > 1000)
			{
#if !defined(QT_NO_DEBUG)
				qDebug() << "D AuMail::StopThreads Waiting for the Mail thread to die.";
#endif
				t.restart();
			}
		}
		m_tMailClient = 0;
	}
}

void AuMail::SetRecipient(AuId personId)
{
	int personIndex = m_PlayerCombo->findData(personId.GetID());
	if(personIndex >= 0)
		m_PlayerCombo->setCurrentIndex(personIndex);
}

void AuMail::MailSettings()
{
	AuMailSettings set(this);
	set.exec();
}

void AuMail::MailBatchEnd()
{
	m_ActivityIndicator->setPixmap(QPixmap(":data/MailInactive.png"));

	m_BatchProgress->hide();
}

void AuMail::MailBatchStart(int count)
{
	m_ActivityIndicator->setPixmap(QPixmap(":data/MailActive0.png"));
	m_ActivityCount = 0;

	if(count > 100)
	{
		m_BatchProgress->setRange(0, count);
		m_BatchProgress->setValue(0);
		m_BatchProgress->show();
	}
}

void AuMail::AvatarsUpdated()
{
	if(!m_PlayerCombo)
		return;
	m_PlayerCombo->blockSignals(true);
	int curIdx = m_PlayerCombo->currentIndex();
	m_PlayerCombo->clear();

	const DgData& gd = g_Au.LockDataForRead();
	for(int iu=0; iu<6; iu++)
	{
		const AuId& id = gd.ClientUnitId(iu);
		if(!id.IsNull())
		{
			m_PlayerCombo->addItem(gd.ClientUnitName(id), id.GetID());
			if(id == gd.ClientUnitId())
				curIdx = iu;	// This is the character that the player logged in to.
		}
	}
	g_Au.UnlockData();

	m_PlayerCombo->setCurrentIndex(curIdx);
	m_PlayerCombo->blockSignals(false);
}

