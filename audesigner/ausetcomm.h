//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausetcomm.h,v 1.4 2022/02/19 18:22:53 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSetComm_H
#define AuSetComm_H

#include <QWidget>

class QPushButton;
class QRadioButton;
class QSpinBox;

class AuSetComm : public QWidget
{
	Q_OBJECT;
private:
	// Widgets
	QPushButton*						m_FontCloseButton;
	QPushButton*						m_FontGalaxyButton;
	QPushButton*						m_FontHailButton;
	QPushButton*						m_FontSectorButton;
	QPushButton*						m_FontSystemButton;
	QPushButton*						m_FontTitleButton;
	QPushButton*						m_FontUniverseButton;
	QSpinBox*							m_MessageLimitSpin;
	QRadioButton*						m_NotifyAt;
	QRadioButton*						m_NotifyOff;
	QRadioButton*						m_NotifyOn;
	QSpinBox*							m_WordWrapSpin;

public:
	AuSetComm(QWidget* parent=0);
	virtual ~AuSetComm();

signals:
	void MessageLimitChanged(int limit);
	void WordWrapChanged(int limit);

protected slots:
	void ChannelCheckChanged(int checkState, int channelIndex);
	void FontCloseClicked();
	void FontGalaxyClicked();
	void FontHailClicked();
	void FontSectorClicked();
	void FontSystemClicked();
	void FontTitleClicked();
	void FontUniverseClicked();
	void LimitMessages(int limit);
	void MessageLimitSpinChanged(int value);
	void MessageTypeCheckChanged(int checkState, int msgType);
	void NotifyChecked();
	void ScopeColorClicked(int scope);
	void ScreenMessageLimitChanged(int value);
	void SetNonStandardChannels(int checkState);
	void WordWrapChecked(int limit);
	void WordWrapSpinChanged(int value);
};

#endif // AuSetComm_H
