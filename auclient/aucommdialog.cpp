//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommdialog.cpp,v 1.8 2022/02/19 18:10:28 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucommdialog.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "auitem.h"
#include "aushipment.h"
#include "auview.h"
#include "dgdata.h"
#include <QHeaderView>
#include <QPainter>
#include <QScrollBar>

#define c_MsgSpeed	100		// ms between dequeueing messages into the table

AuCommDialog::AuCommDialog(bool isNarrator, QWidget* parent) : QTableWidget(0, 4, parent)
{
	m_MousePressItem = 0;
	m_SelectedMsg = 0;
	setCursor(QCursor(Qt::ArrowCursor));

	horizontalHeader()->hide();
	if(isNarrator)
	{
		setColumnCount(1);
		horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
	}
	else
	{
		setColumnCount(4);
		horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
		horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
		horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
		horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
	}

	setEditTriggers(QAbstractItemView::NoEditTriggers);	// read only

	// Install our custom item delegate to draw rows the way we want.
	/*
	AuCommItemDelegate* delegate = new AuCommItemDelegate(this);
	setItemDelegate(delegate);
	*/

	setSelectionBehavior(QAbstractItemView::SelectRows);

	setSelectionMode(QAbstractItemView::SingleSelection);

	setShowGrid(false);

	verticalHeader()->hide();
	verticalHeader()->setDefaultSectionSize(18);
	verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

	connect(this, SIGNAL(itemSelectionChanged()), SLOT(ItemSelectionChanged()));

	m_Timer = new QTimer(this);
	connect(m_Timer, SIGNAL(timeout()), SLOT(DequeueMessage()));
}

AuCommDialog::~AuCommDialog()
{
	if(m_SelectedMsg)
	{
		delete m_SelectedMsg;
		m_SelectedMsg = 0;
	}
}

void AuCommDialog::ChatReceived(const AuCommPacket& msg)
{
	AuAtomChat inMsg(msg);
	m_MsgQueue.enqueue(inMsg);				// queue the message so the timer can add it to the table in a half second
}

void AuCommDialog::DequeueMessage()
{
	if(!m_MsgQueue.isEmpty())
		AddMessage(m_MsgQueue.dequeue());	// dequeue the next message and add it to the table
}

void AuCommDialog::mousePressEvent(QMouseEvent* event)
{
	QTableWidget::mousePressEvent(event);
	m_MousePressItem = itemAt(event->pos());
}

void AuCommDialog::mouseReleaseEvent(QMouseEvent* event)
{
	QTableWidget::mouseReleaseEvent(event);

	if(event->button() == Qt::RightButton)
	{
		QTableWidgetItem* item = itemAt(event->pos());
		if(item && item == m_MousePressItem)
			emit ItemRightClicked(item);
	}
}

void AuCommDialog::showEvent(QShowEvent* event)
{
	QTableWidget::showEvent(event);

	if(!m_Timer->isActive())
		m_Timer->start(c_MsgSpeed);			// start the timer if this widget is visible

	if(g_AuClient.ChannelLimitMessages())
	{
		// Limit the number of messages displayed on this channel.
		if(m_MsgQueue.count() >= g_AuClient.ChannelMessageLimit())
		{
			// The queue exceeds the channel message limit so discard all currently displayed messages.
			clear();
			while(m_MsgQueue.count() > g_AuClient.ChannelMessageLimit())
				m_MsgQueue.dequeue();	// Discard oldest excess queued messages, down to our channel limit.
		}
	}

	// Dequeue all the messages in the queue into the channel control.
	while(!m_MsgQueue.isEmpty())
		AddMessage(m_MsgQueue.dequeue());
}

void AuCommDialog::hideEvent(QHideEvent* event)
{
	QTableWidget::hideEvent(event);

	// Kill the timer. It will be restarted during the next showEvent.
	if(m_Timer->isActive())
		m_Timer->stop();
}

void AuCommDialog::AddMessage(AuAtomChat inMsg)
{
	// Let the widget try to wordwrap when the app is not forcing it.
	setWordWrap(!g_AuClient.WordWrapState());

	QScrollBar* scroll = verticalScrollBar();
	int scrollValue = scroll ? scroll->value() : 0;							// current position of the vertical scroll bar
	bool scrollAtMax = scroll ? scrollValue == scroll->maximum() : false;	// true if scroll bar is currently at the max.
	bool lastRowCur = (currentRow() == rowCount()-1);	// true if the last row was the current row on entry to this function.

	// Limit the number of messages displayed on this channel.
	if(g_AuClient.ChannelLimitMessages())
		while(rowCount() >= g_AuClient.ChannelMessageLimit())
			removeRow(0);

	// Insert an empty row.
	int lastRow = rowCount();
	insertRow(lastRow);

	// Word Wrapping
	const AuCommPacket* msg = inMsg.CommPacket();
	if(!msg)
		return;													// Not likely.
	QString text;
	if(g_AuClient.WordWrapState())
	{
		// Perform enforced word wrapping.
		if(msg->Text().contains(' '))
		{
			// The text has at least one space.
			// Break it down into individual words.
			QStringList words = msg->Text().simplified().split(' ');

			// Assemble lines with line breaks.
			QString textLine;									// Current text line we are assembling.
			for(QStringList::const_iterator wi = words.constBegin(); wi != words.constEnd(); wi++)
			{
				if((*wi).isEmpty())
					continue;

				if(textLine.isEmpty())
				{
					// We have not accumulated any of the text line yet.
					if((*wi).count() >= g_AuClient.WordWrapCharacters())
					{
						// That one word is enough to use up the entire line length alotted. Some wise guy? Ikkir?
						if(!text.isEmpty())
							text += '\n';						// Add a line break before the next line.
						text += (*wi);							// Add our word as its own line. Current text line stays blank.
					}
					else
						textLine = *wi;							// Start the current text line with our word.
				}
				else
				{
					// We have accumulated some of the current text line. Do not exceed the line length.
					if(textLine.count() + 1 + (*wi).count() > g_AuClient.WordWrapCharacters())
					{
						// We must start a new line before we can add our word.
						if(!text.isEmpty())
							text += '\n';						// Add a line break before the next line.
						text += textLine;						// Add the current text line we have assembled.
						textLine = *wi;							// Start the next current text line with our word.
					}
					else
						textLine += ' ' + *wi;					// Append a space and our word onto the current text line.
				}
			}

			// We could end with a fragment of a line.
			if(!textLine.isEmpty())
			{
				if(!text.isEmpty())
					text += '\n';								// Add a line break before the next line.
				text += textLine;								// Add the current text line we have assembled.
			}
		}
		else
			text = msg->Text();									// No spaces in  the text.
	}
	else
		text = msg->Text();										// Let the widget try to wordwrap the text.

	QString toolTip;
	QTableWidgetItem* firstItem;
	if(msg->IsNarratorMessage())
	{
		firstItem = new QTableWidgetItem(QIcon(msg->Icon()), text);

		// The story instance is retrieved from the binary data of the message.
		quint32 storyInstance = msg->StoryInstance();
		firstItem->setData(Qt::UserRole+1, storyInstance);

		toolTip = tr("The message originated from the active story.");
		if(msg->Type() == AuCommPacket::NarrateTitle)
		{
			// Titles are bold.
			QFont font = firstItem->font();
			font.setBold(true);
			firstItem->setFont(font);
		}
		else if(msg->Type() == AuCommPacket::NarrateChoice)
		{
			// The choice value is retrieved from the binary data of the message.
			const AuCommNarrateChoice* msgChoice = dynamic_cast<const AuCommNarrateChoice*>(msg);
			if(msgChoice)
			{
				qint8 value = msgChoice->ChoiceId();
				firstItem->setData(Qt::UserRole+2, value);
				toolTip += tr("<br>One or more choices are offered to proceed with the story. "
						"<br><b>Double-click to select this choice.</b>");
			}
		}
		else if(msg->Type() == AuCommPacket::NarrateImage)
		{
			// The image data can live in the binary data of the message until someone double-clicks it.
			toolTip += tr("<br>A picture is attached to this message."
					"<br><b>Double-click to view the image.</b>");
		}

		setItem(lastRow, 0, firstItem);
	}
	else
	{
		// Broadcast range icon and sender name.
		// The server has updated the message with the estimated actual range, rather than the broadcast range.
		QIcon rngicon;
		if(msg->Scope() == AuCommStdChannel::Thoughts)
		{
			if(msg->Type() == AuCommPacket::ThoughtMailAlert)
			{
				rngicon = QIcon(":data/MailMessage.png");
				toolTip = tr("The thought was triggered by arrival of an urgent mail message. "
						"Check mail for the full message.");
			}
			else if(msg->FromId() == AuId(1))
			{
				rngicon = QIcon(":data/ComputerEye.png");
				toolTip = tr("The thought resonated through the minds of everyone in Hazeron simultaneously.");
			}
			else if(msg->Type() == AuCommPacket::Silverwill)
			{
				rngicon = QIcon(":data/ComputerEye.png");
				toolTip = tr("The message originated from the Silverwill bug tracker.");
			}
			else
			{
				rngicon = QIcon(":data/Brain.png");
				toolTip = tr("The thought originated in your own mind.");
			}
		}
		else if(msg->Scope() == AuCommStdChannel::Intercom)
		{
			rngicon = QIcon(":data/RangeShip.png");
			toolTip = tr("The message originated aboard the ship or vehicle you are aboard.");
		}
		else if(msg->Scope() == AuCommStdChannel::Narrator && msg->ChannelName() == AuCommStdChannel::VoiceChannelName())
		{
			rngicon = QIcon(":data/RangeClose.png");
			toolTip = tr("The message originated in audible range. It is associated with an active story.");
		}
		else switch(msg->ReceptionRange())
		{
		case AuCommStdChannel::InvalidRange:
		case AuCommStdChannel::RANGE_Count:
			break;
		case AuCommStdChannel::Close:
			if(msg->Scope() == AuCommStdChannel::Audible)
			{
				rngicon = QIcon(":data/RangeClose.png");
				toolTip = tr("The message originated in audible range.");
			}
			else
			{
				rngicon = QIcon(":data/OrderPhone.png");
				toolTip = tr("The message originated at close range.");
			}
			break;
		case AuCommStdChannel::Hail:
			rngicon = QIcon(":data/RangeHail.png");
			toolTip = tr("The message originated within hailing range.");
			break;
		case AuCommStdChannel::SolarSystem:
			rngicon = QIcon(":data/RangeSystem.png");
			toolTip = tr("The message originated in this solar system.");
			break;
		case AuCommStdChannel::Sector:
			rngicon = QIcon(":data/RangeSector.png");
			toolTip = tr("The message originated in this sector or in a neighboring sector.");
			break;
		case AuCommStdChannel::Galaxy:
			rngicon = QIcon(":data/RangeGalaxy.png");
			toolTip = tr("The message originated in a distant sector of this galaxy, "
					"farther than the neighboring sectors of this sector.");
			break;
		case AuCommStdChannel::Universe:
			rngicon = QIcon(":data/RangeUniverse.png");
			if(msg->FromGalaxy() < GALAXY_Count)
				toolTip = tr("The message originated in the %1 galaxy.").arg(GalaxyName(msg->FromGalaxy()));
			else
				toolTip = tr("The message originated in a distant galaxy of Hazeron.");
			break;
		}

		QString sFrom = msg->FromName();

		firstItem = new QTableWidgetItem(inMsg.TimeStamp().UTC().toTimeSpec(Qt::LocalTime).toString("hh:mm"));
		setItem(lastRow, 0, firstItem);

		QTableWidgetItem* itemFrom = new QTableWidgetItem(rngicon, sFrom);
		setItem(lastRow, 1, itemFrom);

		if(msg->Type() == AuCommPacket::ChatChoice)
		{
			// The story instance is retrieved from the binary data of the message.
			quint32 storyInstance = msg->StoryInstance();
			firstItem->setData(Qt::UserRole+1, storyInstance);

			const AuCommChatChoice* msgChoice = dynamic_cast<const AuCommChatChoice*>(msg);
			if(msgChoice)
			{
				qint8 choiceId = msgChoice->ChoiceId();
				firstItem->setData(Qt::UserRole+2, choiceId);
				toolTip += tr("<br>One or more choices are offered to proceed with the story."
						"<br><b>Double-click to select this choice.</b>");
			}
		}

		// Message text and one or two icons, message Type icon and a secondary icon specific to the message type.
		QTableWidgetItem* msgtyp = new QTableWidgetItem(QIcon(msg->Icon()), "");
		setItem(lastRow, 2, msgtyp);

		QTableWidgetItem* msgitem = new QTableWidgetItem(QIcon(msg->Icon2()), text);
		setItem(lastRow, 3, msgitem);

		if(!toolTip.isEmpty())
		{
#if defined(QT_NO_DEBUG)
			if(!g_Au.IsSceneSolo())	// Release build of solo client does not get to see FromId.
#endif
			{
				if(msg->Scope() != AuCommStdChannel::Thoughts && !msg->IsNarratorMessage() && !msg->IsSenderAnonymous())
					toolTip += QString("<br>From Id: <i>%1</i>").arg(msg->FromId().StringId());
			}

			itemFrom->setToolTip(QString("<html>") + toolTip + "</html>");
			msgtyp->setToolTip(QString("<html>") + toolTip + "</html>");
			msgitem->setToolTip(QString("<html>") + toolTip + "</html>");
		}
	}

	// Store the message on the item in a byte array.
	QByteArray bamsg;
	QDataStream out(&bamsg, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << (quint8)msg->Type();
	out << *msg;
	firstItem->setData(Qt::UserRole, QVariant(bamsg));

	if(!toolTip.isEmpty())
		firstItem->setToolTip(QString("<html>") + toolTip + "</html>");

	resizeColumnsToContents();
	resizeRowsToContents();

	if(scrollAtMax)
		scrollToBottom();
	if(lastRowCur)
		setCurrentCell(rowCount()-1, 0);
}

bool AuCommDialog::IsMessageSelected() const
{
	const QItemSelectionModel* ism = selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		if(!sel.isEmpty())
		{
			QModelIndex idxsel = sel.first();
			if(idxsel.isValid())
				return true;
		}
	}
	return false;
}

quint8 AuCommDialog::SelectedMessagePreBoardBerthType() const
{
	if(m_SelectedMsg)
	{
		const AuCommPreBoard* msgPreBoard = dynamic_cast<const AuCommPreBoard*>(m_SelectedMsg);
		if(msgPreBoard)
			return msgPreBoard->BerthType();
	}
	return 0;
}

quint8 AuCommDialog::SelectedMessageSystem() const
{
	if(m_SelectedMsg)
		return m_SelectedMsg->System();
	return 0;
}

QString AuCommDialog::SelectedMessageSystemDesc() const
{
	if(m_SelectedMsg)
		return m_SelectedMsg->SystemDesc();
	return QString("");
}

QString AuCommDialog::SelectedMessageText() const
{
	if(!m_SelectedMsg)
		return "";
	return m_SelectedMsg->Text();
}

quint16 AuCommDialog::SelectedMessageSystemWhich() const
{
	if(m_SelectedMsg)
		return m_SelectedMsg->SystemWhich();
	return 0;
}

quint32 AuCommDialog::SelectedMessageInstallSecs() const
{
	if(m_SelectedMsg)
	{
		const AuCommAcceptUpgradeInstall* msgAcceptUpgradeInstall = dynamic_cast<const AuCommAcceptUpgradeInstall*>(m_SelectedMsg);
		if(msgAcceptUpgradeInstall)
			return msgAcceptUpgradeInstall->InstallSeconds();

		const AuCommOfferUpgradeInstall* msgOfferUpgradeInstall = dynamic_cast<const AuCommOfferUpgradeInstall*>(m_SelectedMsg);
		if(msgOfferUpgradeInstall)
			return msgOfferUpgradeInstall->InstallSeconds();
	}
	return 0;
}

quint8 AuCommDialog::SelectedMessageMinQ() const
{
	if(m_SelectedMsg)
		return m_SelectedMsg->MinQ();
	return 0;
}

void AuCommDialog::SelectedMessageMissingParts(QList<COMMODITYID>& noQ, QList<COMMODITYID>& shortQ, QList<COMMODITYID>& noSupply,
		QList<COMMODITYID>& enough) const
{
	if(m_SelectedMsg)
	{
		const AuCommAcceptRepairAll* msgAcceptRepairAll = dynamic_cast<const AuCommAcceptRepairAll*>(m_SelectedMsg);
		if(msgAcceptRepairAll)
		{
			noQ = msgAcceptRepairAll->PartsUnavailableAtMinQ();
			shortQ = msgAcceptRepairAll->PartsShortAtMinQ();
			noSupply = msgAcceptRepairAll->PartsNone();
			enough = msgAcceptRepairAll->PartsEnough();
			return;
		}

		const AuCommAcceptRepairSystem* msgAcceptRepairSystem = dynamic_cast<const AuCommAcceptRepairSystem*>(m_SelectedMsg);
		if(msgAcceptRepairSystem)
		{
			noQ = msgAcceptRepairSystem->PartsUnavailableAtMinQ();
			shortQ = msgAcceptRepairSystem->PartsShortAtMinQ();
			noSupply = msgAcceptRepairSystem->PartsNone();
			enough = msgAcceptRepairSystem->PartsEnough();
			return;
		}

		const AuCommOfferRepairAll* msgOfferRepairAll = dynamic_cast<const AuCommOfferRepairAll*>(m_SelectedMsg);
		if(msgOfferRepairAll)
		{
			noQ = msgOfferRepairAll->PartsUnavailableAtMinQ();
			shortQ = msgOfferRepairAll->PartsShortAtMinQ();
			noSupply = msgOfferRepairAll->PartsNone();
			enough = msgOfferRepairAll->PartsEnough();
			return;
		}

		const AuCommOfferRepairSystem* msgOfferRepairSystem = dynamic_cast<const AuCommOfferRepairSystem*>(m_SelectedMsg);
		if(msgOfferRepairSystem)
		{
			noQ = msgOfferRepairSystem->PartsUnavailableAtMinQ();
			shortQ = msgOfferRepairSystem->PartsShortAtMinQ();
			noSupply = msgOfferRepairSystem->PartsNone();
			enough = msgOfferRepairSystem->PartsEnough();
			return;
		}
	}
}

quint16 AuCommDialog::SelectedMessageRepairSecs() const
{
	if(m_SelectedMsg)
	{
		const AuCommAcceptRepair* msgAcceptRepair = dynamic_cast<const AuCommAcceptRepair*>(m_SelectedMsg);
		if(msgAcceptRepair)
			return msgAcceptRepair->RepairSeconds();

		const AuCommOfferRepair* msgOfferRepair = dynamic_cast<const AuCommOfferRepair*>(m_SelectedMsg);
		if(msgOfferRepair)
			return msgOfferRepair->RepairSeconds();
	}
	return 0;
}

AuId AuCommDialog::SelectedMessageRepairShipId() const
{
	if(m_SelectedMsg)
		return m_SelectedMsg->RepairShipId();
	return AuId(0);
}

void AuCommDialog::SelectedMessageCityIds(AuIdList& cityIds) const
{
	if(m_SelectedMsg)
	{
		const AuCommCitySurrenderOffered* msgOfferCities = dynamic_cast<const AuCommCitySurrenderOffered*>(m_SelectedMsg);
		if(msgOfferCities)
			msgOfferCities->CityIds(cityIds);
	}
}

AuId AuCommDialog::SelectedMessageCityId() const
{
	if(m_SelectedMsg)
	{
		const AuCommAcceptRepair* msgAcceptRepair = dynamic_cast<const AuCommAcceptRepair*>(m_SelectedMsg);
		if(msgAcceptRepair)
			return msgAcceptRepair->CityId();

		const AuCommOfferRepair* msgOfferRepair = dynamic_cast<const AuCommOfferRepair*>(m_SelectedMsg);
		if(msgOfferRepair)
			return msgOfferRepair->CityId();
	}
	return AuId(0);
}

GALAXY AuCommDialog::SelectedMessageFromGalaxy() const
{
	if(m_SelectedMsg)
		return m_SelectedMsg->FromGalaxy();
	return GALAXY_Count;
}

AuId AuCommDialog::SelectedMessageFromId() const
{
	if(m_SelectedMsg)
		return m_SelectedMsg->FromId();
	return AuId(0);
}

// Only for HailReply message. 0 for all others.
AuId AuCommDialog::SelectedMessageFromSystem() const
{
	if(m_SelectedMsg)
	{
		const AuCommHailReply* msgReply = dynamic_cast<const AuCommHailReply*>(m_SelectedMsg);
		if(msgReply)
			return msgReply->FromSolarSystemId();
	}
	return AuId(0);
}

quint32 AuCommDialog::SelectedMessageFromEmpireId() const
{
	if(m_SelectedMsg)
		return m_SelectedMsg->FromEmpireId();
	return 0;
}

quint32 AuCommDialog::SelectedMessageToEmpireId() const
{
	if(m_SelectedMsg)
		return m_SelectedMsg->ToEmpireId();
	return 0;
}

QString AuCommDialog::SelectedMessageEmpireOfOrigin() const
{
	if(m_SelectedMsg)
		return m_SelectedMsg->EmpireOfOrigin();
	return QString("");
}

QString AuCommDialog::SelectedMessageFromName() const
{
	if(m_SelectedMsg)
		return m_SelectedMsg->FromName();
	return QString("");
}

AuCommPacket::TYPE AuCommDialog::SelectedMessageType() const
{
	if(m_SelectedMsg)
		return m_SelectedMsg->Type();
	return AuCommPacket::CommTypes;				// This is the "invalid" value.
}

AuItem* AuCommDialog::SelectedMessageItem() const
{
	if(!m_SelectedMsg)
		return 0;
	const AuItem* item = m_SelectedMsg->Item();
	if(!item)
		return 0;
	return item->Clone();
}

AuItem* AuCommDialog::SelectedMessageOfferItem() const
{
	if(!m_SelectedMsg)
		return 0;
	const AuItem* item = m_SelectedMsg->OfferItem();
	if(!item)
		return 0;
	return item->Clone();
}

COMMODITYID AuCommDialog::SelectedMessageCommodityId() const
{
	if(m_SelectedMsg)
		return m_SelectedMsg->CommodityId();
	return CID_Invalid;
}

quint32 AuCommDialog::SelectedMessageDesignId() const
{
	if(m_SelectedMsg)
		return m_SelectedMsg->DesignId();
	return 0;
}

unsigned int AuCommDialog::SelectedMessageCount() const
{
	if(m_SelectedMsg)
		return m_SelectedMsg->Count();
	return 0;
}

quint32 AuCommDialog::SelectedMessagePrice() const
{
	if(m_SelectedMsg)
		return m_SelectedMsg->Price();
	return 0;
}

double AuCommDialog::SelectedMessageEstCost() const
{
	if(m_SelectedMsg)
	{
		const AuCommOfferRefit* offerMsg = dynamic_cast<const AuCommOfferRefit*>(m_SelectedMsg);
		if(offerMsg)
			return offerMsg->EstCost();
	}
	return 0;
}

quint32 AuCommDialog::SelectedMessageEstSeconds() const
{
	if(m_SelectedMsg)
	{
		const AuCommOfferRefit* offerMsg = dynamic_cast<const AuCommOfferRefit*>(m_SelectedMsg);
		if(offerMsg)
			return offerMsg->EstSeconds();
	}
	return 0;
}

void AuCommDialog::SelectedMessageRefit(AuRefitShip& refit) const
{
	if(m_SelectedMsg)
	{
		const AuCommOfferRefit* offerMsg = dynamic_cast<const AuCommOfferRefit*>(m_SelectedMsg);
		if(offerMsg)
			refit = offerMsg->Refit();
		else
		{
			const AuCommAcceptRefit* acceptMsg = dynamic_cast<const AuCommAcceptRefit*>(m_SelectedMsg);
			if(acceptMsg)
				refit = acceptMsg->Refit();
		}
	}
}

const AuMissionOrder* AuCommDialog::SelectedMessageMissionOrder() const
{
	if(m_SelectedMsg)
	{
		const AuCommMissionOrderAdd* addOrderMsg = dynamic_cast<const AuCommMissionOrderAdd*>(m_SelectedMsg);
		if(addOrderMsg && addOrderMsg->MissionOrder())
			return addOrderMsg->MissionOrder();

		const AuCommMissionOrderInsert* insOrderMsg = dynamic_cast<const AuCommMissionOrderInsert*>(m_SelectedMsg);
		if(insOrderMsg && insOrderMsg->MissionOrder())
			return insOrderMsg->MissionOrder();

		const AuCommMissionOrderNew* newOrderMsg = dynamic_cast<const AuCommMissionOrderNew*>(m_SelectedMsg);
		if(newOrderMsg && newOrderMsg->MissionOrder())
			return newOrderMsg->MissionOrder();
	}
	return 0;
}

QString AuCommDialog::SelectedMessageMission(AuMission& mission) const
{
	mission.Clear();
	if(m_SelectedMsg)
	{
		const AuCommMissionNew* newMissionMsg = dynamic_cast<const AuCommMissionNew*>(m_SelectedMsg);
		if(newMissionMsg)
		{
			mission = newMissionMsg->Mission();
			return newMissionMsg->MissionName();
		}
	}
	return QString("");
}

quint8 AuCommDialog::SelectedMessageMissionProperty() const
{
	if(m_SelectedMsg)
	{
		const AuCommMissionProperty* propMsg = dynamic_cast<const AuCommMissionProperty*>(m_SelectedMsg);
		if(propMsg)
			return propMsg->Property();
	}
	return 0;
}

quint8 AuCommDialog::SelectedMessageMissionValue() const
{
	if(m_SelectedMsg)
	{
		const AuCommMissionProperty* propMsg = dynamic_cast<const AuCommMissionProperty*>(m_SelectedMsg);
		if(propMsg)
			return propMsg->Value();
	}
	return 0;
}

int AuCommDialog::sizeHintForRow(int row) const
{
	int baseSize = QTableWidget::sizeHintForRow(row);
	if(g_AuClient.WordWrapState())
		return baseSize;

	int margin = 32;

	QString text;
	QTableWidgetItem* rowItem = item(row,3);					// Text column for most channel messages.
	if(rowItem)
		margin += horizontalHeader()->sectionSize(0) + horizontalHeader()->sectionSize(1) + horizontalHeader()->sectionSize(2);
	else
		rowItem = item(row,0);									// Text column for narrator messages with nothing in column 3.
	if(!rowItem)
		return baseSize;
	text = rowItem->text();

	/*
	QTableWidgetItem* rowItem = item(row,0);
	if(!rowItem)
		return baseSize;

	QVariant msg = rowItem->data(Qt::UserRole);	// This is a copy of the original comm message.

	QString text;
	int margin;
	if(qVariantValue<AuCommMessage>(msg).IsNarratorMessage())
	{
		text = rowItem->data(Qt::DisplayRole).toString();
		margin = 32;
	}
	else
	{
		QTableWidgetItem* textItem = item(row, 3);
		if(!textItem)
			return baseSize;
		text = textItem->data(Qt::DisplayRole).toString();
		margin = horizontalHeader()->sectionSize(0) + horizontalHeader()->sectionSize(1)
			+ horizontalHeader()->sectionSize(2) + 32;
	}
	*/

	QPixmap pm(1,1);
	QPainter p(&pm);
	p.setFont(font());
	QRect rc = p.boundingRect(0,0, width()-margin, 1, Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap, text);
	return rc.height() + 2;
}

void AuCommDialog::resizeEvent(QResizeEvent* event)
{
	QTableWidget::resizeEvent(event);
	resizeRowsToContents();
}

void AuCommDialog::ItemSelectionChanged()
{
	//qDebug() << "AuCommDialog::ItemSelectionChanged";
	// Selection changed. Delete our copy of the previously selected message.
	if(m_SelectedMsg)
	{
		delete m_SelectedMsg;
		m_SelectedMsg = 0;
	}

	// Get the message off the currently selected item, if an item is selected.
	const QItemSelectionModel* ism = selectionModel();
	if(ism)
	{
		QModelIndexList sel = ism->selectedIndexes();
		if(!sel.isEmpty())
		{
			QModelIndex idxsel = sel.first();
			if(idxsel.isValid())
			{
				QByteArray ba = idxsel.data(Qt::UserRole).toByteArray();	// This contains a copy of the original comm message.
				QDataStream in(ba);
				in.setVersion(QDataStream::Qt_4_1);
				quint8 msgType;
				in >> msgType;
				m_SelectedMsg = AuCommPacket::CreateCommPacket((AuCommPacket::TYPE)msgType);
				if(m_SelectedMsg)
					in >> *m_SelectedMsg;
			}
		}
	}
}

QString AuCommDialog::SelectedMessageOfferEmpireOfOrigin() const
{
	if(m_SelectedMsg)
		return m_SelectedMsg->OfferEmpireOfOrigin();
	return QString();
}

QImage AuCommDialog::SelectedMessageImage() const
{
	if(m_SelectedMsg)
	{
		const AuCommChatImage* chatImg = dynamic_cast<const AuCommChatImage*>(m_SelectedMsg);
		if(chatImg)
			return chatImg->Image();
		const AuCommNarrateImage* narrImg = dynamic_cast<const AuCommNarrateImage*>(m_SelectedMsg);
		if(narrImg)
			return narrImg->Image();
	}
	return QImage();
}

bool AuCommDialog::HasMessageAbout(const QString& avatarName) const
{
	// Check messages in the queue.
	for(QQueue<AuAtomChat>::const_iterator qi = m_MsgQueue.cbegin(); qi != m_MsgQueue.cend(); qi++)
		if((*qi).CommPacket()->Text().contains(avatarName, Qt::CaseInsensitive))
			return true;

	// Check messages in the widget.
	bool hasMsg = false;
	for(int row=0; row<rowCount(); row++)
	{
		QByteArray ba = item(row,0)->data(Qt::UserRole).toByteArray();	// This contains a copy of the original comm message.
		QDataStream in(ba);
		in.setVersion(QDataStream::Qt_4_1);
		quint8 msgType;
		in >> msgType;
		AuCommPacket* msg = AuCommPacket::CreateCommPacket((AuCommPacket::TYPE)msgType);
		if(msg)
		{
			in >> *msg;

			if(msg->Text().contains(avatarName, Qt::CaseInsensitive))
				hasMsg = true;
			delete msg;
			if(hasMsg)
				break;
		}
	}
	return hasMsg;
}

bool AuCommDialog::SelectedMessageShipment(AuShipment& shipment) const
{
	if(!m_SelectedMsg)
		return false;
	if(!m_SelectedMsg->Shipment())
		return false;
	shipment = *m_SelectedMsg->Shipment();
	return true;
}

