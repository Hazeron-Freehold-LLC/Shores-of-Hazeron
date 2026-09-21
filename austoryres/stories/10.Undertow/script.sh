# The block with the empty segment label ':' is the catalyst for starting every story. It is required.
:
	# Basic story info. storyTitle, storyAuthor, and storyBrief are required.
	storyTitle("Undertow");
	storyAuthor("Haxus");
	storyBrief("Collect samples of eludium for research.");

	# This story is allowed to start automatically repeatedly until the avatar completes it.
	# The avatar can drop this story over and over until it is finished.
	storyAutoRepeatUntilComplete();
	storyInterval(1);	# Minutes before this script can start again, after starting once.

	# At least one 'if' is required; when any of them returns true, the story can start.
	# We want a university and a broker in the system, and an avatar who has completed rocket training but never held eludium.
	sys = avatarEnvironment();
	broker = atomFindBuilding(sys, Broker, 1, Neutral, 1);
	if(avatarStoryCompleted(4) &
			atomHasBuilding(sys, University, 1, Neutral, 1) &
			atomValid(broker) &
			avatar() == atom(16))
		begin;

:begin
	narratorTitle("Undertow");	# Appears centered on screen with a bleep. Optional.
	if(wait(5)) intro;	# Wait 5 seconds and branch.

:intro
	Harles = atomCreateCharacter(broker, 0, 10, "Harles", Male, Left Handed, Empire, Irinam.chr, Mature); 
	characterSetIndestructible(Harles, true);
	sentientCreateGear(Harles, Wear, false, Jeans and Shirt, 10, Empire, "");
	world = atomWorld(broker);
	if(worldAtmosphereBreathable(world)) sayHello;
	if(!worldAtmosphereBreathable(world)) evaSuit;

:evaSuit
	# Equip Harles with an EVA suit.
	sentientCreateGear(Harles, Wear, false, EV Suit, 10, Empire, "");
	sentientCreateGear(Harles, Head, false, EV Helmet, 10, Empire, "");
	goto sayHello;

:sayHello
	av = avatar();
	atomCommChannelToAtom(Harles, av, "Harles-[av.Name]");
	atomComm(Harles, "Harles-[av.Name]", System, "Excuse me, [av.Name]. Please pardon the intrusion.");
	if(wait(5)) serviceNeeded;	# Wait 5 seconds and branch.

:serviceNeeded
	atomComm(Harles, "Harles-[av.Name]", System, "I heard you are an accomplished rocket pilot.");
	atomComm(Harles, "Harles-[av.Name]", System, "My employer has a need for your services.");
	if(wait(10)) meetMe;

:meetMe
	brokerLL = atomWorldLL(broker);
	atomComm(Harles, "Harles-[av.Name]", System, "If you are interested, meet me to discuss the matter, privately.");
	atomComm(Harles, "Harles-[av.Name]", System, "I am waiting in front of the [broker.Name] building, at [brokerLL].");
	atomComm(Harles, "Harles-[av.Name]", System, "I will anticipate your arrival.");
	if(atomDistanceToAtom(Harles, av) < 8) greetings;

:greetings
	sentientSayTo(Harles, av, "Delightful. It is an honor to meet you, [av.Name].", Smile);
	sentientSayTo(Harles, av, "Allow me to introduce myself. I am Harles, at your service.", Smile);
	if(wait(10)) whoAreWe;

:whoAreWe
	sentientSayTo(Harles, av, "I represent Poralis Corporation.", Open);
	sentientSayTo(Harles, av, "As a brokerage company, Poralis is keenly interested in the explosive growth that is going to accompany the expansion into space.", Open);
	sentientSayTo(Harles, av, "Among our many peripheral activities, we conduct scientific research that benefits us.", Open);
	if(wait(20)) eludium;

:eludium
	sentientSayTo(Harles, av, "Our scientists have theorized the existence of a substance that will revolutionize space travel.", Open);
	sentientSayTo(Harles, av, "They believe a material exists that is so elusive, it cannot exist in the presence of any atmosphere.", Open);
	sentientSayTo(Harles, av, "They have dubbed it 'eludium'.", Open);
	if(wait(20)) theDeal;

:theDeal
	sentientSayTo(Harles, av, "Eludium is predicted to be extremely rare and difficult to find.", Open);
	sentientSayTo(Harles, av, "My employer will pay a hefty reward for samples.", Open);
	sentientSayTo(Harles, av, "I am authorized to offer 1000c per unit of eludium, up to one full crate.", Open);
	sentientSayChoice(Harles, 0, "I'll do it.", Open);
	sentientSayChoice(Harles, 1, "Go away. I don't deal with middle men.", Open);
	if(choiceMade() == 0) illWait;
	if(choiceMade() == 1) goAway;
	if(wait(30)) theDeal;

:illWait
	sentientSayTo(Harles, av, "Splendid; I will call my boss to report the good news.", Smile);
	sentientSayTo(Harles, av, "Ms. Ulitas will be very pleased.", Smile);
	avatarStoryResult(1);	# They took the deal, at 1000c each.
	if(wait(15)) deliverToMe;

:goAway
	sentientSayTo(Harles, av, "This is very disappointing. I must report to my superior for guidance.", Frown);
	sentientSayTo(Harles, av, "She will not be happy to hear this.", Frown);
	if(wait(180)) theCall;

:theCall
	Ulitas = atomCreateCharacter(broker, 0, 0, "Ulitas", Female, Right Handed, Empire, Avatar, Mature); 
	characterSetIndestructible(Ulitas, true);
	atomCommChannelToAtom(Ulitas, av, "Ulitas-[av.Name]");
	atomComm(Ulitas, "Ulitas-[av.Name]", System, "[av.MsMr] [av.Name], this is Ms. Ulitas, of Poralis Corporation.");
	atomComm(Ulitas, "Ulitas-[av.Name]", System, "My assistant, Harles, may not have been clear about the urgency of this matter.");
	if(wait(20)) sweetenTheDeal;

:sweetenTheDeal
	atomComm(Ulitas, "Ulitas-[av.Name]", System, "You left me with little choice, [av.Name].");
	atomComm(Ulitas, "Ulitas-[av.Name]", System, "I am prepared to boost my offer to 2000c per unit.");
	atomComm(Ulitas, "Ulitas-[av.Name]", System, "To sweeten the deal, I'll promise to send more jobs your way.");
	atomComm(Ulitas, "Ulitas-[av.Name]", System, "You must give me time to work you in, though.");
	atomCommChoice(Ulitas, "Ulitas-[av.Name]", System, 0, "You have my interest; continue.");
	if(choiceMade() == 0) exactAPromise;

:exactAPromise
	atomComm(Ulitas, "Ulitas-[av.Name]", System, "I am going out on a limb here, placing a lot of trust in you.");
	atomComm(Ulitas, "Ulitas-[av.Name]", System, "Svoth is breathing down our necks. We can't let them get the jump on us.");
	atomComm(Ulitas, "Ulitas-[av.Name]", System, "You better not double cross me; I need you to promise.");
	atomCommChoice(Ulitas, "Ulitas-[av.Name]", System, 0, "I'll get the samples for you. I won't let you down.");
	atomCommChoice(Ulitas, "Ulitas-[av.Name]", System, 1, "No deal.");
	if(choiceMade() == 0) doneDeal;
	if(choiceMade() == 1) noDeal;

:noDeal
	atomComm(Ulitas, "Ulitas-[av.Name]", System, "Your decision is unfortunate.");
	atomComm(Ulitas, "Ulitas-[av.Name]", System, "Good day, [av.Name].");
	characterDepart(Ulitas);
	avatarStoryResult(3);	# They rejected every offer.
	# The story ends when the current block does not branch.

:doneDeal
	atomComm(Ulitas, "Ulitas-[av.Name]", System, "It has been a pleasure, [av.Name].");
	atomComm(Ulitas, "Ulitas-[av.Name]", System, "Good bye. Get that eludium.");
	characterDepart(Ulitas);
	avatarStoryResult(2);	# They took the improved deal, at 2000c each.
	goto backToHarles;

:backToHarles
	sentientSayTo(Harles, av, "Excellent; Ms. Ulitas said you reached an agreement.", Smile);
	goto deliverToMe;

:deliverToMe
	sentientSayTo(Harles, av, "Deliver the samples to me for payment, as soon as you can. I will wait here.", Open);
	if(wait(10)) beDiscreet;

:beDiscreet
	sentientSayTo(Harles, av, "Be discreet. Do not discuss this with anyone.", Closed);
	sentientSayTo(Harles, av, "Our competitors will try to stop you.", Closed);
	# The story ends when the current block does not branch.

