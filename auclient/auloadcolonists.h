//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auloadcolonists.h,v 1.6 2022/02/19 17:58:00 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuLoadColonists_H
#define AuLoadColonists_H

#include "auclientlib.h"
#include <audialog.h>

class AUCLIENT_DLL AuLoadColonists : public AuDialog
{
	Q_OBJECT;
public:
	unsigned int			m_Crew;			// Set to max on entry. Set to count on exit.
	unsigned int			m_Cryo;			// Set to max on entry. Set to count on exit.
	unsigned int			m_Officers;		// Set to max on entry. Set to count on exit.
	unsigned int			m_Passengers;	// Set to max on entry. Set to count on exit.
	unsigned int			m_Troops;		// Set to max on entry. Set to count on exit.

public:
	AuLoadColonists(unsigned int officers, unsigned int crew, unsigned int cryo, unsigned int passengers, unsigned int troops,
			bool isInVehicle, QWidget* parent=0);
	virtual ~AuLoadColonists();

	unsigned int CrewLoaded() const
	{
		// Returns the final value the user entered if the dialog is accepted.
		return m_Crew;
	}

	unsigned int CryoLoaded() const
	{
		// Returns the final value the user entered if the dialog is accepted.
		return m_Cryo;
	}

	unsigned int OfficersLoaded() const
	{
		// Returns the final value the user entered if the dialog is accepted.
		return m_Officers;
	}

	unsigned int PassengersLoaded() const
	{
		// Returns the final value the user entered if the dialog is accepted.
		return m_Passengers;
	}

	unsigned int TroopsLoaded() const
	{
		// Returns the final value the user entered if the dialog is accepted.
		return m_Troops;
	}

public slots:
	void EnableControls();
	void OfferCrewPressed();
	void OfferCryoPressed();
	void OfferOfficersPressed();
	void OfferPassengersPressed();
	void OfferTroopsPressed();
};

#endif // AuLoadColonists_H
