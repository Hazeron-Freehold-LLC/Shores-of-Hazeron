//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augovpolicy.cpp,v 1.16 2022/02/19 18:16:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augovpolicy.h"
//
#include "auapp.h"
#include "aucontrailsample.h"
#include "auglobal.h"
#include "auframe.h"
#include "aupushbutton.h"
#include "auview.h"
#include "daperson.h"
#include "dgdata.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QIcon>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSlider>
#include <QToolButton>

void AuGovPolicyDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleGovernPolicyUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGovPolicy::AuGovPolicy(QWidget* parent) : QWidget(parent)
{
	m_ChangeFlag = new QToolButton(this);
	//m_ChangeFlag->setIcon(QIcon(":data/ImperiumFlag.png"));
	//m_ChangeFlag->setIconSize(QSize(128,128));
	m_ChangeFlag->setToolTip(tr("<html><b>Empire Flag</b><br>"
				"Changes the flag of your empire.<br>"
				"<i>Tips</i><br>"
				"%1 Avatars on line right now may need to relog to see the flag change.<br>"
				"%1 Only governors with appropriate rights can change the flag.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	m_ChangeFlag->setFocusPolicy(Qt::NoFocus);
	connect(m_ChangeFlag, SIGNAL(clicked()), SLOT(ChangeFlag()));

	m_JoinPass = new QPushButton(tr("Change Join Password"), this);
	m_JoinPass->setToolTip(tr("<html>"
				"<b>Join Password</b><br>"
				"A password enables a new avatar to join this empire when they choose the <i>Citizen of the Galaxy</i> scenario. "
				"The empire must have at least one city to be joinable.<br>"
				"<i>Tips</i><br>"
				"%1 Set blank password to disable joining.<br>"
				"%1 Only governors with appropriate rights can change the join password.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(m_JoinPass, SIGNAL(clicked()), SLOT(SetJoinPassword()));

	QLabel* lbContrail = new QLabel(tr("Contrail Color"), this);
	lbContrail->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	// Contrail color hue selector.
	hueContrail = new AuHueSelector(this);
	hueContrail->setToolTip(tr("<html><b>Contrail Color Hue</b><br>"
				"Sets the color hue of the contrails and engine glow for spacecraft and vehicles of your empire."
				"</html>"));
	connect(hueContrail, SIGNAL(HueChanged(quint8)), SLOT(SetContrailHue(quint8)));

	// Contrail color saturation slider.
	satContrail = new QSlider(Qt::Horizontal, this);
	satContrail->setToolTip(tr("<html><b>Contrail Color Saturation</b><br>"
				"Sets the color saturation of the contrails and engine glow for spacecraft and vehicles of your empire."
				"</html>"));
	satContrail->setRange(64, 191);
	connect(satContrail, SIGNAL(valueChanged(int)), SLOT(SetContrailSaturation(int)));

	// Contrail sample.
	sampleContrail = new AuContrailSample(this);
	sampleContrail->setToolTip(tr("<html><b>Contrail Sample</b><br>"
				"Displays a sample of a contrail."
				"</html>"));

	QLabel* lbtheme = new QLabel(tr("Empire Theme Colors"), this);
	lbtheme->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	QLabel* lbthemeext1 = new QLabel(tr("Exterior Primary"), this);
	lbthemeext1->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	QLabel* lbthemeext2 = new QLabel(tr("Exterior Secondary"), this);
	lbthemeext2->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	QLabel* lbthemeext3 = new QLabel(tr("Exterior Tertiary"), this);
	lbthemeext3->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	QLabel* lbthemeint1 = new QLabel(tr("Interior Primary"), this);
	lbthemeint1->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	QLabel* lbthemeint2 = new QLabel(tr("Interior Secondary"), this);
	lbthemeint2->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
	QLabel* lbthemeint3 = new QLabel(tr("Interior Tertiary"), this);
	lbthemeint3->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_ThemeExteriorPrimary = new AuColorButton(this);
	connect(m_ThemeExteriorPrimary, SIGNAL(ColorChanged(QColor)), SLOT(SetThemeColorExteriorPrimary(QColor)));
	m_ThemeExteriorSecondary = new AuColorButton(this);
	connect(m_ThemeExteriorSecondary, SIGNAL(ColorChanged(QColor)), SLOT(SetThemeColorExteriorSecondary(QColor)));
	m_ThemeExteriorTertiary = new AuColorButton(this);
	connect(m_ThemeExteriorTertiary, SIGNAL(ColorChanged(QColor)), SLOT(SetThemeColorExteriorTertiary(QColor)));
	m_ThemeInteriorPrimary = new AuColorButton(this);
	connect(m_ThemeInteriorPrimary, SIGNAL(ColorChanged(QColor)), SLOT(SetThemeColorInteriorPrimary(QColor)));
	m_ThemeInteriorSecondary = new AuColorButton(this);
	connect(m_ThemeInteriorSecondary, SIGNAL(ColorChanged(QColor)), SLOT(SetThemeColorInteriorSecondary(QColor)));
	m_ThemeInteriorTertiary = new AuColorButton(this);
	connect(m_ThemeInteriorTertiary, SIGNAL(ColorChanged(QColor)), SLOT(SetThemeColorInteriorTertiary(QColor)));

	QVBoxLayout* tlblay = new QVBoxLayout;;
	tlblay->addWidget(lbtheme);
	tlblay->addStretch();

	QGridLayout* tlay = new QGridLayout;
	int row=0;
	tlay->addWidget(lbthemeext1, row,0);
	tlay->addWidget(m_ThemeExteriorPrimary, row,1);
	tlay->addWidget(lbthemeint1, row,2);
	tlay->addWidget(m_ThemeInteriorPrimary, row,3);
	row++;
	tlay->addWidget(lbthemeext2, row,0);
	tlay->addWidget(m_ThemeExteriorSecondary, row,1);
	tlay->addWidget(lbthemeint2, row,2);
	tlay->addWidget(m_ThemeInteriorSecondary, row,3);
	row++;
	tlay->addWidget(lbthemeext3, row,0);
	tlay->addWidget(m_ThemeExteriorTertiary, row,1);
	tlay->addWidget(lbthemeint3, row,2);
	tlay->addWidget(m_ThemeInteriorTertiary, row,3);

	QGridLayout* lay = new QGridLayout;
	row=0;
	lay->addWidget(m_ChangeFlag, row,4, 1,1);
	row++;
	lay->addWidget(m_JoinPass, row,4, 1,1);
	row++;
	lay->addWidget(lbContrail, row,0, 1,4);
	lay->addWidget(hueContrail, row,4, 1,3);
	lay->addWidget(sampleContrail, row,7, 2,1);
	row++;
	lay->addWidget(satContrail, row,4, 1,3);
	row++;
	lay->addLayout(tlblay, row,0, 1,4);
	lay->addLayout(tlay, row,4, 1,4);
	row+=2;
	lay->setRowStretch(row,1);	// squish everybody to the top
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

void AuGovPolicy::UpdateEmpireRecord()
{
	//qDebug() << "AuGovPolicy::UpdateEmpireRecord isVisible" << isVisible();
	// If this widget is not visible, just clear the empire id to 0.
	// setVisible calls us to really update the page when we become visible.
	if(!isVisible())
	{
		//qDebug("AuGovPolicy::UpdateEmpireRecord page is not visible. Postponing update until page becomes visible.");
		m_Empire.m_id = 0;
		return;
	}

	//qDebug("AuGovPolicy::UpdateEmpireRecord page is visible. Update contents.");
	// Get new empire record data
	AuView* pView = auApp->Frame()->View();
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
		m_Empire.m_id = person->EmpireId();
	pView->UnlockSceneAtom();

	// Acquire a copy of the empire and government records.
	const DgData& gd = g_Au.LockDataForRead();
	gd.Empire(m_Empire);
	/*
	{
		AuId avId(96846601);
		AuId cityId(95943439);
		quint32 empId = 1365;
		qDebug() << gd.EmpireHtmlName(empId) << gd.EmpireCitizenNameWithTitle(empId, avId) << gd.EmpireCityName(empId, cityId);
		qDebug() << gd.IsEmpireCityInDomain(empId, cityId, avId);
	}
	*/
	g_Au.UnlockData();

	QColor themeColors[AuScFace::THEME_Count];
	m_Empire.ThemeColors(themeColors);

	// Empire information.
	m_ChangeFlag->blockSignals(true);
	QPixmap flagpix = QPixmap::fromImage(m_Empire.m_flag);
	m_ChangeFlag->setIcon(QIcon(flagpix));
	m_ChangeFlag->setIconSize(m_Empire.m_flag.size());
	m_ChangeFlag->blockSignals(false);

	hueContrail->blockSignals(true);
	hueContrail->SetHue(m_Empire.m_ContrailHue);
	hueContrail->SetSaturation(m_Empire.m_ContrailSat);
	hueContrail->blockSignals(false);

	satContrail->blockSignals(true);
	satContrail->setValue(m_Empire.m_ContrailSat);
	satContrail->blockSignals(false);

	sampleContrail->blockSignals(true);
	sampleContrail->SetContrailColor(m_Empire.m_ContrailHue, m_Empire.m_ContrailSat);
	sampleContrail->blockSignals(false);

	m_ThemeExteriorPrimary->blockSignals(true);
	m_ThemeExteriorPrimary->SetColor(themeColors[AuScFace::THEME_ExteriorPrimary]);
	m_ThemeExteriorPrimary->blockSignals(false);

	m_ThemeExteriorSecondary->blockSignals(true);
	m_ThemeExteriorSecondary->SetColor(themeColors[AuScFace::THEME_ExteriorSecondary]);
	m_ThemeExteriorSecondary->blockSignals(false);

	m_ThemeExteriorTertiary->blockSignals(true);
	m_ThemeExteriorTertiary->SetColor(themeColors[AuScFace::THEME_ExteriorTertiary]);
	m_ThemeExteriorTertiary->blockSignals(false);

	m_ThemeInteriorPrimary->blockSignals(true);
	m_ThemeInteriorPrimary->SetColor(themeColors[AuScFace::THEME_InteriorPrimary]);
	m_ThemeInteriorPrimary->blockSignals(false);

	m_ThemeInteriorSecondary->blockSignals(true);
	m_ThemeInteriorSecondary->SetColor(themeColors[AuScFace::THEME_InteriorSecondary]);
	m_ThemeInteriorSecondary->blockSignals(false);

	m_ThemeInteriorTertiary->blockSignals(true);
	m_ThemeInteriorTertiary->SetColor(themeColors[AuScFace::THEME_InteriorTertiary]);
	m_ThemeInteriorTertiary->blockSignals(false);

	EnableControls();
}

void AuGovPolicy::EnableControls()
{
	AuView* pView = auApp->Frame()->View();
	AuId personId = pView->SceneAtomId();
	quint32 personEmpireId = pView->SceneAtomEmpire();

	const DgData& gd = g_Au.LockDataForRead();
	bool canChangeFlag = gd.EmpireCitizenCanSetEmpireFlag(personEmpireId, personId);
	bool hasJoinPass = gd.EmpireHasJoinPassword(personEmpireId);
	bool canChangeJoinPass = gd.EmpireCitizenCanSetJoinPassword(personEmpireId, personId);
	bool canSetContrail = gd.EmpireCitizenCanSetContrailColor(personEmpireId, personId);
	bool canSetColorTheme = gd.EmpireCitizenCanSetColorTheme(personEmpireId, personId);
	g_Au.UnlockData();

	m_ChangeFlag->setEnabled(canChangeFlag);
	m_JoinPass->setEnabled(canChangeJoinPass);
	if(hasJoinPass)
		m_JoinPass->setText(tr("Change Join Password"));
	else
		m_JoinPass->setText(tr("Set Join Password"));

	hueContrail->setEnabled(canSetContrail);
	satContrail->setEnabled(canSetContrail);

	m_ThemeExteriorPrimary->setEnabled(canSetColorTheme);
	m_ThemeExteriorSecondary->setEnabled(canSetColorTheme);
	m_ThemeExteriorTertiary->setEnabled(canSetColorTheme);
	m_ThemeInteriorPrimary->setEnabled(canSetColorTheme);
	m_ThemeInteriorSecondary->setEnabled(canSetColorTheme);
	m_ThemeInteriorTertiary->setEnabled(canSetColorTheme);
}

void AuGovPolicy::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	UpdateEmpireRecord();
}

void AuGovPolicy::SetContrailHue(quint8 hue)
{
	quint8 sat = satContrail->value();

	sampleContrail->SetContrailColor(hue, sat);

	auApp->GovSetContrailColor(hue, sat);
}

void AuGovPolicy::SetContrailSaturation(int sat)
{
	quint8 hue = hueContrail->Hue();

	hueContrail->SetSaturation(sat);
	sampleContrail->SetContrailColor(hue, sat);

	auApp->GovSetContrailColor(hue, sat);
}

void AuGovPolicy::ChangeFlag()
{
	AuView* pView = auApp->Frame()->View();
	quint32 personEmpireId = pView->SceneAtomEmpire();
	AuId personId = pView->SceneAtomId();

	const DgData& gd = g_Au.LockDataForRead();
	bool canChangeFlag = gd.EmpireCitizenCanSetEmpireFlag(personEmpireId, personId);
	g_Au.UnlockData();

	if(canChangeFlag)
	{
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
				QImage nonScaledImg = imFlag;
				if(imFlag.width() > DgEmpire::FlagImageMax() || imFlag.height() > DgEmpire::FlagImageMax())
				{
					// Resize the image.
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
					auApp->EmpireChangeFlag(imFlag);
					QMessageBox::information(this, tr("Change Empire Flag"), tr("Empire flag change request sent to server."));
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
}

void AuGovPolicy::SetThemeColorExteriorPrimary(QColor color)
{
	auApp->GovSetThemeColor(AuScFace::THEME_ExteriorPrimary, color);
}

void AuGovPolicy::SetThemeColorExteriorSecondary(QColor color)
{
	auApp->GovSetThemeColor(AuScFace::THEME_ExteriorSecondary, color);
}

void AuGovPolicy::SetThemeColorExteriorTertiary(QColor color)
{
	auApp->GovSetThemeColor(AuScFace::THEME_ExteriorTertiary, color);
}

void AuGovPolicy::SetThemeColorInteriorPrimary(QColor color)
{
	auApp->GovSetThemeColor(AuScFace::THEME_InteriorPrimary, color);
}

void AuGovPolicy::SetThemeColorInteriorSecondary(QColor color)
{
	auApp->GovSetThemeColor(AuScFace::THEME_InteriorSecondary, color);
}

void AuGovPolicy::SetThemeColorInteriorTertiary(QColor color)
{
	auApp->GovSetThemeColor(AuScFace::THEME_InteriorTertiary, color);
}

void AuGovPolicy::SetJoinPassword()
{
	QString password = QInputDialog::getText(this, tr("Join Password"),
			tr("Enter a password to let new avatars join this empire. Blank password disables joining."),
			QLineEdit::Password);
	auApp->GovSetJoinPassword(password);

}

