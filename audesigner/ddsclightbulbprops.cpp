//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ddsclightbulbprops.cpp,v 1.7 2022/02/19 18:27:14 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "ddsclightbulbprops.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "ausclightprop.h"
#include "audesignerview.h"
#include "dadesigner.h"
#include "daperson.h"
#include <QMouseEvent>

DdScLightBulbProps::DdScLightBulbProps(AuDesignerView* view) : DdCommand(view)
{
}

DdScLightBulbProps::~DdScLightBulbProps()
{

}

QString DdScLightBulbProps::WikiTopic() const
{
	return "Designer_Menu#Light_Bulb";
}

void DdScLightBulbProps::Start()
{
	DdCommand::Start();

	// Configure defaults.
	bool isOmni = false;
	QColor color;
	float attenMeters;
	float innerAngle;
	float outerAngle;
	if(m_pWnd->DesignMode() == DaDesignBase::DESIGN_Spacecraft)
	{
		color = AuGLLight::ColorCarbonArc();
		attenMeters = 200;
		innerAngle = 20;
		outerAngle = 60;
	}
	else if(m_pWnd->DesignMode() == DaDesignBase::DESIGN_Building)
	{
		color = AuGLLight::ColorSodiumVapor();
		attenMeters = 10;
		innerAngle = 30;
		outerAngle = 80;
	}
	else
	{
		color = AuGLLight::ColorHalogen();
		attenMeters = 10;
		innerAngle = 30;
		outerAngle = 60;
	}

	// Get current properties from selection.
	DaPerson* person;
	m_pWnd->LockSceneAtom(&person);
	if(person)
	{
		const DaDesigner* pde = person->FindParentDesignerInEnv();
		if(pde)
		{
			float kL;
			float kQ;
			float cosInnerCone;
			float cosOuterCone;
			pde->PartSelectionLightBulbProperties(m_pWnd->DesignerPartSelection(), color, kL, kQ, cosOuterCone, cosInnerCone);
			float attenFt = sqrtf(1/kQ);
			attenMeters = attenFt / FeetPerMeter();

			if(cosInnerCone >= -1)
				innerAngle = acosf(cosInnerCone) * RAD2DEGf;
			else
				innerAngle = 180;

			if(cosOuterCone >= -1)
				outerAngle = acosf(cosOuterCone) * RAD2DEGf;
			else
				outerAngle = 180;

			isOmni = innerAngle == 180 && outerAngle == 180;
		}
	}
	m_pWnd->UnlockSceneAtom();

	AuScLightProp dlg(isOmni, color, attenMeters, innerAngle, outerAngle, m_pWnd);
	if(auApp->Frame()->ExecDialog(dlg) == QDialog::Accepted)
	{
		float atten = dlg.AttenuationDistance() * FeetPerMeter();
		if(atten > 0)
		{
			QColor color = dlg.LightColor();
			float kL = .005f;
			float kQ = 1/(atten * atten);
			float cosOuterCone, cosInnerCone;
			if(dlg.IsSpotlight())
			{
				if(dlg.OuterConeAngle() < 1000)
					cosOuterCone = cosf(dlg.OuterConeAngle() * DEG2RADf);
				else
					cosOuterCone = cosf(outerAngle * DEG2RADf);
				if(dlg.InnerConeAngle() < 1000)
					cosInnerCone = cosf(dlg.InnerConeAngle() * DEG2RADf);
				else
					cosInnerCone = cosf(innerAngle * DEG2RADf);
			}
			else
			{
				cosOuterCone = -1;
				cosInnerCone = -1;
			}
			auApp->DesignerPartsSetLightBulbProperties(m_pWnd->DesignerPartSelection(), color, kL, kQ, cosOuterCone, cosInnerCone);
		}
	}

	// We only take one shot at this.
	Terminate();
}

void DdScLightBulbProps::UpdatePrompt() const
{
	QString prompt = QObject::tr("<html>"
			"<span style=\"color: %2;\">Configure light bulb properties. "
			"</span>"
			"<span style=\"color: rgb(255, 255, 0);\">?%1Help, </span>"
			"<span style=\"color: rgb(255, 255, 0);\">Esc%1Stop. </span>"
			"</html>")
		.arg('=')
		.arg(m_pWnd->palette().color(QPalette::WindowText).name());
	m_pWnd->CommandPromptChanged(prompt);
}

bool DdScLightBulbProps::OnKeyDown(QKeyEvent* e)
{
	// Pass it down so the coordinate input bar gets first crack at it. The only letters it consumes are A, P and R.
	if(DdCommand::OnKeyDown(e))
		return true;
	return false;
}
