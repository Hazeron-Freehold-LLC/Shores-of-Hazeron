//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausetoverlay.cpp,v 1.4 2022/02/19 17:48:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausetoverlay.h"
//
#include "aucheckbox.h"
#include "auclientglobal.h"
#include "auclientview.h"
#include "dgdata.h"
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QScrollArea>

AuSetOverlay::AuSetOverlay(QWidget *parent) : QWidget(parent)
{
	// On screen view buttons.
	QGroupBox* gbViewButtons = new QGroupBox(tr("On Screen Buttons"));
	{
		QWidget* viewButtons = new QWidget();
		{
			// Make a map to alphabetize them for me.
			const DgData& gd = g_Au.LockDataForRead();
			bool isAgent = gd.ClientPlayerIsAgent() && !gd.ClientPlayerIsProgrammer();
			bool isArchitect = gd.ClientPlayerIsArchitect();
			bool isProgrammer = gd.ClientPlayerIsProgrammer() && !gd.ClientPlayerIsArchitect();
			bool isMinion = gd.ClientPlayerIsMinion();
			g_Au.UnlockData();

			QMap<QString, AuViewButton::VIEWBUTTON> allButtons;
			for(int vbi = AuViewButton::VB_Invalid; vbi < AuViewButton::VB_COUNT; vbi++)
			{
				AuViewButton::VIEWBUTTON vb = (AuViewButton::VIEWBUTTON)vbi;
				if(vb == AuViewButton::VB_Agent && !isAgent)
					continue;
				else if(vb == AuViewButton::VB_Architect && !isArchitect)
					continue;
				else if(vb == AuViewButton::VB_Programmer && !isProgrammer)
					continue;
				else if(vb == AuViewButton::VB_Minion && !isMinion)
					continue;

				if(AuViewButton::SettingsName(vb) != "")
					allButtons.insert(AuViewButton::SettingsName(vb), vb);
			}

			QList<int> hiddenButtons = g_AuClient.ViewButtonsHidden();

			QGridLayout* clay = new QGridLayout(viewButtons);
			int row = 0;
			int col = 0;
			for(QMap<QString, AuViewButton::VIEWBUTTON>::const_iterator ci = allButtons.constBegin(); ci != allButtons.constEnd();
					ci++)
			{
				QLabel* lb = new QLabel(this);
				lb->setPixmap(QPixmap(AuViewButton::Image(*ci)).scaled(64,64, Qt::KeepAspectRatio, Qt::SmoothTransformation));

				AuCheckBoxValue* ck = new AuCheckBoxValue(ci.key(), *ci, this);
				if(hiddenButtons.contains(*ci))
					ck->setCheckState(Qt::Unchecked);
				else
					ck->setCheckState(Qt::Checked);
				connect(ck, SIGNAL(StateChanged(int, int)), SLOT(ViewButtonCheckChanged(int, int)));
				ck->setToolTip(tr("<html>When checked, this button is allowed to appear in the main view window. "
							"Buttons appear in a column along the left edge of the view window when you are in <i>hand</i> mode, "
							"not controlling your avatar."
						"</html>"));

				col=0;
				clay->addWidget(lb, row,col++);
				clay->addWidget(ck, row++,col++);
			}
			clay->setColumnStretch(col, 1);

			clay->setMargin(2);
			clay->setSpacing(2);
			viewButtons->setLayout(clay);
		}
		QScrollArea* vbScroll = new QScrollArea;
		vbScroll->setWidget(viewButtons);

		QVBoxLayout* gblay = new QVBoxLayout();
		gblay->addWidget(vbScroll);
		gblay->setSpacing(2);
		gblay->setMargin(12);
		gbViewButtons->setLayout(gblay);
	}

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(gbViewButtons, row,0, 1,3);
	row++;
	//lay->setRowStretch(row, 1);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuSetOverlay::~AuSetOverlay()
{
}

void AuSetOverlay::ViewButtonCheckChanged(int checkState, int vb)
{
	if(vb <= AuViewButton::VB_Invalid || vb >= AuViewButton::VB_COUNT)
		return;

	g_AuClient.SetViewButtonHidden(vb, checkState != Qt::Checked);
}
