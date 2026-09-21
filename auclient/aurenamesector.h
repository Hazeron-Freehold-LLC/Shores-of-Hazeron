//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aurenamesector.h,v 1.6 2022/02/19 17:59:55 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuRenameSector_H
#define AuRenameSector_H

#include "auclientlib.h"
#include <audialog.h>

class QLineEdit;
class QPushButton;

class AUCLIENT_DLL AuRenameSector : public AuDialog
{
	Q_OBJECT;
private:
	QString		m_SectorName;		// Final name on accept()

	// Widgets
	QLineEdit*		m_SectorNameEdit;
	QPushButton*	m_Ok;

public:
	AuRenameSector(QWidget* parent=0);
	virtual ~AuRenameSector();

	// Only valid on accept()
	const QString& SectorName() const
	{
		return m_SectorName;
	}

protected slots:
	void EnableControls();
	void OkPressed();
	void SectorNameChanged();
	void RandomSectorName();

protected:
	bool IsSectorNameOk() const;													// Checks the text in the edit control.
};

#endif // AuRenameSector_H
