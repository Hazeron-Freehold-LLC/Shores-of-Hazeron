//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: ausctexturecylindricaldlg.h,v 1.2 2022/02/19 18:22:50 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScTextureCylindricalDlg_H
#define AuScTextureCylindricalDlg_H

#include "audesign.h"
#include "audialog.h"
#include "augleffects.h"
#include "dbcommodity.h"

class QLabel;
class QLineEdit;
class QListWidget;
class QListWidgetItem;

class AuScTextureCylindricalDlg : public AuDialog
{
	Q_OBJECT;
private:
	COMMODITYID							m_HullModule;
	AuDesignTex							m_Palette[SCTEXI_Count];

	// Widgets
	QListWidget*						m_TextureList;
	QLabel*								m_TextureSample;
	QLineEdit*							m_TextureReps;
	QLineEdit*							m_TextureSize;

public:
	AuScTextureCylindricalDlg(COMMODITYID hullModule, const AuDesignTex palette[SCTEXI_Count], QWidget* parent=0);
	virtual ~AuScTextureCylindricalDlg();

	// Returns SCTEXI_Count if none.
	SC_TEXTURE_INDEX TextureIndex() const;

	quint16 TextureReps() const;

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

#endif // AuScTextureCylindricalDlg_H
