//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augameplayingcard.cpp,v 1.14 2022/02/19 17:39:48 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augameplayingcard.h"
//
#include "augame.h"
#include <auglobal.h>
#include "autriangle.h"
#include "daperson.h"
#include "dasector.h"
#include <QIcon>
#include <QGridLayout>
#include <QLabel>
#include <QLCDNumber>
#include <QLineEdit>
#include <QSlider>

static AuGLVAO	s_HoldemButtonModels[3];	// GL models of the three buttons used: Dealer, Little Blind, and Big Blind
static AuGLVAO	s_PlayingCardModels[4][55];	// GL models of each style. 52 cards + 2 jokers + 1 faceless card (back on both sides)
static GLuint	s_PlayingCardTexture = 0;	// PlayingCards.png texture id
static AuGLVAO	s_PokerChipModels[9];		// GL models of poker chips. 9 denominations. Uses s_PlayingCardTexture.
static AuGLVAO	s_PokerTableTopModel;		// GL model of the surface of the poker table.

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

QDebug operator<<(QDebug dbg, const AuGamePlayingCard& card)
{
	dbg.nospace() << card.Name();
	return dbg.space();
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

//static
AuGLVAO AuGamePlayingCard::CardModel(AuOpenGLFunctions& GLf, CARD card, SUIT suit, int backStyle)
{
	Q_ASSERT(card >= Ace && card <= King);
	Q_ASSERT(suit >= Joker && suit <= Spades);
	Q_ASSERT(suit != Joker || card <= BackStyle0);	// Joker suit has only three card models: Black, Red, and Faceless
	Q_ASSERT(backStyle >= 0 && backStyle <= 3);
	if(!s_PlayingCardModels[backStyle][0].VAO())
	{
		// Variables used for calculating texture coordinates in the texture.
		const float maxx = 1015;	// width of the texture
		const float maxy = 611;		// height of the texture

		// Calculate texture coords of the selected back style.
		int bx = (backStyle + AuGamePlayingCard::BackStyle0) * 78;
		int by = 4 * 122;		// Top row (of 5 rows) has backs. Texture coords are measured up from the bottom of the texture
		float bs = (bx + .5) / maxx;
		float bt = (by + .5) / maxy;
		float bu = (bx + 78 - .5) / maxx;
		float bv = (by + 122 - .5) / maxy;

		// Create standard card deck of OpenGL objects.
		// Standard bridge playing cards are 2-1/4" x 3-1/2". This fits the aspect ratio of the cards in the texture.
		const float cardWid2 = PlayingCardWidth()/2;	// 1/2 width
		const float cardHt2 = PlayingCardHeight()/2;	// 1/2 height

		for(int suit = AuGamePlayingCard::Clubs; suit <= AuGamePlayingCard::Spades; suit++)
			for(int card = AuGamePlayingCard::Ace; card <= AuGamePlayingCard::King; card++)
			{
				// Calculate card face texture coordinates.
				int fx = card * 78;
				int fy = (4 - suit) * 122;			// Texture coords are measured up from the bottom of the texture
				float fs = (fx + .5) / maxx;
				float ft = (fy + .5) / maxy;
				float fu = (fx + 78 - .5) / maxx;
				float fv = (fy + 122 - .5) / maxy;

				QList<AuTriTexNS> tris;

				// First quad, facing down the -Y axis.
				// Card face, toward a player looking parallel to the +Y axis, which is their unrotated look direction.
				AuPoint n(0,-1,0);
				tris << AuTriTexNS(AuPointTexNS(-cardWid2,0,-cardHt2, fs,ft, n, 1),
						AuPointTexNS(-cardWid2,0,cardHt2, fs,fv, n, 1),
						AuPointTexNS(cardWid2,0,cardHt2, fu,fv, n, 1));

				tris << AuTriTexNS(AuPointTexNS(cardWid2,0,cardHt2, fu,fv, n, 1),
						AuPointTexNS(cardWid2,0,-cardHt2, fu,ft, n, 1),
						AuPointTexNS(-cardWid2,0,-cardHt2, fs,ft, n, 1));

				// Second quad, facing down the +Y axis.
				// Card back, away from a player looking parallel to the +Y axis, which is their unrotated look direction.
				n = AuPoint(0,1,0);
				tris << AuTriTexNS(AuPointTexNS(cardWid2,0,-cardHt2, bs,bt, n, 1),
						AuPointTexNS(cardWid2,0,cardHt2, bs,bv, n, 1),
						AuPointTexNS(-cardWid2,0,cardHt2, bu,bv, n, 1));

				tris << AuTriTexNS(AuPointTexNS(-cardWid2,0,cardHt2, bu,bv, n, 1),
						AuPointTexNS(-cardWid2,0,-cardHt2, bu,bt, n, 1),
						AuPointTexNS(cardWid2,0,-cardHt2, bs,bt, n, 1));

				// Standard cards are the first 52
				s_PlayingCardModels[backStyle][((suit-1) * 13) + card].CreateForModelTexFog(GLf, tris, GL_STATIC_DRAW);
			}

		// Create jokers.
		int suit = 5;
		for(int card = AuGamePlayingCard::Black; card <= AuGamePlayingCard::Red; card++)
		{
			// Calculate card face texture coordinates.
			int fx = card * 78;
			int fy = (4 - suit) * 122;			// Texture coords are measured up from the bottom of the texture
			float fs = (fx + .5) / maxx;
			float ft = (fy + .5) / maxy;
			float fu = (fx + 78 - .5) / maxx;
			float fv = (fy + 122 - .5) / maxy;

			QList<AuTriTexNS> tris;

			// First quad, facing down the -Y axis.
			// Card face, toward a player looking parallel to the +Y axis, which is their unrotated look direction.
			AuPoint n(0,-1,0);
			tris << AuTriTexNS(AuPointTexNS(-cardWid2,0,-cardHt2, fs,ft, n, 1),
					AuPointTexNS(-cardWid2,0,cardHt2, fs,fv, n, 1),
					AuPointTexNS(cardWid2,0,cardHt2, fu,fv, n, 1));

			tris << AuTriTexNS(AuPointTexNS(cardWid2,0,cardHt2, fu,fv, n, 1),
					AuPointTexNS(cardWid2,0,-cardHt2, fu,ft, n, 1),
					AuPointTexNS(-cardWid2,0,-cardHt2, fs,ft, n, 1));

			// Second quad, facing down the +Y axis.
			// Card back, away from a player looking parallel to the +Y axis, which is their unrotated look direction.
			n = AuPoint(0,1,0);
			tris << AuTriTexNS(AuPointTexNS(cardWid2,0,-cardHt2, bs,bt, n, 1),
					AuPointTexNS(cardWid2,0,cardHt2, bs,bv, n, 1),
					AuPointTexNS(-cardWid2,0,cardHt2, bu,bv, n, 1));

			tris << AuTriTexNS(AuPointTexNS(-cardWid2,0,cardHt2, bu,bv, n, 1),
					AuPointTexNS(-cardWid2,0,-cardHt2, bu,bt, n, 1),
					AuPointTexNS(cardWid2,0,-cardHt2, bs,bt, n, 1));

			// Jokers follow the standard 52 cards.
			s_PlayingCardModels[backStyle][52 + card].CreateForModelTexFog(GLf, tris, GL_STATIC_DRAW);
		}

		// Create a card with a back on both sides, for hidden cards so there's no way to peek.
		QList<AuTriTexNS> tris;

		// First quad, facing down the -Y axis.
		// Card face, toward a player looking parallel to the +Y axis, which is their unrotated look direction.
		AuPoint n(0,-1,0);
		tris << AuTriTexNS(AuPointTexNS(-cardWid2,0,-cardHt2, bs,bt, n, 1),
				AuPointTexNS(-cardWid2,0,cardHt2, bs,bv, n, 1),
				AuPointTexNS(cardWid2,0,cardHt2, bu,bv, n, 1));

		tris << AuTriTexNS(AuPointTexNS(cardWid2,0,cardHt2, bu,bv, n, 1),
				AuPointTexNS(cardWid2,0,-cardHt2, bu,bt, n, 1),
				AuPointTexNS(-cardWid2,0,-cardHt2, bs,bt, n, 1));

		// Second quad, facing down the +Y axis.
		// Card back, away from a player looking parallel to the +Y axis, which is their unrotated look direction.
		n = AuPoint(0,1,0);
		tris << AuTriTexNS(AuPointTexNS(cardWid2,0,-cardHt2, bs,bt, n, 1),
				AuPointTexNS(cardWid2,0,cardHt2, bs,bv, n, 1),
				AuPointTexNS(-cardWid2,0,cardHt2, bu,bv, n, 1));

		tris << AuTriTexNS(AuPointTexNS(-cardWid2,0,cardHt2, bu,bv, n, 1),
				AuPointTexNS(-cardWid2,0,-cardHt2, bu,bt, n, 1),
				AuPointTexNS(cardWid2,0,-cardHt2, bs,bt, n, 1));

		// Faceless model is last, after the standard deck and jokers.
		s_PlayingCardModels[backStyle][52 + 2].CreateForModelTexFog(GLf, tris, GL_STATIC_DRAW);
	}

	if(suit == Joker)
		return s_PlayingCardModels[backStyle][52 + card];
	else
		return s_PlayingCardModels[backStyle][((suit-1) * 13) + card];
}

//static
AuGLVAO AuGamePlayingCard::PokerChipModel(AuOpenGLFunctions& GLf, int denom)
{
	Q_ASSERT(denom >= 0 && denom < 9);
	if(!s_PokerChipModels[denom].VAO())
	{
		// Create poker chip model.
		// Variables used for calculating texture coordinates in the texture.
		const float maxx = 1015;	// width of the texture
		const float maxy = 611;		// height of the texture

		// Calculate poker chip texture coordinates.
		int cx;
		int cy;
		if(denom < 5)
		{
			// First row of 5 at very top of texture.
			cx = 609 + denom * 61;
			cy = 550;
		}
		else
		{
			// Second row of 4 just below that top row.
			cx = 609 + (denom-5) * 61;
			cy = 489;
		}
		float cs = (cx+1 + .5) / maxx;
		float ct = (cy+1 + .5) / maxy;
		float cu = (cx + 61 -1 - .5) / maxx;
		float cv = (cy + 61 -1 - .5) / maxy;
		float cenx = (cs + cu)/2.f;			// Center of chip texture.
		float ceny = (ct + cv)/2.f;			// Center of chip texture.
		float texradx = cenx - cs;			// Radius of chip texture.
		float texrady = ceny - ct;			// Radius of chip texture.

		QList<AuTriTexNS> tris;

		// Top of chip.
		AuPoint n(0,0,1);
		double pvx=0, pvy=0;
		float ptx=0, pty=0;
		for(int facet=16; facet>=0; facet--)
		{
			float angle;
			if(facet == 16)
				angle = 0;
			else
				angle = (PIf*2) * facet / 16;
			float ca = cosf(angle);
			float sa = sinf(angle);
			float vx = ca * PokerChipRadius();
			float vy = sa * PokerChipRadius();
			float tx = cenx + ca * texradx;
			float ty = ceny + sa * texrady;

			if(facet < 16)
				tris << AuTriTexNS(AuPointTexNS(0,0,PokerChipThickness(), cenx,ceny, n, 1),
						AuPointTexNS(pvx,pvy,PokerChipThickness(), ptx,pty, n, 1),
						AuPointTexNS(vx,vy,PokerChipThickness(), tx,ty, n, 1));

			pvx = vx;
			pvy = vy;
			ptx = tx;
			pty = ty;
		}

		// Bottom of chip.
		n = AuPoint(0,0,-1);
		for(int facet=0; facet<=16; facet++)
		{
			float angle;
			if(facet == 16)
				angle = 0;
			else
				angle = (PIf*2) * facet / 16;
			float ca = cosf(angle);
			float sa = sinf(angle);
			float vx = ca * PokerChipRadius();
			float vy = sa * PokerChipRadius();
			float tx = cenx + ca * texradx;
			float ty = ceny + sa * texrady;

			if(facet > 0)
				tris << AuTriTexNS(AuPointTexNS(0,0,0, cenx,ceny, n, 1),
						AuPointTexNS(pvx,pvy,0, ptx,pty, n, 1),
						AuPointTexNS(vx,vy,0, tx,ty, n, 1));

			pvx = vx;
			pvy = vy;
			ptx = tx;
			pty = ty;
		}

		// Edge of chip.
		float pca=0, psa=0;
		for(int facet=16; facet>=0; facet--)
		{
			float angle;
			if(facet == 16)
				angle = 0;
			else
				angle = (PIf*2) * facet / 16;
			float ca = cosf(angle);
			float sa = sinf(angle);
			float vx = ca * PokerChipRadius();
			float vy = sa * PokerChipRadius();
			float tx = cenx + ca * texradx;
			float ty = ceny + sa * texrady;

			if(facet < 16)
			{
				tris << AuTriTexNS(AuPointTexNS(vx,vy,0, tx,ty, AuPoint(ca, sa, 0), 1),
						AuPointTexNS(vx,vy,PokerChipThickness(), tx,ty, AuPoint(ca, sa, 0), 1),
						AuPointTexNS(pvx,pvy,PokerChipThickness(), ptx,pty, AuPoint(pca, psa, 0), 1));

				tris << AuTriTexNS(AuPointTexNS(pvx,pvy,PokerChipThickness(), ptx,pty, AuPoint(pca, psa, 0), 1),
						AuPointTexNS(pvx,pvy,0, ptx,pty, AuPoint(pca, psa, 0), 1),
						AuPointTexNS(vx,vy,0, tx,ty, AuPoint(ca, sa, 0), 1));
			}

			pvx = vx;
			pvy = vy;
			ptx = tx;
			pty = ty;
			pca = ca;
			psa = sa;
		}

		s_PokerChipModels[denom].CreateForModelTexFog(GLf, tris, GL_STATIC_DRAW);
	}
	return s_PokerChipModels[denom];
}

//static
AuGLVAO AuGamePlayingCard::HoldemButtonModel(AuOpenGLFunctions& GLf, int modelIndex)
{
	Q_ASSERT(modelIndex >= 0 && modelIndex < 3);	// 0=dealer, 1=little blind, 2=big blind
	if(!s_HoldemButtonModels[modelIndex].VAO())
	{
		// Create button model.
		// Variables used for calculating texture coordinates in the texture.
		const float maxx = 1015;	// width of the texture
		const float maxy = 611;		// height of the texture

		if(modelIndex == 0)
		{
			// Dealer button
			// Calculate dealer button texture coordinates.
			int cx = 470;
			int cy = 530;
			float cs = (cx + .5) / maxx;
			float ct = (cy + .5) / maxy;
			float cu = (cx + 80 - .5) / maxx;
			float cv = (cy + 80 - .5) / maxy;
			float cenx = (cs + cu)/2.f;			// Center of button texture.
			float ceny = (ct + cv)/2.f;			// Center of button texture.
			float texradx = cenx - cs;			// Radius of chip texture.
			float texrady = ceny - ct;			// Radius of chip texture.

			const float buttonRadius = 3.f * .0833f;			// 3" diameter
			const float buttonThickness = 10.f * 0.0032808399f;		// 10mm thick

			QList<AuTriTexNS> tris;

			// Top of button.
			AuPoint n(0,0,1);
			double pvx = 0;
			double pvy = 0;
			float ptx = 0;
			float pty = 0;
			for(int facet=16; facet>=0; facet--)
			{
				float angle;
				if(facet == 16)
					angle = 0;
				else
					angle = (PIf*2) * facet / 16;
				float ca = cosf(angle);
				float sa = sinf(angle);
				float vx = ca * buttonRadius;
				float vy = sa * buttonRadius;
				float tx = cenx + ca * texradx;
				float ty = ceny + sa * texrady;

				if(facet < 16)
					tris << AuTriTexNS(AuPointTexNS(0,0,buttonThickness, cenx,ceny, n, 1),
							AuPointTexNS(pvx,pvy,buttonThickness, ptx,pty, n, 1),
							AuPointTexNS(vx,vy,buttonThickness, tx,ty, n, 1));

				pvx = vx;
				pvy = vy;
				ptx = tx;
				pty = ty;
			}

			// Edge of button.
			float pca = 0;
			float psa = 0;
			for(int facet=16; facet>=0; facet--)
			{
				float angle;
				if(facet == 16)
					angle = 0;
				else
					angle = (PIf*2) * facet / 16;
				float ca = cosf(angle);
				float sa = sinf(angle);
				float vx = ca * buttonRadius;
				float vy = sa * buttonRadius;
				float tx = cenx + ca * texradx;
				float ty = ceny + sa * texrady;

				if(facet < 16)
				{
					tris << AuTriTexNS(AuPointTexNS(vx,vy,0, tx,ty, AuPoint(ca, sa, 0), 1),
							AuPointTexNS(vx,vy,buttonThickness, tx,ty, AuPoint(ca, sa, 0), 1),
							AuPointTexNS(pvx,pvy,buttonThickness, ptx,pty, AuPoint(pca, psa, 0), 1));

					tris << AuTriTexNS(AuPointTexNS(pvx,pvy,buttonThickness, ptx,pty, AuPoint(pca, psa, 0), 1),
							AuPointTexNS(pvx,pvy,0, ptx,pty, AuPoint(pca, psa, 0), 1),
							AuPointTexNS(vx,vy,0, tx,ty, AuPoint(ca, sa, 0), 1));
				}

				pvx = vx;
				pvy = vy;
				ptx = tx;
				pty = ty;
				pca = ca;
				psa = sa;
			}

			s_HoldemButtonModels[modelIndex].CreateForModelTexFog(GLf, tris, GL_STATIC_DRAW);
		}
		else if(modelIndex == 1)
		{
			// Little blind button
			// Calculate little blind button texture coordinates.
			int cx = 551;
			int cy = 552;
			float cs = (cx + .5) / maxx;
			float ct = (cy + .5) / maxy;
			float cu = (cx + 58 - .5) / maxx;
			float cv = (cy + 58 - .5) / maxy;
			float cenx = (cs + cu)/2.f;			// Center of button texture.
			float ceny = (ct + cv)/2.f;			// Center of button texture.
			float texradx = cenx - cs;			// Radius of chip texture.
			float texrady = ceny - ct;			// Radius of chip texture.

			const float buttonRadius = 2.f * .0833f;				// 2" diameter
			const float buttonThickness = 7.f * 0.0032808399f;		// 7mm thick

			QList<AuTriTexNS> tris;

			// Top of button.
			AuPoint n(0,0,1);
			double pvx = 0;
			double pvy = 0;
			float ptx = 0;
			float pty = 0;
			for(int facet=16; facet>=0; facet--)
			{
				float angle;
				if(facet == 16)
					angle = 0;
				else
					angle = (PIf*2) * facet / 16;
				float ca = cosf(angle);
				float sa = sinf(angle);
				float vx = ca * buttonRadius;
				float vy = sa * buttonRadius;
				float tx = cenx + ca * texradx;
				float ty = ceny + sa * texrady;

				if(facet < 16)
					tris << AuTriTexNS(AuPointTexNS(0,0,buttonThickness, cenx,ceny, n, 1),
							AuPointTexNS(pvx,pvy,buttonThickness, ptx,pty, n, 1),
							AuPointTexNS(vx,vy,buttonThickness, tx,ty, n, 1));

				pvx = vx;
				pvy = vy;
				ptx = tx;
				pty = ty;
			}

			// Edge of button.
			float pca = 0;
			float psa = 0;
			for(int facet=16; facet>=0; facet--)
			{
				float angle;
				if(facet == 16)
					angle = 0;
				else
					angle = (PIf*2) * facet / 16;
				float ca = cosf(angle);
				float sa = sinf(angle);
				float vx = ca * buttonRadius;
				float vy = sa * buttonRadius;
				float tx = cenx + ca * texradx;
				float ty = ceny + sa * texrady;

				if(facet < 16)
				{
					tris << AuTriTexNS(AuPointTexNS(vx,vy,0, tx,ty, AuPoint(ca, sa, 0), 1),
							AuPointTexNS(vx,vy,buttonThickness, tx,ty, AuPoint(ca, sa, 0), 1),
							AuPointTexNS(pvx,pvy,buttonThickness, ptx,pty, AuPoint(pca, psa, 0), 1));

					tris << AuTriTexNS(AuPointTexNS(pvx,pvy,buttonThickness, ptx,pty, AuPoint(pca, psa, 0), 1),
							AuPointTexNS(pvx,pvy,0, ptx,pty, AuPoint(pca, psa, 0), 1),
							AuPointTexNS(vx,vy,0, tx,ty, AuPoint(ca, sa, 0), 1));
				}

				pvx = vx;
				pvy = vy;
				ptx = tx;
				pty = ty;
				pca = ca;
				psa = sa;
			}

			s_HoldemButtonModels[modelIndex].CreateForModelTexFog(GLf, tris, GL_STATIC_DRAW);
		}
		else
		{
			// Big blind button
			// Calculate little blind button texture coordinates.
			int cx = 551;
			int cy = 493;
			float cs = (cx + .5) / maxx;
			float ct = (cy + .5) / maxy;
			float cu = (cx + 58 - .5) / maxx;
			float cv = (cy + 58 - .5) / maxy;
			float cenx = (cs + cu)/2.f;			// Center of button texture.
			float ceny = (ct + cv)/2.f;			// Center of button texture.
			float texradx = cenx - cs;			// Radius of chip texture.
			float texrady = ceny - ct;			// Radius of chip texture.

			const float buttonRadius = 2.f * .0833f;				// 2" diameter
			const float buttonThickness = 7.f * 0.0032808399f;		// 7mm thick

			QList<AuTriTexNS> tris;

			// Top of button.
			AuPoint n(0,0,1);
			double pvx = 0;
			double pvy = 0;
			float ptx = 0;
			float pty = 0;
			for(int facet=16; facet>=0; facet--)
			{
				float angle;
				if(facet == 16)
					angle = 0;
				else
					angle = (PIf*2) * facet / 16;
				float ca = cosf(angle);
				float sa = sinf(angle);
				float vx = ca * buttonRadius;
				float vy = sa * buttonRadius;
				float tx = cenx + ca * texradx;
				float ty = ceny + sa * texrady;

				if(facet < 16)
					tris << AuTriTexNS(AuPointTexNS(0,0,buttonThickness, cenx,ceny, n, 1),
							AuPointTexNS(pvx,pvy,buttonThickness, ptx,pty, n, 1),
							AuPointTexNS(vx,vy,buttonThickness, tx,ty, n, 1));

				pvx = vx;
				pvy = vy;
				ptx = tx;
				pty = ty;
			}

			// Edge of button.
			float pca = 0;
			float psa = 0;
			for(int facet=16; facet>=0; facet--)
			{
				float angle;
				if(facet == 16)
					angle = 0;
				else
					angle = (PIf*2) * facet / 16;
				float ca = cosf(angle);
				float sa = sinf(angle);
				float vx = ca * buttonRadius;
				float vy = sa * buttonRadius;
				float tx = cenx + ca * texradx;
				float ty = ceny + sa * texrady;

				if(facet < 16)
				{
					tris << AuTriTexNS(AuPointTexNS(vx,vy,0, tx,ty, AuPoint(ca, sa, 0), 1),
							AuPointTexNS(vx,vy,buttonThickness, tx,ty, AuPoint(ca, sa, 0), 1),
							AuPointTexNS(pvx,pvy,buttonThickness, ptx,pty, AuPoint(pca, psa, 0), 1));

					tris << AuTriTexNS(AuPointTexNS(pvx,pvy,buttonThickness, ptx,pty, AuPoint(pca, psa, 0), 1),
							AuPointTexNS(pvx,pvy,0, ptx,pty, AuPoint(pca, psa, 0), 1),
							AuPointTexNS(vx,vy,0, tx,ty, AuPoint(ca, sa, 0), 1));
				}

				pvx = vx;
				pvy = vy;
				ptx = tx;
				pty = ty;
				pca = ca;
				psa = sa;
			}

			s_HoldemButtonModels[modelIndex].CreateForModelTexFog(GLf, tris, GL_STATIC_DRAW);
		}
	}
	return s_HoldemButtonModels[modelIndex];
}

//static
AuGLVAO AuGamePlayingCard::PokerTableTopModel(AuOpenGLFunctions& GLf)
{
	// Create poker table top model.
	if(!s_PokerTableTopModel.VAO())
	{
		// Variables used for calculating texture coordinates in the texture.
		const float maxx = 1015;	// width of the texture
		const float maxy = 611;		// height of the texture

		// Calculate poker chip texture coordinates.
		int cx = 914;
		int cy = 510;
		float cs = (cx + .5) / maxx;
		float ct = (cy + .5) / maxy;
		float cu = (cx + 101 - .5) / maxx;
		float cv = (cy + 101 - .5) / maxy;
		float cenx = (cs + cu)/2.f;			// Center of chip texture.
		float ceny = (ct + cv)/2.f;			// Center of chip texture.
		float texradx = cenx - cs;			// Radius of chip texture.
		float texrady = ceny - ct;			// Radius of chip texture.

		const float topDrop = -2.f * 0.0032808399f;		// Drop from 0 plane, to keep cards from bleeding through.

		QList<AuTriTexNS> tris;

		AuPoint n(0,0,1);
		double pvx = 0;
		double pvy = 0;
		float ptx = 0;
		float pty = 0;
		for(int facet=16; facet>=0; facet--)
		{
			float angle;
			if(facet == 16)
				angle = 0;
			else
				angle = (PIf*2) * facet / 16;
			float ca = cosf(angle);
			float sa = sinf(angle);
			float vx = ca * AuGame::TableTopRadius();
			float vy = sa * AuGame::TableTopRadius();
			float tx = cenx + ca * texradx;
			float ty = ceny + sa * texrady;

			if(facet < 16)
				tris << AuTriTexNS(AuPointTexNS(0,0,topDrop, cenx,ceny, n, 1),
						AuPointTexNS(pvx,pvy,topDrop, ptx,pty, n, 1),
						AuPointTexNS(vx,vy,topDrop, tx,ty, n, 1));

			pvx = vx;
			pvy = vy;
			ptx = tx;
			pty = ty;
		}

		s_PokerTableTopModel.CreateForModelTexFog(GLf, tris, GL_STATIC_DRAW);
	}
	return s_PokerTableTopModel;
}

//static
GLuint AuGamePlayingCard::TextureId(AuGLEffects& re)
{
	// Load the texture with the cards, backs, poker chips, buttons, and other stuff.
	if(!s_PlayingCardTexture)
		s_PlayingCardTexture = re.CreateTextureFromBitmap(STATIC_TEXTURE_ID, "PlayingCards", ":data/PlayingCards.png");
	return s_PlayingCardTexture;
}
