//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audebugmsgbits.h,v 1.4 2022/02/19 18:40:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDebugMsgBits_H
#define AuDebugMsgBits_H

#include <QDialog>

class QCheckBox;

class AuDebugMsgBits : public QDialog
{
	Q_OBJECT;
private:
	// Widgets
	QList<QCheckBox*>					m_CheckBoxes;

public:
	AuDebugMsgBits(QWidget* parent=0);
	virtual ~AuDebugMsgBits();

	quint32 DebugMsgBits() const;

public slots:
	virtual void setVisible(bool visible);
	void Ok();
	void DefaultManager();
	void DefaultMaster();
	void DefaultWorker();

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuDebugMsgBits_H
