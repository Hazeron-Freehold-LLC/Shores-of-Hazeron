//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommsolicititem.h,v 1.10 2022/02/19 17:54:53 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AUCOMMSOLICITITEM_H
#define AUCOMMSOLICITITEM_H

#include "auclientlib.h"
#include "aucommodityfilter.h"
#include <audialog.h>

class AUCLIENT_DLL AuCommSolicitItem : public AuDialog
{
	Q_OBJECT;
public:
	enum TYPE
	{
		All,
		Ammo,
		Commodity,
		ConstructionMaterial,
		Food,
		Fuel,
		Medical,
		SpareParts,
		Tools,
		Weapons,
		Armor,
		NaturalResources,
		Patents,
		Misc,
		Shipments,
	};

	AuCommSolicitItem(COMMODITYID defCommodityId, QWidget* parent=0);

	COMMODITYID CommodityId() const
	{
		return m_lCID;
	}

	TYPE Solicitation() const
	{
		return m_SolicitType;
	}

public slots:
	virtual void setVisible(bool visible);

protected slots:
	void AllPressed();
	void AmmoPressed();
	void ArmorPressed();
	void ConstructionMaterialPressed();
	void EnableControls();
	void FilterCommodities(COMMODITYFILTER);
	void FoodPressed();
	void FuelPressed();
	void ListCommodities();
	void MedPressed();
	void MiscPressed();
	void NaturalResourcesPressed();
	void OkPressed();
	void PatentsPressed();
	void ShipmentsPressed();
	void SparePartsPressed();
	void ToolsPressed();
	void WeaponsPressed();

protected:
	void RestoreGeometry();
	void SaveGeometry();

private:
	COMMODITYFILTER	m_Filter;
	COMMODITYID					m_lCID;			// Commodity id selected when accepted.
	TYPE						m_SolicitType;	// Type of solicitation desired when accepted.
};

#endif // AUCOMMSOLICITITEM_H
