//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auunloadcolonists.h,v 1.6 2022/02/19 18:02:56 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuUnloadColonists_H
#define AuUnloadColonists_H

#include "auclientlib.h"
#include <audialog.h>

class AUCLIENT_DLL AuUnloadColonists : public AuDialog
{
	Q_OBJECT;
public:
	unsigned int			m_Cryo;
	unsigned int			m_Passengers;	// Set to max on entry. Set to count on exit.
	unsigned int			m_Troops;		// Set to max on entry. Set to count on exit.

public:
	AuUnloadColonists(unsigned int cryo, unsigned int passengers, unsigned int troops, bool isInVehicle, QWidget* parent=0);
	virtual ~AuUnloadColonists();

	unsigned int CryoUnloaded() const
	{
		// Returns the final value the user entered if the dialog is accepted.
		return m_Cryo;
	}

	unsigned int PassengersUnloaded() const
	{
		// Returns the final value the user entered if the dialog is accepted.
		return m_Passengers;
	}

	unsigned int TroopsUnloaded() const
	{
		// Returns the final value the user entered if the dialog is accepted.
		return m_Troops;
	}

public slots:
	void EnableControls();
	void OfferCryoPressed();
	void OfferPassengersPressed();
	void OfferTroopsPressed();
};

#endif // AuUnloadColonists_H
