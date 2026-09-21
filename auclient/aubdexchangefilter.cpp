//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aubdexchangefilter.cpp,v 1.17 2022/02/19 18:05:16 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aubdexchangefilter.h"
//
#include "aubdreport.h"
#include "auapp.h"
#include "aucheckbox.h"
#include "auclientglobal.h"
#include "auglobal.h"
#include "auframe.h"
#include "auview.h"
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QSpinBox>

AuBdExchangeFilter::AuBdExchangeFilter(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Building Blueprint Filter"));
	setAuDlgIcon(QIcon(":data/c_Filter.png"));

	// Load current filter settings.
	QSet<AuId> pubExclude;			// Ids of excluded publishers.
	bool terLand, terSeaBottom, intendedIndustry, rated;
	int minRating;
	bool advanced = LoadFilterSettings(pubExclude, terLand, terSeaBottom, intendedIndustry, rated, minRating);

	QLabel* lbInfo = new QLabel(tr("<html>"
				"<p>"
				"The <i>Construction</i> window offers blueprints to build each industry. "
				"Blueprints are listed from storage devices in your gear, such as paper or thumb drives. "
				"Optionally, blueprints published for free in the <i>Building Blueprint Exchange</i> may be included. "
				"</p>"
				"<p>"
				"An industry can be built using any blueprint that meets its building code requirements. "
				"The intended purpose for a blueprint is not a limitation; it is an aesthetic choice of the architect. "
				"This enables the same blueprint to be used by different industries. "
				"</p>"
				"<p>"
				"Blueprints that cannot be built in the current environment are not listed, "
				"unless a biodome or enclosed platform exists on the world. "
				"</p>"
				"<p>"
				"The list can be large. "
				"Filter settings help reduce the number of choices. "
				"</p>"
				"</html>"));
	lbInfo->setWordWrap(true);

	m_Beginner = new QRadioButton(tr("Basic"), this);
	m_Beginner->setToolTip(tr("<html>"
				"<p>"
				"Choices will be limited to a few basic blueprints. "
				"Rating is ignored. "
				"Land and sea construction terrain are included. "
				"Blueprints may not be intended for the industry. "
				"</p>"
				"</html>"));
	m_Beginner->setChecked(!advanced);
	connect(m_Beginner, SIGNAL(clicked()), SLOT(BeginnerClicked()));

	m_Advanced = new QRadioButton(tr("Advanced"), this);
	m_Advanced->setToolTip(tr("<html>"
				"<p>"
				"Blueprint choices will be filtered by these settings. "
				"</p>"
				"</html>"));
	m_Advanced->setChecked(advanced);
	connect(m_Advanced, SIGNAL(clicked()), SLOT(AdvancedClicked()));

	lbRating = new QLabel(tr("Rating"), this);

	m_cbRated = new QCheckBox(tr("Exchange blueprint must be rated."), this);
	m_cbRated->setCheckState(rated ? Qt::Checked : Qt::Unchecked);
	m_cbRated->setToolTip(tr("<html>"
				"Blueprints in the exchange can be filtered by rating. "
				"Rating filter does not apply to blueprints in your gear. "
				"</html>"));
	connect(m_cbRated, SIGNAL(stateChanged(int)), SLOT(RatedChanged(int)));

	m_MinRating = new QSpinBox(this);
	m_MinRating->setRange(1,5);
	m_MinRating->setValue(minRating);
	m_MinRating->setEnabled(rated);
	m_MinRating->setToolTip(tr("<html>"
				"Minimum rating desired. "
				"</html>"));

	lbTer = new QLabel(tr("Construction Terrain"), this);

	m_cbLand = new QCheckBox(tr("Land"), this);
	m_cbLand->setCheckState(terLand ? Qt::Checked : Qt::Unchecked);

	m_cbSeaBottom = new QCheckBox(tr("Sea Bottom"), this);
	m_cbSeaBottom->setCheckState(terSeaBottom ? Qt::Checked : Qt::Unchecked);

	lbIntend = new QLabel(tr("Intended Industry"), this);

	m_cbIntended = new QCheckBox(tr("Blueprint must be intended for the industry."), this);
	m_cbIntended->setToolTip(tr("<html>"
				"When checked, only blueprints intended for the industry will be listed."
				"When unchecked, all blueprints that meet the building code requirements for the industry will be listed."
				"</html>"));
	m_cbIntended->setCheckState(intendedIndustry ? Qt::Checked : Qt::Unchecked);

	lbPub = new QLabel(tr("Blueprint Publishers"), this);

	QWidget* pubcks = CreatePublisherChecks(pubExclude);

	m_PubScroller = new QScrollArea(this);
	m_PubScroller->setWidget(pubcks);
	m_PubScroller->setWidgetResizable(true);

	m_SelectAll = new QPushButton(tr("Select All"), this);
	m_SelectAll->setToolTip(tr("<html>"
				"Selects all publishers. "
				"</html>"));
	connect(m_SelectAll, SIGNAL(clicked()), SLOT(SelectAllPressed()));

	m_DeselectAll = new QPushButton(tr("Deselect All"), this);
	m_DeselectAll->setToolTip(tr("<html>"
				"Deselects all publishers. "
				"</html>"));
	connect(m_DeselectAll, SIGNAL(clicked()), SLOT(DeselectAllPressed()));

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	connect(pbok, SIGNAL(clicked()), SLOT(OkPressed()));

	QHBoxLayout* ratelay = new QHBoxLayout;
	ratelay->addWidget(m_cbRated);
	ratelay->addWidget(m_MinRating);

	QVBoxLayout* ratlay = new QVBoxLayout;
	ratlay->addWidget(lbRating);
	ratlay->addLayout(ratelay);

	QVBoxLayout* cklay = new QVBoxLayout;
	cklay->addWidget(lbTer);
	cklay->addWidget(m_cbLand);
	cklay->addWidget(m_cbSeaBottom);

	QVBoxLayout* inlay = new QVBoxLayout;
	inlay->addWidget(lbIntend);
	inlay->addWidget(m_cbIntended);

	QVBoxLayout* play = new QVBoxLayout;
	play->addWidget(lbPub);
	play->addWidget(m_PubScroller);

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addWidget(m_SelectAll);
	blay->addWidget(m_DeselectAll);
	blay->addStretch();
	blay->addWidget(pbok);

	QVBoxLayout* alay = new QVBoxLayout;
	alay->addLayout(ratlay);
	alay->addLayout(cklay);
	alay->addLayout(inlay);
	alay->addLayout(play);
	alay->addLayout(blay);

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbInfo, row,0, 1,2);
	row++;
	lay->addWidget(m_Beginner, row,0, 1,2);
	row++;
	lay->addWidget(m_Advanced, row,0, 1,2);
	row++;
	lay->addLayout(alay, row,1);
	lay->setColumnMinimumWidth(0, 20);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuBdExchangeFilter::~AuBdExchangeFilter()
{
}

void AuBdExchangeFilter::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
		EnableControls();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuBdExchangeFilter::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/BuildingExchangeFilter/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/BuildingExchangeFilter/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/BuildingExchangeFilter/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/BuildingExchangeFilter/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuBdExchangeFilter::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/BuildingExchangeFilter/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/BuildingExchangeFilter/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/BuildingExchangeFilter/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/BuildingExchangeFilter/SizeY",sz.height());
}

QWidget* AuBdExchangeFilter::CreatePublisherChecks(const QSet<AuId>& pubExclude)
{
	QHash<AuId, QString> publishers;
	QMultiMap<QString, AuId> alphabetized;
	QHash<quint32, AuBdDesignExchangeEntryClient>& designs = g_AuClient.LockBdDesignExchange();
	for(QHash<quint32, AuBdDesignExchangeEntryClient>::const_iterator di = designs.cbegin(); di != designs.cend(); di++)
		if(!publishers.contains((*di).DesignArchitectId()))
		{
			publishers.insert((*di).DesignArchitectId(), (*di).DesignArchitectName());
			alphabetized.insert((*di).DesignArchitectName().toUpper(), (*di).DesignArchitectId());
		}
	g_AuClient.UnlockBdDesignExchange();

	QGridLayout* publay = new QGridLayout;
	int row=0;
	for(QMultiMap<QString, AuId>::const_iterator pi = alphabetized.cbegin(); pi != alphabetized.cend(); pi++)
	{
		bool include = !pubExclude.contains(*pi);	// Include all who are not specifically excluded.

		AuCheckBoxValue64* ckIncluded = new AuCheckBoxValue64(publishers[*pi], (*pi).GetID(), this);
		m_PubChecks.append(ckIncluded);
		ckIncluded->setChecked(include);
		if(include)
			ckIncluded->setToolTip(tr("<html>Blueprints by checked publishers are listed.</html>"));
		else
			ckIncluded->setToolTip(tr("<html>Blueprints by unchecked publishers are not listed.</html>"));

		QLabel* lbid = new QLabel(QString("<html><small><i>%1</i></small></html>").arg((*pi).StringId()), this);

		publay->addWidget(ckIncluded, row,0);
		publay->addWidget(lbid, row,1);
		row++;
	}

	QWidget* pubcks = new QWidget;
	pubcks->setLayout(publay);
	return pubcks;
}

void AuBdExchangeFilter::OkPressed()
{
	bool advanced = m_Advanced->isChecked();
	bool terLand = m_cbLand->checkState() == Qt::Checked ? true : false;
	bool terSeaBottom = m_cbSeaBottom->checkState() == Qt::Checked ? true : false;
	bool intendedIndustry = m_cbIntended->checkState() == Qt::Checked ? true : false;
	bool mustBeRated = m_cbRated->checkState() == Qt::Checked ? true : false;
	int minRating = m_MinRating->value();
	g_Au.Settings().writeEntry("Widget/BuildingExchangeFilter/Advanced", advanced);
	g_Au.Settings().writeEntry("Widget/BuildingExchangeFilter/TerrainLand", terLand);
	g_Au.Settings().writeEntry("Widget/BuildingExchangeFilter/TerrainSeaBottom", terSeaBottom);
	g_Au.Settings().writeEntry("Widget/BuildingExchangeFilter/IntendedIndustry", intendedIndustry);
	g_Au.Settings().writeEntry("Widget/BuildingExchangeFilter/MustBeRated", mustBeRated);
	g_Au.Settings().writeEntry("Widget/BuildingExchangeFilter/MinRating", minRating);

	QSet<AuId> pubExclude;
	for(QList<AuCheckBoxValue64*>::const_iterator pi = m_PubChecks.cbegin(); pi != m_PubChecks.cend(); pi++)
		if(!(*pi)->isChecked())
			pubExclude.insert(AuId((*pi)->Value()));

	QByteArray bapub;
	QDataStream out(&bapub, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << pubExclude;
	g_Au.Settings().writeEntry("Widget/BuildingExchangeFilter/Publishers", bapub);
	accept();
}

//static
bool AuBdExchangeFilter::LoadFilterSettings(QSet<AuId>& excludePub, bool& terLand, bool& terSeaBottom, bool& intendedIndustry,
		bool& mustBeRated, int& minRating)
{
	bool advanced = g_Au.Settings().readBoolEntry("Widget/BuildingExchangeFilter/Advanced",false);
	terLand = g_Au.Settings().readBoolEntry("Widget/BuildingExchangeFilter/TerrainLand",true);
	terSeaBottom = g_Au.Settings().readBoolEntry("Widget/BuildingExchangeFilter/TerrainSeaBottom",true);
	intendedIndustry = g_Au.Settings().readBoolEntry("Widget/BuildingExchangeFilter/IntendedIndustry",false);
	mustBeRated = g_Au.Settings().readBoolEntry("Widget/BuildingExchangeFilter/MustBeRated",false);
	minRating = g_Au.Settings().readNumEntry("Widget/BuildingExchangeFilter/MinRating",2);
	if(minRating < 1 || minRating > 5)
		minRating = 2;

	QByteArray bapub;
	g_Au.Settings().readEntry("Widget/BuildingExchangeFilter/Publishers", bapub);
	QDataStream in(bapub);
	in.setVersion(QDataStream::Qt_4_1);
	in >> excludePub;
	return advanced;
}

//static
bool AuBdExchangeFilter::LoadFilter(QSet<AuId>& excludePub, bool& terLand, bool& terSeaBottom, bool& intendedIndustry,
		bool& mustBeRated, int& minRating)
{
	// Load filter settings.
	bool advanced = LoadFilterSettings(excludePub, terLand, terSeaBottom, intendedIndustry, mustBeRated, minRating);
	if(!advanced)
	{
		// Override the settings with the basic filter.
		terLand = true;
		terSeaBottom = true;
		intendedIndustry = false;
		mustBeRated = false;
		minRating = 2;

		// Exclude designs by all architects except Haxus.
		excludePub.clear();
		QHash<quint32, AuBdDesignExchangeEntryClient>& designs = g_AuClient.LockBdDesignExchange();
		for(QHash<quint32, AuBdDesignExchangeEntryClient>::const_iterator di = designs.cbegin(); di != designs.cend(); di++)
			if((*di).DesignArchitectId() != AuId::Haxus())
				excludePub.insert((*di).DesignArchitectId());
		g_AuClient.UnlockBdDesignExchange();
	}
	return advanced;
}

bool AuBdExchangeFilter::TestFilter(const QSet<AuId>& excludePub, bool terLand, bool terSeaBottom, bool intendedIndustry,
		INDUSTRYID industryId, ORBITZONE ozone, bool gasGiant, bool breathableAtmo, bool enclosurePresent, bool mustBeRated,
		int minRating, const AuBdReport& rpt, float rating)
{
	if(mustBeRated)
	{
		if(rating < 1)
			return false;	// A rating less than 1 is not valid.
		if(rating < minRating)
			return false;	// Rating of the design is lower than the filter minimum.
	}

	// Check intended industry.
	if(intendedIndustry && industryId != rpt.IndustryId())
		return false;

	// Check excluded publishers.
	if(excludePub.contains(rpt.ArchitectId()))
		return false;

	// Check terrain.
	if(enclosurePresent &&
			industryId != IID_Biodome &&
			industryId != IID_AerialPlatform &&
			industryId != IID_GroundPlatform &&
			industryId != IID_SeaPlatform &&
			industryId != IID_AirportTerminal &&
			industryId != IID_BaseAirTerminal)
	{
		// Can the design be placed in an enclosure that is present.
		return true;
	}

	if(terLand && !gasGiant)
	{
		// Can the design be placed on the land?
		if(ozone == OZONE_Frigid)
		{
			if((rpt.SkeletonOkForFrigidLand() || rpt.ArmorOkForFrigidLand()) &&
					rpt.LifeSupportOkForFrigidLand())
				return true;
		}
		else if(ozone == OZONE_Inferno)
		{
			if((rpt.SkeletonOkForInfernoLand() || rpt.ArmorOkForInfernoLand()) &&
					rpt.LifeSupportOkForInfernoLand())
				return true;
		}
		else if(!breathableAtmo)
		{
			if(rpt.LifeSupportOkForVacuumLand())
				return true;
		}
		else
			return true;
	}

	if(terSeaBottom)
	{
		// Can the design be placed on the sea bottom?
		if(gasGiant)
		{
			if((rpt.SkeletonOkForGasGiantCore() || rpt.ArmorOkForGasGiantCore()) &&
					rpt.LifeSupportOkForGasGiantCore())
				return true;
		}
		else
		{
			if(ozone == OZONE_Frigid)
			{
				if((rpt.SkeletonOkForFrigidSeaBottom() || rpt.ArmorOkForFrigidSeaBottom()) &&
						rpt.LifeSupportOkForFrigidSeaBottom())
					return true;
			}
			else if(ozone != OZONE_Inferno)
			{
				if((rpt.SkeletonOkForSeaBottom() || rpt.ArmorOkForSeaBottom()) &&
						rpt.LifeSupportOkForSeaBottom())
					return true;
			}
		}
	}
	return false;
}

void AuBdExchangeFilter::RatedChanged(int state)
{
	Q_UNUSED(state);
	EnableControls();
}

void AuBdExchangeFilter::AdvancedClicked()
{
	EnableControls();
}

void AuBdExchangeFilter::BeginnerClicked()
{
	EnableControls();
}

void AuBdExchangeFilter::EnableControls()
{
	if(m_Beginner->isChecked())
	{
		m_cbIntended->setEnabled(false);
		m_cbLand->setEnabled(false);
		m_cbRated->setEnabled(false);
		m_cbSeaBottom->setEnabled(false);
		m_MinRating->setEnabled(false);
		for(QList<AuCheckBoxValue64*>::iterator pi = m_PubChecks.begin(); pi != m_PubChecks.end(); pi++)
			(*pi)->setEnabled(false);
		m_SelectAll->setEnabled(false);
		m_DeselectAll->setEnabled(false);
	}
	else
	{
		m_cbIntended->setEnabled(true);
		m_cbLand->setEnabled(true);
		m_cbRated->setEnabled(true);
		m_cbSeaBottom->setEnabled(true);
		m_MinRating->setEnabled(m_cbRated->isChecked());
		for(QList<AuCheckBoxValue64*>::iterator pi = m_PubChecks.begin(); pi != m_PubChecks.end(); pi++)
			(*pi)->setEnabled(true);
		m_SelectAll->setEnabled(true);
		m_DeselectAll->setEnabled(true);
	}
}

void AuBdExchangeFilter::SelectAllPressed()
{
	for(QList<AuCheckBoxValue64*>::iterator pi = m_PubChecks.begin(); pi != m_PubChecks.end(); pi++)
	{
		(*pi)->setChecked(true);
		(*pi)->setToolTip(tr("<html>Blueprints by checked publishers are listed.</html>"));
	}
}

void AuBdExchangeFilter::DeselectAllPressed()
{
	for(QList<AuCheckBoxValue64*>::iterator pi = m_PubChecks.begin(); pi != m_PubChecks.end(); pi++)
	{
		(*pi)->setChecked(false);
		(*pi)->setToolTip(tr("<html>Blueprints by unchecked publishers are not listed.</html>"));
	}
}

