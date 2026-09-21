//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: austarmaphighlightdialog.cpp,v 1.6 2022/02/19 18:16:59 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "austarmaphighlightdialog.h"
//
#include "aucheckbox.h"
#include "dgdata.h"
#include <QCheckBox>
#include <QLabel>
#include <QMultiMap>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QVBoxLayout>

AuStarMapHighlightDialog::AuStarMapHighlightDialog(quint32 viewerEmpireId,
		bool showEmpirePlaces,
		bool showFriendPlaces,
		bool showNeutralPlaces,
		bool showEnemyPlaces,
		bool showUnknownPlaces,
		bool showExploredPlaces,
		bool showDetectedPlaces,
		bool unknownPrecedesEnemy,
		const QList<quint32>& empiresIgnored,
		QWidget* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":data/Settings.png"));
	setAuDlgTitle(tr("Star Map Highlighting"));

	QLabel* lbHighlight = new QLabel(tr("Highlighting"), this);

	m_ShowEmpire = new QCheckBox(tr("Empire City Present"), this);
	m_ShowEmpire->setCheckState(showEmpirePlaces ? Qt::Checked : Qt::Unchecked);
	m_ShowEmpire->setIcon(QIcon(":data/SpotEmpire.png"));
	m_ShowEmpire->setToolTip(tr("<html>"
				"Explored solar systems and sectors will be highlighted with a blue color when a city of your empire or a vassal "
				"empire is present."
				"</html>"));

	m_ShowFriend = new QCheckBox(tr("Friend City Present"), this);
	m_ShowFriend->setCheckState(showFriendPlaces ? Qt::Checked : Qt::Unchecked);
	m_ShowFriend->setIcon(QIcon(":data/SpotFriend.png"));
	m_ShowFriend->setToolTip(tr("<html>"
				"Explored solar systems and sectors will be highlighted with a green color when a city of a friend empire is "
				"present."
				"</html>"));

	m_ShowNeutral = new QCheckBox(tr("Neutral City Present"), this);
	m_ShowNeutral->setCheckState(showNeutralPlaces ? Qt::Checked : Qt::Unchecked);
	m_ShowNeutral->setIcon(QIcon(":data/SpotNeutral.png"));
	m_ShowNeutral->setToolTip(tr("<html>"
				"Explored solar systems and sectors will be highlighted with a light grey color when a city of a neutral empire is "
				"present."
				"</html>"));

	m_ShowEnemy = new QCheckBox(tr("Enemy City Present"), this);
	m_ShowEnemy->setCheckState(showEnemyPlaces ? Qt::Checked : Qt::Unchecked);
	m_ShowEnemy->setIcon(QIcon(":data/SpotEnemy.png"));
	m_ShowEnemy->setToolTip(tr("<html>"
				"Explored solar systems and sectors will be highlighted with a red color when a city of an enemy empire is present."
				"</html>"));

	m_ShowUnknown = new QCheckBox(tr("Unknown City Present"), this);
	m_ShowUnknown->setCheckState(showUnknownPlaces ? Qt::Checked : Qt::Unchecked);
	m_ShowUnknown->setIcon(QIcon(":data/SpotUnknown.png"));
	m_ShowUnknown->setToolTip(tr("<html>"
				"Explored solar systems and sectors will be highlighted with a pink color when a city of an unknown empire is "
				"present."
				"</html>"));

	m_ShowExplored = new QCheckBox(tr("Explored Solar System"), this);
	m_ShowExplored->setCheckState(showExploredPlaces ? Qt::Checked : Qt::Unchecked);
	m_ShowExplored->setIcon(QIcon(":data/SpotExplored.png"));
	m_ShowExplored->setToolTip(tr("<html>"
				"Solar systems will be highlighted with a dark green color when they have been explored by your empire."
				"</html>"));

	m_ShowDetected = new QCheckBox(tr("Detected Warp Signature in Solar System"), this);
	m_ShowDetected->setCheckState(showDetectedPlaces ? Qt::Checked : Qt::Unchecked);
	m_ShowDetected->setIcon(QIcon(":data/SpotDetected.png"));
	m_ShowDetected->setToolTip(tr("<html>"
				"Unexplored solar systems will be highlighted with a bright blue white color "
				"when a warp signature has been detected by your empire. "
				"Your empire must have radars and warp technology or spacecraft with sensors and warp drives "
				"to detect warp signatures."
				"</html>"));

	QLabel* lbPrecedence = new QLabel(tr("Highlight Precedence"), this);

	m_EnemyPrecedent = new QRadioButton(tr("Enemy empires take precedence over unknown empires."), this);
	m_EnemyPrecedent->setChecked(!unknownPrecedesEnemy);
	m_EnemyPrecedent->setIcon(QIcon(":data/SpotEnemy.png"));
	m_EnemyPrecedent->setToolTip(tr("<html>"
				"Explored solar systems and sectors will be highlighted with a red color when cities of enemy and unknown empires "
				"are present."
				"</html>"));

	m_UnknownPrecedent = new QRadioButton(tr("Unknown empires take precedence over enemy empires."), this);
	m_UnknownPrecedent->setChecked(unknownPrecedesEnemy);
	m_UnknownPrecedent->setIcon(QIcon(":data/SpotUnknown.png"));
	m_UnknownPrecedent->setToolTip(tr("<html>"
				"Explored solar systems and sectors will be highlighted with a pink color when cities of enemy and unknown empires "
				"are present."
				"</html>"));

	QLabel* lbEmpires = new QLabel(tr("Empires to Show"), this);

	QPushButton* allEmpires = new QPushButton(tr("All"), this);
	allEmpires->setToolTip(tr("<html>"
				"Checks the check box for all known empires."
				"</html>"));
	connect(allEmpires, SIGNAL(clicked()), SLOT(CheckAllEmpires()));

	QPushButton* noEmpires = new QPushButton(tr("None"), this);
	noEmpires->setToolTip(tr("<html>"
				"Unchecks the check box for all known empires."
				"</html>"));
	connect(noEmpires, SIGNAL(clicked()), SLOT(UncheckAllEmpires()));

	QPushButton* invertEmpires = new QPushButton(tr("Invert"), this);
	invertEmpires->setToolTip(tr("<html>"
				"Inverts the check box for all known empires. "
				"Check boxes that are checked will be unchecked. "
				"Check boxes that are unchecked will be checked."
				"</html>"));
	connect(invertEmpires, SIGNAL(clicked()), SLOT(InvertCheckedEmpires()));

	// Construct scroll area of check boxes for empires.
	QScrollArea* empireScroller = new QScrollArea;
	{
		QWidget* empires = new QWidget;

		QGridLayout* elay = new QGridLayout;

		// Alphabetize the list of empires known to the viewer's empire using a map.
		const DgData& gd = g_Au.LockDataForRead();
		DgEmpireHash::const_iterator viewerEi = gd.m_gEmpires.find(viewerEmpireId);
		if(viewerEi != gd.m_gEmpires.cend())
		{
			QMultiMap<QString, quint32> empiresSorted;	// map to sort empires by name. multi because this could make duplicates
			empiresSorted.insert((*viewerEi).second.Name().toUpper(), viewerEmpireId);
			for(unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::const_iterator ki = (*viewerEi).second.m_Stance.begin();
					ki != (*viewerEi).second.m_Stance.end(); ki++)
				empiresSorted.insert((*ki).second.m_name.toUpper(), (*ki).first);

			// Create a check box for every empire.
			int erow=0;
			for(QMultiMap<QString, quint32>::const_iterator si = empiresSorted.cbegin(); si != empiresSorted.cend(); si++)
			{
				QString name;
				if(*si == viewerEmpireId)
					name = (*viewerEi).second.Name();
				else
				{
					unordered_map<quint32, AuKnownEmpire, AuEmpireIdHash>::const_iterator ki =
						(*viewerEi).second.m_Stance.find(*si);
					Q_ASSERT(ki != (*viewerEi).second.m_Stance.end());
					name = (*ki).second.m_name;
				}

				AuCheckBoxValue* cb = new AuCheckBoxValue(name, *si);
				cb->setCheckState(empiresIgnored.contains(*si) ? Qt::Unchecked : Qt::Checked);
				cb->setToolTip(tr("<html>"
							"When checked, cities belonging to %1 will be included when highlighting solar systems and sectors.<br>"
							"When unchecked, cities belonging to %1 will be excluded when highlighting solar systems and sectors."
							"</html>").arg(name));
				m_EmpireChecks.append(cb);

				// Get the flag, if we have it locally. It may not be available.
				QLabel* lbflag = new QLabel;
				DgEmpireHash::const_iterator ei = gd.m_gEmpires.find(*si);
				if(ei != gd.m_gEmpires.end())
				{
					QPixmap pixflag;
					pixflag.convertFromImage((*ei).second.m_flag);
					lbflag->setPixmap(pixflag);
				}

				elay->addWidget(lbflag, erow,0);
				elay->addWidget(cb, erow,1);
				erow++;
			}
		}
		g_Au.UnlockData();

		elay->setMargin(2);
		elay->setSpacing(2);
		empires->setLayout(elay);

		empireScroller->setWidget(empires);
	}

	QPushButton* pbok = new QPushButton(tr("Apply"), this);
	pbok->setAutoDefault(true);
	pbok->setDefault(true);
	pbok->setToolTip(tr("<html>Highlights places in the star map using these settings.</html>"));
	connect(pbok, SIGNAL(clicked()), SLOT(ApplyFilter()));

	QPushButton* pbclear = new QPushButton(tr("Default"), this);
	pbok->setToolTip(tr("<html>Highlights places in the star map using default settings. "
				"Highlight precedence is not affected.</html>"));
	connect(pbclear, SIGNAL(clicked()), SLOT(ClearFilter()));

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(lbHighlight, row,0, 1,4);
	row++;
	lay->addWidget(m_ShowEmpire, row,1, 1,2);
	lay->addWidget(pbok, row,3);
	row++;
	lay->addWidget(m_ShowFriend, row,1, 1,2);
	lay->addWidget(pbclear, row,3);
	row++;
	lay->addWidget(m_ShowNeutral, row,1, 1,3);
	row++;
	lay->addWidget(m_ShowEnemy, row,1, 1,3);
	row++;
	lay->addWidget(m_ShowUnknown, row,1, 1,3);
	row++;
	lay->addWidget(m_ShowExplored, row,1, 1,3);
	row++;
	lay->addWidget(m_ShowDetected, row,1, 1,3);
	row++;
	lay->addWidget(lbPrecedence, row,0, 1,4);
	row++;
	lay->addWidget(m_EnemyPrecedent, row,1, 1,3);
	row++;
	lay->addWidget(m_UnknownPrecedent, row,1, 1,3);
	row++;
	lay->addWidget(lbEmpires, row,0, 1,4);
	row++;
	lay->addWidget(allEmpires, row,1);
	lay->addWidget(noEmpires, row,2);
	lay->addWidget(invertEmpires, row,3);
	row++;
	lay->addWidget(empireScroller, row,1, 1,3);
	row++;
	lay->setColumnMinimumWidth(0, 16);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuStarMapHighlightDialog::~AuStarMapHighlightDialog()
{
}

void AuStarMapHighlightDialog::setVisible(bool visible)
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

void AuStarMapHighlightDialog::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/GalaxyFilterDialog/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/GalaxyFilterDialog/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/GalaxyFilterDialog/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/GalaxyFilterDialog/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuStarMapHighlightDialog::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/GalaxyFilterDialog/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/GalaxyFilterDialog/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/GalaxyFilterDialog/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/GalaxyFilterDialog/SizeY",sz.height());
}

void AuStarMapHighlightDialog::ApplyFilter()
{
	accept();
}

void AuStarMapHighlightDialog::ClearFilter()
{
	m_ShowEmpire->setCheckState(Qt::Checked);
	m_ShowFriend->setCheckState(Qt::Checked);
	m_ShowNeutral->setCheckState(Qt::Checked);
	m_ShowEnemy->setCheckState(Qt::Checked);
	m_ShowUnknown->setCheckState(Qt::Checked);
	m_ShowExplored->setCheckState(Qt::Checked);
	m_ShowDetected->setCheckState(Qt::Checked);

	m_EnemyPrecedent->setChecked(false);
	m_UnknownPrecedent->setChecked(true);

	CheckAllEmpires();

	accept();
}

void AuStarMapHighlightDialog::CheckAllEmpires()
{
	for(QList<AuCheckBoxValue*>::iterator ci = m_EmpireChecks.begin(); ci != m_EmpireChecks.end(); ci++)
		if((*ci)->checkState() != Qt::Checked)
			(*ci)->setCheckState(Qt::Checked);
}

void AuStarMapHighlightDialog::InvertCheckedEmpires()
{
	for(QList<AuCheckBoxValue*>::iterator ci = m_EmpireChecks.begin(); ci != m_EmpireChecks.end(); ci++)
		if((*ci)->checkState() == Qt::Checked)
			(*ci)->setCheckState(Qt::Unchecked);
		else
			(*ci)->setCheckState(Qt::Checked);
}

void AuStarMapHighlightDialog::UncheckAllEmpires()
{
	for(QList<AuCheckBoxValue*>::iterator ci = m_EmpireChecks.begin(); ci != m_EmpireChecks.end(); ci++)
		if((*ci)->checkState() == Qt::Checked)
			(*ci)->setCheckState(Qt::Unchecked);
}

QList<quint32> AuStarMapHighlightDialog::IgnoredEmpires() const
{
	QList<quint32> ignored;
	for(QList<AuCheckBoxValue*>::const_iterator ci = m_EmpireChecks.cbegin(); ci != m_EmpireChecks.cend(); ci++)
		if((*ci)->checkState() != Qt::Checked)
			ignored.append((*ci)->Value());
	return ignored;
}

bool AuStarMapHighlightDialog::ShowEmpire() const
{
	return m_ShowEmpire->checkState() == Qt::Checked;
}

bool AuStarMapHighlightDialog::ShowEnemy() const
{
	return m_ShowEnemy->checkState() == Qt::Checked;
}

bool AuStarMapHighlightDialog::ShowExplored() const
{
	return m_ShowExplored->checkState() == Qt::Checked;
}

bool AuStarMapHighlightDialog::ShowDetected() const
{
	return m_ShowDetected->checkState() == Qt::Checked;
}

bool AuStarMapHighlightDialog::ShowFriend() const
{
	return m_ShowFriend->checkState() == Qt::Checked;
}

bool AuStarMapHighlightDialog::ShowNeutral() const
{
	return m_ShowNeutral->checkState() == Qt::Checked;
}

bool AuStarMapHighlightDialog::ShowUnknown() const
{
	return m_ShowUnknown->checkState() == Qt::Checked;
}

bool AuStarMapHighlightDialog::UnknownPrecedesEnemy() const
{
	return m_UnknownPrecedent->isChecked();
}

bool AuStarMapHighlightDialog::EnemyPrecedesUnknown() const
{
	return m_EnemyPrecedent->isChecked();
}
