//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausctextureprojectdlg.h,v 1.2 2022/02/19 18:22:50 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScTextureProjectDlg_H
#define AuScTextureProjectDlg_H

#include "audesign.h"
#include "audialog.h"
#include "augleffects.h"
#include "dbcommodity.h"

class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;

class AuScTextureProjectDlg : public AuDialog
{
	Q_OBJECT;
private:
	COMMODITYID							m_HullModule;
	AuDesignTex							m_Palette[SCTEXI_Count];

	// Widgets
	QListWidget*						m_TextureList;
	QLabel*								m_TextureSample;
	QLineEdit*							m_TextureSize;

public:
	AuScTextureProjectDlg(COMMODITYID hullModule, const AuDesignTex palette[SCTEXI_Count], QWidget* parent=0);
	virtual ~AuScTextureProjectDlg();

	// Returns SCTEXI_Count if none.
	SC_TEXTURE_INDEX TextureIndex() const;

	// Returns the size of the texture in feet.
	double TextureSize() const;

public slots:
	void OkClicked();
	virtual void setVisible(bool visible);

protected:
	void ListTextures(int curIdx);
	void RestoreGeometry();
	void SaveGeometry();
	void TextureSample(int sct);

protected slots:
	void TextureChanged(QListWidgetItem* current, QListWidgetItem* previous);
};

#endif // AuScTextureProjectDlg_H
