//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austorydialog.cpp,v 1.15 2022/02/19 18:17:04 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "austorydialog.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "aucommpacket.h"
#include "aunoise.h"
#include "aupicturewidget.h"
#include "austory.h"
#include "austylesheet.h"
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

AuStoryMsgWidget::AuStoryMsgWidget(const AuCommPacket& msg, QWidget* parent) : QWidget(parent)
{
	m_Decay = 0;
	m_DecayLabel = 0;
	m_Msg = msg.Clone();

	switch(msg.Type())
	{
	case AuCommPacket::ChatAudio:
		{
			QToolButton* tb = new QToolButton(this);
			tb->setIcon(QIcon(":data/Speaker.png"));
			tb->setIconSize(QSize(16,16));
			tb->setToolTip(tr("<html>"
						"Replay Audio"
						"</html>"));
			connect(tb, SIGNAL(clicked()), SLOT(PlayAudio()));

			m_DecayLabel = new QLabel(msg.Text(), this);
			m_DecayLabel->setStyleSheet(QString("color: #808080; font-size: 18px;"));
			m_DecayLabel->setWordWrap(true);

			QGridLayout* lay = new QGridLayout;
			lay->addWidget(tb, 0,0);
			lay->addWidget(m_DecayLabel, 1,0, 1,2);
			lay->setMargin(2);
			lay->setSpacing(2);
			setLayout(lay);
		}
		break;

	case AuCommPacket::NarrateAudio:
		{
			QToolButton* tb = new QToolButton(this);
			tb->setIcon(QIcon(":data/Speaker.png"));
			tb->setIconSize(QSize(16,16));
			tb->setToolTip(tr("<html>"
						"Replay Audio"
						"</html>"));
			connect(tb, SIGNAL(clicked()), SLOT(PlayAudio()));

			m_DecayLabel = new QLabel(msg.Text(), this);
			m_DecayLabel->setStyleSheet(QString("font-size: 18px;"));
			m_DecayLabel->setWordWrap(true);

			QGridLayout* lay = new QGridLayout;
			lay->addWidget(tb, 0,0);
			lay->addWidget(m_DecayLabel, 1,0, 1,2);
			lay->setMargin(2);
			lay->setSpacing(2);
			setLayout(lay);
		}
		break;

	case AuCommPacket::ChatChoice:
		{
			QPushButton* pb = new QPushButton(msg.Text(), this);
			pb->setIcon(QIcon(":data/NarrateChoice.png"));
			pb->setStyleSheet(QString("font-size: 18px;"));
			connect(pb, SIGNAL(clicked()), SLOT(ChoicePicked()));

			QVBoxLayout* lay = new QVBoxLayout;
			lay->addWidget(pb);
			lay->setMargin(2);
			lay->setSpacing(2);
			setLayout(lay);
		}
		break;

	case AuCommPacket::NarrateChoice:
		{
			QPushButton* pb = new QPushButton(msg.Text(), this);
			pb->setIcon(QIcon(":data/NarrateChoice.png"));
			pb->setStyleSheet(QString("font-size: 18px;"));
			connect(pb, SIGNAL(clicked()), SIGNAL(ChoicePicked()));

			QVBoxLayout* lay = new QVBoxLayout;
			lay->addWidget(pb);
			lay->setMargin(2);
			lay->setSpacing(2);
			setLayout(lay);
		}
		break;

	case AuCommPacket::ChatImage:
		{
			AuPictureWidget* lbimg = new AuPictureWidget(false, this);
			try
			{
				const AuCommChatImage& msgChat = dynamic_cast<const AuCommChatImage&>(msg);
				//lbimg->SetBackgroundColor(QColor(12,12,12));
				lbimg->SetImage(msgChat.Image());
				int imht = msgChat.Image().height();
				lbimg->setMinimumHeight(imht);
				lbimg->setToolTip(tr("<html>"
							"Drag to Pan<br>Wheel to Magnify"
							"</html>"));
			}
			catch(const bad_cast&)
			{
				// dynamic_cast failed
			}

			// Place small italics text centered under the picture.
			m_DecayLabel = new QLabel(msg.Text(), this);
			m_DecayLabel->setAlignment(Qt::AlignCenter);
			m_DecayLabel->setStyleSheet(QString("color: #808080; font-size: 12px; font-style: italic;"));
			m_DecayLabel->setWordWrap(true);

			QVBoxLayout* lay = new QVBoxLayout;
			lay->addWidget(lbimg);
			lay->addWidget(m_DecayLabel);
			lay->setMargin(2);
			lay->setSpacing(2);
			setLayout(lay);
		}
		break;

	case AuCommPacket::NarrateImage:
		{
			AuPictureWidget* lbimg = new AuPictureWidget(false, this);
			try
			{
				const AuCommNarrateImage& msgChat = dynamic_cast<const AuCommNarrateImage&>(msg);
				//lbimg->SetBackgroundColor(QColor(12,12,12));
				lbimg->SetImage(msgChat.Image());
				int imht = msgChat.Image().height();
				lbimg->setMinimumHeight(imht);
				lbimg->setToolTip(tr("<html>"
							"Drag to Pan<br>Wheel to Magnify"
							"</html>"));
			}
			catch(const bad_cast&)
			{
				// dynamic_cast failed
			}

			// Place small italics text centered under the picture.
			m_DecayLabel = new QLabel(msg.Text(), this);
			m_DecayLabel->setAlignment(Qt::AlignCenter);
			m_DecayLabel->setStyleSheet(QString("font-size: 12px; font-style: italic;"));
			m_DecayLabel->setWordWrap(true);

			QVBoxLayout* lay = new QVBoxLayout;
			lay->addWidget(lbimg);
			lay->addWidget(m_DecayLabel);
			lay->setMargin(2);
			lay->setSpacing(2);
			setLayout(lay);
		}
		break;

	case AuCommPacket::Narrate:
	case AuCommPacket::NarrateVideo:
		{
			m_DecayLabel = new QLabel(msg.Text(), this);
			m_DecayLabel->setStyleSheet(QString("font-size: 18px;"));
			m_DecayLabel->setWordWrap(true);

			QVBoxLayout* lay = new QVBoxLayout;
			lay->addWidget(m_DecayLabel);
			lay->setMargin(2);
			lay->setSpacing(2);
			setLayout(lay);
		}
		break;

	case AuCommPacket::NarrateTitle:
		{
			m_DecayLabel = new QLabel(msg.Text(), this);
			m_DecayLabel->setAlignment(Qt::AlignCenter);
			m_DecayLabel->setStyleSheet(QString("color: #ffffff; font-size: 24px; font-weight: bold;"));
			m_DecayLabel->setWordWrap(true);

			QVBoxLayout* lay = new QVBoxLayout;
			lay->addWidget(m_DecayLabel);
			lay->setMargin(2);
			lay->setSpacing(2);
			setLayout(lay);
		}
		break;

	default:
		if(msg.CommodityId() != CID_Invalid)
		{
			QLabel* lbimg = new QLabel(this);
			lbimg->setPixmap(QPixmap(g_Au.CommodityBitmapName(msg.CommodityId())).scaled(32,32, Qt::KeepAspectRatio,
						Qt::SmoothTransformation));

			m_DecayLabel = new QLabel(msg.Text(), this);
			m_DecayLabel->setStyleSheet(QString("color: #808080; font-size: 18px;"));
			m_DecayLabel->setWordWrap(true);

			QHBoxLayout* lay = new QHBoxLayout;
			lay->addWidget(lbimg);
			lay->addWidget(m_DecayLabel);
			lay->setMargin(2);
			lay->setSpacing(2);
			setLayout(lay);
		}
		else
		{
			m_DecayLabel = new QLabel(msg.Text(), this);
			m_DecayLabel->setStyleSheet(QString("color: #808080; font-size: 18px;"));
			m_DecayLabel->setWordWrap(true);

			QVBoxLayout* lay = new QVBoxLayout;
			lay->addWidget(m_DecayLabel);
			lay->setMargin(2);
			lay->setSpacing(2);
			setLayout(lay);
		}
		break;
	}
}

AuStoryMsgWidget::AuStoryMsgWidget(const QString& title, QWidget* parent) : QWidget(parent)
{
	m_Decay = 0;
	m_DecayLabel = 0;
	m_Msg = 0;

	QLabel* lb = new QLabel(title, this);
	lb->setStyleSheet(QString("color: #ffffff; font-size: 20px;"));
	lb->setWordWrap(true);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lb);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuStoryMsgWidget::~AuStoryMsgWidget()
{
	if(m_Msg)
		delete m_Msg;
}

void AuStoryMsgWidget::ChoicePicked()
{
	if(!m_Msg)
		return;

	// Get values needed to emit the choice signal.
	switch(m_Msg->Type())
	{
	case AuCommPacket::ChatChoice:
		{
			const AuCommChatChoice* msgChat = dynamic_cast<const AuCommChatChoice*>(m_Msg);
			if(msgChat)
				emit ChoicePickedForStory(msgChat->StoryInstance(), msgChat->ChoiceId(), msgChat->FromId());
		}
		break;

	case AuCommPacket::NarrateChoice:
		{
			const AuCommNarrateChoice* msgChat = dynamic_cast<const AuCommNarrateChoice*>(m_Msg);
			if(msgChat)
				emit ChoicePickedForStory(msgChat->StoryInstance(), msgChat->ChoiceId(), msgChat->FromId());
		}
		break;

	default:
		break;
	}
}

void AuStoryMsgWidget::ChoiceDisable(quint32 storyInstance, qint8 value, AuId offeredBy)
{
	if(!m_Msg)
		return;
	if(!isEnabled())
		return;
	if(m_Msg->FromId() != offeredBy)
		return;

	// Get values needed to emit the choice signal.
	switch(m_Msg->Type())
	{
	case AuCommPacket::ChatChoice:
		{
			const AuCommChatChoice* msgChat = dynamic_cast<const AuCommChatChoice*>(m_Msg);
			if(msgChat && msgChat->StoryInstance() == storyInstance && msgChat->ChoiceId() == value)
				setEnabled(false);
		}
		break;

	case AuCommPacket::NarrateChoice:
		{
			const AuCommNarrateChoice* msgChat = dynamic_cast<const AuCommNarrateChoice*>(m_Msg);
			if(msgChat && msgChat->StoryInstance() == storyInstance && msgChat->ChoiceId() == value)
				setEnabled(false);
		}
		break;

	default:
		break;
	}
}

void AuStoryMsgWidget::ChoiceHide(quint32 storyInstance, qint8 value, AuId offeredBy)
{
	if(!m_Msg)
		return;
	if(!isEnabled())
		return;
	if(!isVisible())
		return;
	if(m_Msg->FromId() != offeredBy)
		return;

	// Get values needed to emit the choice signal.
	switch(m_Msg->Type())
	{
	case AuCommPacket::ChatChoice:
		{
			const AuCommChatChoice* msgChat = dynamic_cast<const AuCommChatChoice*>(m_Msg);
			if(msgChat && msgChat->StoryInstance() == storyInstance && msgChat->ChoiceId() == value)
				setVisible(false);
		}
		break;

	case AuCommPacket::NarrateChoice:
		{
			const AuCommNarrateChoice* msgChat = dynamic_cast<const AuCommNarrateChoice*>(m_Msg);
			if(msgChat && msgChat->StoryInstance() == storyInstance && msgChat->ChoiceId() == value)
				setVisible(false);
		}
		break;

	default:
		break;
	}
}

void AuStoryMsgWidget::ChoicePickedCleanup(quint32 storyInstance, qint8 value, AuId offeredBy)
{
	if(!m_Msg)
		return;
	if(!isEnabled())
		return;
	if(!isVisible())
		return;
	if(m_Msg->FromId() != offeredBy)
		return;

	// Get values needed to emit the choice signal.
	switch(m_Msg->Type())
	{
	case AuCommPacket::ChatChoice:
		{
			const AuCommChatChoice* msgChat = dynamic_cast<const AuCommChatChoice*>(m_Msg);
			if(msgChat && msgChat->StoryInstance() == storyInstance)
			{
				setEnabled(false);
				setVisible(msgChat->ChoiceId() == value);
			}
		}
		break;

	case AuCommPacket::NarrateChoice:
		{
			const AuCommNarrateChoice* msgChat = dynamic_cast<const AuCommNarrateChoice*>(m_Msg);
			if(msgChat && msgChat->StoryInstance() == storyInstance)
			{
				setEnabled(false);
				setVisible(msgChat->ChoiceId() == value);
			}
		}
		break;

	default:
		break;
	}
}

QColor AuStoryMsgWidget::DecayColor(const QColor& colorToDecay) const
{
	// Decay ticks are 1/10 of a second.
	if(m_Decay < 0 || m_Decay >= 150)
		return colorToDecay;

	float x;
	if(m_Decay <= 5)
		x = (float)m_Decay / 10.f;	// Decay down from pure white (at 1) to 1/2 of the way to colorToDecay.
	else
		x = .5f;					// Hold there until we start returning the colorToDecay on entry.
	float r = AuNoiseF::Cosine_Interpolate(1, colorToDecay.redF(), x);
	float g = AuNoiseF::Cosine_Interpolate(1, colorToDecay.greenF(), x);
	float b = AuNoiseF::Cosine_Interpolate(1, colorToDecay.blueF(), x);
	QColor decayedColor;
	decayedColor.setRedF(r);
	decayedColor.setGreenF(g);
	decayedColor.setBlueF(b);
	return decayedColor;
}

void AuStoryMsgWidget::Decay()
{
	// Decay ticks are 1/10 of a second.
	if(m_Decay > 150)
		return;
	if(m_DecayLabel && m_Msg)
	{
		switch(m_Msg->Type())
		{
		case AuCommPacket::ChatChoice:
		case AuCommPacket::NarrateChoice:
			break;

		case AuCommPacket::ChatImage:
			{
				QString style = QString("color: %1; font-size: 12px; font-style: italic;")
					.arg(DecayColor(QColor("#808080")).name());
				if(m_DecayLabel->styleSheet() != style)
					m_DecayLabel->setStyleSheet(style);
			}
			break;

		case AuCommPacket::NarrateImage:
			{
				QString style = QString("color: %1; font-size: 12px; font-style: italic;")
					.arg(DecayColor(AuStyleSheet::HazeronButtonText()).name());
				if(m_DecayLabel->styleSheet() != style)
					m_DecayLabel->setStyleSheet(style);
			}
			break;

		case AuCommPacket::Narrate:
		case AuCommPacket::NarrateAudio:
		case AuCommPacket::NarrateVideo:
			{
				QString style = QString("color: %1; font-size: 18px;")
					.arg(DecayColor(AuStyleSheet::HazeronButtonText()).name());
				if(m_DecayLabel->styleSheet() != style)
					m_DecayLabel->setStyleSheet(style);
			}
			break;

		case AuCommPacket::NarrateTitle:
			break;

		default:
			{
				QString style = QString("color: %1; font-size: 18px;").arg(DecayColor(QColor("#808080")).name());
				if(m_DecayLabel->styleSheet() != style)
					m_DecayLabel->setStyleSheet(style);
			}
			break;
		}
	}
	m_Decay++;
}

void AuStoryMsgWidget::PlayAudio()
{
	if(!m_Msg)
		return;

	const AuCommNarrateAudio* msgNarrateAudio = dynamic_cast<const AuCommNarrateAudio*>(m_Msg);
	if(msgNarrateAudio)
		auApp->QueueSound(msgNarrateAudio->SoundData());
	else
	{
		const AuCommChatAudio* msgChatAudio = dynamic_cast<const AuCommChatAudio*>(m_Msg);
		if(msgChatAudio)
			auApp->QueueSound(msgChatAudio->SoundData());
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuStoryDialog::AuStoryDialog(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Story Dialog"));
	setAuDlgIcon(QIcon(":data/OrderRead.png"));
	m_AutoScroll = true;
	m_AutoScrolling = false;
	m_StoryInstance = 0;
	m_Timer = 0;

	m_UnseenIcon[0] = QIcon(":data/OrderRead.png");
	m_UnseenIcon[1] = QIcon(":data/OrderRead.png");
	m_UnseenIcon[2] = QIcon(":data/StoryClose.png");
	m_UnseenIcon[3] = QIcon(":data/OrderRead.png");

	// Build the layout.
	QWidget* msgs = new QWidget;
	m_MsgLayout = new QVBoxLayout;
	m_MsgLayout->addStretch();	// There is always a stretch at the end, to keep things packed to the top.
	msgs->setLayout(m_MsgLayout);

	m_MsgScroller = new QScrollArea;
	m_MsgScroller->setWidget(msgs);
	m_MsgScroller->setWidgetResizable(true);
	connect(m_MsgScroller->verticalScrollBar(), SIGNAL(valueChanged(int)), SLOT(ScrollBarChanged(int)));

	m_AutoLabel = new QLabel(QString("<small>%1</small>").arg(g_Au.MiddleDotSymbol()), this);	// Autoscroll is engaged.
	m_AutoLabel->setAlignment(Qt::AlignCenter);
	m_AutoLabel->setToolTip(tr("<html>"
				"Scroll to the end to resume autoscroll. "
				"</html>"));

	QToolButton* hintButton = new QToolButton(this);
	hintButton->setIcon(QIcon(":data/StoryHint.png"));
	hintButton->setToolTip(tr("<html>"
				"Requests a hint about how to proceed in the story. "
				"</html>"));
	hintButton->setIconSize(QSize(16,16));
	connect(hintButton, SIGNAL(clicked()), SLOT(ShowHint()));

	m_UnseenButton = new QToolButton(this);
	m_UnseenButton->setToolTip(tr("<html>"
				"Resume Autoscroll"
				"</html>"));
	m_UnseenButton->setIconSize(QSize(16,16));
	m_UnseenButton->hide();	// visible when there are unseen messages
	connect(m_UnseenButton, SIGNAL(clicked()), SLOT(StartAutoScroll()));

	/*
	QPushButton* disableStoryDialog = new QPushButton(tr("Do Not Use Story Dialog"), this);
	disableStoryDialog->setStyleSheet(QString("font-size: 9px;"));
	disableStoryDialog->setChecked(g_AuClient.ShowStoryDialog());
	disableStoryDialog->setToolTip(tr("<html>"
				"Disables use of this story dialog. "
				"<b>Closes this window.</b> "
				"<br>"
				"Story dialog can be restored on the <i>Windows</i> page of the <i>Settings</i> window. "
				"</html>"));
	connect(disableStoryDialog, SIGNAL(clicked()), SLOT(DisableStoryDialog()));
	*/

	QHBoxLayout* blay = new QHBoxLayout;
	//blay->addWidget(disableStoryDialog);
	blay->addWidget(hintButton);
	blay->addWidget(m_AutoLabel, 1);
	blay->addWidget(m_UnseenButton);
	blay->setMargin(0);
	blay->setSpacing(0);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(m_MsgScroller);
	lay->addLayout(blay);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuStoryDialog::~AuStoryDialog()
{
}

void AuStoryDialog::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();

		// Create a timer to keep this page updated.
		if(!m_Timer)
		{
			m_Timer = new QTimer(this);
			connect(m_Timer, SIGNAL(timeout()), SLOT(UpdateContents()));
		}
		if(!m_Timer->isActive())
			m_Timer->start(100);
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

QSize AuStoryDialog::sizeHint() const
{
	return QSize(600,800);
}

void AuStoryDialog::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/StoryDialog/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/StoryDialog/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/StoryDialog/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/StoryDialog/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuStoryDialog::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/StoryDialog/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/StoryDialog/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/StoryDialog/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/StoryDialog/SizeY",sz.height());
}

void AuStoryDialog::MessageReceived(const AuCommPacket& msg)
{
	m_StoryInstance = msg.StoryInstance();

	// When adding a choice, we must disable any existing choice with the same id and story instance.
	switch(msg.Type())
	{
	case AuCommPacket::ChatChoice:
		try
		{
			const AuCommChatChoice& msgChat = dynamic_cast<const AuCommChatChoice&>(msg);
			m_MsgWidgetsLock.lock();
			for(QList<AuStoryMsgWidget*>::iterator wi = m_MsgWidgets.begin(); wi != m_MsgWidgets.end(); wi++)
				(*wi)->ChoiceHide(msgChat.StoryInstance(), msgChat.ChoiceId(), msgChat.FromId());
			m_MsgWidgetsLock.unlock();
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
		break;

	case AuCommPacket::NarrateChoice:
		try
		{
			const AuCommNarrateChoice& msgChat = dynamic_cast<const AuCommNarrateChoice&>(msg);
			m_MsgWidgetsLock.lock();
			for(QList<AuStoryMsgWidget*>::iterator wi = m_MsgWidgets.begin(); wi != m_MsgWidgets.end(); wi++)
				(*wi)->ChoiceHide(msgChat.StoryInstance(), msgChat.ChoiceId(), msgChat.FromId());
			m_MsgWidgetsLock.unlock();
		}
		catch(const bad_cast&)
		{
			// dynamic_cast failed
		}
		break;

	default:
		break;
	}

	// Preserve the vertical scroll bar position.
	int vpos = m_MsgScroller->verticalScrollBar()->value();

	// Remove the stretch from the end of the layout so we can add another widget.
	int layCount = m_MsgLayout->count();
	if(layCount)
	{
		// Really, there is ALWAYS a stretch at the end of this layout.
		QLayoutItem* endStretch = m_MsgLayout->takeAt(layCount-1);
		delete endStretch->widget();
		delete endStretch;
	}

	// When the current speaker changes, add a sub title like "Targos says,"
	if(msg.FromName() != m_Speaker || msg.ChannelName() != m_SpeakerChannel)
	{
		m_Speaker = msg.FromName();				// Narrator messages have a blank FromName.
		m_SpeakerChannel = msg.ChannelName();
		if(!m_Speaker.isEmpty())
		{
			if(m_SpeakerChannel == AuCommStdChannel::VoiceChannelName())
			{
				AuStoryMsgWidget* mw = new AuStoryMsgWidget(tr("%1 says,").arg(m_Speaker), this);
				m_MsgLayout->addWidget(mw);
			}
			else if(m_SpeakerChannel == AuCommStdChannel::VoiceChannelName())
			{
				AuStoryMsgWidget* mw = new AuStoryMsgWidget(tr("%1 thinks,").arg(m_Speaker), this);
				m_MsgLayout->addWidget(mw);
			}
			else
			{
				AuStoryMsgWidget* mw = new AuStoryMsgWidget(tr("%1 on %2,").arg(m_Speaker).arg(m_SpeakerChannel), this);
				m_MsgLayout->addWidget(mw);
			}
		}
	}

	quint32 scriptId = 0;
	AuStory* story = g_Au.LockStory(msg.StoryInstance());
	if(story)
		scriptId = story->ScriptId();
	g_Au.UnlockStory(msg.StoryInstance());
	if(scriptId)
	{
		QString title;
		const AuScripts* scripts = g_Au.LockScripts();
		title = scripts->ScriptTitle(scriptId);
		g_Au.UnlockScripts();
		setAuDlgTitle(title);
	}

	// Add the message.
	AuStoryMsgWidget* mw = new AuStoryMsgWidget(msg, this);
	connect(mw, SIGNAL(ChoicePickedForStory(quint32,qint8,AuId)), SIGNAL(ChoicePicked(quint32,qint8,AuId)));
	m_MsgLayout->addWidget(mw);
	m_MsgWidgetsLock.lock();
	m_MsgWidgets.append(mw);
	m_MsgWidgetsLock.unlock();

	// Add a stretch at the end of the layout so everything remains tight to the top.
	m_MsgLayout->addStretch();	// There is ALWAYS a stretch at the end of this layout.

	// Restore the vertical scroll bar position.
	m_MsgScroller->verticalScrollBar()->setValue(vpos);
}

void AuStoryDialog::ChoicePickedCleanup(quint32 storyInstance, qint8 value, AuId offeredBy)
{
	m_MsgWidgetsLock.lock();
	for(QList<AuStoryMsgWidget*>::iterator wi = m_MsgWidgets.begin(); wi != m_MsgWidgets.end(); wi++)
		(*wi)->ChoicePickedCleanup(storyInstance, value, offeredBy);
	m_MsgWidgetsLock.unlock();
}

void AuStoryDialog::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be recreated during the next showEvent.
		if(m_Timer && m_Timer->isActive())
			m_Timer->stop();
		return;
	}

	// Decay messages that are visible in the scroller window.
	QRect rcScroller = m_MsgScroller->rect();
	bool hasUnseenMsg = false;
	m_MsgWidgetsLock.lock();
	for(QList<AuStoryMsgWidget*>::iterator wi = m_MsgWidgets.begin(); wi != m_MsgWidgets.end(); wi++)
	{
		QRect rcMsg = (*wi)->rect();
		QPoint pMsg((rcMsg.left() + rcMsg.right())/2, rcMsg.top() + 15);	// Point in the rect of the message widget.
		pMsg = (*wi)->mapToGlobal(pMsg);
		pMsg = m_MsgScroller->mapFromGlobal(pMsg);							// Point moved into the space of the scroller rect.
		if(rcScroller.contains(pMsg) || (*wi)->HasBeenSeen())
			(*wi)->Decay();													// Decay visible messages and those that have been seen.
		else
			hasUnseenMsg = true;
	}
	m_MsgWidgetsLock.unlock();

	if(hasUnseenMsg)
	{
		if(!m_UnseenButton->isVisible())
			m_UnseenButton->show();
		m_UnseenButton->setIcon(m_UnseenIcon[AuTime::Now().Period(250) % 4]);
	}
	else
		m_UnseenButton->hide();

	// Autoscroll
	m_AutoScrolling = m_AutoScroll && m_MsgScroller->verticalScrollBar()->value() < m_MsgScroller->verticalScrollBar()->maximum();
	if(m_AutoScrolling)
	{
		int curval = m_MsgScroller->verticalScrollBar()->value();
		int nuval = (curval + m_MsgScroller->verticalScrollBar()->maximum())/2;
		if(nuval == curval)
			nuval = curval + 1;
		else if(nuval > curval + 10)
			nuval = m_MsgScroller->verticalScrollBar()->value() + 10;
		m_MsgScroller->blockSignals(true);
		m_MsgScroller->verticalScrollBar()->setValue(nuval);
		m_MsgScroller->blockSignals(false);
	}
}

void AuStoryDialog::ScrollBarChanged(int value)
{
	if(m_AutoScrolling)
		return;
	m_AutoScroll = value == m_MsgScroller->verticalScrollBar()->maximum();
	if(m_AutoScroll)
		m_AutoLabel->setText(QString("<small>%1</small>").arg(g_Au.MiddleDotSymbol()));	// Autoscroll is engaged.
	else
		m_AutoLabel->setText(tr("<html><small><i>Autoscroll Off</i></small></html>"));
}

void AuStoryDialog::DisableStoryDialog()
{
	g_AuClient.SetStoryDialog(false);
}

void AuStoryDialog::StartAutoScroll()
{
	m_AutoScroll = true;
	m_AutoLabel->setText(QString("<small>%1</small>").arg(g_Au.MiddleDotSymbol()));	// Autoscroll is engaged.
}

void AuStoryDialog::ShowHint()
{
	emit HintRequested(m_StoryInstance);
}

