# The block with the empty segment label ':' is the catalyst for starting every story. It is required.
:
	# Basic story info. storyTitle, storyAuthor, and storyBrief are required.
	storyTitle("Relic");
	storyAuthor("Haxus");
	storyBrief("A package arrives by courier. It contains a mystery.");

	# This story is allowed to start automatically repeatedly until the avatar completes it.
	# The avatar can drop this story over and over until it is finished.
	storyAutoRepeatUntilComplete();
	storyInterval(1500);	# a little longer than daily

	# At least one 'if' is required; when any of them returns true, the story can start.
	P = avatar();
	sys = avatarEnvironment();
	ship = unitAboardShip(P);
	sta = unitStationInTradeRange(ship, Neutral, 1);
	if(atomValid(sta) &											# Avatar's ship is in trade range of a station.
			atomHasBuilding(sys, University, 1, Neutral, 1))	# Solar system contains a university.
		begin;

:begin
	narratorTitle("Relic");
	if(secondsElapsed() >= 30) packageArrives;

:packageArrives
	P = avatar();
	ship = unitInsideShip(P);
	sta = unitStationInTradeRange(ship, Neutral, 1);
	atomComm(sta, Hail, Hail, "[P.TitleName], we are holding a Parcel Xpress package for you.");
	if(!atomValid(sta)) waitForStation;
	if(secondsElapsed() >= 10) offerPackage;

:offerPackage
	sta = unitStationInTradeRange(ship, Neutral, 1);
	if(atomIsAtom(sta,ship)) offerAboard;
	if(!atomIsAtom(sta,ship)) offerComeAboard;

:offerAboard
	atomComm(sta, Hail, Hail, "Wait right there and someone will get it for you.");
	goto waitInStation;

:offerComeAboard
	atomComm(sta, Hail, Hail, "Come aboard and someone will get it for you.");
	goto waitInStation;

:waitInStation
	sta = unitStationInTradeRange(ship, Neutral, 1);
	inship = unitInsideShip(P);
	if(!atomValid(sta)) waitForStation;							# station no longer in range? destroyed?
	if(atomIsAtom(sta, inship)) deliverPackage;					# avatar is aboard the station

:waitForStation
	# waiting to be in range of a station again.
	ship = unitInsideShip(P);
	sta = unitStationInTradeRange(ship, Neutral, 1);
	if(atomValid(sta)) packageArrives;

:deliverPackage
	dguy = atomCreateCharacter(P, 0, 2, "ParcX Delivery Guy", Male, Right Handed, Empire, Avatar, Mature);
	characterSetIndestructible(dguy, true);
	sentientCreateGear(dguy, Wear, false, ParcX Uniform, 100, Empire, "");
	sentientCreateGear(dguy, Hand, false, ParcX Box, 100, Empire,
			"<table border=\"1\" cellpadding=\"2\" cellspacing=\"0\"><tbody>"
			"<tr><td><img src=\"qrc:data/ParcXLogo.png\"></td></tr>"
			"<tr><td>Svoth Enterprises<br>Archaeological Research Expedition<br>Remote Operating Location 687</td></tr>"
			"<tr><td>[P.TitleName]<br>"
			"[ship.NameWithEmpire]</td></tr>"
			"</tbody></table>");
	sentientFollowAvatar(dguy);
	sentientSayTo(dguy, P, "[P.TitleName], I have a parcel for you. One moment, please.", Closed);
	if(atomDistanceToAtom(P, dguy) < 3) dropPackage;

:dropPackage
	# Make the box and its contents permanent right away. This is the ending block of the story so we don't have to worry about
	# the player exploiting this free stuff.
	sentientSayTo(dguy, P, "Here you go [P.maamsir].", Closed);
	sentientStay(dguy);
	box = sentientDropGear(dguy, Hand);
	itemMakePermanent(box);
	itemCreateContent(box, false, 1, Vulcium Knife, 254, Zuul, "<img src=\"qrc:data/Matrix.png\">");	# not permanent yet
	itemCreateContent(box, true, 1, Paper, 128, Empire,
			"<i>Dr. Millton<br>"
			"Professor of Archaeology<br>"
			"Svoth Enterprises<br>"
			"Archaeological Research Expedition<br>"
			"Remote Operating Location 687</i><p>"
			"<i>[P.TitleName]<br>"
			"[ship.NameWithEmpire]<p>"
			"<p><i>Dear [P.Name],</i></p>"
			"<p><i>As usual at a dig, the generator is off line so I decided to do this the old fashioned way, with pen and paper. "
			"This little scrap was the only precious piece of paper I could come up with. "
			"You must forgive my hand writing.</i></p>"
			"<p><i>You know you have always been my favorite [P.niecenephew]. "
			"It would warm my heart if I could convince you to pursue archaeology with your old uncle. "
			"Your <s>help</s> assistance would be a huge relief. "
			"I don't know if I have all that many years left in me to do this work. "
			"It's blasted <s>dangerous</s> hard work and the hours are long.</i></p>"
			"<p><i>To grab your interest, I enclosed a most interesting artifact. "
			"It was discovered by an intern at a recent dig. "
	 		"I <s>take</s> took possession of it for 'further study', ahem. "
	 		"The most remarkable thing about this <s>knife</s> dagger is not that it was forged of Q254 vulcium "
			"or its unusually shabby condition. "
	 		"I have dated it accurately; incredibly, it is older <s>to</s> than the oldest known ringworld.</i></p>"
	 		"<p><i>My <s>assistant</s> friend Dr. Xian is being reassigned <s>at</s> to another "
			"<s>university</s> teaching position. "
	 		"Dr. Xian was an extremely competent assistant and valiant <s>protect</s> guardian of my honor at those "
			"grueling peer reviews. "
	 		"She and I were growing close. "
	 		"I will miss her.</i></p>"
	 		"<p><i><s>Dr. Xian</s> She developed the AI used to scan hand written messages for content, the kind that "
			"scans Parcel Xpress packages. "
	 		"It is useful to know what 'the machine' <s>be</s> is capable of and what it's not, like its inability to "
			"read scribbled out words. "
	 		"Sometimes, mistakes reveal what a person really intended to say, no matter how <s>careful</s> "
			"conscientious they are.</i></p>"
	 		"<p><i>Don't let me down; I know you're smart. "
	 		"I am <s>trust</s> depending on you to figure this out. "
	 		"There's <s>no one</s> nobody else for me to turn to, my friend. "
	 		"I hope to see you soon.</i></p>"
	 		"<p><i>Best regards,</i></p>"
	 		"<p><i>Uncle Millton</i></p>"
			"</i>");
	sentientSayTo(dguy, P, "Have a great day [P.maamsir].", Smile);
	characterSetIndestructible(dguy, false);
	goto theEnd;

:theEnd
	narratorHint("You must possess the Zuul Vulcium Knife to complete the story.");
	# Don't let the story end until the avatar has the vulcium knife.
	if(avatarHasItemSpecific(Vulcium Knife, 254, Zuul, "<img src=\"qrc:data/Matrix.png\">")) exit;

:exit
	# Must get to here to keep the knife and complete the story.
	# Change the item so they won't drop it if they die. It is needed later.
	avatarItemSetDropOnDeath(1, Vulcium Knife, 254, Zuul, "<img src=\"qrc:data/Matrix.png\">");
	# Once the item becomes permanent it loses its association with the story. The story can no longer make changes to it.
	avatarItemMakePermanent(Vulcium Knife, 254, Zuul, "<img src=\"qrc:data/Matrix.png\">");
	# The story ends when the current block does not branch.
