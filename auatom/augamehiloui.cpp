//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: augamehiloui.cpp,v 1.17 2022/02/19 17:39:45 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "augamehiloui.h"
//
#include "augamehilo.h"
#include "auglobal.h"
#include "aupushbutton.h"
#include "daperson.h"
#include <QLabel>
#include <QLCDNumber>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QTextBrowser>
#include <QVBoxLayout>

AuGameHiLoSetup::AuGameHiLoSetup(const AuGameHiLo& game, QWidget* parent) : AuDialog(parent)
{
	// Restore my previous settings.
	quint32 ante = g_Au.Settings().readNumEntry("Game Table/HiLo Number Game/Ante", (int)game.Ante());
	if(ante > 10000000)
		ante = 10000000;
	quint8 digits = g_Au.Settings().readNumEntry("Game Table/HiLo Number Game/Digits", game.Digits());
	if(digits < 1)
		digits = 1;
	else if(digits > 6)
		digits = 6;

	setAuDlgIcon(QIcon(":data/GameHiLo.png"));
	setAuDlgTitle(tr("<b>HiLo Number Game Setup</b>"));

	m_Ok = new QPushButton(tr("Start Game"), this);
	m_Ok->setToolTip(tr("<html>Starts the game with the settings that are shown.</html>"));
	m_Ok->setAutoDefault(true);
	m_Ok->setDefault(true);
	connect(m_Ok, SIGNAL(clicked()), SLOT(StartGame()));

	// Add a box to set the ante.
	QLabel* lbante = new QLabel(QObject::tr("Ante %1").arg(g_Au.CurrencySymbol()), this);

	m_Ante = new QLineEdit(this);
	m_Ante->setText(QVariant(ante).toString());

	// Add a slider to set the number of digits, so we can strictly control the range.
	QLabel* lbdigits = new QLabel(QObject::tr("Random Digits"));

	QLCDNumber* lcdigits = new QLCDNumber(1, this);
	lcdigits->display(digits);

	m_Digits = new QSlider(Qt::Horizontal, this);
	m_Digits->setTickInterval(1);
	m_Digits->setTickPosition(QSlider::TicksAbove);
	m_Digits->setRange(1,6);
	m_Digits->setValue(digits);
	QObject::connect(m_Digits, SIGNAL(valueChanged(int)), lcdigits, SLOT(display(int)));

	// List the players in the game.
	// A kick button is added for everyone but the dealer.
	QGridLayout* layPlayers = new QGridLayout;
	int row=0;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
	{
		m_PlayerIds[player] = game.PlayerId(player);
		if(!m_PlayerIds[player].IsNull())
		{
			QLabel* lbhead = new QLabel(this);
			lbhead->setPixmap(QPixmap(":data/Passenger.png"));
			layPlayers->addWidget(lbhead, row,0);

			m_PlayerNames[player] = new QLabel(game.PlayerName(player), this);
			layPlayers->addWidget(m_PlayerNames[player], row,1);

			if(player != 0)
			{
				// Allow dealer to kick any of the players who were automatically enrolled because they are sitting at the table.
				m_PlayerKick[player] = new AuPushButtonValue(player, this);
				m_PlayerKick[player]->setIcon(QIcon(":data/CommKick.png"));
				m_PlayerKick[player]->setToolTip(tr("Kicks the player from the game."));
				connect(m_PlayerKick[player], SIGNAL(ClickedValue(int)), SLOT(KickPlayer(int)));
				layPlayers->addWidget(m_PlayerKick[player], row,2);
			}
			else
				m_PlayerKick[player] = 0;		// No kick button for the dealer.
			layPlayers->setColumnStretch(3,1);	// Scrunch them over to the left.
			row++;
		}
		else
		{
			// No player in that slot.
			m_PlayerNames[player] = 0;
			m_PlayerKick[player] = 0;
		}
	}

	// Create the layout of the settings dialog.
	QGridLayout* lay = new QGridLayout;
	row=0;
	lay->addLayout(layPlayers, row,0, 1,4);

	// Add game controls.
	lay->setRowStretch(row++, 1);			// Make a stretchy row to crunch the names to the top when there's extra space.
	lay->addWidget(lbante, row,0);
	lay->addWidget(m_Ante, row++,1, 1,2);
	lay->addWidget(lbdigits, row,0);
	lay->addWidget(m_Digits, row,1);
	lay->addWidget(lcdigits, row++,2);

	// Add the Start Game button.
	lay->addWidget(m_Ok, row,0, 1,3);

	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuGameHiLoSetup::~AuGameHiLoSetup()
{
}

void AuGameHiLoSetup::StartGame()
{
	// Collect and validate my settings from the widgets.
	int players = 0;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
		if(!m_PlayerIds[player].IsNull())
			players++;
	if(!players)
		return;		// Can't start a game with no players.

	qint32 ante = QVariant(m_Ante->text()).toInt();
	if(ante > 10000000)
		ante = 10000000;	// Ante limited to 10M.
	else if(ante < 0)
		ante = 0;			// Negative bet not permitted.

	quint8 digits = m_Digits->value();

	// Pack my settings into a blob the game will recognize at the other side.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << ante;
	out << digits;
	for(int player=0; player<MAX_GAME_PLAYERS; player++)
		out << m_PlayerIds[player];

	// Emit a signal that ausceneclient can put into a packet to the server with my game settings.
	emit Accepted(AuGameHiLo::GameType(), ba);
	accept();

	// Save these settings for next time.
	g_Au.Settings().writeEntry("Game Table/HiLo Number Game/Ante", ante);
	g_Au.Settings().writeEntry("Game Table/HiLo Number Game/Digits", digits);
}

void AuGameHiLoSetup::KickPlayer(int playerIndex)
{
	Q_ASSERT(playerIndex >= 1 && playerIndex < MAX_GAME_PLAYERS);
	m_PlayerIds[playerIndex] = AuId(0);
	m_PlayerNames[playerIndex]->hide();
	m_PlayerKick[playerIndex]->hide();
}

void AuGameHiLoSetup::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuGameHiLoSetup::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/GameHiLoSetup/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/GameHiLoSetup/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/GameHiLoSetup/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/GameHiLoSetup/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuGameHiLoSetup::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/GameHiLoSetup/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/GameHiLoSetup/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/GameHiLoSetup/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/GameHiLoSetup/SizeY",sz.height());
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGameHiLoTurn::AuGameHiLoTurn(const AuGameHiLo& game, QWidget* parent) : AuDialog(parent)
{
	m_Digits = game.Digits();
	setAuDlgIcon(QIcon(":data/GameHiLo.png"));
	setAuDlgTitle(tr("<b>HiLo Number Game Turn</b>"));
	setObjectName(game.TurnUIName());

	// Add a box to set the number.
	m_Number = new QLineEdit(this);
	QString mask;
	for(int dig=0; dig < m_Digits; dig++)
		mask += "9";	// 0-9 digit required at each position.
	m_Number->setInputMask(mask);
	connect(m_Number, SIGNAL(textChanged(const QString&)), SLOT(NumberChanged(const QString&)));

	QLabel* lbante = new QLabel(tr("Ante %1").arg(QString("%L1%2").arg(game.Ante()).arg(g_Au.CurrencySymbol())), this);

	m_Ok = new QPushButton(tr("Accept Number"), this);
	m_Ok->setToolTip(tr("<html>Accepts the number you have chosen. "
				"Ante will be deducted from your personal account.</html>"));
	m_Ok->setAutoDefault(true);
	m_Ok->setDefault(true);
	m_Ok->setEnabled(false);
	connect(m_Ok, SIGNAL(clicked()), SLOT(Turn()));

	QPushButton* quit = new QPushButton(tr("Quit"), this);
	quit->setToolTip(tr("<html>Quits the game without entering a number. No money is anted.</html>"));
	connect(quit, SIGNAL(clicked()), SLOT(QuitGame()));

	// Label the number input box.
	QLabel* lbnumber = new QLabel(QObject::tr("Enter A %1 Digit Number").arg(m_Digits), this);

	// Create the layout of the settings dialog.
	QGridLayout* lay = new QGridLayout;

	// Add game controls.
	int row=0;
	lay->addWidget(lbante, row++,0, 1,2);
	lay->addWidget(lbnumber, row++,0, 1,2);
	lay->addWidget(m_Number, row++,0, 1,2);

	// Add the Turn and Quit buttons.
	lay->addWidget(m_Ok, row,0);
	lay->addWidget(quit, row,1);

	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuGameHiLoTurn::~AuGameHiLoTurn()
{
}

void AuGameHiLoTurn::Turn()
{
	qint32 number = QVariant(m_Number->text()).toInt();

	// Pack my data into a blob the game will recognize at the other side.
	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_1);
	out << number;

	// Emit a signal that ausceneclient can put into a packet to the server with my game settings.
	emit Accepted(AuGameHiLo::GameType(), ba);
	accept();
}

void AuGameHiLoTurn::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuGameHiLoTurn::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/GameHiLoTurn/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/GameHiLoTurn/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/GameHiLoTurn/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/GameHiLoTurn/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuGameHiLoTurn::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/GameHiLoTurn/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/GameHiLoTurn/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/GameHiLoTurn/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/GameHiLoTurn/SizeY",sz.height());
}

void AuGameHiLoTurn::QuitGame()
{
	emit Quit();
	reject();
}

void AuGameHiLoTurn::NumberChanged(const QString& nuText)
{
	m_Ok->setEnabled(nuText.length() == m_Digits);
}

//-+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuGameHiLoFinish::AuGameHiLoFinish(const QByteArray& gameResult, QWidget* parent) : AuDialog(parent)
{
	setAuDlgIcon(QIcon(":data/GameHiLo.png"));
	setAuDlgTitle(tr("<b>HiLo Number Game Over</b>"));

	// Game result variables.
	quint32 randomNumber;			// The final random number.
	double share;					// Each winner's share of the kitty.
	QStringList winnerNames;		// Names of the winner.
	QStringList playerNames;		// Names of the players.
	QList<quint32> playerNumbers;	// Numbers entered by the players.

	// Deserialize the game result from the blob.
	quint8 listCount;
	quint8 digits;
	QString name;
	quint32 number;
	QDataStream in(gameResult);
	in.setVersion(QDataStream::Qt_4_1);
	in >> randomNumber;
	in >> digits;
	in >> share;
	in >> listCount;				// winner list
	for(; listCount; listCount--)
	{
		in >> name;
		winnerNames.append(name);
	}
	in >> listCount;				// players list
	for(; listCount; listCount--)
	{
		in >> name;
		in >> number;
		playerNames.append(name);
		playerNumbers.append(number);
	}

	// Create a little html report of the results.
	QString result("<html>");

	if(winnerNames.count() == 1)
	{
		// There was only one winner.
		result += tr("<img src=\"qrc:data/c_Money.png\"> %1 <i>WINS</i> %2")
			.arg(winnerNames.first())
			.arg(QString("%L1%2").arg(share).arg(g_Au.CurrencySymbol()));
	}
	else
	{
		// There was more than one winner.
		result += tr("<img src=\"qrc:data/CommCrew.png\"> <i>TIE</i> Between %1 Players!<br><br>").arg(winnerNames.count());

		for(QList<QString>::const_iterator wni = winnerNames.constBegin(); wni != winnerNames.constEnd(); wni++)
			result += QString("%1<br>").arg(*wni);
		result += "<br>";

		result += tr("<img src=\"qrc:data/c_Money.png\"> Each Player Wins %1")
			.arg(QString("%L1%2").arg(share).arg(g_Au.CurrencySymbol()));
	}

	// Reveal the final random number.
	result += "<table><tbody>";
	result += tr("<tr><td></td><td><img src=\"qrc:data/Game.png\"> <b>%1</b></td></tr>").arg(randomNumber, digits, 10, QChar('0'));

	// List the players
	for(int player=0; player<playerNames.count(); player++)
		result += tr("<tr><td><img src=\"qrc:data/Passenger.png\"> %1</td>"
				"<td><img src=\"qrc:data/ToggleMail.png\"> %2</td></tr>").arg(playerNames[player])
			.arg(QString("%1").arg(playerNumbers[player], digits, 10, QChar('0')));
	result += "</tbody></table>";

	// Terminate the report.
	result += "</html>";

	// Create the one and only widget.
	QTextBrowser* tb = new QTextBrowser;
	tb->setReadOnly(true);
	tb->setText(result);

	// Create the overall layout for the dialog.
	QVBoxLayout* lay = new QVBoxLayout;
	lay->addWidget(tb);
	lay->setMargin(2);
	lay->setSpacing(2);
	setAuDlgLayout(lay);
}

AuGameHiLoFinish::~AuGameHiLoFinish()
{
}

void AuGameHiLoFinish::setVisible(bool visible)
{
	if(visible)
	{
		AuDialog::setVisible(visible);
		RestoreGeometry();
	}
	else
	{
		SaveGeometry();
		AuDialog::setVisible(visible);
	}
}

void AuGameHiLoFinish::RestoreGeometry()
{
	int posX = g_Au.Settings().readNumEntry("Widget/GameHiLoFinish/PosX",-123);
	int posY = g_Au.Settings().readNumEntry("Widget/GameHiLoFinish/PosY",-123);
	QSize szdef = sizeHint();
	int sizeX = g_Au.Settings().readNumEntry("Widget/GameHiLoFinish/SizeX",szdef.width());
	int sizeY = g_Au.Settings().readNumEntry("Widget/GameHiLoFinish/SizeY",szdef.height());
	resize(sizeX, sizeY);
	if(posX != -123 && posY != -123)
		move(posX, posY);
}

void AuGameHiLoFinish::SaveGeometry()
{
	QPoint ps = pos();
	QSize sz = size();
	g_Au.Settings().writeEntry("Widget/GameHiLoFinish/PosX",ps.x());
	g_Au.Settings().writeEntry("Widget/GameHiLoFinish/PosY",ps.y());
	g_Au.Settings().writeEntry("Widget/GameHiLoFinish/SizeX",sz.width());
	g_Au.Settings().writeEntry("Widget/GameHiLoFinish/SizeY",sz.height());
}
