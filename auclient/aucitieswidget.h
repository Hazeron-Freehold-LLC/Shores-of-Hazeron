//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucitieswidget.h,v 1.8 2022/02/19 17:53:24 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCitiesWidget_H
#define AuCitiesWidget_H

#include "auclientlib.h"
#include "auid.h"
#include <QTableWidget>

class QMouseEvent;

class AUCLIENT_DLL AuCitiesWidget : public QTableWidget
{
	Q_OBJECT;
private:
	quint32								m_EmpireId;				// Empire cities that are listed.

public:
	AuCitiesWidget(QWidget* parent=0);
	virtual ~AuCitiesWidget();

	// Retrieves the ids of all selected cities
	// Returns governance authority if 'personId' governs ALL of the cities in the list that is returned.
	// Returns 0 = no governance authority, 1 = office authority, 2 = founder authority
	int SelectedCities(AuIdList& cityIds, const AuId& personId) const;

	// Retrieves the ids and names of all selected cities
	// Returns governance authority if 'personId' governs ALL of the cities in the list that is returned.
	// Returns 0 = no governance authority, 1 = office authority, 2 = founder authority
	int SelectedCities(AuIdList& cityIds, QStringList& cityNames, const AuId& personId) const;

	void SelectedCitySystems(AuIdList& systemIds, QStringList& systemNames) const;

public slots:
	void ListCities(quint32 empireId);

signals:
	void CityRightClicked(AuId cityId);

protected:
	virtual void mouseReleaseEvent(QMouseEvent* event);
};

#endif // AuCitiesWidget_H
