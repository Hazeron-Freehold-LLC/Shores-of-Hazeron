//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auordnancetransfer.h,v 1.10 2022/05/04 21:17:16 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuOrdnanceTransfer_H
#define AuOrdnanceTransfer_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"
#include "auid.h"
#include "davehicle.h"
#include <QWidget>

class QLabel;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;
class QTimer;

class AUCLIENT_DLL AuOrdnanceTransfer : public QWidget
{
	Q_OBJECT;
private:
	AuId								m_LoaderId;				// Id of ship or city providing the ordnance.
	COMMODITYID							m_WeaponCID;			// Weapon/turret/vehicle loading ordnance.
	quint32								m_WeaponUnits;			// Number of weapon bay units, when selecting for a bay weapon.

	// Widgets
	QLabel*								m_Desc;
	QLabel*								m_GunPic;
	QPushButton*						m_LoadGun;
	QPushButton*						m_LoadOrd0;
	QPushButton*						m_LoadOrd1;
	QLabel*								m_MyGunDesc;
	QLabel*								m_MyGunPic;
	QLabel*								m_MyOrd0Desc;
	QLabel*								m_MyOrd0Pic;
	QLabel*								m_MyOrd1Desc;
	QLabel*								m_MyOrd1Pic;
	QLabel*								m_MyName;
	QLabel*								m_MyPic;
	QLabel*								m_OrdnanceDesc;
	QLabel*								m_OrdnancePic;
	QTableWidget*						m_OrdnanceTable;
	QLabel*								m_Ord0Pic;
	QLabel*								m_Ord1Pic;
	QLabel*								m_OtherDesc;
	QLabel*								m_OtherPic;
	QPushButton*						m_SelectAmmo;
	QTimer*								m_timer;			// For updates.

public:
	AuOrdnanceTransfer(QWidget* parent=0);
	virtual ~AuOrdnanceTransfer();

	void OrdnanceReceived(const QByteArray& ba);

	virtual QSize sizeHint() const;

protected:
	virtual void showEvent(QShowEvent* event);
	void LoadSelectedOrdnance(DaVehicle::WEAPON_RACK rack);

protected slots:
	void LoadSelectedToGun();
	void LoadSelectedToOrd0();
	void LoadSelectedToOrd1();
	void OrdnanceSelected();
	void RequestOrdnance();
	void UpdateContents();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuCargoOrdnanceDock : public AuDockWidget
{
public:
	AuOrdnanceTransfer*					m_Content;

public:
	AuCargoOrdnanceDock(QWidget* parent) : AuDockWidget(":data/c_HeavyWeapon.png", tr("<b>Ordnance</b>"),
			tr("Ordnance"), parent)
	{
		m_Content = new AuOrdnanceTransfer(this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/CargoOrdnanceAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/CargoOrdnanceAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuOrdnanceTransfer_H
