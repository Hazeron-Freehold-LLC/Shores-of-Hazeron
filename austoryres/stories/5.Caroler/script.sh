# The block with the empty segment label ':' is the catalyst for starting every story. It is required.
:
	# Basic story info. storyTitle, storyAuthor, and storyBrief are required.
	storyTitle("Caroler");
	storyAuthor("Haxus");
	storyBrief("A caroler offers Christmas tidings.");

	# This story is allowed to start automatically repeatedly.
	storyAutoRepeat();
	storyInterval(60);	# Minutes before this script can start again, after starting once.

	# At least one 'if' is required; when any of them returns true, the story can start.
	P = avatar();
	city = atomCity(P);
	if(cityPopulation(city) > 0 &
			dateIsChristmas())
		carolerArrives;

:carolerArrives
	Noël = atomCreateCharacter(P, 100, 6, "Noël", Female, Right Handed, Empire, Worker, Juvenile);
	sentientCreateGear(Noël, Wear, false, Jeans and Shirt, 1, Empire, "");
	sentientCreateGear(Noël, Hand, false, Candle, 1, Empire, "");
	sentientStay(Noël);
	sentientSayTo(Noël, P, "Merry Christmas to you.", Smile);
	if(atomDistanceToAtom(Noël, P) < 7) offerTidings;

:offerTidings
	sentientSay(Noël, "And the angel said unto them, Fear not: for, behold, I bring you good tidings of great joy, "
			"which shall be to all people. For unto you is born this day in the city of David a Saviour, "
			"which is Christ the Lord.", Smile);
	if(secondsElapsed() >= 10) offerCarol;

:offerCarol
	sentientSayTo(Noël, P, "Would you like to hear a Christmas carol?", Smile);
	sentientSayChoice(Noël, 0, "Yes, please.", Smile);
	sentientSayChoice(Noël, 1, "No, thanks kid.", Smile);
	sentientSayChoice(Noël, 2, "Tell me the story of Christmas.", Smile);
	if(choiceMade() == 0) playCarol;
	if(choiceMade() == 1) goAway;
	if(choiceMade() == 2) tellChristmas;
	if(secondsElapsed() >= 60) goAway;

:tellChristmas
	sentientSay(Noël, "This is the story of Christmas, from the book of Luke, chapter 2.", Grin);
	if(secondsElapsed() >= 5) christmas1;

:christmas1
	sentientSay(Noël, "And it came to pass in those days, that there went out a decree from Caesar Augustus, "
			"that all the world should be taxed.", Smile);
	if(secondsElapsed() >= 10) christmas2;

:christmas2
	sentientSay(Noël, "(And this taxing was first made when Cyrenius was governor of Syria.)", Smile);
	if(secondsElapsed() >= 10) christmas3;

:christmas3
	sentientSay(Noël, "And all went to be taxed, every one into his own city.", Smile);
	if(secondsElapsed() >= 5) christmas4;

:christmas4
	sentientSay(Noël, "And Joseph also went up from Galilee, out of the city of Nazareth, into Judaea, unto the city of David, "
			"which is called Bethlehem; (because he was of the house and lineage of David:)", Smile);
	if(secondsElapsed() >= 15) christmas5;

:christmas5
	sentientSay(Noël, "To be taxed with Mary his espoused wife, being great with child.", Smile);
	if(secondsElapsed() >= 6) christmas6;

:christmas6
	sentientSay(Noël, "And so it was, that, while they were there, the days were accomplished that she should be delivered.", Smile);
	if(secondsElapsed() >= 10) christmas7;

:christmas7
	sentientSay(Noël, "And she brought forth her firstborn son, and wrapped him in swaddling clothes, and laid him in a manger; "
			"because there was no room for them in the inn.", Smile);
	if(secondsElapsed() >= 15) christmas8;

:christmas8
	sentientSay(Noël, "And there were in the same country shepherds abiding in the field, keeping watch over their flock by night.", Smile);
	if(secondsElapsed() >= 12) christmas9;

:christmas9
	sentientSay(Noël, "And, lo, the angel of the Lord came upon them, and the glory of the Lord shone round about them: "
			"and they were sore afraid.", Smile);
	if(secondsElapsed() >= 15) christmas10;

:christmas10
	sentientSay(Noël, "And the angel said unto them, Fear not: for, behold, I bring you good tidings of great joy, "
			"which shall be to all people.", Smile);
	if(secondsElapsed() >= 15) christmas11;

:christmas11
	sentientSay(Noël, "For unto you is born this day in the city of David a Saviour, which is Christ the Lord.", Smile);
	if(secondsElapsed() >= 10) christmas12;

:christmas12
	sentientSay(Noël, "And this shall be a sign unto you; Ye shall find the babe wrapped in swaddling clothes, lying in a manger.", Smile);
	if(secondsElapsed() >= 10) christmas13;

:christmas13
	sentientSay(Noël, "And suddenly there was with the angel a multitude of the heavenly host praising God, and saying,", Smile);
	if(secondsElapsed() >= 10) christmas14;

:christmas14
	sentientSay(Noël, "Glory to God in the highest, and on earth peace, good will toward men.", Smile);
	if(secondsElapsed() >= 10) christmas15;

:christmas15
	sentientSay(Noël, "And it came to pass, as the angels were gone away from them into heaven, the shepherds said one to another, "
			"Let us now go even unto Bethlehem, and see this thing which is come to pass, which the Lord hath made known unto us.", Smile);
	if(secondsElapsed() >= 20) christmas16;

:christmas16
	sentientSay(Noël, "And they came with haste, and found Mary, and Joseph, and the babe lying in a manger.", Smile);
	if(secondsElapsed() >= 10) christmas17;

:christmas17
	sentientSay(Noël, "And when they had seen it, they made known abroad the saying which was told them concerning this child.", Smile);
	if(secondsElapsed() >= 10) christmas18;

:christmas18
	sentientSay(Noël, "And all they that heard it wondered at those things which were told them by the shepherds.", Smile);
	if(secondsElapsed() >= 10) christmas19;

:christmas19
	sentientSay(Noël, "But Mary kept all these things, and pondered them in her heart.", Smile);
	if(secondsElapsed() >= 10) christmas20;

:christmas20
	sentientSay(Noël, "And the shepherds returned, glorifying and praising God for all the things that they had heard and seen, "
			"as it was told unto them.", Smile);
	if(secondsElapsed() >= 15) christmas21;

:christmas21
	sentientSay(Noël, "And when eight days were accomplished for the circumcising of the child, his name was called JESUS, "
			"which was so named of the angel before he was conceived in the womb.", Smile);
	if(secondsElapsed() >= 20) christmas22;

:christmas22
	sentientSay(Noël, "Later on, wise men bearing gifts travel from the east, to see the babe, and an evil king tries to kill him.", Smile);
	sentientSay(Noël, "Would you like to hear my Christmas song?", Smile);
	sentientSayChoice(Noël, 0, "Yes, please.", Smile);
	sentientSayChoice(Noël, 1, "No, thanks kid.", Smile);
	sentientSayChoice(Noël, 2, "Tell the story about the wise men and the evil king.", Smile);
	if(choiceMade() == 0) playCarolAfterStory;
	if(choiceMade() == 1) goAway;
	if(choiceMade() == 2) matthewStart;
	if(secondsElapsed() >= 60) goAway;

:matthewStart
	sentientSay(Noël, "This is the story of the wise men, from the book of Matthew, chapter 2.", Smile);
	if(secondsElapsed() >= 5) matthew1;

:matthew1
	sentientSay(Noël, "Now when Jesus was born in Bethlehem of Judaea in the days of Herod the king, behold, "
			"there came wise men from the east to Jerusalem.", Smile);
	if(secondsElapsed() >= 15) matthew2;

:matthew2
	sentientSay(Noël, "Saying, Where is he that is born King of the Jews? for we have seen his star in the east, "
			"and are come to worship him.", Smile);
	if(secondsElapsed() >= 15) matthew3;

:matthew3
	sentientSay(Noël, "When Herod the king had heard these things, he was troubled, and all Jerusalem with him.", Smile);
	if(secondsElapsed() >= 10) matthew4;

:matthew4
	sentientSay(Noël, "And when he had gathered all the chief priests and scribes of the people together, "
			"he demanded of them where Christ should be born.", Smile);
	if(secondsElapsed() >= 15) matthew5;

:matthew5
	sentientSay(Noël, "And they said unto him, In Bethlehem of Judaea: for thus it is written by the prophet,", Smile);
	if(secondsElapsed() >= 10) matthew6;

:matthew6
	sentientSay(Noël, "AND THOU BETHLEHEM, IN THE LAND OF JUDA, ART NOT THE LEAST AMONG THE PRINCES OF JUDA: "
			"FOR OUT OF THEE SHALL COME A GOVERNOR, THAT SHALL RULE MY PEOPLE ISRAEL.", Smile);
	if(secondsElapsed() >= 20) matthew7;

:matthew7
	sentientSay(Noël, "Then Herod, when he had privily called the wise men, enquired of them diligently what time the star appeared.", Smile);
	if(secondsElapsed() >= 10) matthew8;

:matthew8
	sentientSay(Noël, "And he sent them to Bethlehem, and said, Go and search diligently for the young child; "
			"and when ye have found him, bring me word again, that I may come and worship him also.", Smile);
	if(secondsElapsed() >= 20) matthew9;

:matthew9
	sentientSay(Noël, "When they had heard the king they departed; and, lo, the star, which they saw in the east, "
			"went before them, till it came and stood over where the young child was.", Smile);
	if(secondsElapsed() >= 18) matthew10;

:matthew10
	sentientSay(Noël, "When they saw the star, they rejoiced with exceeding great joy.", Smile);
	if(secondsElapsed() >= 10) matthew11;

:matthew11
	sentientSay(Noël, "And when they were come into the house, they saw the young child with Mary his mother, and fell down, "
			"and worshipped him: and when they had opened their treasures, they presented unto him gifts; gold, "
			"and frankincense, and myrhh.", Smile);
	if(secondsElapsed() >= 25) matthew12;

:matthew12
	sentientSay(Noël, "And being warned of God in a dream that they should not return to Herod, "
			"they departed into their own country another way.", Smile);
	if(secondsElapsed() >= 15) matthew13;

:matthew13
	sentientSay(Noël, "And when they were departed, behold, the angel of the Lord appeareth to Joseph in a dream, saying, "
			"Arise, and take the young child and his mother, and flee into Egypt, and be thou there until I bring thee word: "
			"for Herod will seek the young child to destroy him.", Smile);
	if(secondsElapsed() >= 30) matthew14;

:matthew14
	sentientSay(Noël, "When he arose, he took the young child and his mother by night, and departed into Egypt:", Smile);
	if(secondsElapsed() >= 10) matthew15;

:matthew15
	sentientSay(Noël, "And was there until the death of Herod: "
			"that it might be fulfilled which was spoken of the Lord by the prophet, saying, OUT OF EGYPT HAVE I CALLED MY SON.", Smile);
	if(secondsElapsed() >= 18) matthew16;

:matthew16
	sentientSay(Noël, "Then Herod, when he saw that he was mocked of the wise men, was exceeding wroth, and sent forth, "
			"and slew all the children that were in Bethlehem, and in all the coasts thereof, from two years old and under, "
			"according to the time which he had diligently enquired of the wise men.", Smile);
	if(secondsElapsed() >= 30) matthew17;

:matthew17
	sentientSay(Noël, "Then was fulfilled that which was spoken by Jeremy the prophet, saying,", Smile);
	if(secondsElapsed() >= 8) matthew18;

:matthew18
	sentientSay(Noël, "IN RAMA WAS THERE A VOICE HEARD, LAMENTATION, AND WEEPING, AND GREAT MOURNING, "
			"RACHEL WEEPING FOR HER CHILDREN, AND WOULD NOT BE COMFORTED, BECAUSE THEY ARE NOT.", Smile);
	if(secondsElapsed() >= 20) matthew19;

:matthew19
	sentientSay(Noël, "But when Herod was dead, behold, an angel of the Lord appeareth in a dream to Joseph in Egypt,", Smile);
	if(secondsElapsed() >= 10) matthew20;

:matthew20
	sentientSay(Noël, "Saying, Arise, and take the young child and his mother, and go into the land of Israel: "
			"for they are dead which sought the young child's life.", Smile);
	if(secondsElapsed() >= 18) matthew21;

:matthew21
	sentientSay(Noël, "And he arose, and took the young child and his mother, and came into the land of Israel.", Smile);
	if(secondsElapsed() >= 12) matthew22;

:matthew22
	sentientSay(Noël, "But when he heard that Archelaus did reign in Judaea in the room of his father Herod, "
			"he was afraid to go thither: notwithstanding, being warned of God in a dream, "
			"he turned aside into the parts of Galilee:", Smile);
	if(secondsElapsed() >= 25) matthew23;

:matthew23
	sentientSay(Noël, "And he came and dwelt in a city called Nazareth: that it might be fulfilled which was spoken by the prophets, "
			"He shall be called a Nazarene.", Smile);
	if(secondsElapsed() >= 15) matthewEnd;

:matthewEnd
	sentientSay(Noël, "That is the end of Matthew book 3.", Smile);
	sentientSay(Noël, "Would you like to hear my Christmas song?", Smile);
	sentientSayChoice(Noël, 0, "Yes, please.", Smile);
	sentientSayChoice(Noël, 1, "No, thanks kid.", Smile);
	if(choiceMade() == 0) playCarolAfterStory;
	if(choiceMade() == 1) goAway;
	if(secondsElapsed() >= 60) goAway;

:playCarol
	sentientSayAudio(Noël, SilentNight.wav, "Here goes...", Smile);
	if(secondsElapsed() >= 45) donePlaying;

:donePlaying
	sentientSay(Noël, "I just learned to play this musical instrument.", Smile);
	sentientSay(Noël, "Silent Night is the only song I know.", Smile);
	sentientSay(Noël, "Would you like to hear it again?", Smile);
	sentientSayChoice(Noël, 0, "Yes, please.", Smile);
	sentientSayChoice(Noël, 1, "No, thanks kid.", Smile);
	sentientSayChoice(Noël, 2, "Tell me the story of Christmas.", Smile);
	if(choiceMade() == 0) playCarolAgain;
	if(choiceMade() == 1) goAway;
	if(choiceMade() == 2) tellChristmas;
	if(secondsElapsed() >= 30) goAway;

:playCarolAfterStory
	sentientSayAudio(Noël, SilentNight.wav, "Here goes...", Smile);
	if(secondsElapsed() >= 45) donePlayingAfterStory;

:donePlayingAfterStory
	sentientSay(Noël, "I just learned to play this musical instrument.", Smile);
	sentientSay(Noël, "Silent Night is the only song I know.", Smile);
	sentientSay(Noël, "Would you like to hear it again?", Smile);
	sentientSayChoice(Noël, 0, "Yes, please.", Smile);
	sentientSayChoice(Noël, 1, "No, thanks kid.", Smile);
	if(choiceMade() == 0) playCarolAgain;
	if(choiceMade() == 1) goAway;
	if(secondsElapsed() >= 30) goAway;

:playCarolAgain
	sentientSayAudio(Noël, SilentNight.wav, "Here goes again...", Smile);
	if(secondsElapsed() >= 45) donePlayingAgain;

:donePlayingAgain
	sentientSay(Noël, "I think I am getting better with practice.", Smile);
	sentientSay(Noël, "Would you like to hear it again?", Smile);
	sentientSayChoice(Noël, 0, "Yes, please.", Smile);
	sentientSayChoice(Noël, 1, "No, thanks kid.", Smile);
	if(choiceMade() == 0) playCarolAgain;
	if(choiceMade() == 1) goAway;
	if(secondsElapsed() >= 30) goAway;

:goAway
	sentientSay(Noël, "Peace and good will to you.", Smile);
	# The story ends when the current block does not branch.
