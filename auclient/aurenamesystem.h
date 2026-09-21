//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aurenamesystem.h,v 1.6 2022/02/19 18:00:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuRenameSystem_H
#define AuRenameSystem_H

#include "auclientlib.h"
#include <audialog.h>

class QLineEdit;
class QPushButton;

class AUCLIENT_DLL AuRenameSystem : public AuDialog
{
	Q_OBJECT;
private:
	QString		m_SystemName;		// Final name on accept()

	// Widgets
	QLineEdit*		m_SystemNameEdit;
	QPushButton*	m_Ok;

public:
	AuRenameSystem(QWidget* parent=0);
	virtual ~AuRenameSystem();

	// Only valid on accept()
	const QString& SystemName() const
	{
		return m_SystemName;
	}

protected slots:
	void EnableControls();
	void OkPressed();
	void SystemNameChanged();
	void RandomSystemName();

protected:
	bool IsSystemNameOk() const;													// Checks the text in the edit control.
};

#endif // AuRenameSystem_H
