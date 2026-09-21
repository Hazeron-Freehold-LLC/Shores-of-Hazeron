//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auemoji.h,v 1.8 2022/02/19 18:30:31 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuEmoji_H
#define AuEmoji_H

#include "augloballib.h"
#include "aupose.h"

class AuVisage;
class QDebug;

class AuEmoji
{
	QString								m_Emoji;				// From emoji-test.txt. e.g. ⏰
	QString								m_Group;				// From emoji-test.txt. e.g. Travel & Places
	QString								m_Name;					// From emoji-test.txt. e.g. E0.6 alarm clock
	QString								m_Status;				// From emoji-test.txt. e.g. fully-qualified
	QString								m_SubGroup;				// From emoji-test.txt. e.g. time
	QString								m_Tag;					// Init from emoji-test.txt. User can customize. e.g. alarm clock
	AuVisage::VISAGE					m_Visage;				// Facial expression can be customized by the user.

public:
	AuEmoji(const QString& group, const QString& subgroup, const QString& name, const QString& status, const QString& emoji);
	AuEmoji();
	virtual ~AuEmoji();
	AuEmoji(const AuEmoji& c);

public:
	const AuEmoji& operator=(const AuEmoji& c);

public:
	void ClearVisage();

	// Returns the default tag for this emoji made by stripping the version off the name.
	QString DefaultTag() const;

	// Returns the default facial expression for this emoji.
	AuVisage::VISAGE DefaultVisage() const;

	inline const QString& Emoji() const
	{
		return m_Emoji;
	}

	inline const QString& Group() const
	{
		return m_Group;
	}

	inline bool IsVisageChanged() const
	{
		return Visage() != DefaultVisage();
	}

	inline bool IsTagChanged() const
	{
		return Tag() != DefaultTag();
	}

	inline const QString& Name() const
	{
		return m_Name;
	}

	inline void SetVisage(AuVisage::VISAGE visage)
	{
		m_Visage = visage;
	}

	inline void SetTag(const QString& tag)
	{
		m_Tag = tag;
	}

	inline const QString& Tag() const
	{
		return m_Tag;
	}

	inline AuVisage::VISAGE Visage() const
	{
		return m_Visage;
	}

	// Returns true if this emoji is associated with a facial expression. 'visage' is configured accordingly.
	// Returns false if this emoji is not associated with a facial expression. 'visage' is set to the default constructor.
	inline bool Visage(AuVisage& visage) const
	{
		visage = AuVisage(m_Visage);
		return m_Visage != AuVisage::Invalid;
	}
};

QDebug operator<<(QDebug dbg, const AuEmoji& o);

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuEmojiList : public QList<AuEmoji>
{
public:
	// Changes the 'tag' on all emojis named 'emojiName'.
	void ChangeTag(const QString& emojiName, const QString& tag);

	// Changes the 'visage' on all emojis named 'emojiName'.
	void ChangeVisage(const QString& emojiName, AuVisage::VISAGE visage);

	// Locate an emoji and returns its index.
	int EmojiIndex(const QString& emoji) const;

	// Loads the emoji-test.txt file.
	void LoadStandardEmojis();

	// Loads tags from 'in' and updates these emojis.
	void LoadTags(QDataStream& in);

	// Loads facial expressions from 'in' and updates these emojis.
	void LoadVisages(QDataStream& in);

	// Replaces emoji tags in 'raw'. E.g. :alarm clock: might be replaced with ⏰, if 'alarm clock' is a tag in the emojis list.
	QString ReplaceTags(const QString& raw) const;

	// Splits 'raw' into words and emojis. Expects ReplaceTags to have already been run and emojis to be emojis, not tags.
	QStringList SplitWords(const QString& textWithEmojis) const;

	// Stores tags that have been customized into 'out'.
	void StoreTags(QDataStream& out) const;

	// Stores facial expressions that have been customized into 'out'.
	void StoreVisages(QDataStream& out) const;

	// Returns true if 'emoji' is associated with a facial expression. 'visage' is configured accordingly.
	// Returns false if 'emoji' is not associated with a facial expression. 'visage' is set to the default constructor.
	bool Visage(const QString& emoji, AuVisage& visage) const;
};

#endif // AuEmoji_H
