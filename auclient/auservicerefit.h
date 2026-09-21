//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auservicerefit.h,v 1.7 2022/02/19 18:00:50 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuServiceRefit_H
#define AuServiceRefit_H

#include "auclientlib.h"
#include "auscreport.h"
#include <QWidget>

class AuLineEditValue;
class AuServiceDialog;
class AuSliderValue;
class QLabel;
class QLineEdit;
class QPushButton;
class QSlider;
class QSpinBox;
class QSplitter;
class QTextBrowser;

class AUCLIENT_DLL AuServiceRefit : public QWidget
{
	Q_OBJECT;
private:
	AuScReport							m_DesignReport;
	AuId								m_ShipId;

	// Widgets
	QTextBrowser*						m_Analysis;
	QSlider*							m_Armor;
	QLineEdit*							m_ArmorLb;
	QHash<quint16, AuSliderValue*>		m_Bays;
	QHash<quint16, AuLineEditValue*>	m_BayLbs;
	QSlider*							m_Capacitor;
	QLineEdit*							m_CapacitorLb;
	QSlider*							m_Cryo;
	QLineEdit*							m_CryoLb;
	QHash<quint16, AuSliderValue*>		m_FTLDrives;
	QHash<quint16, AuLineEditValue*>	m_FTLDriveLbs;
	QSlider*							m_Fuel;
	QLineEdit*							m_FuelLb;
	QSlider*							m_Hold;
	QLineEdit*							m_HoldLb;
	QLabel*								m_Info;
	QSlider*							m_LifeSupport;
	QLineEdit*							m_LifeSupportLb;
	QSlider*							m_ManeuverDrive;
	QLineEdit*							m_ManeuverDriveLb;
	QSpinBox*							m_MinQ;
	QSlider*							m_PowerPlant;
	QLineEdit*							m_PowerPlantLb;
	QPushButton*						m_RefitButton;
	QSlider*							m_Sensors;
	QLineEdit*							m_SensorsLb;
	QSlider*							m_Shields;
	QLineEdit*							m_ShieldsLb;
	QSplitter*							m_Splitter;
	QSlider*							m_Skeleton;
	QLineEdit*							m_SkeletonLb;

public:
	AuServiceRefit(AuServiceDialog* parent=0);

protected slots:
	void ArmorEdited();
	void BayEdited(int partId);
	void CapacitorEdited();
	void CryoEdited();
	void FTLDriveEdited(int partId);
	void FuelEdited();
	void HoldEdited();
	void LifeSupportEdited();
	void ManeuverDriveEdited();
	void PowerPlantEdited();
	void ResetVolumes();
	void SensorsEdited();
	void SetArmor(int value);
	void SetBay(int partId, int value);
	void SetCapacitor(int value);
	void SetCryo(int value);
	void SetFTLDrive(int partId, int value);
	void SetFuel(int value);
	void SetHold(int value);
	void SetLifeSupport(int value);
	void SetManeuverDrive(int value);
	void SetPowerPlant(int value);
	void SetSensors(int value);
	void SetShields(int value);
	void SetSkeleton(int value);
	void ShieldsEdited();
	void SkeletonEdited();
	void RequestRefit();
	void SplitterMoved(int pos, int index);

private:
	void LimitVolumeSliders(QSlider* control);
	void SetInfoText(bool sizeMismatch);
	quint32 SumVolumeSliders() const;
	quint32 VolumeValue(const QString& s) const;
};

#endif // AuServiceRefit_H
