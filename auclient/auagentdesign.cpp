//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auagentdesign.cpp,v 1.7 2022/02/19 18:03:30 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auagentdesign.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "dadesignbase.h"
#include "dgdata.h"
#include <QGridLayout>
#include <QInputDialog>
#include <QPushButton>

AuAgentDesign::AuAgentDesign(QWidget* parent) : QWidget(parent)
{
	const DgData& gd = g_Au.LockDataForRead();
	bool isProgrammer = gd.ClientPlayerIsProgrammer();	// also true if architect
	//bool isArchitect = gd.ClientPlayerIsArchitect();
	g_Au.UnlockData();

	QPushButton* pbloadsc = new QPushButton(QIcon(g_Au.CommodityBitmapName(CID_ScDesignDrawing)), tr("Load Spacecraft Design..."));
	connect(pbloadsc, SIGNAL(clicked()), SLOT(LoadScDesign()));
	if(isProgrammer)
		pbloadsc->setToolTip(QString("<html>Loads a spacecraft design given its blueprint number.<br>"
					"%1 Loads the design in a design studio.<br>"
					"%1 Spawns a ship outside a design studio. "
					"You are placed at the helm but a berth is not assigned.</html>")
				.arg(g_Au.MiddleDotSymbol()));
	else
		pbloadsc->setToolTip(QString("<html>Loads a spacecraft design given its blueprint number if you are in a "
					"design studio.</html>"));

	QPushButton* pbloadbd = new QPushButton(QIcon(g_Au.CommodityBitmapName(CID_BdDesignDrawing)), tr("Load Building Design..."));
	connect(pbloadbd, SIGNAL(clicked()), SLOT(LoadBdDesign()));
	pbloadbd->setToolTip(QString("<html>Loads a building design given its blueprint number if you are in a "
				"design studio.</html>"));

	QPushButton* pbdesigner = new QPushButton(QIcon(":data/bDesignStudio.png"), tr("Enter Design Studio"));
	connect(pbdesigner, SIGNAL(clicked()), SLOT(EnterDesigner()));
	pbdesigner->setToolTip(QString("<html>Enters a design studio instance. "
				"You must be on a world. "
				"</html>"));

	// Construct layout.
	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(pbdesigner, row++,0);
	lay->addWidget(pbloadsc, row++,0);
	lay->addWidget(pbloadbd, row++,0);
	lay->setRowStretch(row, 1);
	lay->setColumnStretch(1, 1);
	setLayout(lay);
}

AuAgentDesign::~AuAgentDesign()
{
}

void AuAgentDesign::setVisible(bool visible)
{
	QWidget::setVisible(visible);
}

void AuAgentDesign::LoadScDesign()
{
	quint32 designId = g_Au.Settings().readNumEntry("Widget/Admin/LoadShipReference", 0);
	QString designDef;
	if(designId)
		designDef = QVariant(designId).toString();

	bool ok;
	QString sid = QInputDialog::getText(this, tr("Load Spacecraft Design"),
			tr("<html>"
				"Enter the blueprint number of the ship design to load."
				"</html>"),
			QLineEdit::Normal, designDef, &ok);
	designId = QVariant(sid).toUInt();
	if(ok && designId)
	{
		g_Au.Settings().writeEntry("Widget/Admin/LoadShipReference", (int)designId);
		auApp->DesignOpen(designId, DaDesignBase::DESIGN_Spacecraft);	// Try to load an Sc design.
	}
}

void AuAgentDesign::LoadBdDesign()
{
	quint32 designId = g_Au.Settings().readNumEntry("Widget/Admin/LoadBuildingReference", 0);
	QString designDef;
	if(designId)
		designDef = QVariant(designId).toString();

	bool ok;
	QString sid = QInputDialog::getText(this, tr("Load Building Design"),
			tr("<html>"
				"Enter the blueprint number of the building design to load."
				"</html>"),
			QLineEdit::Normal, designDef, &ok);
	designId = QVariant(sid).toUInt();
	if(ok && designId)
	{
		g_Au.Settings().writeEntry("Widget/Admin/LoadBuildingReference", (int)designId);
		auApp->DesignOpen(designId, DaDesignBase::DESIGN_Building);	// Try to load a Bd design.
	}
}

void AuAgentDesign::EnterDesigner()
{
	auApp->EnterStudio(0, QString());
}

