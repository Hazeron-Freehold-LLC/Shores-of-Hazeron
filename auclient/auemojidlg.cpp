//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auemojidlg.cpp,v 1.15 2022/02/19 18:10:41 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auemojidlg.h"
//
#include "auapp.h"
#include "auclientglobal.h"
#include "auframe.h"
#include "aupushbutton.h"
#include "autoolbutton.h"
#include "dasentient.h"
#include <QComboBox>
#include <QGridLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QSpinBox>

#define d_Rows 7
#define d_Cols 6

AuEmojiDlg::AuEmojiDlg(QWidget* parent) : AuDialog(parent), m_Emojis(g_Au.Emojis())
{
	setAuDlgTitle(tr("Emojis"));
	setAuDlgIcon(QIcon(":data/Emoji.png"));
	SetWikiTopic("Emoji");

	QList<int> favorites = g_Au.Settings().readIntListEntry("Widget/EmojiDlg/Favorites");

	// Determine where the page breaks are.
	m_Group = new QComboBox(this);
	int perPage = d_Rows * d_Cols;
	int onpage = 0;
	int index = 0;
	QString group;
	for(AuEmojiList::const_iterator ei = m_Emojis.cbegin(); ei != m_Emojis.cend(); ei++)
	{
		if(group != (*ei).Group())
		{
			// Group changed or this is the first time through.
			group = (*ei).Group();
			onpage = 0;
			m_PageBreaks.append(index);
			m_Group->addItem(group, m_PageBreaks.size());	// group name with page number
		}
		else if(onpage+1 == perPage)
		{
			// Reached the end of a page.
			onpage = 0;
			m_PageBreaks.append(index);
		}
		else
		{
			// Group didn't change and end of page not reached.
			onpage++;
		}
		index++;
	}
	connect(m_Group, SIGNAL(currentIndexChanged(int)), SLOT(GroupChanged(int)));

	int page = g_Au.Settings().readNumEntry("Widget/EmojiDlg/Page",1);
	if(page < 1 || page > m_PageBreaks.size())
		page = 1;
	m_PageSpin = new QSpinBox(this);
	m_PageSpin->setRange(1,m_PageBreaks.size());
	m_PageSpin->setValue(page);
	m_PageSpin->setToolTip(tr("<html>"
				"Page Number"
				"</html>"));
	connect(m_PageSpin, SIGNAL(valueChanged(int)), SLOT(PageChanged(int)));

	QGridLayout* elay = new QGridLayout;
	index = 0;
	for(int row=0; row<d_Rows; row++)
	{
		for(int col=0; col<d_Cols; col++)
		{
			AuPushButtonExpandValue* tb = new AuPushButtonExpandValue(index, this);
			tb->SetFocusPolicy(Qt::NoFocus);
			tb->SetIconSize(QSize(20,20));	// Affects the size of the expand indicator.
			connect(tb, SIGNAL(Clicked(int)), SLOT(EmojiClicked(int)));
			connect(tb, SIGNAL(DoubleClicked(int)), SLOT(EmojiDoubleClicked(int)));
			connect(tb, SIGNAL(ExpandClicked(int)), SLOT(EmojiExpandClicked(int)));
			m_ButtonGrid.insert(index, tb);
			elay->addWidget(tb, row,col);
			index++;
		}
	}
	elay->setColumnStretch(d_Cols, 1);

	QPushButton* pbdone = new QPushButton(tr("Done"), this);
	pbdone->setDefault(true);
	pbdone->setAutoDefault(true);
	pbdone->setToolTip(tr("<html>"
				"Closes this emoji window. "
				"You can also double click an emoji to close this window. "
				"</html>"));
	connect(pbdone, SIGNAL(clicked()), SLOT(accept()));

	m_BLay = new QHBoxLayout;
	m_BLay->addWidget(m_Group);
	m_BLay->addWidget(m_PageSpin);
	m_BLay->addStretch();
	m_BLay->addWidget(pbdone);

	for(QList<int>::const_iterator fi = favorites.cbegin(); fi != favorites.cend(); fi++)
	{
		int ei = *fi;
		AuPushButtonExpandValue* tb = new AuPushButtonExpandValue(m_Favorites.size(), this);	// value is m_Favorites index
		tb->SetFocusPolicy(Qt::NoFocus);
		AuVisage::VISAGE face = m_Emojis[ei].Visage();
		if(face != AuVisage::Invalid)
		{
			tb->SetStyleSheet(QString("color: #ffffff;"));
			tb->SetText(QString("%1").arg(m_Emojis[ei].Emoji()));
			tb->SetToolTip(QString("<html>%1<br><small>Visage</small> %2</html>").arg(m_Emojis[ei].Tag())
					.arg(AuVisage(face).Name()));
		}
		else
		{
			tb->SetStyleSheet(QString(""));
			tb->SetText(m_Emojis[ei].Emoji());
			tb->SetToolTip(m_Emojis[ei].Tag());
		}
		connect(tb, SIGNAL(Clicked(int)), SLOT(FavoriteClicked(int)));
		connect(tb, SIGNAL(DoubleClicked(int)), SLOT(FavoriteDoubleClicked(int)));
		connect(tb, SIGNAL(ExpandClicked(int)), SLOT(FavoriteExpandClicked(int)));
		m_Favorites.append(tb);
		m_BLay->addWidget(tb);
	}

	QVBoxLayout* lay = new QVBoxLayout;
	lay->addStretch(10);
	lay->addLayout(elay);
	lay->addLayout(m_BLay);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuEmojiDlg::~AuEmojiDlg()
{
}

void AuEmojiDlg::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
		UpdateButtons();
	}
	else if(isVisible())
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuEmojiDlg::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/EmojiDlg/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/EmojiDlg/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/EmojiDlg/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/EmojiDlg/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuEmojiDlg::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/EmojiDlg/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/EmojiDlg/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/EmojiDlg/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/EmojiDlg/SizeY",sz.height());

	g_Au.Settings().writeEntry("Widget/EmojiDlg/Page",m_PageSpin->value());

	QList<int> favs;
	for(QList<AuPushButtonExpandValue*>::const_iterator fi = m_Favorites.cbegin(); fi != m_Favorites.cend(); fi++)
		if(!(*fi)->isHidden())
		{
			QString emoji = (*fi)->Text();
			int ei = m_Emojis.EmojiIndex(emoji);
			if(ei >= 0)
				favs.append(ei);
		}
	g_Au.Settings().writeEntry("Widget/EmojiDlg/Favorites", favs);
}

void AuEmojiDlg::UpdateButtons()
{
	// Determine range of emojis on the page.
	int pageIndex = m_PageSpin->value() - 1;
	int firstIndex = m_PageBreaks[pageIndex];		// emoji index
	int lastIndex;
	if(pageIndex == m_PageBreaks.size() - 1)
		lastIndex = m_Emojis.size() - 1;
	else
		lastIndex = m_PageBreaks[pageIndex + 1] - 1;

	m_Group->blockSignals(true);
	m_Group->setCurrentText(m_Emojis[firstIndex].Group());
	m_Group->blockSignals(false);

	int index = 0;
	for(int ei = firstIndex; ei <= lastIndex; ei++)
	{
		UpdateButton(ei);
		m_ButtonGrid[index]->setVisible(true);
		index++;
	}

	// Hide remaining buttons.
	int perPage = d_Rows * d_Cols;
	for(;index < perPage; index++)
		m_ButtonGrid[index]->setVisible(false);
}

void AuEmojiDlg::EmojiClicked(int index)
{
	if(m_ButtonGrid[index]->Text().isEmpty())
		return;	// blank button
	int pageIndex = m_PageSpin->value() - 1;
	int ei = m_PageBreaks[pageIndex] + index;		// emoji index
	emit Emoji(m_Emojis[ei].Emoji());
}

void AuEmojiDlg::EmojiDoubleClicked(int index)
{
	Q_UNUSED(index);
	accept();
}

void AuEmojiDlg::EmojiExpandClicked(int index)
{
	if(m_ButtonGrid[index]->Text().isEmpty())
		return;	// blank button
	m_WhichEmoji = m_PageBreaks[m_PageSpin->value()-1] + index;	// emoji index
	QMenu menu;
	menu.addAction(tr("Add to Favorites"), this, SLOT(AddFavorite()));
	menu.addAction(tr("Change Tag..."), this, SLOT(ChangeEmojiTag()));
	menu.addAction(tr("Restore Tag..."), this, SLOT(RestoreEmojiTag()));
	menu.addAction(tr("Change Visage..."), this, SLOT(ChangeEmojiVisage()));
	menu.addAction(tr("Restore Visage..."), this, SLOT(RestoreEmojiVisage()));
	menu.addAction(tr("Clear Visage..."), this, SLOT(ClearEmojiVisage()));
	menu.exec(QCursor::pos());
}

void AuEmojiDlg::PageChanged(int pageNumber)
{
	Q_UNUSED(pageNumber);
	UpdateButtons();
}

void AuEmojiDlg::GroupChanged(int index)
{
	int pageNumber = m_Group->itemData(index).toInt();
	m_PageSpin->blockSignals(true);
	m_PageSpin->setValue(pageNumber);
	m_PageSpin->blockSignals(false);
	UpdateButtons();
}

void AuEmojiDlg::AddFavorite()
{
	int ei = m_WhichEmoji;
	AuPushButtonExpandValue* tb = new AuPushButtonExpandValue(m_Favorites.size(), this);	// value is m_Favorites index
	tb->SetFocusPolicy(Qt::NoFocus);
	AuVisage::VISAGE face = m_Emojis[ei].Visage();
	if(face != AuVisage::Invalid)
	{
		tb->SetStyleSheet(QString("color: #ffffff;"));
		tb->SetText(QString("%1").arg(m_Emojis[ei].Emoji()));
		tb->SetToolTip(QString("<html>%1<br><small>Visage</small> %2</html>").arg(m_Emojis[ei].Tag())
				.arg(AuVisage(face).Name()));
	}
	else
	{
		tb->SetStyleSheet(QString(""));
		tb->SetText(m_Emojis[ei].Emoji());
		tb->SetToolTip(m_Emojis[ei].Tag());
	}
	connect(tb, SIGNAL(Clicked(int)), SLOT(FavoriteClicked(int)));
	connect(tb, SIGNAL(DoubleClicked(int)), SLOT(FavoriteDoubleClicked(int)));
	connect(tb, SIGNAL(ExpandClicked(int)), SLOT(FavoriteExpandClicked(int)));
	m_Favorites.append(tb);
	m_BLay->addWidget(tb);
}

void AuEmojiDlg::ChangeEmojiTag()
{
	// Get the current tag for the emoji.
	int ei = m_WhichEmoji;
	QString curTag = m_Emojis[ei].Tag();

	// Request a new different tag from the user.
	bool ok;
	QString tag = QInputDialog::getText(this, g_Au.ApplicationName(),
			tr("<html><p>"
				"Enter a new tag for the emoji. "
				"The new tag cannot be empty. "
				"Tags are case sensitive. "
				"</p>"
				"<p>"
				"The current tag is shown. "
				"</p></html>"), QLineEdit::Normal, curTag, &ok).trimmed();
	if(!ok || tag.isEmpty() || tag == curTag)
		return;

	// Change the tag in my emojis list and the global list.
	m_Emojis[ei].SetTag(tag);
	EmojisChanged();

	// Update the tool tip on the button grid.
	UpdateButton(ei);
}

void AuEmojiDlg::RestoreEmojiTag()
{
	// Get the original tag for the emoji.
	int ei = m_WhichEmoji;
	AuEmojiList emojis;
	emojis.LoadStandardEmojis();
	QString origTag = emojis[ei].Tag();

	QString smsg = tr("<html>The original tag is <b>%1</b>. "
			"Are you sure you want to restore the tag?"
			"</html>").arg(origTag);
	QMessageBox mb(g_Au.ApplicationName(), smsg, QMessageBox::Question, QMessageBox::Yes, QMessageBox::Cancel,
			QMessageBox::NoButton, this);
	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Yes)
		return;

	// Change the tag in my emojis list and the global list.
	m_Emojis[ei].SetTag(origTag);
	EmojisChanged();

	// Update the tool tip on the button grid.
	UpdateButton(ei);
}

void AuEmojiDlg::FavoriteClicked(int favIndex)
{
	QString emoji = m_Favorites[favIndex]->Text();
	emit Emoji(emoji);
}

void AuEmojiDlg::FavoriteDoubleClicked(int favIndex)
{
	Q_UNUSED(favIndex);
	accept();
}

void AuEmojiDlg::FavoriteExpandClicked(int favIndex)
{
	m_WhichEmoji = favIndex;
	QMenu menu;
	menu.addAction(tr("Remove from Favorites"), this, SLOT(RemoveFavorite()));
	menu.exec(QCursor::pos());
}

void AuEmojiDlg::RemoveFavorite()
{
	m_Favorites[m_WhichEmoji]->hide();
}

void AuEmojiDlg::ChangeEmojiVisage()
{
	// Get the current visage for the emoji.
	int ei = m_WhichEmoji;
	AuVisage::VISAGE curFace = m_Emojis[ei].Visage();
	QString faceEx;
	if(curFace == AuVisage::Invalid)
		faceEx = tr("None");
	else
		faceEx = AuVisage(curFace).Name();

	// Request a new different visage from the user.
	QStringList expressions;
	for(int f=0; f<AuVisage::FACE_Count; f++)
	{
		QString exname;
		if(f)
			exname = AuVisage((AuVisage::VISAGE)f).Name();
		else
			exname = tr("None");
		expressions.append(exname);
	}

	bool ok;
	QString fex = QInputDialog::getItem(this, g_Au.ApplicationName(),
			tr("<html><p>"
				"Choose a new visage for the emoji. "
				"A visage is optional. "
				"</p>"
				"<p>"
				"Choose <b>none</b> to clear the visage. "
				"When cleared, the emoji has no effect on your avatar's facial expression. "
				"</p>"
				"<p>"
				"The current visage is shown. "
				"</p></html>"), expressions, curFace, false, &ok).trimmed();
	if(!ok || fex.isEmpty() || fex == faceEx)
		return;

	AuVisage::VISAGE face = (AuVisage::VISAGE)expressions.indexOf(fex);

	// Change the visage in my emojis list and the global list.
	m_Emojis[ei].SetVisage(face);
	EmojisChanged();

	// Update the tool tip on the button grid.
	UpdateButton(ei);
}

void AuEmojiDlg::RestoreEmojiVisage()
{
	// Get the original visage for the emoji.
	int ei = m_WhichEmoji;
	AuEmojiList emojis;
	emojis.LoadStandardEmojis();
	AuVisage::VISAGE origFace = emojis[ei].Visage();
	QString faceEx;
	if(origFace == AuVisage::Invalid)
		faceEx = "none";
	else
		faceEx = AuVisage(origFace).Name();

	QString smsg = tr("<html>The original visage is <b>%1</b>. "
			"Are you sure you want to restore the visage?"
			"</html>").arg(faceEx);
	QMessageBox mb(g_Au.ApplicationName(), smsg, QMessageBox::Question, QMessageBox::Yes, QMessageBox::Cancel,
			QMessageBox::NoButton, this);
	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Yes)
		return;

	// Change the visage in my emojis list and the global list.
	m_Emojis[ei].SetVisage(origFace);
	EmojisChanged();

	// Update the tool tip on the button grid.
	UpdateButton(ei);
}

void AuEmojiDlg::ClearEmojiVisage()
{
	// Get the current visage for the emoji.
	int ei = m_WhichEmoji;
	AuVisage::VISAGE curFace = m_Emojis[ei].Visage();
	QString faceEx;
	if(curFace == AuVisage::Invalid)
		faceEx = "none";
	else
		faceEx = AuVisage(curFace).Name();

	QString smsg = tr("<html>The current visage is <b>%1</b>. "
			"Are you sure you want to clear the visage?"
			"</html>").arg(faceEx);
	QMessageBox mb(g_Au.ApplicationName(), smsg, QMessageBox::Question, QMessageBox::Yes, QMessageBox::Cancel,
			QMessageBox::NoButton, this);
	int result = auApp->Frame()->ExecMessageBox(mb);
	if(result != QMessageBox::Yes)
		return;

	// Change the tag in my emojis list and the global list.
	m_Emojis[ei].ClearVisage();
	EmojisChanged();

	// Update the tool tip on the button grid.
	UpdateButton(ei);
}

void AuEmojiDlg::UpdateButton(int emojiIndex)
{
	// Update the button on the grid.
	int gridIndex = emojiIndex - m_PageBreaks[m_PageSpin->value()-1];	// button grid index
	int fontSize = 32 - m_Emojis[emojiIndex].Emoji().length() * 2;
	if(fontSize < 12)
		fontSize = 12;
	AuVisage::VISAGE face = m_Emojis[emojiIndex].Visage();
	if(face != AuVisage::Invalid)
	{
		m_ButtonGrid[gridIndex]->SetStyleSheet(QString("font-size: %1px; margin: 0px; color: #ffffff;").arg(fontSize));
		m_ButtonGrid[gridIndex]->SetText(m_Emojis[emojiIndex].Emoji());
		m_ButtonGrid[gridIndex]->SetToolTip(QString("<html>%1<br><small>Visage</small> %2</html>").arg(m_Emojis[emojiIndex].Tag())
				.arg(AuVisage(face).Name()));
	}
	else
	{
		m_ButtonGrid[gridIndex]->SetStyleSheet(QString("font-size: %1px; margin: 0px;").arg(fontSize));
		m_ButtonGrid[gridIndex]->SetText(m_Emojis[emojiIndex].Emoji());
		m_ButtonGrid[gridIndex]->SetToolTip(m_Emojis[emojiIndex].Tag());
	}

	// Favorites.
	for(QList<AuPushButtonExpandValue*>::const_iterator fi = m_Favorites.cbegin(); fi != m_Favorites.cend(); fi++)
	{
		if(m_Emojis[emojiIndex].Emoji() != (*fi)->Text())
			continue;

		if(face != AuVisage::Invalid)
		{
			(*fi)->SetStyleSheet(QString("color: #ffffff;"));
			(*fi)->SetText(QString("%1").arg(m_Emojis[emojiIndex].Emoji()));
			(*fi)->SetToolTip(QString("<html>%1<br><small>Visage</small> %2</html>").arg(m_Emojis[emojiIndex].Tag())
					.arg(AuVisage(face).Name()));
		}
		else
		{
			(*fi)->SetStyleSheet(QString(""));
			(*fi)->SetText(m_Emojis[emojiIndex].Emoji());
			(*fi)->SetToolTip(m_Emojis[emojiIndex].Tag());
		}
	}
}

void AuEmojiDlg::EmojisChanged()
{
	auApp->SetEmojis(m_Emojis);
}

