//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubddesignprops.h,v 1.3 2022/02/19 18:21:18 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuBdDesignProps_H
#define AuBdDesignProps_H

#include "audialog.h"
#include "aubddesign.h"

class AuLineEditValue;
class AuSliderValue;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSlider;
class QSplitter;
class QSpinBox;
class QTableWidget;
class QTextBrowser;
class QTextEdit;

class AuBdDesignProps : public AuDialog
{
	Q_OBJECT;
public:
	// Values
	AuBdDesign							m_DesignRec;

private:
	// Widgets
	QTextBrowser*						m_Analysis;
	QSlider*							m_Arena;
	QLineEdit*							m_ArenaLb;
	QSlider*							m_Armor;
	QLineEdit*							m_ArmorLb;
	QSlider*							m_Auditorium;
	QLineEdit*							m_AuditoriumLb;
	QHash<quint16, AuSliderValue*>		m_Bays;
	QHash<quint16, AuLineEditValue*>	m_BayLbs;
	QSlider*							m_Capacitor;
	QLineEdit*							m_CapacitorLb;
	QComboBox*							m_CapacitorMod;
	QTextEdit*							m_Description;
	QSlider*							m_Fuel;
	QLineEdit*							m_FuelLb;
	QSlider*							m_Hold;
	QLineEdit*							m_HoldLb;
	QSlider*							m_HomeLarge;
	QLineEdit*							m_HomeLargeLb;
	QSlider*							m_HomeMedium;
	QLineEdit*							m_HomeMediumLb;
	QSlider*							m_HomeSmall;
	QLineEdit*							m_HomeSmallLb;
	QComboBox*							m_Hull;
	QComboBox*							m_IndustriesCombo;
	QLabel*								m_IndustriesInfo;
	QSlider*							m_LifeSupport;
	QLineEdit*							m_LifeSupportLb;
	QComboBox*							m_LifeSupportMod;
	QSlider*							m_Lounge;
	QLineEdit*							m_LoungeLb;
	QLineEdit*							m_Name;
	QSlider*							m_Office;
	QLineEdit*							m_OfficeLb;
	QPushButton*						m_Ok;
	QSlider*							m_PowerPlant;
	QLineEdit*							m_PowerPlantLb;
	QComboBox*							m_PowerPlantMod;
	QSlider*							m_Sensors;
	QLineEdit*							m_SensorsLb;
	QSlider*							m_Shields;
	QLineEdit*							m_ShieldsLb;
	QSlider*							m_Shop;
	QLineEdit*							m_ShopLb;
	QSlider*							m_Skeleton;
	QLineEdit*							m_SkeletonLb;
	QSplitter*							m_Splitter;
	QSlider*							m_Store;
	QLineEdit*							m_StoreLb;
	QComboBox*							m_Structure;

public:
	AuBdDesignProps(const AuBdDesign& bldRec, QWidget* parent=0);
	virtual ~AuBdDesignProps();

public slots:
	virtual void setVisible(bool visible);

private slots:
	void ArenaEdited();
	void ArmorEdited();
	void AuditoriumEdited();
	void BayEdited(int partId);
	void CapacitorEdited();
	void CapacitorModuleChanged(int index);
	void DescriptionChanged();
	void EnableControls();
	void HoldEdited();
	void HomeLargeEdited();
	void HomeMediumEdited();
	void HomeSmallEdited();
	void IndustryChanged(int index);
	void LifeSupportEdited();
	void LifeSupportModuleChanged(int index);
	void LoungeEdited();
	void MaterialChanged(int index);
	void NameChanged(const QString& text);
	void OfficeEdited();
	void PowerPlantEdited();
	void PowerPlantModuleChanged(int index);
	void SensorsEdited();
	void SetArena(int value);
	void SetArmor(int value);
	void SetAuditorium(int value);
	void SetBay(int partId, int value);
	void SetCapacitor(int value);
	void SetHold(int value);
	void SetHomeLarge(int value);
	void SetHomeMedium(int value);
	void SetHomeSmall(int value);
	void SetLifeSupport(int value);
	void SetLounge(int value);
	void SetOffice(int value);
	void SetPowerPlant(int value);
	void SetSensors(int value);
	void SetShields(int value);
	void SetShop(int value);
	void SetSkeleton(int value);
	void SetStore(int value);
	void ShieldsEdited();
	void ShopEdited();
	void SkeletonEdited();
	void SplitterMoved(int pos, int index);
	void StoreEdited();
	void StructureChanged(int index);
	void UpdateReport();

private:
	void LimitVolumeSliders(QSlider* control);
	void ListIndustries();
	void ListModules(QComboBox* cb, SYSTEM system, COMMODITYID currentModuleCID);
	void RestoreGeometry();
	void SaveGeometry();
	int SumVolumeSliders() const;
	quint32 VolumeValue(const QString& s) const;
};

#endif // AuBdDesignProps_H
