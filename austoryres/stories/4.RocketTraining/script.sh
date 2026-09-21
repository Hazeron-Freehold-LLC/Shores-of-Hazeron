# The block with the empty segment label ':' is the catalyst for starting every story. It is required.
:
	# Basic story info. storyTitle, storyAuthor, and storyBrief are required.
	storyTitle("Rocket Training");
	storyAuthor("Haxus");
	storyBrief("Learn to fly a rocket from an expert.");

	# This story is allowed to start automatically repeatedly until the avatar completes it.
	# The avatar can drop this story over and over until it is finished.
	storyAutoRepeatUntilComplete();
	storyInterval(3);	# Minutes before this script can start again, after starting once.

	# At least one 'if' is required; when any of them returns true, the story can start.
	P = avatar();
	city = atomCity(P);
	rocket = cityFindNewVehicle(city, Space Rocket);	# Find a new space rocket at an aircraft factory.
	if(avatarInitScenario(Reach for the Stars) &
			atomValid(rocket) &
			cityHasItem(city, Hydrogen) &
			cityHasItem(city, EV Suit) &
			cityHasItem(city, EV Helmet)) begin;

:begin
	narratorTitle("Rocket Training");
	goto report;

:report
	Гага́рин = atomCreateCharacter(rocket, 90, 4, "Гага́рин", Male, Right Handed, Empire, Worker, Mature);
	sentientStay(Гага́рин);
	characterEquipFromCity(Гага́рин, Wear, Jeans and Shirt, "Ю́рий Алексе́евич Гага́рин");
	characterEquipFromCity(Гага́рин, Wear, Leather Clothing, "Ю́рий Алексе́евич Гага́рин");
	characterEquipFromCity(Гага́рин, Wear, EV Suit, "Vostok 3KA-3 - Гага́рин");
	characterEquipFromCity(Гага́рин, Head, EV Helmet, "Vostok 3KA-3 - Кедр");
	narratorSay("[P.Name], report to the aircraft factory at once for rocket training.");
	narratorSay("Гага́рин is waiting for you near the rocket at [Гага́рин.GlobeLL].");
	narratorHint("Go to Гага́рин. He is standing by the rocket at the aircraft factory, wearing an environment suit.");
	if(atomDistanceToAtom(P, Гага́рин) < 5) welcome;

:welcome
	sentientSay(Гага́рин, "You are late cadet. Come here!", Smile);
	if(wait(10)) intro;

:intro
	sentientSay(Гага́рин, "I am Гага́рин. That is Gagarin if you do not speak Russian.", Smile);
	sentientSay(Гага́рин, "I teach the basic rocketry course.", Smile);
	sentientSay(Гага́рин, "Are you ready to learn?", Smile);
	narratorHint("Stand close to Гага́рин and pay attention.");
	sentientSayChoice(Гага́рин, 0, "Yes sir. I am ready.", Smile);
	sentientSayChoice(Гага́рин, 1, "No sir. I already know how to fly a space rocket.", Smile);
	if(choiceMade() == 0) welcome2;
	if(choiceMade() == 1) trainingSkipped;
	if(wait(30)) intro;

:welcome2
	sentientSay(Гага́рин, "Do you see that rocket there? Do not touch it. Do only as I say!", Smile);
	narratorHint("Do only what Гага́рин tells you to do. Do not get into the rocket yet.");
	if(wait(10)) appreciate;

:appreciate
	sentientSay(Гага́рин, "Beautiful isn't it?", Smile);
	sentientSay(Гага́рин, "Powerful, streamlined, no vehicle withstands atmospheric stresses like a rocket.", Smile);
	sentientSay(Гага́рин, "I was first person to orbit the world in a rocket, a Vostok.", Smile);
	sentientSay(Гага́рин, "Listen to what I say and you may live.", Laugh1Sec);
	narratorHint("Appreciate the rocket. Trust Гага́рин.");
	if(wait(20) & !avatarHadItem(EV Suit)) equipSuit;
	if(wait(20) & !avatarHadItem(EV Helmet)) equipHelmet;
	if(wait(20)) missionBrief;

:equipSuit
	sentientSay(Гага́рин, "You are poorly equipped to become cosmonaut.", Smile);
	sentientSay(Гага́рин, "Here! Take my environment suit. I can no longer use it.", Smile);
	sentientSay(Гага́рин, "The suit has a lamp. Turn it on and off by using it.", Smile);
	sentientDropGearItem(Гага́рин, EV Suit);
	narratorHint("Pick up the environment suit.");
	if(wait(20) & !avatarHadItem(EV Helmet) & avatarHasItem(EV Suit)) equipHelmet;
	if(wait(20) & avatarHasItem(EV Suit)) missionBrief;

:equipHelmet
	sentientSay(Гага́рин, "You do not have a helmet. You cannot go into space without a helmet.", Smile);
	sentientSay(Гага́рин, "I wore this helmet on Vostok 1 mission. Take it!", Smile);
	sentientSay(Гага́рин, "Use it, to open and close the visor. Visor must be closed for protection in space.", Smile);
	sentientSay(Гага́рин, "Your body fatigues faster and recovers slower when visor is closed.", Smile);
	narratorHint("Pick up the environment helmet.");
	sentientDropGearItem(Гага́рин, EV Helmet);
	if(wait(25) & avatarHasItem(EV Helmet)) missionBrief;

:missionBrief
	sentientSay(Гага́рин, "Training mission is simple; a monkey could do it.", Laugh1Sec);
	sentientSay(Гага́рин, "Do not go beyond what is expected.", Smile);
	sentientSay(Гага́рин, "You will fly rocket above atmosphere of this planet.", Smile);
	sentientSay(Гага́рин, "There you will exit the rocket to gain EVA experience in environment suit.", Smile);
	sentientSay(Гага́рин, "Then you must reenter rocket and return to the city safely.", Smile);
	sentientSay(Гага́рин, "You will be paid 500¢ hazard pay if you accomplish the mission.", Smile);
	narratorHint("Read the mission briefing.");
	if(wait(35)) suitUp;

:suitUp
	sentientSay(Гага́рин, "Suit up!", Smile);
	sentientSay(Гага́рин, "Check your personal equipment.", Smile);
	sentientSay(Гага́рин, "Be sure you are wearing the environment suit properly.", Smile);
	sentientSay(Гага́рин, "Visor can be open; cockpit of the rocket is pressurized.", Smile);
	narratorHint("Wear an EV suit and helmet.");
	if(wait(25)) changeChannel;

:changeChannel
	sentientSay(Гага́рин, "You will not be able to hear my voice when you get inside the rocket.", Smile);
	sentientSay(Гага́рин, "All further rocket training communications will be on the Hail channel.", Smile);
	sentientSay(Гага́рин, "Switching to Hail channel now.", Smile);
	if(wait(25)) getIn;

:getIn
	atomComm(Гага́рин, Hail, System, "I am on Hail channel now. Get into the rocket.");
	atomComm(Гага́рин, Hail, System, "To enter rocket, look at it and press the E key.");
	atomComm(Гага́рин, Hail, System, "E key exits rocket once you are aboard.");
	narratorHint("Wait for it...");
	if(avatarInVehicle(Space Rocket)) checkFuel;
	if(wait(20)) getIn;

:checkFuel
	atomComm(Гага́рин, Hail, System, "Do not press any button until I tell you what it does!");
	atomComm(Гага́рин, Hail, System, "Bottom corner of your screen shows fuel level as green bar with fuel pump on it.");
	atomComm(Гага́рин, Hail, System, "Fully fueled rocket can operate for 20 minutes.");
	atomComm(Гага́рин, Hail, System, "That is plenty of time to complete training mission.");
	atomComm(Гага́рин, Hail, System, "Here are ways to get fuel.");
	atomComm(Гага́рин, Hail, System, "- Press K to request fuel and repairs while rocket is sitting at an air terminal or in trade "
			"zone of space station.");
	atomComm(Гага́рин, Hail, System, "- Press K to refuel rocket using Hydrogen from your inventory when you are aboard.");
	atomComm(Гага́рин, Hail, System, "- Select Hydrogen as current item and click on the rocket to refuel from outside.");
	narratorHint("Get into the rocket. Press K to refuel if needed.");
	if(wait(55) & avatarInVehicle(Space Rocket)) hudOn;

:hudOn
	atomComm(Гага́рин, Hail, System, "Turn on rocket lights with L key.");
	atomComm(Гага́рин, Hail, System, "This illuminates HUD instrumentation in front of you.");
	atomComm(Гага́рин, Hail, System, "Press , and . buttons to adjust HUD intensity.");
	atomComm(Гага́рин, Hail, System, "Central to HUD is ladder indicating pitch and roll, currently at maximum pitch up.");
	atomComm(Гага́рин, Hail, System, "Number at left is velocity in meters per second.");
	atomComm(Гага́рин, Hail, System, "Number at right is altitude above sea level in meters.");
	atomComm(Гага́рин, Hail, System, "Bar graph at top left is hull stress. Keep the indicator within solid part of bar.");
	atomComm(Гага́рин, Hail, System, "Bar graph at bottom right is main booster throttle setting.");
	atomComm(Гага́рин, Hail, System, "Curved horizontal tape across bottom is heading.");
	atomComm(Гага́рин, Hail, System, "Numbers below heading show latitude and longitude.");
	narratorHint("Get into the rocket. Review the HUD lesson.");
	if(wait(55) & avatarInVehicle(Space Rocket)) controls;

:controls
	atomComm(Гага́рин, Hail, System, "PageUp and PageDown adjust main booster throttle. Delete neutralizes throttle.");
	atomComm(Гага́рин, Hail, System, "Once airborne, W A S D Z and C buttons turn, pitch, and roll the rocket.");
	atomComm(Гага́рин, Hail, System, "Arrow buttons apply lateral thrust with retrorockets.");
	atomComm(Гага́рин, Hail, System, "Very important! Space bar applies brakes on longitudinal axis of rocket.");
	atomComm(Гага́рин, Hail, System, "Q applies brakes laterally and vertically.");
	atomComm(Гага́рин, Hail, System, "Space bar and Q together apply brakes on all axes.");
	atomComm(Гага́рин, Hail, System, "H holds brakes on all axes to hold position.");
	narratorHint("Get into the rocket. Review the controls lesson.");
	if(wait(40) & avatarInVehicle(Space Rocket)) prepareBlastOff;

:prepareBlastOff
	atomComm(Гага́рин, Hail, System, "Ground school is complete. Prepare to blast off!");
	if(wait(5) & avatarInVehicle(Space Rocket)) checkList;

:checkList
	atomComm(Гага́рин, Hail, System, "Write down longitude and latitude. This is where you will return.");
	atomComm(Гага́рин, Hail, System, "Things happen quickly on blast off.");
	atomComm(Гага́рин, Hail, System, "Scroll Hail channel to the end now, so window autoscrolls as new messages arrive.");
	narratorHint("Get into the rocket.");
	if(wait(25) & avatarInVehicle(Space Rocket)) blastOff;

:blastOff
	atomComm(Гага́рин, Hail, System, "Increase throttle to maximum; Shift+PageUp does this.");
	atomComm(Гага́рин, Hail, System, "Count down 3 2 1 to allow fuel time to vaporize in combustion chamber.");
	atomComm(Гага́рин, Hail, System, "Поехали! Let's go! Press P to ignite rocket main booster.");
	narratorHint("Get into the rocket. Press Shift+PgUp then press P. Blast off!");
	if(avatarInVehicle(Space Rocket) & avatarInOrbit()) inOrbit;

:inOrbit
	atomComm(Гага́рин, Hail, System, "Stop the rocket immediately!");
	atomComm(Гага́рин, Hail, System, "Hold Q and Spacebar together until the rocket stops.");
	narratorHint("Get into the rocket. Stop the rocket by holding Space and Q together.");
	if(avatarInVehicle(Space Rocket) & !avatarInVehicleMoving(Space Rocket)) park;

:park
	atomComm(Гага́рин, Hail, System, "The rocket has stopped. Congratulation! You have achieved orbit.");
	atomComm(Гага́рин, Hail, System, "Neutralize the throttle with Delete then press H to hold the brakes. "
			"Do NOT turn off main booster.");
	atomComm(Гага́рин, Hail, System, "Release Q and Space. Rocket will hold position in space as long as main booster is on.");
	narratorHint("Use H to hold the rocket's position with brakes.");
	if(wait(20) & avatarInVehicle(Space Rocket) & !avatarInVehicleMoving(Space Rocket)) rotate;

:rotate
	atomComm(Гага́рин, Hail, System, "Rotate the rocket using W A S D Z and C keys.");
	atomComm(Гага́рин, Hail, System, "Get a look at your world from space.");
	if(wait(20) & avatarInVehicle(Space Rocket) & !avatarInVehicleMoving(Space Rocket)) eva;

:eva
	atomComm(Гага́рин, Hail, System, "Prepare for EVA.");
	atomComm(Гага́рин, Hail, System, "Insure helmet visor is closed.");
	atomComm(Гага́рин, Hail, System, "Press E to exit the rocket.");
	narratorHint("Get out of the rocket.");
	if(!avatarInVehicle(Space Rocket)) eva2;

:eva2
	atomComm(Гага́рин, Hail, System, "EV suit is equipped with maneuvering jets.");
	atomComm(Гага́рин, Hail, System, "Do not move quickly; it is easy to lose control.");
	atomComm(Гага́рин, Hail, System, "Move using brief taps on the W A S D Z and C buttons to feel the effect of weightlessness.");
	atomComm(Гага́рин, Hail, System, "Space bar applies brakes.");
	atomComm(Гага́рин, Hail, System, "Practice EVA for one minute. I will tell you when the lesson is complete.");
	narratorHint("Practice zero-G movement.");
	if(avatarEVAMinutes() >= 1) getBackIn;

:getBackIn
	atomComm(Гага́рин, Hail, System, "EVA lesson is over.");
	atomComm(Гага́рин, Hail, System, "Get back into the rocket immediately; mission is not yet complete.");
	atomComm(Гага́рин, Hail, System, "Hardest part of mission lies ahead, returning to base.");
	narratorHint("Get into the rocket.");
	if(avatarInVehicle(Space Rocket)) navOrbit;

:navOrbit
	atomComm(Гага́рин, Hail, System, "Press H to release the brakes.");
	atomComm(Гага́рин, Hail, System, "Fly rocket around the planet to landing coordinates.");
	atomComm(Гага́рин, Hail, System, "Land anywhere in the city to complete mission successfully.");
	atomComm(Гага́рин, Hail, System, "Reentry must be slower than 700m/s or the rocket will disintegrate.");
	atomComm(Гага́рин, Hail, System, "Number at left center of HUD is overall speed in m/s.");
	atomComm(Гага́рин, Hail, System, "Small +/- number below overall speed is lateral speed.");
	atomComm(Гага́рин, Hail, System, "Number at right center of HUD is altitude above mean sea level (MSL) in meters.");
	atomComm(Гага́рин, Hail, System, "Small +/- number below altitude is vertical speed.");
	atomComm(Гага́рин, Hail, System, "Use Q to reduce lateral and vertical speeds.");
	narratorHint("Reenter the atmosphere in the rocket.");
	if(!avatarInOrbit()) navLand;

:navLand
	atomComm(Гага́рин, Hail, System, "We are tracking the rocket inside atmosphere. ♪ Родина слышит, Родина знает ♫");
	atomComm(Гага́рин, Hail, System, "To land, stop the rocket a short distance above ground.");
	atomComm(Гага́рин, Hail, System, "Rotate rocket to point skyward and lower the rocket to settle on tail fins.");
	atomComm(Гага́рин, Hail, System, "Extinguish the main booster by pressing P.");
	atomComm(Гага́рин, Hail, System, "Bring it home, cosmonaut!");
	narratorHint("Land the rocket in a city.");
	if(avatarInVehicleOnGround(Space Rocket) & avatarInCity()) missionComplete;
	if(avatarInVehicleOnGround(Space Rocket) & !avatarInCity()) wrongLandingSite;

:wrongLandingSite
	atomComm(Гага́рин, Hail, System, "The rocket has landed outside of city.");
	atomComm(Гага́рин, Hail, System, "Take off and land somewhere in the city to complete mission.");
	narratorHint("Get into the rocket. Land the rocket in a city.");
	if(avatarInVehicleOnGround(Space Rocket) & avatarInCity()) missionComplete;

:missionComplete
	atomComm(Гага́рин, Hail, System, "Mission complete!");
	atomComm(Гага́рин, Hail, System, "You were credited 500¢ hazard pay; redeem this credit at any bank.");
	atomComm(Гага́рин, Hail, System, "Please. Keep environment suit. I am honored for you to wear it.");
	atomComm(Гага́рин, Hail, System, "Good work, comrade!");
	avatarCreditGovtDebt(500);
	if(wait(20)) aboutGagarin;

:aboutGagarin
	narratorImage(Yuri_Gagarin_with_awards.jpg, "Yuri Gagarin", "To learn about the real life of cosmonaut Yuri Gagarin, "
			"open https://www.space.com/16159-first-man-in-space.html in your browser. "
			"To do that, you can right-click this message in the comm channel.");
	if(wait(10)) quietExit;

:trainingSkipped
	atomComm(Гага́рин, Hail, System, "Good luck, rocket jockey. You're going to need it.");
	if(wait(10)) quietExit;

:quietExit
