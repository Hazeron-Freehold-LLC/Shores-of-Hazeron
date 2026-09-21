#nocompile
# Script for the new player tutorial.

# The block with the empty segment label ':' is the catalyst for starting every
# story. The 'title', 'author', and 'brief' must be declared in the catalyst
# section. At least one 'if' is required for evaluation of the catalyst. When
# any of the catalyst 'if's returns true, the story can appear in the list of
# stories that are ready to join.
# The catalyst section may optionally contain an interval declaration. This
# specifies the number of minutes that must pass before the story can be
# started again, after the previous time it was started.
:
	storyTitle("Veil of Targoss");
	storyAuthor("Haxus");
	storyBrief("Tutorial for starting a new empire with only a flag.");
	# interval(10);
	#if(avatarHasItem(flag) & !system.named() & planet.breathable() & planet.flora() & planet.fauna()) basics;
	if(avatarXp() < 0) basics;	#DHH This will never be true. This script is disabled.

:basics
	narratorTitle("Basic Controls");
	narratorSay("Move the mouse to turn left and right and to look up and down.\nHold the Ctrl key to free the mouse to control your hand instead of your body.\nW walks forward; S walks backward. Shift-W runs forward.\nA and D sidestep left and right.\nMouse Wheel selects your current item.\nLeft Mouse Button uses your current item on a target.\nMiddle Mouse Button or Wheel Click uses your current item on yourself.\nThe backspace key switches between first-person and top-down views.\nF4 displays your personal inventory.\nF1 displays detailed documentation.");
	narratorSayChoice(0, "Press F3 to display a transcript of these messages. Select this message then double-click or press Enter to continue.");
	if(choiceMade() == 0) introduction;

:introduction
	narratorTitle("Day Break");
	narratorSay("Last night seems distant, surreal, but the ache in my muscles tells me it really happened. Why couldn't it have been some awful nightmare?");
	narratorSay("My former life is gone, crushed to ruins. Friends, neighbors, home, all are gone, replaced by the solitary moan of the wind.");
	narratorSay("Last night was a torture test of my endurance. Uncounted miles trail in the wake of my blind flight through the wilderness, running, walking, running until my lungs screamed. Twice, unseen animals of the night picked up my trail, driving me onward. Delirious and scratched by briars, I stumbled headlong through the darkness, fixated on my goal.");
	narratorSay("I collapsed from utter exhaustion just before dawn, awakening here in these unfamiliar surroundings. A few tormented hours of sleep were not enough but I must keep moving. I must get to the sea.");
	narratorSayChoice(0, "Press F3 to display a transcript of these messages. Select this message then double-click or press Enter to continue.");
	if(choiceMade() == 0) begin;

:begin
	narratorTitle("Arrival");
	narratorSay("About a week ago, mysterious travelers arrived at the outskirts of town. Over night an enclave of tents sprang up, apparently covering a large structure beneath. No one saw them arrive; they were just there in the morning.");
	narratorSay("A few curious folks went out to greet them, only to be turned away well short of the tents. People soon became nervous and rumours spread about strange sights and stranger noises.");
	narratorSay("Everyone wanted to know what was going on. People kept their kids close to home and they started locking their doors at night.");
	narratorSay("I got worried when Constable Ricci cordoned off the dirt road going out that side of town.");
	narratorSayChoice(0, "Press F3 to display a transcript of these messages. Select this message then double-click or press Enter to continue.");
	if(choiceMade() == 0) diplomacy;

:diplomacy
	narratorTitle("Diplomacy");
	narratorSay("The town council held a special session to demand answers. Who were these strange travelers? Where did they come from? How did they get here? They have no live stock; how did they transport the tents and people and boxes? Did they build something without a permit?");
	narratorSay("A confrontation was brewing. My neighbor Don Stucker is on the town council; he kept me informed all along. It was pretty exciting with minute-by-minute updates. Nothing like this had ever happened in our quiet little town before.");
	narratorSay("The council appointed three of its members to greet the unwanted visitors; Don was one of them. I don't think he was all that eager but Don was always one of the first to step forward to do the tough jobs.");
	narratorSay("The delegation decided to find out the motives of the strangers and then to try pursuading them to move on to another town.");
	narratorSayChoice(0, "Press F3 to display a transcript of these messages. Select this message then double-click or press Enter to continue.");
	if(choiceMade() == 0) confrontation;

:confrontation
	narratorTitle("Confrontation");
	narratorSay("By now the whole town knew what was happening. Plenty of people turned out to watch. I found a good vantage point in a tree where I could see everything.");
	narratorSay("The delegation of Councilors, clad in the most formal garments of their offices, marched side-by-side to the entrance of the enclave. Constable Ricci accompanied them in case there was any trouble. I waved at Don as he passed and he grinned nervously at me. As a veteran he got the honor of standard-bearer for the delegation so he couldn't wave back. At least the guidon gave him something to hang on to, which helped to control the trembling of his hands.");
	narratorSay("Some of the older kids from the neighborhood followed a short distance back on bicycles, weaving back and forth in the dust. Constable Ricci bristled and made a sour face at them. Ned Nagle almost got him to break ranks when he rode up close to his heels. Ned was no stranger to the constable and was enjoying this chance to get under his skin.");
	narratorSay("The councilors approached the entrance. An awful howl rose up from among the tents, striking fear in the hearts of everyone who could hear. People cried out; some ran in a panic. The kids on their bicycles turned and fled at top speed, hunkered down and leaning forward, legs pumping.");
	narratorSay("The chorus of howls and cries grew louder and more insistent from within the enclave. The screaming voices joined together in a heart pounding chant that could only be described as a war cry. Every sane cell of my body demanded to run now but no muscle would obey. In stark frozen terror I saw it all happen.");
	narratorSay("With a flurry of violence and steel and roar of noise, scores of figures poured out of the tents. Riding all kinds of animals and machines, the invaders destroyed every building in the town with fire and lightning and explosions. Their cries sounded eerily gleeful as they slaughtered every person in their path without mercy.");
	narratorSayChoice(0, "Press F3 to display a transcript of these messages. Select this message then double-click or press Enter to continue.");
	if(choiceMade() == 0) escape;

:escape
	narratorTitle("Escape");
	narratorSay("As the attack wave poured out of the enclave, the councilmen looked at each other bewildered and turned to run. They were lost from sight as the wave of attackers poured over them, hacking, blasting, screaming insanely. Their mangled bodies lie broken in the street after the wave passed.");
	narratorSay("Unseen, I slipped from my perch and rushed to their fallen bodies. The sight was awful; there was no way I could save them.");
	narratorSay("My heart pounded and blood rushed in my ears with a roar, drowning out the distant screams and explosions and horrid sounds of death consuming my town. My senses felt detached, like I was only an observer looking in from the outside, like I wasn't really there.");
	narratorSay("I knelt down to hold Don's gasping body. He was nearly hewn in two and was having a hard time breathing. In his hand was the deadly weapon he pulled from his own mortal wound. Don smiled calmly as he handed it to me and said, 'Targoss was right. We must get off this island.' With barely a shudder, the light left his eyes. Peace overcame his face and Don was gone; he looked comfortable, happy.");
	narratorSay("No! I screamed as I took the knife from Don's hand. Tears welled up in my eyes, blurring my vision. I looked at the knife as though it were alive and might leap forward at any moment to steal my life as well. My mind was spinning. The legend of Targoss is just that, a legend, right?");
	narratorSay("Confused, dazed, I picked up the trampled flag of my people from the dirt road, where it had fallen next to Don's lifeless body. My shaking hands carefully wrapped the Dagger of Targoss in the blood-streaked flag. Then I ran, blindly; with every ounce of energy in me I ran into the night. I knew I needed to get to the coast; destiny would accept no other action.");
	narratorSayChoice(0, "Press F3 to display a transcript of these messages. Select this message then double-click or press Enter to continue.");
	if(choiceMade() == 0) legends;

:legends
	narratorTitle("Legends");
	narratorSay("My people tell tales around the cooking fires at night; this is our heritage.");
	narratorSay("Every child learns the story of Targoss and Falla, how they fell from the sky to fulfill their destiny on this planet. This popular tale of heroism and romance in ancient times ignites the imagination of every new generation.");
	narratorSay("The legend tells a rambling story of Targoss, a lonely traveler lured to live among the stars by Umans. These powerful beings create Falla, the perfect mate for Targoss, and the two fall passionately in love.");
	narratorSay("The two young lovers are transported across the heavens in a fantastic vessel, leaving trails of light in its wake. They survive horrific monsters and face evil villains in the harrowing adventures of their travels. Encounters with Umans alternate between good and evil; at times the Umans are their saviors, other times cruel demons.");
	narratorSay("During a particularly gruesome encounter, Targoss acquires a fabulous dagger forged of green metal unlike any on this world. Legends attribute mystical properties to Targoss' dagger, including incredible deadliness and endurance. In many of the stories of Targoss' adventures, the dagger is close at his side.");
	narratorSay("The odyssey ends peculiarly when their vessel 'emerges from the sun' and ignites into flames, joining the sun in the sky. The young lovers fall safely to the ground, naked, without any possessions. According to legend, Targoss looked upon the new land and said, 'We must get off this island.'");
	narratorSay("Facing their greatest trial, Targoss and Falla embrace the world together. The planet accepts them and their children multiply to populate the entire world.");
	narratorSay("Legends say the the cycle will be renewed when the dagger returns to an heir of Targoss and Falla, who will leave this island to claim a kingdom on the Shores of Hazeron.");
	narratorSayChoice(0, "Press F3 to display a transcript of these messages. Select this message then double-click or press Enter to continue.");
	if(choiceMade() == 0) getoff;

:getoff
	narratorTitle("Get Off This Island");
	narratorSay("As Coucilor Stucker lie dieing in my arms, he repeated the words of Targoss, 'We must get off this island!' Alone in the wilderness I feel little choice but to follow his advice. It is time to get moving, to go to the coast as quickly as I can.");
	narratorSay("The survivors of the town need food and shelter. Their help will be invaluable to get off this island. A new town will give the survivors hope and a new beginning.");
	narratorSay("Since I only have one flag, I must select a location for the new town wisely. A copse of trees near the coast will provide a source of wood for building boats. I must sail to the Shores of Hazeron; it is my destiny.");
	narratorSay("Locate a tree on land near the coast; be sure all four corners of the terrain tile are above water.");
	narratorSay("Select the flag as your current item when you are ready to build a town.");
	if(city.hasBuilding(town square,1)) construction;
	if(avatarCurrentItem(flag)) flaginhand;

:flaginhand
	narratorTitle("Old Glory");
	narratorSay("A flag is all that is needed to begin building a new town. It is mostly a symbolic gesture, steeped heavily in tradition and ceremony. Our people have repeated this practice for generations.");
	narratorSay("A town starts by raising a flag in the place that will become the new town square. To raise the flag held in your hand, press F10 to open the Construction toolbar and select Town Square. While holding the Ctrl key, a green outline appears around the terrain tile under the mouse cursor. Click on the terrain to raise the flag and build the town square.");
	narratorSay("Do not build the town square on the only tree in the area.");
	narratorSay("Use the backspace key to switch between plan view and first-person view. Plan view is helpful when planning roads and buildings. Use the backspace key often for a birds-eye-view of your town.");
	narratorSay("Build a Town Square and stand near the flagpole.");
	P = avatar();
	if(city.empire() & unitAtBuilding(P, town square)) flagraised;

:flagraised
	narratorTitle("Ascension");
	narratorSay("I carefully unwrapped the Dagger of Targoss from the flag. The green metal glinted as it caught the light. Using the knife, I felled a sturdy sapling to use as a pole. The knife's exceptionally keen edge made short work of the sapling.");
	narratorSay("I bowed my head before the pole and recited a few words of blessing. It seemed like the right thing to do before raising the flag for the first time.");
	narratorSay("A group of people gathered a short distance away. Huddled together they began murmuring about what they had just witnessed. One of them fell to his knees and groveled; the others quickly followed suit.");
	narratorSay("Someone cried out joyfully, 'The heir to the throne of Targoss and Falla has returned! The wielder of the Dagger is our one true leader!'");
	narratorSay("Another voice chimed in, 'Hail to the king of the world!'");
	narratorSayChoice(0, "Press F3 to display a transcript of these messages. Select this message then double-click or press Enter to continue.");
	if(choiceMade() == 0) construction;

:construction
	narratorTitle("Construction");
	narratorSay("Construction basics:\n  a) A town square is built on land that is not next to any road or building.\n  b) Roads are built next to roads or a town square.\n  c) Buildings are built next to roads or a town square. The Construction toolbar shows specific terrain requirements for each type of building.");
	narratorSay("Materials are needed to construct buildings. Logs make a good building material; they are produced by logging camps. A logging camp may be built on land where trees are growing naturally.");
	narratorSay("Search the area for a tree. Trees are distinguished from other plants by their thick woody trunk. Size does not matter.");
	narratorSay("If the tree you find is not next to the town square, you must build a dirt road from the town square to the terrain next to the tree.");
	narratorSay("To build a dirt road, press the Dirt Road button on the Construction toolbar. While holding the Ctrl key, the arrow keys will toggle road directions on and off. When all direction arrows are off, a level road slab is built. Slabs can take a lot of labor to build, depending upon the amount of terrain leveling required.");
	narratorSay("Road sections appear with a construction sign at the center. Approach the sign and look at it. While holding the Ctrl key, click Work Hard to complete the road.");
	narratorSay("Build as many dirt road sections as needed to reach the terrain tile next to the tree.");
	narratorSay("Select Logging Camp from the Construction toolbar. Hold the Ctrl key and click on the terrain where the tree is growing. A construction sign appears; click Work Hard to complete the logging camp.");
	narratorSay("Enter the building when the logging camp is completed.");
	if(city.empire() & city.hasBuilding(logging camp,1)) campbuilt;

:campbuilt
	narratorTitle("Refugees");
	narratorSay("News quickly spread across the land. The survivors of your people are overjoyed at hearing of the return of the king. Build homes for them and they will come to you from the wilderness.");
	narratorSay("The refugees will be hungry when they arrive; they will need food. These poor people have been living in the wilderness. They will not be happy if there is no food to eat.");
	narratorSay("Farms add homes to a town and they produce food. Build a farm using logs from the logging camp.");
	narratorSay("Be sure the current building material is rough logs. The current building material is selected at the upper right corner of the Construction toolbar.");
	narratorSay("Select Farm on the Construction toolbar. The cursor shows a log to confirm the current building material. While holding the Ctrl key, click on the terrain next to the road or the town square. A new construction sign shows a picture of a log with 0 over 30 next to it. This indicates that 30 logs are needed but none have been delivered.");
	narratorSay("Every city maintains a stock pile. The maximum amount of each commodity that can be stocked is determined by the buildings in the town. Construction materials, such as logs, can be fetched from the stock pile.");
	narratorSay("Return to the logging camp and look at the manufacturing panel.");
	narratorSay("Click Go on the manufacturing panel in the building at the logging camp. A number such as Q123x1 appears showing the quality and quantity of logs that were produced. The quality can range from 1 to 255. Click Go repeatedly until 30 logs have accumulated in the city's stock pile.");
	if(city.empire() & city.hasItemCount(log,30)) havelogs;

:havelogs
	narratorTitle("The Farm");
	narratorSay("Good work. The city's stock pile contains enough logs to build the farm.");
	narratorSay("Return to the farm and click Fetch on the construction sign. The quality of the logs determines the failure rate when they are fetched. If all 30 units could not be fetched then some of the logs failed. Click Fetch again until all 30 logs have been delivered. If necessary, return to the logging camp and chop some more wood.");
	narratorSay("Click the Work Hard button repeatedly until all required labor has been applied. This will complete construction of the farm.");
	if(city.empire() & city.hasBuilding(farm,1)) farmbuilt;

:farmbuilt
	narratorTitle("Agriculture and Livestock");
	narratorSay("Enter the farm and produce some food. Farms produce many different products using various combinations of materials. Try producing a little of each commodity the farm can produce. This will put some food into the town's inventory for your first citizens to eat.");
	narratorSay("Plants require water and sunlight. If your planet is dry, you may have to click Fetch many times to get water from the environment. Sunlight cannot be fetched from the environment while it is dark outside.");
	narratorSay("Animals and animal by-products require water and food. They do not require sunlight so you can produce them at night.");
	narratorSay("Farm animals are tame; they can be ridden for transportation. Use the farm to produce an animal. Approach it closely. Look at the animal and press E to mount it; E again dismounts it. An animal cannot be mounted if it is too small. Once mounted, movement keys control the animal instead of your own body. Move up (C key) or jump (Space Bar) with a flying animal to take off; its wing characteristics determine whether it will glide or hover when no movement keys are pressed. Not all animals that have wings can fly.");
	narratorSay("When a farm has no live domestic animals, a herbivore is gathered from the terrain in which the farm was built. Animal species vary according to terrain altitude, vegetation density, and time of day. If your farm does not produce any suitable riding animals, try again at a different time of day or try using another farm built in different terrain conditions.");
	narratorSay("When a farm already has a live domestic animal, its DNA is used to produce another animal just like it, though of opposite sex.");
	narratorSay("When a farm has two or more live domestic animals, subsequent animals are slaughtered immediately and added to the town's inventory. A meat processing plant can convert animal carcasses into meat and leather.");
	narratorSay("A farm producing animal by-products but not animals will host only one live domestic animal.");
	if(city.hasFood()) havefood;

:havefood
	narratorTitle("A Town is Born");
	narratorSay("Excellent! The town has food, shelter, and jobs. Refugees will start moving into the town. Thanks to your leadership, your people now have a bright and hopeful future.");
	narratorSay("People will move into the homes in the town. During daylight, they will work the jobs that are available. They will construct buildings and manufacture commodities. Citizens will not plan the locations of new buildings or roads; that is your job.");
	narratorSay("The town will send status reports to you periodically by mail. Status reports provide valuable information about the needs and wants of a town's citizens. The morale of the citizens determines whether people move to the city or move away from the city.");
	narratorSay("Press F5 to open the Mail toolbar. Look at your town status reports. It may take a few minutes for the first report to arrive.");
	narratorSayChoice(0, "Press F3 to display a transcript of these messages. Select this message then double-click or press Enter to continue.");
	if(choiceMade() == 0) economy;

:economy
	narratorTitle("Economy");
	narratorSay("Let's take a brief look at the economy developing in the town.");
	narratorSay("Together the farm and the logging camp create two homes and four jobs. The Construction toolbar shows that the logging camp adds two jobs; the farm adds two jobs and two homes. The population will not grow beyond the number of homes. Two more homes are needed to enable the population to grow to fill all of the jobs available.");
	narratorSay("Build two houses. The Construction toolbar shows that a house adds one job and two homes. Thus, two more houses will add two jobs and four homes to the town.");
	if(city.empire() & city.hasBuilding(house,2)) housesbuilt;

:housesbuilt
	narratorTitle("Expansion");
	narratorSay("The town has grown into a thriving agricultural community. With your guidance, this town can continue to grow into a city.");
	narratorSay("More towns will be needed eventually. Lets make another town now, to see how towns interconnect and share resources.");
	narratorSay("A flag will be needed to build a town square, just like when this town was founded. Textiles and a sewing needle are needed to make a flag.");
	narratorSay("Begin by building a carpenter. The carpenter can make the sewing needles.");
	if(city.empire() & city.hasBuilding(carpenter,1)) carpenter;

:carpenter
	narratorTitle("Tool Age");
	narratorSay("Construction and manufacturing processes use tools from the town's stock pile whenever possible. This happens when Work Hard or Go are pushed, either by you or by a citizen. Tools eventually wear out so it is a good idea to manufacture a steady supply of the tools your citizens need.");
	narratorSay("Sewing needles are essential when working with textiles. Change the carpenter's manufacturing process to make sewing needles using logs.");
	if(city.empire() & city.hasItem(sewing needle)) carpenter2;

:carpenter2
	narratorSay("The sewing needles in the stock pile will wear out as they are used. It is a good idea to let the carpenter make them from now on.");
	narratorSay("Hammers will be needed to build more complex buildings; they are also essential to boat-building. Plan another carpenter in the town. This time, let the citizens complete the construction. When it is finished, the new carpenter will start making hammers; you won't need to change it.");
	if(city.empire() & city.hasBuilding(carpenter,2)) textiles;

:textiles
	narratorTitle("Plant Fiber");
	narratorSay("Sewing needles enable your citizens to work with textiles. Textiles can be made from plant fiber.");
	narratorSay("Build an orchard or another farm. Change it to grow plant fiber.");
	if(city.empire() & city.hasItem(plant fiber)) havefiber;

:havefiber
	narratorTitle("Textiles");
	narratorSay("Raw plant fiber must be processed into textiles. A textile mill converts plant fiber into textiles.");
	narratorSay("Build a textile mill and process some of the plant fiber into textiles.");
	if(city.empire() & city.hasItem(textiles)) havetextiles;

:havetextiles
	narratorTitle("Sewing a Flag");
	narratorSay("A clothing factory makes many different products using textiles.");
	narratorSay("Build a clothing factory. It will start producing flags using textiles.");
	if(city.empire() & city.hasItem(flag)) haveflags;

:haveflags
	narratorTitle("Retail");
	narratorSay("The clothing factory has produced some flags. They were added to the town's inventory.");
	narratorSay("Build a retail store so you can buy a flag that has been produced. Retail stores sell products from the town's stock pile, including flags.");
	narratorSay("The inventory toolbar shows your current bank account balance. You are paid each time you click Work Hard or Go. By now you have accumulated enough money to buy some flags.");
	if(city.empire() & city.hasBuilding(retail store,1)) buyflag;

:buyflag
	narratorTitle("Buy a Flag");
	narratorSay("Enter the retail store and buy a flag. A wise leader is never caught in the wilderness without a flag; they are small, buy two or three.");
	if(avatarHasItem(flag)) hasflag;

:hasflag
	narratorTitle("Embrace the Sea");
	narratorSay("A flag enables you to start building another town anywhere you choose. You are now ready to continue the quest for your kingdom across the sea.");
	narratorSay("Build a wharf. A town with a wharf is connected by sea to other towns that also have a wharf. A wharf also produces fish, a source of food for the townspeople.");
	if(city.empire() & city.hasBuilding(wharf,1)) buildshipyard;

:buildshipyard
	narratorTitle("Build a Boat");
	narratorSay("The wharf is completed. This town can now trade goods with any other town on this planet that has a wharf.");
	narratorSay("Build a ship yard. The ship yard builds boats that you can use to cross the sea. Ship yards are picky about their location; they need a coastal location that is next to open sea to launch the new boats. There cannot be any sandbars blocking the way, no matter how small.");
	if(city.empire() & city.hasBuilding(ship yard,1)) buildaboat;

:buildaboat
	narratorTitle("Lumber Mill");
	narratorSay("Enter the ship yard and look at the manufacturing panel. A sail boat is under construction; milled lumber is needed. A lumber mill is needed to cut rough logs into lumber.");
	narratorSay("Build a lumber mill to cut logs into lumber. Lumber makes more efficient use of logs and lumber enables taller buildings to be constructed.");
	if(city.empire() & city.hasBuilding(lumber mill,1)) sailaway;

:sailaway
	narratorTitle("Master the Sea");
	narratorSay("The lumber mill is completed. Return to the ship yard and wait for the sail boat to be finished. Do not stand in the water in front of the ship yard; your presense in the terrain tile will prevent the sail boat from spawning.");
	narratorSay("Approach the sail boat closely and look at it. Use the E key to climb aboard. PageUp and PageDown raise and lower the sail.");
	narratorSay("Sail across the sea to a distant land. When you get there, use a flag you purchased to build a new town square near the coast.");
	if(city.empire() & city.hasBuilding(town square,1) & !city.hasBuilding(wharf,1)) connectwharfs;

:connectwharfs
	narratorTitle("Sea Lanes");
	narratorSay("Connect this new town with your first town by sea.");
	narratorSay("To do this, start construction of a wharf. Use stone as the building material. Look at its construction signs to see how much stone is needed to complete construction.");
	narratorSay("Build a mine in the new town and quarry enough stone to build the wharf.");
	narratorSay("The completed wharf will connect the new town by sea with the first town.");
	narratorSay("Complete construction of the wharf in the new town.");
	if(city.empire() & city.hasBuilding(wharf,1) & !city.hasBuilding(ship yard,1)) importsupplies;

:importsupplies
	narratorTitle("Free Trade");
	narratorSay("The wharf is completed. This town is now connected to the first town by sea.");
	narratorSay("Retail stores import the commodities that they sell from other cities. Retail stores stock many useful commodities, such as building materials.");
	narratorSay("Build a retail store to import building materials from your first town.");
	if(city.empire() & city.hasBuilding(retail store,1)) makecantina;

:makecantina
	narratorTitle("Waiting for the Shipment");
	narratorSay("The retail store is finished. After the retail store receives a shipment of commodities, the town's stock pile will contain milled lumber. This will be used to build the cantina.");
	narratorSay("Taverns sell food and drinks. Like the retail store, a cantina imports the commodities that it sells. This will bring food into the town for the citizens.");
	narratorSay("Build a cantina using milled lumber. Enter the cantina when it is completed.");
	P = avatar();
	if(city.empire() & unitAtBuilding(P, cantina) & building.stories(1)) destiny;

:destiny
	narratorTitle("Destiny");
	narratorSay("This cantina is a popular place for the locals. Bards sing songs and tell tales. People meet to relax at the end of a hard day.");
	narratorSay("The adventures of Targoss and Falla remain popular. Many of the old legends and folklore have seen a revival lately, in songs and stories. The romance and adventure of being caught up among the stars has renewed appeal lately.");
	narratorSay("Everyone knows that things are different now. The king has returned; the cycle has been renewed. A quiet anticipation of what may happen next can be felt from every passer-by. The old legends fall short of telling the tale from here.");
	narratorSay("Listening to the stories, the Dagger of Targoss close by my side, my mind begins to wander.");
	narratorSay("Targoss said, 'We must get off this island.' I am master of this entire world but I have never found the Shores of Hazeron. Where is this kingdom? Was Don trying to tell me something more, when he repeated the words of Targoss with his dying breath?");
	narratorSay("As I look to the night sky sprinkled with stars, I know in my heart that my destiny lies in the footsteps of Targoss and Falla. The Shores of Hazeron are out there somewhere. I must go there to claim my throne.");
	narratorSayChoice(0, "Press F3 to display a transcript of these messages. Select this message then double-click or press Enter to continue.");
	if(choiceMade() == 0) shores;

:shores
	narratorTitle("Shores of Hazeron");
	narratorSay("It is time to claim your throne among the stars.");
	narratorSay("Use the knowledge you have learned to build an interstellar empire.");
	narratorSay("Good luck. Watch out for Umans.");
