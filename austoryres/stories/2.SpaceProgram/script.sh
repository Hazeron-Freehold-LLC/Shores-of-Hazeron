#nocompile
# The block with the empty segment label ':' is the catalyst for starting every story. It is required.
:
	# Basic story info. storyTitle, storyAuthor, and storyBrief are required.
	storyTitle("Space Program");
	storyAuthor("Haxus");
	storyBrief("Targoss teaches how to set up a space program.");

	# This story is allowed to start automatically one time.
	storyAutoStart();

	# At least one 'if' is required; when any of them returns true, the story can start.
	av = avatar();
	city = atomCity(av);
	if(cityHasBuilding(city, Aircraft Factory) &
			cityHasBuilding(city, University) &
			cityHasPower(city) &
			!cityHasBuilding(city, Spacecraft Factory)) begin;

:begin
	narratorTitle("Space Program");
	if(secondsElapsed() >= 5) targossArrives;

:targossArrives
	narratorSay("Targoss has arrived.");
	narratorHint("Move close to Targoss.");
	av = avatar();
	Targoss = atomCreateCharacter(av, 180, 20, "Targoss", Male, Right Handed, Empire, Avatar);
	characterSetIndestructible(Targoss, true);
	sentientCreateGear(Targoss, Wear, false, Fabric Clothing, 1, Empire, "");
	sentientCreateGear(Targoss, Hand, true, Torch, 1, Empire, "");
	sentientFollowAvatar(Targoss);
	if(atomDistanceToAtom(av, Targoss) < 8) welcome;

:welcome
	narratorHint("Double click or press Enter on a message with a green splat, to accept that choice.");
	sentientSayAudio(Targoss, "Your city appears ready to develop a more advanced space program.");
	sentientSayAudio(Targoss, "I will help set that up so you can build a space station and space ships.");
	sentientSayChoice(Targoss, 0, "Thank you. Please continue.");
	sentientSayChoice(Targoss, 1, "No, thank you. I know how to make space stations and space ships.");
	if(choiceMade() == 0) helpWanted;
	if(choiceMade() == 1) goodBye1;
	if(secondsElapsed() >= 120) welcome;

:goodBye1
	# Story ended early, assertively.
	avatarStoryResult(1);
	goto goodBye;

:goodBye
	sentientSayAudio(Targoss, "Good bye.");
	goto theEnd;	# Branch before Targoss departs or the good bye message never gets sent.

:theEnd
	characterDepart(Targoss);
	# The story ends when the current block does not branch.

:helpWanted
	sentientSayAudio(Targoss, "The city has an aircraft factory and a university.");
