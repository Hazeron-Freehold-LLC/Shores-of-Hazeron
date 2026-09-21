//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auspacecraftaccount.h,v 1.9 2022/05/04 21:01:54 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuSpacecraftAccount_H
#define AuSpacecraftAccount_H

#include "auclientlib.h"
#include "audockwidget.h"
#include "auglobal.h"
#include "auid.h"
#include "auroster.h"
#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QTableWidget;
class QTimer;

class AUCLIENT_DLL AuSpacecraftAccount : public QWidget
{
	Q_OBJECT;
private:
	QTimer*			m_timer;		// For updates.

	// Widgets
	QLineEdit*		lefunds;
	QLineEdit*		leplayerfunds;
	QLineEdit*		leshipfunds;
	QLineEdit*		leshiptaxfunds;
	QPushButton*	pbdeposit;
	QPushButton*	pbwithdraw;

public:
	AuSpacecraftAccount(QWidget* parent);
	virtual ~AuSpacecraftAccount();

	virtual QSize sizeHint() const;

signals:
	void SendShipFunds(double amount, AuId idShip);

	void ReceiveShipFunds(double amount, AuId idShip);

protected:
	virtual void showEvent(QShowEvent* event);

protected slots:
	void EnableControls();
	void ReceiveFunds();
	void SendFunds();
	void UpdateContents();
	void UpdateText();
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUCLIENT_DLL AuSpacecraftAccountDock : public AuDockWidget
{
public:
	AuSpacecraftAccount*				m_Content;

public:
	AuSpacecraftAccountDock(QWidget* parent) : AuDockWidget(":data/c_Money.png", tr("<b>Spacecraft Account</b>"),
			tr("Account"), parent)
	{
		m_Content = new AuSpacecraftAccount(this);
		setWidget(m_Content);

		int dockSides = g_Au.Settings().readNumEntry("Dock/SpacecraftAccountAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/SpacecraftAccountAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuSpacecraftAccount_H
