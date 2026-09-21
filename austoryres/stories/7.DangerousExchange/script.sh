# The block with the empty segment label ':' is the catalyst for starting every story. It is required.
:
	# Basic story info. storyTitle, storyAuthor, and storyBrief are required.
	storyTitle("Dangerous Exchange");
	storyAuthor("Haxus");
	storyBrief("Meeting with Dr. Xian at the University. A precious exchange takes place. Danger lurks.");

	# This story is allowed to start automatically repeatedly until the avatar completes it.
	# The avatar can drop this story over and over until it is finished.
	storyAutoRepeatUntilComplete();
	storyInterval(1);	# Minutes before this script can start again, after starting once.

	# At least one 'if' is required; when any of them returns true, the story can start.
	P = avatar();
	if(avatarStoryCompleted(6) &
			unitAtBuilding(P, University) &
			avatarHasItemSpecific(Vulcium Knife, 254, Zuul, "<img src=\"qrc:data/Matrix.png\">"))
		begin;

:begin
	narratorTitle("Dangerous Exchange");
	if(secondsElapsed() >= 30) xianArrives;

:xianArrives
	P = avatar();
	xian = atomCreateCharacter(P, 45, 4, "Dr. Xian", Female, Left Handed, Empire, Avatar, Mature);
	characterSetIndestructible(xian, true);	# Don't let Xian die.
	sentientGoToAvatar(xian);		# Walk up to the avatar
	sentientCreateGear(xian, Wear, false, Jeans and Shirt, 200, Empire, "");
	sentientCreateGear(xian, Hand, false, Type K Preon Ring, 182, Malacon, "<i>To Xian with Love from Mill</i>");
	sentientSayTo(xian, P, "Hello. You must be [P.TitleName]. I have been expecting you.", Smile);
	sentientSayTo(xian, P, "Have you heard from Dr. Millton?", Smile);
	if(secondsElapsed() >= 15) talkAboutMillton;

:talkAboutMillton
	sentientStay(xian);
	sentientSayTo(xian, P, "The last email I received from him was over a month ago. "
			"He said his [P.niecenephew] would be coming to visit. "
			"Do you have information about Mill? I am so worried about him.", Smile);
	if(secondsElapsed() >= 30) askForArtifact;

:askForArtifact
	narratorHint("Select the Zuul Vulcium Knife as your current item so Dr. Xian can see it.");
	sentientSayTo(xian, P, "Mill told me you might have something interesting to show me.", Smile);
	I = itemOnGroundSpecific(50, Vulcium Knife, 254, Zuul, "<img src=\"qrc:data/Matrix.png\">");
	if(secondsElapsed() >= 60) askForArtifact;	# repeat my words every minute
	if(avatarHasItemInHandSpecific(Vulcium Knife, 254, Zuul, "<img src=\"qrc:data/Matrix.png\">")) knifeSeen;
	if(atomValid(I)) pickUpKnife;

:pickUpKnife
	narratorHint("Select the Zuul Vulcium Knife as your current item so Dr. Xian can see it.");
	sentientSayTo(xian, P, "That looks like the knife.", Perplexed);
	sentientSayTo(xian, P, "I wanted you to hold it in your hand so I could see it, not throw it on the ground.", Perplexed);
	if(secondsElapsed() >= 60) askForArtifact;	# repeat my words every minute
	if(avatarHasItemInHandSpecific(Vulcium Knife, 254, Zuul, "<img src=\"qrc:data/Matrix.png\">")) knifeSeen;

:knifeSeen
	# The vulcium knife is no longer permanent. It will now be associated with this story instance.
	sentientSayTo(xian, P, "Oh no! This is terrible! Is Dr. Millton ok? What's happened to him?", Frown);
	sentientSayTo(xian, P, "I knew something must be wrong because I hadn't heard from him in a while, "
			"much longer than usual.", Frown);
	if(secondsElapsed() >= 25) knifeIsMessage;

:knifeIsMessage
	sentientSayTo(xian, P, "Mill and I discussed many possibilities in detail; we made plans just in case.", Grim);
	sentientSayTo(xian, P, "The knife is a message to me from Millton. He needs my ring.", Grim);
	if(secondsElapsed() >= 20) ringNeeded;

:ringNeeded
	sentientSayTo(xian, P, "Ohh! You must take this ring. Mill gave it to me; we were going to be married.", Cry2Sec);
	sentientSayTo(xian, P, "He must be in serious trouble to want it. He needs it now. Take the ring to Mill.", Cry2Sec);
	sentientSayTo(xian, P, "Tell him my analysis supports his hypothesis but there are important details he should know.", Cry2Sec);
	if(secondsElapsed() >= 30) troubleArrives;

:troubleArrives
	lennerd = atomCreateCharacter(P, 180, 5, "Lennerd", Male, Right Handed, Ogar, Bullywug.chr, Mature);
	sentientCreateGear(lennerd, Wear, false, EV Suit, 192, Ogar, "");
	sentientCreateGear(lennerd, Head, false, EV Helmet, 192, Ogar, "");
	sentientCreateGear(lennerd, Hand, false, Pistol, 192, Ogar, "");
	sentientCreateGear(lennerd, Waist, false, Pistol Ammunition, 192, Ogar, "");
	sentientCreateGear(lennerd, Waist, false, Pistol Ammunition, 192, Ogar, "");
	sentientCreateGear(lennerd, Waist, false, Pistol Ammunition, 192, Ogar, "");
	sentientCreateGear(lennerd, Waist, false, Pistol Ammunition, 192, Ogar, "");
	sentientFireWeapon(lennerd);
	sentientSayTo(lennerd, xian, "How sweet. I'll take dat! Drop de ring right now brain-hag.", Frown);
	goto weakDemands;

:weakDemands
	sentientSayTo(xian, lennerd, "Who are you? What do you want? Where is Mill?", Perplexed);
	if(!atomValid(lennerd) | creatureIsDyingOrDead(lennerd)) giveRing;
	if(secondsElapsed() >= 10) lennerdInCharge;

:giveRing
	sentientDropGear(xian, Hand);
	if(secondsElapsed() >= 3) revealBaby;

:lennerdInCharge
	sentientGoToUnit(lennerd, xian);
	sentientSayTo(lennerd, xian, "Shuddup! I'm in charge here. Dr. Millton maybe forgot who he works for, eh.", Frown);
	sentientSayTo(lennerd, xian, "Dr. Karlov says to put a stop to it, one of my specialties.", Frown);
	if(!atomValid(lennerd) | creatureIsDyingOrDead(lennerd)) giveRing;
	if(secondsElapsed() >= 20) looseEnds;

:looseEnds
	sentientSayTo(lennerd, xian, "De boss only wants science dat can be made into a weapon.", Closed);
	sentientSayTo(lennerd, xian, "Dis ancient history crap is for de birds, just a bunch of dry old bones, "
			"can't even chew em.", Closed);
	if(!atomValid(lennerd) | creatureIsDyingOrDead(lennerd)) giveRing;
	if(secondsElapsed() >= 20) scram;

:scram
	sentientSayTo(lennerd, P, "Scram [P.Name]. Don't get involved. ", Frown);
	sentientSayTo(lennerd, P, "Put de knife away and leave now before you get hurt!", Frown);
	if(!atomValid(lennerd) | creatureIsDyingOrDead(lennerd)) giveRing;
	if(secondsElapsed() >= 15) submission;

:submission
	sentientSayTo(xian, lennerd, "Please don't hurt me. Here. Here is the ring. Take it.", Cry2Sec);
	sentientItemSetDropOnDeath(xian, 2, Type K Preon Ring, 182, Malacon, "<i>To Xian with Love from Mill</i>");
	sentientSayTo(lennerd, xian, "Tank you for your cooperation. I'll put in a good word for you wit da boss.", Smile);
	characterWantItemSpecific(lennerd, Type K Preon Ring, 182, Malacon, "<i>To Xian with Love from Mill</i>");
	sentientSellGearItem(xian, lennerd, Type K Preon Ring, 0);
	if(!atomValid(lennerd) | creatureIsDyingOrDead(lennerd)) revealBaby;
	if(sentientHasItemSpecific(lennerd, Type K Preon Ring, 182, Malacon, "<i>To Xian with Love from Mill</i>")) yerPregnant;
	if(avatarHasItemSpecific(Type K Preon Ring, 182, Malacon, "<i>To Xian with Love from Mill</i>")) funnyGuy;

:funnyGuy
	sentientSay(lennerd, "Gimme de ring dork. I told you to stay out o dis. Now yer gonna get it!", Frown);
	sentientAttackUnit(lennerd, P);
	if(!atomValid(lennerd) | creatureIsDyingOrDead(lennerd)) revealBaby;
	if(sentientHasItemSpecific(lennerd, Type K Preon Ring, 182, Malacon, "<i>To Xian with Love from Mill</i>")) yerPregnant;
	if(secondsElapsed() >= 20) funnyGuy2;

:funnyGuy2
	sentientSay(lennerd, "Gimme de ring I said!", Frown);
	sentientAttackUnit(lennerd, P);
	if(!atomValid(lennerd) | creatureIsDyingOrDead(lennerd)) revealBaby;
	if(sentientHasItemSpecific(lennerd, Type K Preon Ring, 182, Malacon, "<i>To Xian with Love from Mill</i>")) yerPregnant;
	if(secondsElapsed() >= 20) funnyGuy2;

:yerPregnant
	sentientSayTo(lennerd, xian, "Dr. Karlov asked me to leave you wit a little reminder who's payin yer bills.", Closed);
	if(!atomValid(lennerd) | creatureIsDyingOrDead(lennerd)) revealBaby;
	if(secondsElapsed() >= 10) twoFer;

:twoFer
	sentientSayTo(lennerd, xian, "De company doc says yer pregnant. You and Millton been bumpin uglies eh?", Smile);
	sentientSayTo(lennerd, xian, "Let's see if I can get two fer de price o one.", Smile);
	if(!atomValid(lennerd) | creatureIsDyingOrDead(lennerd)) noTravel;
	if(secondsElapsed() >= 10) handsUpDontShoot;

:handsUpDontShoot
	sentientAttackUnit(lennerd, xian);
	sentientFireWeapon(lennerd);			# make sure Lennerd gets a shot off
	sentientSayTo(xian, lennerd, "Oof! My baby! Don't shoot my baby.", Cry2Sec);
	creatureSetHealth(xian, 10);			# 10% health
	if(!atomValid(lennerd) | creatureIsDyingOrDead(lennerd)) noTravel;
	goto soLong;

:soLong
	sentientSay(lennerd, "So long suckers!", Laugh2Sec);
	sentientMoveBack(lennerd, 1000);		# move away
	if(!atomValid(lennerd) | creatureIsDyingOrDead(lennerd)) noTravel;
	if(secondsElapsed() >= 10) dieBully;

:dieBully
	sentientSay(lennerd, "Wait. Wut? Nooooo! Uh. Aaaaagh! Gu gu gu uh uuuuh.", Sad);
	sentientDropGearItem(lennerd, Type K Preon Ring);
	characterDie(lennerd);
	if(secondsElapsed() >= 15) lastLaugh;

:lastLaugh
	sentientGoToUnit(xian, lennerd);
	sentientSayTo(xian, lennerd, "Eww that worked gruesomely well. His brain was minced into a fine mulch.", Sardonic);
	sentientSayTo(xian, P, "My experiments with weaponized nanobot meat grinders proved useful after all.", Sardonic);
	sentientSayTo(xian, P, "It pays to have a few tricks up your sleeve.", Sardonic);
	if(secondsElapsed() >= 30) noTravel;

:revealBaby
	sentientSayTo(xian, P, "Thank you [P.Name] for saving me from that evil Bullywug. I was afraid for my baby.", Open);
	sentientSayTo(xian, P, "Mill doesn't know. I am pregnant with his child.", Open);
	if(secondsElapsed() >= 20) noTravel;

:noTravel
	sentientSayTo(xian, P, "I cannot travel in my condition; it's best if I stay. ", Grim);
	sentientSayTo(xian, P, "I know a close friend who will take me in, discreetly, until the baby is born.", Grim);
	if(secondsElapsed() >= 20) getKnife;

:getKnife
	I = itemOnGroundSpecific(50, Vulcium Knife, 254, Zuul, "<img src=\"qrc:data/Matrix.png\">");
	sentientGetItem(xian, I);
	if(sentientHasItemSpecific(xian, Vulcium Knife, 254, Zuul, "<img src=\"qrc:data/Matrix.png\">")) studyKnife;
	if(avatarHasItemSpecific(Vulcium Knife, 254, Zuul, "<img src=\"qrc:data/Matrix.png\">")) requestKnife;
	if(!atomValid(I)) knifeLost;

:knifeLost
	sentientSayTo(xian, P, "Oh no, I can't find the knife.", Sad);
	sentientSayTo(xian, P, "Please help me find it. It's important.", Sad);
	if(secondsElapsed() >= 15) getKnife;

:requestKnife
	sentientSayTo(xian, P, "May I have the knife, please?", Smile);
	characterWantItemSpecific(xian, Vulcium Knife, 254, Zuul, "<img src=\"qrc:data/Matrix.png\">");
	if(secondsElapsed() >= 10) getKnife;

:studyKnife
	storyDroppable(false);	# Xian has the knife so the story must not be dropped or it cannot be restarted.
	sentientSayTo(xian, P, "I have the knife. I will study it in my lab.", Grim);
	sentientSayTo(xian, P, "The hilt has an imprint I would like to examine closely. It may prove useful to you and Mill.", Grim);
	sentientSayTo(xian, P, "I will consult with a colleague who specializes in ancient technology.", Grim);
	if(secondsElapsed() >= 30) takeRing;

:takeRing
	narratorHint("You must possess the Malacon Preon Ring to complete the story.");
	sentientSayTo(xian, P, "Take the ring [P.Name]. I think that Bullywug dropped it when he fell.", Open);
	sentientSayTo(xian, P, "Find Mill! He may still be at the dig site.", Open);
	sentientSayTo(xian, P, "They always set up a beacon for the supply ships. You will hear it when you are in the system.", Open);
	# Don't let the story end until the avatar has the preon ring.
	if(avatarHasItemSpecific(Type K Preon Ring, 182, Malacon, "<i>To Xian with Love from Mill</i>")) theEnd;

:theEnd
	sentientSayTo(xian, P, "You are involved now.", Open);
	sentientSayTo(xian, P, "You must bring Millton back to me. He will know how to find me.", Open);
	sentientSayTo(xian, P, "Dr. Karlov is a nasty guy. He will try to stop you.", Open);
	sentientSayTo(xian, P, "Be careful [P.Name].", Open);
	# Must get to here to keep the ring and complete the story.
	# Change the item so it won't drop if they die. It is needed later.
	avatarItemSetDropOnDeath(1, Type K Preon Ring, 182, Malacon, "<i>To Xian with Love from Mill</i>");
	# Once the item becomes permanent it loses its association with the story. The story can no longer make changes to it.
	avatarItemMakePermanent(Type K Preon Ring, 182, Malacon, "<i>To Xian with Love from Mill</i>");
	# The story ends when the current block does not branch.
