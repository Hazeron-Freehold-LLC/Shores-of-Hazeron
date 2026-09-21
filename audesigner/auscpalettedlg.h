//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auscpalettedlg.h,v 1.2 2022/02/19 18:22:44 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuScPaletteDlg_H
#define AuScPaletteDlg_H

#include "audesign.h"
#include "audialog.h"
#include "augleffects.h"
#include "dbcommodity.h"

class QLabel;
class QListWidget;
class QListWidgetItem;

class AuScPaletteDlg : public AuDialog
{
	Q_OBJECT;
private:
	COMMODITYID							m_HullModule;
	AuDesignTex							m_Palette[SCTEXI_Count];
	bool								m_PaletteChanged;

	// Widgets
	QListWidget*						m_TextureList;
	QLabel*								m_TextureSample;

public:
	AuScPaletteDlg(COMMODITYID hullModule, const AuDesignTex palette[SCTEXI_Count], QWidget* parent=0);
	virtual ~AuScPaletteDlg();

	// Returns SCTEXI_Count if none.
	SC_TEXTURE_INDEX CurrentTextureIndex() const;

	inline const AuDesignTex* TexturePalette() const
	{
		return m_Palette;
	}

	inline bool TexturePaletteChanged() const
	{
		return m_PaletteChanged;
	}

public slots:
	void ChooseCustomTexture();
	void ChooseLibraryTexture();
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

#endif // AuScPaletteDlg_H
