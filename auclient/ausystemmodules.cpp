//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausystemmodules.cpp,v 1.4 2022/02/19 18:17:07 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausystemmodules.h"
//
#include "auglobal.h"
#include "aucombobox.h"
#include <QGridLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>

AuSystemModules::AuSystemModules(bool isBuilding, int lowestDeck, quint16 cncModule, quint16 ftlDriveModule, quint16 hullModule,
		quint16 lifeSupportModule, quint16 maneuverDriveModule, quint16 powerPlantModule, quint16 sensorModule,
		quint16 shieldModule, const unordered_map<quint16, quint16>& bayModules,
		const unordered_map<quint16, quint16>& transporterModules, QWidget* parent) :
	AuDialog(parent)
{
	setAuDlgIcon(QIcon(":data/Design.png"));
	setAuDlgTitle("System Modules");
	m_CnCModule = cncModule;
	m_FTLDriveModule = ftlDriveModule;
	m_HullModule = hullModule;
	m_LifeSupportModule = lifeSupportModule;
	m_ManeuverDriveModule = maneuverDriveModule;
	m_PowerPlantModule = powerPlantModule;
	m_SensorModule = sensorModule;
	m_ShieldModule = shieldModule;
	m_BayModules = bayModules;
	m_TransporterModules = transporterModules;

	QLabel* lbCnC = new QLabel(this);
	lbCnC->setPixmap(QPixmap(SystemIcon(SYS_CommandAndControl)));

	m_CnC = new QComboBox(this);
	m_CnC->setToolTip(ScSystemName(SYS_CommandAndControl));
	ListModules(m_CnC, SYS_CommandAndControl, (COMMODITYID)cncModule, isBuilding);
	connect(m_CnC, SIGNAL(activated(int)), SLOT(CnCModuleChanged(int)));

	QLabel* lbFTLDrive = new QLabel(this);
	lbFTLDrive->setPixmap(QPixmap(SystemIcon(SYS_FTLDrive)));

	m_FTLDrive = new QComboBox(this);
	m_FTLDrive->setToolTip(ScSystemName(SYS_FTLDrive));
	ListModules(m_FTLDrive, SYS_FTLDrive, (COMMODITYID)ftlDriveModule, isBuilding);
	connect(m_FTLDrive, SIGNAL(activated(int)), SLOT(FTLDriveModuleChanged(int)));

	QLabel* lbHull = new QLabel(this);
	lbHull->setPixmap(QPixmap(SystemIcon(SYS_Hull)));

	m_Hull = new QComboBox(this);
	m_Hull->setToolTip(ScSystemName(SYS_Hull));
	ListModules(m_Hull, SYS_Hull, (COMMODITYID)hullModule, isBuilding);
	connect(m_Hull, SIGNAL(activated(int)), SLOT(HullModuleChanged(int)));

	QLabel* lbLifeSupport = new QLabel(this);
	lbLifeSupport->setPixmap(QPixmap(SystemIcon(SYS_LifeSupport)));

	m_LifeSupport = new QComboBox(this);
	m_LifeSupport->setToolTip(ScSystemName(SYS_LifeSupport));
	ListModules(m_LifeSupport, SYS_LifeSupport, (COMMODITYID)lifeSupportModule, isBuilding);
	connect(m_LifeSupport, SIGNAL(activated(int)), SLOT(LifeSupportModuleChanged(int)));

	QLabel* lbManeuverDrive = new QLabel(this);
	lbManeuverDrive->setPixmap(QPixmap(SystemIcon(SYS_ManeuverDrive)));

	m_ManeuverDrive = new QComboBox(this);
	m_ManeuverDrive->setToolTip(ScSystemName(SYS_ManeuverDrive));
	ListModules(m_ManeuverDrive, SYS_ManeuverDrive, (COMMODITYID)maneuverDriveModule, isBuilding);
	connect(m_ManeuverDrive, SIGNAL(activated(int)), SLOT(ManeuverDriveModuleChanged(int)));

	QLabel* lbPowerPlant = new QLabel(this);
	lbPowerPlant->setPixmap(QPixmap(SystemIcon(SYS_PowerPlant)));

	m_PowerPlant = new QComboBox(this);
	m_PowerPlant->setToolTip(ScSystemName(SYS_PowerPlant));
	ListModules(m_PowerPlant, SYS_PowerPlant, (COMMODITYID)powerPlantModule, isBuilding);
	connect(m_PowerPlant, SIGNAL(activated(int)), SLOT(PowerPlantModuleChanged(int)));

	QLabel* lbSensor = new QLabel(this);
	lbSensor->setPixmap(QPixmap(SystemIcon(SYS_Sensor)));

	m_Sensor = new QComboBox(this);
	m_Sensor->setToolTip(ScSystemName(SYS_Sensor));
	ListModules(m_Sensor, SYS_Sensor, (COMMODITYID)sensorModule, isBuilding);
	connect(m_Sensor, SIGNAL(activated(int)), SLOT(SensorModuleChanged(int)));

	QLabel* lbShield = new QLabel(this);
	lbShield->setPixmap(QPixmap(SystemIcon(SYS_Shield)));

	m_Shield = new QComboBox(this);
	m_Shield->setToolTip(ScSystemName(SYS_Shield));
	ListModules(m_Shield, SYS_Shield, (COMMODITYID)shieldModule, isBuilding);
	connect(m_Shield, SIGNAL(activated(int)), SLOT(ShieldModuleChanged(int)));

	// Bay Modules
	QLabel* lbbaytitle0 = 0;
	QLabel* lbbaytitle1 = 0;
	unordered_map<quint16, QLabel*> lbbays;
	if(!bayModules.empty())
	{
		lbbaytitle0 = new QLabel(this);
		lbbaytitle0->setPixmap(QPixmap(SystemIcon(SYS_Weapon)));
		QString sysname = ScSystemName(SYS_Weapon);
		lbbaytitle1 = new QLabel(sysname, this);
		for(unordered_map<quint16, quint16>::const_iterator mi = bayModules.begin(); mi != bayModules.end(); mi++)
		{
			char cdeck = (char)((*mi).first >> 8);
			short int deck = cdeck - lowestDeck + 1;
			unsigned char sub = ((*mi).first & 0xff) + 1;
			QString name = QString("%1 %2 Deck %3").arg(sysname).arg(sub).arg(deck);

			QLabel* lb = new QLabel(name, this);
			lbbays.insert(pair<quint16, QLabel*>((*mi).first, lb));

			AuComboBoxValue* cb = new AuComboBoxValue((*mi).first, this);
			cb->setToolTip(name);
			ListModules(cb, SYS_Weapon, (COMMODITYID)(*mi).second, isBuilding);
			connect(cb, SIGNAL(CurrentIndexChanged(int, int)), SLOT(BayModuleChanged(int, int)));

			m_Bays.insert(pair<quint16, AuComboBoxValue*>((*mi).first, cb));
		}
	}

	// Transporter Modules
	QLabel* lbtransportertitle0 = 0;
	QLabel* lbtransportertitle1 = 0;
	unordered_map<quint16, QLabel*> lbtransporters;
	if(!transporterModules.empty())
	{
		lbtransportertitle0 = new QLabel(this);
		lbtransportertitle0->setPixmap(QPixmap(SystemIcon(SYS_Transporter)));
		QString sysname = ScSystemName(SYS_Transporter);
		lbtransportertitle1 = new QLabel(sysname, this);
		for(unordered_map<quint16, quint16>::const_iterator mi = transporterModules.begin(); mi != transporterModules.end(); mi++)
		{
			char cdeck = (char)((*mi).first >> 8);
			short int deck = cdeck - lowestDeck + 1;
			unsigned char sub = ((*mi).first & 0xff) + 1;
			QString name = QString("%1 %2 Deck %3").arg(sysname).arg(sub).arg(deck);

			QLabel* lb = new QLabel(name, this);
			lbtransporters.insert(pair<quint16, QLabel*>((*mi).first, lb));

			AuComboBoxValue* cb = new AuComboBoxValue((*mi).first, this);
			cb->setToolTip(name);
			ListModules(cb, SYS_Transporter, (COMMODITYID)(*mi).second, isBuilding);
			connect(cb, SIGNAL(CurrentIndexChanged(int, int)), SLOT(TransporterModuleChanged(int, int)));

			m_Transporters.insert(pair<quint16, AuComboBoxValue*>((*mi).first, cb));
		}
	}

	QPushButton* pbok = new QPushButton(tr("Apply Changes"));
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));
	pbok->setToolTip(tr("Apply system module changes to the design."));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbCnC, row,0);
	lay->addWidget(m_CnC, row,1);
	row++;
	lay->addWidget(lbFTLDrive, row,0);
	lay->addWidget(m_FTLDrive, row,1);
	row++;
	lay->addWidget(lbHull, row,0);
	lay->addWidget(m_Hull, row,1);
	row++;
	lay->addWidget(lbLifeSupport, row,0);
	lay->addWidget(m_LifeSupport, row,1);
	row++;
	lay->addWidget(lbManeuverDrive, row,0);
	lay->addWidget(m_ManeuverDrive, row,1);
	row++;
	lay->addWidget(lbPowerPlant, row,0);
	lay->addWidget(m_PowerPlant, row,1);
	row++;
	lay->addWidget(lbSensor, row,0);
	lay->addWidget(m_Sensor, row,1);
	row++;
	lay->addWidget(lbShield, row,0);
	lay->addWidget(m_Shield, row,1);
	row++;

	if(lbbaytitle0)
	{
		lay->addWidget(lbbaytitle0, row,0);
		lay->addWidget(lbbaytitle1, row,1);
		row++;
		unordered_map<quint16, QLabel*>::const_iterator blbi = lbbays.begin();
		unordered_map<quint16, AuComboBoxValue*>::const_iterator bcbi = m_Bays.begin();
		while(blbi != lbbays.end() && bcbi != m_Bays.end())
		{
			lay->addWidget((*bcbi).second, row,1);
			lay->addWidget((*blbi).second, row,2);
			row++;
			blbi++;
			bcbi++;
		}
	}

	if(lbtransportertitle0)
	{
		lay->addWidget(lbtransportertitle0, row,0);
		lay->addWidget(lbtransportertitle1, row,1);
		row++;
		unordered_map<quint16, QLabel*>::const_iterator tlbi = lbtransporters.begin();
		unordered_map<quint16, AuComboBoxValue*>::const_iterator tcbi = m_Transporters.begin();
		while(tlbi != lbtransporters.end() && tcbi != m_Transporters.end())
		{
			lay->addWidget((*tcbi).second, row,1);
			lay->addWidget((*tlbi).second, row,2);
			row++;
			tlbi++;
			tcbi++;
		}
	}

	lay->addWidget(pbok, row,0, 1,3);
	setAuDlgLayout(lay);
}

AuSystemModules::~AuSystemModules()
{
}

void AuSystemModules::ListModules(QComboBox* cb, SYSTEM system, COMMODITYID currentModuleCID, bool isBuilding)
{
	for(DbCommodityArray::const_iterator ci = g_Au.m_CommodityArray.cbegin(); ci != g_Au.m_CommodityArray.cend(); ci++)
		if(isBuilding)
		{
			if(DbCommodity::IsBdModule(system, (*ci).m_id))
				cb->addItem(QIcon((*ci).BitmapName()), (*ci).m_name, (*ci).m_id);
		}
		else
		{
			if(DbCommodity::IsScModule(system, (*ci).m_id))
				cb->addItem(QIcon((*ci).BitmapName()), (*ci).m_name, (*ci).m_id);
		}

	int curIndex = cb->findData(currentModuleCID);
	if(curIndex >= 0)
		cb->setCurrentIndex(curIndex);
}

void AuSystemModules::CnCModuleChanged(int index)
{
	m_CnCModule = m_CnC->itemData(index).toInt();
}

void AuSystemModules::FTLDriveModuleChanged(int index)
{
	m_FTLDriveModule = m_FTLDrive->itemData(index).toInt();
}

void AuSystemModules::HullModuleChanged(int index)
{
	m_HullModule = m_Hull->itemData(index).toInt();
}

void AuSystemModules::LifeSupportModuleChanged(int index)
{
	m_LifeSupportModule = m_LifeSupport->itemData(index).toInt();
}

void AuSystemModules::ManeuverDriveModuleChanged(int index)
{
	m_ManeuverDriveModule = m_ManeuverDrive->itemData(index).toInt();
}

void AuSystemModules::PowerPlantModuleChanged(int index)
{
	m_PowerPlantModule = m_PowerPlant->itemData(index).toInt();
}

void AuSystemModules::SensorModuleChanged(int index)
{
	m_SensorModule = m_Sensor->itemData(index).toInt();
}

void AuSystemModules::ShieldModuleChanged(int index)
{
	m_ShieldModule = m_Shield->itemData(index).toInt();
}

void AuSystemModules::BayModuleChanged(int index, int value)
{
	quint16 which = (quint16)value;
	m_BayModules[which] = m_Bays[which]->itemData(index).toInt();
}

void AuSystemModules::TransporterModuleChanged(int index, int value)
{
	quint16 which = (quint16)value;
	m_TransporterModules[which] = m_Transporters[which]->itemData(index).toInt();
}
