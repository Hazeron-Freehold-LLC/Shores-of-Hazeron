//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auconstruction.cpp,v 1.43 2022/02/19 18:25:09 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auconstruction.h"
//
#include "auagentteleport.h"
#include "auatomrenderinfo.h"
#include "aubdexchangefilter.h"
#include "aubdreport.h"
#include "auclientglobal.h"
#include "audesignerapp.h"
#include "audesignerframe.h"
#include "audesignerview.h"
#include "augear.h"
#include "autoolbutton.h"
#include "daperson.h"
#include "daworldll.h"
#include "dgdata.h"
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QStackedLayout>
#include <QTimer>

void AuConstructionDock::UpdateTitleShortcut()
{
	AuFrame* frame = auApp->Frame();
	QString control = frame->ControlBinding(AuFrame::CI_ToggleConstructionUI);
	if(control.isEmpty())
		SetTitle(OriginalTitle());
	else
		SetTitle(OriginalTitle() + " (" + control + ")");
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuSkyLineWidget::AuSkyLineWidget(QWidget* parent) : QWidget(parent)
{
	m_BuildingCount = 0;
	m_FenceCount = 0;
	m_StreetLightCount = 0;
	m_BuildingVolume = 0;
	m_EnvType = 0;
	m_VolumeLimit = 0;
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

QSize AuSkyLineWidget::sizeHint() const
{
	return QSize(256,32);
}

void AuSkyLineWidget::SetBuildings(quint32 volume, quint32 buildingCount, quint32 fenceCount, quint32 streetLightCount, int envType)
{
	m_BuildingCount = buildingCount;
	m_FenceCount = fenceCount;
	m_StreetLightCount = streetLightCount;
	if(volume > m_VolumeLimit)
		m_BuildingVolume = m_VolumeLimit;
	else
		m_BuildingVolume = volume;
	m_EnvType = envType;
	update();
}

void AuSkyLineWidget::SetVolumeLimit(quint32 vl)
{
	m_VolumeLimit = vl;
	if(m_BuildingVolume > m_VolumeLimit)
		m_BuildingVolume = m_VolumeLimit;
	update();
}

void AuSkyLineWidget::paintEvent(QPaintEvent* ev)
{
	QWidget::paintEvent(ev);

	QPainter p(this);

	if(m_VolumeLimit)
	{
		// The first 20 street lights and the first 20 fences are free.
		quint32 bldCountForLimit = m_BuildingCount;
		if(m_FenceCount > 20)
			bldCountForLimit += m_FenceCount - 20;
		if(m_StreetLightCount > 20)
			bldCountForLimit += m_StreetLightCount - 20;

		// Load sky background.
		QImage bg;
		float fracVolFilled = (float)m_BuildingVolume / (float)m_VolumeLimit;
		//float fracVolFilled = AuTime::Now().CurrentCycle(30000);	//DHH cycle through images
		quint32 worldBldLimit = AuCityReport::WorldBuildingCountLimit();
		quint32 worldBldGreen = worldBldLimit - 50;	// Top of green range
		quint32 worldBldYellow = worldBldLimit - 5;	// Top of yellow range
		if(fracVolFilled < .9f && bldCountForLimit <= worldBldGreen)
		{
			if(m_EnvType != 4)
				bg = QImage(":data/SkyLineSky.png").scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			else
				bg = QImage(":data/SkyLineStars.png").scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);	// moon
		}
		else if(fracVolFilled < .99f && bldCountForLimit <= worldBldYellow)
			bg = QImage(":data/SkyLineSkyYellow.png").scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		else
			bg = QImage(":data/SkyLineSkyRed.png").scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

		// Draw sky background.
		p.drawImage(0,0, bg);

		// Draw empty desert terrain.
		QString envImg[] =
		{
			QString(":data/SkyLineStars.png"),
			QString(":data/SkyLineDesert.png"),
			QString(":data/SkyLinePlants.png"),
			QString(":data/SkyLineWater.png"),
			QString(":data/SkyLineMoon.png"),
		};

		QImage ter = QImage(envImg[m_EnvType]).scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		p.drawImage(0,0, ter);

		if(fracVolFilled > 0)
		{
			// Select the image with the number of buildings appropriate to the fraction of building volume used.
			// Y = (1 - X)^2 * images
			const int images = 127;							// There are 127 images.
			float mx = 1 - fracVolFilled;					// 1 minus X
			float mx2 = mx * mx;							// (1 - X)^2
			int image = (1 - mx2) * images;					// 0 to 127

			// Select the image with the number of buildings appropriate to the fraction of building count used.
			int imgC = (int)(sqrtf(bldCountForLimit) * 9);	// 200 buildings needed to reach 127.
			if(imgC > (int)bldCountForLimit)
				imgC = bldCountForLimit;					// Use building count, if the image number is less than that.
			if(image < imgC)
				image = imgC;
			if(image > 127)
				image = 127;
			else if(!image)
				image = 1;
			image = 128 - image;	// Images are reversed; number 1 is fully developed city; 127 is barely any city.

			// Draw buildings foreground, on top of the sky and desert.
			QString imName = QString(":data/SkyLineBuildings.%1.png").arg(image);
			QImage bd = QImage(imName).scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			p.drawImage(0,-2, bd);
		}
	}
	else
	{
		// Not at a planet?
		// Load starry background.
		QImage bg = QImage(":data/SkyLineStars.png").scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

		// Draw starry background.
		p.drawImage(0,0, bg);
	}
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuConstructButton::AuConstructButton(QWidget* receiver, QWidget* parent, AuScFence::STYLE fence) : QPushButton(parent)
{
	const DgData& gd = g_Au.LockDataForRead();
	const DbIndustry& dbi = gd.Industry(IID_Fence);
	m_Text = gd.IndustryName(IID_Fence);

	QString tip = QString("<b>%1</b><br>").arg(AuScFence::StyleName(fence));
	bool isWall = false;
	switch(fence)
	{
	case AuScFence::Posts:
		cpid = CPID_FencePosts;
		tip += tr("Posts block movement and gunfire.");
		break;
	case AuScFence::PostsT:
		cpid = CPID_FencePostsT;
		tip += tr("Posts block movement and gunfire.");
		break;
	case AuScFence::Wire:
		cpid = CPID_FenceWire;
		tip += tr("Wire fences block movement of large things. "
				"Movement of creatures is not blocked if their max health is 100hp or less. "
				"Gunfire is not blocked.");
		break;
	case AuScFence::WireT:
		cpid = CPID_FenceWireT;
		tip += tr("Wire fences block movement of large things. "
				"Movement of creatures is not blocked if their max health is 100hp or less. "
				"Gunfire is not blocked.");
		break;
	case AuScFence::Range:
		cpid = CPID_FenceRange;
		tip += tr("Range fencing blocks movement of most things. "
				"Movement of creatures is not blocked if their max health is 10hp or less. "
				"Gunfire is not blocked.");
		break;
	case AuScFence::RangeT:
		cpid = CPID_FenceRangeT;
		tip += tr("Range fencing blocks movement of most things. "
				"Movement of creatures is not blocked if their max health is 10hp or less. "
				"Gunfire is not blocked.");
		break;
	case AuScFence::ChainLink:
		cpid = CPID_FenceChainLink;
		tip += tr("Chain link fences block movement. "
				"Gunfire is not blocked.");
		break;
	case AuScFence::ChainLinkRazorWire:
		cpid = CPID_FenceChainLinkRazorWire;
		tip += tr("Chain link fences block movement. "
				"Gunfire is not blocked.");
		break;
	case AuScFence::SplitRail:
		cpid = CPID_FenceSplitRail;
		tip += tr("Split rail fences block movement of large things. "
				"Movement of creatures is not blocked if their max health is 100hp or less. "
				"Gunfire is not blocked.");
		break;
	case AuScFence::WoodYard:
		cpid = CPID_FenceWoodYard;
		tip += tr("Stockade fences block movement and gunfire.");
		break;
	case AuScFence::SandBagWall:
		cpid = CPID_FenceSandBagWall;
		tip += tr("Sand bag walls block movement and gunfire.");
		isWall = true;
		break;
	case AuScFence::BrickWall:
		cpid = CPID_FenceBrickWall;
		tip += tr("Walls block movement and gunfire.");
		isWall = true;
		break;
	case AuScFence::StoneWall:
		cpid = CPID_FenceStoneWall;
		tip += tr("Walls block movement and gunfire.");
		isWall = true;
		break;
	case AuScFence::Custom:
		cpid = CPID_FenceCustom;
		tip += tr("Custom walls and fences block movement and gunfire.");
		isWall = true;
		break;
	case AuScFence::STYLE_Count:
		break;
	}
	tip += "<br>";
	tip += dbi.BuildRuleText();
	if(!dbi.Note().isEmpty())
		tip += dbi.Note();
	g_Au.UnlockData();

	AuScFence fencePart;
	fencePart.SetStyle(fence, 0);
	m_Image = QPixmap(fencePart.TypeIcon());

	if(isWall)
		tip += tr("<i>Placement Tips</i><br>"
				"%1 Press <i>G</i> to locate the closest existing fence or wall post. "
				"<br>"
				"%1 Hold <i>Shift</i> while drawing the second end, to constrain the angle of the wall line."
				"<br>"
				"%1 Constraint angle is set in <b>Constraint Angle</b> control."
				"<br>"
				"%1 Press <i>Space</i> to continue the next fence where the previous fence ended.")
			.arg(g_Au.MiddleDotSymbol());
	else
		tip += tr("<i>Placement Tips</i><br>"
				"%1 Press <i>G</i> to locate the closest existing fence or wall post. "
				"<br>"
				"%1 Hold <i>Shift</i> while drawing the second end, to constrain the angle of the fence line."
				"<br>"
				"%1 Constraint angle is set in <b>Constraint Angle</b> control."
				"<br>"
				"%1 Press <i>Space</i> to continue the next fence where the previous fence ended.")
			.arg(g_Au.MiddleDotSymbol());
	setToolTip(tip);

	idi = IID_Fence;
	m_FenceStyle = fence;
	setMouseTracking(true);
	setMaximumSize(QSize(60,60));
	setMinimumSize(QSize(60,60));
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	if(receiver)
	{
		connect(this, SIGNAL(mouseMove(INDUSTRYID, CONSTRUCTIONPROCESSID)),
				receiver, SLOT(ListBuildingHover(INDUSTRYID, CONSTRUCTIONPROCESSID)));
		connect(this, SIGNAL(mousePress(INDUSTRYID, CONSTRUCTIONPROCESSID)),
				receiver, SLOT(BuildingSelected(INDUSTRYID, CONSTRUCTIONPROCESSID)));
	}
}

AuConstructButton::AuConstructButton(QWidget* receiver, QWidget* parent, INDUSTRYID industryId) : QPushButton(parent)
{
	const DgData& gd = g_Au.LockDataForRead();
	const DbIndustry& dbi = gd.Industry(industryId);
	m_Image = QPixmap(dbi.BitmapName());
	m_Text = dbi.Name();

	QString tip = QString("<b>%1</b><br>").arg(m_Text);
	if(!dbi.DoesWhat().isEmpty())
		tip += dbi.DoesWhat() + "<br>";
	tip += dbi.BuildRuleText();
	if(!dbi.Note().isEmpty())
		tip += dbi.Note();
	g_Au.UnlockData();

	// Buildings can be rotated.
	if(industryId == IID_AirportTerminal || industryId == IID_BaseAirTerminal)
		tip += tr("<br><i>Placement Tips</i><br>"
				"%1 <b>Cannot be placed inside a biodome or enclosed platform.</b><br>"
				"%1 Arrow indicates front facing; it usually faces toward the road, toward the sea for coastal buildings.<br>"
				"%1 <i>Space</i> aligns to the obstructing road or building.<br>"
				"%1 <i>Left/Right Arrows</i> rotates the building.<br>"
				"%1 Rotation angle step is set in <b>Constraint Angle</b> control.<br>"
				"%1 Hold <i>Shift</i> to rotate in 1 degree steps.<br>"
				"%1 <i>Up/Down Arrows</i> change manufacturing process.<br>"
				"%1 <i>* (asterisk)</i> switches <b>Make Product</b> on off.")
			.arg(g_Au.MiddleDotSymbol());
	else
		tip += tr("<br><i>Placement Tips</i><br>"
				"%1 Arrow indicates front facing; it usually faces toward the road, toward the sea for coastal buildings.<br>"
				"%1 <i>Space</i> aligns to the obstructing road or building.<br>"
				"%1 <i>Left/Right Arrows</i> rotates the building.<br>"
				"%1 Rotation angle step is set in <b>Constraint Angle</b> control.<br>"
				"%1 Hold <i>Shift</i> to rotate in 1 degree steps.<br>"
				"%1 <i>Up/Down Arrows</i> change manufacturing process.<br>"
				"%1 <i>* (asterisk)</i> switches <b>Make Product</b> on off.")
			.arg(g_Au.MiddleDotSymbol());
	setToolTip(tip);

	idi = industryId;
	cpid = CPID_Building;
	m_FenceStyle = AuScFence::Posts;
	setMouseTracking(true);
	setMaximumSize(QSize(60,60));
	setMinimumSize(QSize(60,60));
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	if(receiver)
	{
		connect(this, SIGNAL(mouseMove(INDUSTRYID, CONSTRUCTIONPROCESSID)),
				receiver, SLOT(ListBuildingHover(INDUSTRYID, CONSTRUCTIONPROCESSID)));
		connect(this, SIGNAL(mousePress(INDUSTRYID, CONSTRUCTIONPROCESSID)),
				receiver, SLOT(BuildingSelected(INDUSTRYID, CONSTRUCTIONPROCESSID)));
	}
}

AuConstructButton::AuConstructButton(const char* imageName, QWidget* receiver, QWidget* parent, QString sText, QString toolTip,
		INDUSTRYID industryId, CONSTRUCTIONPROCESSID conProcId) : QPushButton(parent)
{
	m_Image = QPixmap(imageName);
	m_Text = sText;

	QString tip(toolTip);
	if(tip.isEmpty())
		tip = QString("<b>%1</b>").arg(m_Text);
	else
		tip = QString("<b>%1</b><br>%2<br>").arg(m_Text).arg(toolTip);

	switch(conProcId)
	{
	case CPID_RoadGrade:
		tip += tr("<br><i>Placement Tips</i><br>"
				"%1 Hold <i>Shift</i> while drawing the second end, to constrain the angle of the graded area.<br>"
				"%1 Constraint angle is set in <b>Constraint Angle</b> control.<br>"
				"%1 Press <i>Space</i> to continue the next road where the previous road ended."
				"<br>"
				"%1 <i>Arrows</i> or the <i>mouse wheel</i> switch grading style while drawing.")
			.arg(g_Au.MiddleDotSymbol());
		break;

	case CPID_Irrigation:
		tip += tr("<br><i>Placement Tips</i><br>"
				"%1 Hold <i>Shift</i> while drawing the second end, to constrain the angle of the irrigated area.<br>"
				"%1 Constraint angle is set in <b>Constraint Angle</b> control.<br>"
				"%1 Press <i>Space</i> to continue the next irrigated area where the previous irrigated area ended."
				"<br>"
				"%1 Does not change terrain grading.")
			.arg(g_Au.MiddleDotSymbol());
		break;

	case CPID_Clearing:
		tip += tr("<br><i>Placement Tips</i><br>"
				"%1 Hold <i>Shift</i> while drawing the second end, to constrain the angle of the cleared area.<br>"
				"%1 Constraint angle is set in <b>Constraint Angle</b> control.<br>"
				"%1 Press <i>Space</i> to continue the next cleared area where the previous cleared area ended."
				"<br>"
				"%1 Does not change terrain grading.")
			.arg(g_Au.MiddleDotSymbol());
		break;

	case CPID_Defoliant:
		tip += tr("<br><i>Placement Tips</i><br>"
				"%1 Hold <i>Shift</i> while drawing the second end, to constrain the angle of the defoliated area.<br>"
				"%1 Constraint angle is set in <b>Constraint Angle</b> control.<br>"
				"%1 Press <i>Space</i> to continue the next defoliated area where the previous defoliated area ended."
				"<br>"
				"%1 Does not change terrain grading.")
			.arg(g_Au.MiddleDotSymbol());
		break;

	case CPID_DirtRoad:
		tip += tr("<br><i>Placement Tips</i><br>"
				"%1 Hold <i>Shift</i> while drawing the second end, to constrain the angle of the road.<br>"
				"%1 Constraint angle is set in <b>Constraint Angle</b> control.<br>"
				"%1 Press <i>Space</i> to continue the next road where the previous road ended."
				"<br>"
				"%1 <i>Arrows</i> or the <i>mouse wheel</i> switch grading style while drawing.<br>"
				"%1 Hold <i>Ctrl</i> to change an existing road to dirt construction.")
			.arg(g_Au.MiddleDotSymbol());
		break;
	case CPID_AsphaltRoad:
		tip += tr("<br><i>Placement Tips</i><br>"
				"%1 Hold <i>Shift</i> while drawing the second end, to constrain the angle of the road.<br>"
				"%1 Constraint angle is set in <b>Constraint Angle</b> control.<br>"
				"%1 Press <i>Space</i> to continue the next road where the previous road ended."
				"<br>"
				"%1 <i>Arrows</i> or the <i>mouse wheel</i> switch grading style while drawing.<br>"
				"%1 Hold <i>Ctrl</i> to change an existing road to asphalt construction.<br>")
			.arg(g_Au.MiddleDotSymbol());
		break;
	case CPID_ConcreteRoad:
		tip += tr("<br><i>Placement Tips</i><br>"
				"%1 Hold <i>Shift</i> while drawing the second end, to constrain the angle of the road.<br>"
				"%1 Constraint angle is set in <b>Constraint Angle</b> control.<br>"
				"%1 Press <i>Space</i> to continue the next road where the previous road ended."
				"<br>"
				"%1 <i>Arrows</i> or the <i>mouse wheel</i> switch grading style while drawing.<br>"
				"%1 Hold <i>Ctrl</i> to change an existing road to concrete construction.")
			.arg(g_Au.MiddleDotSymbol());
		break;

	default:
		break;
	}
	setToolTip(tip);

	idi = industryId;
	cpid = conProcId;
	m_FenceStyle = AuScFence::Posts;
	setMouseTracking(true);
	setMaximumSize(QSize(60,60));
	setMinimumSize(QSize(60,60));
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	if(receiver)
	{
		connect(this, SIGNAL(mouseMove(INDUSTRYID, CONSTRUCTIONPROCESSID)),
				receiver, SLOT(ListBuildingHover(INDUSTRYID, CONSTRUCTIONPROCESSID)));
		connect(this, SIGNAL(mousePress(INDUSTRYID, CONSTRUCTIONPROCESSID)),
				receiver, SLOT(BuildingSelected(INDUSTRYID, CONSTRUCTIONPROCESSID)));
	}
}

void AuConstructButton::mouseMoveEvent(QMouseEvent *e)
{
	QPushButton::mouseMoveEvent(e);
	emit mouseMove(idi, cpid);
}

void AuConstructButton::mousePressEvent(QMouseEvent *e)
{
	QPushButton::mousePressEvent(e);
	emit mousePress(idi, cpid);
}

void AuConstructButton::paintEvent(QPaintEvent* event)
{
	QPushButton::paintEvent(event);

	const int margin = 4;
	QRect rc = rect().adjusted(margin,margin,-margin,-margin);

	// We only draw the text if there is no pixmap.
	QPainter p(this);
	if(!m_Image.isNull())
	{
		if(isEnabled())
			p.drawPixmap(rc, m_Image);
		else
		{
			QImage grey = m_Image.toImage().convertToFormat(QImage::Format_Grayscale8);
			QPixmap pixgrey = QPixmap::fromImage(grey);
			p.setCompositionMode(QPainter::CompositionMode_Screen);
			p.drawPixmap(rc, pixgrey);
		}
	}
	else if(!m_Text.isEmpty())	// <- Remove 'else' to draw text on top of image
	{
		p.setFont(QFont(g_Au.TypefaceSans(), 7));
		// Drop shadow of text.
		p.setPen(QPen(QColor(0,0,0), 0.f));
		p.drawText(rc, Qt::AlignCenter|Qt::TextWordWrap, m_Text);
		// Draw bright text.
		rc.adjust(-1,-1,-1,-1);
		p.setPen(QPen(QColor(255,255,255), 0.f));
		p.drawText(rc, Qt::AlignCenter|Qt::TextWordWrap, m_Text);
	}
	p.end();
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuConstructionWidget::AuConstructionWidget(QWidget* parent) : QWidget(parent)
{
	m_BuildMaterial = CID_Stone;
	m_ExchangeRequested = false;
	m_ImageDesign = 0;
	m_IncludeExchange = g_Au.Settings().readBoolEntry("BdConstruction/Exchange", true);
	m_IsGasGiant = false;
	m_IsQuarry = false;
	m_ListIndustryId = IID_None;
	m_ListCPID = CPID_Building;
	m_Timer = 0;

	AuIconButton* pbstop = new AuIconButton(":data/Hand.png");
	pbstop->setToolTip(tr("Stop Current Construction Command"));
	connect(pbstop, SIGNAL(clicked()), SLOT(StopPressed()));

	AuIconButton* pbcode = new AuIconButton(":data/OrderRead.png");
	pbcode->setToolTip(tr("Building Codes..."));
	connect(pbcode, SIGNAL(clicked()), auApp->Frame(), SLOT(BuildingCodes()));

	AuIconButton* pbdestroy = new AuIconButton(":data/bp_ClearLand.png");
	pbdestroy->setToolTip(tr("<html><b>Demolish Development</b><br>"
				"Demolishes the development at the click location. "
				"Left|right arrows or mouse wheel to cycle through overlapping developments. "
				"</html>"));
	connect(pbdestroy, SIGNAL(clicked()), SLOT(DestroyPressed()));

	AuIconButton* pbclearcraters = new AuIconButton(":data/bp_ClearCraters.png");
	pbclearcraters->setToolTip(tr("<html><b>Clear Craters</b><br>"
				"Clears bomb craters at the click location. "
				"Left|right arrows or mouse wheel to cycle through overlapping craters. "
				"</html>"));
	connect(pbclearcraters, SIGNAL(clicked()), SLOT(ClearCratersPressed()));

	m_Pages = new QStackedLayout;

	// Food Production
	{
		m_Food = new AuIconButton(":data/c_Vegetable.png");
		m_Food->setCheckable(true);
		m_Food->setToolTip(tr("Food Production"));
		m_Food->setChecked(true);
		connect(m_Food, SIGNAL(clicked()), SLOT(FoodPressed()));

		m_FoodPage = new QWidget(this);
		AuConstructButton* pbBakery = new AuConstructButton(this, m_FoodPage, IID_Bakery);
		AuConstructButton* pbDairy = new AuConstructButton(this, m_FoodPage, IID_Dairy);
		AuConstructButton* pbBrewery = new AuConstructButton(this, m_FoodPage, IID_Brewery);
		AuConstructButton* pbDistillery = new AuConstructButton(this, m_FoodPage, IID_Distillery);
		AuConstructButton* pbFarm = new AuConstructButton(this, m_FoodPage, IID_Farm);
		AuConstructButton* pbGristMill = new AuConstructButton(this, m_FoodPage, IID_GristMill);
		AuConstructButton* pbMeatPlant = new AuConstructButton(this, m_FoodPage, IID_MeatProcessingPlant);
		AuConstructButton* pbOrchard = new AuConstructButton(this, m_FoodPage, IID_Orchard);
		AuConstructButton* pbSugarMill = new AuConstructButton(this, m_FoodPage, IID_SugarMill);
		AuConstructButton* pbWharf = new AuConstructButton(this, m_FoodPage, IID_Wharf);
		AuConstructButton* pbWinery = new AuConstructButton(this, m_FoodPage, IID_Winery);

		QGridLayout* pageLay = new QGridLayout;
		pageLay->setMargin(0);
		pageLay->setSpacing(2);
		int row=0;
		int col=0;
		pageLay->addWidget(pbFarm, row,col++);
		pageLay->addWidget(pbOrchard, row,col++);
		col++;
		pageLay->addWidget(pbWharf, row,col++);
		row++;
		col=0;
		pageLay->addWidget(pbMeatPlant, row,col++);
		pageLay->addWidget(pbDairy, row,col++);
		pageLay->addWidget(pbGristMill, row,col++);
		pageLay->addWidget(pbSugarMill, row,col++);
		row++;
		col=0;
		pageLay->addWidget(pbBakery, row,col++);
		pageLay->addWidget(pbBrewery, row,col++);
		pageLay->addWidget(pbDistillery, row,col++);
		pageLay->addWidget(pbWinery, row,col++);
		row++;
		pageLay->setColumnStretch(col, 1);	// Make the last column stretch to push the buttons to the left.
		pageLay->setRowStretch(row, 1);	// Make the last row stretch to push the buttons to the top.
		m_FoodPage->setLayout(pageLay);
		m_Pages->addWidget(m_FoodPage);
	}

	// Housing
	{
		m_Housing = new AuIconButton(":data/Construction.png");
		m_Housing->setCheckable(true);
		m_Housing->setToolTip(tr("Housing"));
		connect(m_Housing, SIGNAL(clicked()), SLOT(HousingPressed()));

		m_HousingPage = new QWidget(this);
		AuConstructButton* pbApartment = new AuConstructButton(this, m_HousingPage, IID_Apartment);
		AuConstructButton* pbHouse = new AuConstructButton(this, m_HousingPage, IID_House);
		AuConstructButton* pbPrivateSecurityContractor = new AuConstructButton(this, m_HousingPage, IID_PoliceAcademy);

		QGridLayout* pageLay = new QGridLayout;
		pageLay->setMargin(0);
		pageLay->setSpacing(2);
		int row=0;
		int col=0;
		pageLay->addWidget(pbHouse, row,col++);
		row++;
		pageLay->addWidget(pbApartment, row,col++);
		row++;
		pageLay->addWidget(pbPrivateSecurityContractor, row,col++);
		row++;
		pageLay->setColumnStretch(col, 1);	// Make the last column stretch to push the buttons to the left.
		pageLay->setRowStretch(row, 1);	// Make the last row stretch to push the buttons to the top.
		m_HousingPage->setLayout(pageLay);
		m_Pages->addWidget(m_HousingPage);
	}

	// Government
	{
		m_Government = new AuIconButton(":data/CommGovernment.png");
		m_Government->setCheckable(true);
		m_Government->setToolTip(tr("Government"));
		connect(m_Government, SIGNAL(clicked()), SLOT(GovernmentPressed()));

		m_GovernmentPage = new QWidget(this);
		pbCapitol = new AuConstructButton(this, m_GovernmentPage, IID_Capitol);
		pbPoliceStation = new AuConstructButton(this, m_GovernmentPage, IID_PoliceDepartment);
		pbBank = new AuConstructButton(this, m_GovernmentPage, IID_Bank);
		AuConstructButton* pbMassMedia = new AuConstructButton(this, m_GovernmentPage, IID_MassMedia);
		AuConstructButton* pbObservatory = new AuConstructButton(this, m_GovernmentPage, IID_Observatory);
		AuConstructButton* pbSchool = new AuConstructButton(this, m_GovernmentPage, IID_School);
		AuConstructButton* pbFireStation = new AuConstructButton(this, m_GovernmentPage, IID_FireDepartment);
		AuConstructButton* pbHospital = new AuConstructButton(this, m_GovernmentPage, IID_Hospital);

		QGridLayout* pageLay = new QGridLayout;
		pageLay->setMargin(0);
		pageLay->setSpacing(2);
		int row=0;
		int col=4;
		pageLay->addWidget(pbMassMedia, row,col, 2,2);
		col+=2;
		pageLay->addWidget(pbObservatory, row,col, 2,2);
		row++;
		col=1;
		pageLay->addWidget(pbCapitol, row,col, 2,2);
		row++;
		col=5;
		pageLay->addWidget(pbSchool, row,col, 2,2);
		row++;
		col=0;
		pageLay->addWidget(pbBank, row,col, 2,2);
		col+=2;
		pageLay->addWidget(pbPoliceStation, row,col, 2,2);
		row++;
		col=4;
		pageLay->addWidget(pbFireStation, row,col, 2,2);
		col+=2;
		pageLay->addWidget(pbHospital, row,col, 2,2);
		row=6;
		col=8;
		pageLay->setColumnStretch(col, 1);	// Make the last column stretch to push the buttons to the left.
		pageLay->setRowStretch(row, 1);	// Make the last row stretch to push the buttons to the top.
		m_GovernmentPage->setLayout(pageLay);
		m_Pages->addWidget(m_GovernmentPage);
	}

	// Military
	{
		m_Military = new AuIconButton(":data/CommMilitary.png");
		m_Military->setCheckable(true);
		m_Military->setToolTip(tr("Military"));
		connect(m_Military, SIGNAL(clicked()), SLOT(MilitaryPressed()));

		m_MilitaryPage = new QWidget(this);
		AuConstructButton* pbHeadquarters = new AuConstructButton(this, m_MilitaryPage, IID_BaseHeadquarters);
		AuConstructButton* pbBaseAirTerminal = new AuConstructButton(this, m_MilitaryPage, IID_BaseAirTerminal);
		AuConstructButton* pbBaseMotorPool = new AuConstructButton(this, m_MilitaryPage, IID_BaseMotorPool);
		AuConstructButton* pbBaseDock = new AuConstructButton(this, m_MilitaryPage, IID_BaseDock);
		AuConstructButton* pbAcademy = new AuConstructButton(this, m_MilitaryPage, IID_BaseAcademy);
		AuConstructButton* pbBarracks = new AuConstructButton(this, m_MilitaryPage, IID_BaseBarracks);
		AuConstructButton* pbMarineBarracks = new AuConstructButton(this, m_MilitaryPage, IID_BaseMarineBarracks);
		AuConstructButton* pbGuardPost = new AuConstructButton(this, m_MilitaryPage, IID_BaseGuardPost);
		AuConstructButton* pbGuardTower = new AuConstructButton(this, m_MilitaryPage, IID_BaseGuardTower);
		AuConstructButton* pbBaseWeapon = new AuConstructButton(this, m_MilitaryPage, IID_BaseWeapon);
		AuConstructButton* pbBaseShield = new AuConstructButton(this, m_MilitaryPage, IID_BaseShield);
		AuConstructButton* pbBaseRadar = new AuConstructButton(this, m_MilitaryPage, IID_BaseRadar);

		QGridLayout* pageLay = new QGridLayout;
		pageLay->setMargin(0);
		pageLay->setSpacing(2);
		int row=0;
		int col=0;
		pageLay->addWidget(pbHeadquarters, row,col++);
		pageLay->addWidget(pbBarracks, row,col++);
		pageLay->addWidget(pbMarineBarracks, row,col++);
		pageLay->addWidget(pbAcademy, row,col++);
		row++;
		col=0;
		pageLay->addWidget(pbGuardPost, row,col++);
		pageLay->addWidget(pbBaseMotorPool, row,col++);
		pageLay->addWidget(pbBaseDock, row,col++);
		pageLay->addWidget(pbBaseAirTerminal, row,col++);
		row++;
		col=0;
		pageLay->addWidget(pbGuardTower, row,col++);
		pageLay->addWidget(pbBaseShield, row,col++);
		pageLay->addWidget(pbBaseWeapon, row,col++);
		pageLay->addWidget(pbBaseRadar, row,col++);
		row++;
		col=4;
		pageLay->setColumnStretch(col, 1);	// Make the last column stretch to push the buttons to the left.
		pageLay->setRowStretch(row, 1);	// Make the last row stretch to push the buttons to the top.
		m_MilitaryPage->setLayout(pageLay);
		m_Pages->addWidget(m_MilitaryPage);
	}

	// Power Generation
	{
		m_Power = new AuIconButton(":data/c_Electricity.png");
		m_Power->setCheckable(true);
		m_Power->setToolTip(tr("Power Generation"));
		connect(m_Power, SIGNAL(clicked()), SLOT(PowerPressed()));

		m_PowerPage = new QWidget(this);
		AuConstructButton* pbCoalPlant = new AuConstructButton(this, m_PowerPage, IID_CoalPowerPlant);
		AuConstructButton* pbGasolinePlant = new AuConstructButton(this, m_PowerPage, IID_GasolinePowerPlant);
		AuConstructButton* pbHydroelectricPlant = new AuConstructButton(this, m_PowerPage, IID_HydroelectricPowerPlant);
		AuConstructButton* pbHydrogenPlant = new AuConstructButton(this, m_PowerPage, IID_HydrogenPowerPlant);
		AuConstructButton* pbNaturalGasPlant = new AuConstructButton(this, m_PowerPage, IID_NaturalGasPowerPlant);
		AuConstructButton* pbNuclearPlant = new AuConstructButton(this, m_PowerPage, IID_NuclearPowerPlant);
		AuConstructButton* pbSolarPlant = new AuConstructButton(this, m_PowerPage, IID_SolarPowerPlant);
		AuConstructButton* pbWindPlant = new AuConstructButton(this, m_PowerPage, IID_WindPowerPlant);

		QGridLayout* pageLay = new QGridLayout;
		pageLay->setMargin(0);
		pageLay->setSpacing(2);
		int row=0;
		int col=0;
		pageLay->addWidget(pbHydroelectricPlant, row,col++);
		pageLay->addWidget(pbSolarPlant, row,col++);
		pageLay->addWidget(pbWindPlant, row,col++);
		row++;
		col=1;
		pageLay->addWidget(pbCoalPlant, row,col++);
		pageLay->addWidget(pbNaturalGasPlant, row,col++);
		row++;
		col=1;
		pageLay->addWidget(pbGasolinePlant, row,col++);
		pageLay->addWidget(pbHydrogenPlant, row,col++);
		pageLay->addWidget(pbNuclearPlant, row,col++);
		row++;
		col=4;
		pageLay->setColumnStretch(col, 1);	// Make the last column stretch to push the buttons to the left.
		pageLay->setRowStretch(row, 1);	// Make the last row stretch to push the buttons to the top.
		m_PowerPage->setLayout(pageLay);
		m_Pages->addWidget(m_PowerPage);
	}

	// Manufacture
	{
		m_Manufacture = new AuIconButton(":data/c_MineMachinery.png");
		m_Manufacture->setCheckable(true);
		m_Manufacture->setToolTip(tr("Manufacture"));
		connect(m_Manufacture, SIGNAL(clicked()), SLOT(ManufacturePressed()));

		m_ManufacturePage = new QWidget(this);
		AuConstructButton* pbExplosivesFactory = new AuConstructButton(this, m_ManufacturePage, IID_ExplosivesFactory);
		AuConstructButton* pbLoggingCamp = new AuConstructButton(this, m_ManufacturePage, IID_LoggingCamp);
		AuConstructButton* pbLumberMill = new AuConstructButton(this, m_ManufacturePage, IID_LumberMill);
		AuConstructButton* pbMine = new AuConstructButton(this, m_ManufacturePage, IID_Mine);
		AuConstructButton* pbNuclearFuelPlant = new AuConstructButton(this, m_ManufacturePage, IID_NuclearFuelPlant);
		AuConstructButton* pbRefinery = new AuConstructButton(this, m_ManufacturePage, IID_Refinery);
		AuConstructButton* pbRocketFactory = new AuConstructButton(this, m_ManufacturePage, IID_RocketMotorFactory);
		AuConstructButton* pbSmelter = new AuConstructButton(this, m_ManufacturePage, IID_Smelter);
		AuConstructButton* pbTextileMill = new AuConstructButton(this, m_ManufacturePage, IID_TextileMill);
		AuConstructButton* pbTreatmentPlant = new AuConstructButton(this, m_ManufacturePage, IID_TreatmentPlant);
		AuConstructButton* pbWell = new AuConstructButton(this, m_ManufacturePage, IID_Well);
		AuConstructButton* pbDrillingRig = new AuConstructButton(this, m_ManufacturePage, IID_DrillingRig);

		QGridLayout* pageLay = new QGridLayout;
		pageLay->setMargin(0);
		pageLay->setSpacing(2);
		int row=0;
		int col=0;
		pageLay->addWidget(pbMine, row,col++);
		pageLay->addWidget(pbLoggingCamp, row,col++);
		pageLay->addWidget(pbWell, row,col++);
		pageLay->addWidget(pbDrillingRig, row,col++);
		row++;
		col=0;
		pageLay->addWidget(pbSmelter, row,col++);
		pageLay->addWidget(pbLumberMill, row,col++);
		pageLay->addWidget(pbRefinery, row,col++);
		pageLay->addWidget(pbTreatmentPlant, row,col++);
		row++;
		col=0;
		pageLay->addWidget(pbTextileMill, row,col++);
		pageLay->addWidget(pbNuclearFuelPlant, row,col++);
		pageLay->addWidget(pbExplosivesFactory, row,col++);
		pageLay->addWidget(pbRocketFactory, row,col++);
		row++;
		col=4;
		pageLay->setColumnStretch(col, 1);	// Make the last column stretch to push the buttons to the left.
		pageLay->setRowStretch(row, 1);	// Make the last row stretch to push the buttons to the top.
		m_ManufacturePage->setLayout(pageLay);
		m_Pages->addWidget(m_ManufacturePage);
	}

	// Tools
	{
		m_Tools = new AuIconButton(":data/c_Hammer.png");
		m_Tools->setCheckable(true);
		m_Tools->setToolTip(tr("Tools and Weapons"));
		connect(m_Tools, SIGNAL(clicked()), SLOT(ToolsPressed()));

		m_ToolsPage = new QWidget(this);
		AuConstructButton* pbArmorer = new AuConstructButton(this, m_ToolsPage, IID_Armorer);
		AuConstructButton* pbCarpenter = new AuConstructButton(this, m_ToolsPage, IID_Carpenter);
		AuConstructButton* pbClothingFactory = new AuConstructButton(this, m_ToolsPage, IID_ClothingFactory);
		AuConstructButton* pbComputerFactory = new AuConstructButton(this, m_ToolsPage, IID_ComputerFactory);
		AuConstructButton* pbJeweler = new AuConstructButton(this, m_ToolsPage, IID_Jeweler);
		AuConstructButton* pbElectronicsFactory = new AuConstructButton(this, m_ToolsPage, IID_ElectronicsFactory);
		AuConstructButton* pbMachineShop = new AuConstructButton(this, m_ToolsPage, IID_MachineShop);
		AuConstructButton* pbMunitionsFactory = new AuConstructButton(this, m_ToolsPage, IID_MunitionsFactory);
		AuConstructButton* pbPaperMill = new AuConstructButton(this, m_ToolsPage, IID_PaperMill);
		AuConstructButton* pbPharmacyLab = new AuConstructButton(this, m_ToolsPage, IID_PharmaceuticalLab);
		AuConstructButton* pbPlasticsFactory = new AuConstructButton(this, m_ToolsPage, IID_PlasticsFactory);
		AuConstructButton* pbWeaponSmith = new AuConstructButton(this, m_ToolsPage, IID_GunSmith);

		QGridLayout* pageLay = new QGridLayout;
		pageLay->setMargin(0);
		pageLay->setSpacing(2);
		int row=0;
		int col=0;
		pageLay->addWidget(pbCarpenter, row,col++);
		pageLay->addWidget(pbMachineShop, row,col++);
		pageLay->addWidget(pbPlasticsFactory, row,col++);
		pageLay->addWidget(pbElectronicsFactory, row,col++);
		row++;
		col=0;
		pageLay->addWidget(pbComputerFactory, row,col++);
		pageLay->addWidget(pbClothingFactory, row,col++);
		pageLay->addWidget(pbArmorer, row,col++);
		pageLay->addWidget(pbJeweler, row,col++);
		row++;
		col=0;
		pageLay->addWidget(pbPharmacyLab, row,col++);
		pageLay->addWidget(pbPaperMill, row,col++);
		pageLay->addWidget(pbWeaponSmith, row,col++);
		pageLay->addWidget(pbMunitionsFactory, row,col++);
		row++;
		col=4;
		pageLay->setColumnStretch(col, 1);	// Make the last column stretch to push the buttons to the left.
		pageLay->setRowStretch(row, 1);	// Make the last row stretch to push the buttons to the top.
		m_ToolsPage->setLayout(pageLay);
		m_Pages->addWidget(m_ToolsPage);
	}

	// Transport
	{
		m_Transport = new AuIconButton(":data/c_SpaceFighter.png");
		m_Transport->setCheckable(true);
		m_Transport->setToolTip(tr("Transportation"));
		connect(m_Transport, SIGNAL(clicked()), SLOT(TransportPressed()));

		m_TransportPage = new QWidget(this);
		AuConstructButton* pbAircraftFactory = new AuConstructButton(this, m_TransportPage, IID_AircraftFactory);
		AuConstructButton* pbAirportRadar = new AuConstructButton(this, m_TransportPage, IID_AirportRadar);
		AuConstructButton* pbAirportRepair = new AuConstructButton(this, m_TransportPage, IID_AirportRepairShop);
		AuConstructButton* pbAirportTerminal = new AuConstructButton(this, m_TransportPage, IID_AirportTerminal);
		AuConstructButton* pbDesignStudio = new AuConstructButton(this, m_TransportPage, IID_DesignStudio);
		AuConstructButton* pbShipYard = new AuConstructButton(this, m_TransportPage, IID_ShipYard);
		AuConstructButton* pbSpacecraftFactory = new AuConstructButton(this, m_TransportPage, IID_SpacecraftFactory);
		AuConstructButton* pbHangar = new AuConstructButton(this, m_TransportPage, IID_Hangar);
		AuConstructButton* pbVehicleFactory = new AuConstructButton(this, m_TransportPage, IID_VehicleFactory);

		QGridLayout* pageLay = new QGridLayout;
		pageLay->setMargin(0);
		pageLay->setSpacing(2);
		int row=0;
		int col=0;
		pageLay->addWidget(pbAirportTerminal, row,col, 2,2);
		col+=2;
		pageLay->addWidget(pbAirportRepair, row,col, 2,2);
		col+=2;
		pageLay->addWidget(pbHangar, row,col, 2,2);
		col+=2;
		pageLay->addWidget(pbAirportRadar, row,col, 2,2);
		row+=2;
		col=4;
		pageLay->addWidget(pbAircraftFactory, row,col, 2,2);
		col+=2;
		pageLay->addWidget(pbSpacecraftFactory, row,col, 2,2);
		row++;
		col=1;
		pageLay->addWidget(pbDesignStudio, row,col, 2,2);
		row++;
		col=4;
		pageLay->addWidget(pbVehicleFactory, row,col, 2,2);
		col+=2;
		pageLay->addWidget(pbShipYard, row,col);
		row+=2;
		col=9;
		pageLay->setColumnStretch(col, 1);	// Make the last column stretch to push the buttons to the left.
		pageLay->setRowStretch(row, 1);	// Make the last row stretch to push the buttons to the top.
		m_TransportPage->setLayout(pageLay);
		m_Pages->addWidget(m_TransportPage);
	}

	// Commerce
	{
		m_Commerce = new AuIconButton(":data/MsgBuyOffer.png");
		m_Commerce->setCheckable(true);
		m_Commerce->setToolTip(tr("Commercial"));
		connect(m_Commerce, SIGNAL(clicked()), SLOT(CommercePressed()));

		m_CommercePage = new QWidget(this);
		AuConstructButton* pbArena = new AuConstructButton(this, m_CommercePage, IID_Arena);
		AuConstructButton* pbBroker = new AuConstructButton(this, m_CommercePage, IID_Broker);
		AuConstructButton* pbCasino = new AuConstructButton(this, m_CommercePage, IID_Casino);
		AuConstructButton* pbChurch = new AuConstructButton(this, m_CommercePage, IID_Church);
		AuConstructButton* pbGrocery = new AuConstructButton(this, m_CommercePage, IID_Grocery);
		AuConstructButton* pbPark = new AuConstructButton(this, m_CommercePage, IID_Park);
		AuConstructButton* pbRetailStore = new AuConstructButton(this, m_CommercePage, IID_RetailStore);
		AuConstructButton* pbCantina = new AuConstructButton(this, m_CommercePage, IID_Cantina);
		AuConstructButton* pbShed = new AuConstructButton(this, m_CommercePage, IID_Shed);
		AuConstructButton* pbStorageTank = new AuConstructButton(this, m_CommercePage, IID_StorageTank);
		AuConstructButton* pbWarehouse = new AuConstructButton(this, m_CommercePage, IID_Warehouse);
		AuConstructButton* pbZoo = new AuConstructButton(this, m_CommercePage, IID_Zoo);

		QGridLayout* pageLay = new QGridLayout;
		pageLay->setMargin(0);
		pageLay->setSpacing(2);
		int row=0;
		int col=0;
		pageLay->addWidget(pbCantina, row,col++);
		pageLay->addWidget(pbGrocery, row,col++);
		pageLay->addWidget(pbRetailStore, row,col++);
		pageLay->addWidget(pbBroker, row,col++);
		col=0;
		row++;
		pageLay->addWidget(pbArena, row,col++);
		pageLay->addWidget(pbShed, row,col++);
		pageLay->addWidget(pbStorageTank, row,col++);
		pageLay->addWidget(pbWarehouse, row,col++);
		col=0;
		row++;
		pageLay->addWidget(pbCasino, row,col++);
		pageLay->addWidget(pbChurch, row,col++);
		pageLay->addWidget(pbPark, row,col++);
		pageLay->addWidget(pbZoo, row,col++);
		row++;
		pageLay->setColumnStretch(col, 1);	// Make the last column stretch to push the buttons to the left.
		pageLay->setRowStretch(row, 1);	// Make the last row stretch to push the buttons to the top.
		m_CommercePage->setLayout(pageLay);
		m_Pages->addWidget(m_CommercePage);
	}

	// Roads
	{
		m_Roads = new AuIconButton(":data/bp_RoadDirt.bmp");
		m_Roads->setCheckable(true);
		m_Roads->setToolTip(tr("Roads and Landscape"));
		connect(m_Roads, SIGNAL(clicked()), SLOT(RoadsPressed()));

		m_RoadsPage = new QWidget(this);
		AuConstructButton* pbRoadGrade = new AuConstructButton(":data/bp_RoadGrade.bmp",
				this, m_RoadsPage, tr("Grade Area"),
				tr("Graded areas change the shape of existing terrain.<br>"
					"%1 The presence of foliage is not affected.<br>"
					"%1 Grading does not block construction of roads or buildings.<br>"
					"%1 Grading does not interconnect buildings like roads.<br>"
					"%1 Grading requires labor but no materials.").arg(g_Au.MiddleDotSymbol()),
				IID_Road, CPID_RoadGrade);
		AuConstructButton* pbDefoliate = new AuConstructButton(":data/bp_RoadDefoliate.bmp",
				this, m_RoadsPage, tr("Defoliate Area"),
				tr("Defoliated areas remove trees, shrubs and ground cover that are present.<br>"
					"%1 Roads and buildings clear their site appropriately. It is not necessary to clear plants for them.<br>"
					"%1 Defoliation does not block construction of roads or buildings.<br>"
					"%1 Defoliation does not interconnect buildings like roads.<br>"
					"%1 Defoliation requires labor but no materials.").arg(g_Au.MiddleDotSymbol()),
				IID_Landscape, CPID_Defoliant);
		AuConstructButton* pbClearing = new AuConstructButton(":data/bp_RoadClearing.bmp",
				this, m_RoadsPage, tr("Clear Area"),
				tr("Cleared areas remove trees and shrubs that are present. "
					"Any ground cover that is present is mowed.<br>"
					"%1 Roads and buildings clear their site appropriately. It is not necessary to clear plants for them.<br>"
					"%1 Clearing does not block construction of roads or buildings.<br>"
					"%1 Clearing does not interconnect buildings like roads.<br>"
					"%1 Clearing requires labor but no materials.").arg(g_Au.MiddleDotSymbol()),
				IID_Landscape, CPID_Clearing);
		AuConstructButton* pbIrrigate = new AuConstructButton(":data/bp_RoadIrrigate.bmp",
				this, m_RoadsPage, tr("Irrigate Area"),
				tr("Irrigated areas grow local trees, shrubs and ground cover. "
					"Ground cover is mowed.<br>"
					"%1 Irrigation does not consume water from city supplies.<br>"
					"%1 Irrigation does not block construction of roads or buildings.<br>"
					"%1 Irrigation does not interconnect buildings like roads.<br>"
					"%1 Irrigation requires labor but no materials.").arg(g_Au.MiddleDotSymbol()),
				IID_Landscape, CPID_Irrigation);
		AuConstructButton* pbDirtRoad = new AuConstructButton(":data/bp_RoadDirt.bmp",
				this, m_RoadsPage, tr("Dirt Road"), tr("Roads interconnect buildings."),
				IID_Road, CPID_DirtRoad);
		AuConstructButton* pbAsphaltRoad = new AuConstructButton(":data/bp_RoadAsphalt.bmp",
				this, m_RoadsPage, tr("Asphalt Road"), tr("Roads interconnect buildings. "
					"Asphalt roads cannot be built on inferno worlds."),
				IID_Road, CPID_AsphaltRoad);
		AuConstructButton* pbConcreteRoad = new AuConstructButton(":data/bp_RoadConcrete.bmp",
				this, m_RoadsPage, tr("Concrete Road"), tr("Roads interconnect buildings."),
				IID_Road, CPID_ConcreteRoad);
		AuConstructButton* pbBridge = new AuConstructButton(this, m_RoadsPage, IID_Bridge);
		AuConstructButton* pbStreetLight = new AuConstructButton(this, m_RoadsPage, IID_StreetLight);

		m_RoadType = new QComboBox(this);
		m_RoadType->setToolTip(tr("<html><b>Grading Style</b><br>"
					"Selects the type of grading for the next road or graded area to be drawn.<br>"
					"%1 <i>Sloping grade</i> goes up or down when the ends are at different altitudes.<br>"
					"%1 <i>Level grade</i> forces the altitude of the second end to match the starting end, "
					"so the ground is level.<br>"
					"%1 Level altitude is determined by the altitude of the starting point.<br>"
					"%1 <i>Left/Right Arrows</i> switch grading while drawing."
					"%1 <i>Up/Down Arrows</i> or the <i>mouse wheel</i> raises and lowers the elevation relative to the terrain, "
					"to make ridges and trenches."
					"</html>").arg(g_Au.MiddleDotSymbol()));
		m_RoadType->addItem(tr("Sloping Grade"));
		m_RoadType->addItem(tr("Level Grade"));
		if(g_AuClient.RoadType() == 0)
			m_RoadType->setCurrentText(tr("Sloping Grade"));
		else
			m_RoadType->setCurrentText(tr("Level Grade"));
		connect(m_RoadType, SIGNAL(currentIndexChanged(const QString&)), SLOT(RoadTypeChanged(const QString&)));

		QString minSlab = QString("%1").arg(ceilf(MinSpacecraftSlabWidthMeters()));
		QString minVehSlab = QString("%1").arg(MinVehicleSlabWidthMeters());
		QString maxWidth = QString("%1").arg(MaxRoadWidthMeters());

		m_RoadWidth = new QComboBox(this);
		m_RoadWidth->setToolTip(tr("<html><b>Road Width</b><br>"
				"The width of the next road, slab, grade, irrigation or defoliant, in meters.<br>"
				"%1 Decimals are ok.<br>"
				"%1 Minimum width of a road is %2m.<br>"
				"%1 Maximum width of a road is %3m."
				"</html>")
				.arg(g_Au.MiddleDotSymbol())
				.arg(MinRoadWidthMeters())
				.arg(maxWidth));
		m_RoadWidth->setEditable(true);
		m_RoadWidth->addItem("8");
		m_RoadWidth->addItem("16");
		m_RoadWidth->addItem("24");
		m_RoadWidth->addItem("32");
		m_RoadWidth->addItem(minVehSlab);
		m_RoadWidth->addItem(minSlab);
		m_RoadWidth->addItem(maxWidth);
		if(g_AuClient.RoadType() == 0)
			m_RoadWidth->setEditText(QString("%1").arg(g_AuClient.RoadWidthInMeters()));
		else
			m_RoadWidth->setEditText(QString("%1").arg(g_AuClient.SlabWidthInMeters()));
		connect(m_RoadWidth, SIGNAL(editTextChanged(const QString&)), SLOT(RoadWidthChanged(const QString&)));

		m_RoundAngle = new QComboBox(this);
		m_RoundAngle->setToolTip(tr("<html><b>Constraint Angle</b><br>"
				"The number of degrees for constraining the angle of buildings, roads and landscape areas."
				"</html>"));
		m_RoundAngle->setEditable(true);
		m_RoundAngle->addItem("90");
		m_RoundAngle->addItem("60");
		m_RoundAngle->addItem("45");
		m_RoundAngle->addItem("30");
		m_RoundAngle->addItem("22.5");
		m_RoundAngle->addItem("11.25");
		m_RoundAngle->addItem("10");
		m_RoundAngle->addItem("5");
		m_RoundAngle->addItem("1");
		m_RoundAngle->setEditText(QString("%1").arg(g_AuClient.RoundingAngle()));
		connect(m_RoundAngle, SIGNAL(editTextChanged(const QString&)), SLOT(RoundingAngleChanged(const QString&)));

		QGridLayout* pageLay = new QGridLayout;
		pageLay->setMargin(0);
		pageLay->setSpacing(2);
		int row=0;
		int col=0;
		pageLay->addWidget(pbDirtRoad, row,col++);
		pageLay->addWidget(pbAsphaltRoad, row,col++);
		pageLay->addWidget(pbConcreteRoad, row,col++);
		pageLay->addWidget(pbBridge, row,col++);
		row++;
		col=0;
		pageLay->addWidget(pbRoadGrade, row,col++);
		pageLay->addWidget(pbDefoliate, row,col++);
		pageLay->addWidget(pbClearing, row,col++);
		pageLay->addWidget(pbIrrigate, row,col++);
		row++;
		col=0;
		pageLay->addWidget(m_RoadType, row,col, 1,2);
		pageLay->addWidget(pbStreetLight, row,3, 2,1);
		row++;
		pageLay->addWidget(m_RoadWidth, row,col, 1,2);
		row++;
		col=4;
		pageLay->setColumnStretch(col, 1);	// Make the last column stretch to push the buttons to the left.
		pageLay->setRowStretch(row, 1);	// Make the last row stretch to push the buttons to the top.
		m_RoadsPage->setLayout(pageLay);
		m_Pages->addWidget(m_RoadsPage);
	}

	// Fences
	{
		m_Fences = new AuIconButton(":data/bFenceWall.png");
		m_Fences->setCheckable(true);
		m_Fences->setToolTip(tr("Fences and Walls"));
		connect(m_Fences, SIGNAL(clicked()), SLOT(FencesPressed()));

		m_FencesPage = new QWidget(this);
		AuConstructButton* pbPosts = new AuConstructButton(this, m_FencesPage, AuScFence::Posts);
		AuConstructButton* pbPostsT = new AuConstructButton(this, m_FencesPage, AuScFence::PostsT);
		AuConstructButton* pbWire = new AuConstructButton(this, m_FencesPage, AuScFence::Wire);
		AuConstructButton* pbWireT = new AuConstructButton(this, m_FencesPage, AuScFence::WireT);
		AuConstructButton* pbRange = new AuConstructButton(this, m_FencesPage, AuScFence::Range);
		AuConstructButton* pbRangeT = new AuConstructButton(this, m_FencesPage, AuScFence::RangeT);
		AuConstructButton* pbChainLink = new AuConstructButton(this, m_FencesPage, AuScFence::ChainLink);
		AuConstructButton* pbChainLinkRazorWire = new AuConstructButton(this, m_FencesPage, AuScFence::ChainLinkRazorWire);
		AuConstructButton* pbSplitRail = new AuConstructButton(this, m_FencesPage, AuScFence::SplitRail);
		AuConstructButton* pbWoodYard = new AuConstructButton(this, m_FencesPage, AuScFence::WoodYard);
		AuConstructButton* pbSandBagWall = new AuConstructButton(this, m_FencesPage, AuScFence::SandBagWall);
		AuConstructButton* pbCustomWall = new AuConstructButton(this, m_FencesPage, AuScFence::Custom);

		QGridLayout* pageLay = new QGridLayout;
		pageLay->setMargin(0);
		pageLay->setSpacing(2);
		int row=0;
		int col=0;
		pageLay->addWidget(pbPosts, row,col++);
		pageLay->addWidget(pbWire, row,col++);
		pageLay->addWidget(pbRange, row,col++);
		pageLay->addWidget(pbSplitRail, row,col++);
		row++;
		col=0;
		pageLay->addWidget(pbPostsT, row,col++);
		pageLay->addWidget(pbWireT, row,col++);
		pageLay->addWidget(pbRangeT, row,col++);
		pageLay->addWidget(pbWoodYard, row,col++);
		row++;
		col=0;
		pageLay->addWidget(pbChainLink, row,col++);
		pageLay->addWidget(pbChainLinkRazorWire, row,col++);
		pageLay->addWidget(pbSandBagWall, row,col++);
		pageLay->addWidget(pbCustomWall, row,col++);
		row++;
		col=4;
		pageLay->setColumnStretch(col, 1);	// Make the last column stretch to push the buttons to the left.
		pageLay->setRowStretch(row, 1);	// Make the last row stretch to push the buttons to the top.
		m_FencesPage->setLayout(pageLay);
		m_Pages->addWidget(m_FencesPage);
	}

	// Superstructures
	{
		m_Superstructure = new AuIconButton(":data/bBiodome.png");
		m_Superstructure->setCheckable(true);
		m_Superstructure->setToolTip(tr("Superstructures"));
		connect(m_Superstructure, SIGNAL(clicked()), SLOT(SuperstructurePressed()));

		m_SuperstructurePage = new QWidget(this);
		AuConstructButton* pbBiodome = new AuConstructButton(this, m_SuperstructurePage, IID_Biodome);
		AuConstructButton* pbAerialPlatform = new AuConstructButton(this, m_SuperstructurePage, IID_AerialPlatform);
		AuConstructButton* pbGroundPlatform = new AuConstructButton(this, m_SuperstructurePage, IID_GroundPlatform);
		AuConstructButton* pbSeaPlatform = new AuConstructButton(this, m_SuperstructurePage, IID_SeaPlatform);

		QGridLayout* pageLay = new QGridLayout;
		pageLay->setMargin(0);
		pageLay->setSpacing(2);
		int row=0;
		int col=0;
		pageLay->addWidget(pbBiodome, row,col++);
		row++;
		pageLay->addWidget(pbAerialPlatform, row,col++);
		pageLay->addWidget(pbGroundPlatform, row,col++);
		pageLay->addWidget(pbSeaPlatform, row,col++);
		row++;
		col=4;
		pageLay->setColumnStretch(col, 1);	// Make the last column stretch to push the buttons to the left.
		pageLay->setRowStretch(row, 1);	// Make the last row stretch to push the buttons to the top.
		m_SuperstructurePage->setLayout(pageLay);
		m_Pages->addWidget(m_SuperstructurePage);
	}

	m_IndustryLabel = new QLabel(this);

	QLabel* lbStruc = new QLabel(tr("Blueprint"), this);

	m_Blueprints = new QComboBox(this);
	m_Blueprints->setToolTip(tr("<html>"
				"<p>"
				"Choose the building blueprint to use for new construction. "
				"</p>"
				"<p>"
				"The list contains building blueprints published freely in the exchange, "
				"plus building blueprints you have in your gear. "
				"Blueprints that cannot be built in the current environment are not listed, "
				"unless a biodome or enclosed platform exists on the world. "
				"</p>"
				"</html>"));
	connect(m_Blueprints, SIGNAL(currentIndexChanged(int)), SLOT(BlueprintChanged(int)));

	m_BlueprintInfo = new QLabel(this);
	m_BlueprintInfo->setToolTip(tr("<html>"
				"<p>"
				"Homes and jobs are determined by the building blueprint. "
				"</p>"
				"<p>"
				"Many industries can incorporate homes in their design. "
				"Each home houses one citizen. "
				"Large homes are more favorable than small homes. "
				"</p>"
				"<p>"
				"Manufacturing jobs are created by industries that make things. "
				"These employ the workers at the production lines that manufacture commodities. "
				"</p>"
				"<p>"
				"Professional jobs are created by industries that do not make things. "
				"These employ the essential workers that do the business of the industry, the ones who count for the industry. "
				"</p>"
				"<p>"
				"Service jobs are non-essential jobs created by the industry, such as the office required at a factory. "
				"Service jobs are also created by optional facilities in the building blueprint, "
				"such as arenas, auditoriums, lounges, stores, transporters and vehicle launch|recovery systems. "
				"</p>"
				"</html>"));

	m_BlueprintExchange = new AuIconButton(":data/Exchange.png", this);
	m_BlueprintExchange->setToolTip(tr("<html>"
				"Include blueprints from the exchange, appropriate to the type of building selected. "
				"Exchange blueprints are listed that are freely accessible to you. "
				"Blueprints that are not free must exist on media in your gear to be used. "
				"</html>"));
	m_BlueprintExchange->setCheckable(true);
	m_BlueprintExchange->setChecked(m_IncludeExchange);
	connect(m_BlueprintExchange, SIGNAL(clicked()), SLOT(BlueprintExchangePressed()));

	m_BlueprintFilter = new AuIconButton(":data/c_Filter.png", this);
	m_BlueprintFilter->setToolTip(tr("<html>"
				"Filters blueprints that are shown. "
				"</html>"));
	connect(m_BlueprintFilter, SIGNAL(clicked()), SLOT(BlueprintFilterPressed()));

	m_ManufLabel = new QLabel(this);
	m_ManufLabel->setWordWrap(true);
	m_ManufLabel->setToolTip(tr("<html>"
				"Commodities that can be manufactured by the building. "
				"Some buildings do not manufacture anything."
				"</html>"));

	m_ProductsLabel = new QLabel(this);
	m_ProductsLabel->setMinimumSize(QSize(1, 50));	// 3 rows of 16 plus 1 pixel between each row

	m_ckWorker = new QCheckBox(tr("Make Product"), this);
	m_ckWorker->setCheckState(Qt::Checked);
	m_ckWorker->setToolTip(tr("Enables the new building to employ workers to make products."));
	connect(m_ckWorker, SIGNAL(stateChanged(int)), SLOT(EnableWorker(int)));

	m_cbProducts = new QComboBox(this);
	m_cbProducts->setToolTip(tr("Choose the product to make.<br>"
				"When a product is selected, this tooltip will show the commodities required to manufacture it."));
	connect(m_cbProducts, SIGNAL(currentIndexChanged(int)), SLOT(ManufacturingProcessSelected(int)));

	QLabel* lbMaterial = new QLabel(this);
	lbMaterial->setText("Construction Materials");
	lbMaterial->setToolTip(tr("Commodities that are needed to construct the building.\n"
				"Some buildings do not need any materials to construct.\n"
				"The ToolTip on the picture below shows the amount needed."));

	m_MaterialsLabel = new QLabel(this);
	m_MaterialsLabel->setMinimumSize(QSize(1, 32));

	m_MatlLogs = new AuIconButton(":data/c_Wood.png", this);
	m_MatlLogs->setToolTip(tr("Build using rough logs."));
	m_MatlLogs->setCheckable(true);
	connect(m_MatlLogs, SIGNAL(clicked()), SLOT(LogsPressed()));

	m_MatlLumber = new AuIconButton(":data/c_Lumber.png", this);
	m_MatlLumber->setToolTip(tr("Build using milled lumber."));
	m_MatlLumber->setCheckable(true);
	connect(m_MatlLumber, SIGNAL(clicked()), SLOT(LumberPressed()));

	m_MatlStone = new AuIconButton(":data/c_Stone.png", this);
	m_MatlStone->setToolTip(tr("Build using stone."));
	m_MatlStone->setCheckable(true);
	m_MatlStone->setChecked(true);
	connect(m_MatlStone, SIGNAL(clicked()), SLOT(StonePressed()));

	m_MatlMetal = new AuIconButton(":data/c_Metal.png", this);
	m_MatlMetal->setToolTip(tr("Build using metal."));
	m_MatlMetal->setCheckable(true);
	connect(m_MatlMetal, SIGNAL(clicked()), SLOT(MetalPressed()));

	m_MatlMagmium = new AuIconButton(":data/c_Magmium.png", this);
	m_MatlMagmium->setToolTip(tr("Build using magmium."));
	m_MatlMagmium->setCheckable(true);
	connect(m_MatlMagmium, SIGNAL(clicked()), SLOT(MagmiumPressed()));

	m_MatlVulcium = new AuIconButton(":data/c_Vulcium.png", this);
	m_MatlVulcium->setToolTip(tr("Build using vulcium."));
	m_MatlVulcium->setCheckable(true);
	connect(m_MatlVulcium, SIGNAL(clicked()), SLOT(VulciumPressed()));

	m_MatlAdamantine = new AuIconButton(":data/c_Adamantine.png", this);
	m_MatlAdamantine->setToolTip(tr("Build using adamantine."));
	m_MatlAdamantine->setCheckable(true);
	connect(m_MatlAdamantine, SIGNAL(clicked()), SLOT(AdamantinePressed()));

	m_SkyLine = new AuSkyLineWidget(this);
	m_SkyLine->setToolTip(tr("<html>"
				"<p>"
				"This tool tip will show the number and volume of buildings present. "
				"It will also show the maximum volume of buildings supported by the world. "
				"All worlds are limited to <b>%1</b> buildings, including street lights and fences. "
				"The first 20 street lights and the first 20 fences are not counted. "
				"</p>"
				"</html>").arg(AuCityReport::WorldBuildingCountLimit()));

	m_Image = new QLabel(this);

	QHBoxLayout* mlay = new QHBoxLayout();
	mlay->addWidget(m_Roads);
	mlay->addWidget(m_Fences);
	mlay->addWidget(m_Superstructure);
	mlay->addStretch(1);
	mlay->addWidget(pbstop);
	mlay->addWidget(pbcode);
	mlay->addStretch(1);
	mlay->addWidget(pbclearcraters);
	mlay->addWidget(pbdestroy);
	mlay->addStretch(1);
	mlay->setMargin(0);
	mlay->setSpacing(2);

	QHBoxLayout* blay = new QHBoxLayout();
	blay->addWidget(m_Government);
	blay->addWidget(m_Food);
	blay->addWidget(m_Housing);
	blay->addWidget(m_Manufacture);
	blay->addWidget(m_Tools);
	blay->addWidget(m_Commerce);
	blay->addStretch(1);
	blay->addWidget(m_Power);
	blay->addWidget(m_Transport);
	blay->addStretch(1);
	blay->addWidget(m_Military);
	blay->addStretch(1);
	blay->setMargin(0);
	blay->setSpacing(2);

	QHBoxLayout* matlay = new QHBoxLayout();
	matlay->addWidget(m_MatlLogs);
	matlay->addWidget(m_MatlLumber);
	matlay->addWidget(m_MatlStone);
	matlay->addWidget(m_MatlMetal);
	matlay->addWidget(m_MatlMagmium);
	matlay->addWidget(m_MatlVulcium);
	matlay->addWidget(m_MatlAdamantine);
	matlay->addStretch();
	matlay->setMargin(0);
	matlay->setSpacing(2);

	QHBoxLayout* bplay = new QHBoxLayout();
	bplay->addWidget(m_Blueprints);
	bplay->addWidget(m_BlueprintExchange);
	bplay->addWidget(m_BlueprintFilter);
	bplay->setMargin(0);
	bplay->setSpacing(2);

	QHBoxLayout* rlay = new QHBoxLayout();
	rlay->addWidget(m_RoundAngle);
	rlay->addStretch();

	// Layout the widgets.
	QVBoxLayout* lay = new QVBoxLayout();
	lay->addLayout(mlay);
	lay->addLayout(blay);
	lay->addWidget(m_SkyLine);
	lay->addLayout(m_Pages);
	lay->addLayout(rlay);
	lay->addWidget(m_IndustryLabel);
	lay->addWidget(m_ManufLabel);
	lay->addWidget(m_ProductsLabel);
	lay->addWidget(m_ckWorker);
	lay->addWidget(m_cbProducts);
	lay->addWidget(lbStruc);
	lay->addLayout(bplay);
	lay->addWidget(m_BlueprintInfo);
	lay->addWidget(lbMaterial);
	lay->addWidget(m_MaterialsLabel);
	lay->addLayout(matlay);
	lay->addWidget(m_Image);
	lay->addStretch(10);
	lay->setMargin(2);
	lay->setSpacing(2);
	setLayout(lay);
}

AuConstructionWidget::~AuConstructionWidget()
{
}

void AuConstructionWidget::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	if(!m_ExchangeRequested)
	{
		// Post a request for exchange blueprints, so they are ready for us.
		auApp->BdExchangeRequestDesigns();
		m_ExchangeRequested = true;	// Do this only once.
	}

	// Create a timer to keep this page updated.
	if(!m_Timer)
	{
		m_Timer = new QTimer(this);
		connect(m_Timer, SIGNAL(timeout()), SLOT(UpdateContents()));
	}
	if(!m_Timer->isActive())
		m_Timer->start(5000);
	UpdateContents();
}

void AuConstructionWidget::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	// Kill the timer. It will be recreated during the next showEvent.
	if(m_Timer && m_Timer->isActive())
		m_Timer->stop();

	emit WindowHidden();
}

void AuConstructionWidget::UpdateContents()
{
	if(!isVisible())
	{
		// Kill the timer. It will be recreated during the next showEvent.
		if(m_Timer && m_Timer->isActive())
			m_Timer->stop();
		return;
	}

	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	if(!pFrame)
		return;
	AuDesignerView* pView = pFrame->DesignerView();
	if(!pView)
		return;
	quint32 empireId = pView->SceneAtomEmpire();
	int envType = pView->SceneAtomEnvType();
	quint32 bldLimit = pView->SceneAtomWorldBuildingVolumeLimit();
	quint32 bldCount = pView->SceneAtomWorldBuildingCount();
	quint32 fenceCount = pView->SceneAtomWorldFenceCount();
	quint32 streetLightCount = pView->SceneAtomWorldStreetLightCount();
	quint32 bldVol = pView->SceneAtomWorldBuildingVolume();
	m_SkyLine->SetVolumeLimit(bldLimit);
	m_SkyLine->SetBuildings(bldVol, bldCount, fenceCount, streetLightCount, envType);
	if(bldLimit)
	{
		if(!bldCount && !streetLightCount)
			m_SkyLine->setToolTip(tr("<html>"
						"<p>"
						"There are no buildings or street lights on the world. "
						"</p>"
						"<p>"
						"This world supports up to <b>%L3m%2</b> of buildings and street lights. "
						"All worlds are limited to <b>%6</b> buildings; "
						"the first 20 street lights and the first 20 fences do not count. "
						"</html>")
					.arg(AuGlobal::CubedSymbol())
					.arg(bldLimit)
					.arg(AuCityReport::WorldBuildingCountLimit()));
		else
			m_SkyLine->setToolTip(tr("<html>"
						"<p>"
						"There are %L4 buildings and %L5 street lights on the world, with total volume of <b>%L1m%2</b>. "
						"</p>"
						"<p>"
						"This world supports up to <b>%L3m%2</b> of buildings and street lights. "
						"All worlds are limited to <b>%6</b> buildings; "
						"the first 20 street lights and the first 20 fences do not count. "
						"</p>"
						"</html>")
					.arg(bldVol)
					.arg(AuGlobal::CubedSymbol())
					.arg(bldLimit)
					.arg(bldCount)
					.arg(streetLightCount)
					.arg(AuCityReport::WorldBuildingCountLimit()));
	}
	else
		m_SkyLine->setToolTip(tr("<html>"
					"<p>"
					"This tool tip will show the number and volume of buildings present. "
					"It will also show the maximum volume of buildings supported by the world. "
					"All worlds are limited to <b>%6</b> buildings; "
					"the first 20 street lights and the first 20 fences do not count. "
					"</p>"
					"</html>")
				.arg(AuCityReport::WorldBuildingCountLimit()));

	// Some buildings cannot be built without an empire.
	pbBank->setEnabled(empireId);
	pbCapitol->setEnabled(empireId);
	pbPoliceStation->setEnabled(empireId);
	m_Military->setEnabled(empireId);
	if(empireId)
		m_Military->setToolTip(tr("Military"));
	else
		m_Military->setToolTip(tr("Military buildings cannot be constructed without an empire. "
					"To create an empire, start by hand crafting a flag."));
}

void AuConstructionWidget::StopPressed()
{
	AuDesignerFrame* frame = auDesignerApp->DesignerFrame();
	AuDesignerView* view = frame->DesignerView();
	view->EndCommands();
}

void AuConstructionWidget::FencesPressed()
{
	m_Pages->setCurrentWidget(m_FencesPage);
	m_Commerce->setChecked(false);
	m_Fences->setChecked(true);
	m_Food->setChecked(false);
	m_Government->setChecked(false);
	m_Housing->setChecked(false);
	m_Manufacture->setChecked(false);
	m_Military->setChecked(false);
	m_Power->setChecked(false);
	m_Roads->setChecked(false);
	m_Superstructure->setChecked(false);
	m_Tools->setChecked(false);
	m_Transport->setChecked(false);
}

void AuConstructionWidget::FoodPressed()
{
	m_Pages->setCurrentWidget(m_FoodPage);
	m_Commerce->setChecked(false);
	m_Fences->setChecked(false);
	m_Food->setChecked(true);
	m_Government->setChecked(false);
	m_Housing->setChecked(false);
	m_Manufacture->setChecked(false);
	m_Military->setChecked(false);
	m_Power->setChecked(false);
	m_Roads->setChecked(false);
	m_Superstructure->setChecked(false);
	m_Tools->setChecked(false);
	m_Transport->setChecked(false);
}

void AuConstructionWidget::HousingPressed()
{
	m_Pages->setCurrentWidget(m_HousingPage);
	m_Commerce->setChecked(false);
	m_Fences->setChecked(false);
	m_Food->setChecked(false);
	m_Government->setChecked(false);
	m_Housing->setChecked(true);
	m_Manufacture->setChecked(false);
	m_Military->setChecked(false);
	m_Power->setChecked(false);
	m_Roads->setChecked(false);
	m_Superstructure->setChecked(false);
	m_Tools->setChecked(false);
	m_Transport->setChecked(false);
}

void AuConstructionWidget::GovernmentPressed()
{
	m_Pages->setCurrentWidget(m_GovernmentPage);
	m_Commerce->setChecked(false);
	m_Fences->setChecked(false);
	m_Food->setChecked(false);
	m_Government->setChecked(true);
	m_Housing->setChecked(false);
	m_Manufacture->setChecked(false);
	m_Military->setChecked(false);
	m_Power->setChecked(false);
	m_Roads->setChecked(false);
	m_Superstructure->setChecked(false);
	m_Tools->setChecked(false);
	m_Transport->setChecked(false);
}

void AuConstructionWidget::MilitaryPressed()
{
	m_Pages->setCurrentWidget(m_MilitaryPage);
	m_Commerce->setChecked(false);
	m_Fences->setChecked(false);
	m_Food->setChecked(false);
	m_Government->setChecked(false);
	m_Housing->setChecked(false);
	m_Manufacture->setChecked(false);
	m_Military->setChecked(true);
	m_Power->setChecked(false);
	m_Roads->setChecked(false);
	m_Superstructure->setChecked(false);
	m_Tools->setChecked(false);
	m_Transport->setChecked(false);
}

void AuConstructionWidget::PowerPressed()
{
	m_Pages->setCurrentWidget(m_PowerPage);
	m_Commerce->setChecked(false);
	m_Fences->setChecked(false);
	m_Food->setChecked(false);
	m_Government->setChecked(false);
	m_Housing->setChecked(false);
	m_Manufacture->setChecked(false);
	m_Military->setChecked(false);
	m_Power->setChecked(true);
	m_Roads->setChecked(false);
	m_Superstructure->setChecked(false);
	m_Tools->setChecked(false);
	m_Transport->setChecked(false);
}

void AuConstructionWidget::ManufacturePressed()
{
	m_Pages->setCurrentWidget(m_ManufacturePage);
	m_Commerce->setChecked(false);
	m_Fences->setChecked(false);
	m_Food->setChecked(false);
	m_Government->setChecked(false);
	m_Housing->setChecked(false);
	m_Manufacture->setChecked(true);
	m_Military->setChecked(false);
	m_Power->setChecked(false);
	m_Roads->setChecked(false);
	m_Superstructure->setChecked(false);
	m_Tools->setChecked(false);
	m_Transport->setChecked(false);
}

void AuConstructionWidget::ToolsPressed()
{
	m_Pages->setCurrentWidget(m_ToolsPage);
	m_Commerce->setChecked(false);
	m_Fences->setChecked(false);
	m_Food->setChecked(false);
	m_Government->setChecked(false);
	m_Housing->setChecked(false);
	m_Manufacture->setChecked(false);
	m_Military->setChecked(false);
	m_Power->setChecked(false);
	m_Roads->setChecked(false);
	m_Superstructure->setChecked(false);
	m_Tools->setChecked(true);
	m_Transport->setChecked(false);
}

void AuConstructionWidget::TransportPressed()
{
	m_Pages->setCurrentWidget(m_TransportPage);
	m_Commerce->setChecked(false);
	m_Fences->setChecked(false);
	m_Food->setChecked(false);
	m_Government->setChecked(false);
	m_Housing->setChecked(false);
	m_Manufacture->setChecked(false);
	m_Military->setChecked(false);
	m_Power->setChecked(false);
	m_Roads->setChecked(false);
	m_Superstructure->setChecked(false);
	m_Tools->setChecked(false);
	m_Transport->setChecked(true);
}

void AuConstructionWidget::CommercePressed()
{
	m_Pages->setCurrentWidget(m_CommercePage);
	m_Commerce->setChecked(true);
	m_Fences->setChecked(false);
	m_Food->setChecked(false);
	m_Government->setChecked(false);
	m_Housing->setChecked(false);
	m_Manufacture->setChecked(false);
	m_Military->setChecked(false);
	m_Power->setChecked(false);
	m_Roads->setChecked(false);
	m_Superstructure->setChecked(false);
	m_Tools->setChecked(false);
	m_Transport->setChecked(false);
}

void AuConstructionWidget::RoadsPressed()
{
	m_Pages->setCurrentWidget(m_RoadsPage);
	m_Commerce->setChecked(false);
	m_Fences->setChecked(false);
	m_Food->setChecked(false);
	m_Government->setChecked(false);
	m_Housing->setChecked(false);
	m_Manufacture->setChecked(false);
	m_Military->setChecked(false);
	m_Power->setChecked(false);
	m_Roads->setChecked(true);
	m_Superstructure->setChecked(false);
	m_Tools->setChecked(false);
	m_Transport->setChecked(false);
}

void AuConstructionWidget::SuperstructurePressed()
{
	m_Pages->setCurrentWidget(m_SuperstructurePage);
	m_Commerce->setChecked(false);
	m_Fences->setChecked(false);
	m_Food->setChecked(false);
	m_Government->setChecked(false);
	m_Housing->setChecked(false);
	m_Manufacture->setChecked(false);
	m_Military->setChecked(false);
	m_Power->setChecked(false);
	m_Roads->setChecked(false);
	m_Superstructure->setChecked(true);
	m_Tools->setChecked(false);
	m_Transport->setChecked(false);
}

void AuConstructionWidget::BlueprintExchangePressed()
{
	m_IncludeExchange = !m_IncludeExchange;
	m_BlueprintExchange->setChecked(m_IncludeExchange);
	ListBuilding(m_ListIndustryId, m_ListCPID);

	g_Au.Settings().writeEntry("BdConstruction/Exchange", m_IncludeExchange);
}

void AuConstructionWidget::BlueprintFilterPressed()
{
	AuBdExchangeFilter filter(this);
	if(filter.exec() != QDialog::Accepted)
		return;
	ListBuilding(m_ListIndustryId, m_ListCPID);
}

void AuConstructionWidget::UpdateRoadSettings()
{
	if(m_RoadType)
		RoadTypeChanged(m_RoadType->currentText());
}

void AuConstructionWidget::BuildingSelected(INDUSTRYID industryId, CONSTRUCTIONPROCESSID cpid)
{
	switch(cpid)
	{
	case CPID_Clearing:
		// Update global settings with what we see in the UI controls.
		UpdateRoadSettings();

		ListBuilding(industryId, cpid);
		emit PlanLandscapeClearing();
		break;

	case CPID_Defoliant:
		// Update global settings with what we see in the UI controls.
		UpdateRoadSettings();

		ListBuilding(industryId, cpid);
		emit PlanLandscapeDefoliate();
		break;

	case CPID_Irrigation:
		// Update global settings with what we see in the UI controls.
		UpdateRoadSettings();

		ListBuilding(industryId, cpid);
		emit PlanLandscapeIrrigate();
		break;

	case CPID_RoadGrade:
		// Update global settings with what we see in the UI controls.
		UpdateRoadSettings();

		ListBuilding(industryId, cpid);
		emit PlanRoadGrade();
		break;

	case CPID_DirtRoad:
		// Update global settings with what we see in the UI controls.
		UpdateRoadSettings();

		ListBuilding(industryId, cpid);
		emit PlanRoadDirt();
		break;

	case CPID_AsphaltRoad:
		// Update global settings with what we see in the UI controls.
		UpdateRoadSettings();

		ListBuilding(industryId, cpid);
		emit PlanRoadAsphalt();
		break;

	case CPID_ConcreteRoad:
		// Update global settings with what we see in the UI controls.
		UpdateRoadSettings();

		ListBuilding(industryId, cpid);
		emit PlanRoadConcrete();
		break;

	case CPID_FencePosts:
		ListBuilding(industryId, cpid);
		emit PlanFence(AuScFence::Posts, 0, CID_Invalid);
		break;
	case CPID_FencePostsT:
		ListBuilding(industryId, cpid);
		emit PlanFence(AuScFence::PostsT, 0, CID_Invalid);
		break;
	case CPID_FenceWire:
		ListBuilding(industryId, cpid);
		emit PlanFence(AuScFence::Wire, 0, CID_Invalid);
		break;
	case CPID_FenceWireT:
		ListBuilding(industryId, cpid);
		emit PlanFence(AuScFence::WireT, 0, CID_Invalid);
		break;
	case CPID_FenceRange:
		ListBuilding(industryId, cpid);
		emit PlanFence(AuScFence::Range, 0, CID_Invalid);
		break;
	case CPID_FenceRangeT:
		ListBuilding(industryId, cpid);
		emit PlanFence(AuScFence::RangeT, 0, CID_Invalid);
		break;
	case CPID_FenceChainLink:
		ListBuilding(industryId, cpid);
		emit PlanFence(AuScFence::ChainLink, 0, CID_Invalid);
		break;
	case CPID_FenceChainLinkRazorWire:
		ListBuilding(industryId, cpid);
		emit PlanFence(AuScFence::ChainLinkRazorWire, 0, CID_Invalid);
		break;
	case CPID_FenceSplitRail:
		ListBuilding(industryId, cpid);
		emit PlanFence(AuScFence::SplitRail, 0, CID_Invalid);
		break;
	case CPID_FenceWoodYard:
		ListBuilding(industryId, cpid);
		emit PlanFence(AuScFence::WoodYard, 0, CID_Invalid);
		break;
	case CPID_FenceSandBagWall:
		ListBuilding(industryId, cpid);
		emit PlanFence(AuScFence::SandBagWall, 0, CID_Invalid);
		break;
	case CPID_FenceBrickWall:
		ListBuilding(industryId, cpid);
		emit PlanFence(AuScFence::BrickWall, 0, CID_Invalid);
		break;
	case CPID_FenceStoneWall:
		ListBuilding(industryId, cpid);
		emit PlanFence(AuScFence::StoneWall, 0, CID_Invalid);
		break;

	case CPID_FenceCustom:
		{
			m_ListIndustryId = industryId;
			m_ListCPID = cpid;
			ListBuilding(m_ListIndustryId, m_ListCPID);

			// Insure there is a construction process for the current blueprint, with selected building material.
			AuBdReport orpt;
			if(!OriginalBuildingReport(orpt))
				return;

			// Fences do not manufacture anything.
			m_ckWorker->setCheckState(Qt::Unchecked);

			emit PlanFence(AuScFence::Custom, orpt.DesignId(), m_BuildMaterial);
		}
		break;

	default:
		{
			m_ListIndustryId = industryId;
			m_ListCPID = cpid;
			ListBuilding(m_ListIndustryId, m_ListCPID);

			// Insure there is a construction process for the current blueprint, with selected building material.
			AuBdReport orpt;
			if(!OriginalBuildingReport(orpt))
				return;

			// Use the current manufacturing process.
			quint16 mpid = m_cbProducts->itemData(m_cbProducts->currentIndex()).toUInt();
			const DgData& gd = g_Au.LockDataForRead();
			bool enableWorker = gd.DefaultWorkerEnabled(mpid);	// true if process worker is enabled by default.
			g_Au.UnlockData();

			// Set default state of process worker based on manufacturing process.
			m_ckWorker->setCheckState(enableWorker ? Qt::Checked : Qt::Unchecked);

			//qDebug() << "AuConstructionWidget::BuildingSelected" << industryId << "oproc" << oproc.DesignId();
			emit PlanBuilding(industryId, orpt, m_BuildMaterial, mpid, enableWorker);
		}
		break;
	}
}

void AuConstructionWidget::ListBuildingHover(INDUSTRYID industryId, CONSTRUCTIONPROCESSID conProcId)
{
	AuDesignerFrame* pFrame = auDesignerApp->DesignerFrame();
	if(!pFrame)
		return;
	AuDesignerView* pView = pFrame->DesignerView();
	if(!pView)
		return;
	if(pView->Command())
		return;

	m_ListIndustryId = industryId;
	m_ListCPID = conProcId;
	ListBuilding(m_ListIndustryId, m_ListCPID);
}


void AuConstructionWidget::ListBuilding(INDUSTRYID industryId, CONSTRUCTIONPROCESSID conProcId)
{
	if(industryId == IID_None)
		return;
	AuFrame* pFrame = auApp->Frame();
	if(!pFrame)
		return;
	AuView* pView = pFrame->View();
	if(!pView)
		return;

	bool terLand, terSeaBottom, intendedIndustry, mustBeRated;
	int minRating;
	QSet<AuId> excludePub;
	AuBdExchangeFilter::LoadFilter(excludePub, terLand, terSeaBottom, intendedIndustry, mustBeRated, minRating);

	m_BlueprintsEx.clear();
	m_BlueprintsGear.clear();

	// Harsh environment adds construction materials to all but roads, fences, and enclosed construction.
	ORBITZONE ozone = OZONE_Habitable;
	bool breathableAtmo = true;
	bool gasGiant = false;
	bool enclosurePresent = false;

	AuId sceneAtomId;
	DaPerson* person;
	pView->LockSceneAtom(&person);
	if(person)
	{
		sceneAtomId = person->Id();
		const DaWorldLL* planet = person->FindParentWorldInEnv();
		if(planet)
		{
			// Check the environment.
			ozone = planet->OrbitZone();
			breathableAtmo = planet->HasBreathableAtmosphere();
			gasGiant = planet->IsGasGiant();
			enclosurePresent = planet->HasEnclosure();

			// List the building design disks for 'industryId' the person is carrying.
			m_Patents = AuKnownPatents();				// Start with a fresh list.
			const AuGear* inv = person->LockGearForRead();
			ListDesigns(*inv, industryId, m_BlueprintsGear, excludePub, terLand, terSeaBottom, intendedIndustry, ozone, gasGiant,
					breathableAtmo, enclosurePresent);
			person->ListPatents(m_Patents);
			person->UnlockGear();

			// Get empire patents.
			if(person->EmpireId())
			{
				const DgData& gd = g_Au.LockDataForRead();
				DgEmpireHash::const_iterator ei = gd.m_gEmpires.find(person->EmpireId());
				if(ei != gd.m_gEmpires.cend())
					(*ei).second.ListIndustryPatents(m_Patents, industryId, gd);
				g_Au.UnlockData();
			}
		}
	}
	pView->UnlockSceneAtom();

	const DgData& gd = g_Au.LockDataForRead();
	const DbIndustry& dbi = gd.Industry(industryId);

	// List appropriate building designs published freely in the exchange.
	if(m_IncludeExchange)
	{
		QHash<quint32, AuBdDesignExchangeEntryClient>& designs = g_AuClient.LockBdDesignExchange();
		for(QHash<quint32, AuBdDesignExchangeEntryClient>::const_iterator di = designs.cbegin(); di != designs.cend(); di++)
		{
			if((*di).DesignState() != AuDesign::Final)
				continue;
			if((*di).Price() != 0 && (*di).DesignArchitectId() != sceneAtomId)
				continue;
			if((*di).IndustryCanUse(dbi) != 0)
				continue;
			if(!AuBdExchangeFilter::TestFilter(excludePub, terLand, terSeaBottom, intendedIndustry, industryId, ozone, gasGiant,
						breathableAtmo, enclosurePresent, mustBeRated, minRating, (*di).Report(), (*di).Rating()))
				continue;

			QString design = tr("%1 by %2").arg((*di).DesignName()).arg((*di).DesignArchitectName());
			m_BlueprintsEx.insert(pair<QString, AuBdReport>(design, (*di).Report()));
		}
		g_AuClient.UnlockBdDesignExchange();
	}

	// Industry
	QString industryName = dbi.m_name;
	g_Au.UnlockData();
	ListIndustryProcesses(industryId);

	// Building design selections.
	m_Blueprints->blockSignals(true);
	m_Blueprints->clear();
	if((industryId == IID_Fence && conProcId != CPID_FenceCustom) ||
			industryId == IID_Road ||
			industryId == IID_Landscape)
	{
		switch(conProcId)
		{
		case CPID_RoadGrade:
			m_IndustryLabel->setText(tr("<b>Grade Area</b>"));
			m_ManufLabel->setText(tr("Graded areas change the shape of existing terrain."));
			m_Blueprints->setToolTip(tr("Building blueprint not applicable to graded area."));
			break;
		case CPID_Irrigation:
			m_IndustryLabel->setText(tr("<b>Irrigate Area</b>"));
			m_ManufLabel->setText(tr("Irrigated areas grow local trees, shrubs and ground cover. "
						"Ground cover is mowed."));
			m_Blueprints->setToolTip(tr("Building blueprint not applicable to irrigated area."));
			break;
		case CPID_Clearing:
			m_IndustryLabel->setText(tr("<b>Clear Area</b>"));
			m_ManufLabel->setText(tr("Cleared areas remove trees and shrubs that are present. "
						"Any ground cover that is present is mowed."));
			m_Blueprints->setToolTip(tr("Building blueprint not applicable to cleared area."));
			break;
		case CPID_Defoliant:
			m_IndustryLabel->setText(tr("<b>Defoliate Area</b>"));
			m_ManufLabel->setText(tr("Defoliated areas remove trees, shrubs and ground cover that are present."));
			m_Blueprints->setToolTip(tr("Building blueprint not applicable to defoliated area."));
			break;
		case CPID_DirtRoad:
			m_IndustryLabel->setText(tr("<b>Dirt Road</b>"));
			m_ManufLabel->setText(tr("Roads interconnect buildings."));
			m_Blueprints->setToolTip(tr("Building blueprint not applicable to dirt road."));
			break;
		case CPID_AsphaltRoad:
			m_IndustryLabel->setText(tr("<b>Asphalt Road</b>"));
			m_ManufLabel->setText(tr("Roads interconnect buildings."));
			m_Blueprints->setToolTip(tr("Building blueprint not applicable to asphalt road."));
			break;
		case CPID_ConcreteRoad:
			m_IndustryLabel->setText(tr("<b>Concrete Road</b>"));
			m_ManufLabel->setText(tr("Roads interconnect buildings."));
			m_Blueprints->setToolTip(tr("Building blueprint not applicable to concrete road."));
			break;

		case CPID_FencePosts:
			m_IndustryLabel->setText(tr("<b>Wood Posts</b>"));
			m_ManufLabel->setText(tr("Posts block movement and gunfire."));
			m_Blueprints->setToolTip(tr("Building blueprint and construction material not applicable to built in fence styles."));
			break;
		case CPID_FencePostsT:
			m_IndustryLabel->setText(tr("<b>Tee Posts</b>"));
			m_ManufLabel->setText(tr("Posts block movement and gunfire."));
			m_Blueprints->setToolTip(tr("Building blueprint and construction material not applicable to built in fence styles."));
			break;
		case CPID_FenceWire:
			m_IndustryLabel->setText(tr("<b>Wire Fence on Wood Posts</b>"));
			m_ManufLabel->setText(tr("Wire fences block movement of large things. "
						"Movement of creatures is not blocked if their max health is 100hp or less. "
						"Gunfire is not blocked."));
			m_Blueprints->setToolTip(tr("Building blueprint and construction material not applicable to built in fence styles."));
			break;
		case CPID_FenceWireT:
			m_IndustryLabel->setText(tr("<b>Wire Fence on Tee Posts</b>"));
			m_ManufLabel->setText(tr("Wire fences block movement of large things. "
						"Movement of creatures is not blocked if their max health is 100hp or less. "
						"Gunfire is not blocked."));
			m_Blueprints->setToolTip(tr("Building blueprint and construction material not applicable to built in fence styles."));
			break;
		case CPID_FenceRange:
			m_IndustryLabel->setText(tr("<b>Range Fence on Wood Posts</b>"));
			m_ManufLabel->setText(tr("Range fencing blocks movement of most things. "
						"Movement of creatures is not blocked if their max health is 10hp or less. "
						"Gunfire is not blocked."));
			m_Blueprints->setToolTip(tr("Building blueprint and construction material not applicable to built in fence styles."));
			break;
		case CPID_FenceRangeT:
			m_IndustryLabel->setText(tr("<b>Range Fence on Tee Posts</b>"));
			m_ManufLabel->setText(tr("Range fencing blocks movement of most things. "
						"Movement of creatures is not blocked if their max health is 10hp or less. "
						"Gunfire is not blocked."));
			m_Blueprints->setToolTip(tr("Building blueprint and construction material not applicable to built in fence styles."));
			break;
		case CPID_FenceChainLink:
			m_IndustryLabel->setText(tr("<b>Chain Link Fence</b>"));
			m_ManufLabel->setText(tr("Chain link fences block movement. "
						"Gunfire is not blocked."));
			m_Blueprints->setToolTip(tr("Building blueprint and construction material not applicable to built in fence styles."));
			break;
		case CPID_FenceChainLinkRazorWire:
			m_IndustryLabel->setText(tr("<b>Chain Link Fence with Razor Wire</b>"));
			m_ManufLabel->setText(tr("Chain link fences block movement. "
						"Gunfire is not blocked."));
			m_Blueprints->setToolTip(tr("Building blueprint and construction material not applicable to built in fence styles."));
			break;
		case CPID_FenceSplitRail:
			m_IndustryLabel->setText(tr("<b>Split Rail Fence</b>"));
			m_ManufLabel->setText(tr("Split rail fences block movement of large things. "
						"Movement of creatures is not blocked if their max health is 100hp or less. "
						"Gunfire is not blocked."));
			m_Blueprints->setToolTip(tr("Building blueprint and construction material not applicable to built in fence styles."));
			break;
		case CPID_FenceWoodYard:
			m_IndustryLabel->setText(tr("<b>Stockade Fence</b>"));
			m_ManufLabel->setText(tr("Stockade fences block movement and gunfire."));
			m_Blueprints->setToolTip(tr("Building blueprint and construction material not applicable to built in fence styles."));
			break;
		case CPID_FenceSandBagWall:
			m_IndustryLabel->setText(tr("<b>Sand Bag Wall</b>"));
			m_ManufLabel->setText(tr("Sand bag walls block movement and gunfire."));
			m_Blueprints->setToolTip(tr("Building blueprint and construction material not applicable to built in wall styles."));
			break;
		case CPID_FenceBrickWall:
			m_IndustryLabel->setText(tr("<b>Brick Wall</b>"));
			m_ManufLabel->setText(tr("Walls block movement and gunfire."));
			m_Blueprints->setToolTip(tr("Building blueprint and construction material not applicable to built in wall styles."));
			break;
		case CPID_FenceStoneWall:
			m_IndustryLabel->setText(tr("<b>Stone Wall</b>"));
			m_ManufLabel->setText(tr("Walls block movement and gunfire."));
			m_Blueprints->setToolTip(tr("Building blueprint and construction material not applicable to built in wall styles."));
			break;
		case CPID_FenceCustom:
			m_IndustryLabel->setText(tr("<b>Custom Wall or Fence</b>"));
			m_ManufLabel->setText(tr("Custom walls and fences block movement and gunfire."));
			break;
		default:
			break;
		}
	}
	else
	{
		if(industryId == IID_Fence)
		{
			m_IndustryLabel->setText(tr("<b>Custom Wall or Fence</b>"));
			m_ManufLabel->setText(tr("Custom walls and fences require a blueprint. They may block movement and gunfire."));
		}
		else
		{
			m_IndustryLabel->setText(QString("<b>%1</b>").arg(industryName));
			m_ManufLabel->setText(dbi.DoesWhat());
		}
		quint32 prefDesignId = g_Au.Settings().readNumEntry(QString("BdConstruction/%1").arg(industryId), 0);

		// List designs the person is carrying.
		for(multimap<QString, AuBdReport>::const_iterator di = m_BlueprintsGear.begin(); di != m_BlueprintsGear.end(); di++)
		{
			QString itemText = QString("%1 %2").arg((*di).second.DesignId()).arg((*di).first);
			m_Blueprints->addItem(QPixmap(g_Au.CommodityBitmapName(CID_BdDesignDrawing)), itemText,
					QVariant((*di).second.DesignId()));
		}

		// List building designs that have been published in the exchange.
		for(multimap<QString, AuBdReport>::const_iterator di = m_BlueprintsEx.begin(); di != m_BlueprintsEx.end(); di++)
		{
			QString itemText = QString("%1 %2").arg((*di).second.DesignId()).arg((*di).first);
			m_Blueprints->addItem(QPixmap(":data/Exchange.png"), itemText, QVariant((*di).second.DesignId()));
		}

		m_Blueprints->setToolTip(tr("<html>Choose the %1 blueprint to use for new construction. "
					"The list contains %2 blueprints in the exchange plus %3 blueprints in your gear.</html>")
				.arg(industryName)
				.arg(m_BlueprintsEx.size())
				.arg(m_BlueprintsGear.size()));

		// Select the user's preferred design id for the building.
		int prefIdx = m_Blueprints->findData(prefDesignId);
		if(prefIdx >= 0)
			m_Blueprints->setCurrentIndex(prefIdx);
	}
	m_Blueprints->blockSignals(false);

	BlueprintChanged(m_Blueprints->currentIndex());
}

void AuConstructionWidget::ManufacturingProcessSelected(int iIndex)
{
	// Change tooltip to show the commodities required to manufacture the selected product.
	quint16 mpid = m_cbProducts->itemData(iIndex).toUInt();
	//qDebug() << "AuConstructionWidget::ManufacturingProcessSelected index" << iIndex << "mpid" << mpid;
	if(mpid)
		FillManufacturingProcessesCombo(mpid);
	emit PlanManufacturingProcess(mpid);
}

void AuConstructionWidget::SelectManufacturingProcess(quint16 mpid)
{
	// Change tooltip to show the commodities required to manufacture the selected product.
	int iIndex = m_cbProducts->findData(QVariant(mpid));
	//qDebug() << "AuConstructionWidget::SelectManufacturingProcess index" << iIndex << "mpid" << mpid;
	if(iIndex >= 0)
	{
		m_cbProducts->setCurrentIndex(iIndex);
		FillManufacturingProcessesCombo(mpid);
	}
	else
		m_cbProducts->setToolTip(tr("Choose the product to make.<br>"
					"When a product is selected, this tooltip will show the commodities required to manufacture it."));
}

void AuConstructionWidget::FillManufacturingProcessesCombo(quint16 defaultMpid)
{
	DbManufacturingProcess bMan;

	const DgData& gd = g_Au.LockDataForRead();
	if(gd.ManufacturingProcess(defaultMpid, bMan))
	{
		QMap<COMMODITYID, int> cids;	// value is count consumed, 0 for tools that are not directly consumed
		for(DbManufacturingComponentArray::const_iterator mpit = gd.m_bManufacturingComponentArray.cbegin();
				mpit != gd.m_bManufacturingComponentArray.cend(); mpit++)
		{
			if((*mpit).m_idmp == defaultMpid)
			{
				// Found component of the construction process.
				if((*mpit).IsRequired())
				{
					COMMODITYID cid = (COMMODITYID)(*mpit).m_idc;
					if(!cids.contains(cid))
						cids.insert(cid, (*mpit).m_consumed);	// consumes may be zero for needed items like tools
					else if((*mpit).m_consumed > 0)
						cids[cid] += (*mpit).m_consumed;
				}
			}
		}
		g_Au.UnlockData();

		QString matls;
		if(!cids.isEmpty())
		{
			matls = QString("<b>Makes %1 %2</b><br><i>Requires</i>")
				.arg(bMan.m_produces).arg(g_Au.CommodityName((COMMODITYID)bMan.m_idc));
			for(QMap<COMMODITYID,int>::const_iterator cit = cids.cbegin(); cit != cids.cend(); cit++)
			{
				// List commodity names.
				matls += "<br>" + g_Au.CommodityName(cit.key());
				if(cit.value() > 0)
				{
					if(cit.key() == CID_Money)
						matls += QString(" %L1%2 x quality level").arg(cit.value()).arg(g_Au.CurrencySymbol());
					else
						matls += QString(" %1").arg(cit.value());
				}
			}
		}
		else
			matls = QString("<b>Makes %1 %2</b>").arg(bMan.m_produces).arg(g_Au.CommodityName((COMMODITYID)bMan.m_idc));
		m_cbProducts->setToolTip(matls);
	}
	else
	{
		m_cbProducts->setToolTip("");
		g_Au.UnlockData();
	}
}

void AuConstructionWidget::ListDesigns(const AuGear& inv, INDUSTRYID industryId, multimap<QString, AuBdReport>& designs,
		const QSet<AuId>& excludePub, bool terLand, bool terSeaBottom, bool intendedIndustry, ORBITZONE ozone, bool gasGiant,
		bool breathableAtmo, bool enclosurePresent)
{
	if(industryId == IID_None)
		return;
	AuGearPosList allGearPos;
	inv.ListAll(allGearPos);
	const DgData& gd = g_Au.LockDataForRead();
	const DbIndustry& dbi = gd.Industry(industryId);
	for(AuGearPosList::const_iterator gi = allGearPos.cbegin(); gi != allGearPos.cend(); gi++)
	{
		const AuItemBdDesign* item = dynamic_cast<const AuItemBdDesign*>(inv.ItemAt(*gi));
		if(item)
		{
			if(item->Exchange().IndustryCanUse(dbi) != 0)
				continue;
			if(!AuBdExchangeFilter::TestFilter(excludePub, terLand, terSeaBottom, intendedIndustry, industryId, ozone, gasGiant,
						breathableAtmo, enclosurePresent, false, 0, item->Exchange().Report(), 0))
				continue;

			QString design = tr("%1 by %2").arg(item->Exchange().DesignName()).arg(item->Exchange().DesignArchitectName());
			designs.insert(pair<QString, AuBdReport>(design, item->Exchange().Report()));
			continue;
		}

		const AuItemStorageMedia* storage = dynamic_cast<const AuItemStorageMedia*>(inv.ItemAt(*gi));
		if(storage)
		{
			for(AuBdDesignExchange::const_iterator bi = storage->BlueprintsBd().cbegin();
					bi != storage->BlueprintsBd().cend(); bi++)
			{
				if((*bi).IndustryCanUse(dbi) != 0)
					continue;
				if(!AuBdExchangeFilter::TestFilter(excludePub, terLand, terSeaBottom, intendedIndustry, industryId, ozone, gasGiant,
							breathableAtmo, enclosurePresent, false, 0, (*bi).Report(), 0))
					continue;

				QString design = tr("%1 by %2").arg((*bi).DesignName()).arg((*bi).DesignArchitectName());
				designs.insert(pair<QString, AuBdReport>(design, (*bi).Report()));
			}
			continue;
		}
	}
	g_Au.UnlockData();
}

void AuConstructionWidget::BlueprintChanged(int index)
{
	if(index < 0)
	{
		m_Image->setPixmap(QPixmap());
		m_ImageDesign = 0;
		ChangeMaterial();
		return;
	}

	quint32 designId = m_Blueprints->itemData(index).toUInt();
	if(!designId)
	{
		m_Image->setPixmap(QPixmap());
		m_ImageDesign = 0;
		ChangeMaterial();
		return;
	}

	// We can show an image if the design is in the exchange.
	QHash<quint32, AuBdDesignExchangeEntryClient>& designs = g_AuClient.LockBdDesignExchange();
	QHash<quint32, AuBdDesignExchangeEntryClient>::const_iterator di = designs.find(designId);
	if(di != designs.cend())
	{
		m_Image->setPixmap(QPixmap::fromImage((*di).Thumb()));
		m_ImageDesign = designId;
	}
	else
	{
		m_Image->setPixmap(QPixmap(":data/BdNoImage.png"));
		m_ImageDesign = 0;
	}
	g_AuClient.UnlockBdDesignExchange();

	ChangeMaterial();
	if(m_ListIndustryId == IID_Mine)
		ListIndustryProcesses(m_ListIndustryId);	// Update process list, because quarries change what a mine can make.

	// Insure there is a construction process for the current blueprint, with selected building material.
	AuBdReport orpt;
	if(!OriginalBuildingReport(orpt))
		return;

	emit PlanConstructionProcess(orpt);

	// Store last design id selected for a given industry.
	g_Au.Settings().writeEntry(QString("BdConstruction/%1").arg(m_ListIndustryId), designId);
}

void AuConstructionWidget::ListIndustryProcesses(INDUSTRYID industryId)
{
	QString prods;
	int wid = 14 * 17 - 1;			// Keep the size the same all the time. This may be triggering a Qt bug.
	int ht = 3 * 17 - 1;			// Keep the size the same all the time. This may be triggering a Qt bug.
	QImage im(wid,ht,QImage::Format_ARGB32);
	m_cbProducts->blockSignals(true);
	m_cbProducts->clear();
	QPainter p(&im);
	p.setCompositionMode(QPainter::CompositionMode_Clear);
	p.fillRect(0,0,wid,ht, QBrush(Qt::transparent));
	{
		QVector<COMMODITYID> cids;
		int defmpid = 0;
		const DgData& gd = g_Au.LockDataForRead();
		for(DbManufacturingProcessArray::const_iterator pit = gd.m_bManufacturingProcessArray.cbegin();
				pit != gd.m_bManufacturingProcessArray.cend(); pit++)
		{
			if((*pit).m_idi != industryId)
				continue;
			COMMODITYID cid = (COMMODITYID)(*pit).m_idc;
			if(m_IsQuarry && cid != CID_Stone)
				continue;	// Quarry can only make stone.
			// Do not list patented processes unless the patent is available in the city.
			if(!gd.m_bManufacturingComponentArray.HasNecessaryPatents((*pit).m_id, m_Patents))
				continue;
			if(!cids.contains(cid))
				cids.append(cid);

			m_cbProducts->addItem(QPixmap(g_Au.CommodityBitmapName(cid)), (*pit).m_name, (*pit).m_id);

			if(!defmpid || (*pit).m_isDefault)
				defmpid = (*pit).m_id;
		}
		g_Au.UnlockData();
		if(!cids.isEmpty() || industryId == IID_SpacecraftFactory)
		{
			// Split into multiple rows otherwise the bitmap gets too long and forces the widget to get wider.
			p.setCompositionMode(QPainter::CompositionMode_SourceOver);
			int xpos = 0;
			int ypos = 0;
			bool patentShown = false;
			for(QVector<COMMODITYID>::const_iterator cit = cids.cbegin(); cit != cids.cend(); cit++)
			{
				// List commodity names.
				if(prods.isEmpty())
					prods = g_Au.CommodityName(*cit);
				else
					prods += "<br>" + g_Au.CommodityName(*cit);

				// Only one patent icon is needed.
				if(DbCommodity::IsPatent(*cit))
				{
					if(patentShown)
						continue;
					patentShown = true;
				}
				QImage prod(g_Au.CommodityBitmapName(*cit));
				QRect rc(xpos,ypos, 16,16);
				p.drawImage(rc, prod);
				xpos += 17;
				if(xpos >= wid)
				{
					// Move down to beginning of next row.
					xpos = 0;
					ypos += 17;
				}
			}
			if(industryId == IID_SpacecraftFactory)
			{
				// List commodity names.
				if(prods.isEmpty())
					prods = QObject::tr("Spacecraft");
				else
					prods += "<br>" + QObject::tr("Spacecraft");

				QImage prod(":data/c_Spacecraft.png");
				QRect rc(xpos,ypos, 16,16);
				p.drawImage(rc, prod);
				xpos += 17;
				if(xpos >= wid)
				{
					// Move down to beginning of next row.
					xpos = 0;
					ypos += 17;
				}
			}

			// Set tooltip appropriately.
			SelectManufacturingProcess(defmpid);
		}
		else
		{
			// Create a tool tip for those industries that don't manufacture anything.
			const DgData& gd = g_Au.LockDataForRead();
			m_cbProducts->setToolTip(tr("%1 does not make anything.").arg(gd.IndustryName(industryId)));
			g_Au.UnlockData();
		}
	}
	m_cbProducts->blockSignals(false);
	if(prods.isEmpty())
	{
		m_ProductsLabel->setPixmap(QPixmap());
		m_ProductsLabel->setToolTip(tr("This building does not make things."));
	}
	else
	{
		QPixmap products = QPixmap::fromImage(im);
		m_ProductsLabel->setPixmap(products);
		m_ProductsLabel->setToolTip(prods);
	}
}

bool AuConstructionWidget::CurrentBuildingReport(AuBdReport& report) const
{
	if(!OriginalBuildingReport(report))
		return false;
	report.SetConstructionMaterial(m_BuildMaterial);
	return true;
}

bool AuConstructionWidget::OriginalBuildingReport(AuBdReport& report) const
{
	quint32 designId = m_Blueprints->itemData(m_Blueprints->currentIndex()).toUInt();
	if(!designId)
		return false;

	// Locate the original report for the current blueprint.
	for(multimap<QString, AuBdReport>::const_iterator di = m_BlueprintsGear.begin(); di != m_BlueprintsGear.end(); di++)
	{
		if((*di).second.DesignId() == designId)
		{
			report = (*di).second;
			return true;
		}
	}
	for(multimap<QString, AuBdReport>::const_iterator di = m_BlueprintsEx.begin(); di != m_BlueprintsEx.end(); di++)
	{
		if((*di).second.DesignId() == designId)
		{
			report = (*di).second;
			return true;
		}
	}
	return false;
}

void AuConstructionWidget::ChangeMaterial()
{
	// Locate the construction process for the current blueprint.
	AuBdReport report;

	// List required commodities as icons.
	QPixmap matl;
	QString matls;
	QString homesJobs;
	int wid = 14 * 17;	// Allow 1 pixel spacing.
	int ht = 32;
	QImage im(wid,ht,QImage::Format_ARGB32);
	QPainter p(&im);
	p.setCompositionMode(QPainter::CompositionMode_Clear);
	p.fillRect(0,0,wid,ht, QBrush(Qt::transparent));
	if(CurrentBuildingReport(report))
	{
		// Show homes and jobs provided by the building when used for the industry.
		quint32 homesLarge = 0;
		quint32 homesMedium = 0;
		quint32 homesSmall = 0;
		quint32 manufacturingJobs = 0;
		quint32 serviceJobs = 0;
		quint32 stores = 0;
		quint32 lounges = 0;
		quint32 proJobs = 0;
		m_IsQuarry = m_ListIndustryId == IID_Mine && !report.HasShop();
		if(m_ListIndustryId != IID_None)
		{
			const DgData& gd = g_Au.LockDataForRead();
			const DbIndustry& dbi = gd.Industry(m_ListIndustryId);
			report.IndustryInfo(dbi, homesLarge, homesMedium, homesSmall, manufacturingJobs, serviceJobs, stores, lounges, proJobs);
			g_Au.UnlockData();
		}
		if(homesLarge || homesMedium || homesSmall)
		{
			if(homesLarge && homesMedium && homesSmall)
				homesJobs += tr("Homes <b>%L1 Large %L2 Medium %L3 Small</b>").arg(homesLarge).arg(homesMedium).arg(homesSmall);
			else if(homesLarge && homesMedium)
				homesJobs += tr("Homes <b>%L1 Large %L2 Medium</b>").arg(homesLarge).arg(homesMedium);
			else if(homesLarge && homesSmall)
				homesJobs += tr("Homes <b>%L1 Large %L2 Small</b>").arg(homesLarge).arg(homesSmall);
			else if(homesMedium && homesSmall)
				homesJobs += tr("Homes <b>%L1 Medium %L2 Small</b>").arg(homesMedium).arg(homesSmall);
			else if(homesLarge)
				homesJobs += tr("Homes <b>%L1 Large</b>").arg(homesLarge);
			else if(homesMedium)
				homesJobs += tr("Homes <b>%L1 Medium</b>").arg(homesMedium);
			else
				homesJobs += tr("Homes <b>%L1 Small</b>").arg(homesSmall);
		}
		else
			homesJobs += tr("Homes <b>None</b>");
		homesJobs += "<br>";
		if(manufacturingJobs)
		{
			if(serviceJobs)
				homesJobs += tr("Jobs <b>%L1 Manufacturing %L2 Service</b>").arg(manufacturingJobs).arg(serviceJobs);
			else
				homesJobs += tr("Jobs <b>%L1 Manufacturing</b>").arg(manufacturingJobs);
		}
		else if(proJobs)
		{
			if(serviceJobs)
				homesJobs += tr("Jobs <b>%L1 Professional %L2 Service</b>").arg(proJobs).arg(serviceJobs);
			else
				homesJobs += tr("Jobs <b>%L1 Professional</b>").arg(proJobs);
		}
		else if(serviceJobs)
			homesJobs += tr("Jobs <b>%L1 Service</b>").arg(serviceJobs);
		else
			homesJobs += tr("Jobs <b>None</b>");

		QMap<COMMODITYID, int> cids;	// value is count consumed, 0 for tools that are not directly consumed
		DeConstructionProcess cproc = report.ConstructionProcess();
		cproc.ChangeMaterial(CID_Lumber, m_BuildMaterial);
		for(DeProcessComponentArray::const_iterator cpit = cproc.Components().cbegin(); cpit != cproc.Components().cend(); cpit++)
		{
			// Found component of the construction process.
			if((*cpit).IsRequired())
			{
				COMMODITYID cid = (*cpit).CommodityId();
				if(!cids.contains(cid))
					cids.insert(cid, (*cpit).QtyConsumed());	// consumes may be zero for needed items like tools
				else if((*cpit).QtyConsumed() > 0)
					cids[cid] += (*cpit).QtyConsumed();
			}
		}

		if(!cids.isEmpty())
		{
			p.setCompositionMode(QPainter::CompositionMode_SourceOver);
			int xpos = 0;
			int ypos = 0;
			for(QMap<COMMODITYID,int>::const_iterator cit = cids.cbegin(); cit != cids.cend(); cit++)
			{
				COMMODITYID cid = cit.key();
				QImage prod(g_Au.CommodityBitmapName(cid));
				QRect rc(xpos,ypos, 16,16);
				p.drawImage(rc, prod);
				xpos += 17;
				if(xpos >= wid)
				{
					// Move down to beginning of next row.
					xpos = 0;
					ypos += 17;
				}

				// List commodity names.
				if(matls.isEmpty())
					matls = g_Au.CommodityName(cid);
				else
					matls += "\n" + g_Au.CommodityName(cid);
				if(cit.value() > 0)
					matls += QString(" %1").arg(cit.value());
			}
		}
	}
	matl = QPixmap::fromImage(im);
	m_MaterialsLabel->setPixmap(matl);
	m_MaterialsLabel->setToolTip(matls);
	m_BlueprintInfo->setText(homesJobs);
}

void AuConstructionWidget::LogsPressed()
{
	m_BuildMaterial = CID_Wood;
	emit PlanBuildingMaterial(m_BuildMaterial);

	m_MatlLogs->setChecked(true);
	m_MatlLumber->setChecked(false);
	m_MatlStone->setChecked(false);
	m_MatlMetal->setChecked(false);
	m_MatlMagmium->setChecked(false);
	m_MatlVulcium->setChecked(false);
	m_MatlAdamantine->setChecked(false);
	ChangeMaterial();
}

void AuConstructionWidget::LumberPressed()
{
	m_BuildMaterial = CID_Lumber;
	emit PlanBuildingMaterial(m_BuildMaterial);

	m_MatlLogs->setChecked(false);
	m_MatlLumber->setChecked(true);
	m_MatlStone->setChecked(false);
	m_MatlMetal->setChecked(false);
	m_MatlMagmium->setChecked(false);
	m_MatlVulcium->setChecked(false);
	m_MatlAdamantine->setChecked(false);
	ChangeMaterial();
}

void AuConstructionWidget::StonePressed()
{
	m_BuildMaterial = CID_Stone;
	emit PlanBuildingMaterial(m_BuildMaterial);

	m_MatlLogs->setChecked(false);
	m_MatlLumber->setChecked(false);
	m_MatlStone->setChecked(true);
	m_MatlMetal->setChecked(false);
	m_MatlMagmium->setChecked(false);
	m_MatlVulcium->setChecked(false);
	m_MatlAdamantine->setChecked(false);
	ChangeMaterial();
}

void AuConstructionWidget::MetalPressed()
{
	m_BuildMaterial = CID_Metal;
	emit PlanBuildingMaterial(m_BuildMaterial);

	m_MatlLogs->setChecked(false);
	m_MatlLumber->setChecked(false);
	m_MatlStone->setChecked(false);
	m_MatlMetal->setChecked(true);
	m_MatlMagmium->setChecked(false);
	m_MatlVulcium->setChecked(false);
	m_MatlAdamantine->setChecked(false);
	ChangeMaterial();
}

void AuConstructionWidget::MagmiumPressed()
{
	m_BuildMaterial = CID_Magmium;
	emit PlanBuildingMaterial(m_BuildMaterial);

	m_MatlLogs->setChecked(false);
	m_MatlLumber->setChecked(false);
	m_MatlStone->setChecked(false);
	m_MatlMetal->setChecked(false);
	m_MatlMagmium->setChecked(true);
	m_MatlVulcium->setChecked(false);
	m_MatlAdamantine->setChecked(false);
	ChangeMaterial();
}

void AuConstructionWidget::VulciumPressed()
{
	m_BuildMaterial = CID_Vulcium;
	emit PlanBuildingMaterial(m_BuildMaterial);

	m_MatlLogs->setChecked(false);
	m_MatlLumber->setChecked(false);
	m_MatlStone->setChecked(false);
	m_MatlMetal->setChecked(false);
	m_MatlMagmium->setChecked(false);
	m_MatlVulcium->setChecked(true);
	m_MatlAdamantine->setChecked(false);
	ChangeMaterial();
}

void AuConstructionWidget::AdamantinePressed()
{
	m_BuildMaterial = CID_Adamantine;
	emit PlanBuildingMaterial(m_BuildMaterial);

	m_MatlLogs->setChecked(false);
	m_MatlLumber->setChecked(false);
	m_MatlStone->setChecked(false);
	m_MatlMetal->setChecked(false);
	m_MatlMagmium->setChecked(false);
	m_MatlVulcium->setChecked(false);
	m_MatlAdamantine->setChecked(true);
	ChangeMaterial();
}

void AuConstructionWidget::ClearCratersPressed()
{
	AuDesignerFrame* frame = auDesignerApp->DesignerFrame();
	AuDesignerView* view = frame->DesignerView();
	view->PlanClearCraters();
}

void AuConstructionWidget::DestroyPressed()
{
	AuDesignerFrame* frame = auDesignerApp->DesignerFrame();
	AuDesignerView* view = frame->DesignerView();
	view->PlanClearDevelopments();
}

void AuConstructionWidget::RoadTypeChanged(const QString& text)
{
	//qDebug() << "AuTabBuild::RoadTypeChanged type" << text;
	if(text == tr("Level Grade"))
	{
		g_AuClient.SetRoadType(1);
		m_RoadWidth->setEditText(QString("%1").arg(g_AuClient.SlabWidthInMeters()));
	}
	else
	{
		g_AuClient.SetRoadType(0);
		m_RoadWidth->setEditText(QString("%1").arg(g_AuClient.RoadWidthInMeters()));
	}
}

void AuConstructionWidget::RoadWidthChanged(const QString& text)
{
	QVariant var(text);
	if(g_AuClient.RoadType() == 0)
		g_AuClient.SetRoadWidth(var.toFloat());
	else
		g_AuClient.SetSlabWidth(var.toFloat());
}

// This responds to a change to the check box.
void AuConstructionWidget::EnableWorker(int enableWorker)
{
	if(enableWorker == Qt::Unchecked)
		m_cbProducts->setEnabled(false);
	else
		m_cbProducts->setEnabled(true);

	emit PlanManufacturingWorkerOk(enableWorker?true:false);
}

void AuConstructionWidget::RoundingAngleChanged(const QString& text)
{
	QVariant var(text);
	g_AuClient.SetRoundingAngle(var.toDouble());
}
