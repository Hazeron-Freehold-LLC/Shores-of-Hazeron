# The block with the empty segment label ':' is the catalyst for starting every story. It is required.
:
	# Basic story info. storyTitle, storyAuthor, and storyBrief are required.
	storyTitle("Beacon");
	storyAuthor("Haxus");
	storyBrief("A beacon calls for attention.");

	# This story is allowed to start automatically repeatedly until the avatar completes it.
	# The avatar can drop this story over and over until it is finished.
	storyAutoRepeatUntilComplete();
	storyInterval(1);	# Minutes before this script can start again, after starting once.

	# At least one 'if' is required; when any of them returns true, the story can start.
	av = avatar();
	ship = unitInsideShip(av);
	digwld = findPlanet(Uninhabitable, 90);	# 90 is max hydrographics
	sys = avatarEnvironment();
	sec = atomSector(sys);
	if(avatarStoryCompleted(7) &
			avatarHasItemSpecific(Type K Preon Ring, 182, Malacon, "<i>To Xian with Love from Mill</i>") &
			atomValid(digwld) &
			sectorHasHengeSystem(sec,1) &
			blueprintSpacecraftLoad(2007))		# true when the design is loaded. Signals the need for it to be loaded.
		begin;
	#if(avatarStoryCompleted(7))
	#		debugStart;

:begin
	av = avatar();
	ship = unitInsideShip(av);
	digwld = findPlanet(Uninhabitable, 90);	# 90 is max hydrographics
	wll = worldLandLocation(digwld, 2, 50);	# decoy dig location
	rll = worldLandLocation(digwld, 3, 100);	# real location of the dig
	beac = worldCreateItem(digwld, wll, false, Beacon, 167, Empire, "");
	itemSetMessage(beac, "Archaeological Research Expedition Remote Operating Location 687");
	vll = worldRelativeLL(digwld, wll, 0, 40);
	worldCreateVehicle(digwld, vll, Space Transport, 178, Empire);
	gll = worldRelativeLL(digwld, vll, 8, 0);
	worldCreateItem(digwld, gll, false, Hydrogen, 197, Empire, "");
	gll = worldRelativeLL(digwld, vll, 8, 1);
	worldCreateItem(digwld, gll, false, Hydrogen, 197, Empire, "");
	tll = worldRelativeLL(digwld, wll, 25, 0);
	tent = worldCreateItem(digwld, tll, false, Pressure Tent, 201, Empire, "");
	hostage = atomCreateCharacter(tent, 0, 0, "Duster", Male, Right Handed, Empire, Avatar, Mature);
	sentientCreateGear(hostage, Wear, false, EV Suit, 192, Empire, "");
	sentientCreateGear(hostage, Head, false, EV Helmet, 192, Empire, "");
	sentientCreateGear(hostage, Waist, false, Plastic Explosive, 188, Ogar, "");
	# bomb = worldCreateItem(digwld, tll, false, Plastic Explosive, 188, Ogar, "");
	bomb = sentientDropGear(hostage, Waist);	# This seems to work better on steep hills.
	sentientCreateGear(hostage, Waist, true, Paper, 192, Empire,
			"<b>Flight Plan</b><br>"
			"x Check weather<br>"
			"x Load pressure tent onto transport<br>"
			"x Preflight transport<br>"
			"x Fly direct to aux location [wll]<br>"
			"x Erect pressure tent<br>"
			"o Check weather<br>"
			"o Refuel transport<br>"
			"o Preflight transport<br>"
			"o Fly direct to ROL687 [rll]<br>"
			"o Go direct to rec tent");
	itemArmWeapon(bomb, hostage, Zuul);
	if(wait(30)) title;

:title
	narratorTitle("Beacon");
	if(atomDistanceToAtom(av, hostage) < 7) pleadForHelp;

:pleadForHelp
	sentientSayTo(hostage, av, "STOP! It is a trap!", Sad);
	sentientSayTo(hostage, av, "I cannot move and there is a bomb under me. It could go off at any moment.", Sad);
	sentientSayTo(hostage, av, "Please help me if you can. I do not want to die.", Sad);
	if(!atomValid(hostage) | creatureIsDyingOrDead(hostage)) hostageKilled;
	if(!atomValid(bomb) & !creatureIsDyingOrDead(hostage)) defused;
	if(atomDistanceToAtom(av, hostage) > 10) detonate;
	if(wait(30)) waitAllDay;

:waitAllDay
	itemDetonate(bomb);
	# Make Dr. Karlov's spacecraft.
	sll = worldRelativeLL(digwld, rll, 0, 80);
	ship = worldCreateShip(digwld, sll, 20, 2007, 211, Ogar, "Incisor");
	shipChargeCapacitor(ship);
	shipRefuel(ship, 192);
	shipPowerPlant(ship, 1);
	shipManeuverDrive(ship, 1);
	shipShields(ship, 1);
	shipChargeShields(ship);
	shipInteriorLight(ship, 2);
	shipCreateCrew(ship, Bullywug.chr);
	shipCreateTroops(ship, Bullywug.chr);
	karlov = shipCreateCharacter(ship, 1, "Dr. Karlov", Male, Right Handed, Ogar, ShortBaldEyeSockets.chr, Mature);
	sentientCreateGear(karlov, Wear, false, Jeans and Shirt, 100, Ogar, "");
	atomComm(karlov, Hail, System, "Time's up moron. Did you think I was going to wait all day?");
	goto buildDigSite;

:detonate
	itemDetonate(bomb);
	goto hostageKilled;

:hostageKilled
	# Make Dr. Karlov's spacecraft.
	sll = worldRelativeLL(digwld, rll, 0, 80);
	ship = worldCreateShip(digwld, sll, 20, 2007, 211, Ogar, "Incisor");
	shipChargeCapacitor(ship);
	shipRefuel(ship, 192);
	shipPowerPlant(ship, 1);
	shipManeuverDrive(ship, 1);
	shipShields(ship, 1);
	shipChargeShields(ship);
	shipInteriorLight(ship, 2);
	shipCreateCrew(ship, Bullywug.chr);
	shipCreateTroops(ship, Bullywug.chr);
	karlov = shipCreateCharacter(ship, 1, "Dr. Karlov", Male, Right Handed, Ogar, ShortBaldEyeSockets.chr, Mature);
	sentientCreateGear(karlov, Wear, false, Jeans and Shirt, 100, Ogar, "");
	atomComm(karlov, Hail, System, "Ahahahahaaa! Moron. Did you really think you could match wits with me?");
	goto buildDigSite;

:defused
	sentientSayTo(hostage, av, "Whew! Thank you for freeing me.", Open);
	sentientSayTo(hostage, av, "I flew over here from the ROL to set up this pressure tent.", Open);
	sentientSayTo(hostage, av, "They said they were going to start an auxiliary dig site.", Open);
	if(!atomValid(hostage) | creatureIsDyingOrDead(hostage)) hostageKilled;
	if(wait(25)) doubleCrossed;

:doubleCrossed
	sentientSayTo(hostage, av, "That is when Dr. Karlov's bullywugs jumped me.", Frown);
	sentientSayTo(hostage, av, "They said we had guests and I was going to be the welcoming committee.", Frown);
	if(!atomValid(hostage) | creatureIsDyingOrDead(hostage)) hostageKilled;
	if(wait(25)) cropDuster;

:cropDuster
	sentientSayTo(hostage, av, "A friend talked me into coming along on this dig, to be the site pilot.", Grim);
	sentientSayTo(hostage, av, "He said archaelogy was cool and the girls were hot, and I would get to fly on exotic worlds.", Grim);
	if(!atomValid(hostage) | creatureIsDyingOrDead(hostage)) hostageKilled;
	if(wait(30)) crap;

:crap
	sentientSayTo(hostage, av, "I fly crop dusters. I did not sign up for this crap!", Sad);
	if(!atomValid(hostage) | creatureIsDyingOrDead(hostage)) hostageKilled;
	if(wait(10)) exitStageLeft;

:exitStageLeft
	sentientDropGear(hostage, Waist);
	sentientSayTo(hostage, av, "Fly yourselves back to the ROL; I'm out of here!", Sad);
	sentientGoToLL(hostage, rll);	# head off on foot to the other site
	goto depart;

:depart
	# Make Dr. Karlov's spacecraft.
	sll = worldRelativeLL(digwld, rll, 0, 80);
	ship = worldCreateShip(digwld, sll, 20, 2007, 211, Ogar, "Incisor");
	shipChargeCapacitor(ship);
	shipRefuel(ship, 192);
	shipPowerPlant(ship, 1);
	shipManeuverDrive(ship, 1);
	shipShields(ship, 1);
	shipChargeShields(ship);
	shipInteriorLight(ship, 2);
	shipCreateCrew(ship, Bullywug.chr);
	shipCreateTroops(ship, Bullywug.chr);
	karlov = shipCreateCharacter(ship, 1, "Dr. Karlov", Male, Right Handed, Ogar, ShortBaldEyeSockets.chr, Mature);
	sentientCreateGear(karlov, Wear, false, Jeans and Shirt, 100, Ogar, "");
	atomComm(karlov, Hail, System, "Very clever, evading my little trap.");
	atomComm(karlov, Hail, System, "It won't do you any good moron. My intellect is far superior to yours.");
	goto buildDigSite;

:buildDigSite
	atomComm(karlov, Hail, System, "Farewell losers; you were a bit late. I got what I needed.");
	atomComm(karlov, Hail, System, "Better luck next time, heh.");
	shipRetreat(ship);
	# Build the real dig site.
	tll = worldRelativeLL(digwld, rll, -25, 25);
	worldCreateItem(digwld, tll, false, Pressure Tent, 201, Empire, "");
	cll = worldRelativeLL(digwld, rll, -25, 26);
	worldCreateItem(digwld, cll, false, Crate, 163, Empire, "");
	cll = worldRelativeLL(digwld, rll, -26, 25);
	worldCreateItem(digwld, cll, false, Crate, 163, Empire, "");
	cll = worldRelativeLL(digwld, rll, -24, 23);
	crate = worldCreateItem(digwld, cll, false, Crate, 163, Empire, "");

	mimi = atomCreateCharacter(crate, 0, 2, "Mimi", Female, Right Handed, Empire, CatLightColor.chr, Mature);
	characterSetIndestructible(mimi, true);
	sentientCreateGear(mimi, Wear, false, Jeans and Shirt, 200, Empire, "");
	sentientCreateGear(mimi, Wear, false, EV Suit, 192, Empire, "");
	sentientCreateGear(mimi, Head, false, EV Helmet, 192, Empire, "");
	sentientCreateGear(mimi, Hand, false, Lantern, 129, Empire, "");

	tll = worldRelativeLL(digwld, rll, -25, -25);
	worldCreateItem(digwld, tll, false, Pressure Tent, 201, Empire, "");

	tll = worldRelativeLL(digwld, rll, 25, 0);
	worldCreateItem(digwld, tll, false, Pressure Tent, 201, Empire, "");

	tll = worldRelativeLL(digwld, rll, 25, 25);
	worldCreateItem(digwld, tll, false, Pressure Tent, 201, Empire, "");

	tll = worldRelativeLL(digwld, rll, 25, -25);
	worldCreateItem(digwld, tll, false, Pressure Tent, 201, Empire, "");

	cll = worldRelativeLL(digwld, rll, 35, -40);
	worldCreateItem(digwld, cll, false, Crate, 163, Empire, "");
	cll = worldRelativeLL(digwld, rll, 33, -40);
	worldCreateItem(digwld, cll, false, Crate, 163, Empire, "");

	if(atomDistanceToAtom(av, mimi) < 5) whoAreYou;

:whoAreYou
	sentientSayTo(mimi, av, "Who are you? Please don't hurt me.", Frown);
	goto recognized;

:recognized
	sentientSayTo(mimi, av, "Oh, you are [av.Name]. Dr. Millford talked about you.", Smile);
	goto bullywugs;

:bullywugs
	sentientSayTo(mimi, av, "Dr. Karlov showed up a few days ago and started throwing his weight around.", Frown);
	sentientSayTo(mimi, av, "His bullywugs took over the place.", Frown);
	if(wait(20)) notes;

:notes
	sentientSayTo(mimi, av, "Karlov demanded all the computers and notes related to Dr. Millford's work.", Frown);
	sentientSayTo(mimi, av, "As owner of the expedition; he claimed all of the equipment was his.", Frown);
	if(wait(20)) auxcamp;

:auxcamp
	sentientSayTo(mimi, av, "Yesterday the bullywugs and the crop duster pilot loaded up the transport and left.", Grim);
	sentientSayTo(mimi, av, "I think Karlov's ship beamed them back because we never saw the pilot or the transport again.", Grim);
	sentientSayTo(mimi, av, "They took the landing beacon so I don't know how the supply ship is going to find us.", Grim);
	if(wait(30)) dagger;

:dagger
	sentientSayTo(mimi, av, "Karlov had the ancient ceremonial dagger that Pam'l discovered.", Grim);
	sentientSayTo(mimi, av, "She was one of the newer interns; it was a big deal for her.", Grim);
	if(wait(20)) example;

:example
	sentientSayTo(mimi, av, "Pam'l was brought out and questioned in front of everyone about the dagger.", Sad);
	sentientSayTo(mimi, av, "After he was finished, Karlov decided to make an example out of her.", Sad);
	sentientSayTo(mimi, av, "He let the bullywugs butcher and eat her in front of the whole team. It was awful.", Sad);
	if(wait(30)) loadup;

:loadup
	sentientSayTo(mimi, av, "The bullywugs got really excited when your ship arrived in orbit. That's when they herded everybody into their ship.", Open);
	sentientSayTo(mimi, av, "They snickered among themselves as they loaded up.", Open);
	sentientSayTo(mimi, av, "I hid among these crates, figuring I'd take my chances until the supply ship arrived.", Open);
	if(wait(25)) drmillton;

:drmillton
	sentientSayTo(mimi, av, "Karlov was really upset that Dr. Millton was already gone when he got here.", Open);
	sentientSayTo(mimi, av, "He stomped and cursed for quite some time, throwing things about.", Open);
	sentientSayTo(mimi, av, "It infuriated him when nobody knew where Dr. Millton went.", Open);
	if(wait(30)) exceptme;

:exceptme
	sentientSayTo(mimi, av, "Teehee, except me. I know where he went.", Grin);
	goto wheredhego;

:wheredhego
	sentientSayTo(mimi, av, "The night before he left, Dr. Millton asked me to help him load up the other transport.", Smile);
	sentientSayTo(mimi, av, "His destination was supposed to be a secret, but I think he would want you to know.", Smile);
	sentientSayTo(mimi, av, "He could be in danger after Karlov gets done cracking his computer.", Smile);
	if(wait(30)) hewent;

:hewent
	sec = atomSector(digwld);
	sys = sectorHengeSystem(sec,1);
	hwld = hengeSystemWorld(sys,1);
	hsysname = hengeSystemName(sys);
	hwname = hengeWorldName(hwld);
	if(atomInSystem(av, sys)) wentnear;
	if(!atomInSystem(av, sys)) wentfar;

:wentnear
	# Henge world is in this system.
	sentientSayTo(mimi, av, "Dr. Millford took the transport to [hwname] in this system.", Open);
	sentientSayTo(mimi, av, "That's where we were working before we came here; the environment was so much nicer there.", Open);
	sentientSayTo(mimi, av, "He wanted to talk again with the village shaman.", Open);
	if(wait(30)) illstay;

:wentfar
	# Henge world is in a different system.
	sentientSayTo(mimi, av, "Dr. Millford took the transport to the [hsysname] system in this sector.", Open);
	sentientSayTo(mimi, av, "That's where we were working before we came here; the environment was so much nicer there.", Open);
	sentientSayTo(mimi, av, "He wanted to talk again with the village shaman on [hwname].", Open);
	if(wait(30)) illstay;

:illstay
	sentientSayTo(mimi, av, "The supply ship is supposed to be here within the next few days.", Open);
	sentientSayTo(mimi, av, "I'll pack up the remaining gear so they can haul this stuff back to the university.", Open);
	sentientSayTo(mimi, av, "I will tell them what happened.", Open);
	if(wait(30)) goodbye;

:goodbye
	sentientSayTo(mimi, av, "Good bye. I hope Dr. Millford is ok.", Smile);
	goto syswait;

:syswait
	narratorHint("Travel to [hwname] in the [hsysname] system.");
	if(atomInSystem(av, sys)) arrivedHengeSystem;

:arrivedHengeSystem
	# Wait for a bit to make sure they stay in the system and don't just shoot through the edge at warp speed.
	# This also lets them get in a little closer when traveling direct through open space.
	narratorHint("Wait.");
	if(wait(120)) checkAgain;

:checkAgain
	narratorHint("Wait.");
	hwld = hengeSystemWorld(sys,1);
	hll = hengeWorldLL(hwld);
	if(atomInSystem(av, sys) & !hengeWorldPortalActive(hwld,hll)) portalClear;	# Must not start if the portal is active.
	if(!atomInSystem(av, sys)) syswait;		# They left the system before two minutes was up.

#:debugStart
	# Debug the ceremony starting from here.
#av = avatar();
#sec = atomSector(av);
#sys = avatarEnvironment();
#hwld = hengeSystemWorld(sys,1);
#hll = hengeWorldLL(hwld);
#hsysname = hengeSystemName(sys);
#hwname = hengeWorldName(hwld);
#if(!hengeWorldPortalActive(hwld,hll)) portalClear;

:portalClear
	# Indigenous village
	hwldid = hengeWorld(hwld);	# Safe to get actual world id now that they are in the system.
	tll = worldLandLocationNear(hwldid, hll, 1, 100, 200);	# location of village
	worldCreateIndigenous(hwldid, tll, Avatar);		# force indigenous village spawn with villagers

	# Dr. Millford's transport
	vll = worldRelativeLL(hwldid, tll, 30, 0);
	veh = worldCreateVehicle(hwldid, vll, Space Transport, 178, Empire);
	vehicleRefuel(veh);
	goto campReady;

:campReady
	xll = worldRelativeLL(hwldid, tll, 3, 1);	# Don't give them the exact location of the fire.
	mill = worldCreateCharacter(hwldid, hll, "Dr. Millford", Male, Right Handed, Empire, Avatar, Mature);
	atomComm(mill, Hail, System, "<whispering> Can anybody read me? I'm at [xll] on [hwldid.Name].");
	atomComm(mill, Hail, System, "<whispering> The village shaman confined me to a tent under guard; I'm not sure what's going on.");
	atomComm(mill, Hail, System, "<whispering> Abei says they are preparing me for a great honor.");
	if(wait(30)) theyAreHere;

:theyAreHere
	atomComm(mill, Hail, System, "<whispering> They're here!");
	atomComm(mill, Hail, System, "<whispering> Uh oh! This does not look good.");
	goto village;

:village
	narratorHint("Go to the village at [xll] on [hwldid.Name].");
	characterDepart(mill);						# We'll bring him back in a bit.
	vll = worldRelativeLL(hwldid, tll, 0, -3);
	villager = worldCreateCharacter(hwldid, vll, "Abei", Female, Left Handed, None, Indigenous, Mature);
	characterSetIndestructible(villager, true);
	sentientCreateGear(villager, Hand, false, Candle, 188, None, "");
	if(atomDistanceToAtom(av, villager) < 4) badshaman;

:badshaman
	sentientSayTo(villager, av, "Help us. Shaman Lotar has lost his mind.", Sad);
	sentientSayTo(villager, av, "He is obsessed with the ceremony of life.", Sad);
	if(wait(15)) hellandback;
#goto timetogo;		# debug

:hellandback
	sentientSayTo(villager, av, "Many years ago he said he was transported to hell and back.", Frown);
	sentientSayTo(villager, av, "No one believes him.", Frown);
	sentientSayTo(villager, av, "Kids make fun of him and pretend they are demons.", Frown);
	if(wait(20)) obsession;

:obsession
	sentientSayTo(villager, av, "His obsession got worse after the outworlders showed up.", Open);
	sentientSayTo(villager, av, "The more he talked to Dr. Millford, the wilder he became.", Open);
	if(wait(20)) millreturns;

:millreturns
	sentientSayTo(villager, av, "When Dr. Millford returned yesterday, Lotar ordered the adults of the village to seize him.", Perplexed);
	goto preparehim;

:preparehim
	sentientSayTo(villager, av, "Lotar has been frantically preparing for the ceremony of life since then.", Perplexed);
	sentientSayTo(villager, av, "He is convinced Dr. Millford is the key to opening the passage.", Perplexed);
	if(wait(20)) tothesacrifice;

:tothesacrifice
	sentientSayTo(villager, av, "Dr. Millford will be sacrificed.", Sad);
	sentientSayTo(villager, av, "His life will be offered as a symbolic key, to open the door to the underworld.", Sad);
	if(wait(20)) timetogo;

:timetogo
	dll = worldRelativeLL(hwldid, hll, -5, 0);
	drum = worldCreateItem(hwldid, dll, true, Drum, 188, None, "");	# Drum is permanent so the player can take it.
	itemSetState(drum, 1);
	dll = worldRelativeLL(hwldid, dll, 0, -1);
	drummer = worldCreateCharacter(hwldid, dll, "Morgan", Male, Right Handed, None, Indigenous, Mature);
	characterSetIndestructible(drummer, true);

	sentientSayTo(villager, av, "It is time.", Open);
	sentientSayTo(villager, av, "We must go to the ceremony.", Open);
	worldHengeCeremony(hwldid, hll, 1);
	goto departforceremony;

:departforceremony
	sentientHengeCeremony(villager, hll);
	goto ceremony;

:ceremony
	# Create a new Dr. Millford character.
	mill = worldCreateCharacter(hwldid, hll, "Dr. Millford", Male, Right Handed, Empire, Avatar, Mature);
	characterSetIndestructible(mill, true);

	ell = worldRelativeLL(hwldid, hll, 2, 1);
	shaman = worldCreateCharacter(hwldid, ell, "Shaman Lotar", Male, Right Handed, Zuul, Indigenous, Mature);
	characterSetIndestructible(shaman, true);
	sentientCreateGear(shaman, Wear, false, Jeans and Shirt, 200, None, "");
	sentientCreateGear(shaman, Hand, true, Knife, 18, None, "");

	if(atomDistanceToAtom(av, mill) < 15) startceremony;

:startceremony
	sentientSay(shaman, "The ceremony of life begins.", Closed);
	narratorSay("<natives fall silent>");
	goto givelife;

:givelife
	sentientSay(shaman, "The great fire gives light and heat to the world.", Closed);
	sentientSay(villager, "<natives intone> It is warm and safe and merciless.", Closed);
	goto ringOfLife;

:ringOfLife
	sentientSay(shaman, "The ring of life holds everyone in its embrace.", Closed);
	sentientSay(villager, "<natives intone> We are compassed by the ring in life and death.", Closed);
	goto lifeCycle;

:lifeCycle
	sentientSay(shaman, "Life moves from night to day and back to night again in a cycle that never ends.", Closed);
	sentientSay(villager, "<natives chant> WE RISE UP IN THE DAY. WE FALL INTO THE NIGHT.", Closed);
	goto wearelife;

:wearelife
	sentientSay(shaman, "By this sacrifice of life; we shall rise into a new day.", Closed);
	sentientSay(villager, "<natives cry out wildly> WE ARE THE LIFE!", Closed);
	sentientSay(mill, "Aaaaaaah!", Agape);
	if(atomDistanceToAtom(av, mill) < 6) millscreams;

:millscreams
	sentientSay(mill, "[av.Name]! By the Gods!", Sad);
	goto madman;

:madman
	sentientSay(mill, "[av.Name] these savages are burning me alive!", Sad);
	sentientSay(mill, "The village shaman is a madman.", Sad);
	goto thering;

:thering
	narratorHint("Select a preon ring as your current item.");
	narratorHint("You must be standing within the inner ring of large flat stones.");
	sentientSay(mill, "Hold up the ring.", Frown);
	sentientSay(mill, "You must be standing within the circle of large inner stones, but not in the fire!", Frown);
	sentientSay(mill, "HURRY!", Frown);
	if(hengeWorldPortalActive(hwld, hll)) portal;
	if(wait(20)) pleasethering;

:pleasethering
	narratorHint("Select a preon ring as your current item.");
	narratorHint("You must be standing within the inner ring of large flat stones.");
	sentientSay(mill, "[av.Name]! The ring! Hold it up.", Sad);
	if(hengeWorldPortalActive(hwld, hll)) portal;
	if(wait(30)) pleasethering;

:portal
	itemSetState(drum, 0);				# Stop the music
	worldHengeCeremony(hwldid, hll, 0);	# Stop the dancing.
	goto see;

:see
	sentientSay(shaman, "SEE! What did I tell you? Look upon it.", Grin);
	sentientSay(shaman, "Ridicule me no more!", Grin);
	goto descend;

:descend
	sentientSay(shaman, "I shall descend to my master. When I return, you will quail before me.", Grin);
	sentientGoToLLExactly(shaman, hll);
	if(wait(10)) tellhimtogo;

:tellhimtogo
	narratorHint("Enter the portal.");
	sentientGoToAvatar(villager);
	sentientSayTo(villager, av, "Go after them, while you can!", Sad);
	rwld = unitAtWorld(av);
	if(worldIsRingworld(rwld)) otherside;
	if(wait(30)) tellhimtogo;

:otherside
	sentientStay(shaman);
	if(wait(30) & atomDistanceToAtom(av, shaman) < 3) whatisthisplace;

:whatisthisplace
	pwld = unitAtWorld(av);
	sentientSay(shaman, "What is this place?", Perplexed);
	sentientSay(shaman, "It is strange.", Perplexed);
	goto confused;

:confused
	sentientSay(shaman, "When I passed the gate years before, there was a great roar and the sky filled with fire.", Perplexed);
	sentientSay(shaman, "I was terrified, certain I had fallen into the furnace of hades.", Perplexed);
	if(wait(30)) vow;

:vow
	sentientSay(shaman, "Fear overcame me and I dove back through the portal.", Sad);
	sentientSay(shaman, "Later, ashamed of my actions, I vowed to return some day.", Sad);
	if(wait(30)) docwentback;

:docwentback
	sentientSay(shaman, "Dr. Millford was here a short while ago.", Closed);
	sentientSay(shaman, "He staggered and fell back through the portal.", Closed);
	if(wait(20)) goback;

:goback
	sentientSay(shaman, "The portal will not stay open long.", Open);
	sentientSay(shaman, "You should return.", Open);
	sentientSay(shaman, "I cannot go back to a life of ridicule.", Open);
	goto waitingToGetBack;

:waitingToGetBack
	narratorHint("Return to [hwname] in the [hsysname] system.");
	nwld = unitAtWorld(av);
	if(atomIsAtom(hwldid, nwld)) delayForScene;		# They returned to the henge world.
	if(atomValid(nwld) & !atomIsAtom(hwldid, nwld) & !atomIsAtom(pwld, nwld)) theEnd;	# They went to a different world.

:delayForScene
	# Wait a little bit for their scene to develop.
	if(wait(10)) tookHim;

:tookHim
	sentientGoToAvatar(villager);
	sentientSayTo(villager, av, "Dr. Millford stumbled out of the portal smoldering and burnt.", Frown);
	sentientSayTo(villager, av, "Flying people with angry red eyes swooped down and carried him away, laughing with evil glee.", Frown);
	if(wait(20)) karlovflees;

:karlovflees
	sll = worldRelativeLL(hwldid, hll, 0, 200);
	ship = worldCreateShip(hwldid, sll, 20, 2007, 211, Ogar, "Incisor");
	shipChargeCapacitor(ship);
	shipRefuel(ship, 192);
	shipPowerPlant(ship, 1);
	shipManeuverDrive(ship, 1);
	shipShields(ship, 1);
	shipChargeShields(ship);
	shipInteriorLight(ship, 2);
	shipCreateCrew(ship, Bullywug.chr);
	shipCreateTroops(ship, Bullywug.chr);
	shipRetreat(ship);
	karlov = shipCreateCharacter(ship, 1, "Dr. Karlov", Male, Right Handed, Ogar, ShortBaldEyeSockets.chr, Mature);
	mill = shipCreateCharacter(ship, 1, "Dr. Millford", Male, Right Handed, Empire, Avatar, Mature);
	atomComm(mill, Hail, System, "Let me go Karlov. You'll never get away with this.");
	atomComm(karlov, Hail, System, "Fool! My computer experts had no trouble breaking your pathetic security algorithms.");
	atomComm(karlov, Hail, System, "You! Bullywug! Get him to sick bay and shut him up.");
	atomComm(karlov, Hail, System, "There is a little matter to take care of at the university, hahahaha.");
	if(wait(30)) theEnd;

:theEnd
	# The story ends when the current block does not branch.
