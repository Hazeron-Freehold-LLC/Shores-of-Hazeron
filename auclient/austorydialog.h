//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austorydialog.h,v 1.9 2022/04/15 21:00:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuStoryDialog_H
#define AuStoryDialog_H

#include "audialog.h"
#include "auid.h"
#include <QIcon>
#include <QList>
#include <QMutex>

class AuCommPacket;
class QScrollArea;
class QTimer;
class QToolButton;
class QVBoxLayout;

class AuStoryMsgWidget : public QWidget
{
	Q_OBJECT;
	int									m_Decay;				// Number of times my Decay function has been called.
	AuCommPacket*						m_Msg;

	// Widgets
	QLabel*								m_DecayLabel;

public:
	AuStoryMsgWidget(const AuCommPacket& msg, QWidget* parent=0);
	AuStoryMsgWidget(const QString& title, QWidget* parent=0);
	virtual ~AuStoryMsgWidget();

	void ChoiceDisable(quint32 storyInstance, qint8 value, AuId offeredBy);

	void ChoiceHide(quint32 storyInstance, qint8 value, AuId offeredBy);

	void ChoicePickedCleanup(quint32 storyInstance, qint8 value, AuId offeredBy);

	void Decay();

	QColor DecayColor(const QColor& colorToDecay) const;

	inline bool HasBeenSeen() const
	{
		return m_Decay;
	}

protected slots:
	void ChoicePicked();
	void PlayAudio();

signals:
	void ChoicePickedForStory(quint32 storyInstance, qint8 value, AuId offeredBy);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuStoryDialog : public AuDialog
{
	Q_OBJECT;
	bool								m_AutoScroll;
	bool								m_AutoScrolling;
	QString								m_Speaker;				// Sender of the last message received.
	QString								m_SpeakerChannel;		// Channel of the last message received.
	quint32								m_StoryInstance;
	QTimer*								m_Timer;
	QIcon								m_UnseenIcon[4];

	// Widgets
	QLabel*								m_AutoLabel;
	QVBoxLayout*						m_MsgLayout;
	QScrollArea*						m_MsgScroller;
	QList<AuStoryMsgWidget*>			m_MsgWidgets;
	QMutex								m_MsgWidgetsLock;
	QToolButton*						m_UnseenButton;

public:
	AuStoryDialog(QWidget* parent=0);
	virtual ~AuStoryDialog();

public slots:
	void ChoicePickedCleanup(quint32 storyInstance, qint8 value, AuId offeredBy);
	void DisableStoryDialog();
	void MessageReceived(const AuCommPacket& msg);
	void ScrollBarChanged(int value);
	virtual void setVisible(bool visible);
	void ShowHint();
	void StartAutoScroll();
	void UpdateContents();

protected:
	void RestoreGeometry();
	void SaveGeometry();
	virtual QSize sizeHint() const;

signals:
	void ChoicePicked(quint32 storyInstance, qint8 value, AuId offeredBy);
	void HintRequested(quint32 storyInstance);
};

#endif // AuStoryDialog_H
