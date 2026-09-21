//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubioavatar.h,v 1.12 2022/04/20 16:46:40 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBioAvatar_H
#define AuBioAvatar_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"

class AuPushButtonValue;
class QComboBox;
class QLabel;
class QPushButton;
class QTimer;

class AUCLIENT_DLL AuBioAvatar : public QWidget
{
	Q_OBJECT;
private:
	QTimer*								m_timer;

	// Widgets
	QLabel*								lbempirename;
	QLabel*								lbemperorname;
	QLabel*								lbflag;
	QLabel*								lbcredit;
	QLabel*								lbimg;
	QLabel*								lbname;
	QLabel*								lbspawn;
	QLabel*								lbreputation;
	QLabel*								lbwd;
	AuPushButtonValue*					m_Avatars[6];
	QComboBox*							m_Haircut;
	QComboBox*							m_Shave;
	QPushButton*						pbrelease;
	QPushButton*						pbrename;
	QPushButton*						pbspawnrecall;

public:
	AuBioAvatar(QWidget *parent=0);
	~AuBioAvatar();

	virtual QSize sizeHint() const;

protected:
	virtual void showEvent(QShowEvent* event);

protected slots:
	void GrabSelfie();
	void HaircutChanged(int index);
	void ReleaseSlot();
	void RenameAvatar();
	void ShaveChanged(int index);
	void SwitchToAvatar(int index);
	void UpdateContents();
};

#endif // AuBioAvatar_H
