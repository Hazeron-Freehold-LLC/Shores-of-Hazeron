//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumissionorderwidget.h,v 1.9 2022/02/19 17:58:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMissionOrderWidget_H
#define AuMissionOrderWidget_H

#include "auclientlib.h"
#include "auether.h"
#include "aumission.h"
#include "ausensorblip.h"
#include "davessel.h"
#include <QGroupBox>
#include <QHBoxLayout>

class AuCommodityFilter;
class AuSensorBlipList;
class DaSolarSystem;
class QCheckBox;
class QComboBox;
class QLabel;
class QListWidget;
class QRadioButton;
class QSlider;
class QSpinBox;
class QSplitter;
class QStackedWidget;
class QTableWidget;

class AUCLIENT_DLL AuHarvestOpWidget : public QGroupBox
{
	Q_OBJECT;
private:
	// Widgets
	QLineEdit*							m_AmountEdit;
	QLabel*								m_AmountLabel;
	QRadioButton*						m_AmountRadio;
	QComboBox*							m_Commodities;
	QRadioButton*						m_IdleRadio;
	QLabel*								m_PercentLabel;
	QRadioButton*						m_PercentRadio;
	QSlider*							m_PercentSlider;

public:
	AuHarvestOpWidget(const QString& title, COMMODITYID module, COMMODITYID harvestCID, quint8 amountType, quint8 amountPercent,
			quint32 amountQty, QWidget* parent);

	void HarvestOp(AuHarvestOp& op);

protected slots:
	void EnableControls();
	void PercentChanged(int value);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuMissionOrderWidget : public QWidget
{
	Q_OBJECT;
private:
	AuId						m_AirportId;		// Destination airport selected.
	QString						m_AirportName;		// Destination airport selected.
	quint8						m_AmountBuyPercent;	// percent of hold, fuel cell, or capacitor
	quint8						m_AmountBuyType;	// 0 = use m_AmountBuyPercent, 1 = use m_AmountBuyUnits
	quint32						m_AmountBuyUnits;	// units of hold, fuel cell, or capacitor
	quint8						m_AmountSellPercent;	// percent of hold, fuel cell, or capacitor
	quint8						m_AmountSellType;	// 0 = use m_AmountSellPercent, 1 = use m_AmountSellUnits
	quint32						m_AmountSellUnits;	// units of hold, fuel cell, or capacitor
	int							m_ApplyQ;
	AuId						m_Authorization;
	int							m_BayWhich;			// Bay 'which' id of harvester to configure.
	COMMODITYFILTER				m_Category;
	quint8						m_CatBuyType;		// 0 = use m_AmountBuyPercent, 1 and 2 = use m_AmountBuyUnits
	quint8						m_CatSellType;		// 0 = use m_AmountSellPercent, 1 and 2 = use m_AmountSellUnits
	COMMODITYID					m_CIDAtmo;			// Commodity id for atmosphere condenser bay.
	COMMODITYID					m_CIDBuy;
	COMMODITYID					m_CIDMin;			// Commodity id for prospector bay.
	COMMODITYID					m_CIDPart;			// Commodity id for particle collector bay.
	COMMODITYID					m_CIDSea;			// Commodity id for siphon bay.
	COMMODITYID					m_CIDTransfer;
	COMMODITYID					m_CIDWell;			// Commodity id for wildcatter bay.
	AuId						m_CityId;			// Destination city selected.
	QString						m_CityName;			// Destination city selected.
	bool						m_ContentValid;		// True if the current selection and values are valid.
	quint32						m_Count;
	AuEther						m_Ether;
	AuPoint						m_GalaxyCoord;
	quint8						m_GoToRelative;		// 0=galaxy, 1=sector, 2=system
	AuPoint						m_GoTo;
	quint8						m_HarvestFuelThreshold;
	quint8						m_HarvestFuelThresholdByM;
	AuId						m_HarvestMinId;		// Ring or world to harvest minerals
	QString						m_HarvestMinName;	// Ring or world to harvest minerals
	AuHarvestOps				m_HarvestOps;
	AuHarvestOps				m_HarvestOpsByM;
	AuId						m_HarvestPartId;	// World, ring or sun to harvest particles
	QString						m_HarvestPartName;	// World, ring or sun to harvest particles
	AuId						m_HarvestPlanetId;	// Harvest planet or sun selected.
	AuId						m_HarvestPlanetIdByM;	// Harvest planet or sun selected.
	QString						m_HarvestPlanetName;	// Harvest planet or sun selected.
	QString						m_HarvestPlanetNameByM;	// Harvest planet or sun selected.
	AuId						m_HarvestSeaId;		// World to harvest sea resources
	QString						m_HarvestSeaName;	// World to harvest sea resources
	AuId						m_HarvestWellId;	// World to harvest well resources
	QString						m_HarvestWellName;	// World to harvest well resources
	int							m_HoverAlt;			// meters altitude to hover
	bool						m_IsDoOnce;
	bool						m_IsMailCoC;
	bool						m_LightStateHeadlight;
	bool						m_LightStateNavLight;
	bool						m_LightStateRotatingBeacon;
	bool						m_LightStateStrobe;
	quint32						m_MarginBuyCost;	// maximum price
	quint8						m_MarginBuyPercent;	// 0 to 250
	quint8						m_MarginBuyType;	// 0 = use m_MarginBuyPercent, 1 = use m_MarginBuyCost, 2 = pay any price
	quint32						m_MarginSellCost;	// minimum price
	quint8						m_MarginSellPercent;	// 0 to 250
	quint8						m_MarginSellType;	// 0 = use m_MarginSellPercent, 1 = use m_MarginSellCost, 2 = accept any price
	quint8						m_MinQ;
	int							m_Minutes;
	AuMissionOrder::TYPE		m_Order;
	AuId						m_OrderSystemId;	// Solar system id for context of this order's routing to other systems.
	PILOT_UP_POLICY				m_PilotUpPolicy;	// Pilot up policy
	AuId						m_PlanetId;			// Destination planet selected.
	QString						m_PlanetName;		// Destination planet selected.
	QString						m_PlanetLongLat;	// Destination planet longlat entered. Unparsed for correctness.
	CITIZEN_POSTURE				m_Posture;
	int							m_PreboardTime;
	QString						m_ReportChannel;
	AuCommStdChannel::RANGE		m_ReportRange;
	QString						m_ReportSay;
	quint16						m_ReportWhat;
	AuId						m_RouteSystemId;	// Destination solar system selected among those with wormhole routes.
	QString						m_RouteSystemName;	// Destination solar system selected among those with wormhole routes.
	DaSolarSystem*				m_ScannedSystem;	// Created when needed.
	AuPoint						m_SectorCoord;
	AuSensorBlipList			m_SensorBlips;		// Sensor blips requested/received from the server.
	SHIELDFUEL_POLICY			m_ShieldFuelPolicy;	// Shields fuel policy
	SHIELD_POLICY				m_ShieldPolicy;		// Shields policy
	QString						m_ShipName;			// Ship name for christen order.
	AuId						m_StationId;		// Destination station selected.
	QString						m_StationName;		// Destination station selected.
	AuPoint						m_SystemCoord;
	AuPoint						m_SystemGalaxyLoc;	// Destination solar system galaxy coord.
	AuId						m_SystemId;			// Destination solar system selected among all systems.
	QString						m_SystemName;		// Destination solar system selected among all systems.
	AuId						m_TargetId;			// Target atom selected.
	QString						m_TargetName;		// Target atom selected.
	QString						m_Transferee;		// Ship name when m_TransferTarget==1
	double						m_TransferMoneyFromCount;
	int							m_TransferMoneyFromPercent;
	int							m_TransferMoneyFromType;	// 0=percent, 1=amount
	double						m_TransferMoneyToCount;
	int							m_TransferMoneyToPercent;
	int							m_TransferMoneyToType;	// 0=percent, 1=amount
	int							m_TransferPeopleFromBerth;	// 0=officers, 1=crew, 2=troops, 3=passengers
	int							m_TransferPeopleFromCount;
	int							m_TransferPeopleFromPercent;
	int							m_TransferPeopleFromType;	// 0=percent, 1=amount
	int							m_TransferPeopleToBerth;	// 0=officers, 1=crew, 2=troops, 3=passengers
	int							m_TransferPeopleToCount;
	int							m_TransferPeopleToPercent;
	int							m_TransferPeopleToType;	// 0=percent, 1=amount
	int							m_TransferTarget;	// 0=station, 1=named ship

	// Widgets
	QTableWidget*				m_Airports;
	QRadioButton*				m_AmountBuyActualRadio;
	QLabel*						m_AmountBuyLabel;
	QRadioButton*				m_AmountBuyPercentRadio;
	QRadioButton*				m_AmountSellActualRadio;
	QLabel*						m_AmountSellLabel;
	QRadioButton*				m_AmountSellPercentRadio;
	QLabel*						m_AmountTransferFromLabel;
	QLabel*						m_AmountTransferToLabel;
	QRadioButton*				m_ApplyMinQRadio;
	QRadioButton*				m_ApplyShipQRadio;
	QRadioButton*				m_ApplySystemQRadio;
	QComboBox*					m_Bays;
	QLineEdit*					m_BuyCargoAmountEdit;
	QLabel*						m_BuyCargoAmountLabel;
	QLineEdit*					m_BuyCargoCostEdit;
	QLabel*						m_BuyCargoCostLabel;
	QSlider*					m_BuyCargoSlider;
	QLabel*						m_BuyCargoLabel;
	QLabel*						m_BuyCargoCountLabel;
	QLabel*						m_BuyMarginLabel;
	QSlider*					m_BuyMarginSlider;
	QRadioButton*				m_CatBuyActualRadio;
	QLineEdit*					m_CatBuyAmountEdit;
	QLabel*						m_CatBuyAmountLabel;
	QRadioButton*				m_CatBuyEachRadio;
	QLabel*						m_CatBuyLabel;
	QLabel*						m_CatBuyPercentLabel;
	QRadioButton*				m_CatBuyPercentRadio;
	QSlider*					m_CatBuySlider;
	QRadioButton*				m_CatSellActualRadio;
	QLineEdit*					m_CatSellAmountEdit;
	QLabel*						m_CatSellAmountLabel;
	QRadioButton*				m_CatSellEachRadio;
	QLabel*						m_CatSellLabel;
	QLabel*						m_CatSellPercentLabel;
	QRadioButton*				m_CatSellPercentRadio;
	QSlider*					m_CatSellSlider;
	QLabel*						m_CatTransferFromAmountLabel;
	QLabel*						m_CatTransferFromLabel;
	QLabel*						m_CatTransferFromPercentLabel;
	QLabel*						m_CatTransferToAmountLabel;
	QLabel*						m_CatTransferToLabel;
	QLabel*						m_CatTransferToPercentLabel;
	QTableWidget*				m_Cities;
	QComboBox*					m_CommoditiesAtmo;
	QComboBox*					m_CommoditiesBuy;
	QComboBox*					m_CommoditiesMin;
	QComboBox*					m_CommoditiesPart;
	QComboBox*					m_CommoditiesSea;
	QComboBox*					m_CommoditiesTransfer;
	QComboBox*					m_CommoditiesWell;
	AuCommodityFilter*			m_CommodityFilter;
	QLineEdit*					m_CoordsEdit;
	QLabel*						m_CoordsLabel;
	QSpinBox*					m_CountSpin;
	QLabel*						m_CrewCountLabel;
	QLabel*						m_CryoBerthsLoadCountLabel;
	QLabel*						m_CryoBerthsUnloadCountLabel;
	QCheckBox*					m_DoOnce;
	QLabel*						m_DurPatrolLabel;
	QLabel*						m_DurPreboardLabel;
	QLabel*						m_DurWaitLabel;
	QLabel*						m_GoToCoordLabel;
	QLineEdit*					m_GoToCoord;
	QRadioButton*				m_GoToGalaxy;
	QRadioButton*				m_GoToSector;
	QRadioButton*				m_GoToSystem;

	int							m_HarvestByBayPage;
	QMap<quint16, AuHarvestOpWidget*>	m_HarvestByBay;		// key is system which id
	QCheckBox*					m_HarvestDoOnce;
	QLabel*						m_HarvestFuelThresholdLabel;
	QSlider*					m_HarvestFuelThresholdSlider;
	QTableWidget*				m_HarvestPlanetsAndStars;

	int							m_HarvestByModulePage;
	QMap<quint16, AuHarvestOpWidget*>	m_HarvestByModule;	// key is bay module
	QCheckBox*					m_HarvestDoOnceByM;
	QLabel*						m_HarvestFuelThresholdLabelByM;
	QSlider*					m_HarvestFuelThresholdSliderByM;
	QTableWidget*				m_HarvestPlanetsAndStarsByM;
	QLabel*						m_MinQLabel;
	QSpinBox*					m_MinQSpin;

	int							m_ReportPage;
	QCheckBox*					m_ReportCargo;
	QComboBox*					m_ReportChannelCombo;
	QCheckBox*					m_ReportCrew;
	QCheckBox*					m_ReportDamage;
	QCheckBox*					m_ReportDoOnce;
	QCheckBox*					m_ReportFuel;
	QCheckBox*					m_ReportPosition;
	QComboBox*					m_ReportRangeCombo;
	QLineEdit*					m_ReportSayEdit;
	QCheckBox*					m_ReportSensor;

	QVBoxLayout*				m_HarvestOpLay;
	QLabel*						m_HoverAltLabel;
	QSpinBox*					m_HoverAltSpin;
	QLabel*						m_LightHeadlightLabel;
	QRadioButton*				m_LightHeadlightOff;
	QRadioButton*				m_LightHeadlightOn;
	QLabel*						m_LightNavLightLabel;
	QRadioButton*				m_LightNavLightOff;
	QRadioButton*				m_LightNavLightOn;
	QLabel*						m_LightRotatingBeaconLabel;
	QRadioButton*				m_LightRotatingBeaconOff;
	QRadioButton*				m_LightRotatingBeaconOn;
	QLabel*						m_LightStrobeLabel;
	QRadioButton*				m_LightStrobeOff;
	QRadioButton*				m_LightStrobeOn;
	QCheckBox*					m_MailCoC;
	QRadioButton*				m_MarginBuyActualRadio;
	QRadioButton*				m_MarginBuyAnyPriceRadio;
	QLabel*						m_MarginBuyLabel;
	QRadioButton*				m_MarginBuyPercentRadio;
	QRadioButton*				m_MarginSellActualRadio;
	QRadioButton*				m_MarginSellAnyPriceRadio;
	QLabel*						m_MarginSellLabel;
	QRadioButton*				m_MarginSellPercentRadio;
	QLabel*						m_MetersLabel;
	QSpinBox*					m_MinutesSpin;
	QLabel*						m_MinutesLabel;
	QSpinBox*					m_MinutesPreboardSpin;
	QLabel*						m_MinutesPreboardLabel;
	QListWidget*				m_Orders;
	QLabel*						m_PassengerLoadCountLabel;
	QLabel*						m_PassengerUnloadCountLabel;
	QComboBox*					m_PilotUpPolicies;
	QTableWidget*				m_PlanetsAndStars;
	QTableWidget*				m_PlanetsMin;
	QTableWidget*				m_PlanetsPart;
	QTableWidget*				m_PlanetsSea;
	QTableWidget*				m_PlanetsWell;
	QComboBox*					m_Postures;
	QTableWidget*				m_Routes;
	QLineEdit*					m_SellCargoAmountEdit;
	QLabel*						m_SellCargoAmountLabel;
	QLineEdit*					m_SellCargoCostEdit;
	QLabel*						m_SellCargoCostLabel;
	QSlider*					m_SellCargoSlider;
	QLabel*						m_SellCargoLabel;
	QLabel*						m_SellCargoCountLabel;
	QLabel*						m_SellMarginLabel;
	QSlider*					m_SellMarginSlider;
	QComboBox*					m_ShieldFuelPolicies;
	QComboBox*					m_ShieldPolicies;
	QLabel*						m_ShipNameLabel;
	QLineEdit*					m_ShipNameEdit;
	QRadioButton*				m_ShowAll;
	QRadioButton*				m_ShowNamed;
	QWidget*					m_Smashie;
	QSplitter*					m_Splitter;
	QStackedWidget*				m_Stack;
	QTableWidget*				m_Stations;
	QTableWidget*				m_Systems;
	QTableWidget*				m_Targets;
	QLabel*						m_TransferAuthorizationLabel;
	QLabel*						m_TransferCargoCountLabel;
	QLabel*						m_TransferCargoFromAmountLabel;
	QLabel*						m_TransferCargoFromLabel;
	QLabel*						m_TransferCargoToAmountLabel;
	QLabel*						m_TransferCargoToLabel;
	QLineEdit*					m_TransfereeEdit;
	QLabel*						m_TransferFromLabel;
	QLabel*						m_TransferMoneyFromLabel;
	QLineEdit*					m_TransferMoneyFromAmountEdit;
	QRadioButton*				m_TransferMoneyFromAmountRadio;
	QLabel*						m_TransferMoneyFromPercentLabel;
	QRadioButton*				m_TransferMoneyFromPercentRadio;
	QSlider*					m_TransferMoneyFromPercentSlider;
	QLabel*						m_TransferMoneyToLabel;
	QLineEdit*					m_TransferMoneyToAmountEdit;
	QRadioButton*				m_TransferMoneyToAmountRadio;
	QLabel*						m_TransferMoneyToPercentLabel;
	QRadioButton*				m_TransferMoneyToPercentRadio;
	QSlider*					m_TransferMoneyToPercentSlider;
	QRadioButton*				m_TransferNamedShip;
	QLineEdit*					m_TransferPeopleFromAmountEdit;
	QRadioButton*				m_TransferPeopleFromAmountRadio;
	QLabel*						m_TransferPeopleFromBerthLabel;
	QRadioButton*				m_TransferPeopleFromCrewRadio;
	QLabel*						m_TransferPeopleFromLabel;
	QRadioButton*				m_TransferPeopleFromOfficersRadio;
	QRadioButton*				m_TransferPeopleFromPassengersRadio;
	QLabel*						m_TransferPeopleFromPercentLabel;
	QRadioButton*				m_TransferPeopleFromPercentRadio;
	QSlider*					m_TransferPeopleFromPercentSlider;
	QRadioButton*				m_TransferPeopleFromTroopsRadio;
	QLineEdit*					m_TransferPeopleToAmountEdit;
	QRadioButton*				m_TransferPeopleToAmountRadio;
	QLabel*						m_TransferPeopleToBerthLabel;
	QRadioButton*				m_TransferPeopleToCrewRadio;
	QLabel*						m_TransferPeopleToLabel;
	QRadioButton*				m_TransferPeopleToOfficersRadio;
	QRadioButton*				m_TransferPeopleToPassengersRadio;
	QLabel*						m_TransferPeopleToPercentLabel;
	QRadioButton*				m_TransferPeopleToPercentRadio;
	QSlider*					m_TransferPeopleToPercentSlider;
	QRadioButton*				m_TransferPeopleToTroopsRadio;
	QRadioButton*				m_TransferStation;
	QLabel*						m_TransferToLabel;
	QLabel*						m_TroopCountLabel;

public:
	// 'authorization' accompanies transfer orders. It should be the id of the avatar issuing the order.
	// 'order' provides current settings defaults when non-zero. Useful for showing properties of an order.
	AuMissionOrderWidget(const AuId& authorization, const AuId& solarSystem, const AuSensorBlipList& blips,
			const AuPoint& galaxyCoord, const AuPoint& sectorCoord, const AuPoint& systemCoord, const AuMissionOrder* order=0,
			QWidget* parent=0);
	virtual ~AuMissionOrderWidget();

	// Fills 'ops' with the current harvest operations read from the widgets, not the current value of m_HarvestOps.
	void HarvestOps(AuHarvestOps& ops) const;

	inline bool IsContentValid() const
	{
		return m_ContentValid;
	}

	// Returns a mission order corresponding to current settings.
	// Mission order is allocated on the heap using new.
	// Caller must delete the pointer that is returned.
	AuMissionOrder* Order() const;

	void SetSensorBlips(const AuSensorBlipList& blips)
	{
		m_SensorBlips = blips;
		OrderSelected();				// Trigger update of lists with new contacts.
	}

signals:
	void ValidityChanged();

protected:
	void GetValuesFromOrder(const AuMissionOrder* order);
	bool IsLongLatStringValid() const;
	void SetControlVisibility(const QSet<QWidget*>& visibleControls, bool show);
	virtual QSize sizeHint() const;

protected slots:
	void AirportChanged();
	void AmountBuy0Selected();
	void AmountBuy1Selected();
	void AmountSell0Selected();
	void AmountSell1Selected();
	void ApplyQSelected();
	void BayChanged(int index);
	void CargoBuyAmountChanged(const QString& costText);
	void CargoBuyCostChanged(const QString& costText);
	void CargoBuyPercentChanged(int margin);
	void CargoSellAmountChanged(const QString& costText);
	void CargoSellCostChanged(const QString& costText);
	void CargoSellPercentChanged(int margin);
	void CatBuy0Selected();
	void CatBuy1Selected();
	void CatBuy2Selected();
	void CatSell0Selected();
	void CatSell1Selected();
	void CatSell2Selected();
	void CategoryChanged(COMMODITYFILTER filter);
	void CityChanged();
	void CommodityChangedAtmo(int index);
	void CommodityChangedBuy(int index);
	void CommodityChangedMin(int index);
	void CommodityChangedPart(int index);
	void CommodityChangedSea(int index);
	void CommodityChangedTransfer(int index);
	void CommodityChangedWell(int index);
	void CountChanged(int count);
	void EnableControls();
	void GoToCoordChanged(const QString& text);
	void GoToSelected();
	void HarvestFuelThresholdChanged(int index);
	void HarvestFuelThresholdByMChanged(int index);
	void HarvestPlanetChanged();
	void HarvestPlanetChangedByM();
	void HoverAltitudeChanged(int altitude);
	void LightHeadlightOffSelected();
	void LightHeadlightOnSelected();
	void LightNavLightOffSelected();
	void LightNavLightOnSelected();
	void LightRotatingBeaconOffSelected();
	void LightRotatingBeaconOnSelected();
	void LightStrobeOffSelected();
	void LightStrobeOnSelected();
	void LongLatChanged(const QString& text);
	void MarginBuy0Selected();
	void MarginBuy1Selected();
	void MarginBuy2Selected();
	void MarginBuyChanged(int margin);
	void MarginSell0Selected();
	void MarginSell1Selected();
	void MarginSell2Selected();
	void MarginSellChanged(int margin);
	void MinQChanged(int minQ);
	void OrderSelected();
	void PatrolTimeChanged(int minutes);
	void PilotUpPolicyChanged(int index);
	void PlanetChanged();
	void PlanetChangedMin();
	void PlanetChangedPart();
	void PlanetChangedSea();
	void PlanetChangedWell();
	void PostureChanged(int index);
	void PreBoardTimeChanged(int minutes);
	void ReportChannelChanged(const QString& text);
	void ReportRangeChanged(int index);
	void RouteSystemChanged();
	void ShieldFuelPolicyChanged(int index);
	void ShieldPolicyChanged(int index);
	void ShipNameChanged(const QString& text);
	void SplitterMoved(int pos, int index);
	void StationChanged();
	void SystemChanged();
	void TargetChanged();
	void TransferMoneyFromAmount();
	void TransferMoneyFromPercent();
	void TransferMoneyFromPercentChanged(int margin);
	void TransferMoneyToAmount();
	void TransferMoneyToPercent();
	void TransferMoneyToPercentChanged(int margin);
	void TransferNamedShip();
	void TransferPeopleFromAmount();
	void TransferPeopleFromCrew();
	void TransferPeopleFromOfficers();
	void TransferPeopleFromPassengers();
	void TransferPeopleFromPercent();
	void TransferPeopleFromPercentChanged(int margin);
	void TransferPeopleFromTroops();
	void TransferPeopleToAmount();
	void TransferPeopleToCrew();
	void TransferPeopleToOfficers();
	void TransferPeopleToPassengers();
	void TransferPeopleToPercent();
	void TransferPeopleToPercentChanged(int margin);
	void TransferPeopleToTroops();
	void TransferStation();
	void TransfereeChanged(const QString& text);
};

#endif // AuMissionOrderWidget_H
