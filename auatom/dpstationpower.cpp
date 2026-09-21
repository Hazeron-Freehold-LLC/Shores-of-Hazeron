//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstationpower.cpp,v 1.21 2022/04/14 16:44:30 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpstationpower.h"
//
#include "auatomrenderinfo.h"
#include "augleffects.h"
#include "auglobal.h"
#include "ausound.h"
#include "daperson.h"
#include "dgdata.h"
#include "dicontrolinput.h"
#include "dipanelcontrol.h"
#include <qpainter.h>
#include <qsqlquery.h>
#include <QPixmap>

#define d_Consumer						SOUND_Bleep1
#define d_Overclock						SOUND_Bleep4
#define d_Shunt							SOUND_Bleep6
#define d_Supplier						SOUND_Bleep3

static int s_Anim3 = 0;		// for animation of panel
static int s_Anim8 = 0;		// for animation of panel
static int s_Anim12 = 0;	// for animation of panel

// Table of control inputs.
DiControlInputArray s_PowerConsoleControls;		// Controls that may have bindings modified by the user.
const DiControlInput s_PowerConsoleDefaults[] =
{
	// Login
	//DiControlInput(QObject::tr("Login"),
				//DpStationPower::CI_Login, Qt::Key_L, Qt::ShiftModifier,
				//DiControlInput::MT_None, Qt::NoModifier,
				//DiControlInput::JT_None, 0, 100, false),

	// Mappings:
	// Power Shunts are '1' through '8'
	// Supplier Connections are 'a' through 'k'
	// Consumer Connections are 'z' through ','

	// Power Shunts
	DiControlInput(QObject::tr("Power Shunt 1"),
		DpStationPower::CI_PowerShunt1, Qt::Key_unknown,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Power Shunt 2"),
		DpStationPower::CI_PowerShunt2, Qt::Key_unknown,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Power Shunt 3"),
		DpStationPower::CI_PowerShunt3, Qt::Key_unknown,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Power Shunt 4"),
		DpStationPower::CI_PowerShunt4, Qt::Key_unknown,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Power Shunt 5"),
		DpStationPower::CI_PowerShunt5, Qt::Key_unknown,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Power Shunt 6"),
		DpStationPower::CI_PowerShunt6, Qt::Key_unknown,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Power Shunt 7"),
		DpStationPower::CI_PowerShunt7, Qt::Key_unknown,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Power Shunt 8"),
		DpStationPower::CI_PowerShunt8, Qt::Key_unknown,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
	
	// Power Connection
	DiControlInput(QObject::tr("Alter Level 1 Supplier"),
		DpStationPower::CI_Connection1Supplier, Qt::Key_A,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Alter Level 1 Consumer"),
		DpStationPower::CI_Connection1Consumer, Qt::Key_Z,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Alter Level 2 Supplier"),
		DpStationPower::CI_Connection2Supplier, Qt::Key_S,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Alter Level 2 Consumer"),
		DpStationPower::CI_Connection2Consumer, Qt::Key_X,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Alter Level 3 Supplier"),
		DpStationPower::CI_Connection3Supplier, Qt::Key_D,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Alter Level 3 Consumer"),
		DpStationPower::CI_Connection3Consumer, Qt::Key_C,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Alter Level 4 Supplier"),
		DpStationPower::CI_Connection4Supplier, Qt::Key_F,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Alter Level 4 Consumer"),
		DpStationPower::CI_Connection4Consumer, Qt::Key_V,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Alter Level 5 Supplier"),
		DpStationPower::CI_Connection5Supplier, Qt::Key_G,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Alter Level 5 Consumer"),
		DpStationPower::CI_Connection5Consumer, Qt::Key_B,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Alter Level 6 Supplier"),
		DpStationPower::CI_Connection6Supplier, Qt::Key_H,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Alter Level 6 Consumer"),
		DpStationPower::CI_Connection6Consumer, Qt::Key_N,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Alter Level 7 Supplier"),
		DpStationPower::CI_Connection7Supplier, Qt::Key_J,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Alter Level 7 Consumer"),
		DpStationPower::CI_Connection7Consumer, Qt::Key_M,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Alter Level 8 Supplier"),
		DpStationPower::CI_Connection8Supplier, Qt::Key_K,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Alter Level 8 Consumer"),
		DpStationPower::CI_Connection8Consumer, Qt::Key_Comma,
		Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
		DiControlInput::JT_None, 0, 100, false),
};
int s_NumPowerConsoleDefaults = sizeof(s_PowerConsoleDefaults)/sizeof(DiControlInput);

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DpStationPower::DpStationPower() : DpStation()
{
	m_iSystemCount = 0;	// Information needs to be gathered
	
	m_PowerPlantShunts = 0;

	m_iSelectedSupplier = -1;	// default to non-valid value

	for(int iArrayIndex = 0; iArrayIndex < MAX_DISPLAYED_SYSTEMS; iArrayIndex++)
	{
		m_q8SupplierConnectionConsumer[iArrayIndex] = 0;
		m_q8SupplierPower[iArrayIndex] = 0;
		m_SupplierType[iArrayIndex] = SYS_Count;
		m_SupplierDamage[iArrayIndex] = 0;
		m_iSupplierIndex[iArrayIndex] = 0;

		m_q8ConsumerConnectionAndPower[iArrayIndex] = 0;
		m_ConsumerType[iArrayIndex] = SYS_Count;
		m_iConsumerIndex[iArrayIndex] = 0;
	}

	m_LastPowerPlantHealth = -1;
	m_LastPowerPlantShunts = -1;
	m_ScreenTex = 0;
}

DpStationPower::~DpStationPower()
{
}

DpStationPower::DpStationPower(const DpStationPower& c) : DpStation(c)
{
	// Server Variables
	m_iSystemCount = c.m_iSystemCount;

	m_PowerPlantShunts = c.m_PowerPlantShunts;

	m_iSelectedSupplier = c.m_iSelectedSupplier;

	// Client Variables
	for(int iArrayIndex = 0; iArrayIndex < MAX_DISPLAYED_SYSTEMS; iArrayIndex++)
	{
		m_q8SupplierConnectionConsumer[iArrayIndex] = c.m_q8SupplierConnectionConsumer[iArrayIndex];
		m_q8SupplierPower[iArrayIndex] = c.m_q8SupplierPower[iArrayIndex];
		m_SupplierType[iArrayIndex] = c.m_SupplierType[iArrayIndex];
		m_SupplierDamage[iArrayIndex] = c.m_SupplierDamage[iArrayIndex];
		m_iSupplierIndex[iArrayIndex] = c.m_iSupplierIndex[iArrayIndex];
	}
	m_LastPowerPlantHealth = c.m_LastPowerPlantHealth;
	m_LastPowerPlantShunts = c.m_LastPowerPlantShunts;
	for(int iArrayIndex = 0; iArrayIndex < MAX_DISPLAYED_SYSTEMS; iArrayIndex++)
	{
		m_q8ConsumerConnectionAndPower[iArrayIndex] = c.m_q8ConsumerConnectionAndPower[iArrayIndex];
		m_ConsumerType[iArrayIndex] = c.m_ConsumerType[iArrayIndex];
		m_iConsumerIndex[iArrayIndex] = c.m_iConsumerIndex[iArrayIndex];
	}
	m_ScreenTex = 0;
}

void DpStationPower::Rx(QDataStream& in, DaCapsule& capsule)
{
	DpStation::Rx(in, capsule);
	in >> m_iSystemCount;
	quint8 hasWhat;
	in >> hasWhat;
	if(hasWhat & 0x10)
	{
		// Connection information changed.  Update it.
		for(int iSupplierIndex = 0; iSupplierIndex < MAX_DISPLAYED_SYSTEMS; iSupplierIndex++)
			in >> m_q8SupplierConnectionConsumer[iSupplierIndex];
	}
	if(hasWhat & 0x8)
	{
		// Power Plant Information Changed.  Update it.
		quint8 obsolete;
		in >> obsolete;
	}
	if(hasWhat & 0x4)
	{
		// Shunt Information Changed.  Update it.
		in >> m_PowerPlantShunts;
	}
	if(hasWhat & 0x2)
	{
		// Consumer Information Changed.  Update it.
		for(int iConsumerIndex = 0; iConsumerIndex < MAX_DISPLAYED_SYSTEMS; iConsumerIndex++)
		{
			in >> m_q8ConsumerConnectionAndPower[iConsumerIndex];
			quint8 ui8;
			in >> ui8;
			m_ConsumerType[iConsumerIndex] = (SYSTEM)ui8;
			in >> m_iConsumerIndex[iConsumerIndex];
		}
	}
	if(hasWhat & 0x1)
	{
		// Supplier Information Changed.  Update it.
		for(int iSupplierIndex = 0; iSupplierIndex < MAX_DISPLAYED_SYSTEMS; iSupplierIndex++)
		{
			in >> m_q8SupplierPower[iSupplierIndex];
			quint8 ui8;
			in >> ui8;
			m_SupplierType[iSupplierIndex] = (SYSTEM)ui8;
			in >> m_SupplierDamage[iSupplierIndex];
			in >> m_iSupplierIndex[iSupplierIndex];
		}
	}
}

void DpStationPower::Tx(QDataStream& out) const
{
	DpStation::Tx(out);
	out << m_iSystemCount;

	quint8 hasWhat = 0x17;	// Send everything until we can pare this down intelligently.

	out << hasWhat;
	if(hasWhat & 0x10)
	{
		// Connection information changed.  Update it.
		for(int iSupplierIndex = 0; iSupplierIndex < MAX_DISPLAYED_SYSTEMS; iSupplierIndex++)
			out << m_q8SupplierConnectionConsumer[iSupplierIndex];
	}
	if(hasWhat & 0x8)
	{
		// Power Plant Information Changed.  Update it.
		quint8 obsolete = 0;
		out << obsolete;
	}
	if(hasWhat & 0x4)
	{
		// Shunt Information Changed.  Update it.
		out << m_PowerPlantShunts;
	}
	if(hasWhat & 0x2)
	{
		// Consumer Information Changed.  Update it.
		for(int iConsumerIndex = 0; iConsumerIndex < MAX_DISPLAYED_SYSTEMS; iConsumerIndex++)
		{
			out << m_q8ConsumerConnectionAndPower[iConsumerIndex];
			out << (quint8)m_ConsumerType[iConsumerIndex];
			out << m_iConsumerIndex[iConsumerIndex];
		}
	}
	if(hasWhat & 0x1)
	{
		// Supplier Information Changed.  Update it.
		for(int iSupplierIndex = 0; iSupplierIndex < MAX_DISPLAYED_SYSTEMS; iSupplierIndex++)
		{
			out << m_q8SupplierPower[iSupplierIndex];
			out << (quint8)m_SupplierType[iSupplierIndex];
			out << m_SupplierDamage[iSupplierIndex];
			out << m_iSupplierIndex[iSupplierIndex];
		}
	}
}

// Refresh data from valid counterpart.
void DpStationPower::Refresh(const DpStation* pCon, const DaAtom& parent)
{
	DpStation::Refresh(pCon, parent);		// Refresh base class.

	const DpStationPower* pA = (const DpStationPower*)pCon;
	//////////////////////////////////////////////////////////////////////////
	// Server Variables
	m_iSystemCount = pA->m_iSystemCount;

	m_PowerPlantShunts = pA->m_PowerPlantShunts;

	m_iSelectedSupplier = pA->m_iSelectedSupplier;

	for(int iArrayIndex = 0; iArrayIndex < MAX_DISPLAYED_SYSTEMS; iArrayIndex++)
	{
		m_q8SupplierConnectionConsumer[iArrayIndex] = pA->m_q8SupplierConnectionConsumer[iArrayIndex];
		m_q8SupplierPower[iArrayIndex] = pA->m_q8SupplierPower[iArrayIndex];
		m_SupplierType[iArrayIndex] = pA->m_SupplierType[iArrayIndex];
		m_SupplierDamage[iArrayIndex] = pA->m_SupplierDamage[iArrayIndex];
		m_iSupplierIndex[iArrayIndex] = pA->m_iSupplierIndex[iArrayIndex];

		m_q8ConsumerConnectionAndPower[iArrayIndex] = pA->m_q8ConsumerConnectionAndPower[iArrayIndex];
		m_ConsumerType[iArrayIndex] = pA->m_ConsumerType[iArrayIndex];
		m_iConsumerIndex[iArrayIndex] = pA->m_iConsumerIndex[iArrayIndex];
	}
}

bool DpStationPower::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now, DaPerson* person)
{
	DaVessel* ship = dynamic_cast<DaVessel*>(person->Parent());
	if(!ship || !ship->IsDesignDataCacheValid())
		return false;

//qDebug("DpStationPower::Tick SystemCount = %d", m_iSystemCount);	
	// Initialize data if necessary
	if(!m_iSystemCount)
	{
		m_iSystemCount = ship->GetSystemCount();
		for(int sysIdx = 0; sysIdx < MAX_DISPLAYED_SYSTEMS; sysIdx++)
		{
			if(sysIdx < m_iSystemCount)
			{
				m_iSupplierIndex[sysIdx] = sysIdx;
				m_iConsumerIndex[sysIdx] = sysIdx;
			}
			else
			{
				m_iSupplierIndex[sysIdx] = -1;	// No system
				m_iConsumerIndex[sysIdx] = -1;	// No system
			}
		}
		
		m_PowerPlantShunts = ship->GetShunts();
	}
	
	// Collect display information from the Ship
	if(m_iSystemCount && !last.SameStartSec(now))
		ship->UpdateDisplayedSystems(m_iSupplierIndex, m_SupplierType, m_SupplierDamage, m_q8SupplierPower,
				m_q8SupplierConnectionConsumer, m_iConsumerIndex, m_ConsumerType, m_q8ConsumerConnectionAndPower,
				m_PowerPlantShunts);

	return true;
}

// Called between rendering scene frames that contains this object.
void DpStationPower::AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last,
		const AuPoint& location, const DaCreature& sceneAtom)
{
	// Base class animates the lower console pane.
	DpStation::AnimateDeviceObjects(re, ship, last, location, sceneAtom);

	// Get the texture because we're going to make several changes to it.
	QString name;
	name = QString("DCon%1").arg(ship.Id().GetID());
	AuGLTexture *pTex = re.FindTexture(name);
	if(!pTex)
		return;

	s_Anim3++;
	if(s_Anim3 >= 3)
		s_Anim3 = 0;
	s_Anim8++;
	if(s_Anim8 >= 8)
		s_Anim8 = 0;
	s_Anim12++;
	if(s_Anim12 >= 12)
		s_Anim12 = 0;

	// Collect as much information from the ship as we can, stuff that doesn't need to be transmitted by this console then.
	quint8 powerPlantHealth;
	{
		unsigned int hitsLeft = ship.SystemHitsRemaining(SYS_PowerPlant);
		unsigned int ppUnits = ship.PowerPlantUnits();
		if(m_PowerPlantShunts && hitsLeft < ppUnits)
		{
			// Add boost due to power plant shunts. Each shunt supplies 1/8 of the designed power plant's output.
			hitsLeft += DaVessel::PowerPlantShuntsEnabled(m_PowerPlantShunts) * ppUnits / 8;
			if(hitsLeft > ppUnits)
				hitsLeft = ppUnits;
		}
		if(hitsLeft >= ppUnits)
			powerPlantHealth = 255;
		else
			powerPlantHealth = 255 * hitsLeft / ppUnits;
	}

	QImage bmbut;
	bool supplierChange = true;
	if(supplierChange)
	{
		//////////////////////////////////////////////////////////////////////////
		// Power Levels
		const char* bmNames[] =
		{
			":data/dc_SourcePower.bmp",
			":data/dc_SourcePower1.bmp",
			":data/dc_SourcePower2.bmp",
		};
		const char* bmNamesO[] =
		{
			":data/dc_SourcePowerO.bmp",
			":data/dc_SourcePower1O.bmp",
			":data/dc_SourcePower2O.bmp",
		};
		const int iPowerLevelHeight = 32;
		const int iPowerLevelWidth = 7;
		for(int iSupplierIndex = 0; iSupplierIndex < MAX_DISPLAYED_SYSTEMS && iSupplierIndex < m_iSystemCount; iSupplierIndex++)
		{
			QPixmap pmf(iPowerLevelWidth,iPowerLevelHeight);
			if(!pmf.isNull())
			{
				//qDebug("supplier %d power 0x%x", iSupplierIndex, m_q8SupplierPower[iSupplierIndex]);
				const int iSupplierPower = m_q8SupplierPower[iSupplierIndex] & 0x1f;
				bool bOverclocked = (m_q8SupplierPower[iSupplierIndex] >> 5) ? true:false;
				
				QPainter p(&pmf);
				if(bOverclocked)
					bmbut.load(bmNamesO[s_Anim3]);	// Base Overclocked Image
				else
					bmbut.load(bmNames[s_Anim3]);		// Base Image
				p.drawImage(QPoint(0,0), bmbut);	// Put into buffer
				if(iSupplierPower < 0x1f)
				{
					p.setBrush(QBrush(QColor(0,0,0)));	// Prepare black brush
					p.setPen(QPen(QColor(0,0,0), 0.f));		// Prepare Black pen
					p.drawRect(2,iSupplierPower,iPowerLevelWidth-2,iPowerLevelHeight);
				}
				QImage bmf = pmf.toImage();
				pTex->ReplaceSubImage(re, bmf, 25 + (31 * iSupplierIndex), 69);	// Place in main drawing
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Power Suppliers
		for(int iSupplierIndex = 0; iSupplierIndex < MAX_DISPLAYED_SYSTEMS && iSupplierIndex < m_iSystemCount; iSupplierIndex++)
		{
			int iSupplierDamage = m_SupplierDamage[iSupplierIndex];
			SYSTEM iSupplierType = m_SupplierType[iSupplierIndex];
			SystemImage(iSupplierType, bmbut);
			
			// Type
			bool bNumberedSystem = false;
			switch(iSupplierType)
			{
			case SYS_Fuel:
			case SYS_ManeuverDrive:
			case SYS_Hold:
			case SYS_Hull:
			case SYS_LifeSupport:
			case SYS_PowerPlant:
			case SYS_Sensor:
			case SYS_Shield:
			case SYS_Capacitor:
			case SYS_Count:
				break;
			case SYS_CommandAndControl:
			case SYS_VehicleLaunch:
			case SYS_SurgeryUnit:
			case SYS_Transporter:
			case SYS_Turret:
			case SYS_Weapon:
			case SYS_FTLDrive:
			case SYS_VehicleLanding:
				bNumberedSystem = true;
				break;
			}

			// Overlay with arrows periodically to make them blink.
			// This is only necessary when clicking them will actually change to another system.
			if(s_Anim8 >= 5 && m_iSystemCount > MAX_DISPLAYED_SYSTEMS)
			{
				QPixmap pm(16,16);
				QPainter p(&pm);
				p.drawImage(QPoint(0,0), bmbut);		// Put into buffer
				QImage imArrows(":data/dc_SystemArrows.png");
				p.drawImage(QPoint(0,0), imArrows);	// Put into buffer
				bmbut = pm.toImage();
			}

			if(!iSupplierDamage)
			{	
				// System is too damaged to use as a power Supplier
				QPixmap pmfConsumer(16,16);
				if(!pmfConsumer.isNull())
				{	
					QPainter p(&pmfConsumer);
					p.drawImage(QPoint(0,0), bmbut);		// Put into buffer
					p.setBrush(QBrush(QColor(255,0,0,20 + s_Anim8*16)));	// Prepare Red Tint brush
					p.setPen(Qt::NoPen);					// Not using an outline pen
					p.drawRect(0,0,16,16);
					QImage bmf = pmfConsumer.toImage();
					pTex->ReplaceSubImage(re, bmf, 8 + (31 * iSupplierIndex), 85);
				}
				else
					pTex->ReplaceSubImage(re, bmbut, 8 + (31 * iSupplierIndex), 85);
			}
			else
			{
				// System is usable as a Supplier of power
				pTex->ReplaceSubImage(re, bmbut, 8 + (31 * iSupplierIndex), 85);
			}

			// Numbered Coupling
			QImage bmCoupling;
			bmCoupling.load(":data/dc_SourceCoupling.bmp");
			if(bNumberedSystem)
			{
				QPixmap pmfSystem(16,9);
				if(!pmfSystem.isNull())
				{	
					QPainter p(&pmfSystem);
					QFont font(p.font());
					font.setPixelSize(9);
					font.setBold(true);
					p.setFont(font);
					p.drawImage(QPoint(0,0), bmCoupling);	// Put into buffer
					p.setPen(QPen(QColor(0,0,0), 0.f));		// Prepare Black pen
					QString strNumber;
					strNumber.setNum(m_iSupplierIndex[iSupplierIndex]);
					int iStartPixel = 1;
					if(strNumber.length() == 1)
						iStartPixel = 5;
					p.drawText(iStartPixel-1,8-1,strNumber.right(2));
					p.drawText(iStartPixel+1,8-1,strNumber.right(2));
					p.drawText(iStartPixel-1,8,strNumber.right(2));
					p.drawText(iStartPixel+1,8,strNumber.right(2));
					p.drawText(iStartPixel-1,8+1,strNumber.right(2));
					p.drawText(iStartPixel+1,8+1,strNumber.right(2));
					p.setPen(QPen(QColor(128,128,0), 0.f));	// Prepare Gold pen
					p.drawText(iStartPixel,8,strNumber.right(2));

					QImage bmf = pmfSystem.toImage();
					pTex->ReplaceSubImage(re, bmf, 8 + (31 * iSupplierIndex), 76);
				}
				else
					pTex->ReplaceSubImage(re, bmCoupling, 8 + (31 * iSupplierIndex), 76);
			}
			else
				pTex->ReplaceSubImage(re, bmCoupling, 8 + (31 * iSupplierIndex), 76);

			// Damage to Supplier
			const int iDamageLevelHeight = 32;
			const int iDamageLevelWidth = 3;
			QPixmap pmf(iDamageLevelWidth,iDamageLevelHeight);
			if(!pmf.isNull())
			{	
				QPainter p(&pmf);
				bmbut.load(":data/dc_SourceDamage.bmp"); // Base Image
				p.drawImage(QPoint(0,0), bmbut);	// Put into buffer
				if(iSupplierDamage < 15)
				{
					const int iDamageLevel = iDamageLevelHeight * iSupplierDamage / 15;
					p.setBrush(QBrush(QColor(0,0,0)));	// Prepare black brush
					p.setPen(QPen(QColor(0,0,0), 0.f));		// Prepare Black pen
					p.drawRect(0,iDamageLevel,iDamageLevelWidth,iDamageLevelHeight);
				}

				QImage bmf = pmf.toImage();
				pTex->ReplaceSubImage(re, bmf, 4 + (31 * iSupplierIndex), 69);	// Place in main drawing
			}
		}
	}
	bool consumerChange = true;
	if(consumerChange)
	{
		//////////////////////////////////////////////////////////////////////////
		// Power Usage
		const char* bmNames[] =
		{
			":data/dc_DestinationPower.bmp",
			":data/dc_DestinationPower1.bmp",
			":data/dc_DestinationPower2.bmp",
		};
		const int iPowerLevelHeight = 32;
		const int iPowerLevelWidth = 7;
		for(int iConsumerIndex = 0; iConsumerIndex < MAX_DISPLAYED_SYSTEMS && iConsumerIndex < m_iSystemCount; iConsumerIndex++)
		{
			QPixmap pmf(iPowerLevelWidth,iPowerLevelHeight);
			if(!pmf.isNull())
			{	
				//qDebug("consumer %d power 0x%x", iConsumerIndex, m_q8ConsumerConnectionAndPower[iConsumerIndex]);
				const int iConsumerPower = m_q8ConsumerConnectionAndPower[iConsumerIndex] & 0x1f;

				QPainter p(&pmf);
				bmbut.load(bmNames[s_Anim3]);		// Base Image
				p.drawImage(QPoint(0,0), bmbut);	// Put into buffer
				if(iConsumerPower < 0x1f)
				{
					p.setBrush(QBrush(QColor(0,0,0)));	// Prepare black brush
					p.setPen(QPen(QColor(0,0,0), 0.f));		// Prepare Black pen
					p.drawRect(2,0,iPowerLevelWidth-2,iPowerLevelHeight - iConsumerPower-1);
				}
				QImage bmf = pmf.toImage();
				pTex->ReplaceSubImage(re, bmf, 25 + (31 * iConsumerIndex), 5);	// Place in main drawing
			}
		}

		//////////////////////////////////////////////////////////////////////////
		// Power Consumers
		for(int iConsumerIndex = 0; iConsumerIndex < MAX_DISPLAYED_SYSTEMS && iConsumerIndex < m_iSystemCount; iConsumerIndex++)
		{
			//int iConsumerDamage = m_ConsumerDamage[iConsumerIndex];
			SYSTEM iConsumerType = m_ConsumerType[iConsumerIndex];
			SystemImage(iConsumerType, bmbut);
			
			// Type
			bool bNumberedSystem = false;
			switch(iConsumerType)
			{
			case SYS_Fuel:
			case SYS_ManeuverDrive:
			case SYS_Hold:
			case SYS_Hull:
			case SYS_LifeSupport:
			case SYS_PowerPlant:
			case SYS_Sensor:
			case SYS_Shield:
			case SYS_Capacitor:
			case SYS_Count:
				break;
			case SYS_CommandAndControl:
			case SYS_VehicleLaunch:
			case SYS_SurgeryUnit:
			case SYS_Transporter:
			case SYS_Turret:
			case SYS_Weapon:
			case SYS_FTLDrive:
			case SYS_VehicleLanding:
				bNumberedSystem = true;
				break;
			}

			// Overlay with arrows periodically to make them blink.
			// This is only necessary when clicking them will actually change to another system.
			if(s_Anim8 >= 5 && m_iSystemCount > MAX_DISPLAYED_SYSTEMS)
			{
				QPixmap pm(16,16);
				QPainter p(&pm);
				p.drawImage(QPoint(0,0), bmbut);		// Put into buffer
				QImage imArrows(":data/dc_SystemArrows.png");
				p.drawImage(QPoint(0,0), imArrows);	// Put into buffer
				bmbut = pm.toImage();
			}

			if(!(m_q8ConsumerConnectionAndPower[iConsumerIndex] & 0x1f))
			{
				// System is not drawing any power	
				QPixmap pmfConsumer(16,16);
				if(!pmfConsumer.isNull())
				{	
					QPainter p(&pmfConsumer);
					p.drawImage(QPoint(0,0), bmbut);		// Put into buffer
					p.setBrush(QBrush(QColor(255,0,0,20 + s_Anim8*16)));	// Prepare Red Tint brush
					p.setPen(Qt::NoPen);					// Not using an outline pen
					p.drawRect(0,0,16,16);
					QImage bmf = pmfConsumer.toImage();
					pTex->ReplaceSubImage(re, bmf, 8 + (31 * iConsumerIndex), 5);
				}
				else
					pTex->ReplaceSubImage(re, bmbut, 8 + (31 * iConsumerIndex), 5);
			}
			else
			{
				// System is using power
				pTex->ReplaceSubImage(re, bmbut, 8 + (31 * iConsumerIndex), 5);
			}				

			// Numbered Coupling
			QImage bmCoupling;
			bmCoupling.load(":data/dc_DestinationCoupling.bmp");
			if(bNumberedSystem)
			{
				QPixmap pmfCoupling(16,9);
				if(!pmfCoupling.isNull())
				{	
					QPainter p(&pmfCoupling);
					QFont font(p.font());
					font.setPixelSize(9);
					font.setBold(true);
					p.setFont(font);
					p.drawImage(QPoint(0,0), bmCoupling);	// Put into buffer
					p.setPen(QPen(QColor(0,0,0), 0.f));		// Prepare Black pen
					QString strNumber;
					strNumber.setNum(m_iConsumerIndex[iConsumerIndex]);
					int iStartPixel = 1;
					if(strNumber.length() == 1)
						iStartPixel = 5;
					p.drawText(iStartPixel-1,8-1,strNumber.right(2));
					p.drawText(iStartPixel+1,8-1,strNumber.right(2));
					p.drawText(iStartPixel-1,8,strNumber.right(2));
					p.drawText(iStartPixel+1,8,strNumber.right(2));
					p.drawText(iStartPixel-1,8+1,strNumber.right(2));
					p.drawText(iStartPixel+1,8+1,strNumber.right(2));
					p.setPen(QPen(QColor(128,128,0), 0.f));	// Prepare Gold pen
					p.drawText(iStartPixel,8,strNumber.right(2));

					QImage bmfCoupling = pmfCoupling.toImage();
					pTex->ReplaceSubImage(re, bmfCoupling, 8 + (31 * iConsumerIndex), 21);
				}
				else
					pTex->ReplaceSubImage(re, bmCoupling, 8 + (31 * iConsumerIndex), 21);
			}
			else
				pTex->ReplaceSubImage(re, bmCoupling, 8 + (31 * iConsumerIndex), 21);

			// Damage to Consumer
			/*
			const int iDamageLevelHeight = 32;
			const int iDamageLevelWidth = 3;
			QPixmap pmf(iDamageLevelWidth,iDamageLevelHeight);
			if(!pmf.isNull())
			{
				QPainter p(&pmf);
				bmbut.load(":data/dc_DestinationDamage.bmp"); // Base Image
				p.drawImage(QPoint(0,0), bmbut);	// Put into buffer
				if(iConsumerDamage < 15)
				{
					const int iDamageLevel = iDamageLevelHeight * iConsumerDamage / 15;
					p.setBrush(QBrush(QColor(0,0,0)));	// Prepare black brush
					p.setPen(QPen(QColor(0,0,0), 0.f));		// Prepare Black pen
					p.drawRect(0,0,iDamageLevelWidth,iDamageLevelHeight - iDamageLevel-1);
				}
				QImage bmf = pmf.toImage();
				pTex->ReplaceSubImage(re, bmf, 4 + (31 * iConsumerIndex), 5);	// Place in main drawing
			}
			*/
		}
	}
	bool connectionChange = true;
	if(connectionChange)
	{
		//////////////////////////////////////////////////////////////////////////
		// Connections
		QPixmap pmf(":data/dc_TracesBg.png");	// 252x32 pixels This is where we draw the connection traces.

		// Draw supplier side connections and wiring.
		for(int iSupplierIndex = 0; iSupplierIndex < MAX_DISPLAYED_SYSTEMS && iSupplierIndex < m_iSystemCount; iSupplierIndex++)
		{
			if(m_q8SupplierConnectionConsumer[iSupplierIndex] & 0x20)
			{
				// Connected supplier.
				bmbut.load(":data/dc_SourceConnect.bmp");

				// Find each Consumer
				int iConsumer = m_q8SupplierConnectionConsumer[iSupplierIndex] & 0xF;

				// If the Consumer is not on screen draw the line to the middle
				// bottom of the connection area otherwise draw it to the Consumer
				// connection point
				if(!pmf.isNull())
				{
					int ly = 32 - (iSupplierIndex * 4 + 3);
					int sx = (31 * iSupplierIndex);
					QPainter p(&pmf);
					p.setPen(QPen(QColor(236,203,0), 0.f));
					if(iConsumer >= MAX_DISPLAYED_SYSTEMS)
					{
						// Line to off-screen connection point
						ly--;	// Offset up a pixel when going off-screen to the right.
						p.drawLine(10 + sx, 0, 10 + sx, ly);
						p.drawLine(10 + sx, ly, 0, ly);
						p.setPen(QPen(QColor(128,110,0), 0.f));
						ly++;
						p.drawLine(11 + sx, 0, 11 + sx, ly);
						p.drawLine(11 + sx, ly, 0, ly);
					}
					else
					{
						// Line to Consumer Connection point
						int cx = (31 * iConsumer);
						p.drawLine(10 + sx, 0, 10 + sx, ly);
						p.drawLine(10 + sx, ly, 16 + cx, ly);
						p.drawLine(16 + cx, ly, 16 + cx, 32);
						p.setPen(QPen(QColor(128,110,0), 0.f));
						ly++;
						p.drawLine(11 + sx, 0, 11 + sx, ly);
						p.drawLine(11 + sx, ly, 17 + cx, ly);
						p.drawLine(17 + cx, ly, 17 + cx, 32);
					}
				}
			}
			else
				bmbut.load(":data/dc_SourceDisconnect.bmp");	// Disconnected supplier.
			pTex->ReplaceSubImage(re, bmbut, 8 + (31 * iSupplierIndex), 69);
		}

		// Draw consumer side connections and wiring.
		for(int iConsumerIndex = 0; iConsumerIndex < MAX_DISPLAYED_SYSTEMS && iConsumerIndex < m_iSystemCount; iConsumerIndex++)
		{
			if(m_q8ConsumerConnectionAndPower[iConsumerIndex] & 0x40)
			{
				// Connected consumer.
				bmbut.load(":data/dc_DestinationConnect.bmp");
				if(m_q8ConsumerConnectionAndPower[iConsumerIndex] & 0x80)
				{
					// Connected to an off-screen supplier. May also be connected to an on-screen supplier but the suppliers
					// section took care of drawing those lines.
					QPainter p(&pmf);
					p.setPen(QPen(QColor(236,203,0), 0.f));
					// Line to off-screen connection point
					int ly = 32 - (iConsumerIndex * 4 + 3) + 1;	// Offset down a pixel going off-screen to the left.
					int cx = (31 * iConsumerIndex);
					p.drawLine(252, ly, 16 + cx, ly);
					p.drawLine(16 + cx, ly, 16 + cx, 32);
					p.setPen(QPen(QColor(128,110,0), 0.f));
					ly++;
					p.drawLine(252, ly, 17 + cx, ly);
					p.drawLine(17 + cx, ly, 17 + cx, 32);
				}
			}
			else
				bmbut.load(":data/dc_DestinationDisconnect.bmp");	// Disconnected consumer.
			pTex->ReplaceSubImage(re, bmbut, 8 + (31 * iConsumerIndex), 30);
		}

		QImage bmf = pmf.toImage();
		pTex->ReplaceSubImage(re, bmf, 2, 37);
	}

	// Power Plant Shunts
	if(m_LastPowerPlantShunts != m_PowerPlantShunts || m_PowerPlantShunts)	// Shunt Change or a shunt is on
	{
		const char* bmNames[] =
		{
			":data/dc_ShuntOn.bmp",
			":data/dc_ShuntOn.bmp",
			":data/dc_ShuntOn.bmp",
			":data/dc_ShuntOn.bmp",
			":data/dc_ShuntOn.bmp",
			":data/dc_ShuntOn.bmp",
			":data/dc_ShuntOn.bmp",
			":data/dc_ShuntOn.bmp",
			":data/dc_ShuntOn1.bmp",
			":data/dc_ShuntOn2.bmp",
			":data/dc_ShuntOn2.bmp",
			":data/dc_ShuntOn1.bmp",
		};
		m_LastPowerPlantShunts = m_PowerPlantShunts;
		for(int shuntIndex = 0; shuntIndex < MAX_POWER_PLANT_SHUNTS; shuntIndex++)
		{
			// Taking care of the change now.
			if(m_PowerPlantShunts & (1<<shuntIndex))
				bmbut.load(bmNames[s_Anim12]);
			else
				bmbut.load(":data/dc_ShuntOff.bmp");
			pTex->ReplaceSubImage(re, bmbut, 32 + (26 * shuntIndex), 105);
		}
	}

	// Power plant health.
	if(m_LastPowerPlantHealth != powerPlantHealth)
	{
		m_LastPowerPlantHealth = powerPlantHealth;
		const int iDamageLevelHeight = 8;
		const int iDamageLevelWidth = 210;
		QPixmap pmf(iDamageLevelWidth,iDamageLevelHeight);
		if(!pmf.isNull())
		{	
			const int iDamageLevel = iDamageLevelWidth * powerPlantHealth / 255;

			QPainter p(&pmf);
			bmbut.load(":data/dc_PowerPlantDamage.bmp"); // Base Image
			p.drawImage(QPoint(0,0), bmbut);	// Put into buffer
			p.setBrush(QBrush(QColor(0,0,0)));	// Prepare black brush
			p.setPen(QPen(QColor(0,0,0), 0.f));		// Prepare Black pen
			p.drawRect(iDamageLevel,0,iDamageLevelWidth,iDamageLevelHeight);

			QImage bmf = pmf.toImage();
			pTex->ReplaceSubImage(re, bmf, 32, 117);	// Place in main drawing
		}
	}

	// Power plant activity.
	if(ship.PowerPlantWorking())
	{
		const char* bmNames[] =
		{
			":data/dc_PowerPlant1.bmp",
			":data/dc_PowerPlant1.bmp",
			":data/dc_PowerPlant2.bmp",
			":data/dc_PowerPlant2.bmp",
			":data/dc_PowerPlant3.bmp",
			":data/dc_PowerPlant3.bmp",
			":data/dc_PowerPlant4.bmp",
			":data/dc_PowerPlant4.bmp",
		};
		bmbut.load(bmNames[s_Anim8]);
		if(m_PowerPlantShunts)
		{
			QPixmap pmPower(bmbut.width(),bmbut.height());
			QPixmap pmPowerRed(":data/dc_PowerPlantRed.png");
			QPainter p(&pmPower);
			p.drawImage(QPoint(0,0), bmbut);		// Put into buffer
			p.drawPixmap(QPoint(0,0), pmPowerRed);	// Overlay with red transparency
			QImage bmPower = pmPower.toImage();
			pTex->ReplaceSubImage(re, bmPower, 6, 105);
		}
		else
			pTex->ReplaceSubImage(re, bmbut, 6, 105);
	}
	else
	{
		bmbut.load(":data/dc_PowerPlant0.bmp");
		pTex->ReplaceSubImage(re, bmbut, 6, 105);
	}
}

void DpStationPower::ConfigureControls(const DiControlInputArray &aAllControls)
{
	// Update my controls with those passed to me.
	for(DiControlInputArray::const_iterator ci = aAllControls.constBegin(); ci != aAllControls.constEnd(); ci++)
	{
		// Locate the corresponding control in my controls array.
		for(DiControlInputArray::iterator myci = s_PowerConsoleControls.begin(); myci != s_PowerConsoleControls.end(); myci++)
		{
			if((*myci).GetId() == (*ci).GetId())
			{
				// Found the corresponding control id. Copy in the nu data.
				*myci = *ci;
				break;
			}
		}
	}
	SaveControls();
}

void DpStationPower::RestoreDefaultControls()
{
	s_PowerConsoleControls.clear();
	for(int ic=0; ic<s_NumPowerConsoleDefaults; ic++)
		s_PowerConsoleControls.append(s_PowerConsoleDefaults[ic]);
	SaveControls();
}

void DpStationPower::LoadControls() const
{
	// Init with my default controls.
	s_PowerConsoleControls.clear();
	for(int ic=0; ic<s_NumPowerConsoleDefaults; ic++)
		s_PowerConsoleControls.append(s_PowerConsoleDefaults[ic]);

	// Load any overrides from the settings file.
	s_PowerConsoleControls.LoadSettings("Controls/Power Console");
}

void DpStationPower::SaveControls() const
{
	// Store my controls in the settings file.
	s_PowerConsoleControls.SaveSettings("Controls/Power Console");
}

// A scene atom should fill this array with control inputs it can accept.
void DpStationPower::EnumControlInputs(DiControlInputArray& aAllControls) const
{
	// Load my static controls array if it hasn't been loaded yet. This data will only exist on the client since EnumControlInputs
	// is not called on the server.
	if(s_PowerConsoleControls.isEmpty())
		LoadControls();

	// Append my controls to the array.
	for(DiControlInputArray::const_iterator ci = s_PowerConsoleControls.constBegin(); ci != s_PowerConsoleControls.constEnd(); ci++)
		aAllControls.append(*ci);
}

// This is how all control inputs are passed to an atom.
// Only control inputs that have been returned by EnumControlInputs will be
// passed.
// Only control inputs that have been activated will be passed.
void DpStationPower::ProcessControlInput(const DiControlInputArray& aActiveControls, DaPerson* sceneAtom, DaVessel& ship,
		CONTROL_CONTEXT cc, const AuPoint& /*location*/)
{
	if(ship.IsNew())
		return;	// This console cannot be used when the ship is "nu".

	for(int ic=0; ic<aActiveControls.count(); ic++)
	{
		const DiControlInput &ci = aActiveControls[ic];
		switch(ci.GetId())
		{
		// Treat the overclock ups and downs as toggles since we don't support partial overclocking.
		case CI_AlterLevel1Up:
			if(cc != CONTROL_Client)
				ToggleOverclock(CI_AlterLevel1Up - CI_AlterLevel1Up, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Overclock, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_AlterLevel2Up:
			if(cc != CONTROL_Client)
				ToggleOverclock(CI_AlterLevel2Up - CI_AlterLevel1Up, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Overclock, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_AlterLevel3Up:
			if(cc != CONTROL_Client)
				ToggleOverclock(CI_AlterLevel3Up - CI_AlterLevel1Up, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Overclock, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_AlterLevel4Up:
			if(cc != CONTROL_Client)
				ToggleOverclock(CI_AlterLevel4Up - CI_AlterLevel1Up, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Overclock, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_AlterLevel5Up:
			if(cc != CONTROL_Client)
				ToggleOverclock(CI_AlterLevel5Up - CI_AlterLevel1Up, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Overclock, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_AlterLevel6Up:
			if(cc != CONTROL_Client)
				ToggleOverclock(CI_AlterLevel6Up - CI_AlterLevel1Up, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Overclock, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_AlterLevel7Up:
			if(cc != CONTROL_Client)
				ToggleOverclock(CI_AlterLevel7Up - CI_AlterLevel1Up, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Overclock, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_AlterLevel8Up:
			if(cc != CONTROL_Client)
				ToggleOverclock(CI_AlterLevel8Up - CI_AlterLevel1Up, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Overclock, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_AlterLevel1Down:
			if(cc != CONTROL_Client)
				ToggleOverclock(CI_AlterLevel1Down - CI_AlterLevel1Down, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Overclock, 1,.8f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_AlterLevel2Down:
			if(cc != CONTROL_Client)
				ToggleOverclock(CI_AlterLevel2Down - CI_AlterLevel1Down, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Overclock, 1,.8f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_AlterLevel3Down:
			if(cc != CONTROL_Client)
				ToggleOverclock(CI_AlterLevel3Down - CI_AlterLevel1Down, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Overclock, 1,.8f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_AlterLevel4Down:
			if(cc != CONTROL_Client)
				ToggleOverclock(CI_AlterLevel4Down - CI_AlterLevel1Down, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Overclock, 1,.8f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_AlterLevel5Down:
			if(cc != CONTROL_Client)
				ToggleOverclock(CI_AlterLevel5Down - CI_AlterLevel1Down, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Overclock, 1,.8f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_AlterLevel6Down:
			if(cc != CONTROL_Client)
				ToggleOverclock(CI_AlterLevel6Down - CI_AlterLevel1Down, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Overclock, 1,.8f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_AlterLevel7Down:
			if(cc != CONTROL_Client)
				ToggleOverclock(CI_AlterLevel7Down - CI_AlterLevel1Down, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Overclock, 1,.8f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_AlterLevel8Down:
			if(cc != CONTROL_Client)
				ToggleOverclock(CI_AlterLevel8Down - CI_AlterLevel1Down, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Overclock, 1,.8f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		// Power Supplier
		case CI_Supplier1Up:
			if(cc != CONTROL_Client)
				FindPreviousSupplier(CI_Supplier1Up - CI_Supplier1Up);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Supplier2Up:
			if(cc != CONTROL_Client)
				FindPreviousSupplier(CI_Supplier2Up - CI_Supplier1Up);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Supplier3Up:
			if(cc != CONTROL_Client)
				FindPreviousSupplier(CI_Supplier3Up - CI_Supplier1Up);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Supplier4Up:
			if(cc != CONTROL_Client)
				FindPreviousSupplier(CI_Supplier4Up - CI_Supplier1Up);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Supplier5Up:
			if(cc != CONTROL_Client)
				FindPreviousSupplier(CI_Supplier5Up - CI_Supplier1Up);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Supplier6Up:
			if(cc != CONTROL_Client)
				FindPreviousSupplier(CI_Supplier6Up - CI_Supplier1Up);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Supplier7Up:
			if(cc != CONTROL_Client)
				FindPreviousSupplier(CI_Supplier7Up - CI_Supplier1Up);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Supplier8Up:
			if(cc != CONTROL_Client)
				FindPreviousSupplier(CI_Supplier8Up - CI_Supplier1Up);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Supplier1Down:
			if(cc != CONTROL_Client)
				FindNextSupplier(CI_Supplier1Down - CI_Supplier1Down);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Supplier2Down:
			if(cc != CONTROL_Client)
				FindNextSupplier(CI_Supplier2Down - CI_Supplier1Down);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Supplier3Down:
			if(cc != CONTROL_Client)
				FindNextSupplier(CI_Supplier3Down - CI_Supplier1Down);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Supplier4Down:
			if(cc != CONTROL_Client)
				FindNextSupplier(CI_Supplier4Down - CI_Supplier1Down);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Supplier5Down:
			if(cc != CONTROL_Client)
				FindNextSupplier(CI_Supplier5Down - CI_Supplier1Down);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Supplier6Down:
			if(cc != CONTROL_Client)
				FindNextSupplier(CI_Supplier6Down - CI_Supplier1Down);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Supplier7Down:
			if(cc != CONTROL_Client)
				FindNextSupplier(CI_Supplier7Down - CI_Supplier1Down);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Supplier8Down:
			if(cc != CONTROL_Client)
				FindNextSupplier(CI_Supplier8Down - CI_Supplier1Down);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		// Power Connection
		case CI_Connection1Supplier:
			if(cc != CONTROL_Client)
				SupplierSelection(CI_Connection1Supplier - CI_Connection1Supplier, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Connection2Supplier:
			if(cc != CONTROL_Client)
				SupplierSelection(CI_Connection2Supplier - CI_Connection1Supplier, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Connection3Supplier:
			if(cc != CONTROL_Client)
				SupplierSelection(CI_Connection3Supplier - CI_Connection1Supplier, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Connection4Supplier:
			if(cc != CONTROL_Client)
				SupplierSelection(CI_Connection4Supplier - CI_Connection1Supplier, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Connection5Supplier:
			if(cc != CONTROL_Client)
				SupplierSelection(CI_Connection5Supplier - CI_Connection1Supplier, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Connection6Supplier:
			if(cc != CONTROL_Client)
				SupplierSelection(CI_Connection6Supplier - CI_Connection1Supplier, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Connection7Supplier:
			if(cc != CONTROL_Client)
				SupplierSelection(CI_Connection7Supplier - CI_Connection1Supplier, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Connection8Supplier:
			if(cc != CONTROL_Client)
				SupplierSelection(CI_Connection8Supplier - CI_Connection1Supplier, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Supplier, 1,.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Connection1Consumer:
			if(cc != CONTROL_Client)
				ConsumerSelection(CI_Connection1Consumer - CI_Connection1Consumer, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,1.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Connection2Consumer:
			if(cc != CONTROL_Client)
				ConsumerSelection(CI_Connection2Consumer - CI_Connection1Consumer, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,1.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Connection3Consumer:
			if(cc != CONTROL_Client)
				ConsumerSelection(CI_Connection3Consumer - CI_Connection1Consumer, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,1.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Connection4Consumer:
			if(cc != CONTROL_Client)
				ConsumerSelection(CI_Connection4Consumer - CI_Connection1Consumer, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,1.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Connection5Consumer:
			if(cc != CONTROL_Client)
				ConsumerSelection(CI_Connection5Consumer - CI_Connection1Consumer, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,1.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Connection6Consumer:
			if(cc != CONTROL_Client)
				ConsumerSelection(CI_Connection6Consumer - CI_Connection1Consumer, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,1.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Connection7Consumer:
			if(cc != CONTROL_Client)
				ConsumerSelection(CI_Connection7Consumer - CI_Connection1Consumer, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,1.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Connection8Consumer:
			if(cc != CONTROL_Client)
				ConsumerSelection(CI_Connection8Consumer - CI_Connection1Consumer, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,1.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		// Power Consumer
		case CI_Consumer1Up:
			if(cc != CONTROL_Client)
				FindPreviousConsumer(CI_Consumer1Up - CI_Consumer1Up);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Consumer2Up:
			if(cc != CONTROL_Client)
				FindPreviousConsumer(CI_Consumer2Up - CI_Consumer1Up);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Consumer3Up:
			if(cc != CONTROL_Client)
				FindPreviousConsumer(CI_Consumer3Up - CI_Consumer1Up);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Consumer4Up:
			if(cc != CONTROL_Client)
				FindPreviousConsumer(CI_Consumer4Up - CI_Consumer1Up);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Consumer5Up:
			if(cc != CONTROL_Client)
				FindPreviousConsumer(CI_Consumer5Up - CI_Consumer1Up);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Consumer6Up:
			if(cc != CONTROL_Client)
				FindPreviousConsumer(CI_Consumer6Up - CI_Consumer1Up);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Consumer7Up:
			if(cc != CONTROL_Client)
				FindPreviousConsumer(CI_Consumer7Up - CI_Consumer1Up);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Consumer8Up:
			if(cc != CONTROL_Client)
				FindPreviousConsumer(CI_Consumer8Up - CI_Consumer1Up);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Consumer1Down:
			if(cc != CONTROL_Client)
				FindNextConsumer(CI_Consumer1Down - CI_Consumer1Down);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Consumer2Down:
			if(cc != CONTROL_Client)
				FindNextConsumer(CI_Consumer2Down - CI_Consumer1Down);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Consumer3Down:
			if(cc != CONTROL_Client)
				FindNextConsumer(CI_Consumer3Down - CI_Consumer1Down);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Consumer4Down:
			if(cc != CONTROL_Client)
				FindNextConsumer(CI_Consumer4Down - CI_Consumer1Down);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Consumer5Down:
			if(cc != CONTROL_Client)
				FindNextConsumer(CI_Consumer5Down - CI_Consumer1Down);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Consumer6Down:
			if(cc != CONTROL_Client)
				FindNextConsumer(CI_Consumer6Down - CI_Consumer1Down);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Consumer7Down:
			if(cc != CONTROL_Client)
				FindNextConsumer(CI_Consumer7Down - CI_Consumer1Down);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Consumer8Down:
			if(cc != CONTROL_Client)
				FindNextConsumer(CI_Consumer8Down - CI_Consumer1Down);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Consumer, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;

		// Power Shunts
		case CI_PowerShunt1:
			if(cc != CONTROL_Client)
				TogglePowerPlantShunt(CI_PowerShunt1 - CI_PowerShunt1, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shunt, 1,.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_PowerShunt2:
			if(cc != CONTROL_Client)
				TogglePowerPlantShunt(CI_PowerShunt2 - CI_PowerShunt1, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shunt, 1,.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_PowerShunt3:
			if(cc != CONTROL_Client)
				TogglePowerPlantShunt(CI_PowerShunt3 - CI_PowerShunt1, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shunt, 1,.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_PowerShunt4:
			if(cc != CONTROL_Client)
				TogglePowerPlantShunt(CI_PowerShunt4 - CI_PowerShunt1, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shunt, 1,.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_PowerShunt5:
			if(cc != CONTROL_Client)
				TogglePowerPlantShunt(CI_PowerShunt5 - CI_PowerShunt1, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shunt, 1,.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_PowerShunt6:
			if(cc != CONTROL_Client)
				TogglePowerPlantShunt(CI_PowerShunt6 - CI_PowerShunt1, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shunt, 1,.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_PowerShunt7:
			if(cc != CONTROL_Client)
				TogglePowerPlantShunt(CI_PowerShunt7 - CI_PowerShunt1, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shunt, 1,.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_PowerShunt8:
			if(cc != CONTROL_Client)
				TogglePowerPlantShunt(CI_PowerShunt8 - CI_PowerShunt1, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Shunt, 1,.5f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		}
	}
}

// This is how mouse control inputs activate controls in the control input array.
// Controls in aActiveControls are activated accordingly.
void DpStationPower::SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc,
		DiControlInputArray &aActiveControls, bool bContinuous)
{
	if(idPanel)
		return;
	double panWid, panHt;
	PanelDimensions(panWid, panHt);
	int ix = (int)(fPanelLoc.x * 256 / panWid);	// X pixel in texture map.
	int iy = (int)(fPanelLoc.y * 128 / panHt);
	QPoint pxHit(ix, iy);

	// Note that the top/bottom is reversed in the rectangles. This works with the iy
	// we've calculated that is also measured from the bottom, not the top as usual in GDI.
	DiPanelControl acontrols[] =
	{
		// Power Level ( 8 x 32 Rectangle )
		// These act as overclocking toggles so the "Down"s are not needed.
		DiPanelControl(CI_AlterLevel1Up, QRect(25,68, 8,32)),
		DiPanelControl(CI_AlterLevel2Up, QRect(56,68, 8,32)),
		DiPanelControl(CI_AlterLevel3Up, QRect(87,68, 8,32)),
		DiPanelControl(CI_AlterLevel4Up, QRect(118,68, 8,32)),
		DiPanelControl(CI_AlterLevel5Up, QRect(149,68, 8,32)),
		DiPanelControl(CI_AlterLevel6Up, QRect(180,68, 8,32)),
		DiPanelControl(CI_AlterLevel7Up, QRect(211,68, 8,32)),
		DiPanelControl(CI_AlterLevel8Up, QRect(242,68, 8,32)),

		// Power Supplier ( Pairs of 8 x 16 rectangles side-by-side )
		DiPanelControl(CI_Supplier1Down, QRect(16,84, 8,16)),
		DiPanelControl(CI_Supplier2Down, QRect(47,84, 8,16)),
		DiPanelControl(CI_Supplier3Down, QRect(78,84, 8,16)),
		DiPanelControl(CI_Supplier4Down, QRect(109,84, 8,16)),
		DiPanelControl(CI_Supplier5Down, QRect(140,84, 8,16)),
		DiPanelControl(CI_Supplier6Down, QRect(171,84, 8,16)),
		DiPanelControl(CI_Supplier7Down, QRect(202,84, 8,16)),
		DiPanelControl(CI_Supplier8Down, QRect(233,84, 8,16)),
		DiPanelControl(CI_Supplier1Up, QRect(8,84, 8,16)),
		DiPanelControl(CI_Supplier2Up, QRect(39,84, 8,16)),
		DiPanelControl(CI_Supplier3Up, QRect(70,84, 8,16)),
		DiPanelControl(CI_Supplier4Up, QRect(101,84, 8,16)),
		DiPanelControl(CI_Supplier5Up, QRect(132,84, 8,16)),
		DiPanelControl(CI_Supplier6Up, QRect(163,84, 8,16)),
		DiPanelControl(CI_Supplier7Up, QRect(194,84, 8,16)),
		DiPanelControl(CI_Supplier8Up, QRect(225,84, 8,16)),

		// Power Connections
		DiPanelControl(CI_Connection1Supplier, QRect(8,64, 16,20)),
		DiPanelControl(CI_Connection2Supplier, QRect(39,64, 16,20)),
		DiPanelControl(CI_Connection3Supplier, QRect(70,64, 16,20)),
		DiPanelControl(CI_Connection4Supplier, QRect(101,64, 16,20)),
		DiPanelControl(CI_Connection5Supplier, QRect(132,64, 16,20)),
		DiPanelControl(CI_Connection6Supplier, QRect(163,64, 16,20)),
		DiPanelControl(CI_Connection7Supplier, QRect(194,64, 16,20)),
		DiPanelControl(CI_Connection8Supplier, QRect(225,65, 16,20)),
		DiPanelControl(CI_Connection1Consumer, QRect(8,22, 16,20)),
		DiPanelControl(CI_Connection2Consumer, QRect(39,22, 16,20)),
		DiPanelControl(CI_Connection3Consumer, QRect(70,22, 16,20)),
		DiPanelControl(CI_Connection4Consumer, QRect(101,22, 16,20)),
		DiPanelControl(CI_Connection5Consumer, QRect(132,22, 16,20)),
		DiPanelControl(CI_Connection6Consumer, QRect(163,22, 16,20)),
		DiPanelControl(CI_Connection7Consumer, QRect(194,22, 16,20)),
		DiPanelControl(CI_Connection8Consumer, QRect(225,22, 16,20)),

		// Power Consumers ( Pairs of 8 x 16 rectangles side-by-side )
		DiPanelControl(CI_Consumer1Down, QRect(16,6, 8,16)),
		DiPanelControl(CI_Consumer2Down, QRect(47,6, 8,16)),
		DiPanelControl(CI_Consumer3Down, QRect(78,6, 8,16)),
		DiPanelControl(CI_Consumer4Down, QRect(109,6, 8,16)),
		DiPanelControl(CI_Consumer5Down, QRect(140,6, 8,16)),
		DiPanelControl(CI_Consumer6Down, QRect(171,6, 8,16)),
		DiPanelControl(CI_Consumer7Down, QRect(202,6, 8,16)),
		DiPanelControl(CI_Consumer8Down, QRect(233,6, 8,16)),
		DiPanelControl(CI_Consumer1Up, QRect(8,6, 8,16)),
		DiPanelControl(CI_Consumer2Up, QRect(39,6, 8,16)),
		DiPanelControl(CI_Consumer3Up, QRect(70,6, 8,16)),
		DiPanelControl(CI_Consumer4Up, QRect(101,6, 8,16)),
		DiPanelControl(CI_Consumer5Up, QRect(132,6, 8,16)),
		DiPanelControl(CI_Consumer6Up, QRect(163,6, 8,16)),
		DiPanelControl(CI_Consumer7Up, QRect(194,6, 8,16)),
		DiPanelControl(CI_Consumer8Up, QRect(225,6, 8,16)),

		// Power Shunts ( 17 x 11 Rectangles )
		DiPanelControl(CI_PowerShunt1, QRect(32,105, 17,11)),
		DiPanelControl(CI_PowerShunt2, QRect(58,105, 17,11)),
		DiPanelControl(CI_PowerShunt3, QRect(84,105, 17,11)),
		DiPanelControl(CI_PowerShunt4, QRect(110,105, 17,11)),
		DiPanelControl(CI_PowerShunt5, QRect(136,105, 17,11)),
		DiPanelControl(CI_PowerShunt6, QRect(162,105, 17,11)),
		DiPanelControl(CI_PowerShunt7, QRect(188,105, 17,11)),
		DiPanelControl(CI_PowerShunt8, QRect(214,105, 17,11)),
	};
	unsigned int icontrols = sizeof(acontrols)/sizeof(DiPanelControl);

	aActiveControls.SampleControls(idPanel, pxHit, bContinuous, acontrols, icontrols);
}

void DpStationPower::FindPreviousSupplier(int iIndex)
{
	// Flag for a Display change if necessary
	// Switch to displaying the previous Supplier system
	if(m_iSystemCount > MAX_DISPLAYED_SYSTEMS)
	{
		bool bTaken = true;
		int iPreviousAvailableSystem = m_iSupplierIndex[iIndex];
		while(bTaken)
		{
			iPreviousAvailableSystem--;
			if(iPreviousAvailableSystem < 0)
				iPreviousAvailableSystem = m_iSystemCount - 1;
			bTaken = false;
			for(int iArrayIndex = 0; iArrayIndex < MAX_DISPLAYED_SYSTEMS; iArrayIndex++)
			{
				// If this system is already being displayed find another
				if(m_iSupplierIndex[iArrayIndex] == iPreviousAvailableSystem)
					bTaken = true;
			}
		}
		m_iSupplierIndex[iIndex] = iPreviousAvailableSystem;
	}
}

void DpStationPower::FindNextSupplier(int iIndex)
{
	// Flag for a Display change if necessary
	// Switch to displaying the previous Supplier system
	if(m_iSystemCount > MAX_DISPLAYED_SYSTEMS)
	{
		bool bTaken = true;
		int iNextAvailableSystem = m_iSupplierIndex[iIndex];
		while(bTaken)
		{
			iNextAvailableSystem++;
			if(iNextAvailableSystem > m_iSystemCount - 1)
				iNextAvailableSystem = 0;
			bTaken = false;
			for(int iArrayIndex = 0; iArrayIndex < MAX_DISPLAYED_SYSTEMS; iArrayIndex++)
			{
				// If this system is already being displayed find another
				if(m_iSupplierIndex[iArrayIndex] == iNextAvailableSystem)
					bTaken = true;
			}
		}
		m_iSupplierIndex[iIndex] = iNextAvailableSystem;
	}
}

void DpStationPower::FindPreviousConsumer(int iIndex)
{
	// Flag for a Display change if necessary
	// Switch to displaying the previous Supplier system
	if(m_iSystemCount > MAX_DISPLAYED_SYSTEMS)
	{
		bool bTaken = true;
		int iPreviousAvailableSystem = m_iConsumerIndex[iIndex];
		while(bTaken)
		{
			iPreviousAvailableSystem--;
			if(iPreviousAvailableSystem < 0)
				iPreviousAvailableSystem = m_iSystemCount - 1;
			bTaken = false;
			for(int iArrayIndex = 0; iArrayIndex < MAX_DISPLAYED_SYSTEMS; iArrayIndex++)
			{
				// If this system is already being displayed find another
				if(m_iConsumerIndex[iArrayIndex] == iPreviousAvailableSystem)
				{
					bTaken = true;
				}
			}
		}
		m_iConsumerIndex[iIndex] = iPreviousAvailableSystem;
	}
}

void DpStationPower::FindNextConsumer(int iIndex)
{
	// Flag for a Display change if necessary
	// Switch to displaying the previous Supplier system
	if(m_iSystemCount > MAX_DISPLAYED_SYSTEMS)
	{
		bool bTaken = true;
		int iNextAvailableSystem = m_iConsumerIndex[iIndex];
		while(bTaken)
		{
			iNextAvailableSystem++;
			if(iNextAvailableSystem > m_iSystemCount - 1)
				iNextAvailableSystem = 0;
			bTaken = false;
			for(int iArrayIndex = 0; iArrayIndex < MAX_DISPLAYED_SYSTEMS; iArrayIndex++)
			{
				// If this system is already being displayed find another
				if(m_iConsumerIndex[iArrayIndex] == iNextAvailableSystem)
				{
					bTaken = true;
				}
			}
		}
		m_iConsumerIndex[iIndex] = iNextAvailableSystem;
	}
}

void DpStationPower::ToggleOverclock(int iIndex, DaVessel& ship)
{
	if(iIndex >= 0 && iIndex < MAX_DISPLAYED_SYSTEMS && iIndex < m_iSystemCount)
	{
		bool nuOverclocked = !ship.IsSupplierOverclocked(m_iSupplierIndex[iIndex]);
		ship.OverclockSystemSupplier(m_iSupplierIndex[iIndex], nuOverclocked);
	}
}

void DpStationPower::SupplierSelection(int iIndex, DaVessel& ship)
{
	if(iIndex >= 0 && iIndex < MAX_DISPLAYED_SYSTEMS && iIndex < m_iSystemCount)
	{
		//qDebug("DpStationPower::SupplierSelection");
		m_iSelectedSupplier = m_iSupplierIndex[iIndex];
		if(!ship.SystemSupplierSelection(m_iSelectedSupplier))
			m_iSelectedSupplier = -1;
	}
}

void DpStationPower::ConsumerSelection(int iIndex, DaVessel& ship)
{
	if(iIndex >= 0 && iIndex < MAX_DISPLAYED_SYSTEMS && iIndex < m_iSystemCount)
	{
		//qDebug("DpStationPower::ConsumerSelection");
		if(m_iSelectedSupplier != -1)
		{
			ship.SystemConsumerSelection(m_iConsumerIndex[iIndex], m_iSelectedSupplier);
			m_iSelectedSupplier = -1;
		}
	}
}

void DpStationPower::TogglePowerPlantShunt(int shuntIndex, DaVessel& ship)
{
	if(shuntIndex >= 0 && shuntIndex < MAX_POWER_PLANT_SHUNTS)
		ship.PowerPlantShuntToggle(shuntIndex);
}

void DpStationPower::CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding)
{
	DpStation::CreateDeviceObjects(ari, parent, inBuilding);
	if(!m_ScreenTex)
		m_ScreenTex = CreateDisplayTexture(*ari.re, parent.Id());
}

//static
GLuint DpStationPower::CreateDisplayTexture(AuGLEffects& re, const AuId& parentId)
{
	QString name = QString("DCon%1").arg(parentId.GetID());
	return re.CreateTextureFromBitmap(parentId.GetID(), name, ":data/dc_Console.bmp");
}

void DpStationPower::DeleteDeviceObjects(AuGLEffects& re)
{
	DpStation::DeleteDeviceObjects(re);
	m_ScreenTex = 0;					// DaAtom deletes all textures used by the owner's atom id.
}

quint64 DpStationPower::MemoryUsage() const
{
	quint64 mem = DpStation::MemoryUsage();
	mem += sizeof(m_iSystemCount);
	mem += sizeof(m_PowerPlantShunts);
	mem += sizeof(m_LastPowerPlantHealth);
	mem += sizeof(m_LastPowerPlantShunts);
	mem += sizeof(m_iSupplierIndex);
	mem += sizeof(m_iConsumerIndex);
	mem += sizeof(m_iSelectedSupplier);
	mem += sizeof(m_SupplierType);
	mem += sizeof(m_SupplierDamage);
	mem += sizeof(m_q8SupplierPower);
	mem += sizeof(m_q8SupplierConnectionConsumer);
	mem += sizeof(m_ConsumerType);
	mem += sizeof(m_q8ConsumerConnectionAndPower);
	mem += sizeof(m_ScreenTex);
	return mem;
}

void DpStationPower::SystemImage(SYSTEM system, QImage& image)
{
	QHash<SYSTEM, QImage>::iterator si = m_SystemImages.find(system);
	if(si == m_SystemImages.end())
	{
		// Prepare to paint the system icon at 16x16 with an opaque black background, no transparency.
		QPixmap sys16(16,16);
		sys16.fill(QColor(0,0,0));	// Fill background with black.

		// Draw the system icon.
		if(!SystemIcon(system).isEmpty())
		{
			// Icon name is not blank.
			QImage syspix(SystemIcon(system));
			if(!syspix.isNull())
			{
				// Image was loaded.
				QPainter p(&sys16);
				p.setRenderHint(QPainter::SmoothPixmapTransform);
				p.drawImage(QRect(0,0,16,16), syspix);
				p.end();
			}
		}

		// Store the result in our list of system icons.
		si = m_SystemImages.insert(system, sys16.toImage());
	}
	image = *si;
}

