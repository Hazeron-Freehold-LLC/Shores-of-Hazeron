//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: auemoji.cpp,v 1.10 2022/02/19 18:33:05 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "auemoji.h"
//
#include <QDebug>

AuEmoji::AuEmoji(const QString& group, const QString& subgroup, const QString& name, const QString& status, const QString& emoji)
{
	m_Emoji = emoji;
	m_Group = group;
	m_Name = name;
	m_Status = status;
	m_SubGroup = subgroup;
	m_Tag = DefaultTag();
	m_Visage = DefaultVisage();
}

AuEmoji::AuEmoji()
{
	m_Visage = AuVisage::Invalid;
}

AuEmoji::~AuEmoji()
{
}

AuEmoji::AuEmoji(const AuEmoji& c)
{
	m_Emoji = c.m_Emoji;
	m_Group = c.m_Group;
	m_Name = c.m_Name;
	m_Status = c.m_Status;
	m_SubGroup = c.m_SubGroup;
	m_Tag = c.m_Tag;
	m_Visage = c.m_Visage;
}

const AuEmoji& AuEmoji::operator=(const AuEmoji& c)
{
	m_Emoji = c.m_Emoji;
	m_Group = c.m_Group;
	m_Name = c.m_Name;
	m_Status = c.m_Status;
	m_SubGroup = c.m_SubGroup;
	m_Tag = c.m_Tag;
	m_Visage = c.m_Visage;
	return *this;
}

QDebug operator<<(QDebug dbg, const AuEmoji& o)
{
	dbg.nospace() << o.Emoji() << " " << o.Tag();
	return dbg.space();
}

QString AuEmoji::DefaultTag() const
{
	// Strip the version off the name for a default tag.
	int si = m_Name.indexOf(' ');
	if(si < 0)
		return "";
	return QString(":%1:").arg(m_Name.mid(si+1).trimmed());
}

AuVisage::VISAGE AuEmoji::DefaultVisage() const
{
	if(m_Emoji == "😫" ||
			m_Emoji == "😩")
		return AuVisage::Agape;
	else if(m_Emoji == "😔" ||
			m_Emoji == "😒" ||
			m_Emoji == "😑" ||
			m_Emoji == "😐" ||
			m_Emoji == "🤨" ||
			m_Emoji == "🤔")
		return AuVisage::Closed;
	else if(m_Emoji == "😿" ||
			m_Emoji == "😭" ||
			m_Emoji == "😢" ||
			m_Emoji == "😥")
		return AuVisage::Cry2Sec;
	else if(m_Emoji == "🙀" ||
			m_Emoji == "👿" ||
			m_Emoji == "😠" ||
			m_Emoji == "☹️" ||
			m_Emoji == "🙁")
		return AuVisage::Frown;
	else if(m_Emoji == "😾" ||
			m_Emoji == "😡" ||
			m_Emoji == "😓" ||
			m_Emoji == "😞" ||
			m_Emoji == "😣" ||
			m_Emoji == "😟")
		return AuVisage::Grim;
	else if(m_Emoji == "😹" ||
			m_Emoji == "😸" ||
			m_Emoji == "😺" ||
			m_Emoji == "🤪" ||
			m_Emoji == "🤩" ||
			m_Emoji == "😅" ||
			m_Emoji == "😆" ||
			m_Emoji == "😁" ||
			m_Emoji == "😄" ||
			m_Emoji == "😃" ||
			m_Emoji == "😀")
		return AuVisage::Grin;
	else if(m_Emoji == "🤭")
		return AuVisage::Laugh1Sec;
	else if(m_Emoji == "😂")
		return AuVisage::Laugh2Sec;
	else if(m_Emoji == "🤣")
		return AuVisage::Laugh4Sec;
	else if(m_Emoji == "😬")
		return AuVisage::Open;
	else if(m_Emoji == "😱" ||
			m_Emoji == "😰" ||
			m_Emoji == "😲" ||
			m_Emoji == "😮" ||
			m_Emoji == "🤯" ||
			m_Emoji == "😵")
		return AuVisage::OpenWide;
	else if(m_Emoji == "😖" ||
			m_Emoji == "😕")
		return AuVisage::Perplexed;
	else if(m_Emoji == "😳" ||
			m_Emoji == "😯" ||
			m_Emoji == "😶" ||
			m_Emoji == "🤐" ||
			m_Emoji == "🤫")
		return AuVisage::Pursed;
	else if(m_Emoji == "😨" ||
			m_Emoji == "😧" ||
			m_Emoji == "😦" ||
			m_Emoji == "😪")
		return AuVisage::Sad;
	else if(m_Emoji == "😼" ||
			m_Emoji == "🙄" ||
			m_Emoji == "😏" ||
			m_Emoji == "😝" ||
			m_Emoji == "😜" ||
			m_Emoji == "😛")
		return AuVisage::Sardonic;
	else if(m_Emoji == "😻" ||
			m_Emoji == "😈" ||
			m_Emoji == "😎" ||
			m_Emoji == "😌" ||
			m_Emoji == "🤗" ||
			m_Emoji == "😋" ||
			m_Emoji == "☺️" ||
			m_Emoji == "😍" ||
			m_Emoji == "😇" ||
			m_Emoji == "😊" ||
			m_Emoji == "😉" ||
			m_Emoji == "🙃" ||
			m_Emoji == "🙂")
		return AuVisage::Smile;
	else if(m_Emoji == "😴")
		return AuVisage::Yawn;
	else
		return AuVisage::Invalid;
}

void AuEmoji::ClearVisage()
{
	m_Visage = AuVisage::Invalid;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

void AuEmojiList::LoadStandardEmojis()
{
	QFile file(":data/emoji-test.txt");
	if(!file.open(QFile::ReadOnly))
	{
		qWarning() << "error: AuEmojiList::LoadEmojis failed to open emoji-test.txt";
		return;
	}

	QTextStream tstream(&file);
	tstream.setCodec("UTF-8");

	QString group;		// Current group as we read through the file.
	QString subgroup;	// Current subgroup as we read through the file.
	while(!tstream.atEnd())
	{
		QString line = tstream.readLine().trimmed();
		if(line.isEmpty())
			continue;
		
		// Parse the line
		QString gkey("# group: ");
		if(line.indexOf(gkey) >= 0)
		{
			// Group change line.
			group = line.mid(gkey.length()).trimmed();
			continue;
		}

		QString sgkey("# subgroup: ");
		if(line.indexOf(sgkey) >= 0)
		{
			// Subgroup change line.
			subgroup = line.mid(sgkey.length()).trimmed();
			continue;
		}

		// Format: code points; status # emoji name
		// Strip the 'emoji name' portion.
		int ci = line.indexOf('#');
		if(ci <= 0)
			continue;	// All other comment lines are ignored and lines with no # at all.
		QString emoname = line.mid(ci+1).trimmed();
		line = line.left(ci).trimmed();
		int ei = emoname.indexOf(' ');
		if(ei < 0)
			continue;
		QString name = emoname.mid(ei+1).trimmed();
		//QString emoji = emoname.left(ei).trimmed();	We will build the emoji ourselves.

		// Strip the 'status' portion.
		int si = line.indexOf(';');
		if(si < 0)
			continue;
		QString status = line.mid(si+1).trimmed();
		if(status != "fully-qualified")
			continue;					// We only want the fully qualified emojis.
		line = line.left(si).trimmed();	// Remainder of the line is the code points portion.

		// Build list of codepoints.
		QVector<uint> codePoints;
		QStringList codes = line.split(' ');
		for(QStringList::const_iterator ci = codes.cbegin(); ci != codes.cend(); ci++)
		{
			uint c = (*ci).toUInt(0, 16);
			codePoints.append(c);
		}

		// Make an emoji from the list of code points.
		QString emoji = QString::fromUcs4(codePoints.constData(), codePoints.size());

		// Create our emoji record with all that juicy data.
		AuEmoji emo(group, subgroup, name, status, emoji);
		//qDebug() << emo;
		append(emo);
	}
	file.close();
	//qDebug() << "AuEmojiList::LoadEmojis loaded" << size();
}

QString AuEmojiList::ReplaceTags(const QString& raw) const
{
	// Replace all fields in the raw string.
	QString ret(raw);
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
		ret.replace((*ei).Tag(), (*ei).Emoji(), Qt::CaseSensitive);
	return ret;
}

int AuEmojiList::EmojiIndex(const QString& emoji) const
{
	int index = 0;
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
	{
		if((*ei).Emoji() == emoji)
			return index;
		index++;
	}
	return -1;
}

void AuEmojiList::StoreTags(QDataStream& out) const
{
	// Construct a map of the names whose tags have changed.
	QMap<QString, QString> changedTags;
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
		if((*ei).IsTagChanged())
			changedTags.insert((*ei).Name(), (*ei).Tag());
	out << changedTags;
}

void AuEmojiList::LoadTags(QDataStream& in)
{
	QMap<QString, QString> changedTags;
	in >> changedTags;
	for(QMap<QString, QString>::const_iterator ci = changedTags.cbegin(); ci != changedTags.cend(); ci++)
		ChangeTag(ci.key(), *ci);
}

void AuEmojiList::ChangeTag(const QString& emojiName, const QString& tag)
{
	for(iterator ei = begin(); ei != end(); ei++)
		if((*ei).Name() == emojiName)
			(*ei).SetTag(tag);
}

QStringList AuEmojiList::SplitWords(const QString& textWithEmojis) const
{
	// Break down the text, one word or emoji at a time.
	QStringList we;
	QString remainder(textWithEmojis.trimmed());
	do
	{
		// Check for an emoji at the start. The last matching emoji should be the longest, most qualified, emoji that matches.
		const_iterator mi = cend();		// This is set to the last matching emoji.
		for(const_iterator ei = cbegin(); ei != cend(); ei++)
			if(remainder.left((*ei).Emoji().length()) == (*ei).Emoji())
				mi = ei;
		if(mi != cend())
		{
			// Strip off the emoji from the remainder.
			remainder = remainder.mid((*mi).Emoji().length()).trimmed();

			// Add the emoji as a word.
			we.append((*mi).Emoji());
		}
		else
		{
			// Strip off a word from the remainder. It could have embedded emojis.
			QString word;
			int si = remainder.indexOf(' ');
			if(si >= 0)
			{
				word = remainder.left(si);
				remainder = remainder.mid(si+1).trimmed();
			}
			else
			{
				word = remainder;
				remainder = QString();
			}

			// Check for emojis embedded in the word.
			int fi = -1;	// Index of first emoji found in the word.
			const_iterator fei = cend();
			for(const_iterator ei = cbegin(); ei != cend(); ei++)
			{
				int i = word.indexOf((*ei).Emoji());
				if(i > 0 && (i < fi || fi < 0))
				{
					fi = i;
					fei = ei;
				}
			}
			if(fi > 0)
			{
				// An emoji was found embedded in the word.
				remainder = word.mid(fi) + remainder;	// Restore the emoji(s) to the remainder. There could be several in a row.
				word = word.left(fi).trimmed();
			}

			// Add the word with no emojis.
			we.append(word);
		}
	}
	while(!remainder.isEmpty());
	//qDebug() << "AuEmojiList::SplitWords text" << textWithEmojis << "words" << we;
	return we;
}

bool AuEmojiList::Visage(const QString& emoji, AuVisage& face) const
{
	// Check for emojis.
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
		if((*ei).Emoji() == emoji)
			return (*ei).Visage(face);

	// Check for old style emotes.
	bool isEmote = false;
	if(emoji.compare("lol", Qt::CaseInsensitive) == 0 ||
			emoji.compare("ha", Qt::CaseInsensitive) == 0 ||
			emoji.compare("hah", Qt::CaseInsensitive) == 0)
	{
		face = AuVisage(AuVisage::Laugh1Sec);
		isEmote = true;
	}
	else if(emoji.compare("lulz", Qt::CaseInsensitive) == 0 ||
			emoji.compare("hehe", Qt::CaseInsensitive) == 0 ||
			emoji.compare("haha", Qt::CaseInsensitive) == 0)
	{
		face = AuVisage(AuVisage::Laugh2Sec);
		isEmote = true;
	}
	else if(emoji.compare("rofl", Qt::CaseInsensitive) == 0 ||
			emoji.compare("lmao", Qt::CaseInsensitive) == 0)
	{
		face = AuVisage(AuVisage::Laugh4Sec);
		isEmote = true;
	}
	else if(emoji.compare("hohum", Qt::CaseInsensitive) == 0 ||
			emoji.compare("|-O", Qt::CaseSensitive) == 0)
	{
		face = AuVisage(AuVisage::Yawn);
		isEmote = true;
	}
	else if(emoji.compare(":D", Qt::CaseSensitive) == 0 ||
			emoji.compare(":-D", Qt::CaseSensitive) == 0 ||
			emoji.compare(":))", Qt::CaseSensitive) == 0 ||
			emoji.compare(":-))", Qt::CaseSensitive) == 0 ||
			emoji.compare("8D", Qt::CaseSensitive) == 0 ||
			emoji.compare("8-D", Qt::CaseSensitive) == 0 ||
			emoji.compare("=D", Qt::CaseSensitive) == 0 ||
			emoji.compare(";D", Qt::CaseSensitive) == 0 ||
			emoji.compare("=3", Qt::CaseSensitive) == 0 ||
			emoji.compare("B^D", Qt::CaseSensitive) == 0 ||
			emoji.compare("XD", Qt::CaseSensitive) == 0 ||
			emoji.compare("X-D", Qt::CaseSensitive) == 0 ||
			emoji.compare("xD", Qt::CaseSensitive) == 0)
	{
		face = AuVisage(AuVisage::Grin);
		isEmote = true;
	}
	else if(emoji.compare("D:", Qt::CaseSensitive) == 0 ||
			emoji.compare("D:<", Qt::CaseSensitive) == 0 ||
			emoji.compare(">:[", Qt::CaseSensitive) == 0 ||
			emoji.compare(">:(", Qt::CaseSensitive) == 0 ||
			emoji.compare("D8", Qt::CaseSensitive) == 0 ||
			emoji.compare("D;", Qt::CaseSensitive) == 0 ||
			emoji.compare("D=", Qt::CaseSensitive) == 0 ||
			emoji.compare("DX", Qt::CaseSensitive) == 0)
	{
		face = AuVisage(AuVisage::Sad);
		isEmote = true;
	}
	else if(emoji.compare(":O", Qt::CaseSensitive) == 0 ||
			emoji.compare(":-O", Qt::CaseSensitive) == 0 ||
			emoji.compare("8-O", Qt::CaseSensitive) == 0 ||
			emoji.compare("O.O", Qt::CaseSensitive) == 0 ||
			emoji.compare("O:", Qt::CaseSensitive) == 0)
	{
		face = AuVisage(AuVisage::Agape);
		isEmote = true;
	}
	else if(emoji.compare(">:O", Qt::CaseSensitive) == 0 ||
			emoji.compare(":@", Qt::CaseSensitive) == 0 ||
			emoji.compare(">:@", Qt::CaseSensitive) == 0)
	{
		face = AuVisage(AuVisage::OpenWide);
		isEmote = true;
	}
	else if(emoji.compare("(:", Qt::CaseSensitive) == 0 ||
			emoji.compare(":)", Qt::CaseSensitive) == 0 ||
			emoji.compare("8)", Qt::CaseSensitive) == 0 ||
			emoji.compare("8-)", Qt::CaseSensitive) == 0 ||
			emoji.compare(":o)", Qt::CaseSensitive) == 0 ||
			emoji.compare(":-o)", Qt::CaseSensitive) == 0 ||
			emoji.compare(":c)", Qt::CaseSensitive) == 0 ||
			emoji.compare("=)", Qt::CaseSensitive) == 0 ||
			emoji.compare("=]", Qt::CaseSensitive) == 0 ||
			emoji.compare(":-)", Qt::CaseSensitive) == 0 ||
			emoji.compare("(-:", Qt::CaseSensitive) == 0 ||
			emoji.compare("(^:", Qt::CaseSensitive) == 0 ||
			emoji.compare(":^)", Qt::CaseSensitive) == 0 ||
			emoji.compare(":')", Qt::CaseSensitive) == 0 ||
			emoji.compare(":'-)", Qt::CaseSensitive) == 0 ||
			emoji.compare(";)", Qt::CaseSensitive) == 0 ||
			emoji.compare("*)", Qt::CaseSensitive) == 0 ||
			emoji.compare("*-)", Qt::CaseSensitive) == 0 ||
			emoji.compare(";-)", Qt::CaseSensitive) == 0 ||
			emoji.compare(";^)", Qt::CaseSensitive) == 0)
	{
		face = AuVisage(AuVisage::Smile);
		isEmote = true;
	}
	else if(emoji.compare(":(", Qt::CaseSensitive) == 0 ||
			emoji.compare("):", Qt::CaseSensitive) == 0 ||
			emoji.compare(":c", Qt::CaseSensitive) == 0 ||
			emoji.compare(":-c", Qt::CaseSensitive) == 0 ||
			emoji.compare(":<", Qt::CaseSensitive) == 0 ||
			emoji.compare(":-<", Qt::CaseSensitive) == 0 ||
			emoji.compare(":[", Qt::CaseSensitive) == 0 ||
			emoji.compare(":-[", Qt::CaseSensitive) == 0 ||
			emoji.compare(":-||", Qt::CaseSensitive) == 0 ||
			emoji.compare(":{", Qt::CaseSensitive) == 0 ||
			emoji.compare(":-(", Qt::CaseSensitive) == 0 ||
			emoji.compare(")-:", Qt::CaseSensitive) == 0 ||
			emoji.compare(":^(", Qt::CaseSensitive) == 0 ||
			emoji.compare(")^:", Qt::CaseSensitive) == 0)
	{
		face = AuVisage(AuVisage::Frown);
		isEmote = true;
	}
	else if(emoji.compare(":'(", Qt::CaseSensitive) == 0 ||
			emoji.compare(":'-(", Qt::CaseSensitive) == 0 ||
			emoji.compare(")':", Qt::CaseSensitive) == 0 ||
			emoji.compare("D':", Qt::CaseSensitive) == 0 ||
			emoji.compare("D-':", Qt::CaseSensitive) == 0)
	{
		face = AuVisage(AuVisage::Cry2Sec);
		isEmote = true;
	}
	else if(emoji.compare(":o", Qt::CaseSensitive) == 0 ||
			emoji.compare("o:", Qt::CaseSensitive) == 0 ||
			emoji.compare("o.o", Qt::CaseSensitive) == 0)
	{
		face = AuVisage(AuVisage::Open);
		isEmote = true;
	}
	else if(emoji.compare(":|", Qt::CaseSensitive) == 0 ||
			emoji.compare("|:", Qt::CaseSensitive) == 0 ||
			emoji.compare(":-|", Qt::CaseSensitive) == 0 ||
			emoji.compare(";-|", Qt::CaseSensitive) == 0 ||
			emoji.compare("^^", Qt::CaseSensitive) == 0 ||
			emoji.compare("^^;", Qt::CaseSensitive) == 0)
	{
		face = AuVisage(AuVisage::Closed);
		isEmote = true;
	}
	else if(emoji.compare(":S", Qt::CaseInsensitive) == 0 ||
			emoji.compare(":$", Qt::CaseSensitive) == 0 ||
			emoji.compare(">:\\", Qt::CaseSensitive) == 0 ||
			emoji.compare(">:/", Qt::CaseSensitive) == 0)
	{
		face = AuVisage(AuVisage::Perplexed);
		isEmote = true;
	}
	else if(emoji.compare(":/", Qt::CaseSensitive) == 0 ||
			emoji.compare("/:", Qt::CaseSensitive) == 0 ||
			emoji.compare("/^:", Qt::CaseSensitive) == 0 ||
			emoji.compare(":^/", Qt::CaseSensitive) == 0 ||
			emoji.compare(":-/", Qt::CaseSensitive) == 0 ||
			emoji.compare("=/", Qt::CaseSensitive) == 0 ||
			emoji.compare("=\\", Qt::CaseSensitive) == 0 ||
			emoji.compare(":\\", Qt::CaseSensitive) == 0 ||
			emoji.compare("\\:", Qt::CaseSensitive) == 0 ||
			emoji.compare(":^\\", Qt::CaseSensitive) == 0 ||
			emoji.compare("\\^:", Qt::CaseSensitive) == 0)
	{
		face = AuVisage(AuVisage::Grim);
		isEmote = true;
	}
	else if(emoji.compare(":P", Qt::CaseInsensitive) == 0 ||
			emoji.compare(":-P", Qt::CaseInsensitive) == 0 ||
			emoji.compare("XP", Qt::CaseInsensitive) == 0 ||
			emoji.compare("X-P", Qt::CaseInsensitive) == 0 ||
			emoji.compare(">:P", Qt::CaseInsensitive) == 0 ||
			emoji.compare("=P", Qt::CaseInsensitive) == 0 ||
			emoji.compare(":j", Qt::CaseInsensitive) == 0 ||
			emoji.compare(":-j", Qt::CaseInsensitive) == 0)
	{
		face = AuVisage(AuVisage::Sardonic);
		isEmote = true;
	}
	else if(emoji.compare("._.", Qt::CaseSensitive) == 0 ||
			emoji.compare(":X", Qt::CaseSensitive) == 0 ||
			emoji.compare(":-X", Qt::CaseSensitive) == 0 ||
			emoji.compare(">.<", Qt::CaseSensitive) == 0)
	{
		face = AuVisage(AuVisage::Pursed);
		isEmote = true;
	}
	else
	{
		face = AuVisage();
		isEmote = false;
	}
	return isEmote;
}

void AuEmojiList::StoreVisages(QDataStream& out) const
{
	// Construct a map of the names whose tags have changed.
	QMap<QString, int> changedFaces;
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
		if((*ei).IsVisageChanged())
			changedFaces.insert((*ei).Name(), (*ei).Visage());
	out << changedFaces;

/*
#if !defined(QT_NO_DEBUG)
	qDebug() << "AuEmojiList::StoreVisages";
	QMultiMap<int, QString> faces;	// key is facial expression, value is emoji
	for(const_iterator ei = cbegin(); ei != cend(); ei++)
	{
		AuVisage::VISAGE face = (*ei).Visage();
		if(face == AuVisage::Invalid)
			continue;
		faces.insert(face, (*ei).Emoji());
	}
	int curex = -1;
	for(QMultiMap<int, QString>::const_iterator fi = faces.cbegin(); fi != faces.cend(); fi++)
	{
		if(fi.key() != curex)
		{
			curex = fi.key();
			qDebug() << "Facial Expression" << AuVisage((AuVisage::VISAGE)fi.key()).Name();
		}
		qDebug() << " " << *fi;
	}
#endif
*/
}

void AuEmojiList::LoadVisages(QDataStream& in)
{
	QMap<QString, int> changedFaces;
	in >> changedFaces;
	for(QMap<QString, int>::const_iterator ci = changedFaces.cbegin(); ci != changedFaces.cend(); ci++)
		ChangeVisage(ci.key(), (AuVisage::VISAGE)*ci);
}

void AuEmojiList::ChangeVisage(const QString& emojiName, AuVisage::VISAGE face)
{
	for(iterator ei = begin(); ei != end(); ei++)
		if((*ei).Name() == emojiName)
			(*ei).SetVisage(face);
}

