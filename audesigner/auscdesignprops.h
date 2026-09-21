//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscdesignprops.h,v 1.4 2022/02/19 18:22:31 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScDesignProps_H
#define AuScDesignProps_H

#include "audialog.h"
#include "auscdesign.h"

class AuLineEditValue;
class AuSliderValue;
class QComboBox;
class QLineEdit;
class QPushButton;
class QSlider;
class QSplitter;
class QSpinBox;
class QTableWidget;
class QTextBrowser;
class QTextEdit;

class AuScDesignProps : public AuDialog
{
	Q_OBJECT;
public:
	// Values
	AuScDesign							m_DesignRec;

private:
	// Widgets
	QTextBrowser*						m_Analysis;
	QSlider*							m_Armor;
	QLineEdit*							m_ArmorLb;
	QHash<quint16, AuSliderValue*>		m_Bays;
	QHash<quint16, AuLineEditValue*>	m_BayLbs;
	QSlider*							m_Capacitor;
	QLineEdit*							m_CapacitorLb;
	QComboBox*							m_CapacitorMod;
	QSlider*							m_Cryo;
	QLineEdit*							m_CryoLb;
	QComboBox*							m_CnC;
	QTextEdit*							m_Description;
	QHash<quint16, AuSliderValue*>		m_FTLDrives;
	QHash<quint16, AuLineEditValue*>	m_FTLDriveLbs;
	QSlider*							m_Fuel;
	QLineEdit*							m_FuelLb;
	QSlider*							m_Hold;
	QLineEdit*							m_HoldLb;
	QComboBox*							m_Hull;
	QComboBox*							m_LandingOrient;
	QSlider*							m_LifeSupport;
	QLineEdit*							m_LifeSupportLb;
	QComboBox*							m_LifeSupportMod;
	QSlider*							m_ManeuverDrive;
	QLineEdit*							m_ManeuverDriveLb;
	QComboBox*							m_ManeuverDriveMod;
	QLineEdit*							m_Name;
	QPushButton*						m_Ok;
	QSlider*							m_PowerPlant;
	QLineEdit*							m_PowerPlantLb;
	QComboBox*							m_PowerPlantMod;
	QSlider*							m_Sensors;
	QLineEdit*							m_SensorsLb;
	QSlider*							m_Shields;
	QLineEdit*							m_ShieldsLb;
	QSlider*							m_Skeleton;
	QLineEdit*							m_SkeletonLb;
	QSplitter*							m_Splitter;
	QSpinBox*							m_SpinMinQ;
	QComboBox*							m_StationOrient;
	QComboBox*							m_Streamline;
	QComboBox*							m_Structure;

public:
	AuScDesignProps(const AuScDesign& shipRec, QWidget* parent=0);
	virtual ~AuScDesignProps();

public slots:
	virtual void setVisible(bool visible);

private slots:
	void ArmorEdited();
	void BayEdited(int partId);
	void CapacitorEdited();
	void CapacitorModuleChanged(int index);
	void CnCModuleChanged(int index);
	void CryoEdited();
	void DescriptionChanged();
	void EnableControls();
	void FTLDriveEdited(int partId);
	void FuelEdited();
	void HoldEdited();
	void HullModuleChanged(int index);
	void LandingOrientationChanged(int index);
	void LifeSupportEdited();
	void LifeSupportModuleChanged(int index);
	void ManeuverDriveEdited();
	void ManeuverDriveModuleChanged(int index);
	void NameChanged(const QString& text);
	void PowerPlantEdited();
	void PowerPlantModuleChanged(int index);
	void SensorsEdited();
	void SetArmor(int value);
	void SetBay(int partId, int value);
	void SetCapacitor(int value);
	void SetCryo(int value);
	void SetFTLDrive(int partId, int value);
	void SetFuel(int value);
	void SetHold(int value);
	void SetLifeSupport(int value);
	void SetMinQ(int value);
	void SetManeuverDrive(int value);
	void SetPowerPlant(int value);
	void SetSensors(int value);
	void SetShields(int value);
	void SetSkeleton(int value);
	void ShieldsEdited();
	void SkeletonEdited();
	void SplitterMoved(int pos, int index);
	void StationOrientationChanged(int index);
	void StreamlineChanged(int index);
	void StructureChanged(int index);
	void UpdateReport();

private:
	void LimitVolumeSliders(QSlider* control);
	void ListModules(QComboBox* cb, SYSTEM system, COMMODITYID currentModuleCID);
	void RestoreGeometry();
	void SaveGeometry();
	int SumVolumeSliders() const;
	quint32 VolumeValue(const QString& s) const;
};

#endif // AuScDesignProps_H
