//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auindustrydlg.cpp,v 1.4 2022/02/19 18:16:10 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auindustrydlg.h"
//
#include "auglobal.h"
#include "dgdata.h"
#include <QComboBox>
#include <QLabel>
#include <QTextBrowser>
#include <QVBoxLayout>

AuIndustryDlg::AuIndustryDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("Building Codes"));
	setAuDlgIcon(QIcon(":data/City.png"));

	QLabel* lbinfo = new QLabel(tr("<html>"
				"Choose an industry to view its building codes. "
				"A building blueprint must meet or exceed the <i>structural requirements</i> "
				"to be used for constructing an industry. "
				"</html>"), this);
	lbinfo->setWordWrap(true);

	// Alphabetize industries.
	QMap<QString, INDUSTRYID> ind;
	const DgData& gd = g_Au.LockDataForRead();
	for(DbIndustryArray::const_iterator ii = gd.m_bIndustryArray.cbegin(); ii != gd.m_bIndustryArray.cend(); ii++)
		if((*ii).IndustryId() != IID_None && (*ii).Structure() != BD_Invalid)
			ind.insert((*ii).m_name, (*ii).IndustryId());
	g_Au.UnlockData();

	m_IndustriesCombo = new QComboBox(this);
	for(QMap<QString, INDUSTRYID>::const_iterator si = ind.cbegin(); si != ind.cend(); si++)
		m_IndustriesCombo->addItem(si.key(), *si);
	connect(m_IndustriesCombo, SIGNAL(currentIndexChanged(int)), SLOT(IndustryChanged(int)));

	m_IndustryInfo = new QTextBrowser(this);
	m_IndustryInfo->setReadOnly(true);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbinfo);
	lay->addWidget(m_IndustriesCombo);
	lay->addWidget(m_IndustryInfo);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuIndustryDlg::~AuIndustryDlg()
{
}

void AuIndustryDlg::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuIndustryDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/IndustryDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/IndustryDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/IndustryDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/IndustryDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);

	INDUSTRYID lastInd = (INDUSTRYID)g_Au.Settings().readNumEntry("/IndustryDlg/LastIndustry", IID_None);
	m_IndustriesCombo->setCurrentIndex(m_IndustriesCombo->findData(lastInd));
}

void AuIndustryDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/IndustryDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/IndustryDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/IndustryDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/IndustryDlg/SizeY",sz.height());

	if(m_IndustriesCombo->currentIndex() >= 0)
	{
		INDUSTRYID ind = (INDUSTRYID)m_IndustriesCombo->itemData(m_IndustriesCombo->currentIndex()).toInt();
		g_Au.Settings().writeEntry("/IndustryDlg/LastIndustry", ind);
	}
	else
		g_Au.Settings().writeEntry("/IndustryDlg/LastIndustry", IID_None);
}

void AuIndustryDlg::IndustryChanged(int index)
{
	if(index < 0)
	{
		m_IndustryInfo->setHtml("");
		return;
	}

	INDUSTRYID ind = (INDUSTRYID)m_IndustriesCombo->itemData(index).toInt();
	const DgData& gd = g_Au.LockDataForRead();
	const DbIndustry& dbi = gd.Industry(ind);

	QString info = tr("<img src=\"qrc%1\" align=\"right\" height=\"120\">")
		.arg(dbi.BitmapName());

	info += "<big><b>";
	info += dbi.Name();
	info += "</b></big>";

	// Does What
	if(!dbi.DoesWhat().isEmpty())
	{
		info += "<p>";
		info += dbi.DoesWhat();
		info += "</p>";
	}

	// Build Rule
	info += "<p>";
	info += dbi.BuildRuleText();
	info += "</p>";

	if(!dbi.Note().isEmpty())
		info += dbi.Note();

	// Code Requirements
	info += tr("Structural Requirements");
	info += tr("<table>"
				"<tbody>"
					"<tr>"
						"<td>"
							"Structure"
						"</td>"
						"<td>"
							"<b>%29</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Arena"
						"</td>"
						"<td>"
							"<b>%3</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Auditorium"
						"</td>"
						"<td>"
							"<b>%4</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Battery"
						"</td>"
						"<td>"
							"<b>%5</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Commodity Storage"
						"</td>"
						"<td>"
							"<b>%6</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Field Area"
						"</td>"
						"<td>"
							"<b>%1</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Generator"
						"</td>"
						"<td>"
							"<b>%8</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Guard Post"
						"</td>"
						"<td>"
							"<b>%9</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Home, Large"
						"</td>"
						"<td>"
							"<b>%10</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Home, Medium"
						"</td>"
						"<td>"
							"<b>%30</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Home, Small"
						"</td>"
						"<td>"
							"<b>%31</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Livestock"
						"</td>"
						"<td>"
							"<b>%11</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Lounge Space"
						"</td>"
						"<td>"
							"<b>%12</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Office Space"
						"</td>"
						"<td>"
							"<b>%13</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Parking for Ground Vehicles"
						"</td>"
						"<td>"
							"<b>%14</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Parking for Space Vehicles"
						"</td>"
						"<td>"
							"<b>%15</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Parking for Water Vehicles"
						"</td>"
						"<td>"
							"<b>%16</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Parking for Spacecraft"
						"</td>"
						"<td>"
							"<b>%32</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Radar"
						"</td>"
						"<td>"
							"<b>%18</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Shield Generator"
						"</td>"
						"<td>"
							"<b>%19</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Workshop Space"
						"</td>"
						"<td>"
							"<b>%20</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Space Vehicle Launchers"
						"</td>"
						"<td>"
							"<b>%24</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Space Vehicle Recovery Systems"
						"</td>"
						"<td>"
							"<b>%25</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Store Space"
						"</td>"
						"<td>"
							"<b>%17</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Surgery Units"
						"</td>"
						"<td>"
							"<b>%21</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Transporters"
						"</td>"
						"<td>"
							"<b>%22</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Turrets"
						"</td>"
						"<td>"
							"<b>%23</b>"
						"</td>"
					"</tr>"
					"<tr>"
						"<td>"
							"Weapon Bays"
						"</td>"
						"<td>"
							"<b>%26</b>"
						"</td>"
					"</tr>"
				"</tbody>"
			"</table>")
		.arg(dbi.Field() > 0 ? tr("%1 Field Required").arg(dbi.Field()) :
				(dbi.Field() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.Arena() > 0 ? tr("%1 Arena Required").arg(dbi.Arena()) :
				(dbi.Arena() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.Auditorium() > 0 ? tr("%1 Auditorium Required").arg(dbi.Auditorium()) :
				(dbi.Auditorium() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.Battery() > 0 ? tr("%1m%2 Batteries Required").arg(dbi.Battery()).arg(g_Au.CubedSymbol()) :
				(dbi.Battery() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.CargoStorage() > 0 ? tr("%1m%2 Space Required").arg(dbi.CargoStorage()).arg(g_Au.CubedSymbol()) :
				(dbi.CargoStorage() == 0 ? tr("Ok") :
				 (dbi.CargoStorage() == -2 ? tr("Limited to 10%2m").arg(g_Au.CubedSymbol()) : tr("No"))))
		.arg(dbi.Generator() > 0 ? tr("%1 Generator Required").arg(dbi.Generator()) :
				(dbi.Generator() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.GuardPost() > 0 ? tr("%1 Post Required").arg(dbi.GuardPost()) :
				(dbi.GuardPost() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.HomeLarge() > 0 ? tr("%1 Large Home Required").arg(dbi.HomeLarge()) :
				(dbi.HomeLarge() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.LivestockPost() > 0 ? tr("%1 Post Required").arg(dbi.LivestockPost()) :
				(dbi.LivestockPost() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.Lounge() > 0 ? tr("%1 Lounge Required").arg(dbi.Lounge()) :
				(dbi.Lounge() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.Office() > 0 ? tr("%1 Office Required").arg(dbi.Office()) :
				(dbi.Office() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.ParkGround() > 0 ? tr("%1 Spot Required").arg(dbi.ParkGround()) :
				(dbi.ParkGround() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.ParkSpace() > 0 ? tr("%1 Spot Required").arg(dbi.ParkSpace()) :
				(dbi.ParkSpace() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.ParkWater() > 0 ? tr("%1 Spot Required").arg(dbi.ParkWater()) :
				(dbi.ParkWater() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.Store() > 0 ? tr("%1 Store Required").arg(dbi.Store()) :
				(dbi.Store() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.Sensor() > 0 ? tr("%1m%2 Radar Required").arg(dbi.Sensor()).arg(g_Au.CubedSymbol()) :
				(dbi.Sensor() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.Shield() > 0 ? tr("%1m%2 Shields Required").arg(dbi.Shield()).arg(g_Au.CubedSymbol()) :
				(dbi.Shield() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.Shop() > 0 ? tr("%1 Workshop Required").arg(dbi.Shop()) :
				(dbi.Shop() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.Surgery() > 0 ? tr("%1 Unit Required").arg(dbi.Surgery()) :
				(dbi.Surgery() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.Transporter() > 0 ? tr("%1 Transporter Required").arg(dbi.Transporter()) :
				(dbi.Transporter() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.Turret() > 0 ? tr("%1 Turret Required, Limit 6").arg(dbi.Turret()) :
				(dbi.Turret() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.VehicleLauncher() > 0 ? tr("%1 Launcher Required").arg(dbi.VehicleLauncher()) :
				(dbi.VehicleLauncher() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.VehicleRecovery() > 0 ? tr("%1 Recovery System Required").arg(dbi.VehicleRecovery()) :
				(dbi.VehicleRecovery() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.WeaponBay() > 0 ? tr("%1 Bay Required").arg(dbi.WeaponBay()) :
				(dbi.WeaponBay() == 0 ? tr("Ok") : tr("No")))
		.arg(BuildingStructureName(dbi.Structure()))
		.arg(dbi.HomeMedium() > 0 ? tr("%1 Medium or Large Home Required").arg(dbi.HomeMedium()) :
				(dbi.HomeMedium() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.HomeSmall() > 0 ? tr("%1 Home of Any Size Required").arg(dbi.HomeSmall()) :
				(dbi.HomeSmall() == 0 ? tr("Ok") : tr("No")))
		.arg(dbi.ParkSpacecraft() > 0 ? tr("%1 Spot Required").arg(dbi.ParkSpacecraft()) :
				(dbi.ParkSpacecraft() == 0 ? tr("Ok") : tr("No")));
	g_Au.UnlockData();
	m_IndustryInfo->setHtml(info);
}
