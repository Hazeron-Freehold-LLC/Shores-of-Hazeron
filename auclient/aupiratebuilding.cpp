//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aupiratebuilding.cpp,v 1.7 2022/02/19 18:16:34 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aupiratebuilding.h"
//
#include "auglobal.h"
#include "dgdata.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AuPirateBuilding::AuPirateBuilding(quint32 empireId, QWidget* parent) : AuDialog(parent)
{
	m_EmpireId = empireId;

	// Get the list of all possible industries built by the pirate empire, in the order they are first needed.
	QString ename;
	QList<INDUSTRYID> baseBuildings;
	QMap<QString, bool> pirateIndustries;
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireName(m_EmpireId, ename);
	gd.PirateBaseBuildings(m_EmpireId, baseBuildings);
	QSet<INDUSTRYID> listed;
	for(QList<INDUSTRYID>::const_iterator bi = baseBuildings.cbegin(); bi != baseBuildings.cend(); bi++)
	{
		if(listed.contains(*bi))
			continue;
		listed.insert(*bi);
		pirateIndustries.insert(gd.IndustryName(*bi), true);
	}
	g_Au.UnlockData();

	setAuDlgTitle(tr("%1 Building Blueprint").arg(ename));
	setAuDlgIcon(QIcon(":data/City.png"));

	QLabel* lbinfo = new QLabel(tr("<html>"
				"<p>"
				"Submit a building blueprint for use by <b>%1</b> when constructing bases. "
				"</p>"
				"<p>"
				"Choose what industry will be built using the blueprint. "
				"Every industry must have a blueprint before %1 can operate in the game. "
				"</p>"
				"</html>").arg(ename), this);
	lbinfo->setWordWrap(true);

	m_Industries = new QComboBox(this);
	for(QMap<QString, bool>::const_iterator ii = pirateIndustries.cbegin(); ii != pirateIndustries.cend(); ii++)
		m_Industries->addItem(ii.key());
	m_Industries->setToolTip(tr("<html"
				"Not all pirates build the same buildings. "
				"Only the industries built by the current pirate empire are shown. "
				"</html>"));
	m_Industries->setCurrentIndex(-1);
	connect(m_Industries, SIGNAL(currentIndexChanged(const QString&)), SLOT(IndustryChanged(const QString&)));

	QLabel* lbdesign = new QLabel(tr("<html>"
				"Enter the blueprint number to use when building the industry. "
				"<b>It must work in all environments.</b>"
				"<br>"
				"The current blueprint for the industry is shown, if any. "
				"</html>"), this);
	lbdesign->setWordWrap(true);

	m_Blueprint = new QLineEdit(this);

	QPushButton* pbok = new QPushButton(tr("Ok"), this);
	pbok->setDefault(true);
	pbok->setAutoDefault(true);
	connect(pbok, SIGNAL(clicked()), SLOT(accept()));

	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(pbok);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(lbinfo);
	lay->addWidget(m_Industries);
	lay->addWidget(lbdesign);
	lay->addWidget(m_Blueprint);
	lay->addLayout(blay);
	setAuDlgLayout(lay);
}

AuPirateBuilding::~AuPirateBuilding()
{
}

void AuPirateBuilding::setVisible(bool visible)
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

void AuPirateBuilding::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/PirateBuilding/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/PirateBuilding/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/PirateBuilding/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/PirateBuilding/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuPirateBuilding::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/PirateBuilding/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/PirateBuilding/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/PirateBuilding/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/PirateBuilding/SizeY",sz.height());
}

quint32 AuPirateBuilding::Blueprint() const
{
	return m_Blueprint->text().toUInt();
}

INDUSTRYID AuPirateBuilding::Industry() const
{
	QString iname = m_Industries->currentText();
	const DgData& gd = g_Au.LockDataForRead();
	INDUSTRYID iid = gd.IndustryId(iname);
	g_Au.UnlockData();
	return iid;
}

void AuPirateBuilding::IndustryChanged(const QString& iname)
{
	const DgData& gd = g_Au.LockDataForRead();
	INDUSTRYID iid = gd.IndustryId(iname);
	AuBdPref bdpref = gd.EmpirePreferredBlueprints(m_EmpireId, iid);
	g_Au.UnlockData();
	QString defaultBp;
	const QList<AuBlueprintPref>& bp = bdpref.Blueprints();
	if(!bp.empty())
		defaultBp = QString("%1").arg(bp.first().BlueprintId());
	m_Blueprint->setText(defaultBp);
}
