# The block with the empty segment label ':' is the catalyst for starting every story. It is required.
:
	# Basic story info. storyTitle, storyAuthor, and storyBrief are required.
	storyTitle("Reach for the Stars");
	storyAuthor("Haxus");
	storyBrief("Targoss teaches how to play Shores of Hazeron, from savage to space age.");

	# This story is allowed to start automatically repeatedly until the avatar completes it.
	# The avatar can drop this story over and over until it is finished.
	storyAutoRepeatUntilComplete();
	storyInterval(3);

	# At least one 'if' is required; when any of them returns true, the story can start.
	av = avatar();
	world = atomWorld(av);
	if(avatarInitScenario(Reach for the Stars) &
			avatarLineageAvatar() &
			unitAtWorld(av) &
			!unitInOrbit(av) &
			!unitAboardShip(av)) begin;

:begin
	narratorTitle("Reach for the Stars");
	if(secondsElapsed() >= 5) targossArrives;

:targossArrives
	narratorSay("Targoss has arrived.");
	narratorHint("Move close to Targoss.");
	av = avatar();
	Targoss = atomCreateCharacter(av, 180, 20, "Targoss", Male, Right Handed, Empire, Avatar, Mature);
	characterSetIndestructible(Targoss, true);
	sentientCreateGear(Targoss, Wear, false, Jeans and Shirt, 1, Empire, "");
	sentientCreateGear(Targoss, Hand, true, Torch, 1, Empire, "");
	sentientFollowAvatar(Targoss);
	if(atomDistanceToAtom(av, Targoss) < 8) welcome;

:welcome
	sentientSayAudio(Targoss, welcome.wav, "Welcome [av.Name], I am here to help you get started.", Smile);
	if(secondsElapsed() >= 10) basicControls;

:basicControls
	sentientSayTo(Targoss, av, "Right-click to release the mouse from controlling your avatar, "
			"so you can operate window controls.", None);
	sentientSayTo(Targoss, av, "Right-click in the scene to return control to your avatar.", None);
	if(secondsElapsed() >= 20) offerHelp;

:offerHelp
	narratorHint("Answer the question on the Voice channel.");
	sentientSayTo(Targoss, av, "I will teach you how to do these things.", None);
	sentientSayTo(Targoss, av, "1. Forage and hand craft things.", None);
	sentientSayTo(Targoss, av, "2. Communicate and trade with people.", None);
	sentientSayTo(Targoss, av, "3. Construct buildings and manufacture things.", None);
	sentientSayTo(Targoss, av, "4. Form a government and establish cities.", None);
	sentientSayTo(Targoss, av, "5. Claim worlds and steal claims.", None);
	sentientSayTo(Targoss, av, "6. Conquer and defend worlds and cities.", None);
	sentientSayTo(Targoss, av, "7. Begin space exploration.", None);
	sentientSayTo(Targoss, av, "Use the Comm (F3) window to answer my question. Check the Voice channel.", None);
	sentientSayTo(Targoss, av, "Double-click or press Enter on a message with a green splat, to choose a reply.", None);
	sentientSayChoice(Targoss, 0, "Thank you. I could use the help.", None);
	sentientSayChoice(Targoss, 1, "No, thank you. I know how to do those things.", None);
	if(choiceMade() == 0) helpWanted;
	if(choiceMade() == 1) goodBye1;
	if(secondsElapsed() >= 120) tellComm;

:goodBye1
	# Story ended early, assertively.
	avatarStoryResult(1);
	goto goodBye;

:goodBye
	sentientStay(Targoss);
	sentientSay(Targoss, "Talk to the Stargazer of the village. The future of your civilization may depend on it.", None);
	sentientSayAudio(Targoss, bye.wav, "Good bye and good luck in your travels.", None);
	characterSetIndestructible(Targoss, false);
	if(secondsElapsed() >= 10) theEnd;	# Wait before Targoss departs or the good bye message never gets sent.

:theEnd
	characterDepart(Targoss);
	# The story ends when the current block does not branch.

:tellComm
	narratorHint("Answer the question on the Voice channel.");
	sentientSayTo(Targoss, av, "Use the Comm (F3) window to answer my question. Check the Voice channel.", None);
	sentientSayChoice(Targoss, 0, "Yes, please help.", None);
	sentientSayChoice(Targoss, 1, "No, go away.", None);
	if(choiceMade() == 0) helpWanted;
	if(choiceMade() == 1) goodBye1;
	if(secondsElapsed() >= 10) helpMightBeWanted;

:helpMightBeWanted
	sentientSayTo(Targoss, av, "You did not answer so I will proceed.", None);
	if(secondsElapsed() >= 10) helpWanted;

:helpWanted
	narratorHint("Double click or press Enter on the item offer message, offering the torch.");
	sentientSayTo(Targoss, av, "If you want me to go away, at any time, look at the Jobs tab on the Bio (F8) window. "
			"Select the 'Reach for the Stars' story and push the Drop button.", None);
	sentientSayTo(Targoss, av, "Most stories will restart if you drop them. This one will not.", None);
	sentientSayTo(Targoss, av, "Take this torch; it will be dark soon.", None);
	sentientSayTo(Targoss, av, "Right click on my offer to make a bid.", None);
	sentientSellGearItem(Targoss, av, Torch, 0);
	if(!sentientHasItem(Targoss, Torch)) village;

:village
	sentientSayTo(Targoss, av, "The village is a safe haven.", None);
	sentientSayTo(Targoss, av, "Wild animals usually leave you alone in tents and near fires.", None);
	sentientSayTo(Targoss, av, "If you die, you will drop an item and reappear at the village. "
			"Find your carcass to recover the item that was lost.", None);
	if(secondsElapsed() >= 40) villagers;

:villagers
	sentientSayTo(Targoss, av, "Say hello to a villager on the voice channel, to start a conversation.", None);
	sentientSayTo(Targoss, av, "Villagers trade useful items.", None);
	sentientSayTo(Targoss, av, "To trade with them, ask villagers what they are selling.", None);
	if(secondsElapsed() >= 60) healer;

:healer
	sentientSayTo(Targoss, av, "The healer stands next to the fire.", None);
	sentientSayTo(Targoss, av, "Talk to the healer to get a check up and medical treatment.", None);
	if(secondsElapsed() >= 50) elder;

:elder
	sentientSayTo(Targoss, av, "The tent closest to the fire belongs to the village elder.", None);
	sentientSayTo(Targoss, av, "Elders offer wisdom. They sometimes reveal the location of important places.", None);
	if(secondsElapsed() >= 50) lookAround;

:lookAround
	sentientSayTo(Targoss, av, "Look around at things in the environment.", None);
	sentientSayTo(Targoss, av, "Press the ` key to get a description.", None);
	sentientSayTo(Targoss, av, "Press Shift+` to say the description aloud, on the current comm channel.", None);
	if(secondsElapsed() >= 50) forage;

:forage
	sentientSayTo(Targoss, av, "Forage useful items from trees, shrubs and rocks.", None);
	sentientSayTo(Targoss, av, "Use the action buttons at the right or click on items to pick them up, using the hand cursor.", None);
	if(secondsElapsed() >= 50) topDown;

:topDown
	sentientSayTo(Targoss, av, "Top down view is helpful when looking for plant and rock resources.", None);
	sentientSayTo(Targoss, av, "Backspace switches to top down view.", None);
	sentientSayTo(Targoss, av, "Use \ and Shift+\ to change information overlays.", None);
	sentientSayTo(Targoss, av, "] and [ adjusts altitude.", None);
	if(secondsElapsed() >= 70) carve;

:carve
	sentientSayTo(Targoss, av, "A knife can carve a carcass into meat, bone and leather.", None);
	sentientSayTo(Targoss, av, "Use the mouse wheel or { } keys to select the knife.", None);
	sentientSayTo(Targoss, av, "Click a carcass on the ground using the knife, in carve mode, not weapon mode.", None);
	sentientSayTo(Targoss, av, "Carve mode is shown by a meat cleaver next to the knife picture in the scene.", None);
	sentientSayTo(Targoss, av, "Beware, the knife does not work as a weapon in carve mode; it won't hurt anything that way.", None);
	sentientSayTo(Targoss, av, "Weapon mode is shown by a splat (contact), because you must strike the target, and a skewer (piercieng damage).", None);
	if(secondsElapsed() >= 60) handCraft;

:handCraft
	sentientSayTo(Targoss, av, "Many useful items can be made by hand.", None);
	sentientSayTo(Targoss, av, "Use the Handcraft button on the Gear (F4) window to make items.", None);
	if(secondsElapsed() >= 20) fireMaking;

:fireMaking
	sentientSayTo(Targoss, av, "Fire gives light; the ability to make it separates you from the animals.", None);
	sentientSayTo(Targoss, av, "Handcraft a fire to keep wild animals away.", None);
	if(secondsElapsed() >= 20) usefulItems;

:usefulItems
	sentientSayTo(Targoss, av, "Clothing and armor protect you from attack by wild animals.", None);
	sentientSayTo(Targoss, av, "A backpack enables you to carry more gear.", None);
	if(secondsElapsed() >= 20) explore;

:explore
	sentientSayTo(Targoss, av, "In this savage state, you are undetectable by advanced empires.", None);
	sentientSayTo(Targoss, av, "That changes when you develop warp drive technology.", None);
	if(secondsElapsed() >= 20) nomads;

:nomads
	sentientSayTo(Targoss, av, "The villagers are nomadic.", None);
	sentientSayTo(Targoss, av, "They will move, slowly, wherever you travel on this world but they do not like cities.", None);
	if(secondsElapsed() >= 20) planExpedition;

:planExpedition
	narratorHint("Double click or press Enter on the continue message. Wait 5 minutes for Targoss to repeat if there is no message.");
	sentientSayTo(Targoss, av, "Now is a great time to explore your home world.", None);
	sentientSayTo(Targoss, av, "Look for a place to build a city, somewhere with ore and oil deposits.", None);
	sentientSayTo(Targoss, av, "Plan an expedition using the world map on the Locator (F7) window.", None);
	sentientSayTo(Targoss, av, "Handcraft things that might be useful along the way.", None);
	sentientSayTo(Targoss, av, "A sailboat is easy to build and it is a fast safe fun way to travel.", None);
	sentientSayTo(Targoss, av, "You can go exploring, take a break, even log out. I will catch up when you continue.", None);
	sentientSayChoice(Targoss, 0, "Continue to the stone age.", None);
	sentientStay(Targoss);
	if(choiceMade() == 0) catchUp;
	if(secondsElapsed() >= 300) planExpedition;

:catchUp
	narratorHint("Move close to Targoss.");
	# Avatar could have moved to the other side of the world by now. Create a new Targoss here.
	Targoss = atomCreateCharacter(av, 45, 2, "Targoss", Male, Right Handed, Empire, Avatar, Mature);
	characterSetIndestructible(Targoss, true);
	sentientCreateGear(Targoss, Wear, false, Jeans and Shirt, 1, Empire, "");
	sentientFollowAvatar(Targoss);
	if(atomDistanceToAtom(av, Targoss) < 8) reachForTheStars;

:reachForTheStars
	sentientSayTo(Targoss, av, "Life as a nomadic tribesman is simple and dangerous.", None);
	sentientSayTo(Targoss, av, "To reach for the stars, you must build a civilization.", None);
	if(secondsElapsed() >= 20) stoneAge;

:stoneAge
	sentientSayTo(Targoss, av, "This is the beginning of the stone age.", None);
	sentientSayTo(Targoss, av, "Permanent structures are needed to advance your civilization.", None);
	sentientSayTo(Targoss, av, "All buildings require materials to build. Stone is a common construction material.", None);
	if(secondsElapsed() >= 20) planQuarry;

:planQuarry
	narratorHint("Construct a mine, using the Quarry by Haxus blueprint.");
	sentientSayImage(Targoss, BuildQuarry.png, "Build a Mine", "Use the Construction (F11) window to build a mine.", None);
	sentientSayTo(Targoss, av, "Click the Manufacture button, fourth from the left in the second row of buttons.", None);
	sentientSayTo(Targoss, av, "Then click the Mine button, top left in the grid of larger buttons.", None);
	sentientSayTo(Targoss, av, "VERY IMPORTANT. Change the blueprint to 'Quarry by Haxus'.", None);
	sentientSayTo(Targoss, av, "Be sure stone is selected as the construction material.", None);
	sentientSayTo(Targoss, av, "Click the terrain to begin construction of the mine. "
			"Arrows and Shift+arrows rotate the mine when choosing its location.", None);
	if(avatarConstructedBuildingHabitable(Mine)) buildQuarry;
	if(secondsElapsed() >= 130) planQuarry;

:buildQuarry
	sentientSayTo(Targoss, av, "The 'Quarry by Haxus' blueprint requires 4 units of stone to construct.", None);
	sentientSayTo(Targoss, av, "Forage 4 units of stone and drop them on the construction site.", None);
	sentientSayTo(Targoss, av, "Then look at the Building (F10) window.", None);
	if(secondsElapsed() >= 90) workConstruction;

:workConstruction
	sentientSayTo(Targoss, av, "The Construction page of the Building (F10) window is divided into two panes.", None);
	sentientSayTo(Targoss, av, "The left pane shows materials that are consumed. It should show 4 stone needed.", None);
	sentientSayTo(Targoss, av, "The right pane shows tools and things that are helpful or needed but not directly consumed.", None);
	sentientSayTo(Targoss, av, "Push the 'Work Construction Site' button to build the mine.", None);
	sentientSayTo(Targoss, av, "Stone is taken from the site by the construction process.", None);
	sentientSayTo(Targoss, av, "The stone could fail, due to its quality. Gather more stone if needed.", None);
	if(secondsElapsed() >= 60) makeStone;

:makeStone
	sentientSayTo(Targoss, av, "When the mine is completed, it can produce and store stone.", None);
	sentientSayTo(Targoss, av, "Use the Manufacture page on the Building (F10) window to mine some stone.", None);
	if(secondsElapsed() >= 80) makeFarm;

:makeFarm
	narratorHint("Construct a farm, using the Pioneer Cabin by Haxus blueprint.");
	sentientSayTo(Targoss, av, "A growing population requires food and shelter.", None);
	sentientSayTo(Targoss, av, "Use the Construction (F11) window to build a farm.", None);
	sentientSayTo(Targoss, av, "Click the Food Production button, second from the left in the second row of buttons.", None);
	sentientSayTo(Targoss, av, "Then click the Farm button, top left in the grid of larger buttons.", None);
	sentientSayTo(Targoss, av, "Choose the 'Pioneer Cabin by Haxus' blueprint.", None);
	sentientSayTo(Targoss, av, "Place the farm somewhere near the mine but do not build it yet.", None);
	if(avatarConstructedBuildingHabitable(Farm)) connections;
	if(secondsElapsed() >= 120) makeFarm;

:connections
	sentientSayTo(Targoss, av, "Buildings interconnect if they are close enough.", None);
	sentientSayTo(Targoss, av, "Go to the farm site and look at the Building page of the Building (F10) window.", None);
	sentientSayTo(Targoss, av, "It should show the farm connected to 1 other building by land.", None);
	sentientSayTo(Targoss, av, "If the farm is far from the mine, they will not connect. "
		"In that case, build a road to connect them.", None);
	if(secondsElapsed() >= 50) buildFarm;

:buildFarm
	sentientSayTo(Targoss, av, "The 'Pioneer Cabin by Haxus' blueprint requires 14 units of stone to construct.", None);
	sentientSayTo(Targoss, av, "When the farm is connected to the mine, the construction process can fetch materials directly.", None);
	sentientSayTo(Targoss, av, "Push the 'Work Construction Site' button to fetch materials and build the farm.", None);
	sentientSayTo(Targoss, av, "The stone could fail, due to its quality. Go to the mine and make more stone if needed.", None);
	sentientSayTo(Targoss, av, "Don't go inside the building. You won't be able to hear what I say.", None);
	if(secondsElapsed() >= 50) homes;

:homes
	narratorHint("Double click or press Enter on the continue message. Wait 5 minutes for Targoss to repeat if there is no message.");
	sentientSayTo(Targoss, av, "Indigenous people will move into the homes provided by the farm.", None);
	sentientSayTo(Targoss, av, "The Building page of the Building (F10) window shows the current number of residents.", None);
	sentientSayTo(Targoss, av, "During the day, residents of the farm will work the manufacturing jobs, "
			"at the mine and at the farm.", None);
	sentientSayTo(Targoss, av, "Given time, they will make a stock of stone and food.", None);
	sentientSayTo(Targoss, av, "Build more buildings to add homes and create jobs.", None);
	sentientSayTo(Targoss, av, "You can take a break, even log out. I will wait here for you to continue.", None);
	sentientSayChoice(Targoss, 0, "Continue to the age of reason.", None);
	sentientStay(Targoss);
	if(choiceMade() == 0) catchUp2;
	if(secondsElapsed() >= 300) homes;

:catchUp2
	narratorHint("Move close to Targoss.");
	sentientFollowAvatar(Targoss);
	if(atomDistanceToAtom(av, Targoss) < 8) ageOfReason;

:ageOfReason
	sentientSayTo(Targoss, av, "The age of reason begins, chaotically.", None);
	sentientSayTo(Targoss, av, "A large number of buildings and people is difficult to manage.", None);
	sentientSayTo(Targoss, av, "To organize them under common leadership, a government must be formed.", None);
	if(secondsElapsed() >= 20) makeFlag;

:makeFlag
	narratorHint("Hand craft a flag.");
	sentientSayTo(Targoss, av, "Governments are identified by their name and symbolized by their flag.", None);
	sentientSayTo(Targoss, av, "Hand craft a flag to form a government. I will wait.", None);
	if(avatarHasItem(Flag)) makeCity;
	if(secondsElapsed() >= 80) makeFlag;

:makeCity
	narratorHint("Build a capitol building.");
	sentientSayTo(Targoss, av, "The seat of government in a city is at the town hall.", None);
	sentientSayTo(Targoss, av, "Build a capitol building, to be the town hall.", None);
	sentientSayTo(Targoss, av, "The dotted circle is its jurisdiction. Buildings in the circle are in the city limits.", None);
	if(avatarConstructedBuildingHabitable(Capitol)) makePolice;
	if(secondsElapsed() >= 80) makeCity;

:makePolice
	narratorHint("Build a police station building.");
	sentientSayTo(Targoss, av, "Governments have little authority without the power to enforce their will.", None);
	sentientSayTo(Targoss, av, "Police are the instruments of government enforcement.", None);
	sentientSayTo(Targoss, av, "Build a police station, to expand jurisdiction over an area. This adds to the city limits.", None);
	if(avatarConstructedBuildingHabitable(Police Station)) cityReports;
	if(secondsElapsed() >= 90) makePolice;

:cityReports
	narratorHint("Double click or press Enter on the continue message. Wait 5 minutes for Targoss to repeat if there is no message.");
	sentientSayTo(Targoss, av, "All buildings created in the city limits become part of the city.", None);
	sentientSayTo(Targoss, av, "Go to the capitol building. Use the Building (F10) window to manage every building in the city.", None);
	sentientSayTo(Targoss, av, "Press the 'Recent Report' button for helpful information about the city as it grows.", None);
	sentientSayTo(Targoss, av, "You can take a break, even log out. I will wait here for you to continue.", None);
	sentientSayChoice(Targoss, 0, "Continue to claims and conquest.", None);
	sentientStay(Targoss);
	if(choiceMade() == 0) catchUp3;
	if(secondsElapsed() >= 300) cityReports;

:catchUp3
	narratorHint("Move close to Targoss.");
	sentientFollowAvatar(Targoss);
	if(atomDistanceToAtom(av, Targoss) < 8) claimWorld;

:claimWorld
	narratorHint("Claim the world.");
	sentientSayTo(Targoss, av, "You created an empire and built a city but you have not claimed the world.", None);
	sentientSayTo(Targoss, av, "To lay claim to this world, select a flag as your current item. "
			"Then click on the ground where you want the flag pole to appear.", None);
	w = atomWorld(av);
	if(!atomValid(w)) goodBye;	# Exit with zero result code
	if(worldClaimedBy(w, Empire)) worldClaimed;
	if(secondsElapsed() >= 90) claimWorld;

:worldClaimed
	narratorHint("Build a military headquarters building.");
	sentientSayTo(Targoss, av, "Your claim can be stolen easily by someone from another empire, "
			"simply by claiming the world for themselves.", None);
	sentientSayTo(Targoss, av, "To defend your claim, build a military base. Start with a military headquarters building.", None);
	if(avatarConstructedBuildingHabitable(Military Headquarters)) baseEst;
	if(secondsElapsed() >= 90) worldClaimed;

:baseEst
	sentientSayTo(Targoss, av, "The military base defends the entire world.", None);
	sentientSayTo(Targoss, av, "All military buildings on the planet join together as one base. Police stations are not needed.", None);
	sentientSayTo(Targoss, av, "The military HQ manages the base, like the town hall of a city.", None);
	if(secondsElapsed() >= 30) conquerWorld;

:conquerWorld
	sentientSayTo(Targoss, av, "The HQ can be damaged but it cannot be destroyed while any other military buildings exist.", None);
	sentientSayTo(Targoss, av, "Follow these steps to conquer a world for your empire.", None);
	sentientSayTo(Targoss, av, "1. Destroy all military buildings except the HQ.", None);
	sentientSayTo(Targoss, av, "2. Destroy the HQ.", None);
	sentientSayTo(Targoss, av, "3. Claim the world for your empire.", None);
	if(secondsElapsed() >= 50) damageBuildings;

:damageBuildings
	sentientSayTo(Targoss, av, "Cities change allegiance to the empire that claims the world.", None);
	sentientSayTo(Targoss, av, "Buildings can be damaged. This slows their production and reduces morale.", None);
	sentientSayTo(Targoss, av, "However, buildings cannot be completely destroyed while a military HQ is present on their world.", None);
	if(secondsElapsed() >= 30) destroyCity;

:destroyCity
	sentientSayTo(Targoss, av, "The capitol building can be damaged but it cannot be destroyed while any police stations exist.", None);
	sentientSayTo(Targoss, av, "Follow these steps to utterly destroy a city, after the military is destroyed.", None);
	sentientSayTo(Targoss, av, "1. Destroy all police buildings.", None);
	sentientSayTo(Targoss, av, "2. Destroy the capitol.", None);
	sentientSayTo(Targoss, av, "3. Destroy the buildings.", None);
	if(secondsElapsed() >= 30) destroyWisely;

:destroyWisely
	narratorHint("Double click or press Enter on the continue message. Wait 5 minutes for Targoss to repeat if there is no message.");
	sentientSayTo(Targoss, av, "Consider what Sun Tzu said, 'In the practical art of war, the best thing of all is to take the enemy's country whole and intact; to shatter and destroy it is not so good.'", None);
	sentientSayTo(Targoss, av, "You can take a break, even log out. I will wait here for you to continue.", None);
	sentientSayChoice(Targoss, 0, "Continue to the industrial age.", None);
	sentientStay(Targoss);
	if(choiceMade() == 0) catchUp4;
	if(secondsElapsed() >= 300) destroyWisely;

:catchUp4
	narratorHint("Move close to Targoss.");
	sentientFollowAvatar(Targoss);
	if(atomDistanceToAtom(av, Targoss) < 8) industrialAge;

:industrialAge
	narratorHint("Build a power plant to generate electricity.");
	sentientSayTo(Targoss, av, "The industrial age begins.", None);
	sentientSayTo(Targoss, av, "1. Build a carpenter to make hammers from stone.", None);
	sentientSayTo(Targoss, av, "2. Find ore; build a mine there.", None);
	sentientSayTo(Targoss, av, "3. Build a smelter to make metal from ore.", None);
	sentientSayTo(Targoss, av, "4. Build a bank to mint money, using metal.", None);
	sentientSayTo(Targoss, av, "5. Build a university to research technology, which costs money.", None);
	sentientSayTo(Targoss, av, "6. Build a power plant to generate electricity. 'Coal Power Plant by Haxus' is a good start.", None);
	sentientSayTo(Targoss, av, "When your city has electrical power, I will continue.", None);
	city = atomCity(av);
	if(cityHasPower(city)) technologyAge;

:technologyAge
	sentientSayTo(Targoss, av, "Congratulations! Your civilization has reached the technology age.", None);
	sentientSayTo(Targoss, av, "With the help of artificial light, citizens work day and night.", None);
	sentientSayTo(Targoss, av, "Crops grow at night, using artificially lights.", None);
	sentientSayTo(Targoss, av, "The pace of construction and manufacture accelerates.", None);
	if(secondsElapsed() >= 30) beyondTheSky;

:beyondTheSky
	sentientSayTo(Targoss, av, "Technology advances quickly. Quality of life improves for everyone.", None);
	sentientSayTo(Targoss, av, "People turn their attention upward to the stars.", None);
	sentientSayTo(Targoss, av, "An age old question begs for an answer. What lies beyond the sky?", None);
	if(secondsElapsed() >= 30) spaceAge;

:spaceAge
	sentientSayTo(Targoss, av, "Technology leads to the space age.", None);
	sentientSayTo(Targoss, av, "1. Build an armorer to make environment suits, for protection against the harsh vacuum of space.", None);
	sentientSayTo(Targoss, av, "2. Build a refinery to make hydrogen, for rocket fuel.", None);
	sentientSayTo(Targoss, av, "3. Build an aircraft factory to make space rockets.", None);
	sentientSayTo(Targoss, av, "When your city has those things, rocket training will start.", None);
	if(secondsElapsed() >= 50) closing;

:closing
	sentientSayTo(Targoss, av, "Here are a few closing tips.", None);
	sentientSayTo(Targoss, av, "The Governance (F12) window shows a list of all cities. "
			"Right click a city for options.", None);
	sentientSayTo(Targoss, av, "Hail on the Galactic or Help channel for help. Other players are generally helpful.", None);
	sentientSayTo(Targoss, av, "Use the Bug Reports thread on the forum to report problems.", None);
	if(secondsElapsed() >= 40) goodBye2;

:goodBye2
	# Story completed successfully.
	avatarStoryResult(2);
	goto goodBye;

