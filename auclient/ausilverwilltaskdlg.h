//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausilverwilltaskdlg.h,v 1.10 2022/02/19 18:01:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSilverwillTaskDlg_H
#define AuSilverwillTaskDlg_H

#include "auclientlib.h"
#include "aucommstdchannel.h"
#include "audialog.h"

class AuCommChannel;
class AuId;
class QComboBox;
class QLineEdit;
class QPushButton;
class QTextEdit;

class AUCLIENT_DLL AuSilverwillTaskDlg : public AuDialog
{
	Q_OBJECT;
	QString								m_FromName;
	int									m_TaskId;				// Only valid after accept.

	// Widgets
	QComboBox*							m_Categories;
	QTextEdit*							m_Info;
	QPushButton*						m_OkButton;
	QPushButton*						m_OkReply;
	QLineEdit*							m_Origin;
	QComboBox*							m_Priority;
	QLineEdit*							m_Title;

public:
	AuSilverwillTaskDlg(const AuId& fromId, const QString& fromName, const QString& text, AuCommChannel* parent=0);
	virtual ~AuSilverwillTaskDlg();

	AuCommChannel* CommChannel();

	int Category();

	inline int TaskId() const
	{
		return m_TaskId;
	}

	QString TaskTitle() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void ListCategories(QComboBox* cb);
	void RestoreGeometry();
	void SaveGeometry();
	int CreateSilverwillTask();

protected slots:
	void CreateTask();
	void CreateTaskAndReply();
	void EnableControls();
	void ParentChanged(int index);
	void TitleChanged(const QString& text);
};

#endif // AuSilverwillTaskDlg_H
