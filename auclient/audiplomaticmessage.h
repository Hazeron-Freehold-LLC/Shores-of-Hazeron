//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audiplomaticmessage.h,v 1.6 2022/02/19 17:55:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuDiplomaticMessage_H
#define AuDiplomaticMessage_H

#include "auclientlib.h"
#include "audialog.h"

class QLineEdit;

class AUCLIENT_DLL AuDiplomaticMessage : public AuDialog
{
	Q_OBJECT;
private:
	// Widgets
	QLineEdit*							m_MessageEdit;

public:
	AuDiplomaticMessage(QWidget* parent=0);
	virtual ~AuDiplomaticMessage();

	QString MessageText() const;

public slots:
	virtual void setVisible(bool visible);

protected:
	void RestoreGeometry();
	void SaveGeometry();
};

#endif // AuDiplomaticMessage_H
