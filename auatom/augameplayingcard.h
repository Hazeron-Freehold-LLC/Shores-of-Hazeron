//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augameplayingcard.h,v 1.8 2022/02/19 17:31:19 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuGamePlayingCard_H
#define AuGamePlayingCard_H

#include <QColor>
#include <QDebug>
#include <augleffects.h>
#include <aupoint.h>

class AuGamePlayingCard
{
public:
	enum SUIT
	{
		// These are in the row order of PlayingCards.png
		Joker,			// Joker row contains card backs after the two jokers.
		Clubs,
		Diamonds,
		Hearts,
		Spades,
	};

	enum CARD
	{
		// These are in the column order of PlayingCards.png
		Black = 0,	// Black joker in the Joker suit only.
		Red,		// Red joker in the Joker suit only.
		BackStyle0,	// Back style in Joker suit only.
		BackStyle1,	// Back style in Joker suit only.
		BackStyle2,	// Back style in Joker suit only.
		BackStyle3,	// Back style in Joker suit only.
		Ace = 0,
		Deuce,
		Three,
		Four,
		Five,
		Six,
		Seven,
		Eight,
		Nine,
		Ten,
		Jack,
		Queen,
		King,
		AceHigh,	// Used for evaluating hands. Not really a different card type. No texture created at this position.
	};

private:
	quint8						m_CardSuit;						// Card in top nibble, suit in bottom nibble.

public:
	inline AuGamePlayingCard()
	{
		// The default card is invalid. This value is chosen because it will result in drawing the faceless card on the client,
		// which creates the faceless card model after the last joker..
		m_CardSuit = (BackStyle0 << 4) | Joker;
	}

	inline AuGamePlayingCard(SUIT suit, CARD card)
	{
		m_CardSuit = (card << 4) | suit;
	}

	inline AuGamePlayingCard(const AuGamePlayingCard& c)
	{
		m_CardSuit = c.m_CardSuit;
	}

	inline const AuGamePlayingCard& operator=(const AuGamePlayingCard& c)
	{
		m_CardSuit = c.m_CardSuit;
		return *this;
	}

	inline CARD Card() const
	{
		CARD card = (CARD)(m_CardSuit >> 4);	// Top nibble is the card.
		Q_ASSERT(card >= Ace && card <= King);
		return card;
	}

	// 'backStyle' must be in the range 0..3.
	static AuGLVAO CardModel(AuOpenGLFunctions& GLf, CARD card, SUIT suit, int backStyle);

	// Returns card name for non-Joker cards.
	inline static QString CardName(CARD card)
	{
		QString cardName[] =
		{
			QObject::tr("Ace"),
			QObject::tr("Deuce"),
			QObject::tr("Three"),
			QObject::tr("Four"),
			QObject::tr("Five"),
			QObject::tr("Six"),
			QObject::tr("Seven"),
			QObject::tr("Eight"),
			QObject::tr("Nine"),
			QObject::tr("Ten"),
			QObject::tr("Jack"),
			QObject::tr("Queen"),
			QObject::tr("King"),
			QObject::tr("Ace"),
		};
		return cardName[card];
	}

	// Returns the card model that has a back on both sides.
	inline static AuGLVAO FacelessCardModel(AuOpenGLFunctions& GLf, int backStyle)
	{
		return CardModel(GLf, BackStyle0, Joker, backStyle);
	}

	// 'modelIndex' is 0=dealer button, 1=little blind button, 2=big blind button
	static AuGLVAO HoldemButtonModel(AuOpenGLFunctions& GLf, int modelIndex);

	inline QString Name() const
	{
		CARD card = Card();
		SUIT suit = Suit();
		if(suit == Joker)
		{
			if(card == Black)
				return QObject::tr("Black Joker");
			else if(card == Red)
				return QObject::tr("Red Joker");
			else if(card == BackStyle0)
				return QObject::tr("Faceless");
			else
				return QObject::tr("Invalid card %1 in Joker suit.").arg(card);
		}
		else
		{
			QString suitName[] =
			{
				QObject::tr("Joker"),
				QObject::tr("Clubs"),
				QObject::tr("Diamonds"),
				QObject::tr("Hearts"),
				QObject::tr("Spades"),
			};
			return QObject::tr("%1 of %2").arg(CardName(card)).arg(suitName[suit]);
		}
	}

	inline void operator<<(QDataStream& in)
	{
		in >> m_CardSuit;
	}

	void operator>>(QDataStream& out) const
	{
		out << m_CardSuit;
	}

	inline static float PlayingCardHeight()
	{
		return (3.5f*.0833f)*2;					// Standard bridge playing cards are 2-1/4" x 3-1/2". Ours are 2x that.
	}

	inline static float PlayingCardThickness()
	{
		return (.25f*0.0032808399f)*2;			// Typical card is .25mm thick. Ours are 2x that.
	}

	inline static float PlayingCardWidth()
	{
		return (2.5f*.0833f)*2;					// Standard bridge playing cards are 2-1/4" x 3-1/2". Ours are 2x that.
	}

	// 'denom' must be in the range 0 to 8.
	static AuGLVAO PokerChipModel(AuOpenGLFunctions& GLf, int denom);

	inline static float PokerChipRadius()
	{
		return (39.f * 0.0032808399f) / 2;		// 39mm diameter
	}

	inline static float PokerChipThickness()
	{
		return 3.4f * 0.0032808399f;			// 3.4mm thick
	}

	static AuGLVAO PokerTableTopModel(AuOpenGLFunctions& GLf);

	inline SUIT Suit() const
	{
		SUIT suit = (SUIT)(m_CardSuit & 0x0f);	// Bottom nibble is the suit.
		Q_ASSERT(suit >= Joker && suit <= Spades);
		return suit;
	}

	// Returns the texture id used for cards, poker chips, and holdem buttons.
	static GLuint TextureId(AuGLEffects& re);
};

inline QDataStream& operator<<(QDataStream& out, const AuGamePlayingCard& card)
{
	card >> out;
	return out;
}

inline QDataStream& operator>>(QDataStream& in, AuGamePlayingCard& card)
{
	card << in;
	return in;
}

QDebug operator<<(QDebug dbg, const AuGamePlayingCard& card);

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AuGamePlayingCardSprite
{
public:
	AuGamePlayingCard	m_Card;		// The card.
	AuPoint				m_End;		// The destination location of the card.
	AuPoint				m_Loc;		// Current location relative to center of game table top.
	float				m_Rot;		// Z axis rotation so they spin when they fly.
	AuPoint				m_Start;	// The starting location of the card.

	AuGamePlayingCardSprite()
	{
		m_Rot = 0;
	}

	AuGamePlayingCardSprite(const AuGamePlayingCardSprite& c)
	{
		m_Card = c.m_Card;
		m_End = c.m_End;
		m_Loc = c.m_Loc;
		m_Rot = c.m_Rot;
		m_Start = c.m_Start;
	}

	const AuGamePlayingCardSprite& operator=(const AuGamePlayingCardSprite& c)
	{
		m_Card = c.m_Card;
		m_End = c.m_End;
		m_Loc = c.m_Loc;
		m_Rot = c.m_Rot;
		m_Start = c.m_Start;
		return *this;
	}
};

#endif // AuGamePlayingCard_H
