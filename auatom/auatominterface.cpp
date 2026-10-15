//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auatominterface.cpp,v 1.30 2022/05/06 15:05:29 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auatominterface.h"
//
#include "auanimalia.h"
#include "auarenaready.h"
#include "aubddesign.h"
#include "aucity.h"
#include "aucommpacket.h"
#include "audesign.h"
#include "auitdesign.h"
#include "auitreport.h"
#include "auscdesign.h"
#include "auscparts.h"
#include "auscreport.h"
#include "aushipment.h"
#include "austory.h"
#include "daarena.h"
#include "dabd.h"
#include "daatomlist.h"
#include "dacitizen.h"
#include "dadesigner.h"
#include "dagalaxy.h"
#include "daitem.h"
#include "danpc.h"
#include "daperson.h"
#include "dasector.h"
#include "dasolarsystem.h"
#include "davehicle.h"
#include "davessel.h"
#include "daworldll.h"
#include "daworldringsection.h"
#include "dgdata.h"

AuAtomInterface::AuAtomInterface()
{
}

AuAtomInterface::~AuAtomInterface()
{
}

AuAtomInterface::AuAtomInterface(const AuAtomInterface& c) : QObject()
{
	Q_UNUSED(c);
}

void AuAtomInterface::AgentEncounterAsteroid(quint32 minutesFromNow)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->AgentEncounter(2, minutesFromNow);
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsSetGroup(const AuScIdSet& parts, const QString& groupName)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->SetPartsGroup(person->Id(), parts, groupName);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignExit()
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		if(person->FindParentDesignBaseInEnv())
			person->DesignEnvironmentExit();
	}
	UnlockAvatar();
}

bool AuAtomInterface::DesignIsGravityOn()
{
	bool ison = false;
	DaPerson* person = LockAvatar();
	if(person)
	{
		const DaDesignBase* pde = person->FindParentDesignBaseInEnv();
		if(pde)
			ison = pde->IsGravityOn(person->Id());
	}
	UnlockAvatar();
	return ison;
}

bool AuAtomInterface::DesignIsRenderShaded()
{
	bool ison = false;
	DaPerson* person = LockAvatar();
	if(person)
	{
		const DaDesignBase* pde = person->FindParentDesignBaseInEnv();
		if(pde)
			ison = pde->IsRenderSolid(person->Id());
	}
	UnlockAvatar();
	return ison;
}

int AuAtomInterface::DesignMode()
{
	int mode = DaDesignBase::InvalidMode;
	DaPerson* person = LockAvatar();
	if(person)
	{
		const DaDesignBase* pde = person->FindParentDesignBaseInEnv();
		if(pde)
			mode = pde->DesignMode();
	}
	UnlockAvatar();
	return mode;
}

QString AuAtomInterface::DesignName()
{
	QString name;
	DaPerson* person = LockAvatar();
	if(person)
	{
		const DaDesignBase* pde = person->FindParentDesignBaseInEnv();
		if(pde)
			name = pde->DesignName();
	}
	UnlockAvatar();
	return name;
}

void AuAtomInterface::DropGearItem(const AuGearPos& pos)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->DropGearAt(pos);
	UnlockAvatar();
}

void AuAtomInterface::InscribeGearItem(const AuGearPos& pos, const QString& inscription)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->InscribeGearAt(pos, inscription);
	UnlockAvatar();
}

void AuAtomInterface::MoveGearItemToItem(const AuGearPos& fromPos, const AuGearPos& toPos)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->MoveGear(fromPos, toPos);
	UnlockAvatar();
}

void AuAtomInterface::MoveGearItemToSlot(const AuGearPos& fromPos, quint8 toSlotType)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->MoveGear(fromPos, (AuGearSlot::SLOTTYPE)toSlotType);
	UnlockAvatar();
}

void AuAtomInterface::UseGearItem(const AuGearPos& pos)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->UseGearAt(pos);
	UnlockAvatar();
}

void AuAtomInterface::GovDossierAddNote(quint32 dosEmpire, const QString& note)
{
	if(note.isEmpty())
		return;

	int result = 0;
	quint32 empireId = 0;
	AuId personId;
	GALAXY galaxy = GALAXY_Count;
	QString sectorName, systemName, worldName, empireName, nameWithTitle, personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		nameWithTitle = person->TitleName();
		empireId = person->EmpireId();
		const DgData& gd = g_Au.LockDataForRead();
		gd.EmpireName(dosEmpire, empireName);
		if(person->InArena())
			result = 3;
		else if(gd.EmpireCitizenCanManageDossier(empireId, personId))
		{
			result = 1;											// Ok to do it.

			galaxy = person->Galaxy();
			const DaSolarSystem* system = person->FindParentSolarSystem();
			if(system)
				systemName = system->Name();
			const DaSector* sector = person->FindParentSector();
			if(sector)
				sectorName = sector->Name();
			const DaWorld* world = person->FindParentPlanetInEnv();
			if(world)
				worldName = world->Name();
		}
		else
			result = 2;											// Person does not have authority to do that.
		g_Au.UnlockData();
	}
	UnlockAvatar();

	// Notify the player of the result.
	QString resultMsg;
	switch(result)
	{
	default:
	case 0:								// Unknown error.
		break;
	case 1:
		{
			AuDocNote docNote(personId, nameWithTitle, note, galaxy, sectorName, systemName, worldName);
			emit GovDossierAddDocument(empireId, dosEmpire, docNote.Pack());
			resultMsg = QObject::tr("Note added to the dossier on %1.").arg(empireName);
		}
		break;
	case 2:
		resultMsg = QObject::tr("Only authorized government officials can add notes to a dossier.");
		break;
	case 3:
		resultMsg = QObject::tr("Notes cannot be added to the dossier while you are in an arena.");
		break;
	}

	if(!resultMsg.isEmpty())
	{
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::GovDossierDocState(quint32 dosEmpire, const AuTime& docTime, quint8 docType, quint16 docIndex,
		quint8 docState)
{
	if(docState >= AuDocument::DOC_Count)
		return;

	int result = 0;
	quint32 empireId = 0;
	AuId personId;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		empireId = person->EmpireId();
		const DgData& gd = g_Au.LockDataForRead();
		if(gd.EmpireCitizenCanManageDossier(empireId, personId))
			result = 1;											// Ok to do it.
		else
			result = 2;											// Person does not have authority to do that.
		g_Au.UnlockData();
	}
	UnlockAvatar();

	// Notify the player of the result.
	QString resultMsg;
	switch(result)
	{
	default:
	case 0:								// Unknown error.
		break;
	case 1:
		{
			emit GovDossierChangeState(empireId, dosEmpire, docTime, docType, docIndex, docState);
			resultMsg = QObject::tr("State of dossier document changed.");
		}
		break;
	case 2:
		resultMsg = QObject::tr("Only authorized government officials can change the state of a document in a dossier.");
		break;
	}

	/*
	if(!resultMsg.isEmpty())
	{
		// Send the result on the Log channel.
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
	*/
}

void AuAtomInterface::GovDossierDispatchCourier(quint32 dosEmpire, const QString& message)
{
	if(message.isEmpty())
		return;

	bool isGov = false;
	QString empireName, nameWithTitle, personName;
	DaPerson* person = LockAvatar();
	quint32 empireId = 0;
	AuId personId;
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		nameWithTitle = person->TitleName();
		empireId = person->EmpireId();
		const DgData& gd = g_Au.LockDataForRead();
		gd.EmpireName(dosEmpire, empireName);
		isGov = gd.IsEmpireGovernor(empireId, personId);
		g_Au.UnlockData();
	}
	UnlockAvatar();

	// Notify the player of the result.
	QString resultMsg;
	if(isGov)
	{
		// Add the message received document to the other empire's dossier on my empire.
		AuDocDiplomaticMessageReceived docReceived(personId, nameWithTitle, message);
		emit GovDossierAddDocument(dosEmpire, empireId, docReceived.Pack());

		// Add the message sent document to my dossier on the other empire.
		AuDocDiplomaticMessageSent docSent(personId, nameWithTitle, message);
		emit GovDossierAddDocument(empireId, dosEmpire, docSent.Pack());

		resultMsg = tr("The courier accepted my message for %1. It will be delivered promptly and securely.").arg(empireName);
	}
	else
		resultMsg = tr("Only government officials can send diplomatic messages.");

	if(!resultMsg.isEmpty())
	{
		// Send the result on the Log channel.
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::AlertCondition(int condition)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* vessel = person->FindParentVesselInEnv();
		if(vessel)
			vessel->SetAlertCondition((ALERT_CONDITION)condition);
	}
	UnlockAvatar();
}

void AuAtomInterface::HandcraftGo(int mpid)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->HandcraftItem(mpid);
	UnlockAvatar();
}

void AuAtomInterface::SetPictureFrame(const AuGearPos& pos, const QString& url, bool hideURL, bool rotate)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->SetPictureFrame(pos, url, hideURL, rotate);
	UnlockAvatar();
}

void AuAtomInterface::SetFlowerPot(const AuGearPos& pos, quint8 plantSize)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->SetFlowerPot(pos, plantSize);
	UnlockAvatar();
}

void AuAtomInterface::SetMessage(const AuGearPos& pos, const QString& message)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->SetMessage(pos, message);
	UnlockAvatar();
}

void AuAtomInterface::DesignerNew(const AuDesign& design)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde && person->Is(pde->Owner()))
			pde->NewDesign(person->Id(), person->Name(), design);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerSetProperties(const AuDesign& design)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
		{
			pde->SetProperties(design);
			pde->SetModified();
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartAdd(const AuScPart& part)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
		{
			pde->PartAdd(person->Id(), part);
			pde->SetModified();
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartUpdate(quint16 partId, const AuScPart& part)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
		{
			pde->PartUpdate(person->Id(), partId, part);
			pde->SetModified();
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerLoadedFromFile(AuDesign& design)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
		{
			// Clear architect information so sealed parts are not a problem.
			design.SetArchitectAfterFileLoad(person->Id(), person->Name());
			pde->LoadedFromFile(design);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerCopyParts(const AuScSelection& partIds, const AuMatrix4x4& t)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->CopyParts(person->Id(), partIds, t);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerCopyPartObjects(const AuScSelection& partIds, const AuMatrix4x4& t)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->CopyPartObjects(person->Id(), partIds, t);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerMoveLiftStops(const AuScSelection& partIds, const AuMatrix4x4& t)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->MoveLiftStops(person->Id(), partIds, t);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerMoveParts(const AuScSelection& partIds, const AuMatrix4x4& t)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->MoveParts(person->Id(), partIds, t);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerMovePartObjects(const AuScSelection& partIds, const AuMatrix4x4& t)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->MovePartObjects(person->Id(), partIds, t);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerDeleteParts(const AuScSelection& partIds)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->DeleteParts(person->Id(), partIds);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerDeletePartObjects(const AuScSelection& partIds)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->DeletePartObjects(person->Id(), partIds);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerAddParts(const AuScParts& parts)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->AddParts(person->Id(), parts);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerAddPartObjects(quint16 partId, const AuScParts& parts)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->AddPartObjects(person->Id(), partId, parts);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerRedo()
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->Redo(person->Id());
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerUndo()
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->Undo(person->Id());
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerJoinParts(quint16 partId, const AuScSelection& sel)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->JoinParts(person->Id(), partId, sel);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerReverseFaces(const AuScSelection& sel)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->ReverseFaces(person->Id(), sel);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerSplitFaces(const AuScSelection& sel)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->SplitFaces(person->Id(), sel);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerFlattenVertices(const AuScSelection& sel)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->FlattenVertices(person->Id(), sel);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerSmooth(const AuScSelection& sel)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->SmoothVertices(person->Id(), sel);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerJigCut(const AuScSelection& sel)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->JigCutParts(person->Id(), sel);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartFillFace(const AuScSelection& sel, const AuPointArray& boundary, const AuMatrix4x4& gridPlane)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->PartFillFace(person->Id(), sel, boundary, gridPlane);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartSetStopName(quint16 partId, const QString& name)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->SetPartStopName(person->Id(), partId, name);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartSetState(quint16 partId, quint8 partState)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->SetPartState(person->Id(), partId, partState);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsSetHidden(const AuScIdSet& parts, bool hide)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->SetPartsHidden(person->Id(), parts, hide);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsSetLocked(const AuScIdSet& parts, bool lock)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->SetPartsLocked(person->Id(), parts, lock);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsSetOmitted(const AuScIdSet& parts, bool omit)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->SetPartsOmitted(person->Id(), parts, omit);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsSetSeal(const AuScIdSet& parts, bool seal)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->SetPartsSealed(person->Id(), parts, seal);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartAddState(quint16 partId, quint8 state)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->AddPartState(person->Id(), partId, state);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartAddFaces(quint16 partId, const AuScFaceList& faces)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->PartAddFaces(person->Id(), partId, faces);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartDeleteState(quint16 partId, quint8 state)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->DeletePartState(person->Id(), partId, state);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartReplaceFaces(quint16 partId, const AuScSelection& sel, const AuScFaceList& faces)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->PartReplaceFaces(person->Id(), partId, sel, faces);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerCutFaces(const AuScSelection& sel, const AuPointArray& cutLine)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->CutFaces(person->Id(), sel, cutLine);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerFractureFaces(const AuScSelection& sel)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->FractureFaces(person->Id(), sel);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerFuseFaces(const AuScSelection& sel)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->FuseFaces(person->Id(), sel);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerDivideFaces(const AuScSelection& sel, quint8 divisions)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->DivideFaces(person->Id(), sel, divisions);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerMakeNavLightLens(const AuScSelection& partIds, const QColor& color)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->MakeNavLightLens(person->Id(), partIds, color);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerMakeDoor(const AuScSelection& partIds)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->MakeDoor(person->Id(), partIds);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerMakeLightLens(const AuScSelection& partIds, const QColor& lightColor)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->MakeLightLens(person->Id(), partIds, lightColor);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerMakeLandingGear(const AuScSelection& partIds)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->MakeLandingGear(person->Id(), partIds);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerMakeRotatingBeaconLens(const AuScSelection& partIds, const QColor& lightColor,
		const AuPoint& rotationAxis)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->MakeRotatingBeaconLens(person->Id(), partIds, lightColor, rotationAxis);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerMakeStrobeLens(const AuScSelection& partIds, const QColor& lightColor)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->MakeStrobeLens(person->Id(), partIds, lightColor);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerMakeTurboLift(const AuScSelection& partIds)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->MakeTurboLift(person->Id(), partIds);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerMakeTurret(const AuScSelection& partIds, const AuPoint& basePivot, const AuPoint& baseAxis)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->MakeTurret(person->Id(), partIds, basePivot, baseAxis);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerMakeWindow(const AuScSelection& partIds, float opacity)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->MakeWindow(person->Id(), partIds, opacity);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerTurretStation(const AuScSelection& sel, quint8 gunnerPos, const AuPoint& gunnerLoc,
		const AuPoint& gunnerFacing, const AuPoint& corner1, const AuPoint& corner2, const AuMatrix4x4& gridPlane,
		COMMODITYID module)
{
	bool didIt = false;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			didIt = pde->SetTurretStation(person->Id(), sel, (AuScTurret::GUNNER_POSITION)gunnerPos, gunnerLoc, gunnerFacing,
					corner1, corner2, gridPlane, module);
	}
	UnlockAvatar();
	if(didIt)
	{
		AuCommThought msg(QObject::tr("Turret station configured successfully."), personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::DesignerTurretBase(const AuScSelection& sel, const AuPoint& basePivot, const AuPoint& baseAxis)
{
	bool didIt = false;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			didIt = pde->SetTurretBase(person->Id(), sel, basePivot, baseAxis);
	}
	UnlockAvatar();
	if(didIt)
	{
		AuCommThought msg(QObject::tr("Turret base axis configured successfully."), personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::DesignerTurretGun(const AuScSelection& sel, const AuPoint& gunPivot, const AuPoint& gunAxis,
		const AuPoint& gunEmitLoc, const AuPoint& gunEmitVec, const AuPoint& gunSightLoc)
{
	bool didIt = false;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			didIt = pde->SetTurretGun(person->Id(), sel, gunPivot, gunAxis, gunEmitLoc, gunEmitVec, gunSightLoc);
	}
	UnlockAvatar();
	if(didIt)
	{
		AuCommThought msg(QObject::tr("Turret gun configured successfully."), personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::DesignerPartSetOpacity(quint16 partId, float opacity)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->SetPartOpacity(person->Id(), partId, opacity);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerMakeHull(const AuScSelection& partIds)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->MakeHull(person->Id(), partIds);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerMakeMesh(const AuScSelection& partIds)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->MakeMesh(person->Id(), partIds);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerMakeRoom(const AuScSelection& partIds)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->MakeRoom(person->Id(), partIds);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartAddStop(quint16 partId, const QString& stop)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->AddPartStop(person->Id(), partId, stop);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartDeleteStop(quint16 partId, const QString& stop)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->DeletePartStop(person->Id(), partId, stop);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartSetStop(quint16 partId, const QString& stop)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->SetPartStop(person->Id(), partId, stop);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPaintFaces(const AuScSelection& sel, const QColor& paint)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->PaintFaces(person->Id(), sel, paint);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPaintFaces(const AuScSelection& sel, quint8 theme, quint8 opacity)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->PaintFaces(person->Id(), sel, (AuScFace::COLOR_THEME)theme, opacity);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerGlowVertices(const AuScSelection& sel, const QColor& glow)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->GlowVertices(person->Id(), sel, glow);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPolishFaces(const AuScSelection& sel, float shine, float reflectivity)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->PolishFaces(person->Id(), sel, shine, reflectivity);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsSetFaceOpacity(const AuScSelection& sel, float opacity)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->SetFaceOpacity(person->Id(), sel, opacity);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsSetFaceSelfIllumination(const AuScSelection& sel, float selfIllum)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->SetFaceSelfIllumination(person->Id(), sel, selfIllum);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerSetTexturePalette(const AuDesignTex texturePalette[SCTEXI_Count])
{
	// Validate the palette.
	for(int tpi=0; tpi<SCTEXI_Count; tpi++)
		if(!texturePalette[tpi].IsValid())
			return;

	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->SetTexturePalette(texturePalette);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsTextureFaces(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->TextureFaces(person->Id(), sel, texIndex);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsTextureFacesCylindrical(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex,
		quint16 texRepsAtEquator, double texSize, const AuMatrix4x4& gridPlane)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->TextureFacesCylindrical(person->Id(), sel, texIndex, texRepsAtEquator, texSize, gridPlane);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsTextureFacesProject(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex, double texSize,
		const AuMatrix4x4& gridPlane)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->TextureFacesProject(person->Id(), sel, texIndex, texSize, gridPlane);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsTextureFacesSpherical(const AuScSelection& sel, SC_TEXTURE_INDEX texIndex,
		quint16 texRepsAtEquator, const AuMatrix4x4& gridPlane)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->TextureFacesSpherical(person->Id(), sel, texIndex, texRepsAtEquator, gridPlane);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsEffectFaces(const AuScSelection& sel, SURFACE_EFFECT effect)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->EffectFaces(person->Id(), sel, effect);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsEffectFacesCylindrical(const AuScSelection& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		double texSize, const AuMatrix4x4& gridPlane)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->EffectFacesCylindrical(person->Id(), sel, effect, texRepsAtEquator, texSize, gridPlane);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsEffectFacesProject(const AuScSelection& sel, SURFACE_EFFECT effect, double texSize,
		const AuMatrix4x4& gridPlane)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->EffectFacesProject(person->Id(), sel, effect, texSize, gridPlane);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsEffectFacesSpherical(const AuScSelection& sel, SURFACE_EFFECT effect, quint16 texRepsAtEquator,
		const AuMatrix4x4& gridPlane)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->EffectFacesSpherical(person->Id(), sel, effect, texRepsAtEquator, gridPlane);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsEffectMove(AuScSelection sel, QPointF vec, quint8 undoSeq)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->EffectMove(person->Id(), sel, vec, undoSeq);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsEffectRotate(AuScSelection sel, QPointF pivot, double angle, quint8 undoSeq)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->EffectRotate(person->Id(), sel, pivot, angle, undoSeq);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsEffectStretch(AuScSelection sel, QPointF base, double scaleX, double scaleY, quint8 undoSeq)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->EffectStretch(person->Id(), sel, base, scaleX, scaleY, undoSeq);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsEffectUnwrap(AuScSelection sel, SURFACE_EFFECT effect, double texSize,
		AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->EffectUnwrap(person->Id(), sel, effect, texSize, unwrap, angleTol, gridAlign, alignAxis, gridPlane);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerMakeDecal(const AuScSelection& partIds)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->MakeDecal(person->Id(), partIds);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerCopyPartsSymmetrical(const AuScSelection& partIds, const AuMatrix4x4& gridPlane)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->CopyPartsSymmetrical(person->Id(), partIds, gridPlane);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsTextureMove(AuScSelection sel, QPointF vec, quint8 undoSeq)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->TextureMove(person->Id(), sel, vec, undoSeq);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsTextureRotate(AuScSelection sel, QPointF pivot, double angle, quint8 undoSeq)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->TextureRotate(person->Id(), sel, pivot, angle, undoSeq);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsTextureStretch(AuScSelection sel, QPointF base, double scaleX, double scaleY, quint8 undoSeq)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->TextureStretch(person->Id(), sel, base, scaleX, scaleY, undoSeq);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsTextureUnwrap(AuScSelection sel, SC_TEXTURE_INDEX texIndex, double texSize,
			AuScFaceList::UNWRAP_METHOD unwrap, float angleTol, bool gridAlign, int alignAxis, const AuMatrix4x4& gridPlane)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->TextureUnwrap(person->Id(), sel, texIndex, texSize, unwrap, angleTol, gridAlign, alignAxis, gridPlane);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerAssociateParts(const AuScSelection& partIds)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->AssociateParts(person->Id(), partIds);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerSetGravity(bool gravOn)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			person->DesignSetGravity(gravOn);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerSetPreview(quint8 previewType, const AuDesign* design, quint32 genSeed)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->SetPreview(previewType, design, genSeed);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsSetDoorProperties(const AuScSelection& partIds, Qt::CheckState isAutomatic,
		Qt::CheckState initLocked, Qt::CheckState remoteSpacecraft, Qt::CheckState remoteVehicle)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->SetDoorProperties(person->Id(), partIds, isAutomatic, initLocked, remoteSpacecraft, remoteVehicle);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsSetSiteProperties(const AuScSelection& partIds, quint8 grade, quint8 grass, quint8 pavement,
		quint8 rocks, quint8 shrubs, quint8 terBlend, quint8 trees)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->SetSiteProperties(person->Id(), partIds, grade, grass, pavement, rocks, shrubs, terBlend, trees);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsSetVoidProperties(const AuScSelection& partIds, GRAVITY gravityType, const AuPoint& gravityUp,
		HOLD_ACCESS holdAccess, quint8 personnelAccess)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->SetVoidProperties(person->Id(), partIds, gravityType, gravityUp, holdAccess, personnelAccess);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsSetRoomProperties(const AuScSelection& partIds, bool isSunlit, INTERIOR_LIGHT initLight)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->SetRoomProperties(person->Id(), partIds, isSunlit, initLight);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartFillQuad(const AuScSelection& sel, const AuPointArray& boundary)
{
	DesignerPartFillQuad(sel, boundary[0], boundary[1], boundary[2], boundary[3]);
}

void AuAtomInterface::DesignerPartFillQuad(const AuScSelection& sel, const AuPoint& v1, const AuPoint& v2, const AuPoint& v3,
	const AuPoint& v4)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->PartFillQuad(person->Id(), sel, v1, v2, v3, v4);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartFillTriangle(const AuScSelection& sel, const AuPointArray& boundary)
{
	DesignerPartFillTriangle(sel, boundary[0], boundary[1], boundary[2]);
}

void AuAtomInterface::DesignerPartFillTriangle(const AuScSelection& sel, const AuPoint& v1, const AuPoint& v2, const AuPoint& v3)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->PartFillTriangle(person->Id(), sel, v1, v2, v3);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPunchFaces(const AuScSelection& sel, const AuPointArray& cutBnd)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->PunchFaces(person->Id(), sel, cutBnd);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerCheckDoors(const AuScSelection& partIds)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->UpdateHullDoors(person->Id(), partIds);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerDissociateParts(const AuScSelection& partIds)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->DissociateParts(person->Id(), partIds);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerGoToSelection(const AuScSelection& sel)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->GoToSelection(person->Id(), sel);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsMoveDoorSwitches(const AuScSelection& partIds, const AuPoint& switch1, const AuPoint& switch2)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->MoveDoorSwitches(person->Id(), partIds, switch1, switch2);
	}
	UnlockAvatar();
}

void AuAtomInterface::CallTurboLift(const AuId& shipId, quint16 liftId, const QString& stop)
{
	int result = 99;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		const DaAtom* env = person->FindEnvironmentAtom();
		if(env)
		{
			DaCapsule* ship = dynamic_cast<DaCapsule*>(env->FindIdRecursive(shipId));
			if(ship)
				result = ship->CallTurboLift(liftId, stop);
		}
	}
	UnlockAvatar();

	AuCommThoughtMessage::MESSAGE tm = AuCommThoughtMessage::MessageCount;
	switch(result)
	{
	case 0: //  0=lift already at stop, door already open
	case 1: //  1=no door at turbo lift stop
	case 4: //  4=broken turbo lift data
	case 5: //  5=turbo lift moved to stop and door opened
	case 6: //  6=lift already at stop, door opened
		break;
	case 2: //  2=not ready to move
		tm = AuCommThoughtMessage::LiftCalled;
		break;
	case 3: //  3=a door is stuck open,
		tm = AuCommThoughtMessage::LiftCalledDoorStuck;
		break;
	case 7: //  7=a door is blocked open
		tm = AuCommThoughtMessage::LiftCalledDoorBlocked;
		break;
	}
	if(tm != AuCommThoughtMessage::MessageCount)
	{
		AuCommThoughtMessage msg(tm, stop, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::DesignerPartsTransporterPadLocs(const AuScSelection& partIds, const AuPointArray& padLocs)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->TransporterPadLocs(person->Id(), partIds, padLocs);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsWeaponEmitLoc(const AuScSelection& partIds, const AuPoint& emitLoc)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->WeaponEmitLoc(person->Id(), partIds, emitLoc);
	}
	UnlockAvatar();
}

void AuAtomInterface::BdPreview(const AuPoint& fBuildLoc, float zRotation)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdPreview(fBuildLoc, zRotation);
	UnlockAvatar();
}

void AuAtomInterface::BdPreviewFence(const AuPoint& startLoc, const AuPoint& endLoc)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdPreviewFence(startLoc, endLoc);
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsSetLightBulbProperties(const AuScSelection& partIds, const QColor& color, float kL, float kQ,
		float cosOuterCone, float cosInnerCone)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->SetLightBulbProperties(person->Id(), partIds, color, kL, kQ, cosOuterCone, cosInnerCone);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartsSetLightLensProperties(const AuScSelection& partIds, const QColor& color)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->SetLightLensProperties(person->Id(), partIds, color);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPartRenameStop(quint16 partId, const QString& oldStopName, const QString& nuStopName)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->RenamePartStop(person->Id(), partId, oldStopName, nuStopName);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerEffectIntensityVertices(const AuScSelection& sel, float intensity)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->EffectIntensityVertices(person->Id(), sel, intensity);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerPreviewPeople(const AuSpecies& species)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->PreviewPeople(species);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerViewPositionLoad(const AuId& avatarId, quint8 number)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->ViewPositionLoad(avatarId, number);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerViewPositionGrid(const AuId& avatarId, const AuDesignPosition& pos)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->ViewPositionGrid(avatarId, pos);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerViewPositionOrigin(const AuId& avatarId)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->ViewPositionOrigin(avatarId);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerViewPositionSave(const AuId& avatarId, quint8 number)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->ViewPositionSave(avatarId, number);
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignerGridPositionSave(quint8 number, const AuMatrix4x4& gridM)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
			pde->GridPositionSave(person->Id(), number, gridM);
	}
	UnlockAvatar();
}

void AuAtomInterface::AgentGoToAtom(const AuId& destAtomId, bool rideAlso, bool limboRescue)
{
	AuId returnOnFail;
	DaPerson* person = LockAvatar();
	if(person)
	{
		returnOnFail = person->ParentId();

		// Allow ship to go too.
		// Only architect agents can do limbo rescue.
		emit AdminGoTo(person->Id(), destAtomId, rideAlso ? 2 : 0, limboRescue, returnOnFail);
	}
	UnlockAvatar();
}

void AuAtomInterface::AgentGoToGalaxyLoc(quint8 galaxyEnum, const AuPoint& galaxyLoc, bool rideAlso)
{
	AuId travId, systemId;
	DaPerson* person = LockAvatar();
	if(person)
	{
		const DaSolarSystem* system = person->FindParentSolarSystemInEnv();
		if(system)
			systemId = system->Id();
		travId = person->Id();
		if(rideAlso)
		{
			DaAtom* par = person->Parent();
			if(par &&
					(dynamic_cast<DaVehicle*>(par) ||
					 dynamic_cast<DaVessel*>(par) ||
					 dynamic_cast<DaCreature*>(par)))
				travId = par->Id();
		}
	}
	UnlockAvatar();
	if(!travId.IsNull() && !systemId.IsNull())
		emit AdminGoToGalaxyLoc(travId, systemId, AuId::GalaxyId((GALAXY)galaxyEnum), galaxyLoc);
}

void AuAtomInterface::GearDeleteStoredData(const AuGearPos& srcPos, const QSet<COMMODITYID>& patentCIDs,
		const QSet<quint32>& scDesigns, const QSet<quint32>& bdDesigns, const QSet<QString>& missions, const QSet<QString>& papers)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->DeleteStoredData(srcPos, patentCIDs, scDesigns, bdDesigns, missions, papers);
	UnlockAvatar();
}

void AuAtomInterface::CollectBounty(const AuGearPos& pos)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		const AuGear* pinv = person->LockGearForRead();
		const AuItem* item = pinv->ItemAt(pos);
		AuBountyOnItem bounty = item->Bounty();
		if(!bounty.Id().IsNull() && person->Is(bounty.Id()))
			bounty = AuBountyOnItem();  // Cannot collect bounty on myself.
		person->UnlockGear();
		if(!bounty.Id().IsNull())
			emit CollectBounty(person->Id(), person->Name(), pos, bounty);
	}
	UnlockAvatar();
}

void AuAtomInterface::SendFunds(const AuId& toId, const QString& toName, double amount)
{
	// Don't permit amounts less than 1000c.
	if(amount < 1000)
		return;
	AuId personId;
	QString fromName;
	double oldBalance = 0;
	double nuBalance = 0;
	GALAXY fromGalaxy = GALAXY_Count;
	DaPerson* person = LockAvatar();
	if(person)
	{
		// Get info about the sending person.
		personId = person->Id();
		fromName = person->Name();
		oldBalance = person->AccountBalance();
		if(!person->HasProgrammerPower())
		{
			// Agent sends GALAXY_Count so range is not restricted.
			fromGalaxy = person->Galaxy();

			// Limit the transfer to their current account balance.
			if(amount > oldBalance)
				amount = oldBalance;

			// Take the money from the sender's account.
			if(amount >= 1000)
			{
				person->DebitAccount(amount);
				nuBalance = person->AccountBalance();
			}
		}
		else
			nuBalance = oldBalance;
	}
	UnlockAvatar();

	// Amount must be at least the minimum, to prevent abuse.
	if(amount >= 1000)
	{
		// Send a receipt to the person who sent the money.
		// The receiver gets their receipt when their atom is located.
		QString sAmount = QString("%L1%2").arg(amount,0,'f',0).arg(g_Au.CurrencySymbol());
		QString sOldBalance = QString("%L1%2").arg(oldBalance,0,'f',0).arg(g_Au.CurrencySymbol());
		QString sNewBalance = QString("%L1%2").arg(nuBalance,0,'f',0).arg(g_Au.CurrencySymbol());
		QString sReceipt = tr("Old account balance %1<br>%2 sent to %3<br>New account balance %4.")
			.arg(sOldBalance).arg(sAmount).arg(toName).arg(sNewBalance);

		// Get this server's login credentials to send mail.
		const DgData& gd = g_Au.LockDataForRead();
		unsigned int acctId = gd.PlayerId();	  // The local server must be "logged in" as someone to send mail.
		unsigned int confirmNum = gd.ConfirmationNumber();
		g_Au.UnlockData();
		g_Au.SendMail(acctId, confirmNum, AuId::BankMail(), tr("Bank"), personId, AuMailMsg::MSG_FundsTransfer,
				tr("Funds Transfer Receipt"), sReceipt, AuId(0), "", AuId(0), "", GALAXY_Count, AuPoint(0,0,0));

		// Notify the server farm to credit toId with the money.
		emit SendFunds(personId, fromName, toId, amount, fromGalaxy, 0, 0);
	}
}

void AuAtomInterface::GetFromCargo(const AuItem& item)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->GetFromCargo(item);
	UnlockAvatar();
}

void AuAtomInterface::PutIntoCargo(const AuItem& item)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->PutIntoCargo(item);
	UnlockAvatar();
}

void AuAtomInterface::WithdrawShipFunds(double amount, const AuId& shipId)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		// Only an officer of the ship can withdraw funds.
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship && ship->Is(shipId) && ship->InChainOfCommand(person))
		{
			if(amount > ship->AccountBalance())
				amount = ship->AccountBalance();
			if(amount > 0)
			{
				ship->DebitAccount(amount);
				person->CreditAccount(amount);
			}
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::DepositShipFunds(double amount, const AuId& shipId)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		// Any person can deposit funds into the ship they are aboard.
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship && ship->Is(shipId))
		{
			if(amount > person->AccountBalance())
				amount = person->AccountBalance();
			if(amount > 0)
			{
				ship->CreditAccount(amount);
				person->DebitAccount(amount);
			}
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::SetSpeciesName(const AuSpecies& species, const QString& speciesName, bool isWildAnimal)
{
	// Validate the values before propagating the specie naming request.
	if(!IsNameValid(speciesName))
		return;
	AuDNABase* dna = species.CreateDNA();
	if(!dna)
		return;
	delete dna;

	const DgData& gd = g_Au.LockDataForRead();
	const AuId& whoNamed = gd.SpeciesNamedBy(species);	// Returns null id if not named.
	QString oldName = gd.SpeciesName(species, true);	// Returns description if it's not named.
	g_Au.UnlockData();

	DaPerson* person = LockAvatar();
	if(person)
	{
		bool okToRename = whoNamed.IsNull() || whoNamed == person->Id() || person->HasAgentPower();

		QString resultMsg;
		if(okToRename)
		{
			if(whoNamed.IsNull())
			{
				// First time this specie has been named.
				// Update reputation and stats on the person atom.
				person->IncreaseReputation(1);
				person->Stats().CreatureSpeciesNamed(species, isWildAnimal);
				person->IncreaseSkill(AuSkill::Exobiology, 1);
				person->SetModified();
				resultMsg = tr("This %1 specie has never been named. I am calling it %2.").arg(oldName).arg(speciesName);
			}
			else if(whoNamed == person->Id())
				resultMsg = tr("I am changing the name of the %1 specie. I am calling it %2.").arg(oldName).arg(speciesName);
			else
				resultMsg = tr("Using my agent power, I am changing the name of the %1 specie. I am calling it %2.")
					.arg(oldName).arg(speciesName);

			// Propagate the request to name/rename the specie.
			emit NameSpecies(species, speciesName, person->Id());
		}
		else if(AcctIsAgent(person->AccountTypeOnLine()))
			resultMsg = tr("That %1 specie was already named %2 by someone else. "
					"I could change it if my agent powers were enabled.")
				.arg(species.Description()).arg(oldName);
		else
			resultMsg = tr("That %1 specie was already named %2 by someone else.")
				.arg(species.Description()).arg(oldName);

		// Inform the player of the result.
		AuCommThought msg(resultMsg, person->Id(), person->Name());
		g_Au.SendAtomChat(msg);
	}
	UnlockAvatar();
}

void AuAtomInterface::ClearGeo(const AuGlobeLLF& geoLL)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaWorldLL* llworld = person->FindParentWorldInEnv();
		if(llworld)
		{
			if(!llworld->EnemyUnitBlocksDestruction(person->EmpireId()))
			{
				// Check to see if the person is authorized to delete roads.
				if(!llworld->InChainOfCommand(person->Id()))
				{
					AuCommThought msg(tr("I cannot demolish that because I am not in the world's chain of command."),
					person->Id(), person->Name());
					g_Au.SendAtomChat(msg);
				}
				else if(llworld->DeleteGeologicalFeature(geoLL))
				{
					// Feature deleted.
				}
			}
			else
			{
				AuCommThought msg(tr("Nothing can be demolished while an enemy unit is present on this world."),
				person->Id(), person->Name());
				g_Au.SendAtomChat(msg);
			}
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::DesignSetPassword(const QString& pass)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		person->SetDesignPassword(pass);
		person->SetModified();
	}
	UnlockAvatar();
}

quint8 AuAtomInterface::DesignOpen(quint32 designId, int designMode)
{
	quint8 designStudioMode = DaDesignBase::InvalidMode;	// Return value
	AuId personId;
	QString personName;
	int result = 99;
	AuId shipId;
	QString shipName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
		{
			// Only the owner of the design studio can open a nu design.
			// Agent-level admin can open any design.
			if(pde->Owner() == person->Id() || person->HasAgentPower())
			{
				// This could change the design studio mode.
				if(designMode == DaDesignBase::DESIGN_Spacecraft)
					result = pde->LoadScDesign(designId, person->Id(), false);
				else if(designMode == DaDesignBase::DESIGN_Building)
					result = pde->LoadBdDesign(designId, person->Id(), false);
				else if(designMode == DaDesignBase::DESIGN_Item)
					result = pde->LoadItDesign(designId, person->Id(), false);
				else
					result = 94;
				designStudioMode = pde->DesignMode();
			}
			else
				result = 98;
		}
		else if(AcctIsProgrammer(person->AccountTypeOnLine()))
		{
			// Not in a design studio. Programmer can open a designer instance anywhere.
			// Programmer-level admin spawns ship into existence.
			QString resultMsg;
			DaWorld* planet = person->FindParentPlanetInEnv();
			DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(planet);
			if(llworld)
			{
				quint8 errcode;
				DaVessel* ship = llworld->AgentSpawnSpacecraft(person, designId, errcode);
				if(ship)
				{
					// When architect powers are enabled, the ship is randomly named.
					if(person->HasArchitectPower())
					{
						ship->SetName(g_Au.Dice().RandomShipName());
						ship->SetModified();
						result = 95;
					}
					else
						result = 91;
					shipId = ship->Id();
					shipName = ship->Name();
				}
				else
				{
					switch(errcode)
					{
					case 1:
						result = 93;
						break;
					case 2:
						result = 1;
						break;
					case 6:
						result = 96;
						break;
					case 27:
						result = 97;
						break;
					}
				}
			}
			else
				result = 92;
		}
	}
	UnlockAvatar();

	QString resultMsg;
	switch(result)
	{
	case 0:
		resultMsg = tr("Blueprint %1 opened successfully.").arg(designId);
		break;
	case 1:
		if(designMode == DaDesignBase::DESIGN_Spacecraft)
			resultMsg = tr("Library is loading spacecraft blueprint %1. Try again shortly.").arg(designId);
		else if(designMode == DaDesignBase::DESIGN_Building)
			resultMsg = tr("Library is loading building blueprint %1. Try again shortly.").arg(designId);
		else if(designMode == DaDesignBase::DESIGN_Item)
			resultMsg = tr("Library is loading item blueprint %1. Try again shortly.").arg(designId);
		else
			resultMsg = tr("Library is loading the blueprint. Try again shortly.");
		break;
	case 2:
		resultMsg = tr("Blueprint %1 is broken.").arg(designId);
		break;
	case 91:
		resultMsg = tr("Spacecraft created %1 %2.").arg(shipId.GetID()).arg(shipName);
		break;
	case 92:
		resultMsg = tr("I must move to a world to create spacecraft.");
		break;
	case 93:
		resultMsg = tr("An unlikely error occurred.");
		break;
	case 94:
		resultMsg = tr("Design mode %1 is not recognized.").arg(designMode);
		break;
	case 95:
		resultMsg = tr("Spacecraft created %1 named %2.").arg(shipId.GetID()).arg(shipName);
		break;
	case 96:
		resultMsg = tr("Spawn location was blocked.");
		break;
	case 97:
		resultMsg = tr("Blueprint %1 is not valid for manufacture.").arg(designId);
		break;
	case 98:
		resultMsg = tr("I cannot open a blueprint; I am not the owner of this design studio.");
		break;
	case 99:
	default:
		resultMsg = tr("Blueprint %1 could not be opened.").arg(designId);
		break;
	}
	AuCommThought msg(resultMsg, personId, personName);
	g_Au.SendAtomChat(msg);
	return designStudioMode;
}

void AuAtomInterface::StoryChoicePicked(quint32 storyInstance, qint8 value, const AuId& offeredBy)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		const AuScripts* scripts = g_Au.LockScripts();
		AuStory* story = g_Au.LockStory(storyInstance);
		person->StoryChoicePicked(storyInstance, value, story, *scripts, offeredBy);
		g_Au.UnlockStory(storyInstance);
		g_Au.UnlockScripts();
	}
	UnlockAvatar();
}

void AuAtomInterface::CloseJob(const AuJob* jobToClose)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->JobClosed(jobToClose);
	UnlockAvatar();
}

void AuAtomInterface::CloseStory(quint32 storyInstance)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		AuStory* story = g_Au.LockStory(storyInstance);
		person->DepartStory(storyInstance, story);		// 'story' can be null.
		g_Au.UnlockStory(storyInstance);
	}
	UnlockAvatar();
}

void AuAtomInterface::GovSetCapital(int capitalType)
{
	int result = 0;
	AuSpecies species;
	quint32 empireId = 0;
	AuId inCity;
	QString cityName;
	QString domainName;
	QString empireName;
	AuId domainId;	// NULL if capitalType == 0
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		if(!person->BdId().IsNull() && person->InIndustry() == IID_Capitol)
		{
			// Person is standing at a capitol building.
			const DaGalaxy* galaxy = person->FindParentGalaxy();
			const DaSector* sector = person->FindParentSector();
			const DaWorld* planet = person->FindParentPlanetInEnv();
			if(galaxy && sector && planet)
			{
				const DaBd* bd = dynamic_cast<const DaBd*>(planet->FindIdRecursive(person->BdId()));
				if(bd && bd->EmpireId() == person->EmpireId())
				{
					bool hasAuth = false;
					const DgData& gd = g_Au.LockDataForRead();
					switch(capitalType)
					{
					case 0:	// sector capital
						{
							AuIdList placeChain;
							sector->ChainOfPlaces(placeChain);
							hasAuth = gd.EmpireCitizenCanSetSectorCapital(person->EmpireId(), personId, placeChain);
							domainId = sector->Id();	// need sector id when assigning sector capital
							domainName = sector->Name();
						}
						break;
					case 1:	// empire capital
						hasAuth = gd.EmpireCitizenCanSetCapital(person->EmpireId(), personId);
						break;
					case 3:	// galaxy capital
						{
							AuIdList placeChain;
							sector->ChainOfPlaces(placeChain);
							hasAuth = gd.EmpireCitizenCanSetGalaxyCapital(person->EmpireId(), personId, placeChain);
							domainId = galaxy->Id();	// need galaxy id when assigning galaxy capital
							domainName = galaxy->Name();
						}
						break;
					default:
						break;
					}
					g_Au.UnlockData();
					if(hasAuth)
					{
						species = bd->ResidentSpecies();
						inCity = bd->CapitolId();
						cityName = bd->CityName();
						empireId = bd->EmpireId();

						// Empire name
						AuId empireCapitolId;
						const DgData& gd = g_Au.LockDataForRead();
						gd.EmpireCapital(empireId, empireCapitolId);
						gd.EmpireName(empireId, empireName);
						g_Au.UnlockData();

						if(capitalType == 1 || !empireCapitolId.IsNull())
							result = 1;						// Ok to set capital.
						else
							result = 6;						// Need empire capital to set a sector or galaxy capital.
					}
					else
						result = 4;							// Person does not have authority to do that.
				}
			}
		}
		else
			result = 2;											// Person not in a city of the empire.
	}
	UnlockAvatar();

	// Notify the player of the result.
	QString resultMsg;
	switch(result)
	{
	default:
	case 0:								// Unknown error.
	case 2:								// Not in a city.
		resultMsg = tr("Capital city was not changed. I must stand at a capitol building to change the capital.");
		break;
	case 1:								// Ok to set capital
		{
			emit GovSetCapital(empireId, inCity, species, domainId, capitalType);

			switch(capitalType)
			{
			case 0:	// sector capital
				resultMsg = tr("%1 is now the sector capital of %2. "
						"Cities with banks collect taxes. "
						"Set tax rates using the Governance window. "
						"50% of taxes collected by %3 cities in %2 will be sent to %1 once per day. "
						"50% of taxes collected by %1 will be sent to the galaxy capital, or to the empire capital, once per day.")
					.arg(cityName).arg(domainName).arg(empireName);
				break;
			case 1:	// empire capital
				resultMsg = tr("%1 is now the capital city of %2. "
						"Cities with banks collect taxes. "
						"Set tax rates using the Governance window. "
						"50% of taxes collected by %2 cities is sent to their capital once per day. "
						"Taxes are sent to their sector capital, to their galaxy capital, or to %1, "
						"in that order.")
					.arg(cityName).arg(empireName);
				break;
			case 3:	// galaxy capital
				resultMsg = tr("%1 is now the galaxy capital of %2. "
						"Cities with banks collect taxes. "
						"Set tax rates using the Governance window. "
						"50% of taxes collected by %3 cities in %2 will be sent to %1, or to their sector capital, once per day. "
						"50% of taxes collected by %1 will be sent to the empire capital once per day.")
					.arg(cityName).arg(domainName).arg(empireName);
				break;
			}
		}
		break;
	case 4:								// Person has no authority
		resultMsg = tr("Capital city was not changed. I do not have governing authority to declare it the capital.");
		break;
	case 6:								// No sector capital without empire capital.
		if(capitalType == 0)
			resultMsg = tr("Sector capital city was not changed. "
					"Empire capital must be declared before declaring a sector capital.");
		else
			resultMsg = tr("Galaxy capital city was not changed. "
					"Empire capital must be declared before declaring a galaxy capital.");
		break;
	}

	// Send the result on the Log channel.
	AuCommThought msg(resultMsg, personId, personName);
	g_Au.SendAtomChat(msg);
}

void AuAtomInterface::GovSetContrailColor(quint8 hue, quint8 sat)
{
	if(sat < 64 || sat > 191)
		return;

	bool hasAuthority = false;
	quint32 empireId = 0;
	DaPerson* person = LockAvatar();
	if(person)
	{
		// Insure the person is the president or vice president of their empire.
		empireId = person->EmpireId();
		const DgData& gd = g_Au.LockDataForRead();
		hasAuthority = gd.EmpireCitizenCanSetContrailColor(empireId, person->Id());
		g_Au.UnlockData();
	}
	UnlockAvatar();
	if(hasAuthority)
		emit GovSetContrailColor(empireId, hue, sat);
}

void AuAtomInterface::GovSetDefaultPoliticalStance(quint8 stance)
{
	if((POLITICAL_STANCE)stance > POL_Vassal)
		return;

	bool hasAuthority = false;
	quint32 empireId = 0;
	DaPerson* person = LockAvatar();
	if(person)
	{
		// Insure the person is the president or vice president of their empire.
		empireId = person->EmpireId();
		const DgData& gd = g_Au.LockDataForRead();
		hasAuthority = gd.EmpireCitizenCanSetPoliticalStance(empireId, person->Id());
		g_Au.UnlockData();
	}
	UnlockAvatar();
	if(hasAuthority)
		emit GovSetDefaultPoliticalStance(empireId, stance);
}

void AuAtomInterface::GovSetEmpireFlag(const QImage& imflag)
{
	bool hasAuthority = false;
	quint32 empireId = 0;
	DaPerson* person = LockAvatar();
	if(person)
	{
		empireId = person->EmpireId();
		const DgData& gd = g_Au.LockDataForRead();
		hasAuthority = gd.EmpireCitizenCanSetEmpireFlag(empireId, person->Id());
		g_Au.UnlockData();
	}
	UnlockAvatar();
	if(hasAuthority)
		emit EmpireChgFlag(empireId, imflag);
}

void AuAtomInterface::GovSetTax(quint8 income, quint8 sales, quint8 subsidy)
{
	if(income > 200)
		return;
	if(sales > 200)
		return;
	if(subsidy > 200)
		return;

	bool hasAuthority = false;
	quint32 empireId = 0;
	DaPerson* person = LockAvatar();
	if(person)
	{
		// Insure the person is the president or vice president of their empire.
		empireId = person->EmpireId();
		const DgData& gd = g_Au.LockDataForRead();
		hasAuthority = gd.EmpireCitizenCanSetTax(empireId, person->Id());
		g_Au.UnlockData();
	}
	UnlockAvatar();
	if(hasAuthority)
		emit GovSetTax(empireId, income, sales, subsidy);
}

void AuAtomInterface::GovSetCommerce(QByteArray commerce)
{

	bool hasAuthority = false;
	quint32 empireId = 0;
	DaPerson* person = LockAvatar();
	if(person)
	{
		// Insure the person is the president or vice president of their empire.
		empireId = person->EmpireId();
		const DgData& gd = g_Au.LockDataForRead();
		hasAuthority = gd.EmpireCitizenCanSetCommerce(empireId, person->Id());
		g_Au.UnlockData();
	}
	UnlockAvatar();
	if(hasAuthority)
		emit GovSetCommerce(empireId, commerce);
}

void AuAtomInterface::StorySendCurrentBlock(quint32 storyInstance)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		const AuScripts* scripts = g_Au.LockScripts();
		AuStory* story = g_Au.LockStory(storyInstance);
		person->StoryResendCurrentBlock(storyInstance, story, *scripts);
		g_Au.UnlockStory(storyInstance);
		g_Au.UnlockScripts();
	}
	UnlockAvatar();
}

void AuAtomInterface::StorySendHint(quint32 storyInstance)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		const AuScripts* scripts = g_Au.LockScripts();
		AuStory* story = g_Au.LockStory(storyInstance);
		person->StorySendHint(storyInstance, story, *scripts);
		g_Au.UnlockStory(storyInstance);
		g_Au.UnlockScripts();
	}
	UnlockAvatar();
}

void AuAtomInterface::Christen(const QString& name)
{
	if(!IsNameValid(name))
		return;

	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship)
		{
			// Person is aboard a spacecraft. Check for authority to change name.
			if(ship->CaptainId() == person->Id() || (!ship->CaptainsBerthOccupied() && ship->InChainOfCommand(person)))
			{
				ship->SetName(name);
				ship->SetModified();
			}
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::ReconcileRoster()
{
	AuIdList idsToKick;
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship && ship->IsDesignDataCacheValid())
		{
			// Person is aboard a spacecraft.
			if(ship->InChainOfCommand(person))
			{
				// Any officer can kick a non-officer.
				// The person has authority to reconcile non-officers in the roster of the ship.
				const AuRoster& roster = ship->LockRoster();
				for(AuRoster::const_iterator ri = roster.cbegin(); ri != roster.cend(); ri++)
					if(!(*ri).IsHumanOfficer() && !ship->FindIdRecursive((*ri).m_OccId))
						idsToKick.AddIfUnique((*ri).m_OccId);	// Kick the non-officer from the roster.
				ship->UnlockRoster();
			}
		}
	}
	UnlockAvatar();
	for(AuIdList::const_iterator ili = idsToKick.cbegin(); ili != idsToKick.cend(); ili++)
		emit ReleaseBerth(*ili, AuId());
}

void AuAtomInterface::KickBerthOccupant(const AuId& kickId)
{
	bool hasAuthority = false;
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship)
		{
			// Person is aboard a spacecraft.
			if(person->HasProgrammerPower())
				hasAuthority = true;						// Grant authority to programmer to kick anybody from any roster.
			else if(ship->IsOfficer(kickId))
			{
				// Kicking an officer, possibly the captain.
				// Only the captain can kick officers, or themself.
				// Architect can kick anybody, including captain.
				if(ship->CaptainId() == person->Id())
					hasAuthority = true;					// Grant authority to captain to kick any officers.
				else if(person->HasAgentPower())
				{
					// Agents can kick captains of subsidized ships and officers from any ships.
					if(kickId == ship->CaptainId())
						hasAuthority = !ship->IsPrivatelyOwned();	// Grant authority to kick captains of subsidized ships.
					else
						hasAuthority = true;				// Grant authority to kick officers of any ships.
				}
			}
			else
			{
				// Kicking a non-officer.
				// Any officer can kick a non-officer.
				// Agents can kick any non-officer from any roster.
				if(ship->InChainOfCommand(person) || person->HasAgentPower())
					hasAuthority = true;
			}
		}
	}
	UnlockAvatar();
	if(hasAuthority)
		emit ReleaseBerth(kickId, AuId());
}

void AuAtomInterface::SetAcceptCrew(bool accept)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship)
		{
			// Person is aboard a spacecraft.
			// Any officer can change this policy.
			if(ship->InChainOfCommand(person))
				ship->AcceptCrew(accept);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::SetAcceptPassengers(bool accept)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship)
		{
			// Person is aboard a spacecraft.
			// Any officer can change this policy.
			if(ship->InChainOfCommand(person))
				ship->AcceptPassengers(accept);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::SetAcceptTroops(bool accept)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship)
		{
			// Person is aboard a spacecraft.
			// Any officer can change this policy.
			if(ship->InChainOfCommand(person))
				ship->AcceptTroops(accept);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::SetAllowPassengerSeaChests(bool allow)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship)
		{
			// Person is aboard a spacecraft.
			// Any officer can change this policy.
			if(ship->InChainOfCommand(person))
				ship->AllowPassengerSeaChests(allow);
		}
	}
	UnlockAvatar();
}

quint8 AuAtomInterface::DesignStudioEnter(quint16 panelIndex, const QString& password)
{
	quint8 designStudioMode = DaDesignBase::InvalidMode;	// Return value
	DaPerson* person = LockAvatar();
	if(person)
	{
		if(person->InIndustry() == IID_DesignStudio)
		{
			// Scene atom is in a design studio.
			DaAtom* studioParent = person->Parent();
			if(dynamic_cast<DaCreature*>(studioParent))
			{
				QString resultMsg = tr("I must dismount before entering a design studio.");
				AuCommThought msg(resultMsg, person->Id(), person->Name());
				g_Au.SendAtomChat(msg);
			}
			else if(dynamic_cast<DaVehicle*>(studioParent))
			{
				QString resultMsg = tr("I must exit the vehicle before entering a design studio.");
				AuCommThought msg(resultMsg, person->Id(), person->Name());
				g_Au.SendAtomChat(msg);
			}
			else if(dynamic_cast<DaVessel*>(studioParent))
			{
				QString resultMsg = tr("I must exit the spacecraft before entering a design studio.");
				AuCommThought msg(resultMsg, person->Id(), person->Name());
				g_Au.SendAtomChat(msg);
			}
			else
			{
				// The person's parent atom is appropriate to host a design environment. They are not aboard a vehicle or mount.
				DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(studioParent);
				if(llworld)
				{
					// Enter DaDesignBase design studio instance.
					// Determine if the design studio at panelIndex is occupied.
					// Check for an existing studio instance at the panelIndex.
					DaDesignBase* pdbase = llworld->DesignEnvironment(person->InIndustryLL(), panelIndex);
					if(!pdbase)
					{
						// This is the first person to enter the design environment.
						// Establish the password for the entire studio.
						person->SetDesignPassword(password);

						// Enter nu style designer. No mode is chosen yet. User picks mode, e.g. File|New|Spacecraft
						// Create a nu design studio atom subordinate to the parent terrain.
						DaDesigner* pde = new DaDesigner(panelIndex);
						pde->ConnectSignals();
						pde->Create();

						// Set the transform to the scene atom's current position.
						// This is where they will exit the studio.
						pde->SetPosAndOrient(AuAtomPos(studioParent->Id(), person->Transform()));
						person->Ether()->AddAtom(pde);
						g_Au.AtomAdded(pde->Id());

						// The person needs to become subordinate to the design environment.
						AuMatrix4x4 t;
						t.Translate(0, 0, person->GroundHeightAboveCG());
						person->ForcePosStop(AuAtomPos(pde->Id(), t));
						person->SetModified();

						// Start with a nu design to establish a mode and, more importantly, the owner.
						pde->NewDesign(person->Id(), person->Name(), AuScDesign());
						pde->SetNormal();	// New atom is ready for normal thread processing.
						pde->TickTockRecursive();
						designStudioMode = DaDesignBase::DESIGN_Unknown;
					}
					else if(pdbase->IsNormal())
					{
						// The design environment exists and is not slated to die.
						// Join the architect inside.

						// Check password. A blank password prohibits anyone from entering.
						QString entryPass = pdbase->EntryPassword();
						if(password == entryPass)
						{
							// The password matches.
							person->SetDesignPassword(password);

							// The person becomes subordinate to the design environment.
							AuMatrix4x4 t;
							t.Translate(0, 0, person->GroundHeightAboveCG());
							person->ForcePosStop(AuAtomPos(pdbase->Id(), t));
							person->SetModified();

							// Insure there is a design plane present for the nu designer.
							pdbase->InitDesignPlane(person->Id());
							designStudioMode = pdbase->DesignMode();
						}
					}
				}
			}
		}
		else if(person && AcctIsProgrammer(person->AccountTypeOnLine()))
		{
			// Scene atom is a programmer.
			// Programmer can enter a design studio instance anywhere. This is needed when the game is initially started, to get
			// some designs loaded and published.
			DaAtom* studioParent = person->Parent();
			if(dynamic_cast<DaCreature*>(studioParent))
			{
				QString resultMsg = tr("I must dismount before entering a design studio.");
				AuCommThought msg(resultMsg, person->Id(), person->Name());
				g_Au.SendAtomChat(msg);
			}
			else if(dynamic_cast<DaVehicle*>(studioParent))
			{
				QString resultMsg = tr("I must exit the vehicle before entering a design studio.");
				AuCommThought msg(resultMsg, person->Id(), person->Name());
				g_Au.SendAtomChat(msg);
			}
			else if(dynamic_cast<DaVessel*>(studioParent))
			{
				QString resultMsg = tr("I must exit the spacecraft before entering a design studio.");
				AuCommThought msg(resultMsg, person->Id(), person->Name());
				g_Au.SendAtomChat(msg);
			}
			else
			{
				// The person's parent atom is appropriate to host a design environment. They are not aboard a vehicle or mount.
				DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(studioParent);
				if(llworld)
				{
					// This is the first person to enter the design environment.
					// Establish the password for the entire studio.
					person->SetDesignPassword(password);

					// Enter nu style designer. No mode is chosen yet. User picks mode, e.g. File|New|Spacecraft
					// Create a nu design studio atom subordinate to the parent terrain.
					DaDesigner* pde = new DaDesigner(panelIndex);
					pde->ConnectSignals();
					pde->Create();

					// Set the transform to the scene atom's current position.
					// This is where they will exit the studio.
					pde->SetPosAndOrient(AuAtomPos(studioParent->Id(), person->Transform()));
					person->Ether()->AddAtom(pde);
					g_Au.AtomAdded(pde->Id());

					// The person needs to become subordinate to the design environment.
					AuMatrix4x4 t;
					t.Translate(0, 0, person->GroundHeightAboveCG());
					person->ForcePosStop(AuAtomPos(pde->Id(), t));
					person->SetModified();

					// Start with a nu design to establish a mode and, more importantly, the owner.
					pde->NewDesign(person->Id(), person->Name(), AuScDesign());
					pde->SetNormal();	// New atom is ready for normal thread processing.
					pde->TickTockRecursive();
					designStudioMode = DaDesignBase::DESIGN_Unknown;
				}
				else
				{
					QString resultMsg = tr("I must move to a world before entering a design studio.");
					AuCommThought msg(resultMsg, person->Id(), person->Name());
					g_Au.SendAtomChat(msg);
				}
			}
		}
	}
	UnlockAvatar();
	return designStudioMode;
}

void AuAtomInterface::SetUnitAmmo(const AuId& unitId, COMMODITYID ammoCID)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		const DaSolarSystem* system = person->FindParentSolarSystemInEnv();
		if(system)
		{
			DaCitizen* troop = dynamic_cast<DaCitizen*>(system->FindIdRecursive(unitId));
			if(troop)
				troop->SetAmmo(ammoCID);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::SetUnitAmmoCount(const AuId& unitId, quint8 count)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		const DaSolarSystem* system = person->FindParentSolarSystemInEnv();
		if(system)
		{
			DaCitizen* troop = dynamic_cast<DaCitizen*>(system->FindIdRecursive(unitId));
			if(troop)
				troop->SetAmmoCount(count);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::SetUnitArmor(const AuId& unitId, const AuItem& item)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		const DaSolarSystem* system = person->FindParentSolarSystemInEnv();
		if(system)
		{
			DaCitizen* troop = dynamic_cast<DaCitizen*>(system->FindIdRecursive(unitId));
			if(troop)
				troop->SetArmor(item);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::SetUnitClothing(const AuId& unitId, const AuItem& item)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		const DaSolarSystem* system = person->FindParentSolarSystemInEnv();
		if(system)
		{
			DaCitizen* troop = dynamic_cast<DaCitizen*>(system->FindIdRecursive(unitId));
			if(troop)
				troop->SetClothing(item);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::SetUnitHelmet(const AuId& unitId, const AuItem& item)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		const DaSolarSystem* system = person->FindParentSolarSystemInEnv();
		if(system)
		{
			DaCitizen* troop = dynamic_cast<DaCitizen*>(system->FindIdRecursive(unitId));
			if(troop)
				troop->SetHelmet(item);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::SetUnitWeapon(const AuId& unitId, const AuItem& item)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		const DaSolarSystem* system = person->FindParentSolarSystemInEnv();
		if(system)
		{
			DaCitizen* troop = dynamic_cast<DaCitizen*>(system->FindIdRecursive(unitId));
			if(troop)
				troop->SetWeapon(item);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::MissionClear()
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship && ship->InChainOfCommand(person))
		{
			// The person is an officer of the spacecraft they are aboard or in the c-o-c.
			ship->MissionClear();		// Clear the previous mission, without notifying the pirate captain, if any.
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::MissionOrderComplete()
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship && ship->InChainOfCommand(person))
		{
			// The person is an officer of the spacecraft they are aboard or in the c-o-c.
			ship->MissionOrderComplete(0,0);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::MissionAppendOrder(const AuMissionOrder& order)
{
	if(!order.IsValid())
		return;
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship && ship->InChainOfCommand(person))
		{
			// The person is an officer of the spacecraft they are aboard.
			ship->MissionAppendOrder(order);

			// Player Statistics
			person->Stats().OrderMission();
			person->IncreaseSkill(AuSkill::Command, 1);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::MissionDeleteOrder(quint8 orderId)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship && ship->InChainOfCommand(person))
		{
			// The person is an officer of the spacecraft they are aboard.
			ship->MissionDeleteOrder(orderId);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::MissionDeleteCurrentOrder()
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship && ship->InChainOfCommand(person))
		{
			// The person is an officer of the spacecraft they are aboard.
			ship->MissionDeleteOrder(ship->MissionOrderMissionOrderId());
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::MissionInsertOrder(quint8 beforeId, const AuMissionOrder& order)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship && ship->InChainOfCommand(person))
		{
			// The person is an officer of the spacecraft they are aboard.
			ship->MissionInsertOrder(beforeId, order);

			// Player Statistics
			person->Stats().OrderMission();
			person->IncreaseSkill(AuSkill::Command, 1);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::MissionReplaceOrder(quint8 orderId, const AuMissionOrder& order)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship && ship->InChainOfCommand(person))
		{
			// The person is an officer of the spacecraft they are aboard.
			ship->MissionReplaceOrder(orderId, order);

			// Player Statistics
			person->Stats().OrderMission();
			person->IncreaseSkill(AuSkill::Command, 1);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::MissionSetActive(bool isActive)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship && ship->InChainOfCommand(person))
		{
			// The person is an officer of the spacecraft they are aboard.
			ship->MissionSetActive(isActive);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::MissionSetType(quint8 missionType)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship && ship->InChainOfCommand(person))
		{
			// The person is an officer of the spacecraft they are aboard.
			if(missionType < AuMission::TypeCount)
				ship->MissionSetType((AuMission::TYPE)missionType);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::MissionBurnCD(const AuGearPos& mediaPos, const QString& missionName)
{
	if(!IsNameValid(missionName))
		return;

	QString resultMsg;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		const DaVessel* ship = person->FindParentVesselInEnv();
		if(ship && (ship->IsCrewOrTroop(person->Id()) || ship->InChainOfCommand(person)))
		{
			// Find the media they specified for the save.
			AuGear* pinv = person->LockGearForWrite();
			AuItem* mediaItem = pinv->ItemAt(mediaPos);
			if(mediaItem)
			{
				// The media item was found.
				if((mediaItem->CommodityId() == CID_BlankDisk || mediaItem->CommodityId() == CID_Paper))
				{
					AuItem* media = mediaItem->Clone();
					person->UnlockGear();

					media->SetCount(1);
					person->ConsumeItem(*media);

					if(media->UseItem())					// Quality check the result.
					{
						// Replace the blank disk with a mission disk.
						const AuMission& m = ship->LockMission();
						AuItemMission* missionDisk = new AuItemMission(missionName, m, CID_MissionDisk, media->Quality(),
								media->EmpireOfOrigin());
						ship->UnlockMission();

						// Put it wherever there's room.
						person->AddGearItem(*missionDisk);

						resultMsg = tr("%1 mission stored to %2.")
							.arg(missionName)
							.arg(media->SayName());

						delete missionDisk;
					}
					else
					{
						// Media failure.
						resultMsg = tr("The %1 was defective; it failed during the save. "
								"The mission was not saved. "
								"I should try again with another blank disk or paper.").arg(media->SayName());
					}
					delete media;
				}
				else
				{
					AuItemStorageMedia* storageMedia = dynamic_cast<AuItemStorageMedia*>(mediaItem);
					if(storageMedia)
					{
						// Store the mission onto the media.
						AuMission m = ship->LockMission();
						ship->UnlockMission();
						if(storageMedia->StoreMission(missionName, m))
						{
							resultMsg = tr("%1 mission stored to %2.")
								.arg(missionName)
								.arg(storageMedia->SayName());
							person->UpdateGearWeight();
							person->UnlockGear();
						}
						else
						{
							person->UnlockGear();
							resultMsg = tr("The storage media has no free space.");
						}
					}
					else
					{
						person->UnlockGear();
						resultMsg = tr("I need a blank disk or paper or other storage media to store a mission.");
					}
				}
			}
			else
				person->UnlockGear();
		}
	}
	UnlockAvatar();
	if(!resultMsg.isEmpty())
	{
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::MissionNew(const AuMission& mission)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship && ship->InChainOfCommand(person))
		{
			// The person is an officer of the spacecraft they are aboard.
			ship->MissionNew(mission);
			ship->MissionSetActive(false);  // Human captains accept nu missions in inactive state.

			// Player Statistics
			person->Stats().OrderMission();
			person->IncreaseSkill(AuSkill::Command, 1);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::MissionAppend(const AuMission& mission)
{
	if(mission.IsEmpty())
		return;
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship && ship->InChainOfCommand(person))
		{
			// The person is an officer of the spacecraft they are aboard.
			ship->MissionAppend(mission);

			// Player Statistics
			person->Stats().OrderMission();
			person->IncreaseSkill(AuSkill::Command, 1);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::ToggleShipOwnership()
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship)
		{
			QString resultMsg;
			if(ship->IsPrivatelyOwned())
			{
				ship->SetPrivateOwnership(false);
				resultMsg = QObject::tr("%1 is now fleet.").arg(ship->NameWithEmpire());
			}
			else
			{
				// Figure out who the owner will be.
				AuId captId = ship->CaptainId();
				QString captName = ship->CaptainName();
				if(captId.IsNull())
				{
					captId = person->Id();
					captName = person->Name();
				}

				ship->SetPrivateOwnership(true);
				ship->SetPrivateOwner(captId, captName);
				resultMsg = QObject::tr("%1 is now company owned by %2.").arg(ship->NameWithEmpire()).arg(captName);
			}
			ship->SetModified();

			AuCommThought msg(resultMsg, person->Id(), person->Name());
			g_Au.SendAtomChat(msg);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::GovSetPoliticalStance(quint32 towardEmpire, quint8 stance, quint8 vassalTributePercent)
{
	QString unitName;
	quint8 toStance = 0;
	AuId toUnit;
	QString toUnitName;
	quint8 reason = AuDocSetStance::DocumentType();
	quint8 galaxy = 0;
	QString sectorName;
	QString systemName;
	QString worldName;

	bool hasAuthority = false;
	quint32 empireId = 0;
	AuId personId;
	DaPerson* person = LockAvatar();
	if(person)
	{
		// Insure the person is the president or vice president of their empire.
		personId = person->Id();
		empireId = person->EmpireId();
		const DgData& gd = g_Au.LockDataForRead();
		hasAuthority = gd.EmpireCitizenCanSetPoliticalStance(empireId, personId);
		toStance = gd.EmpirePoliticalStance(towardEmpire, empireId);
		g_Au.UnlockData();

		unitName = person->TitleName();
		galaxy = person->Galaxy();
		const DaSolarSystem* system = person->FindParentSolarSystemInEnv();
		if(system)
			systemName = system->Name();
		const DaSector* sector = person->FindParentSector();
		if(sector)
			sectorName = sector->Name();
		const DaWorld* world = person->FindParentPlanetInEnv();
		if(world)
			worldName = world->Name();
	}
	UnlockAvatar();

	if(hasAuthority)
		emit GovSetPoliticalStance(towardEmpire, empireId, stance, vassalTributePercent, personId, unitName,
			toStance, toUnit, toUnitName, reason, galaxy, sectorName, systemName, worldName);
}

void AuAtomInterface::GovLeaveEmpire(const AuId& personId, const QString& personName, quint32 idOldEmpire)
{
	// This player is departing their empire
	emit LeaveEmpire(personId, personName, idOldEmpire);
}

void AuAtomInterface::GovKick(const AuId& personId, const QString& personName, quint32 idOldEmpire)
{
	// This player is attempting to kick a different player from their empire.
	// Check for proper authority to assign an office.
	bool hasAuthority = false;
	AuId sectorId;
	GALAXY galaxy = GALAXY_Count;
	AuId kickerId;
	QString kickerName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		kickerId = person->Id();
		kickerName = person->Name();
		// Get sector id from parent sector.
		const DaSector* sector = person->FindParentSector();
		if(sector)
		{
			sectorId = sector->Id();
			galaxy = sector->Galaxy();
		}

		const DgData& gd = g_Au.LockDataForRead();
		hasAuthority = gd.EmpireCitizenCanKickCitizens(idOldEmpire, person->Id());
		g_Au.UnlockData();
	}
	UnlockAvatar();
	if(!hasAuthority)
	{
		// This is a kick. Report the result.
		QString resultMsg = tr("%1 cannot be kicked from the empire.").arg(personName);
		AuCommThought msg(resultMsg, kickerId, kickerName);
		g_Au.SendAtomChat(msg);
		return;
	}

	emit LeaveEmpire(personId, personName, idOldEmpire);

	// Report the result.
	// This message is broadcast on the Empire channel.
	QString resultMsg = tr("%1 was kicked from the empire.").arg(personName);
	AuCommChat empAnnounce(resultMsg, AuCommStdChannel::EmpireChannelName(), kickerId, kickerName, sectorId, galaxy,
			AuCommStdChannel::Universe, AuCommStdChannel::Empire);
	empAnnounce.SetScopeData(idOldEmpire);
	g_Au.SendAtomChat(empAnnounce);
}

void AuAtomInterface::GovSetBoss(const AuId& governorId, const AuId& bossId)
{

	// Get info from scene atom.
	quint32 empireId = 0;
	AuId sectorId;
	GALAXY galaxy = GALAXY_Count;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		empireId = person->EmpireId();

		// Get sector id from parent sector.
		const DaSector* sector = person->FindParentSector();
		if(sector)
		{
			sectorId = sector->Id();
			galaxy = sector->Galaxy();
		}
	}
	UnlockAvatar();

	// Check for proper authority to assign an office.
	const DgData& gd = g_Au.LockDataForRead();
	bool canDelegate = gd.EmpireCitizenCanDelegate(empireId, personId);
	bool hasOffice = gd.IsEmpireGovernor(empireId, governorId);
	QString govName = gd.EmpireCitizenNameWithTitle(empireId, governorId);
	g_Au.UnlockData();
	if(!canDelegate || !hasOffice)
		return;

	emit GovBossChanged(empireId, governorId, bossId);

	// This message is broadcast on the Empire channel.
	QString resultMsg = tr("Government boss changed for %1.").arg(govName);
	AuCommChat empAnnounce(resultMsg, AuCommStdChannel::EmpireChannelName(), personId, personName, sectorId, galaxy,
			AuCommStdChannel::Universe, AuCommStdChannel::Empire);
	empAnnounce.SetScopeData(empireId);
	g_Au.SendAtomChat(empAnnounce);
}

void AuAtomInterface::GovAssignOffice(const AuId& toWho, const QString& toName, const AuOffice& office)
{
	// Get info from scene atom.
	quint32 empireId = 0;
	AuId sectorId;
	GALAXY galaxy = GALAXY_Count;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		empireId = person->EmpireId();
		// Get sector id from parent sector.
		const DaSector* sector = person->FindParentSector();
		if(sector)
		{
			sectorId = sector->Id();
			galaxy = sector->Galaxy();
		}
	}
	UnlockAvatar();

	// Check for proper authority to assign an office.
	// Filter 'office' against the permissions available to the avatar assigning them.
	// It will return false if personId does not have permission to delegate.
	AuOffice filtOffice(office);		// The filtered result.
	const DgData& gd = g_Au.LockDataForRead();
	bool hasOffice = gd.IsEmpireGovernor(empireId, toWho);	// True if the assignee already holds an office.
	bool filterOk = gd.EmpireCitizenFilterAssignment(empireId, personId, toWho, filtOffice);	// True if ok to assign office.
	if(hasOffice && !gd.EmpireCitizenInChainOfCommand(empireId, personId, toWho))
		filterOk = false;	// Cannot change office of someone who is not in your chain of command.
	g_Au.UnlockData();

	if(filterOk)
	{
		// Assign/update the office settings in the empire record.
		emit OfficeAssigned(toWho, empireId, filtOffice);

		// This message is broadcast on the Empire channel.
		QString resultMsg;
		if(hasOffice)
			resultMsg = tr("Government office changed for %1.").arg(toName);
		else
			resultMsg = tr("Government office assigned to %1.").arg(toName);
		AuCommChat empAnnounce(resultMsg, AuCommStdChannel::EmpireChannelName(), personId, personName, sectorId, galaxy,
				AuCommStdChannel::Universe, AuCommStdChannel::Empire);
		empAnnounce.SetScopeData(empireId);
		g_Au.SendAtomChat(empAnnounce);
	}
	else
	{
		// The office cannot be assigned.
		QString resultMsg = tr("That government office cannot be assigned to %1.").arg(toName);
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::GovResignOffice()
{
	quint32 empireId = 0;
	QString officeTitle;

	AuId sectorId;
	GALAXY galaxy = GALAXY_Count;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		empireId = person->EmpireId();
		const DgData& gd = g_Au.LockDataForRead();
		officeTitle = gd.EmpireCitizenTitle(empireId, personId);
		g_Au.UnlockData();

		// Get sector id from parent sector.
		const DaSector* sector = person->FindParentSector();
		if(sector)
		{
			sectorId = sector->Id();
			galaxy = sector->Galaxy();
		}
	}
	UnlockAvatar();

	if(empireId)
	{
		// Remove the office assignment from the person.
		QString resultMsg = tr("I resigned the office of %1, effective immediately.").arg(officeTitle);

		emit OfficeRevoked(personId, empireId);

		// This message is broadcast on the Empire channel.
		AuCommChat empAnnounce(resultMsg, AuCommStdChannel::EmpireChannelName(), personId, personName, sectorId, galaxy,
				AuCommStdChannel::Universe, AuCommStdChannel::Empire);
		empAnnounce.SetScopeData(empireId);
		g_Au.SendAtomChat(empAnnounce);
	}
}

void AuAtomInterface::GovRevokeOffice(const AuId& fromWho, const QString& fromName)
{
	// Get info from scene atom.
	quint32 empireId = 0;
	AuId sectorId;
	GALAXY galaxy = GALAXY_Count;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		empireId = person->EmpireId();

		// Get sector id from parent sector.
		const DaSector* sector = person->FindParentSector();
		if(sector)
		{
			sectorId = sector->Id();
			galaxy = sector->Galaxy();
		}
	}
	UnlockAvatar();

	// Check for proper authority to revoke an office.
	const DgData& gd = g_Au.LockDataForRead();
	bool hasAuthority = gd.EmpireCitizenInChainOfCommand(empireId, personId, fromWho);
	bool hasOffice = gd.IsEmpireGovernor(empireId, fromWho);
	g_Au.UnlockData();

	if(hasAuthority && hasOffice)
	{
		emit OfficeRevoked(fromWho, empireId);

		// This message is broadcast on the Empire channel.
		QString resultMsg = tr("Government office was revoked from %1.").arg(fromName);
		AuCommChat empAnnounce(resultMsg, AuCommStdChannel::EmpireChannelName(), personId, personName, sectorId, galaxy,
				AuCommStdChannel::Universe, AuCommStdChannel::Empire);
		empAnnounce.SetScopeData(empireId);
		g_Au.SendAtomChat(empAnnounce);
	}
	else
	{
		// The office cannot be assigned.
		QString resultMsg = tr("That government office cannot be revoked from %1.").arg(fromName);
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::GameQuit()
{
	DaPerson* person = LockAvatar();
	if(person && person->IsGameTablePose() && person->IsInCapsule())
	{
		// Ask the capsule to remove the person from the game.
		DaCapsule* cap = person->FindParentCapsuleInEnv();
		if(cap)
			cap->GameExitPlayer(person->StationId(), person->Id());
	}
	UnlockAvatar();
}

bool AuAtomInterface::GameSelected(quint8 gameType)
{
	bool gameSelected = false;	// True if the ship accepts the nu game selection.
	DaPerson* person = LockAvatar();
	if(person)
	{
		// Ask the capsule to select a nu game. It might not do it if a game is in progress.
		DaCapsule* cap = person->FindParentCapsuleInEnv();
		if(cap)
			gameSelected = cap->GameSelected(person->StationId(), gameType, person->Id());
	}
	UnlockAvatar();
	return gameSelected;
}

void AuAtomInterface::GameStart(quint8 gameType, const QByteArray& gameSettings)
{
	DaPerson* person = LockAvatar();
	if(person && person->IsGameTablePose() && person->IsInCapsule())
	{
		// Ask the capsule to select a nu game. It will not do anything if a game is already active at the table.
		DaCapsule* cap = person->FindParentCapsuleInEnv();
		if(cap)
			cap->GameStart(person, gameType, gameSettings);
	}
	UnlockAvatar();
}

void AuAtomInterface::GameTurn(quint8 gameType, const QByteArray& gameTurn)
{
	DaPerson* person = LockAvatar();
	if(person && person->IsGameTablePose() && person->IsInCapsule())
	{
		// Submit the person's game turn move.
		DaCapsule* cap = person->FindParentCapsuleInEnv();
		if(cap)
			cap->GameTurn(person, gameType, gameTurn);
	}
	UnlockAvatar();
}

void AuAtomInterface::TransferSendCargo(const AuId& otherShipId, const AuItem& item)
{
	DaVessel* ship = 0;
	DaVessel* otherShip = 0;
	if(TransferSetup(otherShipId, &ship, &otherShip))
	{
		if(item.CommodityId() == CID_Hydrogen)
		{
			// Fuel is taken from the ship's fuel cells.
			// If the other ship can accept the fuel, attempt to remove it from my ship.
			if(otherShip->CanAddCargo(item) && ship->RemoveCargo(item, item.Count()))
				otherShip->AddCargo(item);	// Add the fuel to the other ship.
		}
		else
		{
			const AuItemHash& sinv = ship->LockCargoForRead();
			AuItemHash::const_iterator ci = sinv.Find(item);
			bool c = ci != sinv.cend();
			ship->UnlockCargo();
			if(c)
			{
				// The cargo was found in the ship's hold.
				// If the other ship can accept the cargo, attempt to remove it from my ship.
				if(otherShip->CanAddCargo(item) && ship->RemoveCargo(item, item.Count()))
					otherShip->AddCargo(item);	// Add the cargo to the other ship.
			}
		}
	}
}

void AuAtomInterface::TransferSendCargo(const AuId& otherShipId, const AuItemHash& cargo)
{
	DaVessel* ship = 0;
	DaVessel* otherShip = 0;
	if(TransferSetup(otherShipId, &ship, &otherShip))
	{
		for(AuItemHash::const_iterator iii = cargo.cbegin(); iii != cargo.cend(); iii++)
		{
			// Locate the item in my ship's hold to determine the count.
			// This cargo transfer moves all of each commodity, or none.
			if((*iii).second->CommodityId() == CID_Hydrogen)
			{
				// Fuel is taken from the ship's fuel cells.
				// If the other ship can accept the fuel, attempt to remove it from my ship.
				if(otherShip->CanAddCargo(*(*iii).second) && ship->RemoveCargo(*(*iii).second, (*iii).second->Count()))
					otherShip->AddCargo(*(*iii).second);	// Add the fuel to the other ship.
			}
			else
			{
				const AuItemHash& sinv = ship->LockCargoForRead();
				AuItemHash::const_iterator ci = sinv.Find(*(*iii).second);
				bool c = ci != sinv.cend();
				ship->UnlockCargo();
				if(c)
				{
					// The cargo was found in my ship's hold.
					// If the other ship can accept the cargo, attempt to remove it from my ship.
					if(otherShip->CanAddCargo(*(*iii).second) && ship->RemoveCargo(*(*iii).second, (*iii).second->Count()))
						otherShip->AddCargo(*(*iii).second);	// Add the cargo to the other ship.
				}
			}
		}
	}
}

void AuAtomInterface::TransferSendCitizens(const AuId& otherShipId, const AuIdList& citizenIds, const AuIdList& swapIds)
{
	DaVessel* ship = 0;
	DaVessel* otherShip = 0;
	if(TransferSetup(otherShipId, &ship, &otherShip))
		TransferCitizens(ship, otherShip, citizenIds, swapIds);
}

void AuAtomInterface::TransferSendMoney(const AuId& otherShipId, double amount)
{
	DaVessel* ship = 0;
	DaVessel* otherShip = 0;
	if(TransferSetup(otherShipId, &ship, &otherShip))
	{
		if(ship->AccountBalance() >= amount)
		{
			ship->DebitAccount(amount);
			otherShip->CreditAccount(amount);
		}
	}
}

void AuAtomInterface::TransferTakeCargo(const AuId& otherShipId, const AuItem& item)
{
	DaVessel* ship = 0;
	DaVessel* otherShip = 0;
	if(TransferSetup(otherShipId, &ship, &otherShip))
	{
		if(item.CommodityId() == CID_Hydrogen)
		{
			// Fuel is taken from the ship's fuel cells.
			// If my ship can accept the fuel, attempt to remove it from the other ship.
			if(ship->CanAddCargo(item) && otherShip->RemoveCargo(item, item.Count()))
				ship->AddCargo(item);	// Add the fuel to my ship.
		}
		else
		{
			const AuItemHash& sinv = otherShip->LockCargoForRead();
			AuItemHash::const_iterator ci = sinv.Find(item);
			bool c = ci != sinv.cend();
			otherShip->UnlockCargo();
			if(c)
			{
				// The cargo was found in the other ship's hold.
				// If my ship can accept the cargo, attempt to remove it from the other ship.
				if(ship->CanAddCargo(item) && otherShip->RemoveCargo(item, item.Count()))
					ship->AddCargo(item);	// Add the cargo to my ship.
			}
		}
	}
}

void AuAtomInterface::TransferTakeCargo(const AuId& otherShipId, const AuItemHash& cargo)
{
	DaVessel* ship = 0;
	DaVessel* otherShip = 0;
	if(TransferSetup(otherShipId, &ship, &otherShip))
	{
		for(AuItemHash::const_iterator iii = cargo.cbegin(); iii != cargo.cend(); iii++)
		{
			// Locate the item in the other ship's hold to determine the count.
			// This cargo transfer moves all of each commodity, or none.
			if((*iii).second->CommodityId() == CID_Hydrogen)
			{
				// Fuel is taken from the ship's fuel cells.
				// If my ship can accept the fuel, attempt to remove it from the other ship.
				if(ship->CanAddCargo(*(*iii).second) && otherShip->RemoveCargo(*(*iii).second, (*iii).second->Count()))
					ship->AddCargo(*(*iii).second);	// Add the fuel to my ship.
			}
			else
			{
				const AuItemHash& sinv = otherShip->LockCargoForRead();
				AuItemHash::const_iterator ci = sinv.Find(*(*iii).second);
				bool c = ci != sinv.cend();
				otherShip->UnlockCargo();
				if(c)
				{
					// The cargo was found in the other ship's hold.
					// If my ship can accept the cargo, attempt to remove it from the other ship.
					if(ship->CanAddCargo(*(*iii).second) && otherShip->RemoveCargo(*(*iii).second, (*iii).second->Count()))
						ship->AddCargo(*(*iii).second);	// Add the cargo to my ship.
				}
			}
		}
	}
}

void AuAtomInterface::TransferTakeCitizens(const AuId& otherShipId, const AuIdList& citizenIds, const AuIdList& swapIds)
{
	DaVessel* ship = 0;
	DaVessel* otherShip = 0;
	if(TransferSetup(otherShipId, &ship, &otherShip))
		TransferCitizens(otherShip, ship, citizenIds, swapIds);
}

void AuAtomInterface::TransferTakeMoney(const AuId& otherShipId, double amount)
{
	DaVessel* ship = 0;
	DaVessel* otherShip = 0;
	if(TransferSetup(otherShipId, &ship, &otherShip))
	{
		if(otherShip->AccountBalance() >= amount)
		{
			otherShip->DebitAccount(amount);
			ship->CreditAccount(amount);
		}
	}
}

bool AuAtomInterface::TransferSetup(const AuId& otherShipId, DaVessel** ship, DaVessel** otherShip)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		if(person->IsInSpacecraft())
		{
			// The person is inside a spacecraft.
			(*ship) = person->FindParentVesselInEnv();
			const DaSolarSystem* system = person->FindParentSolarSystemInEnv();
			if((*ship) && (*ship)->InChainOfCommand(person) && system)
			{
				// The person is in command of the spacecraft they are aboard.
				// Locate the other ship.
				*otherShip = dynamic_cast<DaVessel*>(system->FindIdRecursive(otherShipId));
				if(*otherShip && *otherShip != *ship)
				{
					// Located the other ship.
					// Verify the person has the authority to transfer cargo to/from the other ship.
					bool fuelEmergency = false;
					if(!(*otherShip)->IsMoored() && (*otherShip)->FuelQuantity() <= 0 &&
							((*otherShip)->IsCommandedByAI() || (*otherShip)->IsPilotMannedByAI()))
					{
						if((*otherShip)->IsSpaceStation())
							fuelEmergency = true;
						else if((*otherShip)->IsOnGround() && !(*otherShip)->IsMoving())
						{
							if((*otherShip)->BdId().IsNull())
								fuelEmergency = true;
						}
						else
							fuelEmergency = true;
					}

					bool transferAuthority = false;
					if(fuelEmergency)
						transferAuthority = true;				// Fuel emergency opens the doors.
					else if((*otherShip)->InChainOfCommand(person))
						transferAuthority = true;				// Ships in my chain of command can be looted.
					else if((*otherShip)->MissionOrderType() == AuMissionOrder::StandDown)
						transferAuthority = true;				// Ships that are standing down are eligible for looting.
					else if((*otherShip)->IsNew())
					{
						// The other ship is nu. I can only get into it if its empire is friendly to mine.
						POLITICAL_STANCE stance = PoliticalStance((*otherShip)->EmpireId(), (*ship)->EmpireId());
						if(stance == POL_Empire || stance == POL_Friend || stance == POL_Vassal)
							transferAuthority = true;			// Ship's empire is friendly toward mine; person can loot it.
					}
					else if((*otherShip)->IsSpaceStation() && !(*otherShip)->IsPrivatelyOwned())
						transferAuthority = false;				// No transfer with subsidized station if not in chain of command.
					else if(!(*otherShip)->HasOfficers())
						transferAuthority = true;				// Ship is not nu and it has no officers; person can loot it.

					if(transferAuthority)
					{
						// The person has the authority to transfer cargo to/from the other ship.
						// Verify the distance to the other ship permits cargo transfer.
						AuPoint shipLoc = (*ship)->EnvLocation();
						AuPoint otherShipLoc = (*otherShip)->EnvLocation();
						if(shipLoc.Distance(otherShipLoc) < (*ship)->TransferRange(*otherShip))
						{
							// The other ship is in range. All conditions are ok for the transfer.
							UnlockAvatar();
							return true;
						}
					}
				}
			}
			else
				(*ship) = 0;
		}
	}
	UnlockAvatar();
	return false;
}

void AuAtomInterface::TransferCitizens(DaVessel* fromShip, DaVessel* toShip, const AuIdList& citizenIds,
		const AuIdList& swapIds)
{
	for(AuIdList::const_iterator cii = citizenIds.cbegin(); cii != citizenIds.cend(); cii++)
	{
		DaCitizen* citizen = dynamic_cast<DaCitizen*>(fromShip->FindIdRecursive(*cii));
		if(!citizen)
			continue;

		// The citizen was found. What sort of citizen is it?
		if(citizen->IsNpcCrew())
		{
			// We are transferring a crewman.
			// Crewmen may not server aboard enemy ships.
			POLITICAL_STANCE stance = PoliticalStance(citizen->EmpireId(), toShip->EmpireId());
			if(stance != POL_Enemy)
			{
				if(toShip->CrewBerthsAvailable())
					TransferCitizen(citizen, fromShip, toShip, AuRosterEntry::Crew);
				else if(toShip->CrewBerths())
				{
					// Swap the citizen with someone on the other ship.
					DaCitizen* swap = 0;
					for(AuIdList::const_iterator sii = swapIds.cbegin(); sii != swapIds.cend(); sii++)
					{
						swap = dynamic_cast<DaCitizen*>(toShip->FindIdRecursive(*sii));
						if(swap)
							break;
					}
					if(!swap)
					{
						DaAtomList aboard;
						toShip->FindCrewRecursive(toShip->Id(), aboard);
						if(!aboard.isEmpty())
							swap = dynamic_cast<DaCitizen*>(aboard.first());
					}
					if(swap)
					{
						stance = PoliticalStance(swap->EmpireId(), fromShip->EmpireId());
						if(stance != POL_Enemy)
							TransferSwapCitizens(citizen, swap, fromShip, toShip, AuRosterEntry::Crew);
					}
				}
			}
		}
		else if(citizen->IsNpcOfficer())
		{
			// We are transferring an officer.
			// Officers may only serve on ships from their own empire.
			if(citizen->EmpireId() == toShip->EmpireId())
			{
				if(toShip->OfficerBerthsAvailable())
					TransferCitizen(citizen, fromShip, toShip, AuRosterEntry::OfficerAI);
				else if(toShip->OfficerBerths())
				{
					// Swap the citizen with someone on the other ship.
					DaCitizen* swap = 0;
					for(AuIdList::const_iterator sii = swapIds.cbegin(); sii != swapIds.cend(); sii++)
					{
						swap = dynamic_cast<DaCitizen*>(toShip->FindIdRecursive(*sii));
						if(swap)
							break;
					}
					if(!swap)
						swap = (DaCitizen*)toShip->FindOfficerRecursive();
					if(swap && swap->EmpireId() == fromShip->EmpireId())
						TransferSwapCitizens(citizen, swap, fromShip, toShip, AuRosterEntry::OfficerAI);
				}
			}
		}
		else if(citizen->IsNpcPassenger())
		{
			// We are transferring a passenger.
			// There are no political stance restrictions on passengers when transferring.
			if(toShip->PassengerBerthsAvailable())
				TransferCitizen(citizen, fromShip, toShip, AuRosterEntry::PassengerAI);
			else if(toShip->PassengerBerths())
			{
				// Swap the citizen with someone on the other ship.
				DaCitizen* swap = 0;
				for(AuIdList::const_iterator sii = swapIds.cbegin(); sii != swapIds.cend(); sii++)
				{
					swap = dynamic_cast<DaCitizen*>(toShip->FindIdRecursive(*sii));
					if(swap)
						break;
				}
				if(!swap)
				{
					DaAtomList aboard;
					toShip->FindPassengersRecursive(toShip->Id(), aboard);
					if(!aboard.isEmpty())
						swap = dynamic_cast<DaCitizen*>(aboard.first());
				}
				if(swap)
					TransferSwapCitizens(citizen, swap, fromShip, toShip, AuRosterEntry::PassengerAI);
			}
		}
		else if(citizen->IsNpcTroop())
		{
			// We are transferring a troop.
			// Troops may only serve on ships from their own empire.
			if(citizen->EmpireId() == toShip->EmpireId())
			{
				if(toShip->TroopBerthsAvailable())
					TransferCitizen(citizen, fromShip, toShip, AuRosterEntry::Troop);
				else if(toShip->TroopBerths())
				{
					// Swap the citizen with someone on the other ship.
					DaCitizen* swap = 0;
					for(AuIdList::const_iterator sii = swapIds.cbegin(); sii != swapIds.cend(); sii++)
					{
						swap = dynamic_cast<DaCitizen*>(toShip->FindIdRecursive(*sii));
						if(swap)
							break;
					}
					if(!swap)
					{
						DaAtomList aboard;
						toShip->FindTroopsRecursive(toShip->Id(), aboard);
						if(!aboard.isEmpty())
							swap = dynamic_cast<DaCitizen*>(aboard.first());
					}
					if(swap && swap->EmpireId() == fromShip->EmpireId())
						TransferSwapCitizens(citizen, swap, fromShip, toShip, AuRosterEntry::Troop);
				}
			}
		}
	}
}

// Transfer a citizen from one ship to another.
void AuAtomInterface::TransferCitizen(DaCitizen* citizen, DaVessel* fromShip, DaVessel* toShip, AuRosterEntry::TYPE berthType)
{
	// Change the citizen's berth assignment from the old ship to the nu ship.
	fromShip->RemoveBerthOccupant(citizen->Id());
	toShip->OccupyBerth(berthType, citizen->Id(), citizen->Name());
	citizen->AssignToShip(toShip->Id(), berthType);

	// Physically move the citizen to their berth on the nu ship.
	AuMatrix4x4 t;
	t.Translate(toShip->BerthLocation(citizen->Id()) + AuPoint(0,0,citizen->CGHeightStanding()));
	citizen->ChangePosAndOrient(AuAtomPos(toShip->Id(), t));
	citizen->InvalidateEnvironment();
	citizen->Stand();
	citizen->GoToWork();
	citizen->SetModified();
}

// Swap a citizen with someone on another ship.
void AuAtomInterface::TransferSwapCitizens(DaCitizen* citizen, DaCitizen* swap, DaVessel* fromShip, DaVessel* toShip,
		AuRosterEntry::TYPE berthType)
{
	// Swap the citizen's berth assignments from their old ships to their nu ships.
	fromShip->RemoveBerthOccupant(citizen->Id());
	toShip->RemoveBerthOccupant(swap->Id());
	toShip->OccupyBerth(berthType, citizen->Id(), citizen->Name());
	fromShip->OccupyBerth(berthType, swap->Id(), swap->Name());
	citizen->AssignToShip(toShip->Id(), berthType);
	swap->AssignToShip(fromShip->Id(), berthType);

	// Physically move the citizens to their berths on their nu ships.
	AuMatrix4x4 t;
	t.Translate(toShip->BerthLocation(citizen->Id()) + AuPoint(0,0,citizen->CGHeightStanding()));
	citizen->ChangePosAndOrient(AuAtomPos(toShip->Id(), t));
	citizen->InvalidateEnvironment();
	citizen->Stand();
	citizen->GoToWork();
	citizen->SetModified();

	AuMatrix4x4 tswap;
	tswap.Translate(fromShip->BerthLocation(swap->Id()) + AuPoint(0,0,swap->CGHeightStanding()));
	swap->ChangePosAndOrient(AuAtomPos(fromShip->Id(), tswap));
	swap->InvalidateEnvironment();
	swap->Stand();
	swap->GoToWork();
	swap->SetModified();
}

void AuAtomInterface::TransferSendItemsLoose(const AuId& otherShipId)
{
	DaVessel* ship = 0;
	DaVessel* otherShip = 0;
	if(TransferSetup(otherShipId, &ship, &otherShip))
	{
		DaAtomList items;
		ship->FindItems(items);
		for(DaAtomList::iterator ii = items.begin(); ii != items.end(); ii++)
		{
			(*ii)->ChangePos(AuAtomPos(otherShip->Id(), (*ii)->Transform()));
			(*ii)->InvalidateEnvironment();
			(*ii)->SetModified();
		}

		QString resultMsg = tr("%1 loose items were transferred to %2.")
			.arg(items.size())
			.arg(otherShip->Name());
		AuId personId;
		QString personName;
		Avatar(personId, personName);
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::TransferSendItemsStatic(const AuId& otherShipId)
{
	DaVessel* ship = 0;
	DaVessel* otherShip = 0;
	if(TransferSetup(otherShipId, &ship, &otherShip))
	{
		DaAtomList items;
		ship->FindItemsStatic(items);
		for(DaAtomList::iterator ii = items.begin(); ii != items.end(); ii++)
		{
			(*ii)->ChangePos(AuAtomPos(otherShip->Id(), (*ii)->Transform()));
			(*ii)->InvalidateEnvironment();
			(*ii)->SetModified();
		}

		QString resultMsg = tr("%1 static display items were transferred to %2.")
			.arg(items.size())
			.arg(otherShip->Name());
		AuId personId;
		QString personName;
		Avatar(personId, personName);
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::TransferTakeItemsLoose(const AuId& otherShipId)
{
	DaVessel* ship = 0;
	DaVessel* otherShip = 0;
	if(TransferSetup(otherShipId, &ship, &otherShip))
	{
		DaAtomList items;
		otherShip->FindItems(items);
		for(DaAtomList::iterator ii = items.begin(); ii != items.end(); ii++)
		{
			(*ii)->ChangePos(AuAtomPos(ship->Id(), (*ii)->Transform()));
			(*ii)->InvalidateEnvironment();
			(*ii)->SetModified();
		}

		QString resultMsg = tr("%1 loose items were transferred to %2.")
			.arg(items.size())
			.arg(ship->Name());
		AuId personId;
		QString personName;
		Avatar(personId, personName);
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::TransferTakeItemsStatic(const AuId& otherShipId)
{
	DaVessel* ship = 0;
	DaVessel* otherShip = 0;
	if(TransferSetup(otherShipId, &ship, &otherShip))
	{
		DaAtomList items;
		otherShip->FindItemsStatic(items);
		for(DaAtomList::iterator ii = items.begin(); ii != items.end(); ii++)
		{
			(*ii)->ChangePos(AuAtomPos(ship->Id(), (*ii)->Transform()));
			(*ii)->InvalidateEnvironment();
			(*ii)->SetModified();
		}

		QString resultMsg = tr("%1 static display items were transferred to %2.")
			.arg(items.size())
			.arg(ship->Name());
		AuId personId;
		QString personName;
		Avatar(personId, personName);
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

bool AuAtomInterface::Avatar(AuId& personId, QString& personName)
{
	bool avok = false;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		avok = true;
	}
	UnlockAvatar();
	return avok;
}

void AuAtomInterface::KickStowaways()
{
	QStringList stowaways;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		// Person must be a crew member of the ship or they must be in the chain-of-command of the ship.
		// NOTE: A person with agent powers is considered to be in the chain-of-command of all ships.
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship && (ship->IsCrewOrTroop(person->Id()) || ship->InChainOfCommand(person)))
			ship->KickStowaways(stowaways, !person->HasAgentPower());
	}
	UnlockAvatar();

	QString resultMsg;
	if(stowaways.isEmpty())
		resultMsg = tr("No stowaways aboard.");
	else if(stowaways.count() == 1)
		resultMsg = tr("Kicked stowaway %1.").arg(stowaways[0]);
	else
	{
		resultMsg = tr("Kicked stowaways ");
		for(QStringList::const_iterator si = stowaways.cbegin(); si != stowaways.cend(); si++)
		{
			if(si == stowaways.cbegin())
				resultMsg += *si;
			else if(si == stowaways.cend() - 1)
				resultMsg += " and " + *si;
			else
				resultMsg += ", " + *si;
		}
		resultMsg += ".";
	}

	// Send the result on the Log channel.
	AuCommThought msg(resultMsg, personId, personName);
	g_Au.SendAtomChat(msg);
}

void AuAtomInterface::PostBounty(const AuId& victimId, const QString& victimName, const QString& reason, double reward)
{
	// Test validity of values.
	if(victimId.IsNull() || victimId == AuId(1))
		return;
	if(victimName.isEmpty())
		return;
	if(reason.isEmpty() || reason.count() > 1000)
		return;
	if(reward < 10000)
		return;	// minimum bounty

	AuId posterId;
	QString posterName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		if(reward < person->AccountBalance())
		{
			// The person has the money for the reward.
			posterId = person->Id();
			posterName = person->Name();

			// Take the reward money from the person right away.
			person->DebitAccount(reward);

			// Send acknowledgement.
			QString response = tr("%L1%2 bounty posted on %3.").arg(reward, 0,'f',0).arg(g_Au.CurrencySymbol()).arg(victimName);
			AuCommThought msg(response, person->Id(), person->Name());
			g_Au.SendAtomChat(msg);
		}
	}
	UnlockAvatar();

	if(!posterId.IsNull())
		emit BountyPosted(posterId, posterName, victimId, reason, reward, 0);
}

void AuAtomInterface::GovtAcctDepositFunds(double amount, const AuId& opBd)
{
	QString resultMsg;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		AuIdList placeChain;
		placeChain.append(person->InBdCity());
		person->ChainOfPlaces(placeChain);
		const DgData& gd = g_Au.LockDataForRead();
		bool hasAuthority = gd.EmpireCitizenCanAccessGovtAccount(person->EmpireId(), person->Id(), placeChain);
		g_Au.UnlockData();

		if(person->InIndustry() != IID_Bank && person->InIndustry() != IID_Capitol)
			resultMsg = tr("I must stand at a bank to access the bank system.");
		else if(person->InCityEmpire() != person->EmpireId())
			resultMsg = tr("Only government officials of our empire may access the bank system.");
		else if(!hasAuthority)
			resultMsg = tr("Only government officials may access the bank system.");
		else if(person->AccountBalance() < amount)
			resultMsg = tr("Deposit declined. My personal account balance is only %L1%2.")
				.arg(person->AccountBalance(),0,'f',0)
				.arg(g_Au.CurrencySymbol());
		else
		{
			// Government official is in a bank of their empire.
			DaWorld* planet = person->FindParentPlanetInEnv();
			if(planet)
			{
				DaBd* bd = person->BdOp(opBd);
				if(bd)
				{
					if(!bd->InChainOfCommand(person))
						resultMsg = tr("Only government officials in the chain of command of this city may access the "
								"bank system.");
					else
					{
						bd->CreditAccountGovtDeposit(amount);
						person->DebitAccount(amount);
						double totalBank = bd->GovtAccountBalance();
						resultMsg = tr("%L1%2 deposited into bank system. Bank system balance is %L3%2.")
							.arg(amount,0,'f',0)
							.arg(g_Au.CurrencySymbol())
							.arg(totalBank,0,'f',0);
					}
				}
			}
		}
	}
	UnlockAvatar();

	if(!resultMsg.isEmpty())
	{
		// Send the result message to the person as a log message.
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::GovtAcctRequestBalance(const AuId& opBd)
{
	QString resultMsg;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		AuIdList placeChain;
		placeChain.append(person->InBdCity());
		person->ChainOfPlaces(placeChain);
		const DgData& gd = g_Au.LockDataForRead();
		bool hasAuthority = gd.EmpireCitizenCanAccessGovtAccount(person->EmpireId(), personId, placeChain);
		g_Au.UnlockData();

		if(person->InIndustry() != IID_Bank && person->InIndustry() != IID_Capitol)
			resultMsg = tr("I must stand at a bank to access the bank system.");
		else if(person->InCityEmpire() != person->EmpireId())
			resultMsg = tr("Only government officials of our empire may access the bank system.");
		else if(!hasAuthority)
			resultMsg = tr("Only government officials may access the bank system.");
		else
		{
			// Government official is in a bank of their empire.
			DaWorld* planet = person->FindParentPlanetInEnv();
			if(planet)
			{
				DaBd* bd = person->BdOp(opBd);
				if(bd)
				{
					if(!bd->InChainOfCommand(person))
						resultMsg = tr("Only government officials in the chain of command of this city may access the "
								"bank system.");
					else
					{
						double totalBank = bd->GovtAccountBalance();
						resultMsg = tr("Bank system balance is %L1%2.")
							.arg(totalBank,0,'f',0)
							.arg(g_Au.CurrencySymbol());
					}
				}
			}
		}
	}
	UnlockAvatar();

	if(!resultMsg.isEmpty())
	{
		// Send the result message to the person as a log message.
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::GovtAcctWithdrawFunds(double amount, const AuId& opBd)
{
	QString resultMsg;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		AuIdList placeChain;
		placeChain.append(person->InBdCity());
		person->ChainOfPlaces(placeChain);
		const DgData& gd = g_Au.LockDataForRead();
		bool hasAuthority = gd.EmpireCitizenCanAccessGovtAccount(person->EmpireId(), personId, placeChain);
		g_Au.UnlockData();

		if(person->InIndustry() != IID_Bank && person->InIndustry() != IID_Capitol)
			resultMsg = tr("I must stand at a bank to access the bank system.");
		else if(person->InCityEmpire() != person->EmpireId())
			resultMsg = tr("Only government officials of our empire may access the bank system.");
		else if(!hasAuthority)
			resultMsg = tr("Only government officials may access the bank system.");
		else
		{
			// Government official is in a bank of their empire.
			DaWorld* planet = person->FindParentPlanetInEnv();
			if(planet)
			{
				DaBd* bd = person->BdOp(opBd);
				if(bd)
				{
					if(!bd->InChainOfCommand(person))
						resultMsg = tr("Only government officials in the chain of command of this city may access the "
								"bank system.");
					else if(bd->GovtAccountBalance() < amount)
						resultMsg = tr("Withdrawal declined. Bank system balance is only %L1%2.")
							.arg(bd->GovtAccountBalance(),0,'f',0).arg(g_Au.CurrencySymbol());
					else
					{
						bd->DebitAccountGovtWithdrawal(amount);
						person->CreditAccount(amount);
						double totalBank = bd->GovtAccountBalance();
						resultMsg = tr("%L1%2 withdrawn from bank system. Bank system balance is %L3%2.")
							.arg(amount,0,'f',0)
							.arg(g_Au.CurrencySymbol())
							.arg(totalBank,0,'f',0);
					}
				}
			}
		}
	}
	UnlockAvatar();

	if(!resultMsg.isEmpty())
	{
		// Send the result message to the person as a log message.
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

bool AuAtomInterface::Rename(quint8 renameWhat, const QString& nuName, const AuId& capitolOpId)
{
	if(renameWhat > 5)
		return false;
	if(!nuName.isEmpty() && !IsNameValid(nuName))
		return false;

	QString resultMsg;
	AuId renameId;												// Will be set if the rename was successful, for emission of signal.
	unordered_map<AuId, QString, AuIdHash> planetsRenamed;		// For system renames that propagate to their planets.

	AuId systemId, sectorId;
	QString systemName, sectorName;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		DaSolarSystem* system = person->FindParentSolarSystemInEnv();
		if(system)
		{
			switch(renameWhat)
			{
			case 0:												// City
				if(nuName.isEmpty())
					break;
				if(!person->BdId().IsNull() &&
						(person->InIndustry() == IID_Capitol || person->InIndustry() == IID_BaseHeadquarters))
				{
					DaWorld* planet = person->FindParentPlanetInEnv();
					if(planet)
					{
						DaBd* bd = dynamic_cast<DaBd*>(planet->FindIdRecursive(person->BdId()));
						if(bd)
						{
							if(person->InIndustry() == IID_Capitol)
							{
								// Locate the capitol building.
								if(!bd->InChainOfCommand(person))
									resultMsg = tr("I am not in the chain of command of the city.");
								else
								{
									// Rename the city.
									bd->SetCityName(nuName);
									resultMsg = tr("The city was renamed to %1.").arg(nuName);
									renameId = bd->Id();
								}
							}
							else if(person->InIndustry() == IID_BaseHeadquarters)
							{
								// Locate the headquarters building.
								if(!bd->InChainOfCommand(person))
									resultMsg = tr("I am not in the chain of command of the military base.");
								else
								{
									// Rename the military base.
									bd->SetCityName(nuName);
									resultMsg = tr("The military base was renamed to %1.").arg(nuName);
									renameId = bd->Id();
								}
							}
						}
						else
							resultMsg = tr("Building?");
					}
					else
						resultMsg = tr("Planet?");
				}
				else
					resultMsg = tr("I am not in the capitol building of a city.");
				break;

			case 1:												// Planet
				if(!nuName.isEmpty())
				{
					// Locate the planet.
					DaWorld* planet = person->FindParentPlanetInEnv();
					if(planet)
					{
						if(planet->EmpireId() != person->EmpireId())
						{
							resultMsg = tr("The world cannot be renamed. It must first be claimed for my empire.");
						}
						else
						{
							// Person can rename a world if they are in its chain of command or
							// if they originally claimed it and they still belong to the empire who holds that claim.
							bool okToRename = planet->InChainOfCommand(personId) ||
								person->Is(planet->ClaimedByPersonId()) ||
								person->HasAgentPower();
							if(okToRename)
							{
								// Rename the planet.
								planet->SetName(nuName);
								planet->SetModified();
								resultMsg = tr("The world was renamed to %1.").arg(nuName);
								renameId = planet->Id();

								// Get system and sector information for officer homeworld updates.
								systemId = system->Id();
								systemName = system->Name();
								DaSector* sector = system->FindParentSector();
								if(sector)
								{
									sectorId = system->Id();
									sectorName = system->Name();
								}
							}
							else
							{
								resultMsg = tr("The world cannot be renamed. "
										"I must be in its chain of command or I must be the avatar who staked the claim.");
							}
						}
					}
					else
						resultMsg = tr("I must be at a world to rename the world.");
				}
				break;
			case 2:												// System
				if(!nuName.isEmpty())
				{
					AuIdList placeChain;
					system->ChainOfPlaces(placeChain);
					const DgData& gd = g_Au.LockDataForRead();
					bool inChainOfCommand = gd.IsEmpirePlaceInDomain(person->EmpireId(), placeChain, personId);
					g_Au.UnlockData();
					if(system->IsEmptySystem())
						resultMsg = tr("I cannot rename this region of deep space.");
					else if(!inChainOfCommand)
						resultMsg = tr("I am not in the chain of command of the solar system.");
					else if(!system->OkToRename(person->EmpireId()))
						resultMsg = tr("The solar system cannot be renamed. "
								"There are no cities of my empire or my empire does not have the loyalty of all cities.");
					else
					{
						// Rename the system.
						system->SetNameIncludePlanets(nuName, true);	// Rename that propagates to all planets.
						renameId = system->Id();

						// Get the names of all the planets since they were also changed.
						DaAtomList planets;
						system->FindPlanetsRecursive(planets);
						for(DaAtomList::const_iterator pi = planets.cbegin(); pi != planets.cend(); pi++)
							planetsRenamed.insert(pair<AuId, QString>((*pi)->Id(), (*pi)->Name()));

						resultMsg = tr("The solar system was renamed to %1. "
								"The change propagated to %2 worlds in the solar system.")
							.arg(nuName)
							.arg(planetsRenamed.size());

						// Get system and sector information for officer homeworld updates.
						systemId = system->Id();
						systemName = system->Name();
						DaSector* sector = system->FindParentSector();
						if(sector)
						{
							sectorId = system->Id();
							sectorName = system->Name();
						}
					}
				}
				break;
			case 3:												// Sector
				if(!nuName.isEmpty())
				{
					const DaSector* sector = person->FindParentSector();
					if(sector)
					{
						AuIdList placeChain;
						sector->ChainOfPlaces(placeChain);
						const DgData& gd = g_Au.LockDataForRead();
						bool inChainOfCommand = gd.IsEmpirePlaceInDomain(person->EmpireId(), placeChain, personId);
						bool okToRenameSector = gd.EmpireOkToRenameSector(person->EmpireId(), sector->Id());
						g_Au.UnlockData();
						if(!inChainOfCommand)
							resultMsg = tr("I am not in the chain of command of the sector.");
						else if(!okToRenameSector)
							resultMsg = tr("The sector cannot be renamed. "
									"There are no cities of my empire or there are cities that belong to another empire.");
						else
						{
							// Rename the sector.
							resultMsg = tr("The sector was renamed to %1.").arg(nuName);
							renameId = sector->Id();

							// The manager server is responsible for the sector atom. It will rename the atom when it receives my
							// signal. The change will be posted to the server farm so my local atom will eventually get changed.
						}
					}
				}
				break;
			case 4:												// Young Avatar
				if(!nuName.isEmpty())
				{
					if(person->IsFullGrown())
					{
						resultMsg = tr("I cannot change my name because I am fully grown.");
					}
					else
					{
						// Change the name of the avatar.
						person->SetName(nuName);
						person->SetModified();
						renameId = person->Id();
						resultMsg = tr("Henceforth I shall be called %1.").arg(nuName);
					}
				}
				break;
			case 5:												// Building
				if(person->BdOp(capitolOpId))
				{
					DaBd* bd = person->BdOp(capitolOpId);
					if(bd)
					{
						// nuName can be blank. bd sends its own result thought msgs.
						renameId = person->BdRename(nuName, capitolOpId);
					}
				}
				break;
			}
		}
	}
	UnlockAvatar();
	if(!renameId.IsNull())
	{
		// Emit a signal to notify the rest of the server farm of the name change.
		switch(renameWhat)
		{
		case 0:													// City/Capitol Building/Military HQ
			emit RenameCity(renameId, nuName);
			break;
		case 1:													// Planet
			emit RenamePlanet(renameId, nuName, systemId, systemName, sectorId, sectorName);
			break;
		case 2:													// System
			emit RenameSystem(renameId, nuName);

			// Must send planet rename notifications because their names also changed.
			for(unordered_map<AuId, QString, AuIdHash>::const_iterator pi = planetsRenamed.begin(); pi != planetsRenamed.end();
					pi++)
				emit RenamePlanet((*pi).first, (*pi).second, systemId, systemName, sectorId, sectorName);
			break;
		case 3:													// Sector
			emit RenameSector(renameId, nuName);
			break;
		case 4:
			emit RenameAvatar(renameId, nuName);				// Avatar
			break;
		case 5:
			// Building
			break;
		}
	}

	if(!resultMsg.isEmpty())
	{
		// Send a log message to the person, notifying them of the result.
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
	return !renameId.IsNull();
}

void AuAtomInterface::RedeemGovtDebt(const AuId& opBd)
{
	// See if city holds enough money to reward the player
	QString resultMsg;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		if(person->InIndustry() != IID_Bank && person->InIndustry() != IID_Capitol)
			resultMsg = tr("I must stand at a bank to redeem government debt.");
		else if(person->InCityEmpire() != person->EmpireId())
			resultMsg = tr("Only a bank of my empire can redeem government debt.");
		else
		{
			double amount = person->GetReward();
			if(amount < 1)
				resultMsg = tr("The government of my empire does not owe a debt to me.");
			else
			{
				// Person is standing in a bank that belongs to their empire.
				DaWorld* planet = person->FindParentPlanetInEnv();
				if(planet)
				{
					DaBd* bd = person->BdOp(opBd);
					if(bd)
					{
						double totalBank = bd->GovtAccountBalance();
						if(totalBank > 0)
						{
							if(amount > totalBank)
							{
								amount = totalBank;					// Limit withdrawal to the bank's funds on hand.
								resultMsg = tr("The bank paid %L1%2 toward the debt. "
										"That was all the money in the bank system of the city.")
									.arg(amount,0,'f',0)
									.arg(g_Au.CurrencySymbol());
							}
							else
								resultMsg = tr("The bank paid the entire %L1%2 debt from the bank system.")
									.arg(amount,0,'f',0)
									.arg(g_Au.CurrencySymbol());
							bd->DebitAccountGovtRewards(amount);	// Take funds from the bank.
							person->ReduceReward(amount);		// Reduce reward owed by the amount the bank could provide.

							/*DHH Bd
							// Collect income tax on the amount.
							if(city->EmpireIncomeTax())
							{
								float incomeTax = amount * (city->EmpireIncomeTax()/100);
								float netAmount = amount - incomeTax;
								person->CreditAccount(netAmount);	// Give the reward money to the person, after taxes.
								city->CreditAccountGovtIncomeTax(incomeTax);

								resultMsg += tr(" %L1%2 income tax was collected. "
										"The net amount deposited to my account was %L3%2.")
									.arg(incomeTax,0,'f',0)
									.arg(g_Au.CurrencySymbol())
									.arg(netAmount,0,'f',0);
							}
							else
							*/
								person->CreditAccount(amount);		// Give the reward money to the person.
						}
						else
							resultMsg = tr("The bank system in this city has no funds.");
					}
				}
			}
		}
	}
	UnlockAvatar();

	if(!resultMsg.isEmpty())
	{
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

bool AuAtomInterface::HangarLaunch(bool takeCaptBerth)
{
	QString resultMsg;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		if(person->InIndustry() == IID_Hangar)
		{
			DaWorld* planet = person->FindParentPlanetInEnv();
			Q_ASSERT(planet);
			DaVessel* hangarShip = 0;
			DaBd* bd = person->BdOp(AuId(0));
			if(bd)
				hangarShip = bd->FindShipInHangar(person->Id());
			if(hangarShip)
			{
				// Found a ship to launch.
				// Clear the roster of the entry used to identify the owner of the mothballed ship.
				hangarShip->RemoveBerthOccupant(hangarShip->CaptainId());
				if(takeCaptBerth)
				{
					// Take captain's berth.
					hangarShip->OccupyBerth(AuRosterEntry::Captain, person->Id(), person->Name());
					person->SetSpawnShip(hangarShip->Id(), hangarShip->Name(), AuRosterEntry::Captain,
							hangarShip->IsPrivatelyOwned());
					person->SetModified();
				}

				// Position the ship in orbit, aligned with up.
				AuPoint spawnAt;
				person->GetLocationRelativeToParent(spawnAt, planet);	// Use person's location.
				const double orbitAlt = planet->AtmosphereRadius() + ParkAltAboveAtm();
				spawnAt = planet->FindOrbitSpawnLoc(spawnAt, AuId(0), hangarShip->RoughRadius(), orbitAlt, true);
				AuMatrix4x4 t;
				t.Translate(spawnAt);
				AuPoint upVec = -planet->GravityDir(spawnAt);
				t *= AuPoint(0,0,1).GetAlignmentTransform(upVec);
				// Place the ship subordinate to the planet.
				hangarShip->ChangePosAndOrient(AuAtomPos(planet->Id(), t));

				// Reactivate the ship to bring it out of dormancy.
				hangarShip->Reactivate();
				hangarShip->InvalidateEnvironment();

				// Send a result message to the person.
				resultMsg = tr("%1 launched successfully into orbit.").arg(hangarShip->NameWithEmpire());
				if(takeCaptBerth)
					resultMsg += tr(" Captain's berth assigned. Recall home to go aboard.");
			}
			else
				resultMsg = tr("No ship belonging to me was found in this hangar.");
		}
		else
			resultMsg = tr("I must stand in a hangar to launch a spacecraft.");
	}
	UnlockAvatar();
	if(resultMsg.isEmpty())
		return false;

	AuCommThought msg(resultMsg, personId, personName);
	g_Au.SendAtomChat(msg);
	return true;
}

bool AuAtomInterface::HangarMothball(const AuId& shipId)
{
	QString resultMsg;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		if(person->InIndustry() != IID_Hangar)
			resultMsg = tr("I must stand at a hangar to mothball a spacecraft.");
		else if(PoliticalStance(person->InCityEmpire(), person->EmpireId()) == POL_Enemy)
			resultMsg = tr("This enemy city will not permit me to hangar a spacecraft.");
		else
		{
			DaWorld* planet = person->FindParentPlanetInEnv();
			Q_ASSERT(planet);
			DaVessel* hangarShip = 0;
			DaAtom* hangarAtom = 0;							// The parent atom of the mothballed ship while it is in storage.
			DaBd* bd = person->BdOp(AuId(0));
			if(bd)
			{
				hangarShip = bd->FindShipInHangar(person->Id());
				hangarAtom = bd;
			}

			if(hangarShip)
				resultMsg = tr("This hangar already holds a mothballed ship of mine.");
			else
			{
				DaVessel* mothballShip = dynamic_cast<DaVessel*>(planet->FindIdRecursive(shipId));
				if(!mothballShip)
					resultMsg = tr("I cannot find the ship to store in the hangar.");
				else if(mothballShip->IsSpaceStation())
					resultMsg = tr("%1 is a station; it cannot be hangared.").arg(mothballShip->NameWithEmpire());
				else if(!mothballShip->IsOnGround())
					resultMsg = tr("%1 is not on the ground.").arg(mothballShip->NameWithEmpire());
				else if(!mothballShip->IsReadyForRepairs())
					resultMsg = tr("%1 is not rigged for repairs.").arg(mothballShip->NameWithEmpire());
				else if(mothballShip->InDesignEnvironment())
					resultMsg = tr("%1 is in a design studio.").arg(mothballShip->NameWithEmpire());
				else
				{
					const DaCreature* creature = mothballShip->FindCreatureBlockingMothball();
					if(creature)
					{
						resultMsg = tr("%1 cannot be mothballed while %2 is aboard.")
							.arg(mothballShip->NameWithEmpire())
							.arg(creature->Name());
					}
					else
					{
						// Determine if the person has the authority to mothball the ship.
						int transferAuthority = 0;
						if(mothballShip->IsNew())
						{
							// The other ship is nu. I can only get into it if its empire is friendly to mine.
							POLITICAL_STANCE stance = PoliticalStance(mothballShip->EmpireId(), person->EmpireId());
							if(stance == POL_Empire || stance == POL_Friend || stance == POL_Vassal)
								transferAuthority = 3;				// Ship's empire is friendly toward mine.
						}
						else if(!mothballShip->HasOfficers())
							transferAuthority = 4;		// Ship is not nu and it has no officers.
						else if(mothballShip->InChainOfCommand(person))
							transferAuthority = 1;		// Ships in my chain of command can be mothballed.

						if(!transferAuthority)
							resultMsg = tr("I do not have the authority to mothball %1.")
								.arg(mothballShip->NameWithEmpire());
						else
						{
							// Mothball the ship.
							mothballShip->Mothball(person, hangarAtom);
							resultMsg = tr("%1 mothballed successfully in this hangar.").arg(mothballShip->NameWithEmpire());
						}
					}
				}
			}
		}
	}
	UnlockAvatar();
	if(resultMsg.isEmpty())
		return false;

	AuCommThought msg(resultMsg, personId, personName);
	g_Au.SendAtomChat(msg);
	return true;
}

void AuAtomInterface::AgentEncounter(quint8 type, quint32 designId)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->AgentEncounter(type, designId);
	UnlockAvatar();
}

void AuAtomInterface::AgentSetBits(quint8 bits)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->SetAgentBits(bits);
	UnlockAvatar();
}

void AuAtomInterface::GovSetShieldPolicy(quint8 whichPolicy, quint8 policyValue)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship)
		{
			// Person is aboard a spacecraft.
			// Any officer can change this policy.
			if(ship->InChainOfCommand(person))
			{
				switch(whichPolicy)
				{
				case 0:
					if(/*policyValue >= DaVessel::UnlessNoFuel && */policyValue <= ShieldUpIgnoreFuel)
					{
						ship->SetShieldFuelPolicy((SHIELDFUEL_POLICY)policyValue);
						ship->SetModified();
					}
					break;
				case 1:
					if(/*policyValue >= DaVessel::Up && */policyValue <= ShieldUpIfEnemy)
					{
						ship->SetShieldUpPolicy((SHIELD_POLICY)policyValue);
						ship->SetModified();
					}
					break;
				}
			}
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::GovSetPilotPolicy(quint8 whichPolicy, quint8 policyValue)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
        if(ship)
        {
            // Person is aboard a spacecraft.
            // Any officer can change this policy.
            if(ship->InChainOfCommand(person))
            {
                switch(whichPolicy)
                {
                case 0: // Pilot up policy.
                    if(/*policyValue >= UpAwayFromGravity && */policyValue < PILOT_UP_POLICY_Count)
                    {
                        ship->SetPilotUpPolicy((PILOT_UP_POLICY)policyValue);
                        ship->SetModified();
                    }
                    break;
                case 1: // Access policy.
                    if(policyValue <= ACCESS_Enemy)
                    {
                        ship->SetAccessPolicy((ACCESS_POLICY)policyValue);
                        ship->SetModified();
                    }
                    break;
                case 2: // Current alert condition.
                    if(policyValue <= ConditionRed)
                    {
                        ship->SetAlertCondition((ALERT_CONDITION)policyValue);
                        ship->SetModified();
                    }
                    break;
                case 3: // Vehicle autofetch policy.
                    if(policyValue < 2)
                    {
                        ship->SetAutoFetchVehicles(policyValue);
                        ship->SetModified();
                    }
                    break;
                }
            }
        }
	}
	UnlockAvatar();
}

void AuAtomInterface::AgentGoToLongLat(const AuGlobeLLF& ll, bool rideAlso)
{
	DaPerson* person = LockAvatar();
	if(person && AcctIsAgent(person->AccountTypeOnLine()))
	{
		QString response;
		DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(person->FindParentPlanetInEnv());
		if(llworld)
		{
			// Person is subordinate to world that can deal with longitude/latitude.
			DaUnit* moveAtom = person;
			if(rideAlso)
			{
				if(person->IsAboardVehicle())
					moveAtom = dynamic_cast<DaVehicle*>(person->Parent());  // aboard vehicle
				else if(dynamic_cast<DaCreature*>(person->Parent()))
					moveAtom = dynamic_cast<DaCreature*>(person->Parent()); // riding a mount
				else if(person->IsInSpacecraft())
					moveAtom = person->FindParentVesselInEnv();             // in a ship
			}

			if(moveAtom)
			{
				// Calculate a transformation matrix to put the atom on the ground at the nu location.
				AuPoint loc = llworld->GetSurfaceLocation(ll, moveAtom->GroundHeightAboveCG());
				AuPoint upvec = -llworld->GravityDir(loc);
				AuMatrix4x4 t;
				t.Translate(loc);
				t *= AuPoint(0,0,1).GetAlignmentTransform(upvec);           // Orient so down is toward the planet.
				moveAtom->ForcePosStop(AuAtomPos(llworld->Id(), t));
				if(moveAtom == person)
					person->Stand();                                        // Exit consoles, etc.
				moveAtom->SetModified();
				response = tr("%1 moved to %2 on %3.").arg(moveAtom->Name()).arg(ll.ToString()).arg(llworld->Name());
			}
			else
				response = tr("Nothing moved and I can't exactly say why.");
		}
		else
			response = tr("I have to be at a planet or ringworld to teleport to longitude latitude coordinates.");

		AuCommThought msg(response, person->Id(), person->Name());
		g_Au.SendAtomChat(msg);
	}
	UnlockAvatar();
}

bool AuAtomInterface::ReportCityLast(const AuId& cityId, bool& haveReport, AuCityReport& report, bool& havePrevReport,
		AuCityReport& prevReport, bool& personHasAgentPower, quint32& personEmpire)
{
	haveReport = false;
	havePrevReport = false;
	personHasAgentPower = false;
	personEmpire = 0;

	// Try to find the city locally.
	bool foundCity = false;
	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	if(person && system)
	{
		personEmpire = person->EmpireId();
		personHasAgentPower = person->HasAgentPower();
		const DaBd* bd;
		if(!cityId.IsNull())
			bd = dynamic_cast<const DaBd*>(system->FindIdRecursive(cityId));
		else if(!person->BdId().IsNull())
			bd = dynamic_cast<const DaBd*>(system->FindIdRecursive(person->BdId()));
		else
			bd = 0;
		if(bd)
		{
			foundCity = true;
			if(bd->EmpireId() == personEmpire || personHasAgentPower)
			{
				bd->ReportCurrent(report, false);
				haveReport = true;
				havePrevReport = bd->ReportLast(prevReport);
			}
		}

		if(personEmpire && !foundCity && !cityId.IsNull())
		{
			// City not found in my system. Try the entire root data.
			const DaBd* bd = dynamic_cast<const DaBd*>(person->Ether()->FindNormalAtom(cityId));
			if(bd)
			{
				foundCity = true;
				if(bd->EmpireId() == personEmpire || personHasAgentPower)
				{
					bd->ReportCurrent(report, false);
					haveReport = true;
					havePrevReport = bd->ReportLast(prevReport);
				}
			}
		}
	}
	UnlockAvatar();
	return foundCity;
}

bool AuAtomInterface::ReportCityHistory(const AuId& cityId, bool& haveReport, AuCityReportHistory& report,
		bool& personHasAgentPower, quint32& personEmpire)
{
	haveReport = false;
	personHasAgentPower = false;
	personEmpire = 0;

	// Try to find the city locally.
	bool foundCity = false;
	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	if(person && system)
	{
		personEmpire = person->EmpireId();
		personHasAgentPower = person->HasAgentPower();
		const DaAtom* atom = 0;
		if(!cityId.IsNull())
			atom = system->FindIdRecursive(cityId);
		else if(!person->BdId().IsNull())
			atom = system->FindIdRecursive(person->BdId());
		const DaBd* bd = dynamic_cast<const DaBd*>(atom);
		if(bd)
		{
			foundCity = true;
			if(bd->EmpireId() == personEmpire || personHasAgentPower)
			{
				bd->ReportHistory(report);
				haveReport = true;
				AuCityReport cur;
				bd->ReportCurrent(cur, false);
				report.Append(cur);
			}
		}

		if(!foundCity && !cityId.IsNull())
		{
			// City not found in my system. Try the entire root data.
			const DaBd* bd = dynamic_cast<const DaBd*>(person->Ether()->FindNormalAtom(cityId));
			if(bd)
			{
				foundCity = true;
				if(bd->EmpireId() == personEmpire || personHasAgentPower)
				{
					bd->ReportHistory(report);
					haveReport = true;
					AuCityReport cur;
					bd->ReportCurrent(cur, false);
					report.Append(cur);
				}
			}
		}
	}
	UnlockAvatar();
	if(!personEmpire)
		return false;
	return foundCity;
}

void AuAtomInterface::MailCityLast(const AuId& cityId)
{
	// Try to find the city locally.
	bool foundCity = false;
	quint32 personEmpire = 0;
	bool personHasAgentPower = false;
	AuId personId;
	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	if(person && system)
	{
		personId = person->Id();
		personEmpire = person->EmpireId();
		personHasAgentPower = person->HasAgentPower();
		const DaAtom* atom = 0;
		if(!cityId.IsNull())
			atom = system->FindIdRecursive(cityId);
		else if(!person->InBdCity().IsNull())
			atom = system->FindIdRecursive(person->InBdCity());
		const DaBd* bd = dynamic_cast<const DaBd*>(atom);
		if(bd)
		{
			foundCity = true;
			if(bd->EmpireId() == personEmpire || personHasAgentPower)
				bd->MailLastReport(person->Id());
		}

		if(!foundCity && personEmpire)
		{
			// City not found in my system. Try the entire root data.
			const DaAtom* atom = person->Ether()->FindNormalAtom(cityId);
			const DaBd* bd = dynamic_cast<const DaBd*>(atom);
			if(bd)
			{
				foundCity = true;
				if(bd->EmpireId() == personEmpire)
					bd->MailLastReport(person->Id());
			}
		}
	}
	UnlockAvatar();
	if(!personEmpire)
		return;

	if(!foundCity)
		emit CityMailRequestedSignal(cityId, personId, personEmpire);	// Initiate request for the mail up to master server.
}

void AuAtomInterface::OrdnanceLoad(int rack, const AuItem& ord)
{
	QString resultMsg;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		DaVehicle* vehicle = dynamic_cast<DaVehicle*>(person->Parent());
		DaVessel* ship = dynamic_cast<DaVessel*>(person->Parent());
		if(vehicle)
		{
			// Only certain locations can provide ordnance.
			INDUSTRYID iid = vehicle->InIndustry();
			if(DbIndustry::CanSupplyOrdnance(iid))
			{
				const DaWorld* world = vehicle->FindParentPlanetInEnv();
				if(world)
				{
					if(!vehicle->BdId().IsNull())
					{
						/*DHH
						DaBd* bd = world->FindBd(vehicle->BdId());
						if(bd && bd->TakeOrdnance(ord, vehicle->EmpireId()))
						{
							// If the vehicle's ordnance is full of ammo or it's an energy weapon, it can be returned to stock.
							AuOrdnance oldAmmo = vehicle->WeaponLoad((DaVehicle::WEAPON_RACK)rack);
							if(oldAmmo.IsEnergyWeapon() ||
									(oldAmmo.CommodityId() != CID_Invalid &&
									 oldAmmo.RoundsCount() == vehicle->MaxWeaponRounds(oldAmmo.CommodityId())))
							{
								// Put one unit of the ammo back into the bd's inventory.
								oldAmmo.Reload(1);						// We only return one commodity.
								bd->AddCommodity(*oldAmmo.Ammo(), false);
							}

							vehicle->LoadAmmunition(ord, (DaVehicle::WEAPON_RACK)rack);
							switch(rack)
							{
							case DaVehicle::RACK_Gun:
								resultMsg = tr("%1 loaded into gun on ground at %2.")
									.arg(ord.SayName()).arg(bd->Name());
								break;
							case DaVehicle::RACK_Ord0:
								resultMsg = tr("%1 loaded onto weapon rack 1 on ground at %2.")
									.arg(ord.SayName()).arg(bd->Name());
								break;
							case DaVehicle::RACK_Ord1:
								resultMsg = tr("%1 loaded onto weapon rack 2 on ground at %2.")
									.arg(ord.SayName()).arg(bd->Name());
								break;
							}
						}
						*/
					}
				}
			}
			else if(vehicle->IsInsideSpacecraft() && vehicle->ShipRoomType() == ROOM_VehicleBay)
			{
				DaVessel* ship = vehicle->FindParentVesselInEnv();
				if(ship)
				{
					if(ship->TakeOrdnance(ord, vehicle->EmpireId()))
					{
						// If the vehicle's ordnance is full of ammo or it's an energy weapon, it can be returned to stock.
						AuOrdnance oldAmmo = vehicle->WeaponLoad((DaVehicle::WEAPON_RACK)rack);
						if(oldAmmo.IsEnergyWeapon() ||
								(oldAmmo.CommodityId() != CID_Invalid &&
								 oldAmmo.RoundsCount() == vehicle->MaxWeaponRounds(oldAmmo.CommodityId())))
						{
							// Put one unit of the ammo back into the city's inventory.
							oldAmmo.Reload(1);						// We only return one commodity.
							ship->AddCargo(*oldAmmo.Ammo());
						}

						vehicle->LoadAmmunition(ord, (DaVehicle::WEAPON_RACK)rack);
						switch(rack)
						{
						case DaVehicle::RACK_Gun:
							resultMsg = tr("%1 loaded into gun aboard %2.")
								.arg(ord.SayName()).arg(ship->NameWithEmpire());
							break;
						case DaVehicle::RACK_Ord0:
							resultMsg = tr("%1 loaded onto weapon rack 1 aboard %2.")
								.arg(ord.SayName()).arg(ship->NameWithEmpire());
							break;
						case DaVehicle::RACK_Ord1:
							resultMsg = tr("%1 loaded onto weapon rack 2 aboard %2.")
								.arg(ord.SayName()).arg(ship->NameWithEmpire());
							break;
						}
					}
				}
			}
			else
			{
				bool hasStation = false;
				DaVessel* station = vehicle->StationInTradeRange(vehicle->RoughRadius(), hasStation);
				if(station && station->TakeOrdnance(ord, vehicle->EmpireId()))
				{
					// If the vehicle's ordnance is full of ammo or it's an energy weapon, it can be returned to stock.
					AuOrdnance oldAmmo = vehicle->WeaponLoad((DaVehicle::WEAPON_RACK)rack);
					if(oldAmmo.IsEnergyWeapon() ||
							(oldAmmo.CommodityId() != CID_Invalid &&
							 oldAmmo.RoundsCount() == vehicle->MaxWeaponRounds(oldAmmo.CommodityId())))
					{
						// Put one unit of the ammo back into the city's inventory.
						oldAmmo.Reload(1);						// We only return one commodity.
						station->AddCargo(*oldAmmo.Ammo());
					}

					vehicle->LoadAmmunition(ord, (DaVehicle::WEAPON_RACK)rack);
					switch(rack)
					{
					case DaVehicle::RACK_Gun:
						resultMsg = tr("%1 loaded into gun near %2.")
							.arg(ord.SayName()).arg(station->Name());
						break;
					case DaVehicle::RACK_Ord0:
						resultMsg = tr("%1 loaded onto weapon rack 1 near %2.")
							.arg(ord.SayName()).arg(station->Name());
						break;
					case DaVehicle::RACK_Ord1:
						resultMsg = tr("%1 loaded onto weapon rack 2 near %2.")
							.arg(ord.SayName()).arg(station->Name());
						break;
					}
				}
			}
		}
		else if(ship)
		{
			if(person->IsTurretPose())
			{
				// Person is loading a turret.
				quint16 whichTurret = person->StationId();
				if(ship->CanLoadTurretAmmunition(ord, whichTurret))
				{
					if(ship->TakeOrdnance(ord, ship->EmpireId()))
					{
						ship->LoadTurretAmmunition(ord, whichTurret);
						resultMsg = tr("%1 loaded into turret aboard %2.")
							.arg(ord.SayName()).arg(ship->NameWithEmpire());
					}
					else
					{
						resultMsg = tr("%1 was not found in the hold of %2.")
							.arg(ord.SayName())
							.arg(ship->NameWithEmpire());
					}
				}
				else
				{
					COMMODITYID turretCID = ship->SystemModule(SYS_Turret, whichTurret);
					resultMsg = tr("%1 cannot be loaded into turret aboard %2; a %3 is installed.")
						.arg(ord.SayName())
						.arg(ship->NameWithEmpire())
						.arg(g_Au.CommodityName(turretCID));
				}
			}
			else
			{
				// Person is selecting ammo for a weapon bay.
				const AuItemHash& sinv = ship->LockCargoForRead();
				bool o = sinv.Find(ord) != sinv.end();
				ship->UnlockCargo();
				if(o)
				{
					quint16 whichBay = person->ControlId();
					if(ship->SelectBayWeaponAmmunition(ord, whichBay))
					{
						resultMsg = tr("%1 selected for weapon bay %3 of %2.")
							.arg(ord.SayName()).arg(ship->NameWithEmpire()).arg(whichBay+1);
					}
					else
					{
						COMMODITYID bayCID = ship->SystemModule(SYS_Weapon, whichBay);
						resultMsg = tr("%1 cannot be selected for bay %3 of %2; a %4 is installed.")
							.arg(ord.SayName())
							.arg(ship->NameWithEmpire())
							.arg(whichBay+1)
							.arg(g_Au.CommodityName(bayCID));
					}
				}
			}
		}
	}
	UnlockAvatar();

	if(!resultMsg.isEmpty())
	{
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::ArenaExit()
{
	DaPerson* person = LockAvatar();
	if(person && person->FindParentArena())
		person->ArenaExit();
	UnlockAvatar();
}

void AuAtomInterface::ArenaRecall()
{
	DaPerson* person = LockAvatar();
	if(person && person->FindParentArena())
		person->ArenaRecall();
	UnlockAvatar();
}

void AuAtomInterface::ArenaStatus()
{
	DaPerson* person = LockAvatar();
	if(person && person->FindParentArena())
		person->ArenaStatus();
	UnlockAvatar();
}

void AuAtomInterface::ArenaSetPassword(const QString& pass)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		person->SetDesignPassword(pass);
		person->SetModified();
	}
	UnlockAvatar();
}

void AuAtomInterface::ArenaCreateNew(const AuArenaReady& arena)
{
	if(!arena.m_ShipDesign)
		return;

	DaPerson* person = LockAvatar();
	if(person && person->InIndustry() == IID_Arena)
	{
		DaAtom* studioParent = person->Parent();
		if(dynamic_cast<DaCreature*>(studioParent))
		{
			QString resultMsg = tr("I must dismount before entering an arena.");
			AuCommThought msg(resultMsg, person->Id(), person->Name());
			g_Au.SendAtomChat(msg);
		}
		else if(dynamic_cast<DaVehicle*>(studioParent))
		{
			QString resultMsg = tr("I must exit the vehicle before entering an arena.");
			AuCommThought msg(resultMsg, person->Id(), person->Name());
			g_Au.SendAtomChat(msg);
		}
		else if(dynamic_cast<DaVessel*>(studioParent))
		{
			QString resultMsg = tr("I must exit the spacecraft before entering an arena.");
			AuCommThought msg(resultMsg, person->Id(), person->Name());
			g_Au.SendAtomChat(msg);
		}
		else
		{
			// The person's parent atom is appropriate to host an arena. They are not aboard a vehicle or mount.
			DaWorldLL* llworld = dynamic_cast<DaWorldLL*>(studioParent);
			if(llworld)
			{
				// Try loading the ship design.
				AuScReport r;
				const DgData& sdd = g_Au.LockScDesignsForRead();
				bool loaded = sdd.ScDesignReport(arena.m_ShipDesign, r);
				g_Au.UnlockScDesigns();
				if(!loaded)
				{
					QString resultMsg = tr("Library is loading the spacecraft blueprint. Try again shortly.");
					AuCommThought msg(resultMsg, person->Id(), person->Name());
					g_Au.SendAtomChat(msg);
				}
				else
				{
					// Enter DaArena arena instance.
					// Create a nu arena atom subordinate to the parent terrain.
					DaArena* pde = new DaArena(person->Name());
					pde->ConnectSignals();
					pde->Create();

					// Set the transform to the scene atom's current position.
					// This is where they will exit the arena.
					pde->SetPosAndOrient(AuAtomPos(studioParent->Id(), person->Transform()));
					person->Ether()->AddAtom(pde);
					g_Au.AtomAdded(pde->Id());
					pde->SetupArena(arena);

					// Put the scene atom into the arena. The arena will set them up with a ship.
					// Save parent atom id, location, and inventory for exit from arena.
					person->PrepareToEnterArena(DgEmpire::ArenaTeam1Id());
					person->SetArenaShipDesign(arena.m_ShipDesign);
					AuMatrix4x4 t;
					t.Translate((c_ArenaRingRadius + c_ArenaBoundsRadius)/2, 0, 0);
					person->ForcePosStop(AuAtomPos(pde->Id(), t));
					person->SetModified();
					pde->SetNormal();	// New atom is ready for normal thread processing.
					pde->TickTockRecursive();
				}
			}
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::NameTheBaby(const QString& babyName)
{
	if(!IsNameValid(babyName))
		return;
	DaPerson* person = LockAvatar();
	if(person)
		person->NameTheBaby(babyName);
	UnlockAvatar();
}

void AuAtomInterface::SetAutoEat(bool autoEat)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		person->SetAutoEat(autoEat);
		person->SetModified();
	}
	UnlockAvatar();
}

void AuAtomInterface::TransporterCoord(const AuPoint& coord)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->TransporterSetCoord(coord);
	UnlockAvatar();
}

void AuAtomInterface::TransporterLongLat(const AuGlobeLLF& ll)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->TransporterSetLongLat(ll);
	UnlockAvatar();
}

void AuAtomInterface::MoveItemRight(const AuId& itemId, double amount, bool relativeToItem)
{
	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	if(person && system)
    {
        DaItem* item = (DaItem*)system->FindIdRecursive(itemId);
        if(item && (item->IsStaticDisplayControllableBy(person->Id(), person->EmpireId()) || person->HasAgentPower()))
        {
            AuAtomPos pos(item->Pos());
            if(relativeToItem)
                pos.Translate(amount, 0, 0);
            else
            {
                AuPoint vec = pos.Transform().UnProjectVector(AuPoint(amount, 0, 0));   // Parent vector in my space.
                AuMatrix4x4 t;
                t.Translate(vec);
                pos.TransformBy(t);
            }
            item->ChangePosAndOrientNotParent(pos);
            item->SetStaticRoom(person->CapsuleRoomName());
            item->InvalidateEnvironment();
            item->SetModified();
        }
    }
	UnlockAvatar();
}

void AuAtomInterface::MoveItemLeft(const AuId& itemId, double amount, bool relativeToItem)
{
	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	if(person && system)
    {
        DaItem* item = (DaItem*)system->FindIdRecursive(itemId);
        if(item && (item->IsStaticDisplayControllableBy(person->Id(), person->EmpireId()) || person->HasAgentPower()))
        {
			AuAtomPos pos(item->Pos());
            if(relativeToItem)
                pos.Translate(-amount, 0, 0);
            else
            {
                AuPoint vec = pos.Transform().UnProjectVector(AuPoint(-amount, 0, 0));  // Parent vector in my space.
                AuMatrix4x4 t;
                t.Translate(vec);
                pos.TransformBy(t);
            }
            item->ChangePosAndOrientNotParent(pos);
            item->SetStaticRoom(person->CapsuleRoomName());
            item->InvalidateEnvironment();
            item->SetModified();
        }
    }
	UnlockAvatar();
}

void AuAtomInterface::MoveItemForward(const AuId& itemId, double amount, bool relativeToItem)
{
	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	if(person && system)
    {
        DaItem* item = (DaItem*)system->FindIdRecursive(itemId);
        if(item && (item->IsStaticDisplayControllableBy(person->Id(), person->EmpireId()) || person->HasAgentPower()))
        {
			AuAtomPos pos(item->Pos());
            if(relativeToItem)
                pos.Translate(0, amount, 0);
            else
            {
                AuPoint vec = pos.Transform().UnProjectVector(AuPoint(0, amount, 0));   // Parent vector in my space.
                AuMatrix4x4 t;
                t.Translate(vec);
                pos.TransformBy(t);
            }
            item->ChangePosAndOrientNotParent(pos);
            item->SetStaticRoom(person->CapsuleRoomName());
            item->InvalidateEnvironment();
            item->SetModified();
        }
    }
	UnlockAvatar();
}

void AuAtomInterface::MoveItemBackward(const AuId& itemId, double amount, bool relativeToItem)
{
	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	if(person && system)
    {
        DaItem* item = (DaItem*)system->FindIdRecursive(itemId);
        if(item && (item->IsStaticDisplayControllableBy(person->Id(), person->EmpireId()) || person->HasAgentPower()))
        {
			AuAtomPos pos(item->Pos());
            if(relativeToItem)
                pos.Translate(0, -amount, 0);
            else
            {
                AuPoint vec = pos.Transform().UnProjectVector(AuPoint(0, -amount, 0));  // Parent vector in my space.
                AuMatrix4x4 t;
                t.Translate(vec);
                pos.TransformBy(t);
            }
            item->ChangePosAndOrientNotParent(pos);
            item->SetStaticRoom(person->CapsuleRoomName());
            item->InvalidateEnvironment();
            item->SetModified();
        }
    }
	UnlockAvatar();
}

void AuAtomInterface::MoveItemUp(const AuId& itemId, double amount, bool relativeToItem)
{
	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	if(person && system)
    {
        DaItem* item = (DaItem*)system->FindIdRecursive(itemId);
        if(item && (item->IsStaticDisplayControllableBy(person->Id(), person->EmpireId()) || person->HasAgentPower()))
        {
			AuAtomPos pos(item->Pos());
            if(relativeToItem)
                pos.Translate(0, 0, amount);
            else
            {
                AuPoint vec = pos.Transform().UnProjectVector(AuPoint(0, 0, amount));   // Parent vector in my space.
                AuMatrix4x4 t;
                t.Translate(vec);
                pos.TransformBy(t);
            }
            item->ChangePosAndOrientNotParent(pos);
            item->SetStaticRoom(person->CapsuleRoomName());
            item->InvalidateEnvironment();
            item->SetModified();
        }
    }
	UnlockAvatar();
}

void AuAtomInterface::MoveItemDown(const AuId& itemId, double amount, bool relativeToItem)
{
	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	if(person && system)
    {
        DaItem* item = (DaItem*)system->FindIdRecursive(itemId);
        if(item && (item->IsStaticDisplayControllableBy(person->Id(), person->EmpireId()) || person->HasAgentPower()))
        {
			AuAtomPos pos(item->Pos());
            if(relativeToItem)
                pos.Translate(0, 0, -amount);
            else
            {
                AuPoint vec = pos.Transform().UnProjectVector(AuPoint(0, 0, -amount));  // Parent vector in my space.
                AuMatrix4x4 t;
                t.Translate(vec);
                pos.TransformBy(t);
            }
            item->ChangePosAndOrientNotParent(pos);
            item->SetStaticRoom(person->CapsuleRoomName());
            item->InvalidateEnvironment();
            item->SetModified();
        }
    }
	UnlockAvatar();
}

void AuAtomInterface::RotateItemXPos(const AuId& itemId, double amount, bool relativeToItem)
{
	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	if(person && system)
    {
        DaItem* item = (DaItem*)system->FindIdRecursive(itemId);
        if(item && (item->IsStaticDisplayControllableBy(person->Id(), person->EmpireId()) || person->HasAgentPower()))
        {
			AuAtomPos pos(item->Pos());
            if(relativeToItem)
                pos.RotateX(amount);
            else
            {
                AuPoint vec = pos.UnProjectVector(AuPoint(1, 0, 0));    // Parent's axis in my space.
                pos.RotateAxis(amount, vec);
            }
            item->ChangePosAndOrientNotParent(pos);
            item->SetStaticRoom(person->CapsuleRoomName());
            item->InvalidateEnvironment();
            item->SetModified();
        }
    }
	UnlockAvatar();
}

void AuAtomInterface::RotateItemXNeg(const AuId& itemId, double amount, bool relativeToItem)
{
	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	if(person && system)
    {
        DaItem* item = (DaItem*)system->FindIdRecursive(itemId);
        if(item && (item->IsStaticDisplayControllableBy(person->Id(), person->EmpireId()) || person->HasAgentPower()))
        {
			AuAtomPos pos(item->Pos());
            if(relativeToItem)
                pos.RotateX(-amount);
            else
            {
                AuPoint vec = pos.UnProjectVector(AuPoint(1, 0, 0));    // Parent's axis in my space.
                pos.RotateAxis(-amount, vec);
            }
            item->ChangePosAndOrientNotParent(pos);
            item->SetStaticRoom(person->CapsuleRoomName());
            item->InvalidateEnvironment();
            item->SetModified();
        }
    }
	UnlockAvatar();
}

void AuAtomInterface::RotateItemYPos(const AuId& itemId, double amount, bool relativeToItem)
{
	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	if(person && system)
    {
        DaItem* item = (DaItem*)system->FindIdRecursive(itemId);
        if(item && (item->IsStaticDisplayControllableBy(person->Id(), person->EmpireId()) || person->HasAgentPower()))
        {
			AuAtomPos pos(item->Pos());
            if(relativeToItem)
                pos.RotateY(amount);
            else
            {
                AuPoint vec = pos.UnProjectVector(AuPoint(0, 1, 0));    // Parent's axis in my space.
                pos.RotateAxis(amount, vec);
            }
            item->ChangePosAndOrientNotParent(pos);
            item->SetStaticRoom(person->CapsuleRoomName());
            item->InvalidateEnvironment();
            item->SetModified();
        }
    }
	UnlockAvatar();
}

void AuAtomInterface::RotateItemYNeg(const AuId& itemId, double amount, bool relativeToItem)
{
	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	if(person && system)
    {
        DaItem* item = (DaItem*)system->FindIdRecursive(itemId);
        if(item && (item->IsStaticDisplayControllableBy(person->Id(), person->EmpireId()) || person->HasAgentPower()))
        {
			AuAtomPos pos(item->Pos());
            if(relativeToItem)
                pos.RotateY(-amount);
            else
            {
                AuPoint vec = pos.UnProjectVector(AuPoint(0, 1, 0));    // Parent's axis in my space.
                pos.RotateAxis(-amount, vec);
            }
            item->ChangePosAndOrientNotParent(pos);
            item->SetStaticRoom(person->CapsuleRoomName());
            item->InvalidateEnvironment();
            item->SetModified();
        }
    }
	UnlockAvatar();
}

void AuAtomInterface::RotateItemZPos(const AuId& itemId, double amount, bool relativeToItem)
{
	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	if(person && system)
    {
        DaItem* item = (DaItem*)system->FindIdRecursive(itemId);
        if(item && (item->IsStaticDisplayControllableBy(person->Id(), person->EmpireId()) || person->HasAgentPower()))
        {
			AuAtomPos pos(item->Pos());
            if(relativeToItem)
                pos.RotateZ(amount);
            else
            {
                AuPoint vec = pos.UnProjectVector(AuPoint(0, 0, 1));    // Parent's axis in my space.
                pos.RotateAxis(amount, vec);
            }
            item->ChangePosAndOrientNotParent(pos);
            item->SetStaticRoom(person->CapsuleRoomName());
            item->InvalidateEnvironment();
            item->SetModified();
        }
    }
	UnlockAvatar();
}

void AuAtomInterface::RotateItemZNeg(const AuId& itemId, double amount, bool relativeToItem)
{
	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	if(person && system)
    {
        DaItem* item = (DaItem*)system->FindIdRecursive(itemId);
        if(item && (item->IsStaticDisplayControllableBy(person->Id(), person->EmpireId()) || person->HasAgentPower()))
        {
			AuAtomPos pos(item->Pos());
            if(relativeToItem)
                pos.RotateZ(-amount);
            else
            {
                AuPoint vec = pos.UnProjectVector(AuPoint(0, 0, 1));    // Parent's axis in my space.
                pos.RotateAxis(-amount, vec);
            }
            item->ChangePosAndOrientNotParent(pos);
            item->SetStaticRoom(person->CapsuleRoomName());
            item->InvalidateEnvironment();
            item->SetModified();
        }
    }
	UnlockAvatar();
}

void AuAtomInterface::ResetItemTransform(const AuId& itemId, const AuMatrix4x4& t, const QString& room)
{
	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	if(person && system)
    {
        DaItem* item = (DaItem*)system->FindIdRecursive(itemId);
        if(item && (item->IsStaticDisplayControllableBy(person->Id(), person->EmpireId()) || person->HasAgentPower()))
        {
			item->ChangePosAndOrientNotParent(AuAtomPos(item->ParentId(), t));
            item->SetStaticRoom(room);
            item->InvalidateEnvironment();
            item->SetModified();
        }
    }
	UnlockAvatar();
}

void AuAtomInterface::TakeItem(const AuId& itemId)
{
	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	if(person && system)
    {
        DaItem* item = (DaItem*)system->FindIdRecursive(itemId);
		if(item &&
				(person->Parent() == item->Parent() ||      // Scene atom is subordinate to same parent as item.
                 (dynamic_cast<DaBd*>(item->Parent()) && person->Parent() == item->Parent()->Parent())))    // Item on bd
            person->PickUp(item);
    }
	UnlockAvatar();
}

void AuAtomInterface::BankLoan(double amount, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BankLoan(amount, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BankPay(double amount, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BankPay(amount, opBd);
	UnlockAvatar();
}

void AuAtomInterface::GovSetAnnouncements(const QString& announcements)
{
	int result = 0;
	quint32 empireId = 0;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		empireId = person->EmpireId();
		const DgData& gd = g_Au.LockDataForRead();
		if(gd.EmpireCitizenCanSetAnnouncements(empireId, personId))
			result = 1;											// Ok to set announcements.
		else
			result = 2;											// Person does not have authority to do that.
		g_Au.UnlockData();
	}
	UnlockAvatar();

	// Notify the player of the result.
	QString resultMsg;
	switch(result)
	{
	default:
	case 0:								// Unknown error.
		break;
	case 1:								// Ok to set capital
		{
			emit GovSetAnnouncements(empireId, announcements);
			if(announcements.isEmpty())
				resultMsg = tr("Empire announcements cleared.");
			else
				resultMsg = tr("Empire announcements updated.");
		}
		break;
	case 2:								// City has no Capitol building.
		resultMsg = tr("Only authorized personnel can change empire announcements.");
		break;
	}

	if(!resultMsg.isEmpty())
	{
		// Send the result on the Log channel.
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::GovSetWarCriteria(const AuWarCriteria& crit)
{
	int result = 0;
	quint32 empireId = 0;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		empireId = person->EmpireId();
		const DgData& gd = g_Au.LockDataForRead();
		if(gd.EmpireCitizenCanSetWarCriteria(empireId, personId))
			result = 1;											// Ok to set War Criteria.
		else
			result = 2;											// Person does not have authority to do that.
		g_Au.UnlockData();
	}
	UnlockAvatar();

	// Notify the player of the result.
	QString resultMsg;
	switch(result)
	{
	default:
	case 0:								// Unknown error.
		break;
	case 1:								// Ok to set capital
		{
			emit GovSetWarCriteria(empireId, crit);
			resultMsg = tr("Empire war policy updated.");
		}
		break;
	case 2:								// City has no Capitol building.
		resultMsg = tr("Only authorized personnel can change empire war policy.");
		break;
	}

	if(!resultMsg.isEmpty())
	{
		// Send the result on the Log channel.
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::GovSetThemeColor(quint8 theme, const QColor& color)
{
	int result = 0;
	quint32 empireId = 0;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		empireId = person->EmpireId();
		const DgData& gd = g_Au.LockDataForRead();
		if(gd.EmpireCitizenCanSetColorTheme(empireId, personId))
			result = 1;											// Ok to set announcements.
		else
			result = 2;											// Person does not have authority to do that.
		g_Au.UnlockData();
	}
	UnlockAvatar();

	// Notify the player of the result.
	QString resultMsg;
	switch(result)
	{
	default:
	case 0:								// Unknown error.
		break;
	case 1:
		{
			emit GovSetThemeColor(empireId, theme, color);
			resultMsg = tr("Empire color theme updated.");
		}
		break;
	case 2:
		resultMsg = tr("Only authorized personnel can change empire color theme.");
		break;
	}

	if(!resultMsg.isEmpty())
	{
		// Send the result on the Log channel.
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::ChangeRoadConstruction(const AuId& cityId, const AuGlobeLLF& roadDevLL, CONSTRUCTIONPROCESSID cpid)
{
	QString resultMsg;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	if(person && system)
	{
		personId = person->Id();
		personName = person->Name();
		DaAtom* target = system->FindIdRecursive(cityId);
		DaWorldLL* world = dynamic_cast<DaWorldLL*>(target);	// for geo roads
		if(world)
		{
			// Changing a geo road.
			switch(world->ChangeRoadConstruction(roadDevLL, cpid))
			{
			case 0:
				// Player Statistics
				person->Stats().BuildingPlanned(IID_Road, world, 1);
				person->IncreaseSkill(AuSkill::CityPlanner, 1);
				switch(cpid)
				{
				default:
				case CPID_Irrigation:
					resultMsg = tr("Road construction changed to irrigated area.");
					break;
				case CPID_Clearing:
					resultMsg = tr("Road construction changed to cleared area.");
					break;
				case CPID_Defoliant:
					resultMsg = tr("Road construction changed to defoliated area.");
					break;
				case CPID_RoadGrade:
					resultMsg = tr("Road construction changed to graded area.");
					break;
				case CPID_DirtRoad:
					resultMsg = tr("Road construction changed to dirt.");
					break;
				case CPID_AsphaltRoad:
					resultMsg = tr("Road construction changed to asphalt.");
					break;
				case CPID_ConcreteRoad:
					resultMsg = tr("Road construction changed to concrete.");
					break;
				}
				break;

			case 1:
				resultMsg = tr("Road construction was not changed.");
				break;
			case 2:
				resultMsg = tr("Road construction cannot be changed while it is under construction.");
				break;
			case 3:
				resultMsg = tr("Road was not changed; it was built using that construction.");
				break;
			case 4:
				resultMsg = tr("Road was not changed; its construction cannot be changed.");
				break;
			case 11:
				resultMsg = tr("Road was not found.");
				break;
			case 21:
				resultMsg = tr("Asphalt road cannot be constructed on inferno world.");
				break;
			case 22:
				resultMsg = tr("City was not found.");
				break;
			}
		}
	}
	UnlockAvatar();

	if(!resultMsg.isEmpty())
	{
		// Send the result on the Log channel.
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::DemolishBd(const AuId& bdId)
{
	QString resultMsg;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	DaWorldLL* llworld = person ? dynamic_cast<DaWorldLL*>(person->FindParentPlanetInEnv()) : 0;
	if(person && llworld)
	{
		personId = person->Id();
		personName = person->Name();
		DaBd* bd = llworld->FindBd(bdId);
		if(bd)
		{
			if(!bd->InChainOfCommand(person))
				resultMsg = tr("I do not have authority to demolish the building.");
			else if(llworld->EnemyUnitBlocksDestruction(person->EmpireId()))
				resultMsg = tr("Nothing can be demolished while an enemy unit is present on this world.");
			else if(bd->FindPersonRecursive())
				resultMsg = tr("The building cannot be demolished while someone is in the building.");
			else if(bd->IndustryId() == IID_Capitol)
			{
				// Capitol cannot be demolished if any police stations are in its jurisdiction.
				if(llworld->HasCityPolice(bd->CapitolId()))
					resultMsg = tr("The capitol cannot be demolished while the city has police stations.");
				else
					bd->Demolish();
			}
			else if(bd->IndustryId() == IID_BaseHeadquarters)
			{
				// Base HQ cannot be demolished if any military buildings are on the planet.
				if(llworld->HasAnyMilitaryNotHQ())
					resultMsg = tr("The headquarters cannot be demolished while the world has military buildings.");
				else
					bd->Demolish();
			}
			else
				bd->Demolish();
		}
		else
			resultMsg = tr("No building was found to demolish.");
	}
	UnlockAvatar();
	if(!resultMsg.isEmpty())
	{
		// Send the result on the Log channel.
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::Selfie(const QImage& img)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		person->UpdateSelfie(img);

		AuCommThought msg(tr("My selfie was updated."), person->Id(), person->Name());
		g_Au.SendAtomChat(msg);
	}
	UnlockAvatar();
}

void AuAtomInterface::ClaimSpacecraftTitle()
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		if(person->EmpireId())
		{
			DaVessel* ship = person->FindParentVesselInEnv();
			if(ship)
			{
				if(!ship->IsPrivatelyOwned())
				{
					AuCommThought msg(tr("I can only claim the title to company spacecraft."),
							person->Id(), person->Name());
					g_Au.SendAtomChat(msg);
				}
				else if(ship->OwnerId().IsNull())
				{
					ship->SetPrivateOwner(person->Id(), person->Name());

					AuCommThought msg(tr("I claimed the title to %1.").arg(ship->Name()),
							person->Id(), person->Name());
					g_Au.SendAtomChat(msg);
				}
				else if(ship->OwnerId() == person->Id())
				{
					AuCommThought msg(tr("Silly me, I already own this spacecraft."),
							person->Id(), person->Name());
					g_Au.SendAtomChat(msg);
				}
				else if(person->HasAgentPower())
				{
					ship->SetPrivateOwner(person->Id(), person->Name());

					AuCommThought msg(tr("I used my agent powers to claim the title to %1.").arg(ship->Name()),
							person->Id(), person->Name());
					g_Au.SendAtomChat(msg);
				}
				else
				{
					AuCommThought msg(tr("I cannot claim the title to a spacecraft that already has an owner."),
							person->Id(), person->Name());
					g_Au.SendAtomChat(msg);
				}
			}
			else
			{
				AuCommThought msg(tr("I must board a spacecraft with a free title to claim it."),
						person->Id(), person->Name());
				g_Au.SendAtomChat(msg);
			}
		}
		else
		{
			AuCommThought msg(tr("As a free person, I cannot own any spacecraft."),
					person->Id(), person->Name());
			g_Au.SendAtomChat(msg);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::ReleaseSpacecraftTitle()
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship)
		{
			if(!ship->IsPrivatelyOwned())
			{
				AuCommThought msg(tr("I cannot release the title to a spacecraft I do not own."),
						person->Id(), person->Name());
				g_Au.SendAtomChat(msg);
			}
			else if(ship->OwnerId() == person->Id())
			{
				ship->SetPrivateOwner(AuId(), QString());

				AuCommThought msg(tr("I released the title to %1, relinquishing my ownership.").arg(ship->Name()),
						person->Id(), person->Name());
				g_Au.SendAtomChat(msg);
			}
			else if(person->HasAgentPower())
			{
				ship->SetPrivateOwner(AuId(), QString());

				AuCommThought msg(tr("I used my agent powers to release the title to %1.").arg(ship->Name()),
						person->Id(), person->Name());
				g_Au.SendAtomChat(msg);
			}
			else
			{
				AuCommThought msg(tr("I cannot release the title to a spacecraft I do not own."),
						person->Id(), person->Name());
				g_Au.SendAtomChat(msg);
			}
		}
		else
		{
			AuCommThought msg(tr("I must board a spacecraft I own to release the title."),
					person->Id(), person->Name());
			g_Au.SendAtomChat(msg);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::RepairShopRecover()
{
	bool restricted = false;
	bool isOfficer = false;
	quint8 galaxy = GALAXY_Count;
	QString cityEmpireName;
	AuId shipId;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person && person->InIndustry() == IID_AirportRepairShop)
	{
		personId = person->Id();
		personName = person->Name();
		isOfficer = AuRosterEntry::IsOfficer(person->SpawnShipBerthType());
		if(isOfficer)
		{
			galaxy = person->Galaxy();
			quint32 personEmpire = person->EmpireId();
			quint32 cityEmpire = person->InCityEmpire();
			if(personEmpire != cityEmpire)
			{
				// Check repair policy of city empire toward person's empire.
				const DgData& gd = g_Au.LockDataForRead();
				POLITICAL_STANCE stance = gd.EmpirePoliticalStance(cityEmpire, personEmpire);
				gd.EmpireName(cityEmpire, cityEmpireName);
				restricted = gd.EmpireCommerceRestricted(cityEmpire, CID_Sunlight, stance);	// Sunlight = repair
				g_Au.UnlockData();
			}

			if(!restricted)
				shipId = person->SpawnShip();
		}
	}
	UnlockAvatar();

	if(!isOfficer)
	{
		QString resultMsg = tr("I cannot recover the ship because I don't hold an officer berth.");
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
		return;
	}

	if(restricted)
	{
		QString resultMsg = tr("%1 commerce restrictions prevent this shop from recovering spacecraft for me.")
			.arg(cityEmpireName);
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
		return;
	}

	if(!shipId.IsNull())
		emit SpacecraftStasisRecover(shipId, personId, galaxy, false);
}

void AuAtomInterface::DesignerArchitectSaveSc(AuScDesign& design)
{
	if(!IsNameValid(design.Name()))
		return;
	if(design.Thumbnail().width() > 256 || design.Thumbnail().height() > 256)
		return;

	// New designs should always start in Edit mode.
	if(!design.Id())
		design.SetState(AuDesign::Edit);

	// The client sent us the entire design, potentially finalized.
	DgData& sdd = g_Au.LockScDesignsForWrite();
	int result = sdd.ScDesignSave(design);
	g_Au.UnlockScDesigns();

	AuId personId;
	QString personName;
	if(result == 0 || result == 1)
	{
		DaPerson* person = LockAvatar();
		if(person)
		{
			// The scene should be subordinate to a design environment.
			personId = person->Id();
			personName = person->Name();
			DaDesigner* pde = person->FindParentDesignerInEnv();
			if(pde)
				pde->SetDesignModified(false);
		}
		UnlockAvatar();
		if(design.ArchitectId() != personId)
			return;
	}

	QString resultMsg;
	switch(result)
	{
	case 0:
		resultMsg = QObject::tr("New blueprint %1 %2 added to the library.")
			.arg(design.Id())
			.arg(design.Name());
		break;
	case 1:
		resultMsg = QObject::tr("Existing blueprint %1 %2 replaced in the library.")
			.arg(design.Id())
			.arg(design.Name());
		break;
	case 2:
		resultMsg = QObject::tr("Existing blueprint cannot be replaced in the library.");
		break;
	case 3:
		resultMsg = QObject::tr("Existing blueprint cannot be found in the library.");
		break;
	case 4:
		resultMsg = QObject::tr("New blueprint id %1 already exists in the library.").arg(design.Id());
		break;
	}

	if(!resultMsg.isEmpty())
	{
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::DesignerArchitectDeleteSc(bool isArchitect, quint32 designId)
{
	AuId personId;
	QString personName;
	Avatar(personId, personName);

	DgData& sdd = g_Au.LockScDesignsForWrite();
	int result;
	if(sdd.ScDesignArchitect(designId) == personId || isArchitect)
		result = sdd.ScDesignDelete(designId);
	else
		result = 3;
	g_Au.UnlockScDesigns();

	QString resultMsg;
	switch(result)
	{
	case 0:
		resultMsg = QObject::tr("Spacecraft blueprint %1 was not found to delete from the library.")
			.arg(designId);
		break;
	case 1:
		ScDesignExchangeUnpublish(isArchitect, designId, true);
		resultMsg = QObject::tr("Spacecraft blueprint %1 deleted from the library.")
			.arg(designId);
		break;
	case 2:
		ScDesignExchangeUnpublish(isArchitect, designId, true);
		resultMsg = QObject::tr("Spacecraft blueprint %1 deactivated in the library.")
			.arg(designId);
		break;
	case 3:
		resultMsg = QObject::tr("Spacecraft blueprint %1 belongs to another architect; you cannot delete it.")
			.arg(designId);
		break;
	}

	if(!resultMsg.isEmpty())
	{
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::ScDesignExchangeUnpublish(bool isArchitect, quint32 designId, bool deleting)
{
	AuId personId;
	QString personName;
	Avatar(personId, personName);

	int result;
	QString designName;
	DgData& sdi = g_Au.LockScExchangeForWrite();
	AuScDesignExchange::const_iterator ei = sdi.m_ScExchange->find(designId);
	if(ei == sdi.m_ScExchange->end())
		result = 3;
	else
	{
		designName = (*ei).DesignName();
		if((*ei).DesignArchitectId() != personId && !isArchitect)
			result = 2;
		else if(!sdi.ScExchangeRemove(designId))
			result = 1;
		else
			result = 0;
	}
	g_Au.UnlockScExchange();

	QString resultMsg;
	switch(result)
	{
	case 0:
		emit ScDesignExchangeUnpublished(designId);
		resultMsg = QObject::tr("Spacecraft blueprint %1 %2 unpublished.")
			.arg(designId)
			.arg(designName);
		break;
	case 1:
		if(!deleting)
			resultMsg = QObject::tr("Spacecraft blueprint %1 %2 was not published so it could not be unpublished.")
				.arg(designId)
				.arg(designName);
		else
			resultMsg = QObject::tr("Spacecraft blueprint %1 %2 was not published so it did not need to be unpublished.")
				.arg(designId)
				.arg(designName);
		break;
	case 2:
		resultMsg = QObject::tr("Spacecraft blueprint %1 %2 was published by another architect; you cannot unpublish it.")
			.arg(designId)
			.arg(designName);
		break;
	case 3:
		if(!deleting)
			resultMsg = QObject::tr("Spacecraft blueprint %1 was not published so it could not be unpublished.")
				.arg(designId);
		break;
	}

	if(!resultMsg.isEmpty())
	{
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::ScDesignExchangePublish(AuScDesignExchangeEntry& exchangeItem)
{
	quint32 designId = exchangeItem.DesignId();
	if(!designId)
		return;

	// Get publisher empire.
	quint32 empireId = 0;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		empireId = person->EmpireId();
	}
	UnlockAvatar();

	if(exchangeItem.DesignArchitectId() != personId)
		return;

	const DgData& sdd = g_Au.LockScDesignsForRead();
	bool hasDesign = sdd.ScDesignIsLoaded(designId);
	AuId archId = sdd.ScDesignArchitect(designId);
	if(!hasDesign)
		g_Au.ScDesignNeeded(designId);
	g_Au.UnlockScDesigns();

	int result;
	if(!hasDesign)
		result = 0;
	else if(archId != personId)
		result = 99;
	else
	{
		exchangeItem.SetPublisherEmpire(empireId);
		exchangeItem.SetWhenPublished();

		DgData& sdi = g_Au.LockScExchangeForWrite();
		result = sdi.ScExchangeAdd(exchangeItem);	// Returns 1=added nu entry, 2=replaced entry.
		g_Au.UnlockScExchange();
	}

	QString resultMsg;
	switch(result)
	{
	case 0:
		resultMsg = QObject::tr("Spacecraft blueprint %1 is not loaded. "
				"Allow a few moments for the server to load the design. "
				"Then try again. ")
			.arg(designId);
		break;
	case 1:
		emit ScDesignExchangePublished(exchangeItem);
		resultMsg = QObject::tr("Spacecraft blueprint %1 is now published.")
			.arg(designId);
		break;
	case 2:
		resultMsg = QObject::tr("Spacecraft blueprint %1 publishing information was updated.")
			.arg(designId);
		break;
	case 3:
		resultMsg = QObject::tr("Spacecraft blueprint with the same name and state is already published by you. "
				"Change the name or unpublish the old one.");
		break;
	case 99:
		resultMsg = QObject::tr("Spacecraft blueprint %1 belongs to another architect; you cannot publish it.")
			.arg(designId);
		break;
	}

	if(!resultMsg.isEmpty())
	{
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::DesignerArchitectFinalizeScAssembly(AuScDesign& design)
{
	AuId personId;
	QString personName;
	Avatar(personId, personName);
	if(design.ArchitectId() != personId || !IsNameValid(design.Name()))
		return;
	if(design.Thumbnail().width() > 256 || design.Thumbnail().height() > 256)
		return;

	AuCommThought msg(QObject::tr("Spacecraft assembly %1 received. Verifying...").arg(design.Name()), personId, personName);
	g_Au.SendAtomChat(msg);

	// Validate design to insure it really is ok to manufacture.
	QString resultMsg;
	QStringList msgs;
	if(!design.FinalizeAssembly(msgs))
	{
		resultMsg = QObject::tr("Unable to finalize spacecraft assembly %1 %2.")
			.arg(design.Id())
			.arg(design.Name());
	}
	else
	{
		// The client sent us the entire design.
		DgData& sdd = g_Au.LockScDesignsForWrite();
		int result = sdd.ScDesignSave(design);
		g_Au.UnlockScDesigns();

		switch(result)
		{
		case 0:
			resultMsg = QObject::tr("New spacecraft assembly %1 %2 added to the library. "
					"The blueprint could now be published in the exchange or stored to a device in your gear.")
				.arg(design.Id())
				.arg(design.Name());
			break;

		// The rest of the possible results aren't really possible when saving a nu design, with id of 0.
		case 1:
			resultMsg = QObject::tr("Existing spacecraft assembly %1 %2 replaced in the library.")
				.arg(design.Id())
				.arg(design.Name());
			break;
		case 2:
			resultMsg = QObject::tr("Existing spacecraft assembly cannot be replaced in the library.");
			break;
		case 3:
			resultMsg = QObject::tr("Existing spacecraft assembly cannot be found in the library.");
			break;
		case 4:
			resultMsg = QObject::tr("New spacecraft assembly id %1 already exists in the library.").arg(design.Id());
			break;
		}

		if(!resultMsg.isEmpty())
		{
			AuCommThought msg(resultMsg, personId, personName);
			g_Au.SendAtomChat(msg);
		}
	}
}

void AuAtomInterface::DesignerArchitectFinalizeScDesign(AuScDesign& design, quint32 designId/*=0*/)
{
	AuId personId;
	QString personName;
	Avatar(personId, personName);
	if(design.ArchitectId() != personId || !IsNameValid(design.Name()))
		return;
	if(design.Thumbnail().width() > 256 || design.Thumbnail().height() > 256)
		return;

	AuCommThought msg(QObject::tr("Spacecraft blueprint %1 received. Verifying...").arg(design.Name()), personId, personName);
	g_Au.SendAtomChat(msg);

	// Validate design to insure it really is ok to manufacture.
	QString resultMsg;
	AuScReport report;
	design.ReportDesign(report);
	QStringList msgs;
	if(!report.OkToManufacture(msgs, &design))
	{
		resultMsg = QObject::tr("Spacecraft blueprint %1 is not valid for manufacture.")
			.arg(design.Name());
	}
	else
	{
		// Finalize prepared the design to be saved as new, including setting the id to 0.
		Q_ASSERT(!design.Id());
		DgData& sdd = g_Au.LockScDesignsForWrite();
		int result = sdd.ScDesignSave(design, designId);
		g_Au.UnlockScDesigns();

		switch(result)
		{
		case 0:
			resultMsg = QObject::tr("New spacecraft blueprint %1 %2 added to the library. "
					"The blueprint could now be published in the exchange or stored to a device in your gear.")
				.arg(design.Id())
				.arg(design.Name());
			break;

		// The rest of the possible results aren't really possible when saving a nu design, with id of 0.
		case 1:
			resultMsg = QObject::tr("Existing spacecraft blueprint %1 %2 replaced in the library.")
				.arg(design.Id())
				.arg(design.Name());
			break;
		case 2:
			resultMsg = QObject::tr("Existing spacecraft blueprint cannot be replaced in the library.");
			break;
		case 3:
			resultMsg = QObject::tr("Existing spacecraft blueprint cannot be found in the library.");
			break;
		case 4:
			resultMsg = QObject::tr("New spacecraft blueprint id %1 already exists in the library.").arg(design.Id());
			break;
		}
	}

	if(!resultMsg.isEmpty())
	{
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::ScDesignExchangePurchase(quint32 designId, const AuGearPos& mediaPos)
{
	AuId personId;
	QString personName;
	Avatar(personId, personName);
	bool hasEx = false;
	AuScDesignExchangeEntry e;
	const DgData& sdi = g_Au.LockScExchangeForRead();
	AuScDesignExchange::const_iterator ei = sdi.m_ScExchange->find(designId);
	if(ei != sdi.m_ScExchange->cend())
	{
		e = *ei;
		hasEx = true;
	}
	g_Au.UnlockScExchange();

	QString resultMsg;
	if(hasEx)
	{
		DaPerson* person = LockAvatar();
		if(person)
		{
			// Check stance toward buyer.
			if(e.AccessibleTo(person->Id(), person->EmpireId()))
			{
				// Architect does not have to pay for their own designs.
				quint32 price;
				if(person->Is(e.DesignArchitectId()))
					price = 0;
				else
					price = e.Price();

				// Verify buyer funds.
				if(person->AccountBalance() >= price)
				{
					// Buyer can make the purchase if they have blank media.
					AuGear* pinv = person->LockGearForWrite();
					AuItem* mediaItem = pinv->ItemAt(mediaPos);
					if(mediaItem)
					{
						// The media item was found.
						if((mediaItem->CommodityId() == CID_BlankDisk || mediaItem->CommodityId() == CID_Paper))
						{
							// Blank media is either a disk or paper.
							AuItem* media = mediaItem->Clone();
							person->UnlockGear();

							// Take one from their gear.
							media->SetCount(1);
							person->ConsumeItem(*media);

							if(media->UseItem())					// Quality check the result.
							{
								// Replace the blank media with a design disk or drawing.
								AuItemScDesign* designItem;
								if(media->CommodityId() == CID_BlankDisk)
									designItem = new AuItemScDesign(e, CID_ScDesignDisk, media->Quality(),
											media->EmpireOfOrigin());
								else
									designItem = new AuItemScDesign(e, CID_ScDesignDrawing, media->Quality(),
											media->EmpireOfOrigin());

								// Put it wherever there's room.
								person->AddGearItem(*designItem);

								if(price)
								{
									// Design is not free.
									// Take money from the buyer.
									person->DebitAccount(price);

									// Send the money and a receipt to the seller.
									// They may not get the money if they are in a system transition.
									emit SendFunds(person->Id(), person->Name(), e.DesignArchitectId(), price,
											person->Galaxy(), 2, designId);

									if(media->CommodityId() == CID_BlankDisk)
										resultMsg = tr("%1 burned to disk successfully. "
												"Funds were transferred to the publisher.")
											.arg(designItem->ListName());
									else
										resultMsg = tr("%1 printed on paper successfully. "
												"Funds were transferred to the publisher.")
											.arg(designItem->ListName());
								}
								else
								{
									// Design was free.
									if(media->CommodityId() == CID_BlankDisk)
										resultMsg = tr("%1 burned to disk successfully at no cost.")
											.arg(designItem->ListName());
									else
										resultMsg = tr("%1 printed on paper successfully at no cost.")
											.arg(designItem->ListName());
								}
								delete designItem;
							}
							else
							{
								// Media failure.
								resultMsg = tr("The %1 was defective; it failed during the save. "
										"The purchase was not completed. "
										"I should try again with another blank disk or paper.").arg(media->SayName());
							}
							delete media;
						}
						else
						{
							AuItemStorageMedia* storageMedia = dynamic_cast<AuItemStorageMedia*>(mediaItem);
							if(storageMedia)
							{
								// Store the blueprint onto the media.
								int stores = storageMedia->StoreDesign(e);
								if(stores == 0 || stores == 3)
								{
									if(price)
									{
										// Take money from the buyer.
										person->DebitAccount(price);

										// Send the money and a receipt to the seller.
										// They may not get the money if they are in a system transition.
										emit SendFunds(person->Id(), person->Name(), e.DesignArchitectId(), price,
												person->Galaxy(), 2, designId);

										resultMsg = tr("%1 blueprint stored to %2.")
											.arg(e.DesignName())
											.arg(storageMedia->SayName());
										person->UpdateGearWeight();
										person->UnlockGear();
									}
									else
									{
										resultMsg = tr("%1 blueprint stored to %2 at no cost.")
											.arg(e.DesignName())
											.arg(storageMedia->SayName());
										person->UpdateGearWeight();
										person->UnlockGear();
									}
								}
								else
								{
									person->UnlockGear();
									if(stores == 1)
										resultMsg = tr("The storage media has no free space.");
									else if(stores == 2)
										resultMsg = tr("The storage media already contains the blueprint.");
									else if(stores == 4)
										resultMsg = tr("The storage media already contains a blueprint with the same name and architect.");
									else
										resultMsg = tr("The storage media reported an error.");
								}
							}
							else
							{
								person->UnlockGear();
								resultMsg = tr("I need a blank disk or paper or other storage media to store a copy of the "
										"blueprint.");
							}
						}
					}
					else
					{
						person->UnlockGear();
						// Blank media needed.
						resultMsg = tr("I need a blank disk or paper to store a copy of the blueprint. "
								"The purchase could not be completed.");
					}
				}
				else
				{
					// Insufficient funds.
					resultMsg = tr("I cannot afford the %L1%2 cost to buy the blueprint. "
							"The purchase could not be completed.")
						.arg(price)
						.arg(g_Au.CurrencySymbol());
				}
			}
			else
			{
				// Bad stance.
				resultMsg = tr("I cannot buy the blueprint because of the political stance of the publisher's empire toward "
						"my empire.");
			}
		}
		UnlockAvatar();
	}
	else
	{
		resultMsg = QObject::tr("Blueprint %1 was not found in the blueprint exchange so it could not be purchased.")
			.arg(designId);
	}

	if(!resultMsg.isEmpty())
	{
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

quint8 AuAtomInterface::DesignerGearOpen(quint32 designId, const AuGearPos& pos)
{
	quint8 designStudioMode = DaDesignBase::InvalidMode;	// Return value
	int result = 99;
	bool isSc = false;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
		{
			// Only the owner of the design studio can open a nu design.
			// Agent-level admin can open any design.
			if(pde->Owner() == person->Id() || person->HasAgentPower())
			{
				bool viewOnly = true;
				const AuGear* pinv = person->LockGearForRead();
				const AuItemBdDesign* bditem = dynamic_cast<const AuItemBdDesign*>(pinv->ItemAt(pos));
				const AuItemScDesign* scitem = dynamic_cast<const AuItemScDesign*>(pinv->ItemAt(pos));
				const AuItemStorageMedia* storage = dynamic_cast<const AuItemStorageMedia*>(pinv->ItemAt(pos));
				if(bditem)
				{
					if(AcctIsProgrammer(person->AccountTypeOnLine()))
					{
						designId = bditem->Exchange().DesignId();
						viewOnly = !bditem->Exchange().CanCopy();
						isSc = false;
						result = 0;
					}
					else if(bditem->Exchange().CanCopy() || bditem->Exchange().CanView())
					{
						bool stanceOk = bditem->Exchange().AccessibleTo(person->Id(), person->EmpireId());
						if(stanceOk)
						{
							designId = bditem->Exchange().DesignId();
							viewOnly = !bditem->Exchange().CanCopy();
							isSc = false;
							result = 0;
						}
						else
							result = 97;
					}
					else
						result = 96;
				}
				else if(scitem)
				{
					if(AcctIsProgrammer(person->AccountTypeOnLine()))
					{
						designId = scitem->Exchange().DesignId();
						viewOnly = !scitem->Exchange().CanCopy();
						isSc = true;
						result = 0;
					}
					else if(scitem->Exchange().CanCopy() || scitem->Exchange().CanView())
					{
						bool stanceOk = scitem->Exchange().AccessibleTo(person->Id(), person->EmpireId());
						if(stanceOk)
						{
							designId = scitem->Exchange().DesignId();
							viewOnly = !scitem->Exchange().CanCopy();
							isSc = true;
							result = 0;
						}
						else
							result = 97;
					}
					else
						result = 96;
				}
				else if(storage)
				{
					const AuBdDesignExchange& bpbd = storage->BlueprintsBd();
					AuBdDesignExchange::const_iterator bdi = bpbd.find(designId);
					if(bdi != bpbd.cend())
					{
						if((*bdi).CanCopy() || (*bdi).CanView())
						{
							bool stanceOk = (*bdi).AccessibleTo(person->Id(), person->EmpireId());
							if(stanceOk)
							{
								designId = (*bdi).DesignId();
								viewOnly = !(*bdi).CanCopy();
								isSc = false;
								result = 0;
							}
							else
								result = 97;
						}
						else
							result = 96;
					}
					else
					{
						const AuScDesignExchange& bpsc = storage->BlueprintsSc();
						AuScDesignExchange::const_iterator sci = bpsc.find(designId);
						if(sci != bpsc.cend())
						{
							if((*sci).CanCopy() || (*sci).CanView())
							{
								bool stanceOk = (*sci).AccessibleTo(person->Id(), person->EmpireId());
								if(stanceOk)
								{
									designId = (*sci).DesignId();
									viewOnly = !(*sci).CanCopy();
									isSc = true;
									result = 0;
								}
								else
									result = 97;
							}
							else
								result = 96;
						}
						else
							result = 94;
					}
				}
				else
					result = 95;
				person->UnlockGear();

				if(result == 0 && designId)
				{
					// This could change the design studio mode.
					if(isSc)
						result = pde->LoadScDesign(designId, personId, viewOnly);
					else
						result = pde->LoadBdDesign(designId, personId, viewOnly);
					designStudioMode = pde->DesignMode();
				}
			}
			else
				result = 98;
		}
	}
	UnlockAvatar();

	QString resultMsg;
	switch(result)
	{
	case 0:
		resultMsg = tr("Blueprint %1 opened successfully.").arg(designId);
		break;
	case 1:
		if(isSc)
			resultMsg = tr("Library is loading spacecraft blueprint %1. Try again shortly.").arg(designId);
		else
			resultMsg = tr("Library is loading building blueprint %1. Try again shortly.").arg(designId);
		break;
	case 2:
		resultMsg = tr("Blueprint %1 is broken.").arg(designId);
		break;
	case 94:
		resultMsg = tr("The gear item is not a blueprint and it does not contain a blueprint.");
		break;
	case 95:
		resultMsg = tr("The blueprint was not found in my gear.");
		break;
	case 96:
		resultMsg = tr("The blueprint publisher's terms prevent it from opening in my design studio.");
		break;
	case 97:
		resultMsg = tr("The blueprint publisher's terms make it inaccessible to my empire.");
		break;
	case 98:
		resultMsg = tr("I cannot open a blueprint; I am not the owner of this design studio.");
		break;
	case 99:
	default:
		resultMsg = tr("Blueprint %1 could not be opened.").arg(designId);
		break;
	}
	AuCommThought msg(resultMsg, personId, personName);
	g_Au.SendAtomChat(msg);
	return designStudioMode;
}

void AuAtomInterface::DesignerGearSaveSc(const AuScDesignExchangeEntry& e, const AuGearPos& pos)
{
	QString resultMsg;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		AuGear* pinv = person->LockGearForWrite();
		AuItem* mediaItem = pinv->ItemAt(pos);
		if(mediaItem)
		{
			// The media item was found.
			if((mediaItem->CommodityId() == CID_BlankDisk || mediaItem->CommodityId() == CID_Paper))
			{
				// Blank media is either a disk or paper.
				AuItem* media = mediaItem->Clone();
				person->UnlockGear();

				// Take one from their gear.
				media->SetCount(1);
				person->ConsumeItem(*media);

				if(media->UseItem())					// Quality check the result.
				{
					// Replace the blank media with a design disk or drawing.
					AuItemScDesign* designItem;
					if(media->CommodityId() == CID_BlankDisk)
						designItem = new AuItemScDesign(e, CID_ScDesignDisk, media->Quality(), media->EmpireOfOrigin());
					else
						designItem = new AuItemScDesign(e, CID_ScDesignDrawing, media->Quality(), media->EmpireOfOrigin());

					// Put it wherever there's room.
					person->AddGearItem(*designItem);

					if(media->CommodityId() == CID_BlankDisk)
						resultMsg = tr("%1 burned to disk successfully.").arg(designItem->ListName());
					else
						resultMsg = tr("%1 printed on paper successfully.").arg(designItem->ListName());

					delete designItem;
				}
				else
				{
					// Media failure.
					resultMsg = tr("The %1 was defective; it failed during the save. "
							"The operation was not completed. "
							"I should try again with another blank disk or paper.").arg(media->SayName());
				}
				delete media;
			}
			else
			{
				AuItemStorageMedia* storageMedia = dynamic_cast<AuItemStorageMedia*>(pinv->ItemAt(pos));
				if(storageMedia)
				{
					int stores = storageMedia->StoreDesign(e);
					if(stores == 0 || stores == 3)
					{
						resultMsg = tr("%1 blueprint stored to %2.").arg(e.DesignName()).arg(storageMedia->SayName());
						person->UpdateGearWeight();
						person->UnlockGear();
					}
					else
					{
						person->UnlockGear();
						if(stores == 1)
							resultMsg = tr("The storage media has no free space.");
						else if(stores == 2)
							resultMsg = tr("The storage media already contains the blueprint.");
						else if(stores == 4)
							resultMsg = tr("The storage media already contains a blueprint with the same name and architect.");
						else
							resultMsg = tr("The storage media reported an error.");
					}
				}
				else
				{
					person->UnlockGear();
					resultMsg = tr("I need a blank disk or paper or other storage media to store a copy of the blueprint.");
				}
			}
		}
		else
		{
			person->UnlockGear();
			resultMsg = tr("I need a blank disk or paper or other storage media to store a copy of the blueprint.");
		}
	}
	UnlockAvatar();

	if(!resultMsg.isEmpty())
	{
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

bool AuAtomInterface::ShipmentCreate(const AuShipment& shipment, const AuId& opBd)
{
	bool refreshClient = false;
	QString resultMsg;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();

		DaWorld* planet = person->FindParentPlanetInEnv();
		if(planet)
		{
			DaBd* bd = person->BdOp(opBd);
			if(bd)
			{
				if(bd->InChainOfCommand(person))
				{
					// The city must have a capitol building to be declared the capital.
					switch(bd->ShipmentCreate(shipment))
					{
					case 0:
						refreshClient = true;
						resultMsg = tr("Shipment created.");
						break;
					case 1:
						resultMsg = tr("Broker is missing.");
						break;
					case 2:
						resultMsg = tr("Broker is destroyed. The building must be repaired.");
						break;
					case 3:
						resultMsg = tr("Broker has no completed stories. How odd.");
						break;
					case 4:
						resultMsg = tr("Broker cannot handle any more shipments. "
								"It is limited by the number of offices.");
						break;
					case 5:
						resultMsg = tr("The building cannot handle any more shipments. "
								"It has an overall limit of 255 shipments in progress.");
						break;
					case 6:
						resultMsg = tr("A long haul cargo shipment must be destined for an outsystem city. "
								"That city is in this system. ");
						break;
					}
				}
				else
					resultMsg = tr("Only authorized government officials can create shipments.");
			}
		}
	}
	UnlockAvatar();

	// Notify the player of the result.
	if(!resultMsg.isEmpty())
	{
		// Send the result on the Log channel.
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
	return refreshClient;
}

bool AuAtomInterface::ShipmentDelete(const AuShipment& shipment, const AuId& opBd)
{
	if(shipment.TradeAgreement())
		return false;
	bool refreshClient = false;
	QString resultMsg;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		DaBd* bd = person->BdOp(opBd);
		if(bd)
		{
			if(bd->InChainOfCommand(person))
			{
				switch(bd->ShipmentDelete(shipment))
				{
				case 0:
					refreshClient = true;
					resultMsg = tr("Shipment deleted.");
					break;
				case 1:
					resultMsg = tr("Broker of the shipment is missing.");
					break;
				case 2:
					resultMsg = tr("Broker of the shipment is destroyed. The building must be repaired.");
					break;
				case 3:
					resultMsg = tr("Broker of the shipment has no completed stories. How odd.");
					break;
				case 4:
					resultMsg = tr("No record of the shipment was found. How odd.");
					break;
				}
			}
			else
				resultMsg = tr("Only authorized government officials can delete shipments.");
		}
	}
	UnlockAvatar();

	// Notify the player of the result.
	if(!resultMsg.isEmpty())
	{
		// Send the result on the Log channel.
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}

	return refreshClient;
}

void AuAtomInterface::KickCryoBerthOccupants(const AuSpecies& species, int count)
{
	quint32 countActuallyKicked = 0;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship)
		{
			// Person is aboard a spacecraft.
			if(ship->InChainOfCommand(person))
				countActuallyKicked = ship->KickCryoBerthOccupants(species, count);
		}
	}
	UnlockAvatar();

	QString resultMsg;
	if(countActuallyKicked)
	{
		QString specieName = species.NameOrDesc();
		if(countActuallyKicked == 1)
			resultMsg = tr("Kicked a %1 cryo traveller.").arg(specieName);
		else
			resultMsg = tr("Kicked %L1 %2 cryo travellers.").arg(countActuallyKicked).arg(specieName);
	}
	else
		resultMsg = tr("No cryo travellers were kicked.");
	AuCommThought msg(resultMsg, personId, personName);
	g_Au.SendAtomChat(msg);
}

void AuAtomInterface::ReviveCryoBerthOccupant(const AuSpecies& species, quint8 role)
{
	QString resultMsg;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship)
		{
			// Person is aboard a spacecraft.
			if(ship->InChainOfCommand(person))
			{
				// Insure a berth is available.
				bool berthAvailable;
				if(role == 1)
					berthAvailable = ship->CrewBerthsAvailable();
				else if(role == 2)
					berthAvailable = ship->TroopBerthsAvailable();
				else if(role == 3)
					berthAvailable = ship->PassengerBerthsAvailable();
				else
					berthAvailable = false;

				if(berthAvailable)
				{
					DaNpc* rev = ship->ReviveCryoBerthOccupant(person->Transform(), species, role);
					if(rev)
					{
						QString reviveSay[] =
						{
							tr("Was I in there a long time? I'm cold."),
							tr("Was I in there a long time? I'm hungry."),
							tr("Was I in there a long time? I'm thirsty."),
							tr("What? Where am I?"),
							tr("I will never touch that stuff again. I got to get to work. Wait. What?"),
							tr("The last thing I remember was jogging in the park. Then there was this bright light."),
							tr("I was riding my bike, then WHAM! Now this."),
							tr("Is this a UFO? Was I abducted? You didn't take any of my organs did you?"),
							tr("Is this a UFO? Was I abducted? Are you going to do filthy things to me?"),
							tr("Is this a UFO? Was I abducted? You aren't going to poke things into my navel are you?"),
							tr("Is this a UFO? Was I abducted? You aren't going to poke things into my butt are you?"),
							tr("That is definitely the way to travel. Can I do it again?"),
							tr("You could have asked me first. I might have come along willingly."),
							tr("Nobody ever said space travel would be like that."),
							tr("Somebody told me to get really drunk before going into the cryo unit. Never do that."),
							tr("Wow I feel fantastic! Do we get food now?"),
							tr("Is this some kind of trick. Where am I?"),
							tr("This is crap. I demand to speak to the captain."),
							tr("That's a disappointment. I thought I was being frozen aboard a good ship."),
							tr("Ew what is that smell?"),
							tr("Rats got into my cryo unit somehow. They chewed a big piece of meat off my frozen body."),
							tr("Hey my sandwich is still good, just a little freezer burn."),
							tr("I need some freezer burn ointment. Some of my flesh was exposed while in the cryo unit."),
							tr("Sir, I am ready for work, just feeling a bit groggy."),
							tr("This can't be happening. Please tell me I'm hallucinating."),
							tr("Sir, I am ready to go. Freeze me any time."),
							tr("I need a cigarette. Do you have any smokes?"),
							tr("Is that safe? I'm not feeling so well."),
							tr("Great! Frozen, naked, no equipment! Do you suppose you could hook me up, doc? Pronto."),
							tr("Oh! I'm naked. This is awkward. Do you have any clothing?"),
							tr("Oh! I'm naked. Keep your hands to yourself, whatever you are."),
							tr("That was a drag, Jones. Do you have any fresh herb?"),
							tr("What year is it?"),
							tr("Am I dead? Is this real?"),
							tr("I demand you put me back in there and send me back where I came from."),
							tr("Do you clean these things between occupants? It smells aweful in there."),
							tr("I want a pillow next time."),
							tr("They didn't strap me down well enough. I have bruises from bouncing around the cryo unit."),
							tr("If you are claustrophobic, never get inside one of those. My face was frozen in a scream."),
							tr("The training video makes it look so refreshing, like taking a shower."),
							tr("The guys that run these things always look like morticians."),
							tr("Do I get breakfast now? Somebody said breakfast was part of the deal."),
							tr("What ship is this? Noooo!"),
							tr("What ship is this? Yes!"),
							tr("Are they still after me? Will they be able to find me here?"),
							tr("That will teach that conniver. No divorce lawyers will find me out here."),
							tr("Today is a new day. What work do you have for me, boss?"),
							tr("This is a nice ship. Are you looking for a partner?"),
						};
						int sayCount = sizeof(reviveSay)/sizeof(QString);
						int sayWhich = g_Au.Dice().Roll(1,sayCount,-1);
						AuCommChat msg(reviveSay[sayWhich], AuCommStdChannel::VoiceChannelName(), rev->Id(), rev->Name(),
								rev->FindParentSector()->Id(), rev->Galaxy(), AuCommStdChannel::Close,
								AuCommStdChannel::Audible);
						rev->Say(msg);
						resultMsg = tr("%1 was revived from a cryo berth.").arg(rev->SpotDescription(ship->EmpireId()));
					}
					else
						resultMsg = tr("No traveller with that DNA was found in a cryo berth.");
				}
				else
				{
					if(role == 1)
						resultMsg = tr("No crew berth is available.");
					else if(role == 2)
						resultMsg = tr("No troop berth is available.");
					else if(role == 3)
						resultMsg = tr("No passenger berth is available.");
					else
						resultMsg = tr("No berth is available.");
				}
			}
		}
	}
	UnlockAvatar();
	if(!resultMsg.isEmpty())
	{
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::DesignerArchitectDeleteBd(bool isArchitect, quint32 designId)
{
	AuId personId;
	QString personName;
	Avatar(personId, personName);

	DgData& sdd = g_Au.LockBdDesignsForWrite();
	int result;
	if(sdd.BdDesignArchitect(designId) == personId || isArchitect)
		result = sdd.BdDesignDelete(designId);
	else
		result = 3;
	g_Au.UnlockBdDesigns();

	QString resultMsg;
	switch(result)
	{
	case 0:
		resultMsg = QObject::tr("Building blueprint %1 was not found to delete from the library.")
			.arg(designId);
		break;
	case 1:
		BdDesignExchangeUnpublish(isArchitect, designId, true);
		resultMsg = QObject::tr("Building blueprint %1 deleted from the library.")
			.arg(designId);
		break;
	case 2:
		BdDesignExchangeUnpublish(isArchitect, designId, true);
		resultMsg = QObject::tr("Building blueprint %1 deactivated in the library.")
			.arg(designId);
		break;
	case 3:
		resultMsg = QObject::tr("Building blueprint %1 belongs to another architect; you cannot delete it.")
			.arg(designId);
		break;
	}

	if(!resultMsg.isEmpty())
	{
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::BdDesignExchangeUnpublish(bool isArchitect, quint32 designId, bool deleting)
{
	AuId personId;
	QString personName;
	Avatar(personId, personName);

	int result;
	QString designName;
	DgData& sdi = g_Au.LockBdExchangeForWrite();
	AuBdDesignExchange::const_iterator ei = sdi.m_BdExchange->find(designId);
	if(ei == sdi.m_BdExchange->end())
		result = 3;
	else
	{
		designName = (*ei).DesignName();
		if((*ei).DesignArchitectId() != personId && !isArchitect)
			result = 2;
		else if(!sdi.BdExchangeRemove(designId))
			result = 1;
		else
			result = 0;
	}
	g_Au.UnlockBdExchange();

	QString resultMsg;
	switch(result)
	{
	case 0:
		emit BdDesignExchangeUnpublished(designId);
		resultMsg = QObject::tr("Building blueprint %1 %2 unpublished.")
			.arg(designId)
			.arg(designName);
		break;
	case 1:
		if(!deleting)
			resultMsg = QObject::tr("Building blueprint %1 %2 was not published so it could not be unpublished.")
				.arg(designId)
				.arg(designName);
		else
			resultMsg = QObject::tr("Building blueprint %1 %2 was not published so it did not need to be unpublished.")
				.arg(designId)
				.arg(designName);
		break;
	case 2:
		resultMsg = QObject::tr("Building blueprint %1 %2 was published by another architect; you cannot unpublish it.")
			.arg(designId)
			.arg(designName);
		break;
	case 3:
		if(!deleting)
			resultMsg = QObject::tr("Building blueprint %1 was not published so it could not be unpublished.")
				.arg(designId);
		break;
	}

	if(!resultMsg.isEmpty())
	{
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::DesignerGearSaveBd(const AuBdDesignExchangeEntry& e, const AuGearPos& pos)
{
	QString resultMsg;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		AuGear* pinv = person->LockGearForWrite();
		AuItem* mediaItem = pinv->ItemAt(pos);
		if(mediaItem)
		{
			// The media item was found.
			if((mediaItem->CommodityId() == CID_BlankDisk || mediaItem->CommodityId() == CID_Paper))
			{
				// Blank media is either a disk or paper.
				AuItem* media = mediaItem->Clone();
				person->UnlockGear();

				// Take one from their gear.
				media->SetCount(1);
				person->ConsumeItem(*media);

				if(media->UseItem())					// Quality check the result.
				{
					// Replace the blank media with a design disk or drawing.
					AuItemBdDesign* designItem;
					if(media->CommodityId() == CID_BlankDisk)
						designItem = new AuItemBdDesign(e, CID_BdDesignDisk, media->Quality(), media->EmpireOfOrigin());
					else
						designItem = new AuItemBdDesign(e, CID_BdDesignDrawing, media->Quality(), media->EmpireOfOrigin());

					// Put it wherever there's room.
					person->AddGearItem(*designItem);

					if(media->CommodityId() == CID_BlankDisk)
						resultMsg = tr("%1 burned to disk successfully.").arg(designItem->ListName());
					else
						resultMsg = tr("%1 printed on paper successfully.").arg(designItem->ListName());

					delete designItem;
				}
				else
				{
					// Media failure.
					resultMsg = tr("The %1 was defective; it failed during the save. "
							"The operation was not completed. "
							"I should try again with another blank disk or paper.").arg(media->SayName());
				}
				delete media;
			}
			else
			{
				AuItemStorageMedia* storageMedia = dynamic_cast<AuItemStorageMedia*>(pinv->ItemAt(pos));
				if(storageMedia)
				{
					int stores = storageMedia->StoreDesign(e);
					if(stores == 0 || stores == 3)
					{
						resultMsg = tr("%1 blueprint stored to %2.").arg(e.DesignName()).arg(storageMedia->SayName());
						person->UpdateGearWeight();
						person->UnlockGear();
					}
					else
					{
						person->UnlockGear();
						if(stores == 1)
							resultMsg = tr("The storage media has no free space.");
						else if(stores == 2)
							resultMsg = tr("The storage media already contains the blueprint.");
						else if(stores == 4)
							resultMsg = tr("The storage media already contains a blueprint with the same name and architect.");
						else
							resultMsg = tr("The storage media reported an error.");
					}
				}
				else
				{
					person->UnlockGear();
					resultMsg = tr("I need a blank disk or paper or other storage media to store a copy of the blueprint.");
				}
			}
		}
		else
		{
			person->UnlockGear();
			resultMsg = tr("I need a blank disk or paper or other storage media to store a copy of the blueprint.");
		}
	}
	UnlockAvatar();

	if(!resultMsg.isEmpty())
	{
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::DesignerArchitectSaveBd(AuBdDesign& design)
{
	if(!IsNameValid(design.Name()))
		return;
	if(design.Thumbnail().width() > 256 || design.Thumbnail().height() > 256)
		return;

	// New designs should always start in Edit mode.
	if(!design.Id())
		design.SetState(AuDesign::Edit);

	// The client sent us the entire design, potentially finalized.
	DgData& sdd = g_Au.LockBdDesignsForWrite();
	int result = sdd.BdDesignSave(design);
	g_Au.UnlockBdDesigns();

	AuId personId;
	QString personName;
	if(result == 0 || result == 1)
	{
		DaPerson* person = LockAvatar();
		if(person)
		{
			// The scene should be subordinate to a design environment.
			personId = person->Id();
			personName = person->Name();
			DaDesigner* pde = person->FindParentDesignerInEnv();
			if(pde)
				pde->SetDesignModified(false);
		}
		UnlockAvatar();
		if(design.ArchitectId() != personId)
			return;
	}

	QString resultMsg;
	switch(result)
	{
	case 0:
		resultMsg = QObject::tr("New blueprint %1 %2 added to the library.")
			.arg(design.Id())
			.arg(design.Name());
		break;
	case 1:
		resultMsg = QObject::tr("Existing blueprint %1 %2 replaced in the library.")
			.arg(design.Id())
			.arg(design.Name());
		break;
	case 2:
		resultMsg = QObject::tr("Existing blueprint cannot be replaced in the library.");
		break;
	case 3:
		resultMsg = QObject::tr("Existing blueprint cannot be found in the library.");
		break;
	case 4:
		resultMsg = QObject::tr("New blueprint id %1 already exists in the library.").arg(design.Id());
		break;
	}

	if(!resultMsg.isEmpty())
	{
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::DesignerArchitectFinalizeBdAssembly(AuBdDesign& design, QSet<quint32>& xrefs)
{
	AuId personId;
	QString personName;
	Avatar(personId, personName);
	if(design.ArchitectId() != personId || !IsNameValid(design.Name()))
		return;
	if(design.Thumbnail().width() > 256 || design.Thumbnail().height() > 256)
		return;

	AuCommThought msg(QObject::tr("Building assembly %1 received. Verifying...").arg(design.Name()), personId, personName);
	g_Au.SendAtomChat(msg);

	// Validate design to insure it really is ok to manufacture.
	QString resultMsg;
	QStringList msgs;
	if(!design.FinalizeAssembly(msgs))
	{
		resultMsg = QObject::tr("Unable to finalize building assembly %1 %2.")
			.arg(design.Id())
			.arg(design.Name());
	}
	else
	{
		// The client sent us the entire design.
		DgData& sdd = g_Au.LockBdDesignsForWrite();
		int result = sdd.BdDesignSave(design);
		g_Au.UnlockBdDesigns();

		switch(result)
		{
		case 0:
			{
				design.ExternalReferences(xrefs);
				resultMsg = QObject::tr("New building assembly %1 %2 added to the library. "
						"The blueprint could now be published in the exchange or stored to a device in your gear.")
					.arg(design.Id())
					.arg(design.Name());
			}
			break;

		// The rest of the possible results aren't really possible when saving a nu design, with id of 0.
		case 1:
			{
				design.ExternalReferences(xrefs);
				resultMsg = QObject::tr("Existing building assembly %1 %2 replaced in the library.")
					.arg(design.Id())
					.arg(design.Name());
			}
			break;
		case 2:
			resultMsg = QObject::tr("Existing building assembly cannot be replaced in the library.");
			break;
		case 3:
			resultMsg = QObject::tr("Existing building assembly cannot be found in the library.");
			break;
		case 4:
			resultMsg = QObject::tr("New building assembly id %1 already exists in the library.").arg(design.Id());
			break;
		}

		if(!resultMsg.isEmpty())
		{
			AuCommThought msg(resultMsg, personId, personName);
			g_Au.SendAtomChat(msg);
		}
	}
}

void AuAtomInterface::DesignerArchitectFinalizeBdDesign(AuBdDesign& design, QSet<quint32>& xrefs, quint32 designId/*=0*/)
{
	AuId personId;
	QString personName;
	Avatar(personId, personName);
	if(design.ArchitectId() != personId || !IsNameValid(design.Name()))
		return;
	if(design.Thumbnail().width() > 256 || design.Thumbnail().height() > 256)
		return;

	AuCommThought msg(QObject::tr("Building blueprint %1 received. Verifying...").arg(design.Name()), personId, personName);
	g_Au.SendAtomChat(msg);

	// Validate design to insure it really is ok to manufacture.
	QString resultMsg;
	AuBdReport report;
	design.ReportDesign(report);
	QStringList msgs;
	if(!report.OkToConstruct(msgs, &design))
	{
		resultMsg = QObject::tr("Building blueprint %1 is not valid for construction.")
			.arg(design.Name());
	}
	else
	{
		// Finalize prepared the design to be saved as new, including setting the id to 0.
		Q_ASSERT(!design.Id());
		DgData& sdd = g_Au.LockBdDesignsForWrite();
		int result = sdd.BdDesignSave(design, designId);
		g_Au.UnlockBdDesigns();

		switch(result)
		{
		case 0:
			{
				design.ExternalReferences(xrefs);
				resultMsg = QObject::tr("New building blueprint %1 %2 added to the library. "
						"The blueprint could now be published in the exchange or stored to a device in your gear.")
					.arg(design.Id())
					.arg(design.Name());
			}
			break;

		// The rest of the possible results aren't really possible when saving a nu design, with id of 0.
		case 1:
			{
				design.ExternalReferences(xrefs);
				resultMsg = QObject::tr("Existing building blueprint %1 %2 replaced in the library.")
					.arg(design.Id())
					.arg(design.Name());
			}
			break;
		case 2:
			resultMsg = QObject::tr("Existing building blueprint cannot be replaced in the library.");
			break;
		case 3:
			resultMsg = QObject::tr("Existing building blueprint cannot be found in the library.");
			break;
		case 4:
			resultMsg = QObject::tr("New building blueprint id %1 already exists in the library.").arg(design.Id());
			break;
		}
	}

	if(!resultMsg.isEmpty())
	{
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::BdDesignExchangePublish(AuBdDesignExchangeEntry& exchangeItem)
{
	AuId personId;
	QString personName;
	Avatar(personId, personName);
	quint32 designId = exchangeItem.DesignId();
	if(!designId)
		return;

	if(exchangeItem.DesignArchitectId() != personId)
		return;

	// Get publisher empire.
	quint32 empireId = 0;
	DaPerson* person = LockAvatar();
	if(person)
		empireId = person->EmpireId();
	UnlockAvatar();

	const DgData& sdd = g_Au.LockBdDesignsForRead();
	bool hasDesign = sdd.BdDesignIsLoaded(designId);
	AuId archId = sdd.BdDesignArchitect(designId);
	if(!hasDesign)
		g_Au.BdDesignNeeded(designId);
	g_Au.UnlockBdDesigns();

	int result;
	if(!hasDesign)
		result = 0;
	else if(archId != personId)
		result = 99;
	else
	{
		exchangeItem.SetPublisherEmpire(empireId);
		exchangeItem.SetWhenPublished();

		DgData& sdi = g_Au.LockBdExchangeForWrite();
		result = sdi.BdExchangeAdd(exchangeItem);	// Returns 1=added nu entry, 2=replaced entry.
		g_Au.UnlockBdExchange();
	}

	QString resultMsg;
	switch(result)
	{
	case 0:
		resultMsg = QObject::tr("Building blueprint %1 is not loaded. "
				"Allow a few moments for the server to load the design. "
				"Then try again. ")
			.arg(designId);
		break;
	case 1:
		emit BdDesignExchangePublished(exchangeItem);
		resultMsg = QObject::tr("Building blueprint %1 is now published.")
			.arg(designId);
		break;
	case 2:
		resultMsg = QObject::tr("Building blueprint %1 publishing information was updated.")
			.arg(designId);
		break;
	case 3:
		resultMsg = QObject::tr("Building blueprint with the same name and state is already published by you. "
				"Change the name or unpublish the old one.");
		break;
	case 99:
		resultMsg = QObject::tr("Building blueprint %1 belongs to another architect; you cannot publish it.")
			.arg(designId);
		break;
	}

	if(!resultMsg.isEmpty())
	{
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::BdDesignExchangePurchase(quint32 designId, const AuGearPos& mediaPos)
{
	AuId personId;
	QString personName;
	Avatar(personId, personName);
	bool hasEx = false;
	AuBdDesignExchangeEntry e;
	const DgData& sdi = g_Au.LockBdExchangeForRead();
	AuBdDesignExchange::const_iterator ei = sdi.m_BdExchange->find(designId);
	if(ei != sdi.m_BdExchange->cend())
	{
		e = *ei;
		hasEx = true;
	}
	g_Au.UnlockBdExchange();

	QString resultMsg;
	if(hasEx)
	{
		DaPerson* person = LockAvatar();
		if(person)
		{
			// Check stance toward buyer.
			if(e.AccessibleTo(person->Id(), person->EmpireId()))
			{
				// Architect does not have to pay for their own designs.
				quint32 price;
				if(person->Is(e.DesignArchitectId()))
					price = 0;
				else
					price = e.Price();

				// Verify buyer funds.
				if(person->AccountBalance() >= price)
				{
					// Buyer can make the purchase if they have blank media.
					AuGear* pinv = person->LockGearForWrite();
					AuItem* mediaItem = pinv->ItemAt(mediaPos);
					if(mediaItem)
					{
						// The media item was found.
						if((mediaItem->CommodityId() == CID_BlankDisk || mediaItem->CommodityId() == CID_Paper))
						{
							// Blank media is either a disk or paper.
							AuItem* media = mediaItem->Clone();
							person->UnlockGear();

							// Take one from their gear.
							media->SetCount(1);
							person->ConsumeItem(*media);

							if(media->UseItem())					// Quality check the result.
							{
								// Replace the blank media with a design disk or drawing.
								AuItemBdDesign* designItem;
								if(media->CommodityId() == CID_BlankDisk)
									designItem = new AuItemBdDesign(e, CID_BdDesignDisk, media->Quality(),
											media->EmpireOfOrigin());
								else
									designItem = new AuItemBdDesign(e, CID_BdDesignDrawing, media->Quality(),
											media->EmpireOfOrigin());

								// Put it wherever there's room.
								person->AddGearItem(*designItem);

								if(price)
								{
									// Design is not free.
									// Take money from the buyer.
									person->DebitAccount(price);

									// Send the money and a receipt to the seller.
									// They may not get the money if they are in a system transition.
									emit SendFunds(person->Id(), person->Name(), e.DesignArchitectId(), price,
											person->Galaxy(), 2, designId);

									if(media->CommodityId() == CID_BlankDisk)
										resultMsg = tr("%1 burned to disk successfully. "
												"Funds were transferred to the publisher.")
											.arg(designItem->ListName());
									else
										resultMsg = tr("%1 printed on paper successfully. "
												"Funds were transferred to the publisher.")
											.arg(designItem->ListName());
								}
								else
								{
									// Design was free.
									if(media->CommodityId() == CID_BlankDisk)
										resultMsg = tr("%1 burned to disk successfully at no cost.")
											.arg(designItem->ListName());
									else
										resultMsg = tr("%1 printed on paper successfully at no cost.")
											.arg(designItem->ListName());
								}
								delete designItem;
							}
							else
							{
								// Media failure.
								resultMsg = tr("The %1 was defective; it failed during the save. "
										"The purchase was not completed. "
										"I should try again with another blank disk or paper.").arg(media->SayName());
							}
							delete media;
						}
						else
						{
							AuItemStorageMedia* storageMedia = dynamic_cast<AuItemStorageMedia*>(mediaItem);
							if(storageMedia)
							{
								// Store the blueprint onto the media.
								int stores = storageMedia->StoreDesign(e);
								if(stores == 0 || stores == 3)
								{
									if(price)
									{
										// Take money from the buyer.
										person->DebitAccount(price);

										// Send the money and a receipt to the seller.
										// They may not get the money if they are in a system transition.
										emit SendFunds(person->Id(), person->Name(), e.DesignArchitectId(), price,
												person->Galaxy(), 2, designId);

										resultMsg = tr("%1 blueprint stored to %2.")
											.arg(e.DesignName())
											.arg(storageMedia->SayName());
										person->UpdateGearWeight();
										person->UnlockGear();
									}
									else
									{
										resultMsg = tr("%1 blueprint stored to %2 at no cost.")
											.arg(e.DesignName())
											.arg(storageMedia->SayName());
										person->UpdateGearWeight();
										person->UnlockGear();
									}
								}
								else
								{
									person->UnlockGear();
									if(stores == 1)
										resultMsg = tr("The storage media has no free space.");
									else if(stores == 2)
										resultMsg = tr("The storage media already contains the blueprint.");
									else if(stores == 4)
										resultMsg = tr("The storage media already contains a blueprint with the same name and architect.");
									else
										resultMsg = tr("The storage media reported an error.");
								}
							}
							else
							{
								person->UnlockGear();
								resultMsg = tr("I need a blank disk or paper or other storage media to store a copy of the "
										"blueprint.");
							}
						}
					}
					else
					{
						person->UnlockGear();
						// Blank media needed.
						resultMsg = tr("I need a blank disk or paper to store a copy of the blueprint. "
								"The purchase could not be completed.");
					}
				}
				else
				{
					// Insufficient funds.
					resultMsg = tr("I cannot afford the %L1%2 cost to buy the blueprint. "
							"The purchase could not be completed.")
						.arg(price)
						.arg(g_Au.CurrencySymbol());
				}
			}
			else
			{
				// Bad stance.
				resultMsg = tr("I cannot buy the blueprint because of the political stance of the publisher's empire toward "
						"my empire.");
			}
		}
		UnlockAvatar();
	}
	else
	{
		resultMsg = QObject::tr("Blueprint %1 was not found in the blueprint exchange so it could not be purchased.")
			.arg(designId);
	}

	if(!resultMsg.isEmpty())
	{
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::BdFoundCity(INDUSTRYID industryId, quint32 designId, COMMODITYID material, const AuPoint& buildLoc,
        float zRotation, const AuId& onPlatform, quint16 mpid, bool workerOk, const QString& cityName,
        const QString& solarSystemName, const QString& sectorNameGiven)
{
	if(!IsNameValid(cityName))
		return;
	if(!solarSystemName.isEmpty() && !IsNameValid(solarSystemName))
		return;
	if(!sectorNameGiven.isEmpty() && !IsNameValid(sectorNameGiven))
		return;
	CITYTYPE cityType;
	if(industryId == IID_Capitol)
		cityType = CITY_Capitol;
	else if(industryId == IID_BaseHeadquarters)
		cityType = CITY_MilitaryBase;
	else
		return;
	// Info about the nu city.
	bool cityCreated = false;
	quint32 idEmpire = 0;
	AuId cityId;
	AuId founderId;
	QString founderName;
	AuId planetId;
	QString planetName;
	AuId systemId;
	QString systemName;
	AuPoint systemLoc;
	AuId sectorId;
	QString sectorName;
	AuId galaxyId;
	GALAXY galaxyEnum = GALAXY_Count;

	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	if(person && system)
	{
		systemId = system->Id();
		DaWorldLL* world = person->FindParentWorldInEnv();
		if(world)
		{
			idEmpire = person->EmpireId();

			// The person must be in the chain-of-command of the planet to raise a flag there.
			AuIdList placeChain;
			world->ChainOfPlaces(placeChain);
			const DgData& gd = g_Au.LockDataForRead();
			bool inChainOfCommand = gd.IsEmpirePlaceInDomain(idEmpire, placeChain, person->Id());
			g_Au.UnlockData();

			bool claimedByMe = world->ClaimedByPersonId() == person->Id();

			QString resultMsg;
			if(inChainOfCommand || claimedByMe)
			{
				if(world->CityHereNamed(cityName))
				{
					if(cityType == CITY_Capitol)
						resultMsg = tr("City not established. This world already hosts a city named %1").arg(cityName);
					else
						resultMsg = tr("Base not established. This world already hosts a city named %1").arg(cityName);
				}
				else if(world->HQHereNamed(cityName))
				{
					if(cityType == CITY_Capitol)
						resultMsg = tr("City not established. This world already hosts a base named %1").arg(cityName);
					else
						resultMsg = tr("Base not established. This world already hosts a base named %1").arg(cityName);
				}
				else
				{
					// Reward player with reputation points.
					if(!world->BdHere() || !world->HasWorkerSpecies())
					{
						// Points awarded if this is the first city on the world.
						if(world->HasBreathableAtmosphere())
							person->Reward(0, REW_ColonizeHabitableWorld);
						else
							person->Reward(0, REW_ColonizeUninhabitableWorld);
					}

					DaBd* bd = person->BdBuild(industryId, designId, material, buildLoc, zRotation, onPlatform, mpid, workerOk);
					if(bd)
					{
						cityCreated = true;
						bd->SetCapitol(bd->Id(), idEmpire);
						bd->SetCityName(cityName);

						// Set worker dna in this zone.
						AuGlobeLLF devLL(bd->GlobeLL());	// Location of town square on the globe, when on a globe world.
						if(!world->HasWorkerSpecies())
						{
							world->SetWorkerSpecies(person->Species());
							world->SetModified();
						}

						// Get the name and id of the solar system and sector.
						// The solar system name is changed here because I am the responsible worker server.
						// The sector name is not changed here. That is done by my manager server, who is responsible for the
						// sector.
						// Get name and id from parent solar system. This worker server updates it if it's blank.
						// A blank name is passed in the CityCreated signal if it hasn't been named yet. This triggers
						// propagation of the name change by my manager when it receives the CityCreated packet.
						if(!system->HasName())
						{
							// Player Statistics
							person->Stats().NamedSystem();
							person->IncreaseSkill(AuSkill::SpaceExploration, 1);

							if(!solarSystemName.isEmpty())
							{
								// We were given a solar system name.
								system->SetNameIncludePlanets(solarSystemName, true);
							}
							else
							{
								// No solar system name was given so use the city name.
								system->SetNameIncludePlanets(cityName, true);
							}
						}

						// Solar system is considered to be explored by the builder of a city. I guess their populace looks up
						// once in a while.
						system->ExploredBy(idEmpire);

						// Get name and id from parent sector. Name may be blank. Manager server will update it.
						const DaSector* sector = system->FindParentSector();
						Q_ASSERT(sector);
						const DaGalaxy* galaxy = sector->FindParentGalaxy();
						Q_ASSERT(galaxy);
						if(!sector->HasName())
						{
							// Player Statistics
							person->Stats().NamedSector();
							person->IncreaseSkill(AuSkill::SpaceExploration, 1);

							// Clean up the sector name.
							QString sectorNameClean(sectorNameGiven);
							while(sectorNameClean.right(7) == " Sector")
								sectorNameClean = sectorNameClean.left(sectorNameClean.length() - 7);	// Drop ' Sector'.
							sectorNameClean = sectorNameClean.trimmed();
							if(sectorNameClean == "Sector")
								sectorNameClean = "";				// Don't permit sector named only 'Sector'.

							if(!sectorNameClean.isEmpty())
								sectorName = sectorNameClean;
							else if(!solarSystemName.isEmpty())
								sectorName = solarSystemName;
							else
								sectorName = cityName;
						}
						else
							sectorName = sector->SectorName();		// Name without the trailing 'Sector' suffix.

						// Grab final ids and names. The names may have been changed by building the city.
						cityId = bd->Id();
						founderId = person->Id();
						founderName = person->Name();
						planetId = world->Id();
						planetName = world->Name();					// Was set if the system did not already have a name.
						systemName = system->Name();				// Was set if the system did not already have a name.
						system->GetLocationRelativeToParent(systemLoc, galaxy);
						sectorId = sector->Id();
						galaxyId = galaxy->Id();
						galaxyEnum = galaxy->Galaxy();
						if(g_Au.m_LogMessages & LOG_Construction)
							qDebug() << "AuAtomInterface::BdFoundCity CityCreated empire" << idEmpire
								<< "city" << cityId << cityName << cityType
								<< "founder" << founderId << founderName
								<< "planet" << planetId << planetName
								<< "system" << systemId << systemName << systemLoc
								<< "sector" << sectorId << sectorName
								<< "galaxy" << galaxyId << GalaxyName(galaxyEnum);

						// Remember the cities that I found right away.
						person->Remember(AuMemCity(bd->Id(), bd->CityName(), devLL, planetId, planetName, system->Id(),
									systemName, sectorId, sectorName, galaxyEnum));

						if(g_Au.m_LogMessages & LOG_Construction)
						{
							QString logmsg;
							logmsg = tr("Unit %1 %2 Established %2 (%3) city.")
									.arg(person->Id().GetID()).arg(person->Name())
									.arg(cityName).arg(cityId.GetID());
							g_Au.Log(logmsg);
						}
					}
				}
			}
			else
			{
				// Tell them why the city could not be built.
				if(cityType == CITY_Capitol)
					resultMsg = tr("City not established. I am not in command of this world and it is not claimed by me. "
							"To found a city here, the world must be claimed by me or it must be in my governance domain.");
				else
					resultMsg = tr("Base not established. I am not in command of this world and it is not claimed by me. "
							"To found a base here, the world must be claimed by me or it must be in my governance domain.");
			}
			if(!resultMsg.isEmpty())
			{
				AuCommThought msg(resultMsg, person->Id(), person->Name());
				g_Au.SendAtomChat(msg);
			}
		}
	}
	UnlockAvatar();

	if(cityCreated)
		emit CityCreated(idEmpire,
				cityType, cityId, cityName,
				founderId, founderName,
				planetId, planetName,
				systemId, systemName, systemLoc,
				sectorId, sectorName,
				galaxyId, (quint8)galaxyEnum);
}

void AuAtomInterface::BdBuild(INDUSTRYID industryId, quint32 designId, COMMODITYID material, const AuPoint& buildLoc,
		float zRotation, const AuId& onPlatform, quint16 mpid, bool workerOk)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdBuild(industryId, designId, material, buildLoc, zRotation, onPlatform, mpid, workerOk);
	UnlockAvatar();
}

void AuAtomInterface::BdBuildStorage(INDUSTRYID industryId, quint32 designId, COMMODITYID material, const AuPoint& buildLoc,
		float zRotation, const AuId& onPlatform, quint16 mpid, bool workerOk, COMMODITYID storeCID)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdBuildStorage(industryId, designId, material, buildLoc, zRotation, onPlatform, mpid, workerOk, storeCID);
	UnlockAvatar();
}

void AuAtomInterface::BdConstructionEnabled(bool enabled, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdConstructionEnable(enabled, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdConstructionFetch(const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdConstructionFetch(opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdConstructionGo(const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdConstructionGo(opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureSpacecraftCitizenEnabled(quint32 enabled, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureSpacecraftEnableWorker(enabled, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureCitizenEnabledMPID(quint16 mpid, quint32 enabled, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureEnabledMPID(mpid, enabled, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureFetch(quint16 panelIndex, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureFetch(panelIndex, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureFetchMPID(quint16 mpid, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureFetchMPID(mpid, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureGo(quint16 panelIndex, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureGo(panelIndex, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureGoMPID(quint16 mpid, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureGoMPID(mpid, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureLowerMPID(quint16 mpid, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureLowerMPID(mpid, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureRaiseMPID(quint16 mpid, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureRaiseMPID(mpid, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureMaxMPID(quint16 mpid, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureMaxMPID(mpid, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureClear(const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureClear(opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureMinMPID(quint16 mpid, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureMinMPID(mpid, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureAddMPID(quint16 mpid, quint8 minQ, quint32 workshops, quint32 stockLimit, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureAddMPID(mpid, minQ, workshops, stockLimit, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureDeleteMPID(quint16 mpid, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureDeleteMPID(mpid, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureSetMinQ(quint8 minQ, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureSetMinQ(minQ, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureProcessSetMinQMPID(quint16 mpid, quint8 minQ, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureProcessSetMinQMPID(mpid, minQ, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureProcessSetStockLimitMPID(quint16 mpid, quint32 stockLimit, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureProcessSetStockLimitMPID(mpid, stockLimit, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureProcessSelected(quint16 panelIndex, qint32 mpid, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureProcessSelected(panelIndex, mpid, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureRecordPatents(const AuKnownPatents& patents, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureRecordPatents(patents, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureGetPatents(const QSet<COMMODITYID>& patentCIDs, quint8 maxQ, const AuGearPos& mediaPos,
		const AuId& opBd)
{
	if(patentCIDs.empty())
        return;
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureGetPatents(patentCIDs, maxQ, mediaPos, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureSetLivestock(const AuSpecies& species, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureSetLivestock(species, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureSetWarehouseStorage(COMMODITYID cid, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureSetWarehouseStorage(cid, opBd);
	UnlockAvatar();
}

void AuAtomInterface::GovQuitClaim()
{
	DaPerson* person = LockAvatar();
	if(person)
		person->QuitClaimWorld();
	UnlockAvatar();
}

void AuAtomInterface::BdTakeItem(const AuItem& buyItem, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdTakeItem(buyItem, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdDiscardItem(const AuItem& buyItem, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdDiscardItem(buyItem, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdStoreBuyItem(const AuItem& buyItem, const AuItem& offerItem, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdStoreBuyItem(buyItem, offerItem, opBd);
	UnlockAvatar();
}

void AuAtomInterface::GeoRoad(const AuPoint& startLoc, const AuPoint& endLoc, bool isSlab, float roadWidth, quint8 geoType)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->GeoRoad(startLoc, endLoc, isSlab, roadWidth, geoType);
	UnlockAvatar();
}

void AuAtomInterface::GeoLandscaper(const AuPoint& startLoc, const AuPoint& endLoc, float roadWidth, bool hideRocks, int hideGrass,
        bool hideShrubs, bool hideTrees, WORLDTEX_TYPE pavement)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->GeoLandscape(startLoc, endLoc, roadWidth, hideRocks, hideGrass, hideShrubs, hideTrees, pavement);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureSpacecraftOrderDelete(quint32 designId, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureSpacecraftOrderDelete(designId, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureSpacecraftBuyCompanyShip(quint32 designId, AuGearPos pos, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureSpacecraftBuyCompanyShip(designId, pos, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureSpacecraftProcessCanceled(const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureSpacecraftProcessCanceled(opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureSpacecraftCommissionShip(quint32 designId, AuGearPos pos, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureSpacecraftCommissionShip(designId, pos, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureSpacecraftProcessDefaultSelected(quint32 designId, const QString& designName, quint8 designQ,
		const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureSpacecraftProcessDefaultSelected(designId, designName, designQ, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureSpacecraftSetCompanyPolicy(bool companyOk, quint8 companyMaxQ, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureSpacecraftSetCompanyPolicy(companyOk, companyMaxQ, opBd);
	UnlockAvatar();
}

void AuAtomInterface::BdManufactureSpacecraftSetMinQ(quint8 minQ, const AuId& opBd)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdManufactureSpacecraftSetMinQ(minQ, opBd);
	UnlockAvatar();
}

void AuAtomInterface::CreateNewEmpire(const QString& ename, const QImage& imFlag)
{
	if(!IsNameValid(ename))
		return;
	if(imFlag.isNull() || imFlag.width() > DgEmpire::FlagImageMax() || imFlag.height() > DgEmpire::FlagImageMax() ||
			imFlag.width() < 2 || imFlag.height() < 2)
		return;							// Need a flag
	AuId personId;
	QString personName;
	Avatar(personId, personName);
	const DgData& gd = g_Au.LockDataForRead();
	unsigned int idEmpire = gd.EmpireId(ename);	// Find existing empire with that name.
	g_Au.UnlockData();
	if(idEmpire)
	{
		AuCommThought msg(QObject::tr("An empire named %1 already exists. "
					"A new empire using that name could not be created.").arg(ename), personId, personName);
		g_Au.SendAtomChat(msg);
		return;							// empire by that name already exists
	}

	quint32 personEmpire = 0;
	quint8 personGender = 0;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personEmpire = person->EmpireId();
		personGender = person->Gender();
	}
	UnlockAvatar();

	// Only a person who is not in an empire can create a nu empire.
	if(personEmpire)
		return;

	// Everything appears ok to establish the nu empire.
	emit EstablishNewEmpire(personId, personName, personGender, personEmpire, ename, imFlag);
	AuCommThought msg(QObject::tr("%1 established. Congratulations.").arg(ename), personId, personName);
	g_Au.SendAtomChat(msg);
}

void AuAtomInterface::EmpireGetPatents(const QSet<COMMODITYID>& patentCIDs, quint8 maxQ, const AuGearPos& mediaPos)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->EmpireGetPatents(patentCIDs, maxQ, mediaPos);
	UnlockAvatar();
}

void AuAtomInterface::EmpireRecordPatents(const AuKnownPatents& patents)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->EmpireRecordPatents(patents);
	UnlockAvatar();
}

void AuAtomInterface::HailStore()
{
	DaPerson* person = LockAvatar();
	if(person && !person->BdId().IsNull())
	{
		DaWorld* planet = person->FindParentPlanetInEnv();
		if(planet)
		{
			DaBd* bd = planet->FindBd(person->BdId());
			if(bd)
				bd->SalesmanCalled(*person);
		}
	}
	UnlockAvatar();
}

bool AuAtomInterface::ShipmentCreate(const AuShipment& shipment)
{
	bool refreshClient = false;
	QString resultMsg;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();

		DaVessel* ves = person->FindParentVessel();
		if(ves)
		{
			if(ves->InChainOfCommand(person))
			{
				// The city must have a capitol building to be declared the capital.
				switch(ves->ShipmentCreate(shipment))
				{
				case 0:
					refreshClient = true;
					resultMsg = tr("Shipment created.");
					break;
				case 1:
					resultMsg = tr("Shipments can only be scheduled aboard space stations.");
					break;
				case 4:
					resultMsg = tr("Space station cannot handle any more shipments. "
							"It is limited to two shipments.");
					break;
				case 5:
					resultMsg = tr("The space station cannot handle any more shipments. "
							"It has an overall limit of 255 shipments in progress.");
					break;
				case 6:
					resultMsg = tr("A long haul cargo shipment must be destined for an outsystem city. "
							"That city is in this system. ");
					break;
				}
			}
			else
				resultMsg = tr("I have to be in the chain of command to create shipments.");
		}
	}
	UnlockAvatar();

	// Notify the player of the result.
	if(!resultMsg.isEmpty())
	{
		// Send the result on the Log channel.
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
	return refreshClient;
}

bool AuAtomInterface::ShipmentDelete(const AuShipment& shipment)
{
	bool refreshClient = false;
	QString resultMsg;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		DaVessel* ves = person->FindParentVessel();
		if(ves)
		{
			if(ves->InChainOfCommand(person))
			{
				switch(ves->ShipmentDelete(shipment))
				{
				case 0:
					refreshClient = true;
					resultMsg = tr("Shipment deleted.");
					break;
				case 1:
					resultMsg = tr("Shipments can only be deleted aboard space stations.");
					break;
				case 4:
					resultMsg = tr("No record of the shipment was found. How odd.");
					break;
				}
			}
			else
				resultMsg = tr("I have to be in the chain of command to delete shipments.");
		}
	}
	UnlockAvatar();

	// Notify the player of the result.
	if(!resultMsg.isEmpty())
	{
		// Send the result on the Log channel.
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
	return refreshClient;
}

void AuAtomInterface::StoryScriptSubmitted(const AuId& forWho, const AuScript& script)
{
	AuId personId;
	QString personName;
	Avatar(personId, personName);
	if(!script.AnyAutoStart())
	{
		QString text = tr("Story script '%1' was rejected because it does not autostart.")
				.arg(script.Title());
		AuCommThought msg(text, personId, personName);
		g_Au.SendAtomChat(msg);
		return;
	}

	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	if(person && system)
	{
		QString resultMsg;
		DaPerson* forPerson = dynamic_cast<DaPerson*>(system->FindIdRecursive(forWho));
		if(forPerson)
		{
			forPerson->StoryScriptSubmitted(script);
			resultMsg = tr("Story script '%1' was attached to %2.")
					.arg(script.Title())
					.arg(forPerson->Name());
		}
		else
			resultMsg = tr("Story script '%1' was rejected. %2 could not be found in the solar system.")
					.arg(script.Title())
					.arg(forWho.StringId());
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
	UnlockAvatar();
}

void AuAtomInterface::StoryClearScript(const AuId& forWho)
{
	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	if(person && system)
	{
		QString resultMsg;
		DaPerson* forPerson = dynamic_cast<DaPerson*>(system->FindIdRecursive(forWho));
		if(forPerson)
		{
			int result = forPerson->StoryCancelScript();
			forPerson->SetModified();
			switch(result)
			{
			case 0:
				resultMsg = tr("%1 had no story script to clear.")
						.arg(forPerson->Name());
				break;
			case 1:
				resultMsg = tr("Story script was cleared from %1.")
						.arg(forPerson->Name());
				break;
			case 2:
				resultMsg = tr("Story script was cleared from %1. An associated story job was terminated. ")
						.arg(forPerson->Name());
				break;
			}
		}
		else
			resultMsg = tr("Story script could not be cleared.");
		AuCommThought msg(resultMsg, person->Id(), person->Name());
		g_Au.SendAtomChat(msg);
	}
	UnlockAvatar();
}

void AuAtomInterface::ScDesignExchangeRate(const AuExchangeRatingRec& rating)
{
	if(!rating.IsValid())
		return;
	bool hasEx = false;
	QString designName;
	const DgData& sdi = g_Au.LockScExchangeForRead();
	AuScDesignExchange::const_iterator ei = sdi.m_ScExchange->find(rating.DesignId());
	if(ei != sdi.m_ScExchange->cend())
	{
		designName = (*ei).DesignName();
		hasEx = true;
	}
	g_Au.UnlockScExchange();

	QString resultMsg;
	if(hasEx)
	{
		g_Au.RateBlueprintSc(rating);
		resultMsg = QObject::tr("I gave a rating of %2 to spacecraft blueprint %1 %3 in the exchange.")
			.arg(rating.DesignId())
			.arg(rating.Rating())
			.arg(designName);
		emit BlueprintRatedSc(rating);
	}
	else
	{
		resultMsg = QObject::tr("Blueprint %1 was not found in the spacecraft blueprint exchange so it could not be rated.")
			.arg(rating.DesignId());
	}

	if(!resultMsg.isEmpty())
	{
		AuId personId;
		QString personName;
		Avatar(personId, personName);
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::BdDesignExchangeRate(const AuExchangeRatingRec& rating)
{
	if(!rating.IsValid())
		return;
	bool hasEx = false;
	QString designName;
	const DgData& sdi = g_Au.LockBdExchangeForRead();
	AuBdDesignExchange::const_iterator ei = sdi.m_BdExchange->find(rating.DesignId());
	if(ei != sdi.m_BdExchange->cend())
	{
		designName = (*ei).DesignName();
		hasEx = true;
	}
	g_Au.UnlockBdExchange();

	QString resultMsg;
	if(hasEx)
	{
		g_Au.RateBlueprintBd(rating);
		resultMsg = QObject::tr("I gave a rating of %2 to building blueprint %1 %3 in the exchange.")
			.arg(rating.DesignId())
			.arg(rating.Rating())
			.arg(designName);
		emit BlueprintRatedBd(rating);
	}
	else
	{
		resultMsg = QObject::tr("Blueprint %1 was not found in the building blueprint exchange so it could not be rated.")
			.arg(rating.DesignId());
	}

	if(!resultMsg.isEmpty())
	{
		AuId personId;
		QString personName;
		Avatar(personId, personName);
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::EmpireChangeBuildingPref(quint32 empireId, int industry, quint32 designId)
{
	AuId personId;
	QString personName;
	Avatar(personId, personName);

	emit EmpireChgBdPref(empireId, industry, designId);

    QString empireName;
    const DgData& gd = g_Au.LockDataForRead();
    gd.EmpireName(empireId, empireName);
    QString iname = gd.IndustryName((INDUSTRYID)industry);
    g_Au.UnlockData();

    QString resultMsg = tr("%1 building blueprint for %2 changed to %3.")
        .arg(empireName)
        .arg(iname)
        .arg(designId);
    AuCommThought msg(resultMsg, personId, personName);
    g_Au.SendAtomChat(msg);
}

void AuAtomInterface::EmpireChangeSpacecraftPref(quint32 empireId, int scrole, quint32 designId)
{
	AuId personId;
	QString personName;
	Avatar(personId, personName);

	emit EmpireChgScPref(empireId, scrole, designId);

	QString empireName;
    const DgData& gd = g_Au.LockDataForRead();
    gd.EmpireName(empireId, empireName);
    g_Au.UnlockData();

    QString rname = AuScPrefs::RoleName((AuScPrefs::SCROLE)scrole);

    QString resultMsg = tr("%1 spacecraft blueprint for %2 changed to %3.")
        .arg(empireName)
        .arg(rname)
        .arg(designId);
    AuCommThought msg(resultMsg, personId, personName);
    g_Au.SendAtomChat(msg);
}

void AuAtomInterface::CopySpacecraftTitle(const AuGearPos& mediaPos)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship)
		{
			if(!ship->IsPrivatelyOwned() || ship->OwnerId() != person->Id())
			{
				AuCommThought msg(tr("I cannot copy the title to a spacecraft I do not own."),
						person->Id(), person->Name());
				g_Au.SendAtomChat(msg);
			}
			else
			{
				AuGear* pinv = person->LockGearForWrite();
				AuItem* mediaItem = pinv->ItemAt(mediaPos);
				if(mediaItem)
				{
					// The media item was found.
					if(mediaItem->CommodityId() == CID_BlankDisk || mediaItem->CommodityId() == CID_Paper)
					{
						AuItem* media = mediaItem->Clone();
						person->UnlockGear();

						media->SetCount(1);
						person->ConsumeItem(*media);

						QString resultMsg;
						if(media->UseItem())					// Quality check the result.
						{
							// Replace the blank disk with a mission disk.
							QString titleDesc;
							titleDesc = tr("%3 %L1m%2, blueprint %4 %5")
								.arg(ship->HullVolume())
								.arg(g_Au.CubedSymbol())
								.arg(SpacecraftStructureName(ship->Structure()))
								.arg(ship->DesignId())
								.arg(ship->DesignName());
							AuItemOwnershipTitle* title = new AuItemOwnershipTitle(ship->Id(), ship->NameWithEmpire(),
									titleDesc, CID_SpacecraftTitle, media->Quality(), media->EmpireOfOrigin());

							// Put it wherever there's room.
							person->AddGearItem(*title);

							resultMsg = tr("%1 title of ownership stored to %2.")
								.arg(ship->NameWithEmpire())
								.arg(media->SayName());

							delete title;
						}
						else
						{
							// Media failure.
							resultMsg = tr("The %1 was defective; it failed making the copy. "
									"The title was not saved. "
									"I should try again with another blank disk or paper.").arg(media->SayName());
						}
						delete media;

						AuCommThought msg(resultMsg, person->Id(), person->Name());
						g_Au.SendAtomChat(msg);
					}
					else
						person->UnlockGear();
				}
				else
					person->UnlockGear();
			}
		}
		else
		{
			AuCommThought msg(tr("I must board a spacecraft I own to copy the title."),
					person->Id(), person->Name());
			g_Au.SendAtomChat(msg);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::PayInsurer(double amount)
{
	QString resultMsg;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		if(amount > person->AccountBalance())
			resultMsg = tr("I don't have that much money.");
		else
		{
			double insurance = person->InsuranceBounty();
			if(amount > insurance)
				amount = insurance;
			person->DebitAccount(amount);
			person->PayInsuranceBounty(amount);
			insurance = person->InsuranceBounty();
			if(insurance > 0)
				resultMsg = tr("I paid %L1%2 toward my insurance claim. I still owe %L3%2 to the %4.")
					.arg(amount, 0,'f',0)
					.arg(AuGlobal::CurrencySymbol())
					.arg(insurance, 0,'f',0)
					.arg(InsurerName());
			else
				resultMsg = tr("I paid the entire %L1%2 insurance claim. %4 is completely paid off.")
					.arg(amount, 0,'f',0)
					.arg(AuGlobal::CurrencySymbol())
					.arg(InsurerName());
		}
	}
	UnlockAvatar();
	if(!resultMsg.isEmpty())
	{
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::BdFence(const AuPoint& startLoc, const AuPoint& endLoc, int fenceStyle, const AuId& onPlatform,
		quint32 customDesignId, int mat)
{
	DaPerson* person = LockAvatar();
	if(person)
		person->BdBuildFence(startLoc, endLoc, fenceStyle, onPlatform, customDesignId, mat);
	UnlockAvatar();
}

void AuAtomInterface::SetHairStyle(AuHairStyle::STYLE haircut, AuHairStyle::SHAVE shave)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		person->SetHairStyle(haircut, shave);
		person->SetModified();
	}
	UnlockAvatar();
}

void AuAtomInterface::SetDNA(const AuDNA& dna)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		// Check DNA for validity.
		// The validity check was already performed on the client. If it fails now, they don't get an error message.
		AuDNA::CHROMATID chroma = person->Chromatid();
		HANDEDNESS hand = person->Handedness();
		GROWTHSTAGE stage = person->GrowthStage();
		AuAnimalia animalia(dna, chroma, hand, stage);
		int dnaCheck = animalia.AvatarCheck();
		if(!dnaCheck)
		{
			// DNA checks ok
			person->SetDNA(dna);
			person->SetModified();
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::DomainAdd(const AuId& systemId, const QString& systemName)
{
	bool didIt = false;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		didIt = person->DomainAdd(systemId, systemName);
	}
	UnlockAvatar();
    if(didIt)
    {
        AuCommThought msg(tr("%1 was added to my land.").arg(systemName), personId, personName);
        g_Au.SendAtomChat(msg);
    }
    else
    {
        AuCommThought msg(tr("%1 could not be added to my land. My land is limited to %2 systems.")
                .arg(systemName).arg(AuDomain::LimitPerAvatar()), personId, personName);
        g_Au.SendAtomChat(msg);
    }
}

void AuAtomInterface::DomainRemove(const AuId& systemId, const QString& systemName)
{
	bool didIt = false;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		didIt = person->DomainRemove(systemId);
	}
	UnlockAvatar();
    if(didIt)
    {
        AuCommThought msg(tr("%1 was removed from my land.").arg(systemName), personId, personName);
        g_Au.SendAtomChat(msg);
    }
    else
    {
        AuCommThought msg(tr("%1 could not be removed from my land.").arg(systemName), personId, personName);
        g_Au.SendAtomChat(msg);
    }
}

void AuAtomInterface::DomainAddCurrent()
{
	AuId systemId;
	QString systemName;
	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	if(person && system)
	{
		systemId = system->Id();
		systemName = system->Name();
	}
	UnlockAvatar();
	if(!systemId.IsNull())
		DomainAdd(systemId, systemName);
}

void AuAtomInterface::DesignerArchitectSaveIt(AuItDesign& design)
{
	AuId personId;
	QString personName;
	Avatar(personId, personName);
	if(design.ArchitectId() != personId || !IsNameValid(design.Name()))
		return;
	if(design.Thumbnail().width() > 256 || design.Thumbnail().height() > 256)
		return;

	// New designs should always start in Edit mode.
	if(!design.Id())
		design.SetState(AuDesign::Edit);

	// The client sent us the entire design, potentially finalized.
	DgData& sdd = g_Au.LockItDesignsForWrite();
	int result = sdd.ItDesignSave(design);
	g_Au.UnlockItDesigns();

	if(result == 0 || result == 1)
	{
		DaPerson* person = LockAvatar();
		if(person)
		{
			// The scene should be subordinate to a design environment.
			DaDesigner* pde = person->FindParentDesignerInEnv();
			if(pde)
				pde->SetDesignModified(false);
		}
		UnlockAvatar();
	}

	QString resultMsg;
	switch(result)
	{
	case 0:
		resultMsg = QObject::tr("New blueprint %1 %2 added to the library.")
			.arg(design.Id())
			.arg(design.Name());
		break;
	case 1:
		resultMsg = QObject::tr("Existing blueprint %1 %2 replaced in the library.")
			.arg(design.Id())
			.arg(design.Name());
		break;
	case 2:
		resultMsg = QObject::tr("Existing blueprint cannot be replaced in the library.");
		break;
	case 3:
		resultMsg = QObject::tr("Existing blueprint cannot be found in the library.");
		break;
	case 4:
		resultMsg = QObject::tr("New blueprint id %1 already exists in the library.").arg(design.Id());
		break;
	}

	if(!resultMsg.isEmpty())
	{
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::DesignerArchitectDeleteIt(bool isArchitect, quint32 designId)
{
	AuId personId;
	QString personName;
	Avatar(personId, personName);

	DgData& sdd = g_Au.LockItDesignsForWrite();
	int result;
	if(sdd.ItDesignArchitect(designId) == personId || isArchitect)
		result = sdd.ItDesignDelete(designId);
	else
		result = 3;
	g_Au.UnlockItDesigns();

	QString resultMsg;
	switch(result)
	{
	case 0:
		resultMsg = QObject::tr("Item blueprint %1 was not found to delete from the library.")
			.arg(designId);
		break;
	case 1:
		//ItDesignExchangeUnpublish(designId, true);
		resultMsg = QObject::tr("Item blueprint %1 deleted from the library.")
			.arg(designId);
		break;
	case 2:
		//ItDesignExchangeUnpublish(designId, true);
		resultMsg = QObject::tr("Item blueprint %1 deactivated in the library.")
			.arg(designId);
		break;
	case 3:
		resultMsg = QObject::tr("Item blueprint %1 belongs to another architect; you cannot delete it.")
			.arg(designId);
		break;
	}

	if(!resultMsg.isEmpty())
	{
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::DesignerArchitectFinalizeItAssembly(AuItDesign& design)
{
	AuId personId;
	QString personName;
	Avatar(personId, personName);
	if(design.ArchitectId() != personId || !IsNameValid(design.Name()))
		return;
	if(design.Thumbnail().width() > 64 || design.Thumbnail().height() > 64)
		return;

	AuCommThought msg(QObject::tr("Item assembly %1 received. Verifying...").arg(design.Name()), personId, personName);
	g_Au.SendAtomChat(msg);

	// Validate design to insure it really is ok to manufacture.
	QString resultMsg;
	QStringList msgs;
	if(!design.FinalizeAssembly(msgs))
	{
		resultMsg = QObject::tr("Unable to finalize item assembly %1 %2.")
			.arg(design.Id())
			.arg(design.Name());
	}
	else
	{
		// The client sent us the entire design.
		DgData& sdd = g_Au.LockItDesignsForWrite();
		int result = sdd.ItDesignSave(design);
		g_Au.UnlockItDesigns();

		switch(result)
		{
		case 0:
			resultMsg = QObject::tr("New item assembly %1 %2 added to the library. "
					"The blueprint could now be published in the exchange or stored to a device in your gear.")
				.arg(design.Id())
				.arg(design.Name());
			break;

		// The rest of the possible results aren't really possible when saving a nu design, with id of 0.
		case 1:
			resultMsg = QObject::tr("Existing item assembly %1 %2 replaced in the library.")
				.arg(design.Id())
				.arg(design.Name());
			break;
		case 2:
			resultMsg = QObject::tr("Existing item assembly cannot be replaced in the library.");
			break;
		case 3:
			resultMsg = QObject::tr("Existing item assembly cannot be found in the library.");
			break;
		case 4:
			resultMsg = QObject::tr("New item assembly id %1 already exists in the library.").arg(design.Id());
			break;
		}

		if(!resultMsg.isEmpty())
		{
			AuCommThought msg(resultMsg, personId, personName);
			g_Au.SendAtomChat(msg);
		}
	}
}

void AuAtomInterface::DesignerArchitectFinalizeItDesign(AuItDesign& design, quint32 designId/*=0*/)
{
	AuId personId;
	QString personName;
	Avatar(personId, personName);
	if(design.ArchitectId() != personId || !IsNameValid(design.Name()))
		return;
	if(design.Thumbnail().width() > 64 || design.Thumbnail().height() > 64)
		return;

	AuCommThought msg(QObject::tr("Item blueprint %1 received. Verifying...").arg(design.Name()), personId, personName);
	g_Au.SendAtomChat(msg);

	// Validate design to insure it really is ok to manufacture.
	QString resultMsg;
	AuItReport report;
	design.ReportDesign(report);
	QStringList msgs;
	if(!report.OkToManufacture(msgs, &design))
	{
		resultMsg = QObject::tr("Item blueprint %1 is not valid for manufacture.")
			.arg(design.Name());
	}
	else
	{
		// Finalize prepared the design to be saved as new, including setting the id to 0.
		Q_ASSERT(!design.Id());
		DgData& sdd = g_Au.LockItDesignsForWrite();
		int result = sdd.ItDesignSave(design, designId);
		g_Au.UnlockItDesigns();

		switch(result)
		{
		case 0:
			resultMsg = QObject::tr("New item blueprint %1 %2 added to the library. "
					"The blueprint could now be published in the exchange or stored to a device in your gear.")
				.arg(design.Id())
				.arg(design.Name());
			break;

		// The rest of the possible results aren't really possible when saving a nu design, with id of 0.
		case 1:
			resultMsg = QObject::tr("Existing item blueprint %1 %2 replaced in the library.")
				.arg(design.Id())
				.arg(design.Name());
			break;
		case 2:
			resultMsg = QObject::tr("Existing item blueprint cannot be replaced in the library.");
			break;
		case 3:
			resultMsg = QObject::tr("Existing item blueprint cannot be found in the library.");
			break;
		case 4:
			resultMsg = QObject::tr("New item blueprint id %1 already exists in the library.").arg(design.Id());
			break;
		}
	}

	if(!resultMsg.isEmpty())
	{
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

bool AuAtomInterface::TradeAgreementCreate(const AuTradeAgreement& ta, const AuId& opBd)
{
	bool refreshClient = false;
	QString resultMsg;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		DaWorld* planet = person->FindParentPlanetInEnv();
		if(planet)
		{
			DaBd* bd = person->BdOp(opBd);
			if(bd)
			{
				if(bd->InChainOfCommand(person))
				{
					switch(bd->TradeAgreementCreate(ta))
					{
					case 0:
						refreshClient = true;
						if(ta.IsProposal())
							resultMsg = tr("Trade proposal transmitted.");
						else
							resultMsg = tr("Trade agreement transmitted.");
						break;
					case 1:
						resultMsg = tr("Broker is missing.");
						break;
					case 2:
						resultMsg = tr("Broker is destroyed. The building must be repaired.");
						break;
					case 3:
						resultMsg = tr("Broker has no completed stories. How odd.");
						break;
					case 4:
						resultMsg = tr("Broker cannot handle any more shipments. "
								"It is limited by the number of offices.");
						break;
					case 5:
						resultMsg = tr("The building cannot handle any more shipments. "
								"It has an overall limit of 255 shipments in progress.");
						break;
					case 6:
						resultMsg = tr("A long haul cargo trade agreement must be destined for an outsystem city. "
								"That city is in this system. ");
						break;
					}
				}
				else
					resultMsg = tr("Only authorized government officials can create trade agreements.");
			}
		}
	}
	UnlockAvatar();

	// Notify the player of the result.
	if(!resultMsg.isEmpty())
	{
		// Send the result on the Log channel.
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
	if(refreshClient)
		emit ProposeTrade(ta);
	return refreshClient;
}

bool AuAtomInterface::TradeAgreementDelete(const AuTradeAgreement& ta, const AuId& opBd)
{
	bool refreshClient = false;
	quint8 nuState = AuTradeAgreement::Proposal;	// Not a valid nu state for a deleted agreement.
	bool atDest = true;

	QString resultMsg;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		DaBd* bd = person->BdOp(opBd);
		if(bd)
		{
			if(bd->InChainOfCommand(person))
			{
				switch(bd->TradeAgreementDelete(ta))
				{
				case 0:	// bd is the source
					atDest = true;	// must update corresponding dest
					refreshClient = true;
					if(ta.IsProposal())
						nuState = AuTradeAgreement::Declined;
					else if(ta.IsAgreement())
						nuState = AuTradeAgreement::Terminated;
					if(ta.IsProposal())
						resultMsg = tr("Trade proposal declined.");
					else if(ta.IsAgreement())
						resultMsg = tr("Trade agreement terminated.");
					else
						resultMsg = tr("Trade agreement status deleted.");
					break;
				case 1:	// bd is the destination
					atDest = false;	// must update corresponding origin
					refreshClient = true;
					if(ta.IsProposal())
						nuState = AuTradeAgreement::Declined;
					else if(ta.IsAgreement())
						nuState = AuTradeAgreement::Terminated;
					if(ta.IsProposal())
						resultMsg = tr("Trade proposal declined.");
					else if(ta.IsAgreement())
						resultMsg = tr("Trade agreement terminated.");
					else
						resultMsg = tr("Trade agreement status deleted.");
					break;
				case 2:
					if(ta.IsProposal())
						resultMsg = tr("Broker of the trade proposal is missing.");
					else if(ta.IsAgreement())
						resultMsg = tr("Broker of the trade agreement is missing.");
					else
						resultMsg = tr("Broker of the trade agreement status is missing.");
					break;
				case 3:
					if(ta.IsProposal())
						resultMsg = tr("No record of the trade proposal was found. How odd.");
					else if(ta.IsAgreement())
						resultMsg = tr("No record of the trade agreement was found. How odd.");
					else
						resultMsg = tr("No record of the trade agreement status was found. How odd.");
					break;
				}
			}
			else
				resultMsg = tr("Only authorized government officials can delete trade agreements.");
		}
	}
	UnlockAvatar();

	// Notify the player of the result.
	if(!resultMsg.isEmpty())
	{
		// Send the result on the Log channel.
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}

	if(nuState != AuTradeAgreement::Proposal)
		emit DeactivateTrade(ta, nuState, atDest);
	return refreshClient;
}

bool AuAtomInterface::TradeAgreementAccept(const AuTradeAgreement& ta, const AuId& opBd)
{
	bool refreshClient = false;
	QString resultMsg;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		DaBd* bd = person->BdOp(opBd);
		if(bd)
		{
			if(bd->InChainOfCommand(person))
			{
				// Can only be accepted at the destination city.
				if(bd->TradeAgreementAccept(ta, true))
				{
					refreshClient = true;
					resultMsg = tr("Trade proposal accepted. It is now a trade agreement.");
				}
				else
					resultMsg = tr("Trade proposal can only be accepted at the destination broker.");
			}
			else
				resultMsg = tr("Only authorized government officials can delete trade agreements.");
		}
	}
	UnlockAvatar();

	// Notify the player of the result.
	if(!resultMsg.isEmpty())
	{
		// Send the result on the Log channel.
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}

	if(refreshClient)
		emit AcceptTrade(ta);
	return refreshClient;
}

bool AuAtomInterface::TradeAgreementCounter(const AuTradeAgreement& proposal, const AuTradeAgreement& counter, const AuId& opBd)
{
	bool refreshClient = false;
	QString resultMsg;
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		DaBd* bd = person->BdOp(opBd);
		if(bd)
		{
			if(bd->InChainOfCommand(person))
			{
				switch(bd->TradeAgreementCounter(proposal, counter))
				{
				case 0:
					refreshClient = true;
					resultMsg = tr("Trade counter proposal transmitted.");
					break;
				case 1:
					resultMsg = tr("Broker of the trade proposal is missing.");
					break;
				case 2:
					resultMsg = tr("No record of the trade proposal was found.");
					break;
				case 3:
					resultMsg = tr("Counter proposal could not be added to the building.");
					break;
				case 4:
					resultMsg = tr("Proposal to counter is not a proposal.");
					break;
				case 5:
					resultMsg = tr("Counter proposal is not a proposal.");
					break;
				case 6:
					resultMsg = tr("I cannot counter my own proposal.");
					break;
				}
			}
			else
				resultMsg = tr("Only authorized government officials can delete trade agreements.");
		}
	}
	UnlockAvatar();

	// Notify the player of the result.
	if(!resultMsg.isEmpty())
	{
		// Send the result on the Log channel.
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}

	if(refreshClient)
	{
		emit DeactivateTrade(proposal, AuTradeAgreement::Countered, false);	// Notify orig of the counter.
		emit ProposeTrade(counter);
	}
	return refreshClient;
}

void AuAtomInterface::ReleaseBerth()
{
	AuId personId;
	QString personName;
	Avatar(personId, personName);
	emit ReleaseBerth(personId, AuId());
}

void AuAtomInterface::QueryJobs(QMultiMap<int, AuJobOffer>& jobsToStart)
{
	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	const DaSector* sector = system ? system->FindParentSector() : 0;
	const DaGalaxy* galaxy = system ? system->FindParentGalaxy() : 0;
	if(person && system && sector && galaxy)
	{
		// Make a list of the scripts that the player is eligible to start.
		const AuScripts* scripts = g_Au.LockScripts();
		const AuStories* stories = g_Au.LockStoriesForRead();
		QMap<QString, QVariant> vars;
		for(AuScripts::const_iterator it = scripts->cbegin(); it != scripts->cend(); it++)
		{
			// Insure the player is eligible to start the story.
			vars.clear();
			if(!person->TestStoryCatalyst((*it).second, *stories, 0, vars))
				continue;

			// Create a story job that will be included with the job offer.
			// When AuJobStory is used as an offer, the story script id is substituted for the story instance id.
			AuJobStory story((*it).second.Id(), (*it).second.Title(), (*it).second.Author(), (*it).second.Brief());

			// A story job offer passes the author of the story as the from name. That way story offers sorted by origin get sorted
			// by author name.
			AuJobOffer sinfo((*it).second.Author(), 0, sector->Location(), galaxy->Galaxy(), &story);
			jobsToStart.insert(0, sinfo);
		}
		g_Au.UnlockStories();
		g_Au.UnlockScripts();

		// List job offers that have been posted publicly and that are ok to offer to the person.
		AuTime expiryTest = AuTime::Now().AddMins(3);					// Test against expiration 3 minutes from now.
		const AuJobOffers* jobOffers = g_Au.LockJobOffersForRead();
		for(AuJobOffers::const_iterator ji = jobOffers->cbegin(); ji != jobOffers->cend(); ji++)
		{
			if((*ji).IsExpired(expiryTest))
				continue;											// Job will expire too soon.

			if(galaxy->Galaxy() != (*ji).FromGalaxy())
				continue;

			int range = (int)ceil(sector->Location().Distance((*ji).FromSectorLoc()) / 10);             // result is in sectors
			if(range > MaxJobSectorRange())
				continue;											// Too far away to bother.

			if(!(*ji).IsOkToOffer(*person, range))
				continue;											// Person is not eligible to take the job.

			jobsToStart.insert(range, *ji);
		}
		g_Au.UnlockJobOffers();
	}
	UnlockAvatar();
}

void AuAtomInterface::QueryJobs(QMultiMap<int, AuJobOffer>& jobsToStart, const QList<AuJob::TYPE>& filterTypes, int maxSectorsAway)
{
	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	const DaSector* sector = system ? system->FindParentSector() : 0;
	const DaGalaxy* galaxy = system ? system->FindParentGalaxy() : 0;
	if(person && system && sector && galaxy)
	{
		// Make a list of the scripts that the player is eligible to start.
		if(filterTypes.contains(AuJob::Story))
		{
			const AuScripts* scripts = g_Au.LockScripts();
			const AuStories* stories = g_Au.LockStoriesForRead();
			QMap<QString, QVariant> vars;
			for(AuScripts::const_iterator it = scripts->cbegin(); it != scripts->cend(); it++)
			{
				// Insure the player is eligible to start the story.
				vars.clear();
				if(!person->TestStoryCatalyst((*it).second, *stories, 0, vars))
					continue;

				// Create a story job that will be included with the job offer.
				// When AuJobStory is used as an offer, the story script id is substituted for the story instance id.
				AuJobStory story((*it).second.Id(), (*it).second.Title(), (*it).second.Author(), (*it).second.Brief());

				// A story job offer passes the author of the story as the from name. That way story offers sorted by origin get
				// sorted by author name.
				AuJobOffer sinfo((*it).second.Author(), 0, sector->Location(), galaxy->Galaxy(), &story);
				jobsToStart.insert(0, sinfo);
			}
			g_Au.UnlockStories();
			g_Au.UnlockScripts();
		}

		// List job offers that have been posted publicly and that are ok to offer to the person.
		AuTime expiryTest = AuTime::Now().AddMins(3);					// Test against expiration 3 minutes from now.
		const AuJobOffers* jobOffers = g_Au.LockJobOffersForRead();
		for(AuJobOffers::const_iterator ji = jobOffers->cbegin(); ji != jobOffers->cend(); ji++)
		{
			if((*ji).IsExpired(expiryTest))
				continue;											// Job will expire too soon.

			if(!filterTypes.contains((*ji).Job()->Type()))
				continue;											// Unwanted job type.

			if(galaxy->Galaxy() != (*ji).FromGalaxy())
				continue;

			int range = (int)ceil(sector->Location().Distance((*ji).FromSectorLoc()) / 10);             // result is in sectors
			if(range > maxSectorsAway)
				continue;											// Too far away to bother.

			if(!(*ji).IsOkToOffer(*person, range))
				continue;											// Person is not eligible to take the job.

			jobsToStart.insert(range, *ji);
		}
		g_Au.UnlockJobOffers();
	}
	UnlockAvatar();
}

void AuAtomInterface::ArenaJoin(const AuId& arenaId, const QString& password, quint32 designId, quint8 team)
{
	bool arenaOk = false;
	bool passOk = false;
	bool teamOk = (team == DgEmpire::ArenaTeam1Id() || team == DgEmpire::ArenaTeam2Id() || team == DgEmpire::ArenaObserverId());
	const unordered_map<AuId, AuArenaReady, AuIdHash>& arenas = g_Au.LockArenasForRead();
	unordered_map<AuId, AuArenaReady, AuIdHash>::const_iterator ai = arenas.find(arenaId);
	if(ai != arenas.cend() && !(*ai).second.IsExpired())
	{
		arenaOk = true;
		if((*ai).second.m_Password.isEmpty() || (*ai).second.m_Password == password)
			passOk = true;
		if((*ai).second.m_MatchState != DaArena::Join && (team == DgEmpire::ArenaTeam1Id() || team == DgEmpire::ArenaTeam2Id()))
			teamOk = false;
	}
	g_Au.UnlockArenas();

	QString resultMsg;
	AuId personId;
	QString personName;
	if(arenaOk && passOk && teamOk)
	{
		// We're going to risk using admin goto to get there. D:
		AuId returnOnFail;
		DaPerson* person = LockAvatar();
		if(person)
		{
			personId = person->Id();
			personName = person->Name();
			returnOnFail = person->ParentId();
			person->PrepareToEnterArena(team);
			person->SetArenaShipDesign(designId);
			person->SetModified();
		}
		UnlockAvatar();
		resultMsg = tr("I am entering the arena.");
		if(!personId.IsNull())
			emit AdminGoTo(personId, arenaId, 0, false, returnOnFail);
	}
	else
	{
		Avatar(personId, personName);
		if(!arenaOk)
			resultMsg = tr("The arena match is no longer available.");
		else if(!passOk)
			resultMsg = tr("The password is incorrect. It is case sensitive.");
		else if(!teamOk)
			//resultMsg = tr("The match is closed to players at this time. Observers are welcome to join.");
			resultMsg = tr("The match is closed to players at this time.");
	}
	AuCommThought msg(resultMsg, personId, personName);
	g_Au.SendAtomChat(msg);
}

void AuAtomInterface::JobAccepted(const AuJobOffer& jobOffer)
{
	if(!jobOffer.Job())
		return;
	DaPerson* person = LockAvatar();
	if(person)
		person->JobAccepted(jobOffer);
	UnlockAvatar();
}

void AuAtomInterface::AgentResetStory(const AuId& avatarId, quint32 scriptId)
{
	const AuScripts* scripts = g_Au.LockScripts();
	QString scriptTitle = scripts->ScriptTitle(scriptId);
	g_Au.UnlockScripts();
	if(scriptTitle.isEmpty())
	{
		AuId personId;
		QString personName;
		Avatar(personId, personName);
		AuCommThought msg(tr("Script %1 not found.").arg(scriptId), personId, personName);
		g_Au.SendAtomChat(msg);
		return;
	}

	DaPerson* person = LockAvatar();
	DaSolarSystem* system = person ? person->FindParentSolarSystemInEnv() : 0;
	if(person && system)
	{
		DaPerson* avatar = dynamic_cast<DaPerson*>(system->FindIdRecursive(avatarId));
		if(avatar)
		{
			avatar->ResetStory(scriptId);

			AuCommThought msg(tr("Story '%1' reset for %2.").arg(scriptTitle).arg(avatar->Name()), person->Id(), person->Name());
			g_Au.SendAtomChat(msg);

			if(!person->Is(avatar->Id()))
			{
				// Notify the avatar.
				AuCommThought amsg(tr("Story '%1' reset for %2.").arg(scriptTitle).arg(avatar->Name()), avatar->Id(),
						avatar->Name());
				g_Au.SendAtomChat(amsg);
			}
		}
		else
		{
			AuCommThought msg(tr("Avatar %1 not found.").arg(avatarId.StringId()), person->Id(), person->Name());
			g_Au.SendAtomChat(msg);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::AgentSetDNA(const AuId& chgWho, const AuDNABase& nuDNA)
{
	QString response;									// Agent notification message of the result.
	AuId personId;
	QString personName;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		personName = person->Name();
		DaSolarSystem* system = person->FindParentSolarSystemInEnv();
		if(system)
		{
			DaCreature* creature = dynamic_cast<DaCreature*>(system->FindIdRecursive(chgWho));
			if(creature)
			{
				// Found the creature. Change its DNA.
				response = tr("%1 %2").arg(creature->Name()).arg(creature->Description());

				creature->SetDNA(nuDNA);
				creature->ResetHitPoints();
				creature->ResetHunger();
				creature->ResetStamina();
				creature->SetModified();

				response += tr(" changed DNA to %1, max %2 hp.")
					.arg(creature->Species().Description())
					.arg(creature->MaxHitPoints());
			}
			else
			{
				response = tr("Creature not found to change its DNA.");
			}
		}
	}
	UnlockAvatar();

	if(!response.isEmpty())
	{
		AuCommThought msg(response, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

void AuAtomInterface::AgentScuttleShip(const QString& reason)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship)
		{
			// Release all berths of the spacecraft.
			// This prevents a privately owned ship from respawning.
			ship->ClearRoster();

			// Immediately respawn all people aboard, so we can destroy the ship.
			DaAtomList people;
			ship->FindPeopleRecursive(people);	// Does not find people who are already travelling (spawning?), which is ok.
			for(DaAtomList::iterator pi = people.begin(); pi != people.end(); pi++)
				((DaPerson*)(*pi))->Respawn(false);

			// Compose a reason that will appear in mail sent to everyone aboard.
			QString reasonMsg;
			if(reason.isEmpty())
			{
				reasonMsg = tr("Scuttled by agent. "
						"%1 people aboard were safely recalled home before destruction; "
						"any berths held on the ship were released.").arg(people.count());
			}
			else
			{
				reasonMsg = tr("Scuttled by agent who said, \"%2.\" "
						"%1 people aboard were safely recalled home before destruction; "
						"any berths held on the ship were released.").arg(people.count()).arg(reason);
			}

			if(ship->IsPrivatelyOwned())
				reasonMsg += tr(" Company ship was permanently destroyed; recovery is not possible.");

			// Everybody aboard is safe; destroy the ship.
			ship->DamageAndDestroy(reasonMsg);
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::AgentRecoverShip(const AuId& shipId)
{
	if(shipId.IsNull())
		return;

	quint8 galaxy = GALAXY_Count;
	AuId personId;
	DaPerson* person = LockAvatar();
	if(person)
	{
		personId = person->Id();
		galaxy = person->Galaxy();
	}
	UnlockAvatar();
	if(galaxy != GALAXY_Count)
		emit SpacecraftStasisRecover(shipId, personId, galaxy, true);
}

void AuAtomInterface::TransporterPlaces(QList<AuTransporterTarget>& ttplaces)
{
	DaPerson* person = LockAvatar();
	if(person)
	{
		// Determine the range of the transporter.
		double range = 0;
		AuPoint shipSysLoc;
		AuId shipId;
		if(person->InTransporterRoom())
		{
			// Person is in a transporter room.
			DaCapsule* ship = person->FindParentCapsuleInEnv();
			if(ship)
			{
				range = ship->TransporterRange(person->TransporterSystemWhich());
				shipSysLoc = ship->EnvLocation();
				shipId = ship->Id();
			}
		}
		else
		{
			VEHICLE_SPOT_TYPE spotType = person->InVehicleTransporter();
			if(spotType != SPOT_Invalid)
			{
				// Person is in a vehicle that is parked where it can be transported.
				DaCapsule* ship = person->FindParentCapsuleInEnv();
				if(ship)
				{
					range = ship->TransporterRange(ship->VehicleTransporterSystemId(spotType, false));
					shipSysLoc = ship->EnvLocation();
				}
			}
		}

		if(range > 0)
		{
			double range2 = range * range;		// squared distance for comparison without sqrt
			DaAtom* env = person->FindEnvironmentAtom();

			// Locate all transporter targets in the solar system/environment.
			// We will sort them by target type and then by name.
			QMap<TRANSPORTER_TARGET, QMap<QString, AuTransporterTarget> > places;

			// Buildings
			{
				DaAtomList bds;
				env->FindBdRecursive(bds);
				for(DaAtomList::const_iterator bi = bds.cbegin(); bi != bds.cend(); bi++)
				{
					DaBd* bd = (DaBd*)(*bi);
					TRANSPORTER_TARGET targ = bd->TransporterTarget();
					if(targ == TT_Invalid)
						continue;

					// People can hail non-enemy cities to locate them so we will list them here.
					if(PoliticalStance(bd->EmpireId(), person->EmpireId()) == POL_Enemy &&
							bd->IndustryId() != IID_Capitol &&
							bd->IndustryId() != IID_AirportTerminal)
						continue;							// City does not show up on sensors and it does not answer hail.

					if(bd->EnvLocation().Distance2(shipSysLoc) > range2)
						continue;							// City is out of range of the ship's transporter.
					QString targetName = bd->TransporterTargetName();
					AuTransporterTarget tt(bd->Id(), targetName, targ);
					QMap<TRANSPORTER_TARGET, QMap<QString, AuTransporterTarget> >::iterator pti = places.find(tt.TargetType());
					if(pti == places.end())
					{
						QMap<QString, AuTransporterTarget> tm;
						tm.insert(targetName, tt);
						pti = places.insert(tt.TargetType(), tm);
					}
					else
						(*pti).insert(targetName, tt);
				}
			}

			// Items
			{
				DaAtomList items;
				env->FindItemsRecursive(items, false);
				for(DaAtomList::const_iterator bi = items.cbegin(); bi != items.cend(); bi++)
				{
					const DaItem* item = (const DaItem*)*bi;
					TRANSPORTER_TARGET targ = item->TransporterTarget();
					if(targ == TT_Invalid)
						continue;							// Item is not a transporter target.
					if(item->IsInCapsule() || item->IsAboardVehicle())
						continue;	// Cannot discriminate items aboard spacecraft or vehicles.
					if(item->EnvLocation().Distance2(shipSysLoc) > range2)
						continue;							// Item is out of range of the ship's transporter.
					QString targetName = item->TransporterTargetName();
					AuTransporterTarget tt(item->Id(), targetName, targ);
					QMap<TRANSPORTER_TARGET, QMap<QString, AuTransporterTarget> >::iterator pti = places.find(tt.TargetType());
					if(pti == places.end())
					{
						QMap<QString, AuTransporterTarget> tm;
						tm.insert(targetName, tt);
						pti = places.insert(tt.TargetType(), tm);
					}
					else
						(*pti).insert(targetName, tt);
				}
			}

			// Shipmates
			if(!shipId.IsNull())
			{
				DaAtomList mates;
				env->FindShipMatesRecursive(shipId, mates);
				for(DaAtomList::const_iterator mi = mates.cbegin(); mi != mates.cend(); mi++)
				{
					if((*mi)->IsInCapsule() || (*mi)->IsAboardVehicle())
						continue;	// Cannot discriminate ship mates aboard spacecraft or vehicles.
					if((*mi)->EnvLocation().Distance2(shipSysLoc) > range2)
						continue;							// Ship mate is out of range of the ship's transporter.
					QString targetName = (*mi)->TransporterTargetName();
					AuTransporterTarget tt((*mi)->Id(), targetName, (*mi)->TransporterTarget());
					QMap<TRANSPORTER_TARGET, QMap<QString, AuTransporterTarget> >::iterator pti = places.find(tt.TargetType());
					if(pti == places.end())
					{
						QMap<QString, AuTransporterTarget> tm;
						tm.insert(targetName, tt);
						pti = places.insert(tt.TargetType(), tm);
					}
					else
						(*pti).insert(targetName, tt);
				}
			}

			// Claim flag, indigenous village, and active henge are all sub target destination of a world atom.
			const DaWorldLL* world = dynamic_cast<const DaWorldLL*>(person->FindParentPlanetInEnv());
			if(world && world->FindEnvironmentAtom() == env)
			{
				// World is in the environment with the person.
				if(world->ClaimedByEmpire())
				{
					AuGlobeLLF claimLL = world->ClaimedAtLL();
					AuPoint claimLoc = world->LocOfLL(claimLL);
					claimLoc = world->EnvTransform().Project(claimLoc);
					if(claimLoc.Distance2(shipSysLoc) <= range2)
					{
						QString ename;
						const DgData& gd = g_Au.LockDataForRead();
						gd.EmpireName(world->ClaimedByEmpire(), ename);
						g_Au.UnlockData();
						QString targetName = tr("%1 Claim at %2").arg(ename).arg(claimLL.ToString());

						AuTransporterTarget tt(world->Id(), targetName, TT_ClaimFlag);
						QMap<TRANSPORTER_TARGET, QMap<QString, AuTransporterTarget> >::iterator pti =
							places.find(tt.TargetType());
						if(pti == places.end())
						{
							QMap<QString, AuTransporterTarget> tm;
							tm.insert(targetName, tt);
							pti = places.insert(tt.TargetType(), tm);
						}
						else
							(*pti).insert(targetName, tt);
					}
				}

				if(world->HasVillage())
				{
					AuGlobeLLF villageLL = world->VillageLL();
					AuPoint villageLoc = world->LocOfLL(villageLL);
					villageLoc = world->EnvTransform().Project(villageLoc);
					if(villageLoc.Distance2(shipSysLoc) <= range2)
					{
						QString targetName = tr("Village at %1").arg(villageLL.ToString());

						AuTransporterTarget tt(world->Id(), targetName, TT_Village);
						QMap<TRANSPORTER_TARGET, QMap<QString, AuTransporterTarget> >::iterator pti =
							places.find(tt.TargetType());
						if(pti == places.end())
						{
							QMap<QString, AuTransporterTarget> tm;
							tm.insert(targetName, tt);
							pti = places.insert(tt.TargetType(), tm);
						}
						else
							(*pti).insert(targetName, tt);
					}
				}

				if(world->IsHengeActive())
				{
					AuGlobeLLF hengeLL = world->HengeLL();
					AuPoint hengeLoc = world->LocOfLL(hengeLL);
					hengeLoc = world->EnvTransform().Project(hengeLoc);
					if(hengeLoc.Distance2(shipSysLoc) <= range2)
					{
						QString targetName = tr("Portal at %1").arg(hengeLL.ToString());

						AuTransporterTarget tt(world->Id(), targetName, TT_Henge);
						QMap<TRANSPORTER_TARGET, QMap<QString, AuTransporterTarget> >::iterator pti =
							places.find(tt.TargetType());
						if(pti == places.end())
						{
							QMap<QString, AuTransporterTarget> tm;
							tm.insert(targetName, tt);
							pti = places.insert(tt.TargetType(), tm);
						}
						else
							(*pti).insert(targetName, tt);
					}
				}

				const DaWorldRingSection* rsworld = dynamic_cast<const DaWorldRingSection*>(world);
				if(rsworld && !rsworld->SectionIndex())
				{
					// Section index 0 has the control center at LL(0,0).
					AuGlobeLLF rcLL = AuGlobeLLF(0,0);
					AuPoint rcLoc = world->LocOfLL(rcLL);
					rcLoc = world->EnvTransform().Project(rcLoc);
					if(rcLoc.Distance2(shipSysLoc) <= range2)
					{
						QString targetName = tr("Ringworld Control Center at %1").arg(rcLL.ToString());

						AuTransporterTarget tt(world->Id(), targetName, TT_RingworldControlCenter);
						QMap<TRANSPORTER_TARGET, QMap<QString, AuTransporterTarget> >::iterator pti =
							places.find(tt.TargetType());
						if(pti == places.end())
						{
							QMap<QString, AuTransporterTarget> tm;
							tm.insert(targetName, tt);
							pti = places.insert(tt.TargetType(), tm);
						}
						else
							(*pti).insert(targetName, tt);
					}
				}

				// Geological features.
				int targ = TT_GeologicalFeatureFirst;
				const AuGeologicalFeatures& geof = world->LockGeologicalFeatures();
				for(AuGeologicalFeatures::const_iterator gi = geof.cbegin(); gi != geof.cend(); gi++)
				{
					if(!(*gi)->IsTransporterTarget())
						continue;
					AuGlobeLLF geoLL = (*gi)->LL();
					AuPoint geoLoc = world->LocOfLL(geoLL);
					geoLoc = world->EnvTransform().Project(geoLoc);
					if(geoLoc.Distance2(shipSysLoc) <= range2)
					{
						QString targetName = tr("%1 at %2")
							.arg((*gi)->TransporterName(world->HydrographicLiquid()))
							.arg(geoLL.ToString());

						AuTransporterTarget tt(world->Id(), targetName, (TRANSPORTER_TARGET)targ);
						QMap<TRANSPORTER_TARGET, QMap<QString, AuTransporterTarget> >::iterator pti =
							places.find(tt.TargetType());
						if(pti == places.end())
						{
							QMap<QString, AuTransporterTarget> tm;
							tm.insert(targetName, tt);
							pti = places.insert(tt.TargetType(), tm);
						}
						else
							(*pti).insert(targetName, tt);
					}
					targ++;
					if(targ > TT_GeologicalFeatureLast)
						break;
				}
				world->UnlockGeologicalFeatures();
			}

			for(QMap<TRANSPORTER_TARGET, QMap<QString, AuTransporterTarget> >::const_iterator ti = places.cbegin();
					ti != places.cend(); ti++)
				for(QMap<QString, AuTransporterTarget>::const_iterator pi = (*ti).cbegin(); pi != (*ti).cend(); pi++)
					ttplaces.append(*pi);
		}
	}
	UnlockAvatar();
}

// Responds to client request for all spacecraft that are in range of the transporter room in which they are standing.
void AuAtomInterface::TransporterSpacecraft(QList<AuTransporterShip>& ttships)
{
	DaPerson* person = LockAvatar();
	DaAtom* system = person ? person->FindEnvironmentAtom() : 0;
	if(person && system)
	{
		if(person->InTransporterRoom())
		{
			// Person is in a transporter room.
			DaCapsule* ship = person->FindParentCapsuleInEnv();
			if(ship)
			{
				// Determine the range of the transporter.
				double range = ship->TransporterRange(person->TransporterSystemWhich());
				if(range > 0)
				{
					// Locate all spacecraft in the solar system.
					DaAtomList ships;
					system->FindVesselsRecursive(ships);

					// Remove ships from the list that are not in range of the transporter.
					double range2 = range * range;				// squared distance for comparison without sqrt
					AuPoint shipSysLoc = ship->EnvLocation();
					DaAtomList::iterator si = ships.begin();
					while(si != ships.end())
					{
						DaVessel* otherShip = (DaVessel*)(*si);
						if(otherShip == ship)
						{
							si = ships.erase(si);				// Do not offer my current ship as a destination.
							continue;
						}
						if(otherShip->IsInSpacecraft())
						{
							// Space ships don't show up as separate when subordinate to other spacecraft.
							si = ships.erase(si);
							continue;
						}
						if(otherShip->IsHidingInGroundClutter())
						{
							// Other ship is hiding in ground clutter.
							si = ships.erase(si);
							continue;
						}
						if(otherShip->IsNew())
						{
							// Cannot beam aboard nu ships of other empires.
							POLITICAL_STANCE stance = PoliticalStance(otherShip->EmpireId(), person->EmpireId());
							if(stance == POL_Enemy || stance == POL_Neutral)
							{
								// New ship's stance toward my empire prevents me from beaming aboard.
								si = ships.erase(si);
								continue;
							}
						}

						AuPoint otherShipLoc = otherShip->EnvLocation();
						if(otherShipLoc.Distance2(shipSysLoc) > range2)
							si = ships.erase(si);				// Other ship is out of range of the ship's transporter.
						else
							si++;								// Other ship is in range of the ship's transporter.
					}

					for(DaAtomList::const_iterator si = ships.cbegin(); si != ships.cend(); si++)
						ttships.append(AuTransporterShip((*si)->Id(), (*si)->Name(), (*si)->SpotDescription(person->EmpireId()),
									(*si)->TransporterTarget()));
				}
			}
		}
		else
		{
			VEHICLE_SPOT_TYPE spotType = person->InVehicleTransporter();
			if(spotType != SPOT_Invalid)
			{
				// Person is in a vehicle that is parked where it can be transported.
				DaCapsule* ship = person->FindParentCapsuleInEnv();
				if(ship)
				{
					// Determine the range of the transporter.
					double range = ship->TransporterRange(ship->VehicleTransporterSystemId(spotType, false));
					if(range > 0)
					{
						// Locate all spacecraft in the solar system.
						DaAtomList ships;
						system->FindVesselsRecursive(ships);

						// Remove ships from the list that are not in range of the transporter.
						double range2 = range * range;		// squared distance for comparison without sqrt
						AuPoint shipSysLoc = ship->EnvLocation();
						DaAtomList::iterator si = ships.begin();
						while(si != ships.end())
						{
							DaVessel* otherShip = (DaVessel*)(*si);
							if(otherShip == ship)
							{
								si = ships.erase(si);		// Do not offer my current ship as a destination.
								continue;
							}
							if(otherShip->IsInSpacecraft())
							{
								// Space ships don't show up as separate when subordinate to other spacecraft.
								si = ships.erase(si);
								continue;
							}
							if(otherShip->IsHidingInGroundClutter())
							{
								// Other ship is hiding in ground clutter.
								si = ships.erase(si);
								continue;
							}
							if(otherShip->IsNew())
							{
								// Cannot beam aboard nu ships of other empires.
								POLITICAL_STANCE stance = PoliticalStance(otherShip->EmpireId(), person->EmpireId());
								if(stance == POL_Enemy || stance == POL_Neutral)
								{
									// New ship's stance toward my empire prevents me from beaming aboard.
									si = ships.erase(si);
									continue;
								}
							}

							AuPoint otherShipLoc = otherShip->EnvLocation();
							if(otherShipLoc.Distance2(shipSysLoc) > range2)
								si = ships.erase(si);		// Other ship is out of range of the ship's transporter.
							else
								si++;						// Other ship is in range of the ship's transporter.
						}

						for(DaAtomList::const_iterator si = ships.cbegin(); si != ships.cend(); si++)
							ttships.append(AuTransporterShip((*si)->Id(), (*si)->Name(), (*si)->SpotDescription(person->EmpireId()),
										(*si)->TransporterTarget()));
					}
				}
			}
		}
	}
	UnlockAvatar();
}

// Responds to client request for all transporters that are in range of the transporter room in which they are standing.
void AuAtomInterface::TransporterTransporters(QList<AuTransporterSystem>& tttrans)
{
	DaPerson* person = LockAvatar();
	DaAtom* system = person ? person->FindEnvironmentAtom() : 0;
	if(person && system)
	{
		if(person->InTransporterRoom())
		{
			// Person is in a transporter room.
			DaCapsule* ship = person->FindParentCapsuleInEnv();
			if(ship)
			{
				// Determine the range of the transporter.
				double range = ship->TransporterRange(person->TransporterSystemWhich());
				if(range > 0)
				{
					// Locate all spacecraft in the solar system.
					DaAtomList ships;
					system->FindCapsulesRecursive(ships);

					// Remove ships from the list that are not in range of the transporter and that do not have transporters.
					double range2 = range * range;				// squared distance for comparison without sqrt
					AuPoint shipSysLoc = ship->EnvLocation();
					DaAtomList::iterator si = ships.begin();
					while(si != ships.end())
					{
						DaCapsule* otherShip = (DaCapsule*)(*si);
						if(otherShip == ship && ship->TransporterSystems() < 2)
						{
							si = ships.erase(si);				// Do not offer my current ship as a destination.
							continue;
						}
						if(!otherShip->HasTransporter())
						{
							si = ships.erase(si);				// Other ship does not have transporters.
							continue;
						}
						if(otherShip->IsInSpacecraft())
						{
							// Space ships don't show up as separate when subordinate to other spacecraft.
							si = ships.erase(si);
							continue;
						}
						if(otherShip->IsHidingInGroundClutter())
						{
							// Other ship is hiding in ground clutter.
							si = ships.erase(si);
							continue;
						}
						if(otherShip->IsNew())
						{
							// Cannot beam to transporter of nu ships because there is no power to turn them on.
							si = ships.erase(si);
							continue;
						}

						AuPoint otherShipLoc = otherShip->EnvLocation();
						if(otherShipLoc.Distance2(shipSysLoc) > range2)
							si = ships.erase(si);				// Other ship is out of range of the ship's transporter.
						else
							si++;								// Other ship is in range of the ship's transporter.
					}

					// Notify client of transporters that are in transporter range.
					if(!ships.isEmpty())
					{
						for(DaAtomList::const_iterator si = ships.cbegin(); si != ships.cend(); si++)
						{
							const DaCapsule* cap = (const DaCapsule*)*si;
							QList<quint16> whiches = cap->TransporterSystemWhiches();
							for(QList<quint16>::const_iterator wi = whiches.cbegin(); wi != whiches.cend(); wi++)
								tttrans.append(AuTransporterSystem((*si)->Id(), (*si)->Name(),
											(*si)->SpotDescription(person->EmpireId()), *wi));
						}
					}
				}
			}
		}
		else
		{
			VEHICLE_SPOT_TYPE spotType = person->InVehicleTransporter();
			if(spotType != SPOT_Invalid)
			{
				// Person is in a vehicle that is parked where it can be transported.
				DaCapsule* ship = person->FindParentCapsuleInEnv();
				if(ship)
				{
					// Determine the range of the transporter.
					double range = ship->TransporterRange(ship->VehicleTransporterSystemId(spotType, false));
					if(range > 0)
					{
						// Locate all spacecraft in the solar system.
						DaAtomList ships;
						system->FindCapsulesRecursive(ships);

						// Remove ships from the list that are not in range and that do not have vehicle transporters.
						double range2 = range * range;		// squared distance for comparison without sqrt
						AuPoint shipSysLoc = ship->EnvLocation();
						DaAtomList::iterator si = ships.begin();
						while(si != ships.end())
						{
							DaCapsule* otherShip = (DaCapsule*)(*si);
							if(otherShip == ship && ship->TransporterSystems() < 2)
							{
								si = ships.erase(si);		// Do not offer my current ship as a destination.
								continue;
							}
							if(!otherShip->HasVehicleTransporter())
							{
								si = ships.erase(si);		// Other ship does not have vehicle transporters.
								continue;
							}
							if(otherShip->IsInSpacecraft())
							{
								// Space ships don't show up as separate when subordinate to other spacecraft.
								si = ships.erase(si);
								continue;
							}
							if(otherShip->IsHidingInGroundClutter())
							{
								// Other ship is hiding in ground clutter.
								si = ships.erase(si);
								continue;
							}
							if(otherShip->IsNew())
							{
								// Cannot beam to transporter of nu ships because there is no power to turn them on.
								si = ships.erase(si);
								continue;
							}

							AuPoint otherShipLoc = otherShip->EnvLocation();
							if(otherShipLoc.Distance2(shipSysLoc) > range2)
								si = ships.erase(si);		// Other ship is out of range of the ship's transporter.
							else
								si++;						// Other ship is in range of the ship's transporter.
						}

						for(DaAtomList::const_iterator si = ships.cbegin(); si != ships.cend(); si++)
						{
							const DaCapsule* cap = (const DaCapsule*)*si;
							QList<quint16> whiches = cap->TransporterSystemWhiches();
							for(QList<quint16>::const_iterator wi = whiches.cbegin(); wi != whiches.cend(); wi++)
								tttrans.append(AuTransporterSystem((*si)->Id(), (*si)->Name(),
											(*si)->SpotDescription(person->EmpireId()), *wi));
						}
					}
				}
			}
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::ArenaList(unordered_map<AuId, AuArenaReady, AuIdHash>& arenas)
{
	arenas = g_Au.LockArenasForRead();
	g_Au.UnlockArenas();
}

void AuAtomInterface::HangarQuery(QString& shipDesc, quint32& shipDesign, quint32& shipsInHangar)
{
	//qDebug() << "DtChatChannel::HangarQuery";
	shipDesign = 0;
	shipsInHangar = 0;
	DaPerson* person = LockAvatar();
	if(person && person->InIndustry() == IID_Hangar)
	{
		DaVessel* hangarShip = 0;
		DaBd* bd = person->BdOp(AuId());
		if(bd)
		{
			shipsInHangar = bd->CountShipsInHangar();
			hangarShip = bd->FindShipInHangar(person->Id());
			if(hangarShip)
			{
				shipDesc = hangarShip->NameWithEmpire() + ", " + hangarShip->DesignName();
				shipDesign = hangarShip->DesignId();
			}
		}
	}
	UnlockAvatar();
}

void AuAtomInterface::TransferListCitizens(const AuId& shipId, QByteArray& bacit)
{
	DaVessel* ship = 0;
	DaVessel* otherShip = 0;
	TransferSetup(shipId, &ship, &otherShip);	// This will return false if 'shipId' is my ship but the result is ok here.

	// Get a list of all the citizens aboard who could be transferred.
	DaAtomList citizensAboard;
	if(ship && ship->Is(shipId))
	{
		ship->FindCrewRecursive(ship->Id(), citizensAboard);
		ship->FindOfficersRecursive(ship->Id(), citizensAboard);
		ship->FindPassengersRecursive(ship->Id(), citizensAboard);
		ship->FindTroopsRecursive(ship->Id(), citizensAboard);
	}
	else if(otherShip)
	{
		otherShip->FindCrewRecursive(otherShip->Id(), citizensAboard);
		otherShip->FindOfficersRecursive(otherShip->Id(), citizensAboard);
		otherShip->FindPassengersRecursive(otherShip->Id(), citizensAboard);
		otherShip->FindTroopsRecursive(otherShip->Id(), citizensAboard);
	}

	QDataStream out(&bacit, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << shipId;
	for(DaAtomList::const_iterator ci = citizensAboard.cbegin(); ci != citizensAboard.cend(); ci++)
	{
		const DaCitizen* citizen = dynamic_cast<const DaCitizen*>(*ci);
		if(!citizen)
			continue;

		// Name of the species, or a generic name when the species is not named.
		QString name = tr("<br>%1 %2").arg(citizen->SpeciesName()).arg(citizen->FemaleMale(true));

		// Promotion Goal
		if(citizen->PromotionGoal())
			name += tr("<br>%L1xp.<br>Next Promotion at %L2xp.")
				.arg(citizen->JobExperiencePoints()).arg(citizen->PromotionGoal());
		else
			name += tr("<br>%L1xp").arg(citizen->JobExperiencePoints());

		// Rank icon
		QString icon = citizen->RankIcon();

		// Status
		QString status, statusShort;
		{
			if(citizen->CurrentHitPoints() < citizen->MaxHitPoints())
				status = tr("%1/%2hp").arg(citizen->CurrentHitPoints()).arg(citizen->MaxHitPoints());
			else
				status = tr("%1hp").arg(citizen->CurrentHitPoints());
			statusShort = status;
			if(citizen->CurrentHitPoints() <= 0)
				status = tr("Dying") + " " + status;
			else if(citizen->CurrentHitPoints() < citizen->MaxHitPoints())
				status = tr("Wounded") + " " + status;
			else
				status = tr("Unhurt") + " " + status;

			// Determine combat armor factors based on current armor.
			float baseAbsorb, baseBlock, auxAbsorb, auxBlock;
			citizen->CombatArmorEffectiveness(WM_Contact, WE_Generic, false, AuPoint(0,0), AuPoint(0,0), baseAbsorb, baseBlock,
			auxAbsorb, auxBlock);
			float absorbK = baseAbsorb + auxAbsorb;
			float blockK = baseBlock + auxBlock;
			status += tr("<br>Armor: %1").arg(g_Au.FormatArmorFactors(absorbK, blockK));

			status += tr("<br>Alive Since %1").arg(citizen->CreatedTime().UTC().toLocalTime().toString());
		}

		// Add a citizen to the blob.
		out << citizen->Id();
		out << citizen->Name();
		out << (quint8)citizen->AtomType();
		out << name;
		out << icon;
		out << status;
		out << statusShort;
	}
	out << AuId(0);	// Marks the end of the citizens list.
}

double AuAtomInterface::SensorContacts(AuSensorBlipList& sensorBlips, AuPoint& sensorEnvLoc)
{
	double sensorRange = 0;
	DaPerson* person = LockAvatar();
	if(person)
	{
		// Found the scene atom and it's a person.
		DaCapsule* cap = person->FindParentCapsuleInEnv();
		if(cap && (cap->IsCrewOrTroop(person->Id()) || cap->InChainOfCommand(person)))
		{
			// The person is a crew member of the spacecraft they are aboard.
			cap->PerformSensorConsoleScan(sensorBlips);
			sensorEnvLoc = cap->EnvLocation();
			sensorRange = cap->SensorRange();
		}
	}
	UnlockAvatar();
	return sensorRange;
}

void AuAtomInterface::OrdnanceList(AuId& loaderId, AuItemHash& ordnance)
{
	// Get the list of ordnance available.
	DaPerson* person = LockAvatar();
	if(person)
	{
		const DaVehicle* vehicle = dynamic_cast<const DaVehicle*>(person->Parent());
		const DaVessel* ship = dynamic_cast<const DaVessel*>(person->Parent());
		if(vehicle)
		{
			// Only certain locations can provide ordnance.
			INDUSTRYID iid = vehicle->InIndustry();
			if(DbIndustry::CanSupplyOrdnance(iid))
			{
				const DaWorld* world = vehicle->FindParentPlanetInEnv();
				if(world)
				{
					if(!vehicle->BdId().IsNull())
					{
						const DaBd* bd = world->FindBd(vehicle->BdId());
						if(bd)
						{
							loaderId = bd->Id();
							//DHH bd->ListOrdnanceAvailable(ordnance, vehicle->EmpireId());
						}
					}
				}
			}
			else
			{
				const DaVessel* ship = vehicle->FindParentVesselInEnv();
				if(ship)
				{
					loaderId = ship->Id();
					ship->ListOrdnanceAvailable(ordnance, vehicle->EmpireId());
				}
				else
				{
					bool hasStation = false;
					DaVessel* station = vehicle->StationInTradeRange(vehicle->RoughRadius(), hasStation);
					if(station)
					{
						loaderId = station->Id();
						station->ListOrdnanceAvailable(ordnance, vehicle->EmpireId());
					}
				}
			}
		}
		else if(ship && (person->IsTurretPose() || person->IsFireControlStationPose()))
		{
			loaderId = ship->Id();
			ship->ListOrdnanceAvailable(ordnance, ship->EmpireId());
		}
	}
	UnlockAvatar();
}

//static
void AuAtomInterface::CollectStarMapData(quint32 empireId, AuStarMapData& starMap, int query, GALAXY gal, const AuId& sectorId,
		unsigned int range, const AuId& personSystemId)
{
	// Find every solar system that has been explored by the empire.
	GALAXY cengal;
	AuPoint cenloc;
	AuEther& ether = g_Au.LockEtheriumDelete();
	switch(query)
	{
	case 0:	// universe
	case 1:	// galaxy
	default:
		cengal = GALAXY_Count;
		break;
	case 2:	// sector
		{
			const DaAtom* censect = ether.FindNormalAtom(sectorId);
			if(!censect)
			{
				g_Au.UnlockEtheriumDelete();
				return;
			}
			cengal = censect->Galaxy();
			cenloc = censect->Location();	// in pc.
		}
		break;
	}
	DaAtomList systems;
	ether.ListNormalSolarSystems(systems);
	for(DaAtomList::const_iterator ai = systems.cbegin(); ai != systems.cend(); ai++)
	{
		const DaSolarSystem* rootSystem = dynamic_cast<const DaSolarSystem*>(*ai);
		if(!rootSystem || !rootSystem->IsNormal())
			continue;
		if(!rootSystem->IsExploredBy(empireId) && rootSystem->DetectedSignalStrength(empireId) == 0)
		{
			//qDebug() << "DtSceneServer::CollectStarMapData system" << rootSystem->Id() << rootSystem->Name()
				//<< "not explored by empire" << empireId;
			continue;
		}

		// Found a system explored or detected by the empire. We will list every solar system in the sector so the galaxy viewer
		// shows all the stars in the sectors that it does show.
		const DaSector* sector = dynamic_cast<const DaSector*>(rootSystem->Parent());
		if(!sector)
		{
			//qDebug() << "DtSceneServer::CollectStarMapData system" << rootSystem->Id() << rootSystem->Name()
				//<< "parent atom is not a sector";
			continue;				// Not a sector or the sector has no subatoms.
		}
		const DaGalaxy* galaxy = sector->FindParentGalaxy();
		if(!galaxy)
			continue;

		// We have the galaxy and sector. Test based on mode.
		switch(query)
		{
		case 0:	// universe
		default:
			break;
		case 1:	// galaxy
			if(galaxy->Galaxy() != gal)
				continue;
			break;
		case 2:	// sector
			if(galaxy->Galaxy() != cengal)
				continue;	// Sector is in the wrong galaxy.
			else
			{
				double secrange = sector->Location().Distance(cenloc);	// in pc.
				secrange /= 10;	// distance between sectors in sectors.
				if((unsigned int)floor(secrange) > range)
					continue;	// sector is too far away
			}
			break;
		}

		if(starMap.m_Sectors.find(sector->Id()) != starMap.m_Sectors.cend())
		{
			//qDebug() << "DtSceneServer::CollectStarMapData system" << rootSystem->Id() << rootSystem->Name()
				//<< "sector" << sector->Id() << sector->Name() << "already processed";
			continue;				// The sector has already been processed.
		}

		// Does the empire know the name of the sector?

		// Create a sector record for the viewer.
		AuStarMapSector gvsector;
		sector->StarMapInfo(empireId, gvsector);

		// Fill the sector record with information about its solar systems.
		DaAtomList subs;
		sector->ListNormalSubatoms(subs);
		for(DaAtomList::const_iterator si = subs.cbegin(); si != subs.cend(); si++)
		{
			const DaSolarSystem* system = dynamic_cast<const DaSolarSystem*>(*si);
			if(!system)
				continue;

			// Create a system record for the viewer.
			AuStarMapSystem gvsystem;
			system->StarMapInfo(empireId, gvsystem);

			if(gvsystem.m_InfoBits & AuStarMapSystem::Explored)
			{
				// Fill the system record with information about all cities and spacecraft in explored systems.
				// We may suppress this information if there is no way to get current intel on the system.
				// Master server does not load city atoms but it does have empire records. It's painful but we can search them.
				bool ecity = false;		// true if any empire city in the system
				bool eradar = false;	// true if any empire radar in the system
				bool enearby = false;	// true if any empire city with an airport or air base is near the system
				const DgData& gd = g_Au.LockDataForRead();
				for(DgEmpireHash::const_iterator ei = gd.m_gEmpires.cbegin(); ei != gd.m_gEmpires.cend(); ei++)
				{
					for(AuCityHash::const_iterator ci = (*ei).second.m_Cities->cbegin(); ci != (*ei).second.m_Cities->cend(); ci++)
					{
						if((*ci).second.SystemId() == system->Id())
						{
							// City is in the system.
							// Create a city record for the viewer.
							AuStarMapCity gvcity;
							gvcity.m_Name = (*ci).second.CityName();
							gvcity.m_Empire = (*ei).second.m_id;
							if((*ci).second.Facilities().HasAirBase())
								gvcity.SetAirBase();
							if((*ci).second.Facilities().HasAirport())
								gvcity.SetAirport();
							if((*ci).second.Facilities().HasBaseHQ())
								gvcity.SetBaseHeadquarters();
							if((*ci).second.Facilities().HasCapitol())
								gvcity.SetCapitolBuilding();
							if((*ci).first == (*ei).second.m_capitol)
								gvcity.SetEmpireCapital();
							if((*ci).second.IsGalaxyCapital())
								gvcity.SetGalaxyCapital();
							if((*ci).second.IsSectorCapital())
								gvcity.SetSectorCapital();
							gvsystem.m_Cities.insert(pair<AuId, AuStarMapCity>((*ci).first, gvcity));

							if((*ei).first == empireId)
							{
								ecity = true;
								if(!eradar &&
										((*ci).second.Facilities().HasAirportRadar() || (*ci).second.Facilities().HasBaseRadar()))
									eradar = true;
							}
						}
						else if(!enearby)
						{
							// City is not in the system and we haven't found a city nearby yet.
							if((*ei).first != empireId)
								continue;	// City is not friendly enough to provide us with information.
							if(!(*ci).second.Facilities().HasAirBase() && !(*ci).second.Facilities().HasAirport())
								continue;	// City has no facilities for space travellers.
							// Check distance to the city.
							if((*ci).second.Galaxy() != system->Galaxy())
								continue;	// City is in a distant galaxy.
							double dist = (*ci).second.SystemLoc().Distance(sector->Location() + system->Location());	// in pc
							// The diagonal distance across a sector is 17.32 = √(10² + 10² + 10²) parsecs.
							if(dist <= 17.32)
								enearby = true;
						}
					}
				}
				g_Au.UnlockData();

				// Master server receives position reports from all spacecraft for this moment.
				bool eship = false;		// true if any non-new empire ships are present in the system, not deep space
				bool esensors = false;	// true if any non-new empire ships are present in the system that have functioning sensors
				QMultiHash<AuId, AuStarMapScPos>& scpos = g_Au.LockScPositionReports();
				for(QMultiHash<AuId, AuStarMapScPos>::iterator si = scpos.find(system->Id());
						si != scpos.end() && si.key() == system->Id(); si++)
				{
					if((*si).EmpireId() != empireId)
						continue;				// Ship is not empire.
					if(!(*si).IsNew() && (*si).IsInSystem())
					{
						eship = true;			// non-new ship present in the system, not in deep space
						if((*si).HasSensors())
							esensors = true;	// Sensors in system can be used.
					}
					gvsystem.m_Spacecraft.insert(pair<AuId, AuStarMapSpacecraft>((*si).Id(), *si));
				}
				g_Au.UnlockScPositionReports();

				if(eradar || esensors)
				{
					// Empire sensors that are not in deep space will detect all other spacecraft in the system that
					// are also not in deep space.
					QMultiHash<AuId, AuStarMapScPos>& scpos = g_Au.LockScPositionReports();
					for(QMultiHash<AuId, AuStarMapScPos>::iterator si = scpos.find(system->Id());
							si != scpos.end() && si.key() == system->Id(); si++)
					{
						if(gvsystem.m_Spacecraft.find((*si).Id()) != gvsystem.m_Spacecraft.cend())
							continue;	// Skip ships we already listed.
						if(!(*si).IsInSystem())
							continue;	// Skip ships that are in deep space.
						gvsystem.m_Spacecraft.insert(pair<AuId, AuStarMapSpacecraft>((*si).Id(), *si));
					}
					g_Au.UnlockScPositionReports();
				}
				else if(!ecity && !eship && !enearby && !system->Is(personSystemId))
				{
					// No radars, cities, or non-new ships in the system; no spaceports nearby; scene atom is not in the system.
					// We cannot know about cities that are there, just because the system was explored once.
					gvsystem.m_Cities.clear();
				}
			}

			gvsector.m_Systems.insert(pair<AuId, AuStarMapSystem>(system->Id(), gvsystem));
		}

		starMap.m_Sectors.insert(pair<AuId, AuStarMapSector>(sector->Id(), gvsector));
		//qDebug() << "DtSceneServer::CollectStarMapData sector" << sector->Id() << sector->Name() << "processed,"
			//<< gvsector.m_Systems.count() << "systems";
	}
	g_Au.UnlockEtheriumDelete();
}

int AuAtomInterface::BrokerShipments(AuShipmentList& shipments, AuTradeAgreementList& tas, const AuId& opBd)
{
	int result = 1;
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaBd* bd = person->BdOp(opBd);
		if(bd)
		{
			if(bd->InChainOfCommand(person))
			{
				const AuShipmentList& cityShipments = bd->LockShipmentsForRead();
				for(AuShipmentList::const_iterator si = cityShipments.cbegin(); si != cityShipments.cend(); si++)
					shipments.AddShipment(*si);
				bd->UnlockShipments();

				const AuTradeAgreementList& tradeAgreements = bd->LockTradeAgreementsForRead();
				for(AuTradeAgreementList::const_iterator si = tradeAgreements.cbegin(); si != tradeAgreements.cend(); si++)
					tas.AddTradeAgreement(*si);
				bd->UnlockTradeAgreements();
				result = 0;
			}
			else
				result = 3;
		}
		else
			result = 2;
	}
	UnlockAvatar();
	return result;
}

int AuAtomInterface::Shipments(AuShipmentList& shipments, bool& isStation)
{
	int result = 1;
	DaPerson* person = LockAvatar();
	if(person)
	{
		DaVessel* ves = person->FindParentVessel();
		if(ves)
		{
			isStation = ves->IsSpaceStation();
			if(ves->InChainOfCommand(person))
			{
				const AuShipmentList& stationShipments = ves->LockShipmentsForRead();
				for(AuShipmentList::const_iterator si = stationShipments.cbegin(); si != stationShipments.cend(); si++)
					shipments.AddShipment(*si);
				ves->UnlockShipments();
				result = 0;
			}
			else
				result = 3;
		}
		else
			result = 2;
	}
	UnlockAvatar();
	return result;
}

//static
void AuAtomInterface::ReleaseBerthDown(const AuId& personId, const AuId& shipId)
{
	// The personId is removed from the roster of every spacecraft in my
	// atom list, except the 'shipId' that the person is now berthed with.
	DaAtomList spacecraft;
	DaAtomList atoms;
	AuEther& ether = g_Au.LockEtheriumDelete();
	ether.ListAtoms(atoms);
	for(DaAtomList::const_iterator it = atoms.cbegin(); it != atoms.cend(); it++)
	{
		DaVessel* ship = dynamic_cast<DaVessel*>(*it);
		if(!ship)
			continue;
		if(ship->Is(shipId))
			continue;	// Skip the ship that the person has tied to.
		ship->RemoveBerthOccupant(personId);
		ship->KillDormantAbandoned();					// Dormant ship could be deleted when a person leaves the roster.
	}

	// When the nu ship id is null, the person has released their berth or
	// been kicked. In this case, we must find that person and clear their
	// spawn ship id.
	if(shipId.IsNull())
	{
		DaAtom* atom = ether.FindNormalAtom(personId);
		if(atom)
		{
			DaPerson* person = dynamic_cast<DaPerson*>(atom);
			if(person)
			{
				// The person was found in this server's atom data. Clear their spawn ship id.
				person->ClearSpawnShip();
				person->SetModified();
			}
			else
			{
				// Citizens also occupy berths.
				DaCitizen* citizen = dynamic_cast<DaCitizen*>(atom);
				if(citizen)
				{
					citizen->AssignToShip(AuId(), AuRosterEntry::Invalid);
					citizen->SetModified();
				}
			}
		}
	}
	g_Au.UnlockEtheriumDelete();
}

void AuAtomInterface::GovSetJoinPassword(const QByteArray& passCrypt)
{
	bool hasAuthority = false;
	AuId personId;
	QString personName;
	quint32 empireId = 0;
	DaPerson* person = LockAvatar();
	if(person)
	{
		// Insure the person has authority.
		personId = person->Id();
		personName = person->Name();
		empireId = person->EmpireId();
		const DgData& gd = g_Au.LockDataForRead();
		hasAuthority = gd.EmpireCitizenCanSetJoinPassword(empireId, person->Id());
		g_Au.UnlockData();
	}
	UnlockAvatar();

	QString resultMsg;
	if(hasAuthority)
	{
		emit GovSetJoinPassword(empireId, passCrypt);
		resultMsg = tr("Empire join password updated.");
	}
	else
		resultMsg = tr("Only authorized personnel can change empire join password.");

	if(!resultMsg.isEmpty())
	{
		// Send the result on the Log channel.
		AuCommThought msg(resultMsg, personId, personName);
		g_Au.SendAtomChat(msg);
	}
}

/*
void AuAtomInterface::
{
	DaPerson* person = LockAvatar();
	if(person)
	UnlockAvatar();
}

*/
