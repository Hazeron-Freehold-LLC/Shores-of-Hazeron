//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausetdesigner.cpp,v 1.8 2022/02/19 18:26:06 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ausetdesigner.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "aujoystick.h"
#include "auframe.h"
#include "auview.h"
#include <daperson.h>
#include <davehicle.h>
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QFontDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMenu>
#include <QScrollArea>
#include <QSlider>

AuSetDesigner::AuSetDesigner(QWidget *parent) : QWidget(parent)
{
	QLabel* lbvertsize = new QLabel(tr("Vertex Size"));

	m_VertSizeLabel = new QLabel(QString("%1 pixels").arg(g_Au.DesignerVertexSize()));

	QSlider* slvertsize = new QSlider(Qt::Horizontal, this);
	slvertsize->setRange(6, 60);	// Pixels in size
	slvertsize->setSingleStep(1);
	slvertsize->setPageStep(6);
	slvertsize->setValue(g_Au.DesignerVertexSize());
	slvertsize->setToolTip(tr("<html>Adjusts the size of the dots that appear on selected vertices, "
				"when working in face mode.</html>"));
	connect(slvertsize, SIGNAL(valueChanged(int)), SLOT(SetVertexSize(int)));

	m_FontButton = new QPushButton(tr("Label Font"), this);
	m_FontButton->setFont(g_AuClient.FontDesigner());
	m_FontButton->setToolTip(tr("<html>Font for drawing labels on miscellaneous designer objects.<br>"
				"%1 Cursor text.<br>"
				"%1 Grid Markers.</html>")
			.arg(g_Au.MiddleDotSymbol()));
	connect(m_FontButton, SIGNAL(clicked()), SLOT(FontClicked()));

	QGroupBox* gbColors = new QGroupBox(tr("Colors"));
	{
		m_GridColorButton = new QPushButton(tr("Grid Lines"), this);
		m_GridColorButton->setToolTip(tr("<html>Sets the color for drawing the design grid lines. "
					"<br>"
					"The color selector is configured with two custom colors. "
					"The first is the default color. "
					"The second is the current color.</html>"));
		m_GridColorButton->setStyleSheet(QString("background-color: rgb(0, 0, 0); color: rgb(%1, %2, %3)")
				.arg(g_Au.DesignerGridColor().red()).arg(g_Au.DesignerGridColor().green()).arg(g_Au.DesignerGridColor().blue()));
		connect(m_GridColorButton, SIGNAL(clicked()), SLOT(GridColorClicked()));

		m_GridXColorButton = new QPushButton(tr("X"), this);
		m_GridXColorButton->setToolTip(tr("<html>Sets the color for drawing the design grid X axis and X plane. "
					"<br>"
					"The color selector is configured with two custom colors. "
					"The first is the default color. "
					"The second is the current color.</html>"));
		m_GridXColorButton->setStyleSheet(QString("background-color: rgb(0, 0, 0); color: rgb(%1, %2, %3)")
				.arg(g_Au.DesignerGridXColor().red()).arg(g_Au.DesignerGridXColor().green()).arg(g_Au.DesignerGridXColor().blue()));
		connect(m_GridXColorButton, SIGNAL(clicked()), SLOT(GridXColorClicked()));

		m_GridYColorButton = new QPushButton(tr("Y"), this);
		m_GridYColorButton->setToolTip(tr("<html>Sets the color for drawing the design grid Y axis and Y plane. "
					"<br>"
					"The color selector is configured with two custom colors. "
					"The first is the default color. "
					"The second is the current color.</html>"));
		m_GridYColorButton->setStyleSheet(QString("background-color: rgb(0, 0, 0); color: rgb(%1, %2, %3)")
				.arg(g_Au.DesignerGridYColor().red()).arg(g_Au.DesignerGridYColor().green()).arg(g_Au.DesignerGridYColor().blue()));
		connect(m_GridYColorButton, SIGNAL(clicked()), SLOT(GridYColorClicked()));

		m_GridZColorButton = new QPushButton(tr("Z"), this);
		m_GridZColorButton->setToolTip(tr("<html>Sets the color for drawing the design grid Z axis and Z plane. "
					"<br>"
					"The color selector is configured with two custom colors. "
					"The first is the default color. "
					"The second is the current color.</html>"));
		m_GridZColorButton->setStyleSheet(QString("background-color: rgb(0, 0, 0); color: rgb(%1, %2, %3)")
				.arg(g_Au.DesignerGridZColor().red()).arg(g_Au.DesignerGridZColor().green()).arg(g_Au.DesignerGridZColor().blue()));
		connect(m_GridZColorButton, SIGNAL(clicked()), SLOT(GridZColorClicked()));

		m_LineColorButton = new QPushButton(tr("Edge Lines"), this);
		m_LineColorButton->setToolTip(tr("<html>Sets the color for drawing edge lines and sprites on objects that are not selected. "
					"Sprites are visible on vertices and faces when working in face mode. "
					"Edges lines are not visible when the model is seen in the game. "
					"<br>"
					"The color selector is configured with two custom colors. "
					"The first is the default color. "
					"The second is the current color.</html>"));
		m_LineColorButton->setStyleSheet(QString("background-color: rgb(0, 0, 0); color: rgb(%1, %2, %3)")
				.arg(g_Au.DesignerLineColor().red()).arg(g_Au.DesignerLineColor().green()).arg(g_Au.DesignerLineColor().blue()));
		connect(m_LineColorButton, SIGNAL(clicked()), SLOT(LineColorClicked()));

		m_BackColorButton = new QPushButton(tr("Back Faces"), this);
		m_BackColorButton->setToolTip(tr("<html>Sets the color for highlighting the back side of faces. "
					"Back faces are not visible when the model is seen in the game. "
					"<br>"
					"The color selector is configured with two custom colors. "
					"The first is the default color. "
					"The second is the current color.</html>"));
		m_BackColorButton->setStyleSheet(QString("background-color: rgb(0, 0, 0); color: rgb(%1, %2, %3)")
				.arg(g_Au.DesignerBackColor().red()).arg(g_Au.DesignerBackColor().green()).arg(g_Au.DesignerBackColor().blue()));
		connect(m_BackColorButton, SIGNAL(clicked()), SLOT(BackColorClicked()));

		m_SelColorButton = new QPushButton(tr("Selection"), this);
		m_SelColorButton->setToolTip(tr("<html>Sets the color for highlighting objects that are selected. "
					"<br>"
					"The color selector is configured with two custom colors. "
					"The first is the default color. "
					"The second is the current color.</html>"));
		m_SelColorButton->setStyleSheet(QString("background-color: rgb(0, 0, 0); color: rgb(%1, %2, %3)")
				.arg(g_Au.DesignerSelColor().red()).arg(g_Au.DesignerSelColor().green()).arg(g_Au.DesignerSelColor().blue()));
		connect(m_SelColorButton, SIGNAL(clicked()), SLOT(SelColorClicked()));

		m_WalkColorButton = new QPushButton(tr("Walk Paths"), this);
		m_WalkColorButton->setToolTip(tr("<html>Sets the color for showing walk paths. "
					"<br>"
					"The color selector is configured with two custom colors. "
					"The first is the default color. "
					"The second is the current color.</html>"));
		m_WalkColorButton->setStyleSheet(QString("background-color: rgb(0, 0, 0); color: rgb(%1, %2, %3)")
				.arg(g_Au.DesignerWalkPathColor().red())
				.arg(g_Au.DesignerWalkPathColor().green())
				.arg(g_Au.DesignerWalkPathColor().blue()));
		connect(m_WalkColorButton, SIGNAL(clicked()), SLOT(WalkColorClicked()));

		m_LadderColorButton = new QPushButton(tr("Ladder Paths"), this);
		m_LadderColorButton->setToolTip(tr("<html>Sets the color for showing ladder paths. "
					"<br>"
					"The color selector is configured with two custom colors. "
					"The first is the default color. "
					"The second is the current color.</html>"));
		m_LadderColorButton->setStyleSheet(QString("background-color: rgb(0, 0, 0); color: rgb(%1, %2, %3)")
				.arg(g_Au.DesignerLadderPathColor().red())
				.arg(g_Au.DesignerLadderPathColor().green())
				.arg(g_Au.DesignerLadderPathColor().blue()));
		connect(m_LadderColorButton, SIGNAL(clicked()), SLOT(LadderColorClicked()));

		QVBoxLayout* clay = new QVBoxLayout;
		clay->addWidget(m_LineColorButton);
		clay->addWidget(m_BackColorButton);
		clay->addWidget(m_SelColorButton);
		clay->addWidget(m_GridColorButton);
		clay->addWidget(m_GridXColorButton);
		clay->addWidget(m_GridYColorButton);
		clay->addWidget(m_GridZColorButton);
		clay->addWidget(m_WalkColorButton);
		clay->addWidget(m_LadderColorButton);
		clay->setMargin(12);
		clay->setSpacing(2);

		gbColors->setLayout(clay);
	}

	UNIT_OF_MEASURE uom = g_AuClient.DesignerUnitOfMeasure();

	QLabel* lbuom = new QLabel(tr("Unit of Measure"), this);
	lbuom->setAlignment(Qt::AlignRight|Qt::AlignVCenter);

	m_UnitOfMeasure = new QComboBox(this);
	m_UnitOfMeasure->addItem(tr("Meters"), UOM_Meters);
	m_UnitOfMeasure->addItem(tr("Centimeters"), UOM_Centimeters);
	m_UnitOfMeasure->addItem(tr("Millimeters"), UOM_Millimeters);
	m_UnitOfMeasure->addItem(tr("Yards"), UOM_Yards);
	m_UnitOfMeasure->addItem(tr("Feet"), UOM_Feet);
	m_UnitOfMeasure->addItem(tr("Inches"), UOM_Inches);
	m_UnitOfMeasure->setCurrentIndex(m_UnitOfMeasure->findData(uom));
	m_UnitOfMeasure->setToolTip(tr("<html>"
				"<p>"
				"Unit of measure determines how numbers are interpreted, when coordinates or lengths are entered by the user. "
				"Unit of measure also determines how those numbers are formatted for display. "
				"</p>"

				"<p>"
				"Unit of measure does not affect the way numbers are stored internally. "
				"Designs drawn in feet are fully compatible with designs drawn in meters. "
				"</p>"

				"<p>"
				"Unit of measure is only used in the design environment. "
				"It is not applied elsewhere in the game. "
				"</p>"
				"</html>"));
	connect(m_UnitOfMeasure, SIGNAL(currentIndexChanged(int)), SLOT(UnitOfMeasureChanged(int)));

	QGridLayout* sliderlay = new QGridLayout;
	{
		int row=0;
		sliderlay->addWidget(lbvertsize, row,0);
		sliderlay->addWidget(slvertsize, row,1);
		sliderlay->addWidget(m_VertSizeLabel, row,2);
		row++;
	}
	sliderlay->setMargin(0);
	sliderlay->setSpacing(2);

	QHBoxLayout* ulay = new QHBoxLayout;
	ulay->addWidget(lbuom);
	ulay->addWidget(m_UnitOfMeasure);
	ulay->setMargin(0);
	ulay->setSpacing(2);

	QVBoxLayout* blay = new QVBoxLayout;
	blay->addLayout(sliderlay);
	blay->addLayout(ulay);
	blay->addWidget(m_FontButton);
	blay->addStretch();
	blay->setMargin(0);
	blay->setSpacing(2);

	QHBoxLayout* flay = new QHBoxLayout;
	flay->addLayout(blay);
	flay->addWidget(gbColors);
	flay->setMargin(0);
	flay->setSpacing(2);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addLayout(flay);
	lay->addStretch();
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuSetDesigner::~AuSetDesigner()
{
}

void AuSetDesigner::SetVertexSize(int value)
{
	g_Au.SetDesignerVertexSize(value);
	m_VertSizeLabel->setText(tr("%1 pixels").arg(value));
}

void AuSetDesigner::FontClicked()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, g_AuClient.FontDesigner(), this, tr("Select Label Font"),
			g_Au.UseNativeFontDialog() ? QFontDialog::FontDialogOptions() : QFontDialog::DontUseNativeDialog);
	if(ok)
	{
		g_AuClient.SetFontDesigner(font);
		m_FontButton->setFont(font);
	}
}

void AuSetDesigner::GridColorClicked()
{
	QColor color = g_Au.DesignerDefaultGridColor();
	QColorDialog::setCustomColor(0, color.rgb());				// First custom color is the default scope color.
	color = g_Au.DesignerGridColor();
	QColorDialog::setCustomColor(1, color.rgb());				// Second custom color is the current scope color.
	color = QColorDialog::getColor(color, this, tr("Designer X Grid Color"),
			g_Au.UseNativeColorDialog() ? QColorDialog::ColorDialogOptions() : QColorDialog::DontUseNativeDialog);
	if(color.isValid())
	{
		g_Au.SetDesignerGridColor(color);
		m_GridColorButton->setStyleSheet(QString("background-color: rgb(0, 0, 0); color: rgb(%1, %2, %3)")
				.arg(color.red()).arg(color.green()).arg(color.blue()));
	}
}

void AuSetDesigner::GridXColorClicked()
{
	QColor color = g_Au.DesignerDefaultGridXColor();
	QColorDialog::setCustomColor(0, color.rgb());				// First custom color is the default scope color.
	color = g_Au.DesignerGridXColor();
	QColorDialog::setCustomColor(1, color.rgb());				// Second custom color is the current scope color.
	color = QColorDialog::getColor(color, this, tr("Designer X Grid Color"),
			g_Au.UseNativeColorDialog() ? QColorDialog::ColorDialogOptions() : QColorDialog::DontUseNativeDialog);
	if(color.isValid())
	{
		g_Au.SetDesignerGridXColor(color);
		m_GridXColorButton->setStyleSheet(QString("background-color: rgb(0, 0, 0); color: rgb(%1, %2, %3)")
				.arg(color.red()).arg(color.green()).arg(color.blue()));
	}
}

void AuSetDesigner::GridYColorClicked()
{
	QColor color = g_Au.DesignerDefaultGridYColor();
	QColorDialog::setCustomColor(0, color.rgb());				// First custom color is the default scope color.
	color = g_Au.DesignerGridYColor();
	QColorDialog::setCustomColor(1, color.rgb());				// Second custom color is the current scope color.
	color = QColorDialog::getColor(color, this, tr("Designer Y Grid Color"),
			g_Au.UseNativeColorDialog() ? QColorDialog::ColorDialogOptions() : QColorDialog::DontUseNativeDialog);
	if(color.isValid())
	{
		g_Au.SetDesignerGridYColor(color);
		m_GridYColorButton->setStyleSheet(QString("background-color: rgb(0, 0, 0); color: rgb(%1, %2, %3)")
				.arg(color.red()).arg(color.green()).arg(color.blue()));
	}
}

void AuSetDesigner::GridZColorClicked()
{
	QColor color = g_Au.DesignerDefaultGridZColor();
	QColorDialog::setCustomColor(0, color.rgb());				// First custom color is the default scope color.
	color = g_Au.DesignerGridZColor();
	QColorDialog::setCustomColor(1, color.rgb());				// Second custom color is the current scope color.
	color = QColorDialog::getColor(color, this, tr("Designer Z Grid Color"),
			g_Au.UseNativeColorDialog() ? QColorDialog::ColorDialogOptions() : QColorDialog::DontUseNativeDialog);
	if(color.isValid())
	{
		g_Au.SetDesignerGridZColor(color);
		m_GridZColorButton->setStyleSheet(QString("background-color: rgb(0, 0, 0); color: rgb(%1, %2, %3)")
				.arg(color.red()).arg(color.green()).arg(color.blue()));
	}
}

void AuSetDesigner::BackColorClicked()
{
	QColor color = g_Au.DesignerDefaultBackColor();
	QColorDialog::setCustomColor(0, color.rgb());				// First custom color is the default scope color.
	color = g_Au.DesignerBackColor();
	QColorDialog::setCustomColor(1, color.rgb());				// Second custom color is the current scope color.
	color = QColorDialog::getColor(color, this, tr("Designer Back Face Color"),
			g_Au.UseNativeColorDialog() ? QColorDialog::ColorDialogOptions() : QColorDialog::DontUseNativeDialog);
	if(color.isValid())
	{
		g_Au.SetDesignerBackColor(color);
		m_BackColorButton->setStyleSheet(QString("background-color: rgb(0, 0, 0); color: rgb(%1, %2, %3)")
				.arg(color.red()).arg(color.green()).arg(color.blue()));
	}
}

void AuSetDesigner::LineColorClicked()
{
	QColor color = g_Au.DesignerDefaultLineColor();
	QColorDialog::setCustomColor(0, color.rgb());				// First custom color is the default scope color.
	color = g_Au.DesignerLineColor();
	QColorDialog::setCustomColor(1, color.rgb());				// Second custom color is the current scope color.
	color = QColorDialog::getColor(color, this, tr("Designer Line Color"),
			g_Au.UseNativeColorDialog() ? QColorDialog::ColorDialogOptions() : QColorDialog::DontUseNativeDialog);
	if(color.isValid())
	{
		g_Au.SetDesignerLineColor(color);
		m_LineColorButton->setStyleSheet(QString("background-color: rgb(0, 0, 0); color: rgb(%1, %2, %3)")
				.arg(color.red()).arg(color.green()).arg(color.blue()));
	}
}

void AuSetDesigner::SelColorClicked()
{
	QColor color = g_Au.DesignerDefaultSelColor();
	QColorDialog::setCustomColor(0, color.rgb());				// First custom color is the default scope color.
	color = g_Au.DesignerSelColor();
	QColorDialog::setCustomColor(1, color.rgb());				// Second custom color is the current scope color.
	color = QColorDialog::getColor(color, this, tr("Designer Selection Color"),
			g_Au.UseNativeColorDialog() ? QColorDialog::ColorDialogOptions() : QColorDialog::DontUseNativeDialog);
	if(color.isValid())
	{
		g_Au.SetDesignerSelColor(color);
		m_SelColorButton->setStyleSheet(QString("background-color: rgb(0, 0, 0); color: rgb(%1, %2, %3)")
				.arg(color.red()).arg(color.green()).arg(color.blue()));
	}
}

void AuSetDesigner::UnitOfMeasureChanged(int index)
{
	UNIT_OF_MEASURE uom = (UNIT_OF_MEASURE)m_UnitOfMeasure->itemData(index).toInt();
	g_AuClient.DesignerSetUnitOfMeasure(uom);
}

void AuSetDesigner::LadderColorClicked()
{
	QColor color = g_Au.DesignerDefaultLadderPathColor();
	QColorDialog::setCustomColor(0, color.rgb());				// First custom color is the default scope color.
	color = g_Au.DesignerLadderPathColor();
	QColorDialog::setCustomColor(1, color.rgb());				// Second custom color is the current scope color.
	color = QColorDialog::getColor(color, this, tr("Designer Ladder Path Color"),
			g_Au.UseNativeColorDialog() ? QColorDialog::ColorDialogOptions() : QColorDialog::DontUseNativeDialog);
	if(color.isValid())
	{
		g_Au.SetDesignerLadderPathColor(color);
		m_LadderColorButton->setStyleSheet(QString("background-color: rgb(0, 0, 0); color: rgb(%1, %2, %3)")
				.arg(color.red()).arg(color.green()).arg(color.blue()));
	}
}

void AuSetDesigner::WalkColorClicked()
{
	QColor color = g_Au.DesignerDefaultWalkPathColor();
	QColorDialog::setCustomColor(0, color.rgb());				// First custom color is the default scope color.
	color = g_Au.DesignerWalkPathColor();
	QColorDialog::setCustomColor(1, color.rgb());				// Second custom color is the current scope color.
	color = QColorDialog::getColor(color, this, tr("Designer Walk Path Color"),
			g_Au.UseNativeColorDialog() ? QColorDialog::ColorDialogOptions() : QColorDialog::DontUseNativeDialog);
	if(color.isValid())
	{
		g_Au.SetDesignerWalkPathColor(color);
		m_WalkColorButton->setStyleSheet(QString("background-color: rgb(0, 0, 0); color: rgb(%1, %2, %3)")
				.arg(color.red()).arg(color.green()).arg(color.blue()));
	}
}
