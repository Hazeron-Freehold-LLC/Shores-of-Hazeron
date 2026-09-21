//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: audnasettings.cpp,v 1.94 2022/05/19 15:09:14 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "audnasettings.h"
//
#include "audna.h"
#include "audnawidgets.h"
#include "audnawizard.h"
#include "auglobal.h"
#include "autoolbutton.h"
#include "audnaview.h"
#include <QComboBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QScrollArea>
#include <QSplitter>
#include <QStackedLayout>
#include <QVBoxLayout>

AuDNASettings::AuDNASettings(QWidget* parent) : QWidget(parent)
{
	AuHairStyle::STYLE lastHaircutA =
		(AuHairStyle::STYLE)g_Au.Settings().readNumEntry("DNASettings/HaircutA", AuHairStyle::Unkempt);
	AuHairStyle::STYLE lastHaircutB =
		(AuHairStyle::STYLE)g_Au.Settings().readNumEntry("DNASettings/HaircutB", AuHairStyle::Unkempt);
	AuHairStyle::SHAVE lastShaveA =
		(AuHairStyle::SHAVE)g_Au.Settings().readNumEntry("DNASettings/ShaveA", AuHairStyle::Unshaven);
	AuHairStyle::SHAVE lastShaveB =
		(AuHairStyle::SHAVE)g_Au.Settings().readNumEntry("DNASettings/ShaveB", AuHairStyle::Unshaven);
	COMMODITYID lastAArmor = (COMMODITYID)g_Au.Settings().readNumEntry("DNASettings/ArmorA", CID_Invalid);
	COMMODITYID lastAClothes = (COMMODITYID)g_Au.Settings().readNumEntry("DNASettings/ClothesA", CID_Invalid);
	COMMODITYID lastAHat = (COMMODITYID)g_Au.Settings().readNumEntry("DNASettings/HatA", CID_Invalid);
	COMMODITYID lastBArmor = (COMMODITYID)g_Au.Settings().readNumEntry("DNASettings/ArmorB", CID_Invalid);
	COMMODITYID lastBClothes = (COMMODITYID)g_Au.Settings().readNumEntry("DNASettings/ClothesB", CID_Invalid);
	COMMODITYID lastBHat = (COMMODITYID)g_Au.Settings().readNumEntry("DNASettings/HatB", CID_Invalid);
	int lastGround = g_Au.Settings().readNumEntry("DNASettings/Ground", 0);
	AuPose::POSE lastPoseA = (AuPose::POSE)g_Au.Settings().readNumEntry("DNASettings/PoseA", AuPose::Stand);
	AuPose::POSE lastMouthA = (AuPose::POSE)g_Au.Settings().readNumEntry("DNASettings/MouthA", AuPose::MouthClosed);
	AuPose::POSE lastPoseB = (AuPose::POSE)g_Au.Settings().readNumEntry("DNASettings/PoseB", AuPose::Stand);
	AuPose::POSE lastMouthB = (AuPose::POSE)g_Au.Settings().readNumEntry("DNASettings/MouthB", AuPose::MouthClosed);

	m_View = new AuDNAView(this);

	m_ShowGround = new QComboBox(this);
	m_ShowGround->addItem(tr("No Ground"), 0);
	m_ShowGround->addItem(tr("Floor Grid"), 5);
	m_ShowGround->addItem(tr("Wall Grid"), 6);
	m_ShowGround->addItem(tr("Both Grids"), 7);
	m_ShowGround->addItem(tr("Dirt"), 1);
	m_ShowGround->addItem(tr("Grass"), 2);
	m_ShowGround->addItem(tr("Concrete"), 3);
	m_ShowGround->addItem(tr("Tarmac"), 4);
	m_ShowGround->setToolTip(tr("<html>"
				"Selects reference graphic."
				"</html>"));
	connect(m_ShowGround, SIGNAL(currentIndexChanged(int)), SLOT(SetShowGround(int)));
	m_ShowGround->setCurrentIndex(m_ShowGround->findData(lastGround));

	QToolButton* resetview = new QToolButton(this);
	resetview->setIcon(QIcon(":data/MsgSolicitSales.png"));
	resetview->setToolTip(tr("<html>"
				"<b>Reset View</b>"
				"<br>"
				"Restores the default camera view. "
				"</html>"));
	connect(resetview, SIGNAL(clicked()), m_View, SLOT(ResetView()));

	QToolButton* nudna = new QToolButton(this);
	nudna->setIcon(QIcon(":data/new12x12.png"));
	nudna->setToolTip(tr("<html>"
				"<b>New DNA...</b>"
				"<br>"
				"Removes all chromosomes. "
				"</html>"));
	connect(nudna, SIGNAL(clicked()), SLOT(NewDNA()));

	QToolButton* savedna = new QToolButton(this);
	savedna->setIcon(QIcon(":data/dp_Save.png"));
	savedna->setToolTip(tr("<html>"
				"<b>Save DNA...</b>"
				"<br>"
				"Saves DNA to a file. "
				"</html>"));
	connect(savedna, SIGNAL(clicked()), SLOT(SaveDNA()));

	QToolButton* loaddna = new QToolButton(this);
	loaddna->setIcon(QIcon(":data/dp_Load.png"));
	loaddna->setToolTip(tr("<html>"
				"<b>Load DNA...</b>"
				"<br>"
				"loads DNA from a file. "
				"</html>"));
	connect(loaddna, SIGNAL(clicked()), SLOT(LoadDNA()));

	QToolButton* human = new QToolButton(this);
	human->setIcon(QIcon(":data/Passenger.png"));
	human->setToolTip(tr("<html>"
				"<b>Human DNA...</b>"
				"<br>"
				"Loads chromosomes of a typical human. "
				"</html>"));
	connect(human, SIGNAL(clicked()), SLOT(Human()));

	QToolButton* wizard = new QToolButton(this);
	wizard->setIcon(QIcon(":data/DNA.png"));
	wizard->setToolTip(tr("<html>"
				"<b>DNA Wizard...</b>"
				"<br>"
				"Creates new DNA using a back/next series of windows. "
				"</html>"));
	connect(wizard, SIGNAL(clicked()), SLOT(Wizard()));

	QToolButton* random = new QToolButton(this);
	random->setIcon(QIcon(":data/Game.png"));
	random->setToolTip(tr("<html>"
				"<b>Random...</b>"
				"<br>"
				"Creates random DNA. "
				"<br><b>Unfinished feature disabled.</b>"	//DHH animalia
				"</html>"));
	random->setEnabled(false);	//DHH animalia
	connect(random, SIGNAL(clicked()), SLOT(Random()));

	QToolButton* addChromosome = new QToolButton(this);
	addChromosome->setIcon(QIcon(":data/Plus.png"));
	addChromosome->setToolTip(tr("<html>"
				"<b>Add Chromosome...</b>"
				"<br>"
				"Adds a chromosome. "
				"Properties will be copied from the highest numbered chromosome of the same kind, if one already exists. "
				"<br><b>Unfinished feature disabled.</b>"	//DHH animalia
				"</html>"));
	addChromosome->setEnabled(false);	//DHH animalia
	connect(addChromosome, SIGNAL(clicked()), SLOT(AddChromosome()));

	QLabel* lbAlpha = new QLabel(tr("<b>α Chromatid</b>"), this);
	lbAlpha->setAlignment(Qt::AlignHCenter|Qt::AlignTop);

	QLabel* lbBeta = new QLabel(tr("<b>β Chromatid</b>"), this);
	lbBeta->setAlignment(Qt::AlignHCenter|Qt::AlignTop);

	m_Pages = new QStackedLayout;
	m_Pages->setSpacing(0);
	m_Pages->setMargin(0);
	m_Pages->addWidget(new AuChromoPageBlank);	// Start with a blank page, for when nothing is selected.
	for(int t=0; t<AuChromosome::CTYPE_Count; t++)
	{
		AuChromosome::CTYPE tr = (AuChromosome::CTYPE)t;
		AuChromosome* chromosome = AuChromosome::CreateChromosome(tr);
		QWidget* p = chromosome->MakePropertiesPage(this);
		if(p)
		{
			m_PageIndexes.insert(tr, m_Pages->addWidget(p));
			connect(p, SIGNAL(Apply()), SLOT(ApplyChromosomeChanges()));
			connect(p, SIGNAL(ApplyAll()), SLOT(ApplyChromosomeChangesToAll()));
			connect(p, SIGNAL(ApplyPair()), SLOT(ApplyChromosomeChangesToPair()));
		}
		delete chromosome;
	}

	QMap<QString, AuHairStyle::STYLE> hairStyles;
	for(int s=AuHairStyle::Unkempt+1; s<AuHairStyle::STYLE_Count; s++)
	{
		AuHairStyle::STYLE style = (AuHairStyle::STYLE)s;
		hairStyles.insert(AuHairStyle::Name(style), style);
	}
	QMap<QString, AuHairStyle::SHAVE> shaveStyles;
	for(int s=AuHairStyle::Unshaven+1; s<AuHairStyle::SHAVE_Count; s++)
	{
		AuHairStyle::SHAVE shave = (AuHairStyle::SHAVE)s;
		shaveStyles.insert(AuHairStyle::ShaveName(shave), shave);
	}

	m_AHaircut = new QComboBox;
	m_AHaircut->setToolTip(tr("<html>"
				"<p>"
				"Hair style for α. "
				"This does not affect the DNA. "
				"</p>"
				"<p>"
				"Hair styles affect head hair only. "
				"Hair on other body parts is not styled. "
				"</p>"
				"<p>"
				"<i>Buzz</i>, <i>Close</i>, <i>Straight Out</i>, and <i>Unkempt</i> show every follicle. "
				"Other styles may hide hair. "
				"</p>"
				"</html>"));
	for(int s=0; s<=AuHairStyle::Unkempt; s++)
		m_AHaircut->addItem(AuHairStyle::Name((AuHairStyle::STYLE)s), s);
	for(QMap<QString, AuHairStyle::STYLE>::const_iterator hi = hairStyles.cbegin(); hi != hairStyles.cend(); hi++)
		m_AHaircut->addItem(hi.key(), *hi);
	m_AHaircut->setCurrentIndex(m_AHaircut->findData(lastHaircutA));
	connect(m_AHaircut, SIGNAL(currentIndexChanged(int)), SLOT(AHaircutChanged(int)));

	m_AShave = new QComboBox;
	m_AShave->setToolTip(tr("<html>"
				"<p>"
				"Shave for α. "
				"This does not affect the DNA. "
				"</p>"
				"<p>"
				"Shaves affect head hair only. "
				"Hair on other body parts is not shaved. "
				"</p>"
				"<p>"
				"<i>Close</i>, <i>Straight Out</i>, <i>Stubble</i>, and <i>Unshaven</i> show every follicle. "
				"Other shaves may hide hair. "
				"</p>"
				"</html>"));
	for(int s=0; s<=AuHairStyle::Unshaven; s++)
		m_AShave->addItem(AuHairStyle::ShaveName((AuHairStyle::SHAVE)s), s);
	for(QMap<QString, AuHairStyle::SHAVE>::const_iterator si = shaveStyles.cbegin(); si != shaveStyles.cend(); si++)
		m_AShave->addItem(si.key(), *si);
	m_AShave->setCurrentIndex(m_AShave->findData(lastShaveA));
	connect(m_AShave, SIGNAL(currentIndexChanged(int)), SLOT(AShaveChanged(int)));

	m_AHat = new AuToolButtonValue(lastAHat);
	if(lastAHat == CID_Invalid)
		m_AHat->setIcon(QIcon(":data/SlotHead.png"));
	else
		m_AHat->setIcon(QIcon(g_Au.CommodityBitmapName(lastAHat)));
	m_AHat->setToolTip(tr("<html>"
				"Clothes worn on the head of α. "
				"This does not affect the DNA. "
				"</html>"));
	connect(m_AHat, SIGNAL(clicked()), SLOT(AHatClicked()));

	m_AArmor = new AuToolButtonValue(lastAArmor);
	if(lastAArmor == CID_Invalid)
		m_AArmor->setIcon(QIcon(":data/SlotBodyWear.png"));
	else
		m_AArmor->setIcon(QIcon(g_Au.CommodityBitmapName(lastAArmor)));
	m_AArmor->setToolTip(tr("<html>"
				"Armor worn on the body of α. "
				"This does not affect the DNA. "
				"</html>"));
	connect(m_AArmor, SIGNAL(clicked()), SLOT(AArmorClicked()));

	m_AClothes = new AuToolButtonValue(lastAClothes);
	if(lastAClothes == CID_Invalid)
		m_AClothes->setIcon(QIcon(":data/SlotBodyWear.png"));
	else
		m_AClothes->setIcon(QIcon(g_Au.CommodityBitmapName(lastAClothes)));
	m_AClothes->setToolTip(tr("<html>"
				"Clothes worn on the body of α. "
				"This does not affect the DNA. "
				"</html>"));
	connect(m_AClothes, SIGNAL(clicked()), SLOT(AClothesClicked()));

	m_AMouth = new QComboBox;
	m_AMouth->addItem(QString("%1").arg(AuPose(AuPose::MouthOpenWide).Name()), AuPose::MouthOpenWide);
	m_AMouth->addItem(QString("%1 :D").arg(AuPose(AuPose::MouthGrin).Name()), AuPose::MouthGrin);
	m_AMouth->addItem(QString("%1 :)").arg(AuPose(AuPose::MouthSmile).Name()), AuPose::MouthSmile);
	m_AMouth->addItem(QString("%1 :O").arg(AuPose(AuPose::MouthAgape).Name()), AuPose::MouthAgape);
	m_AMouth->addItem(QString("%1 :P").arg(AuPose(AuPose::MouthSardonic).Name()), AuPose::MouthSardonic);
	m_AMouth->addItem(QString("%1 :o").arg(AuPose(AuPose::MouthOpen).Name()), AuPose::MouthOpen);
	m_AMouth->addItem(QString("%1 :|").arg(AuPose(AuPose::MouthClosed).Name()), AuPose::MouthClosed);
	m_AMouth->addItem(QString("%1 >.<").arg(AuPose(AuPose::MouthPursed).Name()), AuPose::MouthPursed);
	m_AMouth->addItem(QString("%1 :/").arg(AuPose(AuPose::MouthGrim).Name()), AuPose::MouthGrim);
	m_AMouth->addItem(QString("%1 :S").arg(AuPose(AuPose::MouthPerplexed).Name()), AuPose::MouthPerplexed);
	m_AMouth->addItem(QString("%1 :(").arg(AuPose(AuPose::MouthFrown).Name()), AuPose::MouthFrown);
	m_AMouth->addItem(QString("%1 D:").arg(AuPose(AuPose::MouthSad).Name()), AuPose::MouthSad);

	m_AMouth->addItem(QString("%1 lol").arg(AuPose(AuPose::MouthLaugh).Name()), AuPose::MouthLaugh);
	m_AMouth->addItem(QString("%1 :'(").arg(AuPose(AuPose::MouthCry).Name()), AuPose::MouthCry);
	m_AMouth->addItem(QString("%1").arg(AuPose(AuPose::MouthTalk).Name()), AuPose::MouthTalk);
	m_AMouth->addItem(QString("%1").arg(AuPose(AuPose::MouthJabber).Name()), AuPose::MouthJabber);
	m_AMouth->addItem(QString("%1").arg(AuPose(AuPose::MouthWhisper).Name()), AuPose::MouthWhisper);
	m_AMouth->addItem(QString("%1").arg(AuPose(AuPose::MouthYell).Name()), AuPose::MouthYell);

	m_AMouth->addItem(QString("%1").arg(AuPose(AuPose::MouthBite).Name()), AuPose::MouthBite);
	m_AMouth->addItem(QString("%1").arg(AuPose(AuPose::MouthYap).Name()), AuPose::MouthYap);
	m_AMouth->addItem(QString("%1").arg(AuPose(AuPose::MouthYawn).Name()), AuPose::MouthYawn);
	m_AMouth->setToolTip(tr("<html>"
				"<p>"
				"Mouth position for α. "
				"This does not affect the DNA. "
				"</p>"
				"</html>"));
	m_AMouth->setCurrentIndex(m_AMouth->findData(lastMouthA));
	connect(m_AMouth, SIGNAL(currentIndexChanged(int)), SLOT(AMouthChanged(int)));

	m_APose = new QComboBox;
#if !defined(QT_NO_DEBUG)
	m_APose->addItem(tr("Unposed"), AuPose::Init);
#endif
	m_APose->addItem(tr("Stand"), AuPose::Stand);
	m_APose->addItem(tr("Walk"), AuPose::StandWalk);
	m_APose->addItem(tr("Run"), AuPose::StandRun);
	m_APose->addItem(tr("Tread Water"), AuPose::TreadWater);
	m_APose->addItem(tr("Swim"), AuPose::Swim);
	m_APose->addItem(tr("Swim Fast"), AuPose::SwimFast);
	m_APose->addItem(tr("Prone"), AuPose::Prone);
	m_APose->addItem(tr("Crouch"), AuPose::Crouch);
	m_APose->addItem(tr("Sit on Floor"), AuPose::SitOnGround);
	m_APose->addItem(tr("Sit in Chair"), AuPose::Post_Sit);
	m_APose->addItem(tr("Berth, Facing Down"), AuPose::BerthBottomFaceDown);
	m_APose->addItem(tr("Berth, Facing Left"), AuPose::BerthBottomFaceLeft);
	m_APose->addItem(tr("Berth, Facing Up"), AuPose::BerthBottomFaceUp);
	m_APose->addItem(tr("Berth, Facing Right"), AuPose::BerthBottomFaceRight);
	m_APose->addItem(tr("Head Only"), AuPose::EyesOnly);
	if(g_Au.IsMatingOk())
	{
		m_APose->addItem(tr("Mating"), AuPose::ReceptiveMate);
		m_APose->addItem(tr("Mating on Top"), AuPose::MateOnTop);
	}
	m_APose->addItem(tr("Dying, Facing Down"), AuPose::WritheFaceDown);
	m_APose->addItem(tr("Dying, Facing Left"), AuPose::WritheFaceLeft);
	m_APose->addItem(tr("Dying, Facing Up"), AuPose::WritheFaceUp);
	m_APose->addItem(tr("Dying, Facing Right"), AuPose::WritheFaceRight);
	m_APose->addItem(tr("Dead, Facing Down"), AuPose::DeadFaceDown);
	m_APose->addItem(tr("Dead, Facing Left"), AuPose::DeadFaceLeft);
	m_APose->addItem(tr("Dead, Facing Up"), AuPose::DeadFaceUp);
	m_APose->addItem(tr("Dead, Facing Right"), AuPose::DeadFaceRight);
	m_APose->setToolTip(tr("<html>"
				"<p>"
				"Pose for α. "
				"This does not affect the DNA. "
				"</p>"
				"<p>"
				"The body is posed with the right arm bent. "
				"In the game, the dominant arm bends when holding an object, which might not be the right arm. "
				"</p>"
				"</html>"));
	m_APose->setCurrentIndex(m_APose->findData(lastPoseA));
	connect(m_APose, SIGNAL(currentIndexChanged(int)), SLOT(APoseChanged(int)));

	m_BHaircut = new QComboBox;
	m_BHaircut->setToolTip(tr("<html>"
				"<p>"
				"Hair style for β. "
				"This does not affect the DNA. "
				"</p>"
				"<p>"
				"Hair styles affect head hair only. "
				"Hair on other body parts is not styled. "
				"</p>"
				"<p>"
				"<i>Buzz</i>, <i>Close</i>, <i>Straight Out</i>, and <i>Unkempt</i> show every follicle. "
				"Other styles may hide hair. "
				"</p>"
				"</html>"));
	for(int s=0; s<=AuHairStyle::Unkempt; s++)
		m_BHaircut->addItem(AuHairStyle::Name((AuHairStyle::STYLE)s), s);
	for(QMap<QString, AuHairStyle::STYLE>::const_iterator hi = hairStyles.cbegin(); hi != hairStyles.cend(); hi++)
		m_BHaircut->addItem(hi.key(), *hi);
	m_BHaircut->setCurrentIndex(m_BHaircut->findData(lastHaircutB));
	connect(m_BHaircut, SIGNAL(currentIndexChanged(int)), SLOT(BHaircutChanged(int)));

	m_BShave = new QComboBox;
	m_BShave->setToolTip(tr("<html>"
				"<p>"
				"Shave for β. "
				"This does not affect the DNA. "
				"</p>"
				"<p>"
				"Shaves affect head hair only. "
				"Hair on other body parts is not shaved. "
				"</p>"
				"<p>"
				"<i>Close</i>, <i>Straight Out</i>, <i>Stubble</i>, and <i>Unshaven</i> show every follicle. "
				"Other shaves may hide hair. "
				"</p>"
				"</html>"));
	for(int s=0; s<=AuHairStyle::Unshaven; s++)
		m_BShave->addItem(AuHairStyle::ShaveName((AuHairStyle::SHAVE)s), s);
	for(QMap<QString, AuHairStyle::SHAVE>::const_iterator si = shaveStyles.cbegin(); si != shaveStyles.cend(); si++)
		m_BShave->addItem(si.key(), *si);
	m_BShave->setCurrentIndex(m_BShave->findData(lastShaveB));
	connect(m_BShave, SIGNAL(currentIndexChanged(int)), SLOT(BShaveChanged(int)));

	m_BHat = new AuToolButtonValue(lastBHat);
	if(lastBHat == CID_Invalid)
		m_BHat->setIcon(QIcon(":data/SlotHead.png"));
	else
		m_BHat->setIcon(QIcon(g_Au.CommodityBitmapName(lastBHat)));
	m_BHat->setToolTip(tr("<html>"
				"Clothes worn on the head of β. "
				"This does not affect the DNA. "
				"</html>"));
	connect(m_BHat, SIGNAL(clicked()), SLOT(BHatClicked()));

	m_BArmor = new AuToolButtonValue(lastBArmor);
	if(lastBArmor == CID_Invalid)
		m_BArmor->setIcon(QIcon(":data/SlotBodyWear.png"));
	else
		m_BArmor->setIcon(QIcon(g_Au.CommodityBitmapName(lastBArmor)));
	m_BArmor->setToolTip(tr("<html>"
				"Armor worn on the body of β. "
				"This does not affect the DNA. "
				"</html>"));
	connect(m_BArmor, SIGNAL(clicked()), SLOT(BArmorClicked()));

	m_BClothes = new AuToolButtonValue(lastBClothes);
	if(lastBClothes == CID_Invalid)
		m_BClothes->setIcon(QIcon(":data/SlotBodyWear.png"));
	else
		m_BClothes->setIcon(QIcon(g_Au.CommodityBitmapName(lastBClothes)));
	m_BClothes->setToolTip(tr("<html>"
				"Clothes worn on the body of β. "
				"This does not affect the DNA. "
				"</html>"));
	connect(m_BClothes, SIGNAL(clicked()), SLOT(BClothesClicked()));

	m_BMouth = new QComboBox;
	m_BMouth->addItem(QString("%1").arg(AuPose(AuPose::MouthOpenWide).Name()), AuPose::MouthOpenWide);
	m_BMouth->addItem(QString("%1 :D").arg(AuPose(AuPose::MouthGrin).Name()), AuPose::MouthGrin);
	m_BMouth->addItem(QString("%1 :)").arg(AuPose(AuPose::MouthSmile).Name()), AuPose::MouthSmile);
	m_BMouth->addItem(QString("%1 :O").arg(AuPose(AuPose::MouthAgape).Name()), AuPose::MouthAgape);
	m_BMouth->addItem(QString("%1 :P").arg(AuPose(AuPose::MouthSardonic).Name()), AuPose::MouthSardonic);
	m_BMouth->addItem(QString("%1 :o").arg(AuPose(AuPose::MouthOpen).Name()), AuPose::MouthOpen);
	m_BMouth->addItem(QString("%1 :|").arg(AuPose(AuPose::MouthClosed).Name()), AuPose::MouthClosed);
	m_BMouth->addItem(QString("%1 >.<").arg(AuPose(AuPose::MouthPursed).Name()), AuPose::MouthPursed);
	m_BMouth->addItem(QString("%1 :/").arg(AuPose(AuPose::MouthGrim).Name()), AuPose::MouthGrim);
	m_BMouth->addItem(QString("%1 :S").arg(AuPose(AuPose::MouthPerplexed).Name()), AuPose::MouthPerplexed);
	m_BMouth->addItem(QString("%1 :(").arg(AuPose(AuPose::MouthFrown).Name()), AuPose::MouthFrown);
	m_BMouth->addItem(QString("%1 D:").arg(AuPose(AuPose::MouthSad).Name()), AuPose::MouthSad);

	m_BMouth->addItem(QString("%1 lol").arg(AuPose(AuPose::MouthLaugh).Name()), AuPose::MouthLaugh);
	m_BMouth->addItem(QString("%1 :'(").arg(AuPose(AuPose::MouthCry).Name()), AuPose::MouthCry);
	m_BMouth->addItem(QString("%1").arg(AuPose(AuPose::MouthTalk).Name()), AuPose::MouthTalk);
	m_BMouth->addItem(QString("%1").arg(AuPose(AuPose::MouthJabber).Name()), AuPose::MouthJabber);
	m_BMouth->addItem(QString("%1").arg(AuPose(AuPose::MouthWhisper).Name()), AuPose::MouthWhisper);
	m_BMouth->addItem(QString("%1").arg(AuPose(AuPose::MouthYell).Name()), AuPose::MouthYell);

	m_BMouth->addItem(QString("%1").arg(AuPose(AuPose::MouthBite).Name()), AuPose::MouthBite);
	m_BMouth->addItem(QString("%1").arg(AuPose(AuPose::MouthYap).Name()), AuPose::MouthYap);
	m_BMouth->addItem(QString("%1").arg(AuPose(AuPose::MouthYawn).Name()), AuPose::MouthYawn);
	m_BMouth->setToolTip(tr("<html>"
				"<p>"
				"Mouth position for β. "
				"This does not affect the DNA. "
				"</p>"
				"</html>"));
	m_BMouth->setCurrentIndex(m_BMouth->findData(lastMouthB));
	connect(m_BMouth, SIGNAL(currentIndexChanged(int)), SLOT(BMouthChanged(int)));

	m_BPose = new QComboBox;
#if !defined(QT_NO_DEBUG)
	m_BPose->addItem(tr("Unposed"), AuPose::Init);
#endif
	m_BPose->addItem(tr("Stand"), AuPose::Stand);
	m_BPose->addItem(tr("Walk"), AuPose::StandWalk);
	m_BPose->addItem(tr("Run"), AuPose::StandRun);
	m_BPose->addItem(tr("Tread Water"), AuPose::TreadWater);
	m_BPose->addItem(tr("Swim"), AuPose::Swim);
	m_BPose->addItem(tr("Swim Fast"), AuPose::SwimFast);
	m_BPose->addItem(tr("Prone"), AuPose::Prone);
	m_BPose->addItem(tr("Crouch"), AuPose::Crouch);
	m_BPose->addItem(tr("Sit on Floor"), AuPose::SitOnGround);
	m_BPose->addItem(tr("Sit in Chair"), AuPose::Post_Sit);
	m_BPose->addItem(tr("Berth, Facing Down"), AuPose::BerthBottomFaceDown);
	m_BPose->addItem(tr("Berth, Facing Left"), AuPose::BerthBottomFaceLeft);
	m_BPose->addItem(tr("Berth, Facing Up"), AuPose::BerthBottomFaceUp);
	m_BPose->addItem(tr("Berth, Facing Right"), AuPose::BerthBottomFaceRight);
	m_BPose->addItem(tr("Head Only"), AuPose::EyesOnly);
	if(g_Au.IsMatingOk())
	{
		m_BPose->addItem(tr("Mating"), AuPose::ReceptiveMate);
		m_BPose->addItem(tr("Mating on Top"), AuPose::MateOnTop);
	}
	m_BPose->addItem(tr("Dying, Facing Down"), AuPose::WritheFaceDown);
	m_BPose->addItem(tr("Dying, Facing Left"), AuPose::WritheFaceLeft);
	m_BPose->addItem(tr("Dying, Facing Up"), AuPose::WritheFaceUp);
	m_BPose->addItem(tr("Dying, Facing Right"), AuPose::WritheFaceRight);
	m_BPose->addItem(tr("Dead, Facing Down"), AuPose::DeadFaceDown);
	m_BPose->addItem(tr("Dead, Facing Left"), AuPose::DeadFaceLeft);
	m_BPose->addItem(tr("Dead, Facing Up"), AuPose::DeadFaceUp);
	m_BPose->addItem(tr("Dead, Facing Right"), AuPose::DeadFaceRight);
	m_BPose->setToolTip(tr("<html>"
				"<p>"
				"Pose for β. "
				"This does not affect the DNA. "
				"</p>"
				"<p>"
				"The body is posed with the left arm bent. "
				"In the game, the dominant arm bends when holding an object, which might not be the left arm. "
				"</p>"
				"</html>"));
	m_BPose->setCurrentIndex(m_BPose->findData(lastPoseB));
	connect(m_BPose, SIGNAL(currentIndexChanged(int)), SLOT(BPoseChanged(int)));

	m_AStatus = new QLabel(this);
	m_AStatus->setAlignment(Qt::AlignCenter);
	m_AStatus->setToolTip(tr("<html>"
				"Info about α. Colors show suitability for use as an avatar."
				"<br>"
				"<b>Hit Points</b> Amber indicates too many. "
				"<br>"
				"<b>Cg Height</b> Amber indicates too high|low standing. Yellow indicates too high|low sitting. "
				"<br>"
				"<b>Head Height</b> Amber indicates too high|low standing. Yellow indicates too high|low sitting. "
				"<br>"
				"<b>Eye Height</b> Amber indicates too high|low standing. Yellow indicates too high|low sitting. "
				"<br>"
				"<b>Ability Points Used</b> Amber indicates too many. "
				"</html>"));

	m_BStatus = new QLabel(this);
	m_BStatus->setAlignment(Qt::AlignCenter);
	m_BStatus->setToolTip(tr("<html>"
				"Info about β. Colors show suitability for use as an avatar."
				"<br>"
				"<b>Hit Points</b> Amber indicates too many. "
				"<br>"
				"<b>Cg Height</b> Amber indicates too high|low standing. Yellow indicates too high|low sitting. "
				"<br>"
				"<b>Head Height</b> Amber indicates too high|low standing. Yellow indicates too high|low sitting. "
				"<br>"
				"<b>Eye Height</b> Amber indicates too high|low standing. Yellow indicates too high|low sitting. "
				"<br>"
				"<b>Ability Points Used</b> Amber indicates too many. "
				"</html>"));

	QGridLayout* alay = new QGridLayout;
	alay->addWidget(m_AHaircut, 0,0);
	alay->addWidget(m_AMouth, 0,1);
	alay->addWidget(m_AShave, 1,0);
	alay->addWidget(m_APose, 1,1);
	alay->addWidget(m_AHat, 1,2);
	alay->addWidget(m_AArmor, 1,3);
	alay->addWidget(m_AClothes, 1,4);

	QGridLayout* blay = new QGridLayout;
	blay->addWidget(m_BHaircut, 0,0);
	blay->addWidget(m_BMouth, 0,1);
	blay->addWidget(m_BShave, 1,0);
	blay->addWidget(m_BPose, 1,1);
	blay->addWidget(m_BHat, 1,2);
	blay->addWidget(m_BArmor, 1,3);
	blay->addWidget(m_BClothes, 1,4);

	QHBoxLayout* topcenlay = new QHBoxLayout;
	topcenlay->addLayout(alay);
	topcenlay->addStretch();
	topcenlay->addWidget(resetview);
	topcenlay->addWidget(m_ShowGround);
	topcenlay->addStretch();
	topcenlay->addLayout(blay);

	QHBoxLayout* bottomcenlay = new QHBoxLayout;
	bottomcenlay->addWidget(m_AStatus);
	bottomcenlay->addWidget(m_BStatus);

	QVBoxLayout* vlay = new QVBoxLayout;
	vlay->addLayout(topcenlay);
	vlay->addWidget(m_View);
	vlay->addLayout(bottomcenlay);

	QWidget* leftPane = new QWidget;
	leftPane->setLayout(vlay);

	QHBoxLayout* hlay = new QHBoxLayout;
	hlay->addWidget(lbAlpha);
	hlay->addWidget(lbBeta);

	m_Chromosomes = new QVBoxLayout;
	m_Chromosomes->setSpacing(0);
	m_Chromosomes->setMargin(0);

	QHBoxLayout* tblay = new QHBoxLayout;
	tblay->addWidget(addChromosome);
	tblay->addStretch();
	tblay->addWidget(human);
	tblay->addWidget(wizard);
	tblay->addWidget(random);
	tblay->addStretch();
	tblay->addWidget(savedna);
	tblay->addWidget(loaddna);
	tblay->addWidget(nudna);

	QVBoxLayout* tlay = new QVBoxLayout;
	tlay->addLayout(tblay);
	tlay->addLayout(hlay);
	tlay->addLayout(m_Chromosomes);
	tlay->addStretch();
	QWidget* dnaChromosomes = new QWidget;
	dnaChromosomes->setLayout(tlay);

	QScrollArea* scroller = new QScrollArea;
	scroller->setWidget(dnaChromosomes);
	scroller->setWidgetResizable(true);

	QVBoxLayout* rlay = new QVBoxLayout;
	rlay->addWidget(scroller, 1);
	rlay->addLayout(m_Pages);
	rlay->setSpacing(2);
	rlay->setMargin(0);
	QWidget* rightPane = new QWidget;
	rightPane->setLayout(rlay);

	m_Splitter = new QSplitter(Qt::Horizontal, this);
	m_Splitter->addWidget(leftPane);
	m_Splitter->addWidget(rightPane);
	m_Splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	connect(m_Splitter, SIGNAL(splitterMoved(int, int)), SLOT(SplitterMoved(int, int)));

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(m_Splitter);
	lay->setSpacing(2);
	lay->setMargin(2);
	setLayout(lay);

    QByteArray splitterstate;
    g_Au.Settings().readEntry("Widget/DNASettings/Splitter", splitterstate);
    m_Splitter->restoreState(splitterstate);
}

AuDNASettings::~AuDNASettings()
{
}

void AuDNASettings::SplitterMoved(int pos, int index)
{
	Q_UNUSED(pos);
	Q_UNUSED(index);
    g_Au.Settings().writeEntry("Widget/DNASettings/Splitter", m_Splitter->saveState());
}

void AuDNASettings::AddChromosome()
{
	QMap<QString, AuChromosome::CTYPE> addable;
	m_DNA.AddableChromosomes(addable);
	if(addable.empty())
	{
		QString smsg = QObject::tr("No additional chromosomes can be added to the DNA.");
		QMessageBox::information(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok);
		return;
	}

	QStringList addNames = addable.keys();
	bool ok;
	QString addName = QInputDialog::getItem(this, tr("Add Chromosome"),
            tr("<html>"
                "Choose a chromosome to add to the DNA. "
                "</html>"), addNames, 0, false, &ok);
	if(!ok)
		return;
	QMap<QString, AuChromosome::CTYPE>::const_iterator ai = addable.find(addName);
	if(ai == addable.cend())
		return;	// wut?

	// Add the chromosome to my DNA.
	int chg = m_DNA.AddChromosome(*ai) ? 0x03 : 0x00;
	m_Pages->setCurrentIndex(0);
	ListChromosomes();

	// Signal changes that affect the models.
	m_View->SetDNAPair(m_DNA, chg);
	UpdateView();
}

void AuDNASettings::UpdateView()
{
	UpdateStatus();

	AuHairStyle::STYLE cut = (AuHairStyle::STYLE)m_AHaircut->currentData().toInt();
	AuHairStyle::SHAVE shave = (AuHairStyle::SHAVE)m_AShave->currentData().toInt();
	m_View->SetMaleHair(cut, shave);						// Does nothing if there is no change.

	cut = (AuHairStyle::STYLE)m_BHaircut->currentData().toInt();
	shave = (AuHairStyle::SHAVE)m_BShave->currentData().toInt();
	m_View->SetFemaleHair(cut, shave);						// Does nothing if there is no change.

	COMMODITYID hat = (COMMODITYID)m_AHat->Value();
	COMMODITYID armor = (COMMODITYID)m_AArmor->Value();
	COMMODITYID clothes = (COMMODITYID)m_AClothes->Value();
	m_View->SetMaleClothing(hat, armor, clothes);	// Does nothing if there is no change.

	hat = (COMMODITYID)m_BHat->Value();
	armor = (COMMODITYID)m_BArmor->Value();
	clothes = (COMMODITYID)m_BClothes->Value();
	m_View->SetFemaleClothing(hat, armor, clothes);	// Does nothing if there is no change.

	AuPose::POSE pose = (AuPose::POSE)m_APose->currentData().toInt();
	m_View->SetMalePose(pose, true);

	AuPose::POSE mouth = (AuPose::POSE)m_AMouth->currentData().toInt();
	m_View->SetMaleMouthPose(mouth);

	pose = (AuPose::POSE)m_BPose->currentData().toInt();
	m_View->SetFemalePose(pose, true);

	mouth = (AuPose::POSE)m_BMouth->currentData().toInt();
	m_View->SetFemaleMouthPose(mouth);
}

void AuDNASettings::UpdateStatus()
{
	// Female
	int statusFemale = m_View->AvatarCheckFemale();
	int ptsUsedFemale = m_View->AbilityPointsUsedFemale();
	float cgHtFemale = m_View->CgHeightFemale() / FeetPerMeter();
	float eyeHtFemale = cgHtFemale + m_View->CameraHeightFemale() / FeetPerMeter();
	float headHtFemale = cgHtFemale + m_View->HeadHeightFemale() / FeetPerMeter();
	int hpFemale = m_View->HitPointsFemale();

	QString statFemale;
	if(!(statusFemale & 0x8000))
	{
		QString pts;
		if(statusFemale & 0x0001)
			pts = QString("<span style=\"color: rgb(255, 130, 32);\">Pts %1</span>").arg(ptsUsedFemale);
		else
			pts = QString("Pts %1").arg(ptsUsedFemale);

		QString hp;
		if(statusFemale & 0x0002)
			hp = QString("<span style=\"color: rgb(255, 130, 32);\">%L1hp</span>").arg(hpFemale);
		else
			hp = QString("%L1hp").arg(hpFemale);

		QString cg;
		if(statusFemale & 0x0008)
			cg = QString("<span style=\"color: rgb(255, 130, 32);\">Cg %1m</span>").arg(cgHtFemale, 0,'f',2);
		else
			cg = QString("Cg %1m").arg(cgHtFemale, 0,'f',2);

		QString head;
		if(statusFemale & 0x0010)
			head = QString("<span style=\"color: rgb(255, 130, 32);\">Head %1m</span>").arg(headHtFemale, 0,'f',2);
		else if(statusFemale & 0x0040)
			head = QString("<span style=\"color: rgb(255, 255, 32);\">Head %1m</span>").arg(headHtFemale, 0,'f',2);
		else
			head = QString("Head %1m").arg(headHtFemale, 0,'f',2);

		QString eye;
		if(statusFemale & 0x0004)
			eye = QString("<span style=\"color: rgb(255, 130, 32);\">Eye %1m</span>").arg(eyeHtFemale, 0,'f',2);
		else if(statusFemale & 0x0020)
			eye = QString("<span style=\"color: rgb(255, 255, 32);\">Eye %1m</span>").arg(eyeHtFemale, 0,'f',2);
		else
			eye = QString("Eye %1m").arg(eyeHtFemale, 0,'f',2);

		QString lungs;
		if(statusFemale & 0x0080)
			lungs = QString("<span style=\"color: rgb(255, 130, 32);\">Lungs</span>");

		statFemale = QString("<html>%1 %2 %3 %4 %5 %6</html>")
			.arg(hp)
			.arg(cg)
			.arg(head)
			.arg(eye)
			.arg(pts)
			.arg(lungs);
	}
	m_BStatus->setText(statFemale);

	// Male
	int statusMale = m_View->AvatarCheckMale();
	int ptsUsedMale = m_View->AbilityPointsUsedMale();
	float cgHtMale = m_View->CgHeightMale() / FeetPerMeter();
	float eyeHtMale = cgHtMale + m_View->CameraHeightMale() / FeetPerMeter();
	float headHtMale = cgHtMale + m_View->HeadHeightMale() / FeetPerMeter();
	int hpMale = m_View->HitPointsMale();

	QString statMale;
	if(!(statusMale & 0x8000))
	{
		QString pts;
		if(statusMale & 0x0001)
			pts = QString("<span style=\"color: rgb(255, 130, 32);\">Pts %1</span>").arg(ptsUsedMale);
		else
			pts = QString("Pts %1").arg(ptsUsedMale);

		QString hp;
		if(statusMale & 0x0002)
			hp = QString("<span style=\"color: rgb(255, 130, 32);\">%L1hp</span>").arg(hpMale);
		else
			hp = QString("%L1hp").arg(hpMale);

		QString cg;
		if(statusMale & 0x0008)
			cg = QString("<span style=\"color: rgb(255, 130, 32);\">Cg %1m</span>").arg(cgHtMale, 0,'f',2);
		else
			cg = QString("Cg %1m").arg(cgHtMale, 0,'f',2);

		QString head;
		if(statusMale & 0x0010)
			head = QString("<span style=\"color: rgb(255, 130, 32);\">Head %1m</span>").arg(headHtMale, 0,'f',2);
		else if(statusMale & 0x0040)
			head = QString("<span style=\"color: rgb(255, 255, 32);\">Head %1m</span>").arg(headHtMale, 0,'f',2);
		else
			head = QString("Head %1m").arg(headHtMale, 0,'f',2);

		QString eye;
		if(statusMale & 0x0004)
			eye = QString("<span style=\"color: rgb(255, 130, 32);\">Eye %1m</span>").arg(eyeHtMale, 0,'f',2);
		else if(statusMale & 0x0020)
			eye = QString("<span style=\"color: rgb(255, 255, 32);\">Eye %1m</span>").arg(eyeHtMale, 0,'f',2);
		else
			eye = QString("Eye %1m").arg(eyeHtMale, 0,'f',2);

		QString lungs;
		if(statusMale & 0x0080)
			lungs = QString("<span style=\"color: rgb(255, 130, 32);\">Lungs</span>");

		statMale = QString("<html>%1 %2 %3 %4 %5 %6</html>")
			.arg(hp)
			.arg(cg)
			.arg(head)
			.arg(eye)
			.arg(pts)
			.arg(lungs);
	}
	m_AStatus->setText(statMale);
	emit StatusUpdated();
}

void AuDNASettings::NewDNA()
{
	if(!m_DNA.Chain(AuDNA::Alpha).empty())
	{
		QString smsg = tr("<html>Confirm clear of current DNA. "
				"</html>");
		if(QMessageBox::information(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok, QMessageBox::Cancel) != QMessageBox::Ok)
			return;
	}
	m_DNA = AuDNA();
	ListChromosomes();
	m_View->SetDNAPair(m_DNA, 3);
	//m_View->ResetView();
	UpdateView();
}

void AuDNASettings::Human()
{
	QString smsg = tr("<html>Confirm change of current DNA to default human. "
			"</html>");
	if(QMessageBox::information(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok, QMessageBox::Cancel) != QMessageBox::Ok)
		return;
	m_DNA.InitDefaultHuman();
	ListChromosomes();
	m_View->SetDNAPair(m_DNA, 3);
	//m_View->ResetView();
	UpdateView();
}

void AuDNASettings::ListChromosomes()
{
	// Clear existing chromosomes.
	while(!m_ChromosomeWidgets.empty())
	{
		AuChromosomeWidget* w = m_ChromosomeWidgets.takeFirst();
		m_Chromosomes->removeWidget(w);
		w->hide();
		w->deleteLater();
	}

	// Sort current chromosomes.
	QMultiMap<int, AuChromosome*> alphaSorted, betaSorted;
	const AuChromatid& alphaChain = m_DNA.Chain(AuDNA::Alpha);
	const AuChromatid& betaChain = m_DNA.Chain(AuDNA::Beta);
	Q_ASSERT(alphaChain.size() == betaChain.size());
	AuChromatid::const_iterator mi = alphaChain.cbegin();
	AuChromatid::const_iterator fi = betaChain.cbegin();
	while(mi != alphaChain.cend() && fi != betaChain.cend())
	{
		alphaSorted.insert((*mi)->ListWidgetOrder(), *mi);
		betaSorted.insert((*fi)->ListWidgetOrder(), *fi);
		mi++;
		fi++;
	}

	// List current chromosomes.
	QMultiMap<int, AuChromosome*>::const_iterator msi = alphaSorted.cbegin();
	QMultiMap<int, AuChromosome*>::const_iterator fsi = betaSorted.cbegin();
	while(msi != alphaSorted.cend() && fsi != betaSorted.cend())
	{
		AuChromosomeWidget* tw = new AuChromosomeWidget(*msi, *fsi);
		m_Chromosomes->addWidget(tw);
		m_ChromosomeWidgets.append(tw);
		connect(tw, SIGNAL(Clicked(AuChromosomeWidget*)), SLOT(ChromosomeClicked(AuChromosomeWidget*)));
		connect(tw, SIGNAL(Deleted(AuChromosomeWidget*)), SLOT(ChromosomeDeleted(AuChromosomeWidget*)));
		msi++;
		fsi++;
	}
}

void AuDNASettings::ChromosomeClicked(AuChromosomeWidget* tw)
{
	for(QList<AuChromosomeWidget*>::iterator ti = m_ChromosomeWidgets.begin(); ti != m_ChromosomeWidgets.end(); ti++)
		(*ti)->SetSelected(*ti == tw);

	AuChromosome::CTYPE chromosome = tw->ChromosomeType();
	QMap<AuChromosome::CTYPE, int>::const_iterator pi = m_PageIndexes.find(chromosome);
	if(pi == m_PageIndexes.cend())
	{
		qWarning() << "error: AuDNASettings::ChromosomeClicked no page for type" << chromosome;
		return;
	}
	m_Pages->setCurrentIndex(*pi);
	AuChromoPageBase* curPage = dynamic_cast<AuChromoPageBase*>(m_Pages->currentWidget());
	if(curPage)
		curPage->ShowProperties(m_DNA, tw->AlphaChromosome(), tw->BetaChromosome());
}

void AuDNASettings::ChromosomeDeleted(AuChromosomeWidget* tw)
{
	AuChromosome::CTYPE chromosome = tw->AlphaChromosome()->Type();
	AuChromoWhich which = tw->AlphaChromosome()->Which();

	// Remove the widget from the list.
	for(QList<AuChromosomeWidget*>::iterator ti = m_ChromosomeWidgets.begin(); ti != m_ChromosomeWidgets.end(); ti++)
		if(*ti == tw)
		{
			if(tw->IsSelected())
				m_Pages->setCurrentIndex(0);

			tw->deleteLater();
			m_ChromosomeWidgets.erase(ti);
			break;
		}

	// Remove the chromosome from my DNA.
	m_DNA.Remove(chromosome, which);
	m_Pages->setCurrentIndex(0);
	ListChromosomes();

	// Signal changes that affect the models.
	m_View->SetDNAPair(m_DNA, 3);
	UpdateView();
}

void AuDNASettings::ApplyChromosomeChanges()
{
	AuChromoPageBase* curPage = dynamic_cast<AuChromoPageBase*>(m_Pages->currentWidget());
	if(!curPage)
		return;

	// Find the selected chromosome.
	AuChromosomeWidget* tw = 0;
	for(QList<AuChromosomeWidget*>::iterator ti = m_ChromosomeWidgets.begin(); ti != m_ChromosomeWidgets.end(); ti++)
		if((*ti)->IsSelected())
		{
			tw = *ti;
			break;
		}
	if(!tw)
		return;

	int chg = curPage->ApplyProperties(tw->AlphaChromosome(), tw->BetaChromosome());
	if(chg >= 0)
	{
		// Update the list entry.
		tw->ChromosomesChanged();

		// Update the master DNA.
		m_DNA.ApplyChanges(tw->AlphaChromosome(), tw->BetaChromosome());

		// Signal changes that affect the models.
		m_View->SetDNAPair(m_DNA, chg);
		UpdateView();
	}
}

void AuDNASettings::ApplyChromosomeChangesToAll()
{
	AuChromoPageBase* curPage = dynamic_cast<AuChromoPageBase*>(m_Pages->currentWidget());
	if(!curPage)
		return;

	// Find the selected chromosome.
	AuChromosomeWidget* tw = 0;
	for(QList<AuChromosomeWidget*>::iterator ti = m_ChromosomeWidgets.begin(); ti != m_ChromosomeWidgets.end(); ti++)
		if((*ti)->IsSelected())
		{
			tw = *ti;
			break;
		}
	if(!tw)
		return;

	// Apply changes to all similar chromosomes.
	int sumchg = 0;
	for(QList<AuChromosomeWidget*>::iterator ti = m_ChromosomeWidgets.begin(); ti != m_ChromosomeWidgets.end(); ti++)
		if((*ti)->AlphaChromosome()->Type() == tw->AlphaChromosome()->Type())
		{
			int chg = curPage->ApplyProperties((*ti)->AlphaChromosome(), (*ti)->BetaChromosome());
			if(chg >= 0)
			{
				// Update the list entry.
				(*ti)->ChromosomesChanged();

				// Update the master DNA.
				m_DNA.ApplyChanges((*ti)->AlphaChromosome(), (*ti)->BetaChromosome());

				sumchg |= chg;	// Accumulate change bits.
			}
		}

	// Signal changes that affect the models.
	if(sumchg)
	{
		m_View->SetDNAPair(m_DNA, sumchg);
		UpdateView();
	}
}

void AuDNASettings::ApplyChromosomeChangesToPair()
{
	AuChromoPageBase* curPage = dynamic_cast<AuChromoPageBase*>(m_Pages->currentWidget());
	if(!curPage)
		return;

	// Find the selected chromosome.
	AuChromosomeWidget* tw = 0;
	for(QList<AuChromosomeWidget*>::iterator ti = m_ChromosomeWidgets.begin(); ti != m_ChromosomeWidgets.end(); ti++)
		if((*ti)->IsSelected())
		{
			tw = *ti;
			break;
		}
	if(!tw)
		return;

	unsigned int whichIndex = tw->AlphaChromosome()->Which().Index();
	unsigned int pairIndex;
	if(whichIndex & 1)
		pairIndex = whichIndex - 1;	// index is odd so pair it with the previous index
	else
		pairIndex = whichIndex + 1;	// index is even so pair it with the next index

	// Apply changes to all similar chromosomes.
	int sumchg = 0;
	for(QList<AuChromosomeWidget*>::iterator ti = m_ChromosomeWidgets.begin(); ti != m_ChromosomeWidgets.end(); ti++)
		if((*ti)->AlphaChromosome()->Type() == tw->AlphaChromosome()->Type() &&
				((*ti)->AlphaChromosome()->Which().Index() == whichIndex || (*ti)->AlphaChromosome()->Which().Index() == pairIndex))
		{
			int chg = curPage->ApplyProperties((*ti)->AlphaChromosome(), (*ti)->BetaChromosome());
			if(chg >= 0)
			{
				// Update the list entry.
				(*ti)->ChromosomesChanged();

				// Update the master DNA.
				m_DNA.ApplyChanges((*ti)->AlphaChromosome(), (*ti)->BetaChromosome());

				sumchg |= chg;	// Accumulate change bits.
			}
		}

	// Signal changes that affect the models.
	if(sumchg)
	{
		m_View->SetDNAPair(m_DNA, sumchg);
		UpdateView();
	}
}

void AuDNASettings::AHaircutChanged(int index)
{
	AuHairStyle::STYLE cut = (AuHairStyle::STYLE)m_AHaircut->itemData(index).toInt();
	g_Au.Settings().writeEntry("DNASettings/HaircutA", cut);
	UpdateView();
}

void AuDNASettings::AShaveChanged(int index)
{
	AuHairStyle::SHAVE shave = (AuHairStyle::SHAVE)m_AShave->itemData(index).toInt();
	g_Au.Settings().writeEntry("DNASettings/ShaveA", shave);
	UpdateView();
}

void AuDNASettings::APoseChanged(int index)
{
	AuPose::POSE pose = (AuPose::POSE)m_APose->itemData(index).toInt();
	g_Au.Settings().writeEntry("DNASettings/PoseA", pose);
	UpdateView();
}

void AuDNASettings::AMouthChanged(int index)
{
	AuPose::POSE face = (AuPose::POSE)m_AMouth->itemData(index).toInt();
	g_Au.Settings().writeEntry("DNASettings/MouthA", face);
	UpdateView();
}

void AuDNASettings::BHaircutChanged(int index)
{
	AuHairStyle::STYLE cut = (AuHairStyle::STYLE)m_BHaircut->itemData(index).toInt();
	g_Au.Settings().writeEntry("DNASettings/HaircutB", cut);
	UpdateView();
}

void AuDNASettings::BShaveChanged(int index)
{
	AuHairStyle::SHAVE shave = (AuHairStyle::SHAVE)m_BShave->itemData(index).toInt();
	g_Au.Settings().writeEntry("DNASettings/ShaveB", shave);
	UpdateView();
}

void AuDNASettings::BPoseChanged(int index)
{
	AuPose::POSE pose = (AuPose::POSE)m_BPose->itemData(index).toInt();
	g_Au.Settings().writeEntry("DNASettings/PoseB", pose);
	UpdateView();
}

void AuDNASettings::BMouthChanged(int index)
{
	AuPose::POSE face = (AuPose::POSE)m_BMouth->itemData(index).toInt();
	g_Au.Settings().writeEntry("DNASettings/MouthB", face);
	UpdateView();
}

void AuDNASettings::AHatClicked()
{
	COMMODITYID clothingCIDs[] =
	{
		CID_Invalid,
		CID_LeatherHelmet,
		CID_WovenHelmet,
		CID_PlasticHelmet,
		CID_MetalHelmet,
		CID_VacSuitHelmet,
		CID_MetalPowerArmorHelmet,
		CID_MagmiumPowerArmorHelmet,
		CID_VulciumPowerArmorHelmet,
	};
	int clothingCIDcount = sizeof(clothingCIDs) / sizeof(COMMODITYID);

	QMenu menu(this);
	for(int c=0; c<clothingCIDcount; c++)
	{
		COMMODITYID cid = clothingCIDs[c];
		QAction* a;
		if(cid == CID_Invalid)
			a = menu.addAction(QIcon(":data/SlotHead.png"), tr("Uncovered"));
		else
			a = menu.addAction(QIcon(g_Au.CommodityBitmapName(cid)), g_Au.CommodityName(cid));
		a->setData(cid);
	}

	QAction* action = menu.exec(QCursor::pos());
	if(!action)
		return;

	COMMODITYID headClothing = (COMMODITYID)action->data().toInt();
	if(headClothing == m_AHat->Value())
		return;

	if(headClothing == CID_Invalid)
		m_AHat->setIcon(QIcon(":data/SlotHead.png"));
	else
		m_AHat->setIcon(QIcon(g_Au.CommodityBitmapName(headClothing)));
	m_AHat->SetValue(headClothing);

	g_Au.Settings().writeEntry("DNASettings/HatA", headClothing);
	UpdateView();
}

void AuDNASettings::BHatClicked()
{
	COMMODITYID clothingCIDs[] =
	{
		CID_Invalid,
		CID_LeatherHelmet,
		CID_WovenHelmet,
		CID_PlasticHelmet,
		CID_MetalHelmet,
		CID_VacSuitHelmet,
		CID_MetalPowerArmorHelmet,
		CID_MagmiumPowerArmorHelmet,
		CID_VulciumPowerArmorHelmet,
	};
	int clothingCIDcount = sizeof(clothingCIDs) / sizeof(COMMODITYID);

	QMenu menu(this);
	for(int c=0; c<clothingCIDcount; c++)
	{
		COMMODITYID cid = clothingCIDs[c];
		QAction* a;
		if(cid == CID_Invalid)
			a = menu.addAction(QIcon(":data/SlotHead.png"), tr("Uncovered"));
		else
			a = menu.addAction(QIcon(g_Au.CommodityBitmapName(cid)), g_Au.CommodityName(cid));
		a->setData(cid);
	}

	QAction* action = menu.exec(QCursor::pos());
	if(!action)
		return;

	COMMODITYID headClothing = (COMMODITYID)action->data().toInt();
	if(headClothing == m_BHat->Value())
		return;

	if(headClothing == CID_Invalid)
		m_BHat->setIcon(QIcon(":data/SlotHead.png"));
	else
		m_BHat->setIcon(QIcon(g_Au.CommodityBitmapName(headClothing)));
	m_BHat->SetValue(headClothing);

	g_Au.Settings().writeEntry("DNASettings/HatB", headClothing);
	UpdateView();
}

void AuDNASettings::AClothesClicked()
{
	COMMODITYID clothingCIDs[] =
	{
		CID_Invalid,
		CID_LeatherClothes,
		CID_JeansAndShirt,
		CID_CapriHalterOutfit,
		CID_GreyBDU,
		CID_KhakiBDU,
		CID_OliveDrabBDU,
		CID_BlackUniform,
		CID_BlueUniform,
		CID_BrownUniform,
		CID_GoldUniform,
		CID_GreenUniform,
		CID_GreyUniform,
		CID_OrangeUniform,
		CID_PinkUniform,
		CID_RedUniform,
		CID_TealUniform,
		CID_WhiteUniform,
		CID_ParcXUniform,
	};
	int clothingCIDcount = sizeof(clothingCIDs) / sizeof(COMMODITYID);

	QMenu menu(this);
	for(int c=0; c<clothingCIDcount; c++)
	{
		COMMODITYID cid = clothingCIDs[c];
		QAction* a;
		if(cid == CID_Invalid)
			a = menu.addAction(QIcon(":data/SlotBodyWear.png"), tr("Naked"));
		else
			a = menu.addAction(QIcon(g_Au.CommodityBitmapName(cid)), g_Au.CommodityName(cid));
		a->setData(cid);
	}

	QAction* action = menu.exec(QCursor::pos());
	if(!action)
		return;

	COMMODITYID bodyClothing = (COMMODITYID)action->data().toInt();
	if(bodyClothing == m_AClothes->Value())
		return;

	if(bodyClothing == CID_Invalid)
		m_AClothes->setIcon(QIcon(":data/SlotBodyWear.png"));
	else
		m_AClothes->setIcon(QIcon(g_Au.CommodityBitmapName(bodyClothing)));
	m_AClothes->SetValue(bodyClothing);

	g_Au.Settings().writeEntry("DNASettings/ClothesA", bodyClothing);
	UpdateView();
}

void AuDNASettings::BClothesClicked()
{
	COMMODITYID clothingCIDs[] =
	{
		CID_Invalid,
		CID_LeatherClothes,
		CID_JeansAndShirt,
		CID_CapriHalterOutfit,
		CID_GreyBDU,
		CID_KhakiBDU,
		CID_OliveDrabBDU,
		CID_BlackUniform,
		CID_BlueUniform,
		CID_BrownUniform,
		CID_GoldUniform,
		CID_GreenUniform,
		CID_GreyUniform,
		CID_OrangeUniform,
		CID_PinkUniform,
		CID_RedUniform,
		CID_TealUniform,
		CID_WhiteUniform,
		CID_ParcXUniform,
	};
	int clothingCIDcount = sizeof(clothingCIDs) / sizeof(COMMODITYID);

	QMenu menu(this);
	for(int c=0; c<clothingCIDcount; c++)
	{
		COMMODITYID cid = clothingCIDs[c];
		QAction* a;
		if(cid == CID_Invalid)
			a = menu.addAction(QIcon(":data/SlotBodyWear.png"), tr("Naked"));
		else
			a = menu.addAction(QIcon(g_Au.CommodityBitmapName(cid)), g_Au.CommodityName(cid));
		a->setData(cid);
	}

	QAction* action = menu.exec(QCursor::pos());
	if(!action)
		return;

	COMMODITYID bodyClothing = (COMMODITYID)action->data().toInt();
	if(bodyClothing == m_BClothes->Value())
		return;

	if(bodyClothing == CID_Invalid)
		m_BClothes->setIcon(QIcon(":data/SlotBodyWear.png"));
	else
		m_BClothes->setIcon(QIcon(g_Au.CommodityBitmapName(bodyClothing)));
	m_BClothes->SetValue(bodyClothing);

	g_Au.Settings().writeEntry("DNASettings/ClothesB", bodyClothing);
	UpdateView();
}

void AuDNASettings::AArmorClicked()
{
	COMMODITYID clothingCIDs[] =
	{
		CID_Invalid,
		CID_LeatherBodyArmor,
		CID_WovenBodyArmor,
		CID_PlasticBodyArmor,
		CID_MetalBodyArmor,
		CID_VacSuit,
		CID_MetalPowerArmor,
		CID_MagmiumPowerArmor,
		CID_VulciumPowerArmor,
	};
	int clothingCIDcount = sizeof(clothingCIDs) / sizeof(COMMODITYID);

	QMenu menu(this);
	for(int c=0; c<clothingCIDcount; c++)
	{
		COMMODITYID cid = clothingCIDs[c];
		QAction* a;
		if(cid == CID_Invalid)
			a = menu.addAction(QIcon(":data/SlotBodyWear.png"), tr("Unarmored"));
		else
			a = menu.addAction(QIcon(g_Au.CommodityBitmapName(cid)), g_Au.CommodityName(cid));
		a->setData(cid);
	}

	QAction* action = menu.exec(QCursor::pos());
	if(!action)
		return;

	COMMODITYID bodyClothing = (COMMODITYID)action->data().toInt();
	if(bodyClothing == m_AArmor->Value())
		return;

	if(bodyClothing == CID_Invalid)
		m_AArmor->setIcon(QIcon(":data/SlotBodyWear.png"));
	else
		m_AArmor->setIcon(QIcon(g_Au.CommodityBitmapName(bodyClothing)));
	m_AArmor->SetValue(bodyClothing);

	g_Au.Settings().writeEntry("DNASettings/ArmorA", bodyClothing);
	UpdateView();
}

void AuDNASettings::BArmorClicked()
{
	COMMODITYID clothingCIDs[] =
	{
		CID_Invalid,
		CID_LeatherBodyArmor,
		CID_WovenBodyArmor,
		CID_PlasticBodyArmor,
		CID_MetalBodyArmor,
		CID_VacSuit,
		CID_MetalPowerArmor,
		CID_MagmiumPowerArmor,
		CID_VulciumPowerArmor,
	};
	int clothingCIDcount = sizeof(clothingCIDs) / sizeof(COMMODITYID);

	QMenu menu(this);
	for(int c=0; c<clothingCIDcount; c++)
	{
		COMMODITYID cid = clothingCIDs[c];
		QAction* a;
		if(cid == CID_Invalid)
			a = menu.addAction(QIcon(":data/SlotBodyWear.png"), tr("Unarmored"));
		else
			a = menu.addAction(QIcon(g_Au.CommodityBitmapName(cid)), g_Au.CommodityName(cid));
		a->setData(cid);
	}

	QAction* action = menu.exec(QCursor::pos());
	if(!action)
		return;

	COMMODITYID bodyClothing = (COMMODITYID)action->data().toInt();
	if(bodyClothing == m_BArmor->Value())
		return;

	if(bodyClothing == CID_Invalid)
		m_BArmor->setIcon(QIcon(":data/SlotBodyWear.png"));
	else
		m_BArmor->setIcon(QIcon(g_Au.CommodityBitmapName(bodyClothing)));
	m_BArmor->SetValue(bodyClothing);

	g_Au.Settings().writeEntry("DNASettings/ArmorB", bodyClothing);
	UpdateView();
}

void AuDNASettings::SaveDNA()
{
	QString lastFile = g_Au.Settings().readEntry("DNASettings/LastFile", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();

	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

	QString saveFilter;
	saveFilter += QString("Hazeron DNA (*.dna)");
	saveFilter += QString(";;All Files (*.*)");

	QString saveFile = QFileDialog::getSaveFileName(this, tr("Hazeron DNA File"), lastFile, saveFilter, 0,
			QFileDialog::DontUseNativeDialog);
	if(saveFile.isEmpty())
		return;

	// Insure the file has an appropriate suffix.
	QString suffix(".dna");
	if(saveFile.right(suffix.length()).compare(suffix, Qt::CaseInsensitive) != 0)
		saveFile += suffix;

	g_Au.Settings().writeEntry("DNASettings/LastFile", saveFile);

	int result = m_DNA.SaveFile(saveFile);
	if(result == 1)
	{
		QString smsg = QObject::tr("Unable to open file for writing.\n\n%1").arg(saveFile);
		QMessageBox::warning(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok);
	}
}

void AuDNASettings::LoadDNA()
{
	QString lastFile = g_Au.Settings().readEntry("DNASettings/LastFile", "");
	if(lastFile.isEmpty())
		lastFile = QDir::currentPath();

	QFileInfo lastFileInfo(lastFile);
	if(!lastFileInfo.exists())
		lastFile = lastFileInfo.absolutePath();	// Last file does not exist. Drop the file name and just go to the last folder.

	QString loadFilter;
	loadFilter += QString("Hazeron DNA (*.dna)");
	loadFilter += QString(";;All Files (*.*)");

	QString loadFile = QFileDialog::getOpenFileName(this, tr("Hazeron DNA File"), lastFile, loadFilter, 0,
			QFileDialog::DontUseNativeDialog);
	if(loadFile.isEmpty())
		return;

	g_Au.Settings().writeEntry("DNASettings/LastFile", loadFile);

	int result = m_DNA.LoadFile(loadFile);
	switch(result)
	{
	case 0:	// ok
		{
			ListChromosomes();
			m_View->SetDNAPair(m_DNA, 3);
			UpdateView();
		}
		break;
	case 1:	// cannot open file
		{
			QString smsg = QObject::tr("Unable to open file for reading.\n\n%1").arg(loadFile);
			QMessageBox::warning(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok);
			break;
		}
	case 2:	// bad file version
		{
			QString smsg = QObject::tr("File does not contain Hazeron DNA.\n\n%1").arg(loadFile);
			QMessageBox::warning(this, g_Au.ApplicationName(), smsg, QMessageBox::Ok);
			break;
		}
	}
}

void AuDNASettings::SetShowGround(int index)
{
	int showGround = m_ShowGround->itemData(index).toInt();
	m_View->SetShowGround(showGround);

	g_Au.Settings().writeEntry("DNASettings/Ground", showGround);
}

void AuDNASettings::Wizard()
{
	AuDNAWizard wiz(this);
	if(wiz.exec() == QDialog::Accepted)
	{
		m_DNA = wiz.DNA();
		ListChromosomes();
		m_View->SetDNAPair(m_DNA, 3);
		//m_View->ResetView();
		UpdateView();
	}
}

void AuDNASettings::Random()
{
	//DHH
}

void AuDNASettings::SetDNA(const AuDNA& dna)
{
	m_DNA = dna;
	ListChromosomes();
	m_View->SetDNAPair(m_DNA, 3);
	//m_View->ResetView();
	UpdateView();
}

bool AuDNASettings::OkForAvatar()
{
	int checkFemale = m_View->AvatarCheckFemale();
	int checkMale = m_View->AvatarCheckMale();
	return !checkFemale && !checkMale;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuDNASettingsDlg::AuDNASettingsDlg(QWidget* parent) : AuDialog(parent)
{
	setAuDlgTitle(tr("DNA"));
	setAuDlgIcon(QIcon(":data/DNA.png"));

	m_DNASettings = new AuDNASettings(this);
	connect(m_DNASettings, SIGNAL(StatusUpdated()), SLOT(EnableControls()));

	m_OkButton = new QPushButton(tr("Ok"), this);
	m_OkButton->setDefault(true);
	m_OkButton->setAutoDefault(true);
	m_OkButton->setToolTip(tr("<html>"
				"<p>"
				"Accepts the DNA. "
				"</p>"
				"<p>"
				"This button is disabled if either chromatid is not valid for use as an avatar. "
				"Colored status values indicate when there is a problem. "
				"</p>"
				"</html>"));
	connect(m_OkButton, SIGNAL(clicked()), SLOT(OkClicked()));
	
	QHBoxLayout* blay = new QHBoxLayout;
	blay->addStretch();
	blay->addWidget(m_OkButton);

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(m_DNASettings);
	lay->addLayout(blay);
	setAuDlgLayout(lay);
}

AuDNASettingsDlg::~AuDNASettingsDlg()
{
}

void AuDNASettingsDlg::setVisible(bool visible)
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

void AuDNASettingsDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/DNASettingsDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/DNASettingsDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/DNASettingsDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/DNASettingsDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuDNASettingsDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/DNASettingsDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/DNASettingsDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/DNASettingsDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/DNASettingsDlg/SizeY",sz.height());
}

void AuDNASettingsDlg::EnableControls()
{
	m_OkButton->setEnabled(m_DNASettings->OkForAvatar());
}

const AuDNA& AuDNASettingsDlg::DNA() const
{
	return m_DNASettings->DNA();
}

void AuDNASettingsDlg::SetDNA(const AuDNA& dna)
{
	m_DNASettings->SetDNA(dna);
}

void AuDNASettingsDlg::OkClicked()
{
	if(m_DNASettings->OkForAvatar())
		accept();
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

