//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucomm.cpp,v 1.13 2022/02/19 18:10:27 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucomm.h"
//
#include "auapp.h"
#include "aucommchannel.h"
#include "aucommchanneldialog.h"
#include "aucommopenchannel.h"
#include "aucommtabscolumn.h"
#include "aucommstdchannels.h"
#include <auglobal.h>
#include "auframe.h"
#include "auview.h"
#include <daatom.h>
#include <daperson.h>
#include <dasector.h>
#include <daunit.h>
#include <dgdata.h>
#include <QDesktopServices>
#include <QIcon>
#include <QStackedLayout>
#include <QToolButton>
#include <QUrl>

void AuCommDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleCommUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuComm::AuComm(QWidget* parent) : QWidget(parent)
{
	m_Pages = new QStackedLayout();

	/*
	if(auApp->IsSingleUserMode())
	{
		m_TabsCol = 0;
		m_Lay = 0;

		setLayout(m_Pages);

		// Create the voice channel.
		CreateChannel(AuCommStdChannel::VoiceChannelName(), AuCommStdChannel::Audible, false);
	}
	else
	*/
	{
		m_TabsCol = new AuCommTabsColumn(this);
		connect(m_TabsCol, SIGNAL(CurrentChanged(int)), SLOT(CurrentTabChanged(int)));
		connect(m_TabsCol, SIGNAL(TabDoubleClicked(int)), SLOT(FloatChannel(int)));
		connect(m_TabsCol, SIGNAL(TabCloseRequested(int)), SLOT(CloseChannel(int)));
		connect(m_TabsCol, SIGNAL(TabMoved(int, int)), SLOT(ChannelTabMoved(int, int)));

		QToolButton* pbstd = new QToolButton(this);
		pbstd->setIcon(QIcon(":data/CommStdChannel.png"));
		pbstd->setToolTip(tr("<b>Open Standard Channel</b><br>"
					"Opens a standard channel."));
		pbstd->setAutoRaise(true);
		connect(pbstd, SIGNAL(clicked(bool)), SLOT(OpenStdChannel()));

		QToolButton* pbnew = new QToolButton(this);
		pbnew->setIcon(QIcon(":data/CommNew.png"));
		pbnew->setToolTip(tr("<b>Open Channel</b><br>"
					"Opens a channel with a specified name. All players with the same channel open will receive messages that are "
					"broadcast within range."));
		pbnew->setAutoRaise(true);
		connect(pbnew, SIGNAL(clicked(bool)), SLOT(OpenChannel()));

		QToolButton* pbclose = new QToolButton(this);
		pbclose->setIcon(QIcon(":data/CommClose.png"));
		pbclose->setToolTip(tr("<b>Close Non-Standard Channels</b><br>"
					"Closes all channels that are not <i>standard</i> channels."));
		pbclose->setAutoRaise(true);
		connect(pbclose, SIGNAL(clicked(bool)), SLOT(CloseNamedChannels()));

		m_Lay = new QGridLayout();
		m_Lay->addWidget(pbstd, 0, 0);
		m_Lay->addWidget(pbnew, 0, 1);
		m_Lay->addWidget(pbclose, 0, 2);
		m_Lay->addWidget(m_TabsCol, 1,0, 1,5);
		m_Lay->addLayout(m_Pages, 0,5, 2,1);
		m_Lay->setMargin(2);
		m_Lay->setSpacing(2);
		setLayout(m_Lay);

		// Restore the last open channels.
		QList<AuChannel> channels;
		{
			// Read open channels data from the local settings.
			QByteArray ba;
			g_Au.Settings().readEntry("Channels", ba);

			// Extract open channels array from the byte array.
			QDataStream in(ba);
			in.setVersion(QDataStream::Qt_4_1);
			quint16 chancount;
			in >> chancount;
			AuChannel channel;
			while(chancount--)
			{
				in >> channel;

				// Ensure scope is correct for standard channels.
				// This is needed because the scope of two existing channels was changed, Log and Help.
				if(AuCommStdChannel::IsStandardChannel(channel.m_Name))
					channel.m_Scope = AuCommStdChannel::Scope(channel.m_Name);

				channels.append(channel);
			}

			if(channels.isEmpty())
			{
				QStringList defaultChannels;
				AuCommStdChannel::ListDefaultChannels(defaultChannels);
				for(QStringList::const_iterator di = defaultChannels.constBegin(); di != defaultChannels.constEnd(); di++)
					channels.append(AuChannel(*di, AuCommStdChannel::Scope(*di), ""));
			}
		}

		// Since we open each new channel in the first index position, we must reopen them from last to first to retain their order.
		for(QList<AuChannel>::const_iterator ci = channels.constEnd()-1; ci != channels.constBegin()-1; ci--)
		{
			// Don't recreate narrator channels. They will automatically appear if any narrative arrives.
			if((*ci).m_Scope == AuCommStdChannel::Narrator)
				continue;

			// Create the previously-open channel.
			AuCommChannel* channel = CreateChannel((*ci).m_Name, (*ci).m_Scope, false, (*ci).m_NarrDesc);
			if(channel)
				channel->NotifyServer();	// Notify the server that we are listening on this channel.
		}

		// Restore the selected channel tab index.
		int idx = g_Au.Settings().readNumEntry("CurrentChannel", 0);
		m_TabsCol->SetCurrentIndex(idx);
	}
}

AuComm::~AuComm()
{
	//if(!auApp->IsSingleUserMode())
	{
		int idx = m_TabsCol->CurrentIndex();
		g_Au.Settings().writeEntry("CurrentChannel", idx);

		// Pack our open channels into a blob so they are harder to tinker with.
		QByteArray ba;
		QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint16)m_OpenChannels.count();
		for(QList<AuChannel>::const_iterator ci = m_OpenChannels.constBegin(); ci != m_OpenChannels.constEnd(); ci++)
			out << *ci;
		g_Au.Settings().writeEntry("Channels", ba);
	}
}

void AuComm::CurrentTabChanged(int idx)
{
	// Select the page that corresponds to the channel tab that is now current.
	// The pages will not be in a predictable order because the user can move the tabs.
	QString channelName = m_TabsCol->TabText(idx);
	for(int pageIdx=0; pageIdx<m_Pages->count(); pageIdx++)
		if(m_Pages->widget(pageIdx)->objectName() == channelName)
		{
			m_Pages->setCurrentIndex(pageIdx);
			//FocusChatInput();	// This causes undesirable focus loss when the Voice channel isn't open and someone says "Hello".
			break;
		}

	EnableControls();
}

AuCommChannel* AuComm::Channel(const QString& name) const
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return 0;
	AuCommChannel* channel = pFrame->findChild<AuCommChannel*>(name);
	return channel;
}

AuCommChannel* AuComm::CreateChannel(const QString& name, AuCommStdChannel::SCOPE scope, bool selectNewTab, const QString& narrDesc)
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return 0;
	AuCommChannel* channel = Channel(name);
	if(channel)
	{
		qWarning() << "error: AuComm::CreateChannel name already exists" << name;
		return 0;
	}

	channel = new AuCommChannel(name, scope, this);
	if(!channel)
		return 0;
	connect(channel, SIGNAL(ChoicePicked(quint32, qint8, AuId)), SLOT(ChoicePickedInChannel(quint32, qint8, AuId)));
	connect(channel, SIGNAL(MessageSelectionChanged()), SLOT(EnableControls()));
	connect(channel, SIGNAL(Steamworks()), pFrame, SLOT(Steamworks()));
	connect(pFrame, SIGNAL(StoryChoicePickedCleanup(quint32,qint8,AuId)),
			channel, SLOT(StoryChoicePickedCleanup(quint32,qint8,AuId)));

	m_Pages->insertWidget(0, channel);

	m_OpenChannels.prepend(AuChannel(name, scope, narrDesc));

	//if(!auApp->IsSingleUserMode())
	{
		connect(channel, SIGNAL(HelpRequestAccepted(AuId, QString)), pFrame, SLOT(HelpRequestAccepted(AuId, QString)));
		connect(channel, SIGNAL(OpenChannelToSender(AuId)), SLOT(OpenChannelTo(AuId)));
		connect(this, SIGNAL(ChannelSpotTargetOrTerrain(QString, int, bool)),
				channel, SLOT(ChannelSpotTargetOrTerrain(QString, int, bool)));
		connect(auApp, SIGNAL(ServerConnected()), channel, SLOT(NotifyServer()));

		QIcon icon = AuCommStdChannel::ChannelIcon(name, scope);
		int itab = m_TabsCol->InsertTab(0, icon, name);
		m_TabsCol->SetTabShowControl(itab, 1);

		QString desc;
		if(scope != AuCommStdChannel::Narrator)
			desc = QString("<b>%1</b><br>%2").arg(name).arg(AuCommStdChannel::ChannelDescription(name, scope));
		else
			desc = QString("<b>%1</b><br>%2").arg(name).arg(narrDesc);
		desc += tr("<br>"
				"%1 Drag tab to change order. "
				"<br>"
				"%1 Click tab end to close channel. "
				"<br>"
				"%1 Double-click tab to tear off. ").arg(AuGlobal::MiddleDotSymbol());
		if(!desc.isEmpty())
			m_TabsCol->SetTabToolTip(itab, desc);

		// Select the newly added tab.
		if(selectNewTab)
		{
			m_TabsCol->SetCurrentIndex(itab);
			FocusChatInput();
		}
	}

	return channel;
}

void AuComm::ChatReceived(AuCommPacket& msg)
{
	if(msg.IsNarratorMessage())
	{
		Narrate(msg);
		return;
	}

	// Forward the message to the appropriate channel tab, if the channel's tab is open.
	AuCommChannel* channel = Channel(msg.ChannelName());
	if(channel)
		channel->ChatReceived(msg);
}

void AuComm::ListOpenChannels(QStringList& openChannelNames)
{
	openChannelNames.clear();
	for(QList<AuChannel>::const_iterator ci = m_OpenChannels.constBegin(); ci != m_OpenChannels.constEnd(); ci++)
		openChannelNames.append((*ci).m_Name);
}

void AuComm::OpenStdChannel()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;

	// Open channels list lets dialog show only channels that are not already open.
	QStringList openChannelNames;
	ListOpenChannels(openChannelNames);
	AuCommStdChannels chansel(openChannelNames, pFrame);
	int result = auApp->Frame()->ExecDialog(chansel);
	if(result == QDialog::Accepted)
	{
		QString channame = chansel.ChannelName();
		if(!channame.isEmpty())
		{
			AuCommChannel* channel = CreateChannel(channame, AuCommStdChannel::Scope(channame), true, "");
			if(channel)
				channel->NotifyServer();						// Notify the server that we are listening on this channel.
		}
	}
}

void AuComm::OpenChannel()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;

	// Open channels list lets dialog prevent opening one of those.
	QStringList openChannelNames;
	ListOpenChannels(openChannelNames);
	AuCommOpenChannel chansel("", openChannelNames, pFrame);
	int result = auApp->Frame()->ExecDialog(chansel);
	if(result == QDialog::Accepted)
	{
		QString channame = chansel.ChannelName();
		QString narrDesc = chansel.NarratorChannelDesc();
		if(!channame.isEmpty())
		{
			AuCommChannel* channel = CreateChannel(channame, AuCommStdChannel::Scope(channame), true, narrDesc);
			if(channel)
			{
				// Notify the server that we are listening on this channel.
				channel->NotifyServer();
			}
		}
	}
}

void AuComm::OpenChannelTo(AuId senderId)
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	QString personName;
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
		personName = person->Name();
	pView->UnlockSceneAtom();

	QString defChannelName = personName + "." + QVariant(g_Au.Dice().Roll(1,1000)).toString();
	QStringList reservedNames;
	AuCommStdChannel::ListStandardChannels(reservedNames, false);	// Reserve the standard channel names.
	AuCommOpenChannel chansel(defChannelName, reservedNames, pFrame);
	int result = auApp->Frame()->ExecDialog(chansel);
	if(result == QDialog::Accepted)
	{
		QString channame = chansel.ChannelName();
		if(!channame.isEmpty())
		{
			// Create the new channel if it is not already open.
			QStringList openChannelNames;
			ListOpenChannels(openChannelNames);
			if(!openChannelNames.contains(channame))
				CreateChannel(channame, AuCommStdChannel::Scope(channame), true, "");
			auApp->CommOpenChannelTo(channame, senderId);
		}
	}
}

void AuComm::CloseChannel(int tabIdx)
{
	//if(auApp->IsSingleUserMode())
		//return;

	// Locate the page.
	QString channelName;
	if(tabIdx < m_TabsCol->Count())
		channelName = m_TabsCol->TabText(tabIdx);

	// Remove the channel tab.
	if(!channelName.isEmpty())
		CloseChannel(channelName);
}

void AuComm::CloseChannel(QString channelName)
{
	//if(auApp->IsSingleUserMode())
		//return;

	// Remove the tab that corresponds to the channelName.
	for(int tabIdx=0; tabIdx<m_Pages->count(); tabIdx++)
		if(m_TabsCol->TabText(tabIdx) == channelName)
		{
			m_TabsCol->RemoveTab(tabIdx);
			break;
		}

	// Remove the page that corresponds to the channelName.
	for(int pageIdx=0; pageIdx<m_Pages->count(); pageIdx++)
	{
		AuCommChannel* page = (AuCommChannel*)m_Pages->widget(pageIdx);
		if(page->objectName() == channelName)
		{
			m_Pages->removeWidget(page);
			page->CloseChannel();
			page->deleteLater();
			break;
		}
	}

	// Remove the channelName from the lists of open channels.
	for(QList<AuChannel>::iterator ci = m_OpenChannels.begin(); ci != m_OpenChannels.end(); ci++)
		if((*ci).m_Name == channelName)
		{
			m_OpenChannels.erase(ci);
			break;
		}
}

void AuComm::CloseNamedChannels()
{
	//if(auApp->IsSingleUserMode())
		//return;

	for(int pageIdx=m_Pages->count()-1; pageIdx >= 0; pageIdx--)
	{
		AuCommChannel* page = (AuCommChannel*)m_Pages->widget(pageIdx);
		if(page && !page->IsNarrator())
		{
			// Channel page is not a narrator channel.
			QString channelName = page->objectName();
			if(!AuCommStdChannel::IsStandardChannel(channelName))
				CloseChannel(channelName);						// Close the non-standard channel.
		}
	}
}

void AuComm::EnableControls()
{
}

void AuComm::FocusChatInput()
{
	AuCommChannel* page = (AuCommChannel*)m_Pages->currentWidget();
	if(page)
		page->FocusChatInput();
}

void AuComm::Narrate(const AuCommPacket& msg)
{
	// The story title is the channel name from the narrator message.
	const QString& channelName = msg.ChannelName();
	if(channelName.isEmpty())
		return;

	AuCommChannel* channel = Channel(channelName);
	if(!channel)
	{
		// The channel has not been opened.
		// The story instance is stored in the binary data of the message.
		quint32 storyInstance = msg.StoryInstance();

		// The scene atom can provide the story brief given the story instance.
		// When we create the new channel, the story brief makes a good tool tip.
		QString brief;
		AuView* pView = auApp->Frame()->View();
		DaPerson* person;
		pView->LockSceneAtom(&person);
		if(person)
			brief = person->StoryBrief(storyInstance);
		pView->UnlockSceneAtom();

		// Create the channel.
		channel = CreateChannel(channelName, AuCommStdChannel::Narrator, true, brief);
	}
	if(channel)
		channel->ChatReceived(msg);
}

void AuComm::ChoicePickedInChannel(quint32 storyInstance, qint8 value, AuId offeredBy)
{
	emit ChoicePicked(storyInstance, value, offeredBy);
}

void AuComm::SpotTargetOrTerrain(QString targetDesc, int stanceTowardTarget, bool targetIsCivilian, bool logOnly)
{
	AuCommChannel* page;
	if(logOnly)
	{
		page = Channel(AuCommStdChannel::ThoughtsChannelName());
		if(!page)
			page = CreateChannel(AuCommStdChannel::ThoughtsChannelName(),
					AuCommStdChannel::Scope(AuCommStdChannel::ThoughtsChannelName()), false, QString(""));
		page->SpotTargetOrTerrain(targetDesc, (POLITICAL_STANCE)stanceTowardTarget, targetIsCivilian);
	}
	else
	{
		// Send it to all open/visible channels.
		emit ChannelSpotTargetOrTerrain(targetDesc, stanceTowardTarget, targetIsCivilian);
	}
}

void AuComm::SendChat(AuCommPacket& msg)
{
	AuCommChannel* channel = Channel(msg.ChannelName());
	if(!channel)
	{
		channel = CreateChannel(msg.ChannelName(), msg.Scope(), true, QString(""));
		if(!channel)
			return;

		// Notify the server that we are listening on this channel.
		channel->NotifyServer();
	}
	channel->SendChat(msg);
}

void AuComm::ChannelTabMoved(int fromIndex, int toIndex)
{
	if(fromIndex < m_OpenChannels.count())
	{
		AuChannel channelMoved = m_OpenChannels.takeAt(fromIndex);
		m_OpenChannels.insert(toIndex, channelMoved);
	}
}

void AuComm::Animate()
{
	//if(auApp->IsSingleUserMode())
		//return;

	m_TabsCol->Animate();
}

void AuComm::SensorContactsReceived(QByteArray bablips)
{
	// Send it to all pages because more than one can be open/visible. We don't know who is waiting for these. It should only be
	// one page since the blips are destined for a modal dialog that should be visible now; at least it was a short while ago.
	for(int pageIdx=0; pageIdx<m_Pages->count(); pageIdx++)
	{
		AuCommChannel* page = (AuCommChannel*)m_Pages->widget(pageIdx);
		page->SensorContactsReceived(bablips);
	}
}

void AuComm::FloatChannel(int tabIdx)
{
	//if(auApp->IsSingleUserMode())
		//return;

	// Locate the page.
	QString channelName;
	if(tabIdx < m_TabsCol->Count())
		channelName = m_TabsCol->TabText(tabIdx);
	if(channelName.isEmpty())
		return;

	// Remove the tab that corresponds to the channelName.
	for(int tabIdx=0; tabIdx<m_Pages->count(); tabIdx++)
		if(m_TabsCol->TabText(tabIdx) == channelName)
		{
			m_TabsCol->RemoveTab(tabIdx);
			break;
		}

	// Remove the page that corresponds to the channelName.
	AuCommChannel* channel = 0;
	for(int pageIdx=0; pageIdx<m_Pages->count(); pageIdx++)
	{
		AuCommChannel* page = (AuCommChannel*)m_Pages->widget(pageIdx);
		if(page->objectName() == channelName)
		{
			m_Pages->removeWidget(page);
			channel = page;
			break;
		}
	}
	if(!channel)
		return;
	channel->setVisible(true);

	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuCommChannelDialog* floater = new AuCommChannelDialog(pFrame);
	floater->SetChannelWidget(channel);
	connect(floater, SIGNAL(CloseChannel(QString)), SLOT(CloseChannel(QString)));
	connect(floater, SIGNAL(RestoreChannel(AuCommChannel*)), SLOT(RestoreChannel(AuCommChannel*)));
	floater->show();
}

void AuComm::RestoreChannel(AuCommChannel* channel)
{
	//if(auApp->IsSingleUserMode())
		//return;
	QString name(channel->objectName());
	AuCommStdChannel::SCOPE scope = channel->Scope();

	m_Pages->insertWidget(0, channel);

	int index = 0;
	QString narrDesc;
	for(QList<AuChannel>::iterator ci = m_OpenChannels.begin(); ci != m_OpenChannels.end(); ci++, index++)
		if((*ci).m_Name == name)
		{
			narrDesc = (*ci).m_NarrDesc;
			break;
		}

	QIcon icon = AuCommStdChannel::ChannelIcon(name, scope);
	int itab = m_TabsCol->InsertTab(index, icon, name);
	m_TabsCol->SetTabShowControl(itab, 1);

	QString desc;
	if(scope != AuCommStdChannel::Narrator)
		desc = QString("<b>%1</b><br>%2").arg(name).arg(AuCommStdChannel::ChannelDescription(name, scope));
	else
		desc = QString("<b>%1</b><br>%2").arg(name).arg(narrDesc);
	desc += tr("<br>"
			"%1 Drag tab to change order. "
			"<br>"
			"%1 Click tab end to close channel. "
			"<br>"
			"%1 Double-click tab to tear off. ").arg(AuGlobal::MiddleDotSymbol());
	if(!desc.isEmpty())
		m_TabsCol->SetTabToolTip(itab, desc);

	// Select the newly added tab.
	bool selectNewTab = false;
	if(selectNewTab)
	{
		m_TabsCol->SetCurrentIndex(itab);
		FocusChatInput();
	}
}

void AuComm::AddBuddy(AuId id, QString name)
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;
	if(AuCommChannel::AddContact(id, name, false))
		pView->LocalLogMessage(tr("%1 was added to my contact list, accessible on the Mail window.").arg(name));
	else
		pView->LocalLogMessage(tr("%1 was not added to my contact list. "
					"They may already be there or on my block list, accessible on the Mail window.").arg(name));
}

void AuComm::AddSquelch(AuId id, QString name)
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;
	if(AuCommChannel::AddContact(id, name, true))
		pView->LocalLogMessage(tr("%1 was added to my block list, accessible on the Mail window.").arg(name));
	else
		pView->LocalLogMessage(tr("%1 was not added to my block list. "
					"They may already be there or on my contact list, accessible on the Mail window.").arg(name));
}
