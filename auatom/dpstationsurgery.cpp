//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: dpstationsurgery.cpp,v 1.47 2022/04/14 20:48:02 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "dpstationsurgery.h"
//
#include "auatomrenderinfo.h"
#include "aucombat.h"
#include "aucommpacket.h"
#include "augleffects.h"
#include "auglobal.h"
#include "ausound.h"
#include "dabd.h"
#include "dacitizen.h"
#include "daperson.h"
#include "dgdata.h"
#include "dicontrolinput.h"
#include "dipanelcontrol.h"
#include <QPainter>
#include <QPixmap>

#define d_Patient						SOUND_Bleep2
#define d_Procedure						SOUND_Bleep1

// Table of control inputs.
DiControlInputArray s_SurgeryConsoleControls;		// Controls that may have bindings modified by the user.
const DiControlInput s_SurgeryConsoleDefaults[] =
{
	DiControlInput(QObject::tr("Administer Antidote"),
				DpStationSurgery::CI_Antidote, Qt::Key_A,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Inject Antitoxin"),
				DpStationSurgery::CI_Antitoxin, Qt::Key_X,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Decontaminate"),
				DpStationSurgery::CI_Decontaminate, Qt::Key_Q,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Defibrillate"),
				DpStationSurgery::CI_Defibrillate, Qt::Key_D,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Bandage Wounds"),
				DpStationSurgery::CI_Heal, Qt::Key_W,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Perform Physical Therapy"),
				DpStationSurgery::CI_PhysicalTherapy, Qt::Key_Q,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Perform Surgery"),
				DpStationSurgery::CI_Surgery, Qt::Key_S,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Select Previous Condition"),
				DpStationSurgery::CI_PrevCondition, Qt::Key_Z,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
	DiControlInput(QObject::tr("Select Next Condition"),
				DpStationSurgery::CI_NextCondition, Qt::Key_C,
				Qt::NoModifier, DiControlInput::MT_None, Qt::NoModifier,
				DiControlInput::JT_None, 0, 100, false),
};
int s_NumSurgeryConsoleDefaults = sizeof(s_SurgeryConsoleDefaults)/sizeof(DiControlInput);

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

DpStationSurgery::DpStationSurgery(quint16 whichSurgery) : DpStation()
{
	m_ConditionIndex = 0xffff;	// no condition selected
	m_CurHits = 0;
	m_Gender = -1;		// 'no patient' condition
	m_MaxHits = 0;

	m_LastCurHits = 0x7fff;
	m_LastGender = -2;
	m_LastMaxHits = 0xffff;

	m_WhichSurgery = whichSurgery;
	m_ScreenTex = 0;
}

DpStationSurgery::~DpStationSurgery()
{
	m_Conditions.Clear();
}

DpStationSurgery::DpStationSurgery(const DpStationSurgery& c) : DpStation(c)
{
	m_CurHits = c.m_CurHits;
	m_Gender = c.m_Gender;
	m_LastCurHits = c.m_LastCurHits;
	m_LastGender = c.m_LastGender;
	m_LastMaxHits = c.m_LastMaxHits;
	m_MaxHits = c.m_MaxHits;
	m_Name = c.m_Name;
	m_WhichSurgery = c.m_WhichSurgery;

	// Conditions
	c.m_ConditionLock.lockForRead();
	m_ConditionIndex = c.m_ConditionIndex;
	m_ConditionLocs = c.m_ConditionLocs;
	for(AuConditionList::const_iterator ci = c.m_Conditions.cbegin(); ci != c.m_Conditions.cend(); ci++)
		m_Conditions.append((*ci)->Clone());
	c.m_ConditionLock.unlock();
	m_ScreenTex = 0;
}

void DpStationSurgery::Rx(QDataStream& in, DaCapsule& capsule)
{
	DpStation::Rx(in, capsule);
	in >> m_Conditions;
	in >> m_ConditionIndex;
	in >> m_ConditionLocs;
	in >> m_CurHits;
	in >> m_Gender;
	in >> m_MaxHits;
	in >> m_Name;
	in >> m_WhichSurgery;
}

void DpStationSurgery::Tx(QDataStream& out) const
{
	DpStation::Tx(out);

	m_ConditionLock.lockForRead();
	out << m_Conditions;
	out << m_ConditionIndex;
	out << m_ConditionLocs;
	m_ConditionLock.unlock();

	out << m_CurHits;
	out << m_Gender;
	out << m_MaxHits;
	out << m_Name;
	out << m_WhichSurgery;
}

// Refresh data from valid counterpart.
void DpStationSurgery::Refresh(const DpStation* pCon, const DaAtom& parent)
{
	DpStation::Refresh(pCon, parent);		// Refresh base class.

	const DpStationSurgery* pA = (const DpStationSurgery*)pCon;

	// Conditions
	m_Conditions.Clear();
	for(AuConditionList::const_iterator ci = pA->m_Conditions.cbegin(); ci != pA->m_Conditions.cend(); ci++)
		m_Conditions.append((*ci)->Clone());

	m_ConditionIndex = pA->m_ConditionIndex;
	m_ConditionLocs = pA->m_ConditionLocs;
	m_CurHits = pA->m_CurHits;
	m_Gender = pA->m_Gender;
	m_MaxHits = pA->m_MaxHits;
	m_Name = pA->m_Name;
	m_WhichSurgery = pA->m_WhichSurgery;
}

bool DpStationSurgery::Tick(const AuElapsedTimer& last, const AuElapsedTimer& now, DaPerson* person)
{
	Q_UNUSED(last);
	Q_UNUSED(now);
	// Monitor patient on server.
	if(person->IsInCapsule())
	{
		DaCapsule* ship = dynamic_cast<DaCapsule*>(person->Parent());
		if(ship)
		{
			const DaCreature* patient = ship->FindPatient(*person);
			if(patient)
			{
				// There is a patient in the surgery unit.
				m_CurHits = patient->CurrentHitPoints();
				m_MaxHits = patient->MaxHitPoints();
				m_Gender = patient->Gender();
				m_Name = patient->Name();

				// Get conditions from patient.
				m_ConditionLock.lockForWrite();
				m_Conditions.Clear();
				patient->CloneConditions(m_Conditions);
				if(m_ConditionLocs.count() != m_Conditions.count())
				{
					// Condition locations don't match. Recalc condition locations.
					m_ConditionLocs.clear();
					for(AuConditionList::const_iterator ci = m_Conditions.cbegin(); ci != m_Conditions.cend(); ci++)
						m_ConditionLocs.append(GenConditionLoc(**ci));
				}
				m_ConditionLock.unlock();

				// Must call set modified on the person a lot so their screen updates with changes to the patient.
				person->SetModified();
			}
			else
			{
				// There is no patient.
				m_ConditionLock.lockForWrite();
				m_Conditions.Clear();
				m_ConditionIndex = 0xffff;
				m_ConditionLocs.clear();
				m_ConditionLock.unlock();
				m_CurHits = 0;
				m_Gender = -1;			// "No patient" indicator.
				m_MaxHits = 0;
				m_Name = "";
			}
		}
	}
	return false;
}

// Called between rendering scene frames that contains this object.
void DpStationSurgery::AnimateDeviceObjects(AuGLEffects& re, const DaVessel& ship, const AuElapsedTimer& last,
		const AuPoint& location, const DaCreature& sceneAtom)
{
	// Base class animates the lower console pane.
	DpStation::AnimateDeviceObjects(re, ship, last, location, sceneAtom);
	AnimateDeviceObjects(re, ship);
}

// Called between rendering scene frames that contains this object.
void DpStationSurgery::AnimateDeviceObjects(AuGLEffects& re, const DaBd& bd, const AuElapsedTimer& last, const AuPoint& location,
		const DaCreature& sceneAtom)
{
	// Base class animates the lower console pane.
	DpStation::AnimateDeviceObjects(re, bd, last, location, sceneAtom);
	AnimateDeviceObjects(re, bd);
}

// Called between rendering scene frames that contains this object.
void DpStationSurgery::AnimateDeviceObjects(AuGLEffects& re, const DaCapsule& ship)
{
	// Get the texture because we're going to make several changes to it.
	QString name;
	name = QString("MCon%1").arg(ship.Id().GetID());
	AuGLTexture *pTex = re.FindTexture(name);
	if(!pTex)
		return;

	// Draw the patient display area.
	{
		// Draw the appropriate gender image of the patient.
		QPixmap bmp;
		if(m_Gender < 0)
		{
			bmp = QPixmap(250,95);					// Create empty bitmap that is the same size as the patient bitmaps.
			bmp.fill(Qt::black);					// No patient
		}
		else if(m_Gender == Male || m_Gender == Hermaphrodite)
			bmp.load(":data/MalePatient.png");		// Male patient
		else
			bmp.load(":data/FemalePatient.png");	// Female patient
		QPainter p(&bmp);

		// Draw patient name and info about the selected condition on the image.
		p.setFont(QFont(g_Au.TypefaceSans(), 8));
		p.setPen(QPen(QColor(142,187,255,255), 0.f));
		if(m_Gender < 0)
		{
			p.drawText(QRect(0,0,bmp.width(),bmp.height()), Qt::AlignLeft|Qt::AlignTop, QObject::tr("Patient: None"));
		}
		else
		{
			QString text;
			if(!m_Conditions.isEmpty() && m_ConditionIndex < m_Conditions.count())
				text = QObject::tr("Patient: %1\n%2").arg(m_Name).arg(m_Conditions[m_ConditionIndex]->Description());
			else
				text = QObject::tr("Patient: %1").arg(m_Name);

			if(m_Gender == Male || m_Gender == Hermaphrodite)
				p.drawText(QRect(0,0,bmp.width(),bmp.height()), Qt::AlignLeft|Qt::AlignTop, text);		// Male at top
			else
				p.drawText(QRect(0,0,bmp.width(),bmp.height()), Qt::AlignLeft|Qt::AlignBottom, text);	// Female at bottom
		}

		// Draw icons to represent conditions onto the patient image.
		if(!m_ConditionLocs.isEmpty() && m_ConditionLocs.count() == m_Conditions.count())
		{
			// Draw condition icons.
			QPixmap bm1(":data/ConPoison.png");
			QPixmap bm2(":data/ConDisease.png");
			QPixmap bm3(":data/ConParalysis.png");
			QPixmap bm41(":data/ConImplant1.png");	// Least developed implant
			QPixmap bm42(":data/ConImplant2.png");	// Developing implant
			QPixmap bm43(":data/ConImplant3.png");	// Most developed implant
			QPixmap bm5(":data/ConAcid.png");
			QPixmap bm61(":data/ConPregnant1.png");	// Least developed baby
			QPixmap bm62(":data/ConPregnant2.png");	// Developing baby
			QPixmap bm63(":data/ConPregnant3.png");	// Most developed baby
			QPixmap bm64(":data/ConPregnant4.png");	// Baby ready to be born.
			QPixmap bm11(":data/ConBandage.png");
			QPixmap bm12(":data/ConFire.png");
			// Draw bandages in the first pass so they don't cover up other things.
			AuConditionList::const_iterator ci = m_Conditions.cbegin();
			QList<QPoint>::const_iterator cli = m_ConditionLocs.cbegin();
			while(ci != m_Conditions.cend() && cli != m_ConditionLocs.constEnd())
			{
				int iconSize = (*ci)->ConsoleIconSize();
				switch((*ci)->CType())
				{
				case AuCondition::Bandage:
					p.drawPixmap((*cli).x() - iconSize/2, (*cli).y() - iconSize/2, iconSize, iconSize, bm11);
					break;
				case AuCondition::Acid:
				case AuCondition::Antitoxin:
				case AuCondition::Bounty:
				case AuCondition::UNUSED:
				case AuCondition::Disease:
				case AuCondition::Fire:
				case AuCondition::ImplantDNA:
				case AuCondition::ImplantSpecies:
				case AuCondition::Invalid:
				case AuCondition::Paralysis:
				case AuCondition::Poison:
				case AuCondition::Pregnant:
				case AuCondition::Thought:
					break;	// Condition will not show up on the panel.
				}
				ci++;
				cli++;
			}

			// Draw everything except bandages.
			ci = m_Conditions.cbegin();
			cli = m_ConditionLocs.cbegin();
			while(ci != m_Conditions.cend() && cli != m_ConditionLocs.constEnd())
			{
				int iconSize = (*ci)->ConsoleIconSize();
				switch((*ci)->CType())
				{
				case AuCondition::Poison:
					p.drawPixmap((*cli).x() - iconSize/2, (*cli).y() - iconSize/2, iconSize, iconSize, bm1);
					break;
				case AuCondition::Disease:
					p.drawPixmap((*cli).x() - iconSize/2, (*cli).y() - iconSize/2, iconSize, iconSize, bm2);
					break;
				case AuCondition::Paralysis:
					p.drawPixmap((*cli).x() - iconSize/2, (*cli).y() - iconSize/2, iconSize, iconSize, bm3);
					break;
				case AuCondition::ImplantDNA:
					{
						int growthPercent = ((AuConImplantDNA*)(*ci))->GrowthPercent();
						if(growthPercent >= 75)
							p.drawPixmap((*cli).x() - iconSize/2, (*cli).y() - iconSize/2, iconSize, iconSize, bm43);
						else if(growthPercent >= 25)
							p.drawPixmap((*cli).x() - iconSize/2, (*cli).y() - iconSize/2, iconSize, iconSize, bm42);
						else
							p.drawPixmap((*cli).x() - iconSize/2, (*cli).y() - iconSize/2, iconSize, iconSize, bm41);
					}
					break;
				case AuCondition::ImplantSpecies:
					{
						int growthPercent = ((AuConImplantSpecies*)(*ci))->GrowthPercent();
						if(growthPercent >= 75)
							p.drawPixmap((*cli).x() - iconSize/2, (*cli).y() - iconSize/2, iconSize, iconSize, bm43);
						else if(growthPercent >= 25)
							p.drawPixmap((*cli).x() - iconSize/2, (*cli).y() - iconSize/2, iconSize, iconSize, bm42);
						else
							p.drawPixmap((*cli).x() - iconSize/2, (*cli).y() - iconSize/2, iconSize, iconSize, bm41);
					}
					break;
				case AuCondition::Acid:
					p.drawPixmap((*cli).x() - iconSize/2, (*cli).y() - iconSize/2, iconSize, iconSize, bm5);
					break;
				case AuCondition::Pregnant:
					{
						int growthPercent = ((AuConPregnant*)(*ci))->GrowthPercent();
						if(growthPercent >= 99)
							p.drawPixmap((*cli).x() - iconSize/2, (*cli).y() - iconSize/2, iconSize, iconSize, bm64);
						else if(growthPercent >= 66)
							p.drawPixmap((*cli).x() - iconSize/2, (*cli).y() - iconSize/2, iconSize, iconSize, bm63);
						else if(growthPercent >= 33)
							p.drawPixmap((*cli).x() - iconSize/2, (*cli).y() - iconSize/2, iconSize, iconSize, bm62);
						else
							p.drawPixmap((*cli).x() - iconSize/2, (*cli).y() - iconSize/2, iconSize, iconSize, bm61);
					}
					break;
				case AuCondition::Fire:
					p.drawPixmap((*cli).x() - iconSize/2, (*cli).y() - iconSize/2, iconSize, iconSize, bm12);
					break;
				case AuCondition::Antitoxin:
				case AuCondition::Bandage:
				case AuCondition::Bounty:
				case AuCondition::UNUSED:
				case AuCondition::Invalid:
				case AuCondition::Thought:
					break;	// Condition will not show up on the panel.
				}
				ci++;
				cli++;
			}

			// Draw selected icon indicator.
			if(m_ConditionIndex < m_ConditionLocs.count())
			{
				QPixmap bmsel(":data/ConSelection.png");
				p.drawPixmap(m_ConditionLocs[m_ConditionIndex].x() - bmsel.width()/2,
						m_ConditionLocs[m_ConditionIndex].y() - bmsel.height()/2, bmsel);
			}
		}

		// Copy the patient image into the panel texture.
		QImage imp = bmp.toImage();
		pTex->ReplaceSubImage(re, imp, 3,25);
	}

	// Display patient health bar.
	if(m_CurHits != m_LastCurHits || m_MaxHits != m_LastMaxHits || m_Gender != m_LastGender)
	{
		m_LastCurHits = m_CurHits;
		m_LastGender = m_Gender;
		m_LastMaxHits = m_MaxHits;

		const int barwid = 113;
		const int barht = 10;
		QPixmap pmf(barwid,barht);
		if(!pmf.isNull())
		{
			if(m_Gender < 0)
			{
				pmf.fill(Qt::black);
			}
			else
			{
				int curhp = m_CurHits;
				if(curhp < 0)
					curhp = 0;
				else if(curhp > m_MaxHits)
					curhp = m_MaxHits;
				float frachp = (float)curhp/m_MaxHits;
				int hpwid = (int)(barwid * frachp);

				QPainter p(&pmf);

				// Fill background
				QColor bgcol;
				QColor fgcol;
				if(frachp <= .1f)
				{
					fgcol.setRgbF(1,.2f,.2f,1);	// bright red
					bgcol.setRgbF(.2f,.04f,.04f,1.f);	// dim red
				}
				else if(frachp <= .5f)
				{
					fgcol.setRgbF(1,.7f,.2f,1);	// bright amber
					bgcol.setRgbF(.2f,.14f,.04f,1.f);	// dim amber
				}
				else
				{
					fgcol.setRgbF(.2f,1,.2f,1);	// bright green
					bgcol.setRgbF(.04f,.2f,.04f,1.f);	// dim green
				}
				p.fillRect(0,0,hpwid,barht,fgcol);
				p.fillRect(hpwid,0,barwid-hpwid,barht,bgcol);
			}

			QImage bmf = pmf.toImage();
			pTex->ReplaceSubImage(re, bmf, 20,6);
		}
	}

	if(m_Gender < 0)
	{
		// No patient.
		QImage bmf(":data/ac_Blank16x16.bmp");
		pTex->ReplaceSubImage(re, bmf, 3,3);
	}
	else
	{
		if(m_CurHits <= DpBody::DeadHitPoints())
		{
			// Patient is dead.
			QImage bmf(":data/DeadPatient.bmp");
			pTex->ReplaceSubImage(re, bmf, 3,3);
		}
		else if(m_CurHits <= 0)
		{
			// Patient is dying.
			QImage bmf(":data/DieingPatient.png");
			pTex->ReplaceSubImage(re, bmf, 3,3);
		}
		else
		{
			// Patient is alive.
			if(AuElapsedTimer::CurrentSecIsOdd())
			{
				QImage bmf(":data/LivePatient.png");
				pTex->ReplaceSubImage(re, bmf, 3,3);
			}
			else
			{
				QImage bmf(":data/ac_Blank16x16.bmp");
				pTex->ReplaceSubImage(re, bmf, 3,3);
			}
		}
	}
}

void DpStationSurgery::ConfigureControls(const DiControlInputArray &aAllControls)
{
	// Update my controls with those passed to me.
	for(DiControlInputArray::const_iterator ci = aAllControls.cbegin(); ci != aAllControls.cend(); ci++)
	{
		// Locate the corresponding control in my controls array.
		for(DiControlInputArray::iterator myci = s_SurgeryConsoleControls.begin(); myci != s_SurgeryConsoleControls.end(); myci++)
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

void DpStationSurgery::RestoreDefaultControls()
{
	s_SurgeryConsoleControls.clear();
	for(int ic=0; ic<s_NumSurgeryConsoleDefaults; ic++)
		s_SurgeryConsoleControls.append(s_SurgeryConsoleDefaults[ic]);
	SaveControls();
}

void DpStationSurgery::LoadControls() const
{
	// Init with my default controls.
	s_SurgeryConsoleControls.clear();
	for(int ic=0; ic<s_NumSurgeryConsoleDefaults; ic++)
		s_SurgeryConsoleControls.append(s_SurgeryConsoleDefaults[ic]);

	// Load any overrides from the settings file.
	s_SurgeryConsoleControls.LoadSettings("Controls/Surgery Console");
}

void DpStationSurgery::SaveControls() const
{
	// Store my controls in the settings file.
	s_SurgeryConsoleControls.SaveSettings("Controls/Surgery Console");
}

// A scene atom should fill this array with control inputs it can accept.
void DpStationSurgery::EnumControlInputs(DiControlInputArray& aAllControls) const
{
	// Load my static controls array if it hasn't been loaded yet. This data will only exist on the client since EnumControlInputs
	// is not called on the server.
	if(s_SurgeryConsoleControls.isEmpty())
		LoadControls();

	// Append my controls to the array.
	for(DiControlInputArray::const_iterator ci = s_SurgeryConsoleControls.cbegin();
			ci != s_SurgeryConsoleControls.cend(); ci++)
		aAllControls.append(*ci);
}

// This is how all control inputs are passed to a spacecraft console.
// Only control inputs that have been returned by EnumControlInputs will be passed.
// Only control inputs that have been activated will be passed.
void DpStationSurgery::ProcessControlInput(const DiControlInputArray& aActiveControls, DaPerson* sceneAtom, DaVessel& ship,
		CONTROL_CONTEXT cc, const AuPoint& location)
{
	Q_UNUSED(location);
	ProcessControlInput(aActiveControls, sceneAtom, ship, cc);
}

void DpStationSurgery::ProcessControlInput(const DiControlInputArray& aActiveControls, DaPerson* sceneAtom, DaBd& ship,
		CONTROL_CONTEXT cc, const AuPoint& location)
{
	Q_UNUSED(location);
	ProcessControlInput(aActiveControls, sceneAtom, ship, cc);
}

void DpStationSurgery::ProcessControlInput(const DiControlInputArray& aActiveControls, DaPerson* sceneAtom, DaCapsule& ship,
		CONTROL_CONTEXT cc)
{
	if(ship.IsNew())
		return;	// This console cannot be used when the ship is "nu".

	for(int ic=0; ic<aActiveControls.count(); ic++)
	{
		const DiControlInput &ci = aActiveControls[ic];
		switch (ci.GetId())
		{
		case CI_Antidote:
			//qDebug() << "CI_Antidote";
			if(cc != CONTROL_Client)
				DoAntidote(sceneAtom, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Procedure, 1,.96f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Antitoxin:
			//qDebug() << "CI_Antitoxin";
			if(cc != CONTROL_Client)
				DoAntitoxin(sceneAtom, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Procedure, 1,.97f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Decontaminate:
			//qDebug() << "CI_Decontaminate";
			if(cc != CONTROL_Client)
				DoDecontaminateOrDouse(sceneAtom, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Procedure, 1,.94f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Defibrillate:
			//qDebug() << "CI_Defibrillate";
			if(cc != CONTROL_Client)
				DoDefibrillate(sceneAtom, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Procedure, 1,.99f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Heal:
			//qDebug() << "CI_Heal";
			if(cc != CONTROL_Client)
				DoHeal(sceneAtom, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Procedure, 1,.98f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_PhysicalTherapy:
			//qDebug() << "CI_PhysicalTherapy";
			if(cc != CONTROL_Client)
				DoPhysicalTherapy(sceneAtom, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Procedure, 1,.93f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Surgery:
			//qDebug() << "CI_Surgery";
			if(cc != CONTROL_Client)
				DoSurgery(sceneAtom, ship);
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Procedure, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_PrevCondition:
			//qDebug() << "CI_PrevCondition";
			if(cc != CONTROL_Client)
			{
				m_ConditionLock.lockForWrite();
				if(m_Conditions.isEmpty())
					m_ConditionIndex = 0xffff;
				else
				{
					m_ConditionIndex++;
					if(m_ConditionIndex >= m_Conditions.count())
						m_ConditionIndex = 0;
				}
				m_ConditionLock.unlock();
				sceneAtom->SetModified();
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Patient, 1,.9f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_NextCondition:
			//qDebug() << "CI_NextCondition";
			if(cc != CONTROL_Client)
			{
				m_ConditionLock.lockForWrite();
				if(m_Conditions.isEmpty())
					m_ConditionIndex = 0xffff;
				else
				{
					if(m_ConditionIndex)
					{
						m_ConditionIndex--;
						if(m_ConditionIndex >= m_Conditions.count())
							m_ConditionIndex = 0;
					}
					else
						m_ConditionIndex = m_Conditions.count() - 1;
				}
				m_ConditionLock.unlock();
				sceneAtom->SetModified();
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Patient, 1,1);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		case CI_Patient:
			//qDebug() << "CI_Patient" << ci.HitLocation();
			if(cc != CONTROL_Client)
			{
				ProcessDisplayHit(ci.HitLocation());
				sceneAtom->SetModified();
			}
			if(cc != CONTROL_Server)
			{
				AuEffect buttonClick(sceneAtom->ButtonSoundLoc(), AuPoint(0,0), AuPoint(0,0), d_Patient, 1,.75f);
				sceneAtom->AddEffect(buttonClick);
			}
			break;
		}
	}
}

void DpStationSurgery::ProcessDisplayHit(const QPoint& pxHit)
{
	// Select the nearest condition icon to the hit location on the display screen.
	m_ConditionLock.lockForWrite();
	m_ConditionIndex = 0xffff;									// Init current condition index to unselected state.
	int curIdx = 0;
	int bestDist2 = 0;
	for(QList<QPoint>::const_iterator cli = m_ConditionLocs.cbegin(); cli != m_ConditionLocs.cend(); cli++)
	{
		// Calculate squared distance between pxHit and the condition icon.
		int dx = pxHit.x() - (*cli).x();
		int dy = pxHit.y() - (*cli).y();
		int dist2 = dx * dx + dy * dy;							// dist2 is the squared distance
		if(m_ConditionIndex == 0xffff || dist2 < bestDist2)
		{
			m_ConditionIndex = curIdx;
			bestDist2 = dist2;
		}
		curIdx++;
	}

	if(bestDist2 > 100)
		m_ConditionIndex = 0xffff;								// No selection if it's not within 10 pixels.
	m_ConditionLock.unlock();
}

// This is how mouse control inputs activate controls in the control input array.
// Controls in aActiveControls are activated accordingly.
void DpStationSurgery::SampleControlPanelInput(qint16 idPanel, const AuPoint &fPanelLoc, DiControlInputArray &aActiveControls,
		bool bContinuous)
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
		DiPanelControl(CI_Heal, QRect(135,3, 16,16)),
		DiPanelControl(CI_Decontaminate, QRect(152,3, 16,16)),
		DiPanelControl(CI_PhysicalTherapy, QRect(169,3, 16,16)),
		DiPanelControl(CI_Antidote, QRect(186,3, 16,16)),
		DiPanelControl(CI_Antitoxin, QRect(203,3, 16,16)),
		DiPanelControl(CI_Surgery, QRect(220,3, 16,16)),
		DiPanelControl(CI_Defibrillate, QRect(237,3, 16,16)),

		// NOTE: The patient area is last so previous (possibly overlapping) buttons get first crack at mouse clicks.
		DiPanelControl(CI_Patient, QRect(3,25,250,95), false, true),
	};
	unsigned int icontrols = sizeof(acontrols)/sizeof(DiPanelControl);

	aActiveControls.SampleControls(idPanel, pxHit, bContinuous, acontrols, icontrols);
}

// Returns true if a pixel point is within the current patient's body area.
bool DpStationSurgery::TestInsidePatientMask(const QPoint &pxTest) const
{
	if(m_Gender < 0)
		return false;

	QImage im;
	if(m_Gender == Male || m_Gender == Hermaphrodite)
		im.load(":data/MalePatientMask.png");
	else
		im.load(":data/FemalePatientMask.png");
	if(!im.isNull())
		if(pxTest.x() >= 0 && pxTest.x() < im.width() && pxTest.y() >= 0 && pxTest.y() < im.height())
			if(QColor(im.pixel(pxTest.x(), pxTest.y())) == Qt::white)
				return true;
	return false;
}

// NOTE: m_ConditionLock is already locked.
QPoint DpStationSurgery::GenConditionLoc(const AuCondition& condition) const
{
	if(condition.CType() == AuConPregnant::ConditionType())
		return QPoint(140,36);	// Measured off the image.

	// Seed with the create time value so each individual condition always ends up in the same place on the body.
	AuDice dice;
	dice.Seed(condition.CreateTime().MSecsSinceEpoch());
	QPoint bodyLoc;
	bool onBody;
	do
	{
		int ix = dice.Roll(1,250,-1);	// Gen pixel X coordinate in patient image.
		int iy = dice.Roll(1,95,-1);	// Gen pixel Y coordinate in patient image.
		bodyLoc = QPoint(ix, iy);
		onBody = TestInsidePatientMask(bodyLoc);
		if(onBody && m_ConditionLocs.contains(bodyLoc))
			onBody = false;				// Prevent conditions from landing on top of each other.
	}
	while(!onBody);
	return bodyLoc;
}

void DpStationSurgery::DoAntidote(DaPerson* doctor, DaCapsule& ship)
{
	DaCreature* patient = ship.FindPatient(*doctor);
	if(!patient)
		return;
	AuCondition* condition;
	m_ConditionLock.lockForRead();
	if(m_ConditionIndex >= m_Conditions.count())
	{
		if(doctor->HasProgrammerPower() && !patient->IsPoisoned())
		{
			// Induce a poisoned condition on the patient.
			AuConPoison* poison = new AuConPoison(10, 1, AuTime::Now());
			patient->AddCondition(poison);
			patient->SetModified();
		}
		condition = 0;		// No condition is selected.
	}
	else
		condition = m_Conditions[m_ConditionIndex]->Clone();
	m_ConditionLock.unlock();

	// Locate an antidote in the ship's inventory.
	const AuItemHash& sinv = ship.LockCargoForRead();
	AuItemHash::const_iterator ci = sinv.FindBest(CID_Antidote);
	if(ci == sinv.end())
	{
		ship.UnlockCargo();
		QString text = QObject::tr("Ship's pharmacy has no %1.").arg(g_Au.CommodityName(CID_Antidote));
		AuCommThought msg(text, doctor->Id(), doctor->Name());
		g_Au.SendAtomChat(msg);
		if(condition)
			delete condition;
		return;
	}

	AuItem* antidote = (*ci).second->Clone();
	ship.UnlockCargo();

	DoAntidote(doctor, patient, ship, condition, *antidote);

	delete antidote;
	if(condition)
		delete condition;
}

//static
void DpStationSurgery::DoAntidote(DaCreature* doctor, DaCreature* patient, DaCapsule& ship, const AuCondition* condition,
		const AuItem& antidote)
{
	Q_ASSERT(doctor);
	Q_ASSERT(patient);

	quint8 effectiveQ = antidote.Quality();

	// Do a usage/quality test on the item.
	QString itemName = antidote.SayName();
	if(!antidote.TestQuality())
	{
		// The item failed so remove one of them from the cargo hold.
		ship.RemoveCargo(antidote, 1);

		QString text = QObject::tr("%1 consumed from ship's pharmacy.").arg(itemName);
		AuCommThought msg(text, doctor->Id(), doctor->Name());
		g_Au.SendAtomChat(msg);
	}

	// The item functions, even if it fails.
	if(!condition)
	{
		// Improper use of antidote can induce a poisoning, due to not using CID_MedicalProcedure.
		if(!patient->IsNoncombatant())
		{
			AuCombatAttacker attacker(doctor->Id(), QObject::tr("improper use of %2 by %1").arg(doctor->Name()).arg(itemName),
					doctor->EmpireId());

			AuCombatWeapon weapon(WM_Internal);
			weapon.m_CID = CID_Antidote;
			weapon.m_Quality = effectiveQ;
			weapon.m_w1.m_Effect = WE_Poison;

			AuCombatResult result;
			result.m_Damage1 = g_Au.Dice().Roll(1,10);

			patient->CombatApplyDamage(attacker, weapon, AuPoint(0,1), AuPoint(0,0), result);
		}

		// Notify the doctor and the patient what happened.
		QString text = QObject::tr("Improper use of %1.").arg(itemName);
		AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
		g_Au.SendAtomChat(msgDoc);
		AuCommThought msgPat(text, patient->Id(), patient->Name());
		g_Au.SendAtomChat(msgPat);
	}
	else if(condition->CType() == AuConPoison::ConditionType())
	{
		// Correct treatment applied.
		QString condName = condition->Name();
		int removalDamage = condition->RemovalCost();
		if(effectiveQ >= condition->Quality())
		{
			if(patient->RemoveCondition(condition))
			{
				if(removalDamage)
				{
					AuCombatAttacker attacker(doctor->Id(),
							QObject::tr("%1 curing %2 using %3").arg(doctor->Name()).arg(condName).arg(itemName),
							doctor->EmpireId());

					AuCombatWeapon weapon(WM_Internal);
					weapon.m_CID = CID_MedicalProcedure;
					weapon.m_Quality = effectiveQ;
					weapon.m_w1.m_Effect = WE_Poison;

					AuCombatResult result;
					result.m_Damage1 = g_Au.Dice().Roll(1,10);

					patient->CombatApplyDamage(attacker, weapon, AuPoint(0,1), AuPoint(0,0), result);
				}

				// Notify the doctor and the patient what happened.
				QString text = QObject::tr("%1 cured using %2.").arg(condName).arg(itemName);
				AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
				g_Au.SendAtomChat(msgDoc);
				AuCommThought msgPat(text, patient->Id(), patient->Name());
				g_Au.SendAtomChat(msgPat);
			}
			else
			{
				// Notify the doctor what happened.
				QString text = QObject::tr("%1 not cured by %2.").arg(condName).arg(itemName);
				AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
			}
		}
		else
		{
			if(patient->ReduceCondition(condition, effectiveQ))
			{
				if(removalDamage)
				{
					AuCombatAttacker attacker(doctor->Id(),
							QObject::tr("%1 reducing %2 using %3").arg(doctor->Name()).arg(condName).arg(itemName),
							doctor->EmpireId());

					AuCombatWeapon weapon(WM_Internal);
					weapon.m_CID = CID_MedicalProcedure;
					weapon.m_Quality = effectiveQ;
					weapon.m_w1.m_Effect = WE_Poison;

					AuCombatResult result;
					result.m_Damage1 = g_Au.Dice().Roll(1,10);

					patient->CombatApplyDamage(attacker, weapon, AuPoint(0,1), AuPoint(0,0), result);
				}

				// Notify the doctor and the patient what happened.
				QString text = QObject::tr("%1 reduced using %2.").arg(condName).arg(itemName);
				AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
				g_Au.SendAtomChat(msgDoc);
				AuCommThought msgPat(text, patient->Id(), patient->Name());
				g_Au.SendAtomChat(msgPat);
			}
			else
			{
				// Notify the doctor what happened.
				QString text = QObject::tr("%1 not reduced by %2.").arg(condName).arg(itemName);
				AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
			}
		}
	}
	else
	{
		// Incorrect treatment applied.
		if(!patient->IsNoncombatant())
		{
			AuCombatAttacker attacker(doctor->Id(),
					QObject::tr("incorrect medical treatment by %1 using %2").arg(doctor->Name()).arg(itemName),
					doctor->EmpireId());

			AuCombatWeapon weapon(WM_Internal);
			weapon.m_CID = CID_MedicalProcedure;
			weapon.m_Quality = effectiveQ;
			weapon.m_w1.m_Effect = WE_Poison;

			AuCombatResult result;
			result.m_Damage1 = g_Au.Dice().Roll(1,10);

			patient->CombatApplyDamage(attacker, weapon, AuPoint(0,1), AuPoint(0,0), result);
		}

		// Notify the doctor and the patient what happened.
		QString text = QObject::tr("Incorrect medical treatment with %1.").arg(itemName);
		AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
		g_Au.SendAtomChat(msgDoc);
		AuCommThought msgPat(text, patient->Id(), patient->Name());
		g_Au.SendAtomChat(msgPat);
	}
	patient->SetModified();

	// Reset think time whenever a treatment is applied to an AI patient so they will stay in the unit a little longer.
	// This way they will stay as long as they are being treated before deciding to leave.
	DaCitizen* citizen = dynamic_cast<DaCitizen*>(patient);
	if(citizen)
		citizen->RestartThinkTime();
}

void DpStationSurgery::DoAntitoxin(DaPerson* doctor, DaCapsule& ship)
{
	DaCreature* patient = ship.FindPatient(*doctor);
	if(!patient)
		return;
	AuCondition* condition;
	m_ConditionLock.lockForRead();
	if(m_ConditionIndex >= m_Conditions.count())
	{
		if(doctor->HasProgrammerPower() && !patient->HasDisease())
		{
			// Induce a disease condition on the patient.
			AuConDisease* disease = new AuConDisease(254, 255, 31, AuTime::Now());	// The worst disease imaginable.
			patient->AddCondition(disease);
			patient->SetModified();
		}
		condition = 0;		// No condition is selected.
	}
	else
		condition = m_Conditions[m_ConditionIndex]->Clone();
	m_ConditionLock.unlock();

	// Locate an antitoxin in the ship's inventory.
	const AuItemHash& sinv = ship.LockCargoForRead();
	AuItemHash::const_iterator ci = sinv.FindBest(CID_Antitoxin);
	if(ci == sinv.end())
	{
		ship.UnlockCargo();
		QString text = QObject::tr("Ship's pharmacy has no %1.").arg(g_Au.CommodityName(CID_Antitoxin));
		AuCommThought msg(text, doctor->Id(), doctor->Name());
		g_Au.SendAtomChat(msg);
		if(condition)
			delete condition;
		return;
	}

	AuItem* antitoxin = (*ci).second->Clone();
	ship.UnlockCargo();

	DoAntitoxin(doctor, patient, ship, condition, *antitoxin);

	delete antitoxin;
	if(condition)
		delete condition;
}

//static
void DpStationSurgery::DoAntitoxin(DaCreature* doctor, DaCreature* patient, DaCapsule& ship, const AuCondition* condition,
		const AuItem& antitoxin)
{
	Q_ASSERT(doctor);
	Q_ASSERT(patient);
	Q_UNUSED(condition);

	// Do a usage/quality test on the item.
	QString itemName = antitoxin.SayName();
	if(!antitoxin.TestQuality())
	{
		// The item failed so remove one of them from the cargo hold.
		ship.RemoveCargo(antitoxin, 1);

		QString text = QObject::tr("%1 consumed from ship's pharmacy.").arg(itemName);
		AuCommThought msg(text, doctor->Id(), doctor->Name());
		g_Au.SendAtomChat(msg);
	}

	// Locate existing immunization, if any.
	AuConAntitoxin* oldAnti;
	const AuConAntitoxin* ob =
		dynamic_cast<const AuConAntitoxin*>(patient->FindConditionLock(AuConAntitoxin::ConditionType()));
	if(ob)
		oldAnti = (AuConAntitoxin*)ob->Clone();
	else
		oldAnti = 0;
	patient->UnlockConditions();

	QString text;
	if(oldAnti && oldAnti->EffectiveQuality() >= antitoxin.Quality())
	{
		if(oldAnti->EffectiveQuality() > antitoxin.Quality())
			text = QObject::tr("%1 is already immunized with a better quality antitoxin.").arg(patient->Name());
		else
			text = QObject::tr("%1 is already immunized with the same quality antitoxin.").arg(patient->Name());
		AuCommThought msg(text, doctor->Id(), doctor->Name());
		g_Au.SendAtomChat(msg);
	}
	else
	{
		// Patient is not immunized.
		if(oldAnti)
			patient->RemoveCondition(oldAnti);	// Remove the old antitoxin.

		// Add a new immunization.
		AuConAntitoxin* anti = new AuConAntitoxin(antitoxin.Quality(), AuTime::Now());
		patient->AddCondition(anti);
		patient->SetModified();

		if(oldAnti)
			text = QObject::tr("%1 immunization against disease boosted using %2.")
				.arg(patient->Name()).arg(antitoxin.SayName());
		else
			text = QObject::tr("%1 immunized against disease using %2.").arg(patient->Name()).arg(antitoxin.SayName());
		AuCommThought msg(text, doctor->Id(), doctor->Name());
		g_Au.SendAtomChat(msg);

		// Notify the patient if it wasn't me.
		if(oldAnti)
			text = QObject::tr("%1 boosted my immunization against disease using %3.")
				.arg(doctor->Name()).arg(antitoxin.SayName());
		else
			text = QObject::tr("%1 immunized me against disease using %3.").arg(doctor->Name()).arg(antitoxin.SayName());
		AuCommThought msgPat(text, patient->Id(), patient->Name());
		g_Au.SendAtomChat(msgPat);
	}
	if(oldAnti)
		delete oldAnti;
	patient->SetModified();

	// Reset think time whenever a treatment is applied to an AI patient so they will stay in the unit a little longer.
	// This way they will stay as long as they are being treated before deciding to leave.
	DaCitizen* citizen = dynamic_cast<DaCitizen*>(patient);
	if(citizen)
		citizen->RestartThinkTime();
}

void DpStationSurgery::DoDecontaminateOrDouse(DaPerson* doctor, DaCapsule& ship)
{
	DaCreature* patient = ship.FindPatient(*doctor);
	if(!patient)
		return;
	AuCondition* condition;
	m_ConditionLock.lockForRead();
	if(m_ConditionIndex >= m_Conditions.count())
	{
		if(doctor->HasProgrammerPower() && !patient->HasAcid() && !patient->IsBurning())
		{
			// Induce an acid condition on the patient.
			AuConAcid* acid = new AuConAcid(10, 1, AuTime::Now());
			patient->AddCondition(acid);
			patient->SetModified();
		}
		condition = 0;		// No condition is selected.
	}
	else
		condition = m_Conditions[m_ConditionIndex]->Clone();
	m_ConditionLock.unlock();

	// Locate water in the ship's inventory.
	const AuItemHash& sinv = ship.LockCargoForRead();
	AuItemHash::const_iterator ci = sinv.FindBest(CID_Water);
	if(ci == sinv.end())
	{
		ship.UnlockCargo();
		QString text;
		if(condition && condition->CType() == AuCondition::Fire)
			text = QObject::tr("Ship's pharmacy has no %1. You should stop, drop and roll.").arg(g_Au.CommodityName(CID_Water));
		else
			text = QObject::tr("Ship's pharmacy has no %1.").arg(g_Au.CommodityName(CID_Water));
		AuCommThought msg(text, doctor->Id(), doctor->Name());
		g_Au.SendAtomChat(msg);
		if(condition)
			delete condition;
		return;
	}

	QString text = QObject::tr("%1 consumed from ship's pharmacy.").arg((*ci).second->SayName());
	AuCommThought msg(text, doctor->Id(), doctor->Name());
	g_Au.SendAtomChat(msg);

	AuItem* water = (*ci).second->Clone();
	ship.UnlockCargo();

	if(condition && condition->CType() == AuCondition::Fire)
		DoExtinguish(doctor, patient, ship, condition, *water);
	else
		DoDecontaminate(doctor, patient, ship, condition, *water);

	delete water;
	if(condition)
		delete condition;
}

//static
void DpStationSurgery::DoDecontaminate(DaCreature* doctor, DaCreature* patient, DaCapsule& ship, const AuCondition* condition,
		const AuItem& water)
{
	// The effective quality is the smaller of the surgery unit quality or the treatment quality.
	quint8 effectiveQ = water.Quality();

	// Remove one unit of water from the cargo hold.
	QString itemName = water.SayName();
	ship.RemoveCargo(water, 1);

	if(!condition)
	{
		// Notify the doctor and the patient what happened.
		QString text = QObject::tr("%1 washed.").arg(patient->Name());
		AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
		g_Au.SendAtomChat(msgDoc);
		AuCommThought msgPat(text, patient->Id(), patient->Name());
		g_Au.SendAtomChat(msgPat);
	}
	else if(condition->CType() == AuConAcid::ConditionType())
	{
		// Correct treatment applied.
		QString condName = condition->Name();
		if(effectiveQ >= condition->Quality())
		{
			if(patient->RemoveCondition(condition))
			{
				// Notify the doctor and the patient what happened.
				QString text = QObject::tr("%1 decontaminated using %2.").arg(condName).arg(itemName);
				AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
				g_Au.SendAtomChat(msgDoc);
				AuCommThought msgPat(text, patient->Id(), patient->Name());
				g_Au.SendAtomChat(msgPat);
			}
			else
			{
				// Notify the doctor what happened.
				QString text = QObject::tr("%1 not decontaminated by %2.").arg(condName).arg(itemName);
				AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
			}
		}
		else
		{
			if(patient->ReduceCondition(condition, effectiveQ))
			{
				// Notify the doctor and the patient what happened.
				QString text = QObject::tr("%1 reduced using %2.").arg(condName).arg(itemName);
				AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
				g_Au.SendAtomChat(msgDoc);
				AuCommThought msgPat(text, patient->Id(), patient->Name());
				g_Au.SendAtomChat(msgPat);
			}
			else
			{
				// Notify the doctor what happened.
				QString text = QObject::tr("%1 not reduced by %2.").arg(condName).arg(itemName);
				AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
			}
		}
	}
	else
	{
		// Notify the doctor and the patient what happened.
		QString text = QObject::tr("%1 site washed with %2.").arg(condition->Name()).arg(itemName);
		AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
		g_Au.SendAtomChat(msgDoc);
		AuCommThought msgPat(text, patient->Id(), patient->Name());
		g_Au.SendAtomChat(msgPat);
	}
	patient->SetModified();

	// Reset think time whenever a treatment is applied to an AI patient so they will stay in the unit a little longer.
	// This way they will stay as long as they are being treated before deciding to leave.
	DaCitizen* citizen = dynamic_cast<DaCitizen*>(patient);
	if(citizen)
		citizen->RestartThinkTime();
}

//static
void DpStationSurgery::DoExtinguish(DaCreature* doctor, DaCreature* patient, DaCapsule& ship, const AuCondition* condition,
		const AuItem& water)
{
	// Remove one unit of water from the cargo hold.
	QString itemName = water.SayName();
	ship.RemoveCargo(water, 1);

	if(!condition)
	{
		// Notify the doctor and the patient what happened.
		QString text = QObject::tr("%1 washed.").arg(patient->Name());
		AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
		g_Au.SendAtomChat(msgDoc);
		AuCommThought msgPat(text, patient->Id(), patient->Name());
		g_Au.SendAtomChat(msgPat);
	}
	else if(condition->CType() == AuConFire::ConditionType())
	{
		// Correct treatment applied.
		QString condName = condition->Name();
		if(patient->RemoveCondition(condition))
		{
			// Notify the doctor and the patient what happened.
			QString text = QObject::tr("%1 doused using %2.").arg(condName).arg(itemName);
			AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
			g_Au.SendAtomChat(msgDoc);
			AuCommThought msgPat(text, patient->Id(), patient->Name());
			g_Au.SendAtomChat(msgPat);
		}
		else
		{
			// Notify the doctor what happened.
			QString text = QObject::tr("%1 not extinguished by %2.").arg(condName).arg(itemName);
			AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
		}
	}
	else
	{
		// Notify the doctor and the patient what happened.
		QString text = QObject::tr("%1 site washed with %2.").arg(condition->Name()).arg(itemName);
		AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
		g_Au.SendAtomChat(msgDoc);
		AuCommThought msgPat(text, patient->Id(), patient->Name());
		g_Au.SendAtomChat(msgPat);
	}
	patient->SetModified();

	// Reset think time whenever a treatment is applied to an AI patient so they will stay in the unit a little longer.
	// This way they will stay as long as they are being treated before deciding to leave.
	DaCitizen* citizen = dynamic_cast<DaCitizen*>(patient);
	if(citizen)
		citizen->RestartThinkTime();
}

void DpStationSurgery::DoDefibrillate(DaPerson* doctor, DaCapsule& ship)
{
	DaCreature* patient = ship.FindPatient(*doctor);
	if(!patient)
		return;

	if(patient->CurrentHitPoints() <= 0)
	{
		// Correct treatment applied.
		DoDefibrillate(doctor, patient, ship);
	}
	else
	{
		// Incorrect treatment applied.
		if(!patient->IsNoncombatant())
		{
			AuCombatAttacker attacker(doctor->Id(),
					QObject::tr("incorrect medical treatment by %1 using defibrillator").arg(doctor->Name()),
					doctor->EmpireId());

			AuCombatWeapon weapon(WM_Internal);
			weapon.m_CID = CID_MedicalProcedure;
			weapon.m_w1.m_Effect = WE_Electric;

			AuCombatResult result;
			result.m_Damage1 = g_Au.Dice().Roll(1,10);

			patient->CombatApplyDamage(attacker, weapon, AuPoint(0,1), AuPoint(0,0), result);
		}

		// Notify the doctor and the patient what happened.
		QString text = QObject::tr("Incorrect medical treatment with defibrillator.");
		AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
		g_Au.SendAtomChat(msgDoc);
		AuCommThought msgPat(text, patient->Id(), patient->Name());
		g_Au.SendAtomChat(msgPat);
	}
	patient->SetModified();
}

//static
void DpStationSurgery::DoDefibrillate(DaCreature* doctor, DaCreature* patient, DaCapsule& ship)
{
	Q_ASSERT(doctor);
	Q_ASSERT(patient);
	patient->SetHitPoints(1);	// Stop them from dying.

	if(!patient->IsPatientStationPose())
	{
		// Put the patient into the surgery unit. A person might be dressed but oh well, we have no good way to prep them.
		quint16 consoleId = doctor->StationId();
		quint16 surgeryId;
		AuStation station;
		ship.PatientStation(consoleId, surgeryId, station);

		// Adjust for pose.
		// For example, a standing station is positioned on the floor; we must adjust so our feet are there.
		station.OffsetOpM(patient->ConsolePoseOffset(station.Type(), station.OccPos(), patient->CGHeightStanding(),
					patient->TorsoThickness(), station.SeatIndex()));

		patient->ForcePosStop(AuAtomPos(ship.Id(), station.OpM()));

		// Must call SitAtConsole after atom is moved to console location.
		patient->SitAtConsole(surgeryId, station.Type(), station.OccPos(), station.SeatIndex(), doctor->StationId());
	}
	patient->InvalidateEnvironment();
	patient->SetModified();
	patient->Defibrillated(doctor->EmpireId());

	// Notify the doctor and the patient what happened.
	QString text = QObject::tr("%1 resuscitated using defibrillator.").arg(patient->Name());
	AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
	g_Au.SendAtomChat(msgDoc);
	AuCommThought msgPat(text, patient->Id(), patient->Name());
	g_Au.SendAtomChat(msgPat);
}

void DpStationSurgery::DoHeal(DaPerson* doctor, DaCapsule& ship)
{
	DaCreature* patient = ship.FindPatient(*doctor);
	if(!patient)
		return;

	// Locate a medical kit in the ship's inventory.
	const AuItemHash& sinv = ship.LockCargoForRead();
	AuItemHash::const_iterator ci = sinv.FindBest(CID_MedicalKit);
	if(ci == sinv.end())
	{
		ship.UnlockCargo();
		QString text = QObject::tr("Ship's pharmacy has no %1.").arg(g_Au.CommodityName(CID_MedicalKit));
		AuCommThought msg(text, doctor->Id(), doctor->Name());
		g_Au.SendAtomChat(msg);
		return;
	}

	AuItem* medkit = (*ci).second->Clone();
	ship.UnlockCargo();

	DoHeal(doctor, patient, ship, *medkit);

	delete medkit;
}

//static
void DpStationSurgery::DoHeal(DaCreature* doctor, DaCreature* patient, DaCapsule& ship, const AuItem& medicalKit)
{
	// Do a usage/quality test on the item.
	quint8 medQ = medicalKit.Quality();
	QString itemName = medicalKit.SayName();
	if(!medicalKit.TestQuality())
	{
		// The item failed so remove one of them from the cargo hold.
		ship.RemoveCargo(medicalKit, 1);

		QString text = QObject::tr("%1 consumed from ship's pharmacy.").arg(itemName);
		AuCommThought msg(text, doctor->Id(), doctor->Name());
		g_Au.SendAtomChat(msg);
	}

	// The item functions, even if it fails.
	short int curhp = patient->CurrentHitPoints();
	short int maxhp = patient->MaxHitPoints();
	if(curhp < maxhp)
	{
		// Locate existing bandage, if any.
		AuCondition* oldBandage;
		const AuCondition* ob = patient->FindConditionLock(AuConBandage::ConditionType());
		if(ob)
			oldBandage = ob->Clone();
		else
			oldBandage = 0;
		patient->UnlockConditions();

		// Treatment applied.
		QString text;
		if(oldBandage && oldBandage->Quality() >= medQ)
		{
			if(oldBandage->Quality() > medQ)
				text = QObject::tr("%1 is already wrapped in better quality bandages.").arg(patient->Name());
			else
				text = QObject::tr("%1 is already wrapped in the same quality bandages.").arg(patient->Name());
		}
		else
		{
			// Patient is injured. Bandage them.
			if(oldBandage)
				patient->RemoveCondition(oldBandage);	// Remove the old bandage.

			// Add a new bandage.
			AuConBandage* bandage = new AuConBandage(medQ, AuTime::Now());
			patient->AddCondition(bandage);
			patient->SetModified();

			// Reset think time whenever a treatment is applied to an AI patient so they will stay in the unit a little longer.
			// This way they will stay as long as they are being treated before deciding to leave.
			DaCitizen* citizen = dynamic_cast<DaCitizen*>(patient);
			if(citizen)
				citizen->RestartThinkTime();

			if(oldBandage)
				text = QObject::tr("%1 bandages replaced using %2.").arg(patient->Name()).arg(itemName);
			else
				text = QObject::tr("%1 bandaged using %2.").arg(patient->Name()).arg(itemName);
		}
		if(oldBandage)
			delete oldBandage;

		// Notify the doctor and the patient what happened.
		AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
		g_Au.SendAtomChat(msgDoc);
		AuCommThought msgPat(text, patient->Id(), patient->Name());
		g_Au.SendAtomChat(msgPat);
	}
	else
	{
		QString text = QObject::tr("%1 is not injured.").arg(patient->Name());
		AuCommThought msg(text, doctor->Id(), doctor->Name());
		g_Au.SendAtomChat(msg);
	}
}

void DpStationSurgery::DoPhysicalTherapy(DaPerson* doctor, DaCapsule& ship)
{
	DaCreature* patient = ship.FindPatient(*doctor);
	if(!patient)
		return;
	AuCondition* condition;
	m_ConditionLock.lockForRead();
	if(m_ConditionIndex >= m_Conditions.count())
	{
		if(doctor->HasProgrammerPower() && !patient->IsParalyzed())
		{
			// Induce an implant condition on the patient.
			AuConParalysis* paralysis = new AuConParalysis(10, 1, AuTime::Now());
			patient->AddCondition(paralysis);
			patient->SetModified();
		}
		condition = 0;		// No condition is selected.
	}
	else
		condition = m_Conditions[m_ConditionIndex]->Clone();
	m_ConditionLock.unlock();

	DoPhysicalTherapy(doctor, patient, condition);

	if(condition)
		delete condition;
}

//static
void DpStationSurgery::DoPhysicalTherapy(DaCreature* doctor, DaCreature* patient, const AuCondition* condition)
{
	quint8 effectiveQ = 255;	// doctor skill?

	if(!condition)
	{
		// Notify the doctor and the patient what happened.
		QString text = QObject::tr("%1 massaged.").arg(patient->Name());
		AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
		g_Au.SendAtomChat(msgDoc);
		AuCommThought msgPat(text, patient->Id(), patient->Name());
		g_Au.SendAtomChat(msgPat);
	}
	else if(condition->CType() == AuConParalysis::ConditionType())
	{
		// Correct treatment applied.
		QString condName = condition->Name();
		int removalDamage = condition->RemovalCost();
		if(effectiveQ >= condition->Quality())
		{
			if(patient->RemoveCondition(condition))
			{
				if(removalDamage)
				{
					AuCombatAttacker attacker(doctor->Id(),
							QObject::tr("%1 curing %2 using physical therapy").arg(doctor->Name()).arg(condName),
							doctor->EmpireId());

					AuCombatWeapon weapon(WM_Internal);
					weapon.m_CID = CID_MedicalProcedure;
					weapon.m_w1.m_Effect = WE_Stun;

					AuCombatResult result;
					result.m_Damage1 = g_Au.Dice().Roll(1,10);

					patient->CombatApplyDamage(attacker, weapon, AuPoint(0,1), AuPoint(0,0), result);
				}

				// Notify the doctor and the patient what happened.
				QString text = QObject::tr("%1 cured using physical therapy.").arg(condName);
				AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
				g_Au.SendAtomChat(msgDoc);
				AuCommThought msgPat(text, patient->Id(), patient->Name());
				g_Au.SendAtomChat(msgPat);
			}
			else
			{
				// Notify the doctor what happened.
				QString text = QObject::tr("%1 not cured by physical therapy.").arg(condName);
				AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
			}
		}
		else
		{
			if(patient->ReduceCondition(condition, effectiveQ))
			{
				if(removalDamage)
				{
					AuCombatAttacker attacker(doctor->Id(),
							QObject::tr("%1 reducing %2 using physical therapy").arg(doctor->Name()).arg(condName),
							doctor->EmpireId());

					AuCombatWeapon weapon(WM_Internal);
					weapon.m_CID = CID_MedicalProcedure;
					weapon.m_w1.m_Effect = WE_Stun;

					AuCombatResult result;
					result.m_Damage1 = g_Au.Dice().Roll(1,10);

					patient->CombatApplyDamage(attacker, weapon, AuPoint(0,1), AuPoint(0,0), result);
				}

				// Notify the doctor and the patient what happened.
				QString text = QObject::tr("%1 reduced using physical therapy.").arg(condName);
				AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
				g_Au.SendAtomChat(msgDoc);
				AuCommThought msgPat(text, patient->Id(), patient->Name());
				g_Au.SendAtomChat(msgPat);
			}
			else
			{
				// Notify the doctor what happened.
				QString text = QObject::tr("%1 not reduced by physical therapy.").arg(condName);
				AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
			}
		}
	}
	else
	{
		// Incorrect treatment applied.
		if(!patient->IsNoncombatant())
		{
			AuCombatAttacker attacker(doctor->Id(),
					QObject::tr("incorrect medical treatment by %1 using physical therapy").arg(doctor->Name()),
					doctor->EmpireId());

			AuCombatWeapon weapon(WM_Internal);
			weapon.m_CID = CID_MedicalProcedure;
			weapon.m_w1.m_Effect = WE_Stun;

			AuCombatResult result;
			result.m_Damage1 = g_Au.Dice().Roll(1,10);

			patient->CombatApplyDamage(attacker, weapon, AuPoint(0,1), AuPoint(0,0), result);
		}

		// Notify the doctor and the patient what happened.
		QString text = QObject::tr("Incorrect medical treatment with physical therapy.");
		AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
		g_Au.SendAtomChat(msgDoc);
		AuCommThought msgPat(text, patient->Id(), patient->Name());
		g_Au.SendAtomChat(msgPat);
	}
	patient->SetModified();

	// Reset think time whenever a treatment is applied to an AI patient so they will stay in the unit a little longer.
	// This way they will stay as long as they are being treated before deciding to leave.
	DaCitizen* citizen = dynamic_cast<DaCitizen*>(patient);
	if(citizen)
		citizen->RestartThinkTime();
}

void DpStationSurgery::DoSurgery(DaPerson* doctor, DaCapsule& ship)
{
	DaCreature* patient = ship.FindPatient(*doctor);
	if(!patient)
		return;
	AuCondition* condition;
	m_ConditionLock.lockForRead();
	if(m_ConditionIndex >= m_Conditions.count())
		condition = 0;		// No condition is selected.
	else
		condition = m_Conditions[m_ConditionIndex]->Clone();
	m_ConditionLock.unlock();

	DoSurgery(doctor, patient, condition);

	if(condition)
		delete condition;
}

//static
void DpStationSurgery::DoSurgery(DaCreature* doctor, DaCreature* patient, const AuCondition* condition)
{
	quint8 effectiveQ = 255;	// doctor skill?

	if(!condition)
	{
		// Notify the doctor and the patient what happened.
		QString text = QObject::tr("%1 shaved.").arg(patient->Name());
		AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
		g_Au.SendAtomChat(msgDoc);
		AuCommThought msgPat(text, patient->Id(), patient->Name());
		g_Au.SendAtomChat(msgPat);
	}
	else if(condition->CType() == AuConImplantDNA::ConditionType() ||
			condition->CType() == AuConImplantSpecies::ConditionType() ||
			condition->CType() == AuConPregnant::ConditionType())
	{
		// Correct treatment applied.
		QString condName = condition->Name();
		int removalDamage = condition->RemovalCost();
		if(effectiveQ >= condition->Quality())
		{
			if(patient->RemoveCondition(condition))
			{
				if(removalDamage)
				{
					AuCombatAttacker attacker(doctor->Id(),
							QObject::tr("%1 removing %2 using scalpel").arg(doctor->Name()).arg(condName),
							doctor->EmpireId());

					AuCombatWeapon weapon(WM_Internal);
					weapon.m_CID = CID_MedicalProcedure;
					weapon.m_w1.m_Effect = WE_Slash;

					AuCombatResult result;
					result.m_Damage1 = g_Au.Dice().Roll(1,10);

					patient->CombatApplyDamage(attacker, weapon, AuPoint(0,1), AuPoint(0,0), result);
				}

				// Notify the doctor and the patient what happened.
				QString text = QObject::tr("%1 removed using scalpel.").arg(condName);
				AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
				g_Au.SendAtomChat(msgDoc);
				AuCommThought msgPat(text, patient->Id(), patient->Name());
				g_Au.SendAtomChat(msgPat);
			}
			else
			{
				// Notify the doctor what happened.
				QString text = QObject::tr("%1 not removed by scalpel.").arg(condName);
				AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
			}
		}
		else
		{
			if(patient->ReduceCondition(condition, effectiveQ))
			{
				if(removalDamage)
				{
					AuCombatAttacker attacker(doctor->Id(),
							QObject::tr("%1 reducing %2 using scalpel").arg(doctor->Name()).arg(condName),
							doctor->EmpireId());

					AuCombatWeapon weapon(WM_Internal);
					weapon.m_CID = CID_MedicalProcedure;
					weapon.m_w1.m_Effect = WE_Slash;

					AuCombatResult result;
					result.m_Damage1 = g_Au.Dice().Roll(1,10);

					patient->CombatApplyDamage(attacker, weapon, AuPoint(0,1), AuPoint(0,0), result);
				}

				// Notify the doctor and the patient what happened.
				QString text = QObject::tr("%1 reduced using scalpel.").arg(condName);
				AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
				g_Au.SendAtomChat(msgDoc);
				AuCommThought msgPat(text, patient->Id(), patient->Name());
				g_Au.SendAtomChat(msgPat);
			}
			else
			{
				// Notify the doctor what happened.
				QString text = QObject::tr("%1 not reduced by scalpel.").arg(condName);
				AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
			}
		}
	}
	else
	{
		// Incorrect treatment applied.
		if(!patient->IsNoncombatant())
		{
			AuCombatAttacker attacker(doctor->Id(),
					QObject::tr("incorrect medical treatment by %1 using scalpel").arg(doctor->Name()),
					doctor->EmpireId());

			AuCombatWeapon weapon(WM_Internal);
			weapon.m_CID = CID_MedicalProcedure;
			weapon.m_w1.m_Effect = WE_Slash;

			AuCombatResult result;
			result.m_Damage1 = g_Au.Dice().Roll(1,10);

			patient->CombatApplyDamage(attacker, weapon, AuPoint(0,1), AuPoint(0,0), result);
		}

		// Notify the doctor and the patient what happened.
		QString text = QObject::tr("Incorrect medical treatment with scalpel.");
		AuCommThought msgDoc(text, doctor->Id(), doctor->Name());
		g_Au.SendAtomChat(msgDoc);
		AuCommThought msgPat(text, patient->Id(), patient->Name());
		g_Au.SendAtomChat(msgPat);
	}
	patient->SetModified();

	// Reset think time whenever a treatment is applied to an AI patient so they will stay in the unit a little longer.
	// This way they will stay as long as they are being treated before deciding to leave.
	DaCitizen* citizen = dynamic_cast<DaCitizen*>(patient);
	if(citizen)
		citizen->RestartThinkTime();
}

void DpStationSurgery::CreateDeviceObjects(AuAtomRenderInfo& ari, const DaAtom& parent, bool inBuilding)
{
	DpStation::CreateDeviceObjects(ari, parent, inBuilding);
	if(!m_ScreenTex)
		m_ScreenTex = CreateDisplayTexture(*ari.re, parent.Id());
}

//static
GLuint DpStationSurgery::CreateDisplayTexture(AuGLEffects& re, const AuId& parentId)
{
	QString name = QString("MCon%1").arg(parentId.GetID());
	return re.CreateTextureFromBitmap(parentId.GetID(), name, ":data/txConsoleSurgery.bmp");
}

void DpStationSurgery::DeleteDeviceObjects(AuGLEffects& re)
{
	DpStation::DeleteDeviceObjects(re);
	m_ScreenTex = 0;					// DaAtom deletes all textures used by the owner's atom id.
}

quint64 DpStationSurgery::MemoryUsage() const
{
	quint64 mem = DpStation::MemoryUsage();
	mem += sizeof(m_CurHits);
	mem += sizeof(m_Gender);
	mem += sizeof(m_MaxHits);
	mem += StringMemoryUsage(m_Name);
	mem += sizeof(m_WhichSurgery);
	mem += sizeof(m_ConditionLock);
	mem += sizeof(m_ConditionIndex);
	mem += sizeof(QPoint) * m_ConditionLocs.size();
	mem += m_Conditions.MemoryUsage();
	mem += sizeof(m_LastCurHits);
	mem += sizeof(m_LastGender);
	mem += sizeof(m_LastMaxHits);
	mem += sizeof(m_ScreenTex);
	return mem;
}
