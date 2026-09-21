//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auagentempire.cpp,v 1.18 2022/02/19 18:03:38 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auagentempire.h"
//
#include "auapp.h"
#include "auempirewait.h"
#include "auframe.h"
#include "auglobal.h"
#include "aupiratebuilding.h"
#include "dgdata.h"
#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

AuAgentEmpire::AuAgentEmpire(QWidget* parent) : QWidget(parent)
{
	QLabel* empireLabel = new QLabel("Empire");

	QPushButton* pbrefreshempires = new QPushButton("Request Full Empires List");
	connect(pbrefreshempires, SIGNAL(clicked()), SLOT(RefreshEmpires()));
	pbrefreshempires->setToolTip("<html>Requests the server to send you the complete list of empires and characters. "
			"This may take a minute or two.</html>");

	// Change empire data
	m_Empires = new QComboBox;

	m_EmpireCitizens = new QComboBox;

	const DgData& gd = g_Au.LockDataForRead();
	gd.ListAllAvatars(*m_EmpireCitizens);
	gd.ListAllEmpires(*m_Empires);
	bool isProgrammer = gd.ClientPlayerIsProgrammer();	// also true if architect
	g_Au.UnlockData();

	m_Empires->setCurrentIndex(-1);
	m_EmpireCitizens->setCurrentIndex(-1);
	connect(m_Empires, SIGNAL(currentIndexChanged(int)), SLOT(EmpireSelected(int)));

	m_EmpireName = new QLabel;

	m_EmpireRole = new QLabel;

	m_Flag = new QLabel;

	QPushButton* pbchgname = new QPushButton("Change Empire Name...");
	connect(pbchgname, SIGNAL(clicked()), SLOT(EmpireChgName()));

	//QPushButton* pbchgrole = new QPushButton("Change Empire Role...");
	//connect(pbchgrole, SIGNAL(clicked()), SLOT(EmpireChgRole()));

	QPushButton* pbchgflag = new QPushButton("Change Empire Flag...");
	connect(pbchgflag, SIGNAL(clicked()), SLOT(EmpireChgFlag()));

	QPushButton* pbchgemp = new QPushButton("Change Emperor...");
	connect(pbchgemp, SIGNAL(clicked()), SLOT(EmpireChgEmperor()));

	QPushButton* pbpiratebd;
	QPushButton* pbpiratestation;
	QPushButton* pbpirateexplorer;
	QPushButton* pbpiratefreighter;
	QPushButton* pbpiratemilitary;
	if(isProgrammer)
	{
		pbpiratebd = new QPushButton("Pirate Building...");
		connect(pbpiratebd, SIGNAL(clicked()), SLOT(PirateSetBuilding()));
		pbpiratebd->setToolTip(tr("<html>"
					"Selects a blueprint for a pirate empire to use when building a specific industry. "
					"</html>"));

		pbpiratestation = new QPushButton("Pirate Space Station...");
		connect(pbpiratestation, SIGNAL(clicked()), SLOT(PirateSetSpaceStation()));
		pbpiratestation->setToolTip(tr("<html>"
					"Selects a blueprint for a pirate empire to use when building a space station. "
					"</html>"));

		pbpirateexplorer = new QPushButton("Pirate Explorer Spacecraft...");
		connect(pbpirateexplorer, SIGNAL(clicked()), SLOT(PirateSetExplorerShip()));
		pbpirateexplorer->setToolTip(tr("<html>"
					"Selects a blueprint for a pirate empire to use when building exploration spacecraft. "
					"</html>"));

		pbpiratefreighter = new QPushButton("Pirate Freighter Spacecraft...");
		connect(pbpiratefreighter, SIGNAL(clicked()), SLOT(PirateSetFreighterShip()));
		pbpiratefreighter->setToolTip(tr("<html>"
					"Selects a blueprint for a pirate empire to use when building freighter spacecraft. "
					"</html>"));

		pbpiratemilitary = new QPushButton("Pirate Military Spacecraft...");
		connect(pbpiratemilitary, SIGNAL(clicked()), SLOT(PirateSetMilitaryShip()));
		pbpiratemilitary->setToolTip(tr("<html>"
					"Selects a blueprint for a pirate empire to use when building military spacecraft. "
					"</html>"));
	}
	else
	{
		pbpiratebd = 0;
		pbpiratestation = 0;
		pbpirateexplorer = 0;
		pbpiratefreighter = 0;
		pbpiratemilitary = 0;
	}

	QVBoxLayout* flay = new QVBoxLayout;
	flay->addWidget(m_EmpireName);
	flay->addWidget(m_Flag);
	flay->addWidget(m_EmpireRole);
	flay->addStretch();

	QGridLayout* elay = new QGridLayout;
	int row=0;
	elay->addWidget(pbchgname, row,0);
	if(pbpiratebd)
		elay->addWidget(pbpiratebd, row,1);
	if(pbpirateexplorer)
		elay->addWidget(pbpirateexplorer, row,2);
	row++;
	elay->addWidget(pbchgflag, row,0);
	if(pbpiratefreighter)
		elay->addWidget(pbpiratefreighter, row,2);
	row++;
	//elay->addWidget(pbchgrole, row,0);
	if(pbpiratemilitary)
		elay->addWidget(pbpiratemilitary, row,2);
	row++;
	if(pbpiratestation)
		elay->addWidget(pbpiratestation, row,2);
	row++;

	QHBoxLayout* play = new QHBoxLayout;
	play->addWidget(m_EmpireCitizens);
	play->addWidget(pbchgemp);

	QGridLayout* layemp = new QGridLayout;
	row=0;
	layemp->addWidget(pbrefreshempires, row,2);
	row++;
	layemp->addWidget(empireLabel, row,0);
	layemp->addWidget(m_Empires, row,1, 1,2);
	row++;
	layemp->addLayout(flay, row,1);
	layemp->addLayout(elay, row,2);
	row++;
	layemp->addLayout(play, row,1, 1,2);
	row++;
	layemp->setRowStretch(row,1);
	row++;
	setLayout(layemp);
}

AuAgentEmpire::~AuAgentEmpire()
{
}

void AuAgentEmpire::EmpireChgFlag()
{
	int index = m_Empires->currentIndex();
	if(index < 0)
		return;
	quint32 empireId = m_Empires->itemData(index).toUInt();

	// Load last known file folder.
	QString lastFile = g_Au.Settings().readEntry("/Settings/LastFlagFile", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();
	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.
	QString file = QFileDialog::getOpenFileName(this, tr("Change Empire Flag"), lastFile, "", 0,
			g_Au.UseNativeFileDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
	if(!file.isEmpty())
	{
		// Store last known file folder.
		g_Au.Settings().writeEntry("/Settings/LastFlagFile", file);

		QImage imFlag;
		if(imFlag.load(file))
		{
			if(imFlag.width() > DgEmpire::FlagImageMax() || imFlag.height() > DgEmpire::FlagImageMax())
			{
				imFlag = imFlag.scaled(DgEmpire::FlagImageMax(),DgEmpire::FlagImageMax(), Qt::KeepAspectRatio,
						Qt::SmoothTransformation);
			}
			else
			{
				if(imFlag.width() < 2 || imFlag.height() < 2)
				{
					QMessageBox::critical(this, g_Au.ApplicationName(),
							tr("<html>"
								"The flag picture is too small. "
								"Please select a picture that is at least 2 pixels in width and height. "
								"</html>"),
							QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
					imFlag = QImage();	// Discard invalid image.
				}
			}
			imFlag = imFlag.convertToFormat(QImage::Format_Indexed8);	// Reduce color depth for size.

			if(!imFlag.isNull())
			{
				auApp->EmpireChangeFlag(empireId, imFlag);
				QMessageBox::information(this, tr("Change Empire Flag"), tr("Empire flag change request sent to server."));
				m_Flag->setPixmap(QPixmap::fromImage(imFlag));
			}
		}
		else
		{
			QMessageBox::critical(this, g_Au.ApplicationName(),
				"The flag image could not be loaded. Many image formats are\n"
				"supported but some JPEG sub-formats give us trouble. If the image\n"
				"can be loaded with a paint program, try saving it as a different\n"
				"format of image such as PNG or BMP.",
				QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
		}
	}
}

void AuAgentEmpire::EmpireChgName()
{
	int index = m_Empires->currentIndex();
	if(index < 0)
		return;
	quint32 empireId = m_Empires->itemData(index).toUInt();

	QString name;
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireName(empireId, name);
	g_Au.UnlockData();

	bool ok;
	name = QInputDialog::getText(this, tr("Change Empire Name"), tr("Enter a new name for the empire."),
			QLineEdit::Normal, name, &ok).trimmed();
	const DgData& gd2 = g_Au.LockDataForRead();
	quint32 exemp = gd2.EmpireId(name);
	g_Au.UnlockData();
	if(exemp)
	{
		QMessageBox::warning(this, tr("Change Empire Name"),
				tr("That empire name is already in use. Please choose a unique name."));
		return;
	}
	if(ok && IsNameValid(name))
	{
		auApp->EmpireChangeName(empireId, name);
		m_EmpireName->setText(name);
	}
}

void AuAgentEmpire::EmpireChgRole()
{
/*
	int index = m_Empires->currentIndex();
	if(index < 0)
		return;
	quint32 empireId = m_Empires->itemData(index).toUInt();

	QString name;
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireName(empireId, name);
	quint8 curRole = gd.EmpireRole(empireId);
	g_Au.UnlockData();

	// 0=normal empire, 1=pirate empire, 2=admin, 3=arena
	QStringList roleList;
	roleList << tr("Normal Empire");
	roleList << tr("Pirate Empire");
	roleList << tr("Agent Empire");
	roleList << tr("Arena Empire");

	bool ok;
	QString roleName = QInputDialog::getItem(this, tr("Change Empire Role"),
			QObject::tr("Choose a role for the empire. "
				"Game role is not sent to the client so the current role is unknown."),
			roleList, curRole, false, &ok);
	if(ok)
	{
		quint8 newRole;
		if(roleName == tr("Normal Empire"))
			newRole = 0;
		else if(roleName == tr("Pirate Empire"))
			newRole = 1;
		else if(roleName == tr("Agent Empire"))
			newRole = 2;
		else if(roleName == tr("Arena Empire"))
			newRole = 3;
		else
			return;

		QByteArray ba; QDataStream out(&ba, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_1);
		out << (quint8)DtChatChannelPacket::PZ_EmpireChgRole;
		out << (quint32)empireId;
		out << newRole;
		auApp->Send ChatPacket(ba);
	}

	m_EmpireRole->setText(roleName);
*/
}

void AuAgentEmpire::EmpireSelected(int index)
{
	quint32 empireId = m_Empires->itemData(index).toUInt();

	QString name;
	QImage flag;
	const DgData& gd = g_Au.LockDataForRead();
	gd.EmpireName(empireId, name);
	gd.EmpireFlag(empireId, flag);
	bool isPirateReady = gd.IsPirateEmpireReady(empireId);
	AuId emperorId = gd.EmpireEmperor(empireId);
	//quint8 curRole = gd.EmpireRole(empireId);
	g_Au.UnlockData();
	int curRole;
	if(empireId > PIRATE_Invalid && empireId < PIRATE_Count)
		curRole = 1;
	else if(empireId == 102 || empireId == 1441)
		curRole = 2;
	else
		curRole = 0;

	// Select the current emperor in the combo box, so the agent knows who it is.
	m_EmpireCitizens->setCurrentIndex(m_EmpireCitizens->findData(emperorId.GetID()));

	m_EmpireName->setText(name);
	m_Flag->setPixmap(QPixmap::fromImage(flag));

	// 0=normal empire, 1=pirate empire, 2=admin, 3=arena
	QString roleName;
	switch(curRole)
	{
	case 0:	// 0=normal empire
		roleName = tr("Normal Empire");
		break;
	case 1:	// 1=pirate empire
		roleName = tr("Pirate Empire");
		if(!isPirateReady)
			roleName += tr("<br><b>Not Ready</b>");
		break;
	case 2:	// 2=admin
		roleName = tr("Agent Empire");
		break;
	case 3:	// 3=arena
		roleName = tr("Arena Empire");
		break;
	}
	m_EmpireRole->setText(roleName);
}

void AuAgentEmpire::EmpireChgEmperor()
{
	int index = m_Empires->currentIndex();
	if(index < 0)
		return;
	quint32 empireId = m_Empires->itemData(index).toUInt();
	QString empireName = m_Empires->itemText(index);

	index = m_EmpireCitizens->currentIndex();
	if(index < 0)
		return;
	AuId charId = AuId(m_EmpireCitizens->itemData(index).toUInt());
	QString charName = m_EmpireCitizens->itemText(index);

	int button = QMessageBox::question(this, g_Au.ApplicationName(),
		tr("%2 will become the emperor of %1.").arg(empireName).arg(charName),
		QMessageBox::Ok, QMessageBox::Cancel);
	if(button == QMessageBox::Ok)
		auApp->EmpireChangeEmperor(empireId, charId, charName);
}

void AuAgentEmpire::setVisible(bool visible)
{
	QWidget::setVisible(visible);

	if(visible)
		VisibilityRestored();
}

void AuAgentEmpire::VisibilityRestored()
{
	if(!m_DataValid)
		RefreshData();
}

void AuAgentEmpire::EmpireDataInvalid()
{
	m_DataValid = false;
	if(isVisible())
		RefreshData();
}

void AuAgentEmpire::RefreshData()
{
	// Empire Citizens
	{
		// Get info about current selection in combo.
		int curIdx = m_EmpireCitizens->currentIndex();
		AuId citizenId;
		if(curIdx >= 0)
			citizenId = AuId(m_EmpireCitizens->itemData(curIdx).toUInt());

		// Fill the combo with fresh data.
		m_EmpireCitizens->blockSignals(true);
		m_EmpireCitizens->clear();
		const DgData& gd = g_Au.LockDataForRead();
		gd.ListAllAvatars(*m_EmpireCitizens);
		g_Au.UnlockData();

		// Reselect current item, or none at all.
		if(!citizenId.IsNull())
			curIdx = m_EmpireCitizens->findData(citizenId.GetID());
		else
			curIdx = -1;
		m_EmpireCitizens->setCurrentIndex(curIdx);
		m_EmpireCitizens->blockSignals(false);
	}

	// Empires
	{
		// Get info about current selection in combo.
		int curIdx = m_Empires->currentIndex();
		quint32 empireId;
		if(curIdx >= 0)
			empireId = m_Empires->itemData(curIdx).toUInt();
		else
			empireId = 0;

		// Fill the combo with fresh data.
		m_Empires->blockSignals(true);
		m_Empires->clear();
		const DgData& gd = g_Au.LockDataForRead();
		gd.ListAllEmpires(*m_Empires);
		g_Au.UnlockData();

		// Reselect current item, or none at all.
		if(empireId)
			curIdx = m_Empires->findData(empireId);
		else
			curIdx = -1;
		m_Empires->setCurrentIndex(curIdx);
		m_Empires->blockSignals(false);
	}

	m_DataValid = true;
}

void AuAgentEmpire::RefreshEmpires()
{
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;

	auApp->RequestEmpiresAll();

	emit EmpireDataRequested();

	AuEmpireWait waitDlg(this);
	connect(this, SIGNAL(EmpireDataComplete()), &waitDlg, SLOT(accept()));
	pFrame->ExecDialog(waitDlg);

	EmpireDataInvalid();
}

void AuAgentEmpire::PirateSetBuilding()
{
	// Get the id of the current pirate empire.
	int curIdx = m_Empires->currentIndex();
	quint32 empireId;
	if(curIdx >= 0)
		empireId = m_Empires->itemData(curIdx).toUInt();
	else
		empireId = 0;
	if(empireId <= PIRATE_Invalid || empireId >= PIRATE_Count)
		return;

	AuPirateBuilding dlg(empireId, this);
	if(dlg.exec() != QDialog::Accepted)
		return;
	//qDebug() << "AuAgentEmpire::PirateSetBuilding dlg down";
	INDUSTRYID iid = dlg.Industry();
	if(iid == IID_None)
		return;
	//qDebug() << "AuAgentEmpire::PirateSetBuilding iid" << iid;
	quint32 bp = dlg.Blueprint();
	if(bp == 0)
		return;	// Not sure if I should allow 0 to erase.
	//qDebug() << "AuAgentEmpire::PirateSetBuilding bp" << bp;

	PirateSetIndustryBuilding(empireId, iid, bp);
}

void AuAgentEmpire::PirateSetIndustryBuilding(quint32 empireId, INDUSTRYID industry, quint32 blueprint)
{
	//qDebug() << "AuAgentEmpire::PirateSetIndustryBuilding" << empireId << industry << blueprint;
	auApp->PirateSetIndustryBuilding(empireId, industry, blueprint);
}

void AuAgentEmpire::PirateSetSpacecraft(int scrole)
{
	// Get info about current selection in combo.
	int curIdx = m_Empires->currentIndex();
	quint32 empireId;
	if(curIdx >= 0)
		empireId = m_Empires->itemData(curIdx).toUInt();
	else
		empireId = 0;
	if(empireId <= PIRATE_Invalid || empireId >= PIRATE_Count)
		return;

	const DgData& gd = g_Au.LockDataForRead();
	QString ename;
	gd.EmpireName(empireId, ename);
	AuScPref scpref = gd.EmpirePreferredBlueprints(empireId, (AuScPrefs::SCROLE)scrole);
	g_Au.UnlockData();
	QString rname = AuScPrefs::RoleName((AuScPrefs::SCROLE)scrole);
	QString designDef;
	if(!scpref.Blueprints().empty())
		designDef = QString("%1").arg(scpref.Blueprints().first());

	bool ok;
	QString sid = QInputDialog::getText(this, tr("%1 Blueprint Id").arg(rname),
			tr("<html>"
				"Enter the blueprint number for the %2 empire to use for <b>%1</b>. "
				"<br>"
				"The current blueprint is shown. "
				"</html>").arg(rname).arg(ename),
			QLineEdit::Normal, designDef, &ok);
	if(!ok)
		return;

	quint32 designId = QVariant(sid).toUInt();
	if(!designId)
		return;

	auApp->PirateSetSpacecraft(empireId, scrole, designId);
}

void AuAgentEmpire::PirateSetExplorerShip()
{
	PirateSetSpacecraft(AuScPrefs::SCROLE_Explorer);
}

void AuAgentEmpire::PirateSetFreighterShip()
{
	PirateSetSpacecraft(AuScPrefs::SCROLE_Freighter);
}

void AuAgentEmpire::PirateSetMilitaryShip()
{
	PirateSetSpacecraft(AuScPrefs::SCROLE_Military);
}

void AuAgentEmpire::PirateSetSpaceStation()
{
	PirateSetSpacecraft(AuScPrefs::SCROLE_Station);
}

