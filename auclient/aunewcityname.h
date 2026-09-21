//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aunewcityname.h,v 1.6 2022/02/19 17:58:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuNewCityName_H
#define AuNewCityName_H

#include "auclientlib.h"
#include "audef.h"
#include "audialog.h"

class QLineEdit;
class QPushButton;

class AUCLIENT_DLL AuNewCityName : public AuDialog
{
	Q_OBJECT;
private:
	QString		m_CityName;		// Final name on accept()
	QString		m_SectorName;	// Final name on accept()
	QString		m_SystemName;	// Final name on accept()

	// Widgets
	QLineEdit*		m_CityNameEdit;
	QPushButton*	m_Ok;
	QLineEdit*		m_SectorNameEdit;
	QLineEdit*		m_SystemNameEdit;

public:
	AuNewCityName(CITYTYPE type, QWidget* parent=0);
	virtual ~AuNewCityName();

	// Only valid on accept()
	const QString& CityName() const
	{
		return m_CityName;
	}

	// Only valid on accept()
	const QString& SolarSystemName() const
	{
		return m_SystemName;
	}

	// Only valid on accept()
	const QString& SectorName() const
	{
		return m_SectorName;
	}

protected slots:
	void EnableControls();
	void OkPressed();
	void CityNameChanged();
	void RandomCityName();
	void RandomSectorName();
	void RandomSystemName();
	void SectorNameChanged();
	void SystemNameChanged();

protected:
	bool IsCityNameOk() const;													// Checks the text in the edit control.
	bool IsSectorNameOk() const;												// Checks the text in the edit control.
	bool IsSystemNameOk() const;												// Checks the text in the edit control.
};

#endif // AuNewCityName_H
