//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aurenamecity.h,v 1.6 2022/02/19 17:59:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuRenameCity_H
#define AuRenameCity_H

#include "auclientlib.h"
#include "audialog.h"
#include "audef.h"

class QLineEdit;
class QPushButton;

class AUCLIENT_DLL AuRenameCity : public AuDialog
{
	Q_OBJECT;
private:
	QString		m_CityName;		// Final name on accept()

	// Widgets
	QLineEdit*		m_CityNameEdit;
	QPushButton*	m_Ok;

public:
	AuRenameCity(CITYTYPE type, QWidget* parent=0);
	virtual ~AuRenameCity();

	// Only valid on accept()
	const QString& CityName() const
	{
		return m_CityName;
	}

protected slots:
	void EnableControls();
	void OkPressed();
	void CityNameChanged();
	void RandomCityName();

protected:
	bool IsCityNameOk() const;													// Checks the text in the edit control.
};

#endif // AuRenameCity_H
