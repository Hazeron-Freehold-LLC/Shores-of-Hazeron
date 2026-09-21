//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommodityfilter.h,v 1.7 2022/02/19 17:54:42 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuCommodityFilter_H
#define AuCommodityFilter_H

#include "auclientlib.h"
#include "dbcommodity.h"
#include <QWidget>

class AUCLIENT_DLL AuCommodityFilter : public QWidget
{
	Q_OBJECT;

public:
	AuCommodityFilter(bool horizontal=true, QWidget* parent=0);
	virtual ~AuCommodityFilter();

signals:
	void FilterChanged(COMMODITYFILTER);

public slots:
	void AllPressed();
	void AmmoPressed();
	void ArmorPressed();
	void BuildingDesignsPressed();
	void ConstructionMaterialPressed();
	void FoodPressed();
	void FuelPressed();
	void MedPressed();
	void MiscPressed();
	void NaturalResourcesPressed();
	void PatentsPressed();
	void RestoreFilter(const QString& keyName);
	void SaveFilter(const QString& keyName);
	void SetFilter(COMMODITYFILTER filter);
	void SpacecraftDesignsPressed();
	void SparePartsPressed();
	void ToolsPressed();
	void WeaponsPressed();

protected:
	// 'name' is the name of the filter, as returned by CommodityFilterName().
	void SelectFilter(const QString& name);
};

#endif // AuCommodityFilter_H
