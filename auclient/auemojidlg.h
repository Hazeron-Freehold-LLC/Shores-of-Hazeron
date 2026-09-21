//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auemojidlg.h,v 1.5 2022/02/19 17:55:42 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuEmojiDlg_H
#define AuEmojiDlg_H

#include "audialog.h"
#include "auemoji.h"

class AuPushButtonExpandValue;
class QComboBox;
class QHBoxLayout;
class QSpinBox;

class AuEmojiDlg : public AuDialog
{
	Q_OBJECT;
	AuEmojiList							m_Emojis;
	QList<int>							m_PageBreaks;			// Index into m_Emojis at each page break.
	int									m_WhichEmoji;			// Emoji index for menu actions.

	// Widgets
	QHBoxLayout*						m_BLay;
	QList<AuPushButtonExpandValue*>		m_ButtonGrid;
	QList<AuPushButtonExpandValue*>		m_Favorites;
	QComboBox*							m_Group;
	QSpinBox*							m_PageSpin;

public:
	AuEmojiDlg(QWidget* parent=0);
	virtual ~AuEmojiDlg();

public slots:
	virtual void setVisible(bool visible);

protected:
	void EmojisChanged();
	void RestoreGeometry();
	void SaveGeometry();
	void UpdateButtons();

protected slots:
	void AddFavorite();
	void ChangeEmojiTag();
	void ChangeEmojiVisage();
	void ClearEmojiVisage();
	void EmojiClicked(int index);
	void EmojiDoubleClicked(int index);
	void EmojiExpandClicked(int index);
	void FavoriteClicked(int favIndex);
	void FavoriteDoubleClicked(int favIndex);
	void FavoriteExpandClicked(int favIndex);
	void GroupChanged(int pageNumber);
	void PageChanged(int pageNumber);
	void RemoveFavorite();
	void RestoreEmojiTag();
	void RestoreEmojiVisage();
	void UpdateButton(int emojiIndex);

signals:
	void Emoji(QString emoji);
};

#endif // AuEmojiDlg_H
