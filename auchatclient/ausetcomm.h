//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausetcomm.h,v 1.2 2022/02/19 17:47:34 daveh Exp $
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
	QSpinBox*							m_MessageLimitSpin;
	QRadioButton*						m_NotifyAt;
	QRadioButton*						m_NotifyOff;
	QRadioButton*						m_NotifyOn;
	QSpinBox*							m_WordWrapSpin;

public:
	AuSetComm(QWidget* parent=0);
	virtual ~AuSetComm();

signals:
	void WordWrapChanged(int limit);

protected slots:
	void ChannelCheckChanged(int checkState, int channelIndex);
	void LimitMessages(int limit);
	void MessageLimitSpinChanged(int value);
	void MessageTypeCheckChanged(int checkState, int msgType);
	void NotifyChecked();
	void ScopeColorClicked(int scope);
	void SetNonStandardChannels(int checkState);
	void WordWrapChecked(int limit);
	void WordWrapSpinChanged(int value);
};

#endif // AuSetComm_H
