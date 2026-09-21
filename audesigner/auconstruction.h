//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auconstruction.h,v 1.14 2022/02/19 18:21:24 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuConstruction_H
#define AuConstruction_H

#include "aubdreport.h"
#include "audockwidget.h"
#include "auglobal.h"
#include "auknownpatents.h"
#include "auplayeronlinestatus.h"
#include "auscparts.h"
#include "deconstructionprocess.h"
#include <QPushButton>
#include <QWidget>

class AuGear;
class AuIconButton;
class QCheckBox;
class QComboBox;
class QLabel;
class QStackedLayout;
class QTimer;

class AuSkyLineWidget : public QWidget
{
	Q_OBJECT;
private:
	quint32			m_BuildingCount;
	quint32			m_BuildingVolume;
	int				m_EnvType;			// 0=space, 1=desert, 2=plants, 3=water, 4=moon
	quint32			m_FenceCount;
	quint32			m_StreetLightCount;
	quint32			m_VolumeLimit;

public:
	AuSkyLineWidget(QWidget* parent=0);

	virtual QSize sizeHint() const;

	void SetBuildings(quint32 volume, quint32 buildingCount, quint32 fenceCount, quint32 streetLightCount, int envType);

	void SetVolumeLimit(quint32 vl);

protected:
	virtual void paintEvent(QPaintEvent* ev);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuConstructButton : public QPushButton
{
	Q_OBJECT;
	INDUSTRYID idi;
	CONSTRUCTIONPROCESSID cpid;
	AuScFence::STYLE		m_FenceStyle;
	QPixmap					m_Image;
	QString					m_Text;

public:
	// Bd
	AuConstructButton(QWidget* reciever, QWidget* parent, INDUSTRYID industryId);

	// Geo road or landscape
	AuConstructButton(const char* imageName, QWidget* reciever, QWidget* parent, QString sText, QString toolTip,
			INDUSTRYID industryId, CONSTRUCTIONPROCESSID conProcId);

	// Fence or wall
	AuConstructButton(QWidget* reciever, QWidget* parent, AuScFence::STYLE fence);

signals:
	void mouseMove(INDUSTRYID idi, CONSTRUCTIONPROCESSID cpid);
	void mousePress(INDUSTRYID idi, CONSTRUCTIONPROCESSID cpid);

protected:
	virtual void mouseMoveEvent(QMouseEvent *e);
	virtual void mousePressEvent(QMouseEvent *e);
	virtual void paintEvent(QPaintEvent* event);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuConstructionWidget : public QWidget
{
	Q_OBJECT;
private:
	COMMODITYID							m_BuildMaterial;
	bool								m_ExchangeRequested;
	quint32								m_ImageDesign;			// design id in the sample image.
	bool								m_IncludeExchange;
	bool								m_IsGasGiant;
	bool								m_IsQuarry;				// Quarry is a limited mine, based on the blueprint.
	INDUSTRYID							m_ListIndustryId;
	CONSTRUCTIONPROCESSID				m_ListCPID;				// Last construction process button rolled-over with the mouse.
	AuKnownPatents						m_Patents;				// Patents available to the builder.

	multimap<QString, AuBdReport>		m_BlueprintsEx;			// Building blueprints in the exchange.
	multimap<QString, AuBdReport>		m_BlueprintsGear;		// Building blueprints in their gear.

	// Widgets
	QLabel*								m_BlueprintInfo;
	QComboBox*							m_Blueprints;
	AuIconButton*						m_BlueprintExchange;
	AuIconButton*						m_BlueprintFilter;
	QComboBox*							m_cbProducts;
	QCheckBox*							m_ckWorker;
	AuIconButton*						m_Commerce;
	QWidget*							m_CommercePage;
	AuIconButton*						m_Fences;
	QWidget*							m_FencesPage;
	AuIconButton*						m_Food;
	QWidget*							m_FoodPage;
	AuIconButton*						m_Government;
	QWidget*							m_GovernmentPage;
	AuIconButton*						m_Housing;
	QWidget*							m_HousingPage;
	QLabel*								m_Image;
	QLabel*								m_IndustryLabel;
	AuIconButton*						m_Manufacture;
	QWidget*							m_ManufacturePage;
	QLabel*								m_ManufLabel;
	QLabel*								m_MaterialsLabel;
	AuIconButton*						m_MatlAdamantine;
	AuIconButton*						m_MatlLogs;
	AuIconButton*						m_MatlLumber;
	AuIconButton*						m_MatlStone;
	AuIconButton*						m_MatlMetal;
	AuIconButton*						m_MatlMagmium;
	AuIconButton*						m_MatlVulcium;
	AuIconButton*						m_Military;
	QWidget*							m_MilitaryPage;
	QStackedLayout*						m_Pages;
	QLabel*								m_ProductsLabel;
	AuIconButton*						m_Power;
	QWidget*							m_PowerPage;
	AuIconButton*						m_Roads;
	QWidget*							m_RoadsPage;
	QComboBox*							m_RoadType;
	QComboBox*							m_RoadWidth;
	QComboBox*							m_RoundAngle;
	AuSkyLineWidget*					m_SkyLine;
	AuIconButton*						m_Superstructure;
	QWidget*							m_SuperstructurePage;
	AuIconButton*						m_Tools;
	QWidget*							m_ToolsPage;
	QTimer*								m_Timer;
	AuIconButton*						m_Transport;
	QWidget*							m_TransportPage;
	AuConstructButton*					pbBank;
	AuConstructButton*					pbCapitol;
	AuConstructButton*					pbPoliceStation;

public:
	AuConstructionWidget(QWidget* parent=0);
	virtual ~AuConstructionWidget();

public slots:
	void BuildingSelected(INDUSTRYID industryId, CONSTRUCTIONPROCESSID cpid);
	void ListBuilding(INDUSTRYID industryId, CONSTRUCTIONPROCESSID conProcId);
	void ListBuildingHover(INDUSTRYID industryId, CONSTRUCTIONPROCESSID conProcId);

protected:
	void ChangeMaterial();
	bool CurrentBuildingReport(AuBdReport& report) const;	// Original with construction material changed to m_BuildMaterial.
	bool OriginalBuildingReport(AuBdReport& report) const;
	void FillManufacturingProcessesCombo(quint16 defaultMpid);
	virtual void hideEvent(QHideEvent* event);
	void ListDesigns(const AuGear& inv, INDUSTRYID industryId, multimap<QString, AuBdReport>& designs, const QSet<AuId>& excludePub,
			bool terLand, bool terSeaBottom, bool intendedIndustry, ORBITZONE ozone, bool gasGiant, bool breathableAtmo,
			bool enclosurePresent);
	void ListIndustryProcesses(INDUSTRYID industryId);
	virtual void showEvent(QShowEvent* event);
	void UpdateRoadSettings();

protected slots:
	void AdamantinePressed();
	void BlueprintChanged(int index);
	void BlueprintExchangePressed();
	void BlueprintFilterPressed();
	void ClearCratersPressed();
	void CommercePressed();
	void DestroyPressed();
	void EnableWorker(int enableWorker);
	void FencesPressed();
	void FoodPressed();
	void GovernmentPressed();
	void HousingPressed();
	void LogsPressed();
	void LumberPressed();
	void MagmiumPressed();
	void ManufacturingProcessSelected(int iIndex);
	void ManufacturePressed();
	void MetalPressed();
	void MilitaryPressed();
	void PowerPressed();
	void RoadsPressed();
	void RoadTypeChanged(const QString& text);
	void RoadWidthChanged(const QString& text);
	void RoundingAngleChanged(const QString& text);
	void SelectManufacturingProcess(quint16 mpid);
	void StonePressed();
	void StopPressed();
	void SuperstructurePressed();
	void ToolsPressed();
	void TransportPressed();
	void UpdateContents();
	void VulciumPressed();

signals:
	void PlanBuilding(INDUSTRYID industryId, const AuBdReport& report, COMMODITYID constructionMaterial, quint16 mpid,
			bool enableWorker);
	void PlanBuildingMaterial(COMMODITYID eMat);
	void PlanConstructionProcess(const AuBdReport& report);
	// 'fenceStyle' is an AuScFence::STYLE.
	// 'constructionMaterial' is only needed when making a custom fence design.
	void PlanFence(int fenceStyle, quint32 customDesignId, COMMODITYID constructionMaterial);
	void PlanLandscapeClearing();
	void PlanLandscapeDefoliate();
	void PlanLandscapeIrrigate();
	void PlanManufacturingProcess(int iProcessID);
	void PlanManufacturingWorkerOk(bool enableWorker);
	void PlanRoadAsphalt();
	void PlanRoadConcrete();
	void PlanRoadDirt();
	void PlanRoadGrade();
	void WindowHidden();	// emitted when this widget's visibility is set to hidden
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuConstructionDock : public AuDockWidget
{
public:
	AuConstructionWidget*				m_Content;

public:
	AuConstructionDock(QWidget* parent) : AuDockWidget(":data/ConstructionSign.png", tr("<b>Construction</b>"),
			tr("Construction"), parent)
	{
		SetWikiTopic("Construction");
		m_Content = new AuConstructionWidget(this);
		setWidget(m_Content);
		int dockSides = g_Au.Settings().readNumEntry("Dock/ConstructionAreas", (int)allowedAreas());
		setAllowedAreas((Qt::DockWidgetAreas)dockSides);
	}

	virtual void SetAllowedAreas(Qt::DockWidgetAreas areas)
	{
		setAllowedAreas(areas);
		g_Au.Settings().writeEntry("Dock/ConstructionAreas", (int)allowedAreas());
	}

	virtual void UpdateTitleShortcut();
};

#endif // AuConstruction_H
