//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aucommodityfilter.cpp,v 1.4 2022/02/19 18:10:33 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aucommodityfilter.h"
//
#include "auclientglobal.h"
#include "autoolbutton.h"
#include <QBoxLayout>

AuCommodityFilter::AuCommodityFilter(bool horizontal, QWidget* parent) : QWidget(parent)
{
	AuIconButton* pball = new AuIconButton(CommodityFilterIcon(FILTER_All));
	pball->setCheckable(true);
	pball->setChecked(true);									// Default filter selection is 'All'
	pball->setObjectName(CommodityFilterName(FILTER_All));
	pball->setToolTip(tr(CommodityFilterName(FILTER_All)));
	connect(pball, SIGNAL(clicked()), SLOT(AllPressed()));

	AuIconButton* pbammo = new AuIconButton(CommodityFilterIcon(FILTER_Ammo));
	pbammo->setCheckable(true);
	pbammo->setObjectName(CommodityFilterName(FILTER_Ammo));
	pbammo->setToolTip(tr(CommodityFilterName(FILTER_Ammo)));
	connect(pbammo, SIGNAL(clicked()), SLOT(AmmoPressed()));

	AuIconButton* pbarmor = new AuIconButton(CommodityFilterIcon(FILTER_ArmorAndClothes));
	pbarmor->setCheckable(true);
	pbarmor->setObjectName(CommodityFilterName(FILTER_ArmorAndClothes));
	pbarmor->setToolTip(tr(CommodityFilterName(FILTER_ArmorAndClothes)));
	connect(pbarmor, SIGNAL(clicked()), SLOT(ArmorPressed()));

	AuIconButton* pbconst = new AuIconButton(CommodityFilterIcon(FILTER_ConstructionMaterials));
	pbconst->setCheckable(true);
	pbconst->setObjectName(CommodityFilterName(FILTER_ConstructionMaterials));
	pbconst->setToolTip(tr(CommodityFilterName(FILTER_ConstructionMaterials)));
	connect(pbconst, SIGNAL(clicked()), SLOT(ConstructionMaterialPressed()));

	AuIconButton* pbfood = new AuIconButton(CommodityFilterIcon(FILTER_Food));
	pbfood->setCheckable(true);
	pbfood->setObjectName(CommodityFilterName(FILTER_Food));
	pbfood->setToolTip(tr(CommodityFilterName(FILTER_Food)));
	connect(pbfood, SIGNAL(clicked()), SLOT(FoodPressed()));

	AuIconButton* pbfuel = new AuIconButton(CommodityFilterIcon(FILTER_Fuel));
	pbfuel->setCheckable(true);
	pbfuel->setObjectName(CommodityFilterName(FILTER_Fuel));
	pbfuel->setToolTip(tr(CommodityFilterName(FILTER_Fuel)));
	connect(pbfuel, SIGNAL(clicked()), SLOT(FuelPressed()));

	AuIconButton* pbmed = new AuIconButton(CommodityFilterIcon(FILTER_MedicalSupplies));
	pbmed->setCheckable(true);
	pbmed->setObjectName(CommodityFilterName(FILTER_MedicalSupplies));
	pbmed->setToolTip(tr(CommodityFilterName(FILTER_MedicalSupplies)));
	connect(pbmed, SIGNAL(clicked()), SLOT(MedPressed()));

	AuIconButton* pbmisc = new AuIconButton(CommodityFilterIcon(FILTER_Misc));
	pbmisc->setCheckable(true);
	pbmisc->setObjectName(CommodityFilterName(FILTER_Misc));
	pbmisc->setToolTip(tr(CommodityFilterName(FILTER_Misc)));
	connect(pbmisc, SIGNAL(clicked()), SLOT(MiscPressed()));

	AuIconButton* pbbuild = new AuIconButton(CommodityFilterIcon(FILTER_BuildingDesigns));
	pbbuild->setCheckable(true);
	pbbuild->setObjectName(CommodityFilterName(FILTER_BuildingDesigns));
	pbbuild->setToolTip(tr(CommodityFilterName(FILTER_BuildingDesigns)));
	connect(pbbuild, SIGNAL(clicked()), SLOT(BuildingDesignsPressed()));

	AuIconButton* pbspace = new AuIconButton(CommodityFilterIcon(FILTER_SpacecraftDesigns));
	pbspace->setCheckable(true);
	pbspace->setObjectName(CommodityFilterName(FILTER_SpacecraftDesigns));
	pbspace->setToolTip(tr(CommodityFilterName(FILTER_SpacecraftDesigns)));
	connect(pbspace, SIGNAL(clicked()), SLOT(SpacecraftDesignsPressed()));

	AuIconButton* pbspare = new AuIconButton(CommodityFilterIcon(FILTER_SpareParts));
	pbspare->setCheckable(true);
	pbspare->setObjectName(CommodityFilterName(FILTER_SpareParts));
	pbspare->setToolTip(tr(CommodityFilterName(FILTER_SpareParts)));
	connect(pbspare, SIGNAL(clicked()), SLOT(SparePartsPressed()));

	AuIconButton* pbpatents = new AuIconButton(CommodityFilterIcon(FILTER_Patents));
	pbpatents->setCheckable(true);
	pbpatents->setObjectName(CommodityFilterName(FILTER_Patents));
	pbpatents->setToolTip(tr(CommodityFilterName(FILTER_Patents)));
	connect(pbpatents, SIGNAL(clicked()), SLOT(PatentsPressed()));

	AuIconButton* pbtools = new AuIconButton(CommodityFilterIcon(FILTER_Tools));
	pbtools->setCheckable(true);
	pbtools->setObjectName(CommodityFilterName(FILTER_Tools));
	pbtools->setToolTip(tr(CommodityFilterName(FILTER_Tools)));
	connect(pbtools, SIGNAL(clicked()), SLOT(ToolsPressed()));

	AuIconButton* pbweapons = new AuIconButton(CommodityFilterIcon(FILTER_Weapons));
	pbweapons->setCheckable(true);
	pbweapons->setObjectName(CommodityFilterName(FILTER_Weapons));
	pbweapons->setToolTip(tr(CommodityFilterName(FILTER_Weapons)));
	connect(pbweapons, SIGNAL(clicked()), SLOT(WeaponsPressed()));

	AuIconButton* pbnat = new AuIconButton(CommodityFilterIcon(FILTER_NaturalResources));
	pbnat->setCheckable(true);
	pbnat->setObjectName(CommodityFilterName(FILTER_NaturalResources));
	pbnat->setToolTip(tr(CommodityFilterName(FILTER_NaturalResources)));
	connect(pbnat, SIGNAL(clicked()), SLOT(NaturalResourcesPressed()));

	QBoxLayout* lay;
	if(horizontal)
		lay = new QBoxLayout(QBoxLayout::LeftToRight);
	else
		lay = new QBoxLayout(QBoxLayout::TopToBottom);
	lay->setMargin(0);
	lay->setSpacing(0);
	lay->addWidget(pball);
	lay->addWidget(pbfood);
	lay->addWidget(pbfuel);
	lay->addWidget(pbtools);
	lay->addWidget(pbconst);
	lay->addWidget(pbspare);
	lay->addWidget(pbmed);
	lay->addWidget(pbarmor);
	lay->addWidget(pbweapons);
	lay->addWidget(pbammo);
	lay->addWidget(pbspace);
	lay->addWidget(pbbuild);
	lay->addWidget(pbpatents);
	lay->addWidget(pbnat);
	lay->addWidget(pbmisc);
	lay->addStretch(1);
	setLayout(lay);
}

AuCommodityFilter::~AuCommodityFilter()
{
}

void AuCommodityFilter::AllPressed()
{
	SelectFilter(CommodityFilterName(FILTER_All));
}

void AuCommodityFilter::AmmoPressed()
{
	SelectFilter(CommodityFilterName(FILTER_Ammo));
}

void AuCommodityFilter::ArmorPressed()
{
	SelectFilter(CommodityFilterName(FILTER_ArmorAndClothes));
}

void AuCommodityFilter::BuildingDesignsPressed()
{
	SelectFilter(CommodityFilterName(FILTER_BuildingDesigns));
}

void AuCommodityFilter::ConstructionMaterialPressed()
{
	SelectFilter(CommodityFilterName(FILTER_ConstructionMaterials));
}

void AuCommodityFilter::FoodPressed()
{
	SelectFilter(CommodityFilterName(FILTER_Food));
}

void AuCommodityFilter::FuelPressed()
{
	SelectFilter(CommodityFilterName(FILTER_Fuel));
}

void AuCommodityFilter::MedPressed()
{
	SelectFilter(CommodityFilterName(FILTER_MedicalSupplies));
}

void AuCommodityFilter::MiscPressed()
{
	SelectFilter(CommodityFilterName(FILTER_Misc));
}

void AuCommodityFilter::SpacecraftDesignsPressed()
{
	SelectFilter(CommodityFilterName(FILTER_SpacecraftDesigns));
}

void AuCommodityFilter::SparePartsPressed()
{
	SelectFilter(CommodityFilterName(FILTER_SpareParts));
}

void AuCommodityFilter::PatentsPressed()
{
	SelectFilter(CommodityFilterName(FILTER_Patents));
}

void AuCommodityFilter::ToolsPressed()
{
	SelectFilter(CommodityFilterName(FILTER_Tools));
}

void AuCommodityFilter::NaturalResourcesPressed()
{
	SelectFilter(CommodityFilterName(FILTER_NaturalResources));
}

void AuCommodityFilter::WeaponsPressed()
{
	SelectFilter(CommodityFilterName(FILTER_Weapons));
}

// 'name' is the name of the filter, as returned by CommodityFilterName().
void AuCommodityFilter::SelectFilter(const QString& name)
{
	// Set the down state of all buttons to be false, except the one named 'name'.
	QList<AuIconButton*> buttons = findChildren<AuIconButton*>();
	for(QList<AuIconButton*>::iterator bi = buttons.begin(); bi != buttons.end(); bi++)
		(*bi)->setChecked((*bi)->objectName() == name);

	// Make a list of control names in the same order as the COMMODITYFILTER enumeration.
	if(name == CommodityFilterName(FILTER_All))
		emit FilterChanged(FILTER_All);
	else if(name == CommodityFilterName(FILTER_Ammo))
		emit FilterChanged(FILTER_Ammo);
	else if(name == CommodityFilterName(FILTER_ArmorAndClothes))
		emit FilterChanged(FILTER_ArmorAndClothes);
	else if(name == CommodityFilterName(FILTER_BuildingDesigns))
		emit FilterChanged(FILTER_BuildingDesigns);
	else if(name == CommodityFilterName(FILTER_ConstructionMaterials))
		emit FilterChanged(FILTER_ConstructionMaterials);
	else if(name == CommodityFilterName(FILTER_Food))
		emit FilterChanged(FILTER_Food);
	else if(name == CommodityFilterName(FILTER_Fuel))
		emit FilterChanged(FILTER_Fuel);
	else if(name == CommodityFilterName(FILTER_MedicalSupplies))
		emit FilterChanged(FILTER_MedicalSupplies);
	else if(name == CommodityFilterName(FILTER_Misc))
		emit FilterChanged(FILTER_Misc);
	else if(name == CommodityFilterName(FILTER_NaturalResources))
		emit FilterChanged(FILTER_NaturalResources);
	else if(name == CommodityFilterName(FILTER_Patents))
		emit FilterChanged(FILTER_Patents);
	else if(name == CommodityFilterName(FILTER_SpacecraftDesigns))
		emit FilterChanged(FILTER_SpacecraftDesigns);
	else if(name == CommodityFilterName(FILTER_SpareParts))
		emit FilterChanged(FILTER_SpareParts);
	else if(name == CommodityFilterName(FILTER_Tools))
		emit FilterChanged(FILTER_Tools);
	else if(name == CommodityFilterName(FILTER_Weapons))
		emit FilterChanged(FILTER_Weapons);
}

void AuCommodityFilter::SetFilter(COMMODITYFILTER filter)
{
	SelectFilter(CommodityFilterName(filter));
}

void AuCommodityFilter::RestoreFilter(const QString& keyName)
{
	QString filter = g_Au.Settings().readEntry(keyName, "All Items");
	SelectFilter(filter);
}

void AuCommodityFilter::SaveFilter(const QString& keyName)
{
	QList<AuIconButton*> buttons = findChildren<AuIconButton*>();
	for(QList<AuIconButton*>::iterator bi = buttons.begin(); bi != buttons.end(); bi++)
		if((*bi)->isChecked())
		{
			g_Au.Settings().writeEntry(keyName, (*bi)->objectName());
			break;
		}
}

