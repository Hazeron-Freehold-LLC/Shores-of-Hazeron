//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aurenamebuilding.h,v 1.6 2022/02/19 17:59:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuRenameBuilding_H
#define AuRenameBuilding_H

#include "auclientlib.h"
#include <audialog.h>
#include "auid.h"

class QLineEdit;
class QPushButton;

class AUCLIENT_DLL AuRenameBuilding : public AuDialog
{
	Q_OBJECT;
private:
	QString								m_OriginalName;
	AuId								m_CapitolOpId;			// Id of building when operated from a capitol.

	// Widgets
	QPushButton*						m_ClearName;
	QLineEdit*							m_NameEdit;
	QPushButton*						m_Ok;

public:
	AuRenameBuilding(const QString& curName, const AuId& capitolOpId, QWidget* parent=0);
	virtual ~AuRenameBuilding();

protected slots:
	void ClearNamePressed();
	void EnableControls();
	void OkPressed();
	void NameChanged();
	void RandomName();

protected:
	bool IsNameOk() const;										// Checks the text in the edit control.
};

#endif // AuRenameBuilding_H
