//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubldfarm.cpp,v 1.8 2022/02/19 18:09:37 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubldfarm.h"
//
#include "auapp.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include "dabd.h"
#include "daperson.h"
#include "daworldll.h"
#include "dgdata.h"
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuBldFarm::AuBldFarm(QWidget* parent) : QWidget(parent)
{
	m_Timer = 0;
	QLabel* lbico = new QLabel(this);
	lbico->setPixmap(QPixmap(":data/bFarm.png"));

	m_Info = new QLabel(this);
	m_Info->setWordWrap(true);

	m_LivestockButton = new QPushButton(QIcon(":data/c_Animal.png"), tr("Get Livestock from Local Wildlife"));
	m_LivestockButton->setToolTip(tr("<html><b>Livestock DNA</b><br>"
				"Selects the DNA of livestock to be bred at this farm. "
				"Sentient DNA cannot be used to breed livestock. "
				"<br>"
				"%1 You must be in the chain of command of the city to change this setting. "
				"</html>").arg(g_Au.MiddleDotSymbol()));
	connect(m_LivestockButton, SIGNAL(clicked()), SLOT(LivestockButtonClicked()));

	QHBoxLayout* ilay = new QHBoxLayout();
	ilay->addWidget(lbico);
	ilay->addWidget(m_Info);

	// Build the layout.
	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(ilay);
	lay->addWidget(m_LivestockButton);
	lay->addStretch();
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

void AuBldFarm::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	// Create a timer to keep this page updated.
	if(!m_Timer)
	{
		m_Timer = new QTimer(this);
		connect(m_Timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_Timer->isActive())
		m_Timer->start(1000);

	// Show all my widgets.
	QVBoxLayout* lay = (QVBoxLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->show();

	m_Info->setText(tr("<html>"
				"<p>"
				"Farms that produce commodities from livestock need a live animal. "
				"If a domestic animal lives on the farm, it will suffice. "
				"If no animal is alive on the farm, an animal will be bred. "
				"If no animal can be bred, livestock production processes cannot be completed. "
				"</p>"

				"<p>"
				"Farms can domesticate wild animals, in environments inhabited by animals. "
				"The domesticated animal will be one of six herbivore species native to the farm's terrain. "
				"Two species are active at each of three times of day: diurnal, crepuscular and nocturnal. "
				"The farm will produce one of the two active species, depending upon the time of day the animal is captured."
				"</p>"

				"<p>"
				"Farms in hostile environments must get livestock from elsewhere. "
				"A live domestic animal can be placed on a farm or a DNA sample of an animal can be used. "
				"DNA of sentient species cannot be used as livestock. "
				"DNA samples enable domestic animals of all ecological roles to be bred from wild animals: "
				"carnivores, omnivores, scavengers and herbivores. "
				"</p>"

				"<p>"
				"Farms that produce <i>Animal Carcasses</i> will breed a second animal. "
				"The second animal will be the same specie as the first, with complementing gender. "
				"</p>"
				"</html>"));

	if(m_LivestockSpecies.IsNull())
	{
		m_LivestockButton->setIcon(QIcon(":data/c_Animal.png"));
		m_LivestockButton->setText(tr("Get Livestock from Local Wildlife"));
	}
	else
	{
		QString specie = m_LivestockSpecies.NameOrDesc();
		m_LivestockButton->setIcon(QIcon(":data/c_DNASample.png"));
		m_LivestockButton->setText(specie);
	}
}

void AuBldFarm::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Kill the timer. It will be recreated during the next showEvent.
	if(m_Timer && m_Timer->isActive())
		m_Timer->stop();

	// Hide my widgets now or they force the parent AuLabor dialog to be too big even though this page isn't visible.
	QVBoxLayout* lay = (QVBoxLayout*)layout();
	for(int iw=0; iw<lay->count(); iw++)
		if(lay->itemAt(iw)->widget())
			lay->itemAt(iw)->widget()->hide();
	m_Info->setText("");
	m_LivestockButton->setText("");
}

void AuBldFarm::LivestockButtonClicked()
{
	QMenu menu(this);
	QAction* wildlife = menu.addAction(QIcon(":data/c_Animal.png"), tr("Get Livestock from Local Wildlife"));
	QAction* cur = 0;
	QList<AuSpecies> dnaListed;
	if(!m_LivestockSpecies.IsNull())
	{
		dnaListed.append(m_LivestockSpecies);
		QString specie = m_LivestockSpecies.NameOrDesc();
		cur = menu.addAction(QIcon(":data/c_DNASample.png"), specie);
	}

	// List DNA samples in the possession of the person.
	QHash<QAction*, AuSpecies> species;
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person && person->InIndustry() != IID_None)
	{
		bool hasSep = false;
		const AuGear* inv = person->LockGearForRead();
		AuGearPosList allGearPos;
		inv->ListAll(allGearPos);
		person->UnlockGear();
		for(AuGearPosList::const_iterator gi = allGearPos.cbegin(); gi != allGearPos.cend(); gi++)
		{
			const AuItem* gitem = inv->ItemAt(*gi);
			if(gitem->CommodityId() != CID_DNASample)
				continue;
			if(!gitem->DNAIsViable())
				continue;
			if(gitem->DNAIsSentient())
				continue;
			AuSpecies ispecies(gitem->DNASpecies());
			if(ispecies.IsNull())
				continue;
			if(dnaListed.contains(ispecies))
				continue;
			if(!hasSep)
			{
				menu.addSeparator();
				hasSep = true;
			}

			dnaListed.append(ispecies);

			QString specie = ispecies.NameOrDesc();
			QAction* da = menu.addAction(QIcon(":data/c_DNASample.png"), specie);
			species.insert(da, ispecies);
		}
	}
	pView->UnlockSceneAtom();

	QAction* action = menu.exec(QCursor::pos());
	if(!action || action == cur)
		return;
	if(action == wildlife)
		SetLivestockSpecies(AuSpecies());
	else
	{
		QHash<QAction*, AuSpecies>::const_iterator di = species.find(action);
		if(di != species.cend())
			SetLivestockSpecies(*di);
	}
}

void AuBldFarm::SetLivestockSpecies(const AuSpecies& species)
{
	auApp->FarmSetLivestock(species, m_CapitolOpId);
}

void AuBldFarm::UpdateContents()
{
	if(!isVisible())
		return;
	// Find out what my current livestock is.
	AuSpecies livestock;
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person && person->InIndustry() != IID_None)
	{
		DaBd* bdOp = person->BdOp(m_CapitolOpId);
		if(bdOp)
			livestock = bdOp->LivestockSpecies();
	}
	pView->UnlockSceneAtom();

	if(livestock.IsNull())
	{
		m_LivestockButton->setIcon(QIcon(":data/c_Animal.png"));
		m_LivestockButton->setText(tr("Get Livestock from Local Wildlife"));
		m_LivestockSpecies = livestock;
	}
	else if(m_LivestockSpecies != livestock)
	{
		m_LivestockSpecies = livestock;
		QString specie = livestock.NameOrDesc();
		m_LivestockButton->setIcon(QIcon(":data/c_DNASample.png"));
		m_LivestockButton->setText(specie);
	}
}

