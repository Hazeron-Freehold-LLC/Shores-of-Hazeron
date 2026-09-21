//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausilverwillnotedlg.h,v 1.5 2022/02/19 18:01:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSilverwillNoteDlg_H
#define AuSilverwillNoteDlg_H

#include "auclientlib.h"
#include "aucommstdchannel.h"
#include "audialog.h"

class AuCommChannel;
class AuId;
class QComboBox;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTextEdit;

class AUCLIENT_DLL AuSilverwillNoteDlg : public AuDialog
{
	Q_OBJECT;
	QString								m_FromName;

	// Widgets
	QSpinBox*							m_IdSpin;
	QTextEdit*							m_Info;
	QPushButton*						m_OkButton;
	QPushButton*						m_OkReply;
	QLineEdit*							m_Title;

public:
	AuSilverwillNoteDlg(const QString& fromName, int taskId, const QString& taskTitle, const QString& text,
			AuCommChannel* parent=0);
	virtual ~AuSilverwillNoteDlg();

	AuCommChannel* CommChannel();

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
	bool CreateSilverwillNote();

protected slots:
	void CreateNote();
	void CreateNoteAndReply();
	void EnableControls();
};

#endif // AuSilverwillNoteDlg_H
