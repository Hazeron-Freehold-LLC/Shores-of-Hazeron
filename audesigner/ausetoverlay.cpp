//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausetoverlay.cpp,v 1.10 2022/02/19 18:26:07 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausetoverlay.h"
//
#include "auapp.h"
#include "aucheckbox.h"
#include "auclientglobal.h"
#include "audesignerview.h"
#include "aujoystick.h"
#include "auframe.h"
#include "daperson.h"
#include "davehicle.h"
#include "dgdata.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QFontDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMenu>
#include <QScrollArea>
#include <QSlider>

AuViewButtonStyleButton::AuViewButtonStyleButton(const QImage& image, QWidget* parent/*=0*/) : QPushButton(parent)
{
	m_Image = image;

	setMaximumSize(QSize(40,40));
	setMinimumSize(QSize(40,40));
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void AuViewButtonStyleButton::paintEvent(QPaintEvent* event)
{
	QPushButton::paintEvent(event);

	const int margin = 4;
	QRect rc = rect().adjusted(margin,margin,-margin,-margin);

	QPainter p(this);
	p.drawImage(rc, m_Image);
	p.end();
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSetOverlay::AuSetOverlay(QWidget *parent) : QWidget(parent)
{
	ckdrawminime = new QCheckBox(tr("Draw Mini Body Model"));
	ckdrawminime->setCheckState(g_Au.DrawMiniMe()?Qt::Checked:Qt::Unchecked);
	ckdrawminime->setToolTip(tr("<html>When checked, a miniature 3D model of your body is drawn at the bottom of the scene.<br>"
				"When unchecked, a miniature 3D model of your body is not drawn in the scene.</html>"));
	connect(ckdrawminime, SIGNAL(stateChanged(int)), SLOT(SetDrawMiniMe(int)));

	ckshowwindowsize = new QCheckBox(tr("Show Scene Window Size When It Changes"));
	ckshowwindowsize->setCheckState(g_AuClient.ShowWindowSize()?Qt::Checked:Qt::Unchecked);
	ckshowwindowsize->setToolTip(tr("<html>When checked, the size of the scene window is shown briefly when it changes.<br>"
				"When unchecked, size of the scene window is not displayed.</html>"));
	connect(ckshowwindowsize, SIGNAL(stateChanged(int)), SLOT(SetShowWindowSize(int)));

	m_ShowThreads = new QCheckBox(tr("Show Task Queue"));
	m_ShowThreads->setCheckState(g_AuClient.ShowThreads()?Qt::Checked:Qt::Unchecked);
	m_ShowThreads->setToolTip(tr("<html>When checked, an indicator of tasks queued for background thread processing "
				"appears at the top right corner of the scene, with a count of waiting tasks. "
				"The task queue indicator is only shown while tasks are awaiting completion. "
				"<br>"
				"When unchecked, the task queue indicator is not shown.</html>"));
	connect(m_ShowThreads, SIGNAL(stateChanged(int)), SLOT(SetShowThreads(int)));

	m_ShowNet = new QCheckBox(tr("Show Network Activity"));
	m_ShowNet->setCheckState(g_AuClient.ShowNet()?Qt::Checked:Qt::Unchecked);
	m_ShowNet->setToolTip(tr("<html>When checked, an indicator of network activity appears at the top right corner of the scene. "
				"The network activity indicator shows recent network activity within <i>%1</i>. "
				"Blue particles signify outgoing data. "
				"Red particles signify incoming data. "
				"<br>"
				"When unchecked, the network activity indicator is not shown.</html>").arg(g_Au.ApplicationName()));
	connect(m_ShowNet, SIGNAL(stateChanged(int)), SLOT(SetShowNet(int)));

	QComboBox* cblrcursor = new QComboBox();
	cblrcursor->addItem(tr("Cursor Matches Avatar Handedness"), 0);
	cblrcursor->addItem(tr("Left Hand Cursor"), 1);
	cblrcursor->addItem(tr("Right Hand Cursor"), 2);
	cblrcursor->setCurrentIndex(cblrcursor->findData(g_Au.LeftRightCursor()));
	connect(cblrcursor, SIGNAL(activated(int)), SLOT(LeftRightCursorChanged(int)));

	QComboBox* cblrorient = new QComboBox();
	cblrorient->addItem(tr("Arrangement of Screen Elements Matches Avatar Handedness"), 0);
	cblrorient->addItem(tr("Left Hand Arrangement of Screen Elements"), 1);
	cblrorient->addItem(tr("Right Hand Arrangement of Screen Elements"), 2);
	cblrorient->setCurrentIndex(cblrorient->findData(g_Au.LeftRightOrientation()));
	connect(cblrorient, SIGNAL(activated(int)), SLOT(LeftRightOrientationChanged(int)));

	QLabel* lbbodystatsize = new QLabel(tr("Health Bar Size"));

	m_HealthBarSize = new QLabel(QString("%1%").arg(g_Au.BodyStatSize()));

	bodystatsize = new QSlider(Qt::Horizontal, this);
	bodystatsize->setRange(1, 200);	// Percent of size
	bodystatsize->setSingleStep(1);
	bodystatsize->setPageStep(5);
	bodystatsize->setValue(g_Au.BodyStatSize());
	bodystatsize->setToolTip(tr("<html>Adjust the size of the health status bars shown for your body, mount and vehicle.</html>"));
	connect(bodystatsize, SIGNAL(valueChanged(int)), SLOT(SetBodyStatSize(int)));

	QLabel* lbresourcesize = new QLabel(tr("Resource Icon Size"));

	m_ResourceIconSize = new QLabel(QString("%1%").arg(g_Au.ResourceIconSize()));

	slresourcesize = new QSlider(Qt::Horizontal, this);
	slresourcesize->setRange(10, 200);	// Percent of size
	slresourcesize->setSingleStep(1);
	slresourcesize->setPageStep(5);
	slresourcesize->setValue(g_Au.ResourceIconSize());
	slresourcesize->setToolTip(tr("<html>Adjust the size of the resource icons overlayed on the terrain in top down view.</html>"));
	connect(slresourcesize, SIGNAL(valueChanged(int)), SLOT(SetResourceIconSize(int)));

	// Atom font
	m_FontAtomButton = new QPushButton(QIcon(":data/txTargetLockOnBehind.png"), tr("Label Font"), this);
	m_FontAtomButton->setFont(g_AuClient.FontAtom());
	m_FontAtomButton->setToolTip(tr("<html>Font for rendering labels on miscellaneous game objects.<br>"
				"%1 Decal text in designs.<br>"
				"%1 Fire control station reticle information.<br>"
				"%1 Unit names when they are selected.<br>"
				"%1 Room labels in design studio.<br>"
				"%1 Transporter pad sequence labels in design studio.<br>"
				"<b>Program restart required.</b></html>").arg(g_Au.MiddleDotSymbol()));
	connect(m_FontAtomButton, SIGNAL(clicked()), SLOT(FontAtomClicked()));

	QGridLayout* sliderlay = new QGridLayout;
	{
		int row=0;
		sliderlay->addWidget(lbbodystatsize, row,0);
		sliderlay->addWidget(bodystatsize, row,1);
		sliderlay->addWidget(m_HealthBarSize, row,2);
		row++;
		sliderlay->addWidget(lbresourcesize, row,0);
		sliderlay->addWidget(slresourcesize, row,1);
		sliderlay->addWidget(m_ResourceIconSize, row,2);
		row++;
	}

	// On screen view buttons.
	QGroupBox* gbViewButtons = new QGroupBox(tr("On Screen Buttons"));
	{
		QLabel* lbbuttonsize = new QLabel(tr("Size"));

		m_ButtonStyle = new AuViewButtonStyleButton(QImage(g_AuClient.ViewButtonImageName()).scaled(64,64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
		m_ButtonStyle->setMinimumSize(40,40);
		connect(m_ButtonStyle, SIGNAL(clicked()), SLOT(ButtonStyleClicked()));

		if(g_AuClient.ViewButtonSize())
			m_ButtonSize = new QLabel(tr("%1 pixels").arg(g_AuClient.ViewButtonSize()));
		else
			m_ButtonSize = new QLabel(tr("Auto"));

		slbuttonsize = new QSlider(Qt::Horizontal, this);
		slbuttonsize->setRange(0, 100);	// Pixels
		slbuttonsize->setSingleStep(1);
		slbuttonsize->setPageStep(5);
		slbuttonsize->setValue(g_AuClient.ViewButtonSize());
		slbuttonsize->setToolTip(tr("<html>Adjust the size of the buttons overlayed on the main window. "
					"When set to 0, the button size is determined automatically.</html>"));
		connect(slbuttonsize, SIGNAL(valueChanged(int)), SLOT(SetButtonSize(int)));

		QLabel* lbbuttontrans = new QLabel(tr("Transparency"));

		if(g_AuClient.ViewButtonTransparency() < 100)
			m_ButtonTrans = new QLabel(tr("%1%").arg(g_AuClient.ViewButtonTransparency()));
		else
			m_ButtonTrans = new QLabel(tr("Off"));

		slbuttontrans = new QSlider(Qt::Horizontal, this);
		slbuttontrans->setRange(0, 100);	// Percent of size
		slbuttontrans->setSingleStep(1);
		slbuttontrans->setPageStep(5);
		slbuttontrans->setValue(g_AuClient.ViewButtonTransparency());
		slbuttontrans->setToolTip(tr("<html>Adjust the transparency of the buttons overlayed on the main window. "
					"When set to 0, the buttons are not drawn at all and they cannot be clicked.</html>"));
		connect(slbuttontrans, SIGNAL(valueChanged(int)), SLOT(SetButtonTransparency(int)));

		m_ActionButtons = new QCheckBox(tr("Action Buttons"));
		m_ActionButtons->setCheckState(g_AuClient.ViewActionButtons()?Qt::Checked:Qt::Unchecked);
		m_ActionButtons->setToolTip(tr("<html>Action buttons do things with your equipment, "
					"the targeted object, or the spacecraft, vehicle or building you occupy. "
					"Buttons are placed at the center right of the scene, appearing as a black box with an icon and text. "
					"Action buttons are not visible when in top down view. "
					"<br>"
					"Most actions can be accessed using key bindings or by clicking on the targeted object, "
					"regardless of this setting. "
					"<br>"
					"When checked, action buttons are shown. "
					"<br>"
					"When unchecked, action buttons are not shown.</html>"));
		connect(m_ActionButtons, SIGNAL(stateChanged(int)), SLOT(SetShowActionButtons(int)));

		QGridLayout* vbSliderlay = new QGridLayout;
		{
			int row=0;
			vbSliderlay->addWidget(m_ButtonStyle, row,0, 2,1);
			vbSliderlay->addWidget(lbbuttonsize, row,1);
			vbSliderlay->addWidget(slbuttonsize, row,2);
			vbSliderlay->addWidget(m_ButtonSize, row,3);
			row++;
			vbSliderlay->addWidget(lbbuttontrans, row,1);
			vbSliderlay->addWidget(slbuttontrans, row,2);
			vbSliderlay->addWidget(m_ButtonTrans, row,3);
			row++;
			vbSliderlay->addWidget(m_ActionButtons, row,2, 1,2);
			row++;
		}

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
		gblay->addLayout(vbSliderlay);
		gblay->addWidget(vbScroll);
		gblay->setSpacing(2);
		gblay->setMargin(12);
		gbViewButtons->setLayout(gblay);
	}

	QGridLayout* lay = new QGridLayout;
	int row=0;
	lay->addWidget(ckdrawminime, row,0, 1,3);
	row++;
	lay->addWidget(ckshowwindowsize, row,0, 1,3);
	row++;
	lay->addWidget(m_ShowNet, row,0, 1,3);
	row++;
	lay->addWidget(m_ShowThreads, row,0, 1,3);
	row++;
	lay->addLayout(sliderlay, row,0, 1,3);
	row++;
	lay->addWidget(cblrcursor, row,0, 1,3);
	row++;
	lay->addWidget(cblrorient, row,0, 1,3);
	row++;
	lay->addWidget(m_FontAtomButton, row,0, 1,3);
	row++;
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

void AuSetOverlay::SetShowActionButtons(int value)
{
	g_AuClient.SetViewActionButtons(value ? true : false);
}

void AuSetOverlay::SetShowWindowSize(int value)
{
	g_AuClient.SetShowWindowSize(value ? true : false);
}

void AuSetOverlay::SetDrawMiniMe(int value)
{
	g_Au.SetDrawMiniMe(value ? true : false);
}

void AuSetOverlay::SetBodyStatSize(int value)
{
	g_Au.SetBodyStatSize(value);
	m_HealthBarSize->setText(tr("%1%").arg(value));
}

void AuSetOverlay::SetResourceIconSize(int value)
{
	g_Au.SetResourceIconSize(value);
	m_ResourceIconSize->setText(tr("%1%").arg(value));
}

void AuSetOverlay::SetButtonSize(int value)
{
	g_AuClient.SetViewButtonSize(value);
	if(value)
		m_ButtonSize->setText(tr("%1 pixels").arg(value));
	else
		m_ButtonSize->setText(tr("Auto"));
}

void AuSetOverlay::SetButtonTransparency(int value)
{
	g_AuClient.SetViewButtonTransparency(value);
	if(value < 100)
		m_ButtonTrans->setText(tr("%1%").arg(value));
	else
		m_ButtonTrans->setText(tr("Off"));
}

void AuSetOverlay::ButtonStyleClicked()
{
	QMenu menu(this);

	for(int vbs=1; vbs<AuClientGlobal::ViewButtonStyleCount(); vbs++)
	{
		QAction* a = menu.addAction(QIcon(QPixmap(AuClientGlobal::ViewButtonImageName(vbs)).scaled(64,64, Qt::KeepAspectRatio, Qt::SmoothTransformation)),
				AuClientGlobal::ViewButtonStyleName(vbs));
		a->setData(vbs);
	}

	// Custom option is last.
	QAction* a = menu.addAction(QIcon(QPixmap(AuClientGlobal::ViewButtonImageName(0)).scaled(64,64, Qt::KeepAspectRatio, Qt::SmoothTransformation)),
			AuClientGlobal::ViewButtonStyleName(0));
	a->setData(0);

	QAction* action = menu.exec(QCursor::pos());
	if(!action)
		return;

	int vbs = action->data().toInt();
	if(vbs == 0)
	{
		// Custom image. Request the file name.
		QString lastFile = g_Au.Settings().readEntry("/Settings/ViewButtonStyleImage", "");
		if(lastFile.isEmpty())
			lastFile = QDir::currentPath();
		QFileInfo lastFileInfo(lastFile);
		if(!lastFileInfo.exists())
			lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.
		QString file = QFileDialog::getOpenFileName(this, tr("Select Button Image"), lastFile, "", 0,
				g_Au.UseNativeFileDialog() ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog);
		g_AuClient.SetViewButtonStyleImage(file);
	}
	else
		g_AuClient.SetViewButtonStyle(vbs);

	m_ButtonStyle->SetImage(QImage(g_AuClient.ViewButtonImageName()).scaled(64,64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void AuSetOverlay::FontAtomClicked()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, g_AuClient.FontAtom(), this, tr("Select Label Font"),
			g_Au.UseNativeFontDialog() ? QFontDialog::FontDialogOptions() : QFontDialog::DontUseNativeDialog);
	if(ok)
	{
		g_AuClient.SetFontAtom(font);
		m_FontAtomButton->setFont(font);
	}
}

void AuSetOverlay::LeftRightCursorChanged(int value)
{
	g_Au.SetLeftRightCursor(value);
}

void AuSetOverlay::LeftRightOrientationChanged(int value)
{
	g_Au.SetLeftRightOrientation(value);
}

void AuSetOverlay::ViewButtonCheckChanged(int checkState, int vb)
{
	if(vb <= AuViewButton::VB_Invalid || vb >= AuViewButton::VB_COUNT)
		return;

	g_AuClient.SetViewButtonHidden(vb, checkState != Qt::Checked);
}

void AuSetOverlay::SetShowNet(int value)
{
	g_AuClient.SetShowNet(value ? true : false);
}

void AuSetOverlay::SetShowThreads(int value)
{
	g_AuClient.SetShowThreads(value ? true : false);
}
