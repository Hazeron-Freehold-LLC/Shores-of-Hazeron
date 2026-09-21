//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aurepairdamage.cpp,v 1.5 2022/02/19 18:16:39 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aurepairdamage.h"
//
#include "auapp.h"
#include "aucommchannel.h"
#include "auframe.h"
#include "auservicedialog.h"
#include "auview.h"
#include <daperson.h>
#include <davessel.h>
#include <QGridLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QVariant>

AuRepairDamageSystem::AuRepairDamageSystem(QWidget* parent) : QWidget(parent)
{
	m_Icon = new QLabel(this);
	m_Icon->setAlignment(Qt::AlignLeft|Qt::AlignTop);
	m_Icon->setToolTip(tr("<html>"
				"System Type"
				"</html>"));

	m_Name = new QLabel(this);

	m_Bar = new QProgressBar(this);
	m_Bar->setRange(0,100);
	m_Bar->setFormat(QString());
	m_Bar->setMaximumHeight(6);
	m_Bar->setToolTip(tr("<html>"
				"System Health Percentage"
				"</html>"));

	m_PercentIco = new QLabel(this);
	m_PercentIco->setToolTip(tr("<html>"
				"System Status"
				"<br>Red = 10% or Less"
				"<br>Amber = 11% to 50%"
				"<br>Green = 51% to 100%"
				"</html>"));

	m_Percent = new QLabel(this);

	m_Parts = new QLabel(this);
	m_Parts->setAlignment(Qt::AlignLeft|Qt::AlignTop);
	m_Parts->setWordWrap(true);
	m_Parts->setMinimumWidth(32);

	m_Estimate = new QPushButton(tr("Estimate"), this);
	m_Estimate->setToolTip(tr("<html>Requests an estimate to repair a system. "
				"Establish communication with a city first, to get a reply. "
				"You can choose to accept or reject the estimate. "
				"</html>"));
	connect(m_Estimate, SIGNAL(clicked()), SLOT(Estimate()));

	QHBoxLayout* blay = new QHBoxLayout();
	blay->addWidget(m_Name, 1);
	blay->addWidget(m_Percent);
	blay->addWidget(m_PercentIco);
	blay->setSpacing(2);
	blay->setMargin(0);

	QVBoxLayout* nlay = new QVBoxLayout();
	nlay->addLayout(blay);
	nlay->addWidget(m_Bar);
	nlay->addStretch();
	nlay->setSpacing(2);
	nlay->setMargin(0);

	QVBoxLayout* elay = new QVBoxLayout();
	elay->addWidget(m_Estimate);
	elay->addStretch();
	elay->setSpacing(2);
	elay->setMargin(0);

	QHBoxLayout* lay = new QHBoxLayout();
	lay->addWidget(m_Icon);
	lay->addLayout(nlay, 1);
	lay->addWidget(m_Parts);
	lay->addLayout(elay);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);
}

AuRepairDamageSystem::~AuRepairDamageSystem()
{
}

void AuRepairDamageSystem::Update(const AuSysId& sysId, const QString& sysName, const AuDamage& damage, quint32 maxHits)
{
	m_SysId = sysId;
	m_SysName = sysName;

	m_Icon->setPixmap(QPixmap(sysId.Icon()).scaled(22,22, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	m_Name->setText(QString("<html><b>%1</b></html>").arg(sysName));

	quint32 dmg = damage.HitsDamageAddParts(sysId);	// This is in hit points.
	if(dmg > maxHits)
		dmg = maxHits;
	int percDmg;
	if(dmg >= maxHits || maxHits == 0)
		percDmg = 100;
	else
	{
		percDmg = Percent(dmg, maxHits);
		if(!percDmg && dmg)
			percDmg = 1;
		else if(percDmg == 100)
			percDmg = 99;
	}

	if(percDmg >= 90)
		m_PercentIco->setPixmap(QPixmap(":data/AlertRedBright.png").scaled(16,16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	else if(percDmg >= 50)
		m_PercentIco->setPixmap(QPixmap(":data/AlertYellowBright.png").scaled(16,16, Qt::KeepAspectRatio,Qt::SmoothTransformation));
	else
		m_PercentIco->setPixmap(QPixmap(":data/AlertGreen.png").scaled(16,16, Qt::KeepAspectRatio, Qt::SmoothTransformation));

	int health = 100 - percDmg;
	m_Bar->setValue(health);
	if(!dmg)
		m_Percent->setText(QString("%L1hp 100%")
				.arg(maxHits));
	else
		m_Percent->setText(QString("%L1/%L2hp %3%")
				.arg(maxHits - dmg)
				.arg(maxHits)
				.arg(health));

	AuRepairParts parts;
	damage.PartsDamage(sysId, parts);
	QString partsList;
	QString toolTip;
	if(!parts.empty())
	{
		// Make an html formatted list of the parts with icons.
		// Names make them too big; we'll put the names in a tool tip.
		QString part;
		toolTip = tr("Parts Needed");
		for(AuRepairParts::const_iterator pi = parts.cbegin(); pi != parts.cend(); pi++)
		{
			if(pi == parts.cbegin())
				part = QString("<img height=\"16\" src=\"qrc%1\"> %L2")
					.arg(g_Au.CommodityBitmapName(pi.key()))
					.arg(*pi);
			else
				part = QString("<br><img height=\"16\" src=\"qrc%1\"> %L2")
					.arg(g_Au.CommodityBitmapName(pi.key()))
					.arg(*pi);
			partsList += part;
			toolTip += QString("<br>%L1 %2")
				.arg(*pi)
				.arg(g_Au.CommodityName(pi.key()));
		}
	}
	else
		toolTip = tr("No Parts Needed");
	m_Parts->setText(partsList);
	m_Parts->setToolTip(toolTip);
}

void AuRepairDamageSystem::Estimate()
{
	emit Estimate(m_SysId, m_SysName);
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuRepairDamage::AuRepairDamage(AuServiceDialog* parent) : QWidget(parent)
{
	QLabel* lbinfo  = new QLabel(tr("<html>"
				"<p>"
				"The spacecraft must be rigged for repairs: stopped, sensors off, shields off, power plant off, "
				"maneuver drive off and FTL drive off. "
				"</p>"
				"</html>"), this);
	lbinfo->setWordWrap(true);

	QWidget* epanels = new QWidget;
	m_SystemLay = new QVBoxLayout(epanels);
	epanels->setLayout(m_SystemLay);

	m_SystemScroller = new QScrollArea;
	m_SystemScroller->setWidget(epanels);
	m_SystemScroller->setWidgetResizable(true);

	m_RepairButton = new QPushButton(tr("Estimate All Repairs"));
	m_RepairButton->setAutoDefault(true);
	m_RepairButton->setDefault(true);
	m_RepairButton->setToolTip(tr("<html>Requests an estimate to repair all systems. "
				"Establish communication with a city first, to get a reply. "
				"You can choose to accept or reject the estimate. "
				"</html>"));
	connect(m_RepairButton, SIGNAL(clicked()), SLOT(EstimateAll()));

	QLabel* lbMinQ = new QLabel(tr("Minimum Quality of Parts"), this);

	m_MinQ = new QSpinBox(this);
	m_MinQ->setRange(1,255);
	m_MinQ->setToolTip(tr("<html>"
				"Minimum quality of parts to be installed. "
				"Initially, this value is the quality of manufacture of the spacecraft. "
				"<br>"
				"<b>The quality of a system is reduced to the lowest quality part installed into the system.</b>"
				"</html>"));

	QHBoxLayout* blay = new QHBoxLayout();
	blay->addStretch();
	blay->addWidget(lbMinQ);
	blay->addWidget(m_MinQ);
	blay->addWidget(m_RepairButton);

	QVBoxLayout* lay = new QVBoxLayout();
	lay->addWidget(lbinfo);
	lay->addWidget(m_SystemScroller);
	lay->addLayout(blay);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);

	UpdateContents();
}

// Update contents of dialog that might change while the dialog is up.
void AuRepairDamage::UpdateContents()
{
	// Lock the root and find the scene atom.
	AuDamage damage;
	QMultiMap<QString, AuSysId> names;			// System names sorted alphabetically. Names should not duplicate but multi is safe.
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		DaVessel* ship = person->FindParentVesselInEnv();
		if(ship)
		{
			m_MinQ->setValue(ship->QualityMade());
			ship->DamageReportHp(damage);

			// Get the names of those systems.
			QSet<AuSysId> sysIds;
			damage.SystemIds(sysIds);
			for(QSet<AuSysId>::const_iterator di = sysIds.cbegin(); di != sysIds.cend(); di++)
			{
				QString name = ship->SystemName(*di);
				names.insert(name, *di);
			}

			// Insure we have enough systems widgets.
			while(m_SystemWidgets.size() < names.size())
			{
				AuRepairDamageSystem* sys = new AuRepairDamageSystem();
				connect(sys, SIGNAL(Estimate(AuSysId, QString)), SLOT(EstimateSystem(AuSysId, QString)));
				m_SystemWidgets.append(sys);
				m_SystemLay->addWidget(sys);
			}

			// Update systems info. Show/hide system widgets as needed.
			QList<AuRepairDamageSystem*>::iterator si = m_SystemWidgets.begin();
			for(QMultiMap<QString, AuSysId>::const_iterator ni = names.cbegin(); ni != names.cend(); ni++)
			{
				if(!(*ni).IsStructural())
					continue;	// List structural first.
				if(!damage.HitsDamageAddParts(*ni))
					continue;	// System is undamaged.
				(*si)->show();
				(*si)->Update(*ni, ni.key(), damage, ship->SystemHits(*ni));
				si++;
			}
			for(QMultiMap<QString, AuSysId>::const_iterator ni = names.cbegin(); ni != names.cend(); ni++)
			{
				if((*ni).IsStructural())
					continue;	// List equipment/nonstructural after structural.
				if(!damage.HitsDamageAddParts(*ni))
					continue;	// System is undamaged.
				(*si)->show();
				(*si)->Update(*ni, ni.key(), damage, ship->SystemHits(*ni));
				si++;
			}

			// Hide excess systems widgets, if any.
			while(si != m_SystemWidgets.end())
			{
				(*si)->hide();
				si++;
			}
		}
	}
	pView->UnlockSceneAtom();
	m_RepairButton->setEnabled(damage.IsDamaged());
}

void AuRepairDamage::EstimateAll()
{
	AuServiceDialog* svcdlg = dynamic_cast<AuServiceDialog*>(parent());
	if(!svcdlg)
		return;
	AuCommChannel* channel = svcdlg->Channel();
	if(!channel)
		return;
	const AuView* view = auApp->Frame()->View();
	if(!view)
		return;

	AuCommRequestRepairAll msg(MinQ(), channel->objectName(), view->SceneAtomId(), view->SceneAtomName(),
			view->SceneSector(), view->SceneGalaxy(), channel->Range(), channel->Scope());
	channel->SendChat(msg);

	// Close the parent dialog.
	svcdlg->accept();
}

void AuRepairDamage::EstimateSystem(AuSysId sysId, QString systemName)
{
	AuServiceDialog* svcdlg = dynamic_cast<AuServiceDialog*>(parent());
	if(!svcdlg)
		return;
	AuCommChannel* channel = svcdlg->Channel();
	if(!channel)
		return;
	const AuView* view = auApp->Frame()->View();
	if(!view)
		return;

	AuCommRequestRepairSystem msg(sysId.System(), sysId.Which(), systemName, MinQ(), channel->objectName(), view->SceneAtomId(),
			view->SceneAtomName(), view->SceneSector(), view->SceneGalaxy(), channel->Range(), channel->Scope());
	channel->SendChat(msg);

	// Close the parent dialog.
	svcdlg->accept();
}

quint8 AuRepairDamage::MinQ() const
{
	return m_MinQ->value();
}

