//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austarmapsearch.h,v 1.6 2022/02/19 18:01:49 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuStarMapSearch_H
#define AuStarMapSearch_H

#include "auclientlib.h"
#include "audialog.h"
#include "auid.h"

class QCheckBox;
class QLineEdit;
class QListWidget;
class QListWidgetItem;

class AUCLIENT_DLL AuStarMapSearch : public AuDialog
{
	Q_OBJECT;
	// Widgets
	QCheckBox*							m_Case;
	QCheckBox*							m_Cities;
	QCheckBox*							m_CityEmpires;
	QCheckBox*							m_CityNames;
	QCheckBox*							m_RegEx;
	QListWidget*						m_Result;
	QCheckBox*							m_Sectors;
	QCheckBox*							m_Spacecraft;
	QCheckBox*							m_SpacecraftEmpires;
	QCheckBox*							m_SpacecraftNames;
	QCheckBox*							m_SystemCat;
	QCheckBox*							m_Systems;
	QLineEdit*							m_Text;

public:
	AuStarMapSearch(QWidget* parent=0);
	virtual ~AuStarMapSearch();

public slots:
	virtual void CaseChanged(int state);
	virtual void CitiesChanged(int state);
	virtual void CityEmpiresChanged(int state);
	virtual void RegExChanged(int state);
	virtual void SearchNow();
	virtual void SearchResultClicked(QListWidgetItem* item);
	virtual void SectorsChanged(int state);
	virtual void setVisible(bool visible);
	virtual void SpacecraftChanged(int state);
	virtual void SpacecraftEmpiresChanged(int state);
	virtual void SystemCatChanged(int state);
	virtual void SystemsChanged(int state);

protected:
	void RestoreGeometry();
	void SaveGeometry();

signals:
	void PlaceClicked(AuId placeId);
};

#endif // AuStarMapSearch_H
