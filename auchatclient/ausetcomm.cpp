//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausetcomm.cpp,v 1.7 2022/02/19 17:48:15 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausetcomm.h"
//
#include "aucheckbox.h"
#include "auclientglobal.h"
#include "aupushbutton.h"
#include "auview.h"
#include "dgdata.h"
#include <QCheckBox>
#include <QColorDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QIcon>
#include <QLabel>
#include <QRadioButton>
#include <QScrollArea>
#include <QSpinBox>

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSetComm::AuSetComm(QWidget* parent) : QWidget(parent)
{
	const DgData& gd = g_Au.LockDataForRead();
	bool isAgent = gd.ClientPlayerIsAgent();	// also true if programmer or architect
	bool isProgrammer = gd.ClientPlayerIsProgrammer();	// also true if architect
	bool isArchitect = gd.ClientPlayerIsArchitect();
	bool isMinion = gd.ClientPlayerIsMinion();
	g_Au.UnlockData();

	QGroupBox* gbwin = new QGroupBox(tr("Comm Channel Windows"));
	{
		// Word wrap
		QCheckBox* wordWrapCheck = new QCheckBox(tr("Force Word Wrapping"));
		wordWrapCheck->setCheckState(g_AuClient.WordWrapState()?Qt::Checked:Qt::Unchecked);
		wordWrapCheck->setToolTip(tr("<html>When checked, comm channel windows will force line breaks in long messages.<br>"
					"When unchecked, comm channel windows will let the underlying control wordwrap the text, which does not work "
					"for everybody.<br>"
					"<b>This only affects messages that arrive after the setting is changed.</b></html>"));
		connect(wordWrapCheck, SIGNAL(stateChanged(int)), SLOT(WordWrapChecked(int)));

		m_WordWrapSpin = new QSpinBox(this);
		m_WordWrapSpin->setRange(40,500);
		m_WordWrapSpin->setValue(g_AuClient.WordWrapCharacters());
		m_WordWrapSpin->setEnabled(g_AuClient.WordWrapState()?true:false);
		m_WordWrapSpin->setToolTip(tr("<html>Enter the number of characters at which line breaking will be forced in messages "
					"displayed in comm channel windows."
					"</html>"));
		connect(m_WordWrapSpin, SIGNAL(valueChanged(int)), SLOT(WordWrapSpinChanged(int)));

		// channel message limit
		QCheckBox* ckLimitMessages = new QCheckBox(tr("Limit Messages In Comm Channel Windows"));
		ckLimitMessages->setCheckState(g_AuClient.ChannelLimitMessages()?Qt::Checked:Qt::Unchecked);
		ckLimitMessages->setToolTip(tr("<html>When checked, comm channels windows will limit the number of messages displayed. "
					"Older messages will be discarded when the limit is exceeded. "
					"This reduces the delay when selecting channels that have accumulated messages.<br>"
					"When unchecked, comm channels will not limit the number of messages displayed. "
					"This causes a delay when a comm channel is selected that has accumulated a lot of messages.</html>"));
		connect(ckLimitMessages, SIGNAL(stateChanged(int)), SLOT(LimitMessages(int)));

		m_MessageLimitSpin = new QSpinBox(this);
		m_MessageLimitSpin->setRange(0, 1000);
		m_MessageLimitSpin->setValue(g_AuClient.ChannelMessageLimit());
		m_MessageLimitSpin->setEnabled(g_AuClient.ChannelLimitMessages()?true:false);
		m_MessageLimitSpin->setToolTip(tr("<html>Enter the maximum number of messages that will be displayed by each comm channel "
					"window. "
					"Limit the number of messages to reduce the delay that occurs when a channel window is selected that has "
					"accumulated a lot of messages.</html>"));
		connect(m_MessageLimitSpin, SIGNAL(valueChanged(int)), SLOT(MessageLimitSpinChanged(int)));

		m_NotifyOff = new QRadioButton(tr("Color of comm channel tab indicates a message contains your avatar name."), this);
		m_NotifyOff->setChecked(g_AuClient.PopupNotify() == 0);
		m_NotifyOff->setToolTip(tr("<html>With this option, "
					"only the color of the comm channel tab indicates your avatar name is mentioned in a message."));
		connect(m_NotifyOff, SIGNAL(clicked()), SLOT(NotifyChecked()));

		m_NotifyOn = new QRadioButton(tr("Show a popup window if a new message contains your avatar name."), this);
		m_NotifyOn->setChecked(g_AuClient.PopupNotify() == 1);
		m_NotifyOn->setToolTip(tr("<html>Causes a popup window to appear when a new message arrives "
					"that contains your avatar name.<br>"
					"The popup appears in addition to a color change on the comm channel tab."));
		connect(m_NotifyOn, SIGNAL(clicked()), SLOT(NotifyChecked()));

		m_NotifyAt = new QRadioButton(tr("Show a popup window if a new message contains your avatar name preceded by @."), this);
		m_NotifyAt->setChecked(g_AuClient.PopupNotify() == 2);
		m_NotifyAt->setToolTip(tr("<html>Causes a popup window to appear when a new message arrives "
					"that contains your avatar name preceded by an @ sign, e.g. 'Hello @Your Avatar.'<br>"
					"The popup appears in addition to a color change on the comm channel tab."));
		connect(m_NotifyAt, SIGNAL(clicked()), SLOT(NotifyChecked()));

		QGridLayout* laywin = new QGridLayout;
		int row=0;
		laywin->addWidget(wordWrapCheck, row,0);
		laywin->addWidget(m_WordWrapSpin, row,1);
		laywin->addWidget(ckLimitMessages, row,3);
		laywin->addWidget(m_MessageLimitSpin, row,4);
		row++;
		laywin->addWidget(m_NotifyOff, row,0, 1,2);
		row++;
		laywin->addWidget(m_NotifyOn, row,0, 1,2);
		row++;
		laywin->addWidget(m_NotifyAt, row,0, 1,2);
		row++;
		laywin->setColumnStretch(2, 10);
		laywin->setSpacing(2);
		laywin->setMargin(12);
		gbwin->setLayout(laywin);
	}

	QGroupBox* gbon = new QGroupBox(tr("On Screen Messages"));
	{
		// Nonstandard channel messages.
		QCheckBox* cknonstandard = new QCheckBox(tr("Show Nonstandard Channel Messages"));
		cknonstandard->setToolTip(tr("<html>"
				"When checked, messages on channels that are not <i>standard</i> channels will be displayed on the screen for a "
				"short time when they arrive."
				"</html>"));
		if(g_Au.Settings().readNumEntry("OnScreenMessages/Show Non-Standard", 1))
			cknonstandard->setCheckState(Qt::Checked);
		else
			cknonstandard->setCheckState(Qt::Unchecked);
		connect(cknonstandard, SIGNAL(stateChanged(int)), SLOT(SetNonStandardChannels(int)));

		// Standard channels check boxes in a group box.
		QGroupBox* gbchan = new QGroupBox(tr("Show Standard Channel Messages"));
		{
			QWidget* channels = new QWidget();
			{
				// Make a map to alphabetize them for me.
				QStringList channelNames;
				AuCommStdChannel::ListStandardChannels(channelNames, isMinion);
				if(isAgent)
					AuCommStdChannel::ListAgentChannels(channelNames);		// Append agent channel(s) to the list.
				if(isProgrammer)
					AuCommStdChannel::ListProgrammerChannels(channelNames);	// Append programmer channel(s) to the list.
				if(isArchitect)
					AuCommStdChannel::ListArchitectChannels(channelNames);	// Append architect channel(s) to the list.
				QMap<QString,int> stdChannels;
				int nameIndex = 0;
				for(QStringList::const_iterator cni = channelNames.constBegin(); cni != channelNames.constEnd(); cni++, nameIndex++)
					stdChannels.insert(*cni, nameIndex);

				QGridLayout* clay = new QGridLayout(channels);
				int row = 0;
				int col = 0;
				for(QMap<QString,int>::const_iterator ci = stdChannels.constBegin(); ci != stdChannels.constEnd(); ci++)
				{
					QLabel* lb = new QLabel(this);
					lb->setPixmap(AuCommStdChannel::ChannelIcon(ci.key(), AuCommStdChannel::Scope(ci.key())).pixmap(16,16));

					AuCheckBoxValue* ck = new AuCheckBoxValue(ci.key(), *ci, this);
					if(g_Au.Settings().readNumEntry("OnScreenMessageChannels/"+ci.key(), 1))
						ck->setCheckState(Qt::Checked);
					else
						ck->setCheckState(Qt::Unchecked);
					connect(ck, SIGNAL(StateChanged(int, int)), SLOT(ChannelCheckChanged(int, int)));
					ck->setToolTip(tr("<html>When checked, messages on this standard channel will be displayed on the screen for a "
								"short time when they arrive.</html>"));

					col=0;
					clay->addWidget(lb, row,col++);
					clay->addWidget(ck, row++,col++);
				}
				clay->setColumnStretch(col, 1);

				clay->setMargin(2);
				clay->setSpacing(2);
				channels->setLayout(clay);
			}
			QScrollArea* chanscroll = new QScrollArea;
			chanscroll->setWidget(channels);

			QVBoxLayout* chanlay = new QVBoxLayout;
			chanlay->addWidget(chanscroll);
			chanlay->setSpacing(2);
			chanlay->setMargin(12);
			gbchan->setLayout(chanlay);
		}

		// Message Scope Colors
		QGroupBox* gbcolors = new QGroupBox(tr("Colors"));
		{
			QWidget* scopes = new QWidget();
			{
				// Make a map to alphabetize them for me.
				QMap<QString,AuCommStdChannel::SCOPE> msgScopes;
				for(int msgScope=0; msgScope < AuCommStdChannel::ScopeTypes; msgScope++)
				{
					if(msgScope == AuCommStdChannel::Architect && !isArchitect)
						continue;
					if(msgScope == AuCommStdChannel::Programmer && !(isArchitect || isProgrammer))
						continue;
					if(msgScope == AuCommStdChannel::Agent && !(isArchitect || isProgrammer || isAgent))
						continue;
					msgScopes.insert(AuCommStdChannel::ScopeName((AuCommStdChannel::SCOPE)msgScope),
							(AuCommStdChannel::SCOPE)msgScope);
				}

				QGridLayout* tlay = new QGridLayout(scopes);
				int row = 0;
				int col = 0;
				for(QMap<QString, AuCommStdChannel::SCOPE>::const_iterator ti = msgScopes.constBegin(); ti != msgScopes.constEnd(); ti++)
				{
					if(ti.key().isEmpty())
						continue;

					AuPushButtonValue* pb = new AuPushButtonValue(ti.key(), *ti, this);
					pb->setObjectName(QString("ScopeColor%1").arg(*ti));
					QColor color = g_Au.Settings().readColorEntry(QString("OnScreenMessageColors/%1").arg(*ti),
							AuCommStdChannel::ScopeColor(*ti));
					pb->setAutoFillBackground(true);
					pb->setStyleSheet(QString("background-color: rgb(0, 0, 0); color: rgb(%1, %2, %3)")
							.arg(color.red()).arg(color.green()).arg(color.blue()));
					if(*ti == AuCommStdChannel::Thoughts || *ti == AuCommStdChannel::Intercom)
						pb->setToolTip(tr("<html>Color of %1 messages when displayed on the screen.<br><br>"
									"<b>Spot</b> messages ignore this setting. "
									"They are colored according to political stance toward the target.<br><br>"
									"The %1 color chooser window starts with two custom colors. "
									"The first custom color is the default color. "
									"The second custom color is the current color."
									"</html>").arg(ti.key()));
					else
						pb->setToolTip(tr("<html>Color of %1 messages when displayed on the screen. "
									"Color dims as distance increases from the message sender.<br><br>"
									"<b>Spot</b> messages ignore this setting. "
									"They are colored according to political stance toward the target.<br><br>"
									"The %1 color chooser window starts with two custom colors. "
									"The first custom color is the default color. "
									"The second custom color is the current color."
									"</html>").arg(ti.key()));
					connect(pb, SIGNAL(ClickedValue(int)), SLOT(ScopeColorClicked(int)));

					col=0;
					tlay->addWidget(pb, row,col++);
					row++;
				}

				tlay->setMargin(2);
				tlay->setSpacing(2);
				scopes->setLayout(tlay);
			}
			QScrollArea* scopescroll = new QScrollArea;
			scopescroll->setWidget(scopes);

			QVBoxLayout* colorlay = new QVBoxLayout;
			colorlay->addWidget(scopescroll);
			colorlay->setSpacing(2);
			colorlay->setMargin(12);
			gbcolors->setLayout(colorlay);
		}

		QGroupBox* gbtype = new QGroupBox(tr("Show Message Types"));
		{
			// Message Types
			QWidget* types = new QWidget();
			{
				// Make a map to alphabetize them for me.
				QMap<QString,AuCommPacket::TYPE> msgTypes;
				for(int msgType=0; msgType < AuCommPacket::CommTypes; msgType++)
					msgTypes.insert(AuCommPacket::TypeName((AuCommPacket::TYPE)msgType), (AuCommPacket::TYPE)msgType);

				QGridLayout* tlay = new QGridLayout(types);
				int row = 0;
				int col = 0;
				for(QMap<QString, AuCommPacket::TYPE>::const_iterator ti = msgTypes.constBegin(); ti != msgTypes.constEnd(); ti++)
				{
					if(ti.key().isEmpty())
						continue;

					QLabel* lb = new QLabel;
					lb->setPixmap(QPixmap(AuCommPacket::Icon(*ti)).scaled(16,16, Qt::KeepAspectRatio, Qt::SmoothTransformation));

					AuCheckBoxValue* ck = new AuCheckBoxValue(ti.key(), *ti, this);
					if(g_Au.Settings().readNumEntry(QString("OnScreenMessageType/%1").arg(*ti),
								AuCommPacket::OnScreenByDefault(*ti)))
						ck->setCheckState(Qt::Checked);
					else
						ck->setCheckState(Qt::Unchecked);
					connect(ck, SIGNAL(StateChanged(int, int)), SLOT(MessageTypeCheckChanged(int, int)));
					ck->setToolTip(tr("<html>When checked, messages of this type will be displayed on the screen for a short "
							"time when they arrive.</html>"));

					col=0;
					tlay->addWidget(lb, row,col++);
					tlay->addWidget(ck, row++,col++);
				}
				tlay->setColumnStretch(col, 1);

				tlay->setMargin(2);
				tlay->setSpacing(2);
				types->setLayout(tlay);
			}
			QScrollArea* typescroll = new QScrollArea;
			typescroll->setWidget(types);

			QVBoxLayout* typelay = new QVBoxLayout;
			typelay->addWidget(typescroll);
			typelay->setSpacing(2);
			typelay->setMargin(12);
			gbtype->setLayout(typelay);
		}

		QHBoxLayout* layo = new QHBoxLayout;
		layo->addWidget(cknonstandard);
		layo->addStretch();

		QGridLayout* layon = new QGridLayout;
		int row=0;
		layon->addLayout(layo, row,0, 1,3);
		row++;
		layon->addWidget(gbchan, row,0, 2,1);
		layon->addWidget(gbtype, row,1, 2,1);
		layon->addWidget(gbcolors, row,2, 2,1);
		layon->setRowStretch(row,1);
		layon->setColumnStretch(1,2);
		layon->setColumnStretch(2,1);
		layon->setSpacing(2);
		layon->setMargin(12);
		gbon->setLayout(layon);
	}

	// Build the overall layout.
	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(gbwin);
	lay->addWidget(gbon);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuSetComm::~AuSetComm()
{
}

void AuSetComm::NotifyChecked()
{
	int notify;
	if(m_NotifyOff->isChecked())
		notify = 0;
	else if(m_NotifyOn->isChecked())
		notify = 1;
	else
		notify = 2;
	g_AuClient.SetPopupNotify(notify);
}

void AuSetComm::WordWrapChecked(int limit)
{
	g_AuClient.SetWordWrapState(limit?true:false);
	m_WordWrapSpin->setEnabled(g_AuClient.WordWrapState()?true:false);
}

void AuSetComm::WordWrapSpinChanged(int value)
{
	g_AuClient.SetWordWrapCharacters(value);
	emit WordWrapChanged(value);
}

void AuSetComm::LimitMessages(int limit)
{
	g_AuClient.SetChannelLimitMessages(limit?true:false);
	m_MessageLimitSpin->setEnabled(g_AuClient.ChannelLimitMessages()?true:false);
}

void AuSetComm::MessageLimitSpinChanged(int value)
{
	g_AuClient.SetChannelMessageLimit(value);
}

void AuSetComm::SetNonStandardChannels(int checkState)
{
	g_Au.Settings().writeEntry("OnScreenMessages/Show Non-Standard", (checkState == Qt::Checked)?1:0);
}

void AuSetComm::ChannelCheckChanged(int checkState, int channelIndex)
{
	if(channelIndex < 0)
		return;

	const DgData& gd = g_Au.LockDataForRead();
	bool isAgent = gd.ClientPlayerIsAgent();	// also true if programmer or architect
	bool isProgrammer = gd.ClientPlayerIsProgrammer();	// also true if architect
	bool isArchitect = gd.ClientPlayerIsArchitect();
	bool isMinion = gd.ClientPlayerIsMinion();
	g_Au.UnlockData();

	// Standard Channels
	QStringList channelNames;
	AuCommStdChannel::ListStandardChannels(channelNames, isMinion);
	if(isAgent)
		AuCommStdChannel::ListAgentChannels(channelNames);		// Append agent channel(s) to the list.
	if(isProgrammer)
		AuCommStdChannel::ListProgrammerChannels(channelNames);	// Append programmer channel(s) to the list.
	if(isArchitect)
		AuCommStdChannel::ListArchitectChannels(channelNames);	// Append architect channel(s) to the list.

	if(channelIndex < channelNames.count())
		g_Au.Settings().writeEntry("OnScreenMessageChannels/" + channelNames[channelIndex], (checkState == Qt::Checked)?1:0);
}

void AuSetComm::MessageTypeCheckChanged(int checkState, int msgType)
{
	if(msgType < AuCommPacket::Accept || msgType >= AuCommPacket::CommTypes)
		return;

	g_Au.Settings().writeEntry(QString("OnScreenMessageType/%1").arg(msgType), (checkState == Qt::Checked)?1:0);
}

void AuSetComm::ScopeColorClicked(int scope)
{
	QColor color = AuCommStdChannel::ScopeColor((AuCommStdChannel::SCOPE)scope);
	QColorDialog::setCustomColor(0, color.rgb());				// First custom color is the default scope color.
	color = g_Au.Settings().readColorEntry(QString("OnScreenMessageColors/%1").arg(scope), color);
	QColorDialog::setCustomColor(1, color.rgb());				// Second custom color is the current scope color.
	color = QColorDialog::getColor(color, this,
			tr("%1 Message Color").arg(AuCommStdChannel::ScopeName((AuCommStdChannel::SCOPE)scope)),
			g_Au.UseNativeColorDialog() ? QColorDialog::ColorDialogOptions() : QColorDialog::DontUseNativeDialog);
	if(color.isValid())
	{
		g_Au.Settings().writeEntry(QString("OnScreenMessageColors/%1").arg(scope), color);
		AuPushButtonValue* pb = findChild<AuPushButtonValue*>(QString("ScopeColor%1").arg(scope));
		pb->setStyleSheet(QString("background-color: rgb(0, 0, 0); color: rgb(%1, %2, %3)")
				.arg(color.red()).arg(color.green()).arg(color.blue()));
	}
}
