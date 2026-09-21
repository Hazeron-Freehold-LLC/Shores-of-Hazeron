#nocompile
# The block with the empty segment label ':' is the catalyst for starting every story. It is required.
:
	# Basic story info. storyTitle, storyAuthor, and storyBrief are required.
	storyTitle("Getting Started");
	storyAuthor("Haxus");
	storyBrief("Brief introduction for a new player.");
	storyAutoStart();											# This story is allowed to start automatically.

	# Character definitions.
	Targoss = char.define(Male, Right Handed, None, Avatar);

	# Minutes before this script can start again, after starting once.
	# storyInterval(10);

	# At least one 'if' is required; when any of them returns true, the story can start.
	# if(avatarXpLess(10) & !avatarHasItem(Torch)) begin;
	# if(avatarXpMore(10)) begin; #DHH For testing
	if(avatarXp() < 0) begin;	#DHH This will never be true. This script is disabled.

:begin
	narratorTitle("Getting Started");
	goto targoss;

:targoss
	narratorSay("Targoss has arrived.");
	char.spawn(Targoss, 180, 50);
	char.equip(Targoss, Wear, Fabric Clothing, 100);
	char.equip(Targoss, Hand, Torch, 100);
	ch = character(Targoss);
	sentientFollowAvatar(ch);
	if(char.distanceLess(Targoss, 12)) welcome;

:welcome
	char.stay(Targoss);
	P = avatar();
	char.sayAudio(Targoss, welcome.wav, "Welcome [P.Name], I am here to help you get started.");
	if(secondsElapsed() >= 5) welcome2;

:welcome2
	char.sayAudio(Targoss, welcome2.wav, "It is my job to teach you a few things that will help you find your way in this world.");
	if(secondsElapsed() >= 6) turn;

:turn
	#narratorTitle("Looking Around");
	ch = character(Targoss);
	sentientFollowAvatar(ch);
	char.sayAudio(Targoss, turn.wav, "Move the mouse left and right to turn your body. Turn until you are facing me.");
	if(secondsElapsed() >= 6) turn2;

:turn2
	char.sayAudio(Targoss, turn2.wav, "Move the mouse away from you and toward you to look up and down.");
	if(secondsElapsed() >= 5) hand;

:hand
	#narratorTitle("Your Hand");
	char.sayAudio(Targoss, hand.wav, "Click the right mouse button to make the hand cursor appear. "
			"The hand cursor is your character's hand. Use it to operate window buttons and controls.");
	if(secondsElapsed() >= 11) hand2;

:hand2
	char.sayAudio(Targoss, hand2.wav, "Your hand can also be used in the scene to operate doors, "
			"activate controls and grab items.");
	if(secondsElapsed() >= 8) hand3;

:hand3
	char.sayAudio(Targoss, hand3.wav, "Click the right mouse button again to make the hand cursor go away. "
			"The mouse goes back to controlling your body.");
	if(secondsElapsed() >= 8) comm;

:comm
	#narratorTitle("Communication");
	ch = character(Targoss);
	sentientFollowAvatar(ch);
	char.sayAudio(Targoss, comm.wav, "Everything I say is transcribed on your Comm. "
			"Press the F3 key to show the Comm at this time.");
	if(secondsElapsed() >= 8) comm2;

:comm2
	char.sayAudio(Targoss, comm2.wav, "The Comm shows open channels on tabs across the middle. Select the Voice channel tab.");
	if(secondsElapsed() >= 7) comm3;

:comm3
	char.sayAudio(Targoss, comm3.wav, "I am going to send a message that will be marked on the Comm with a large green asterisk. "
			"The asterisk means I am waiting for you to acknowledge the message before I will continue.");
	if(secondsElapsed() >= 12) comm4;

:comm4
	ch = character(Targoss);
	sentientFollowAvatar(ch);
	char.sayAudio(Targoss, comm4.wav, "Double-click the Comm message with the green asterisk to acknowledge it. "
			"You can also select the message and press the enter key to acknowledge it.");
	char.sayChoice(Targoss, 0, "I will wait until you acknowledge this message to continue.");
	char.sayChoice(Targoss, 1, "Go away, I don't need your help.");
	if(choiceMade() == 0) moving;
	if(choiceMade() == 1) helpUnwanted;
	if(secondsElapsed() >= 30) comm4;

:moving
	#narratorTitle("Moving Around");
	char.stay(Targoss);
	char.sayAudio(Targoss, moving.wav, "Excellent! Now that we can communicate, let's go over the basics of moving around.");
	if(secondsElapsed() >= 6) move2;

:move2
	char.sayAudio(Targoss, move2.wav, "To walk forward, hold the W key; Shift+W runs forward.");
	if(secondsElapsed() >= 6) move3;

:move3
	char.sayAudio(Targoss, move3.wav, "To walk backward, hold the S key.");
	if(secondsElapsed() >= 4) move4;

:move4
	char.sayAudio(Targoss, move4.wav, "You can sidestep left or right by pressing the A or D keys.");
	if(secondsElapsed() >= 6) move5;

:move5
	char.sayAudio(Targoss, move5.wav, "Try moving around this area to get the hang of it.");
	if(char.distanceMore(Targoss, 50)) comeback;
	if(char.distanceLess(Targoss, 6)) getback;
	if(secondsElapsed() >= 20) enoughMoving;

:enoughMoving
	ch = character(Targoss);
	sentientFollowAvatar(ch);
	char.sayAudio(Targoss, enoughMoving.wav, "Ok, that's enough moving. Let's go on to the next lesson.");
	if(char.distanceLess(Targoss, 20)) pickUp;

:comeback
	ch = character(Targoss);
	sentientFollowAvatar(ch);
	char.sayAudio(Targoss, comeback.wav, "Wait; I'm not done yet. There's a few more things I'm supposed to teach you.");
	if(char.distanceLess(Targoss, 20)) pickUp;

:getback
	char.sayAudio(Targoss, getback.wav, "Excuse me; that's a little close. Please back off a little.");
	if(char.distanceMore(Targoss, 8)) pickUp;

:pickUp
	T = character(Targoss);
	sentientStay(T);
	torch = sentientDropGearItem(T, Torch);
	itemMakePermanent(torch);
	char.moveBack(Targoss, 4);
	char.sayAudio(Targoss, pickUp.wav, "I have dropped a torch on the ground in front of me. Use your hand to pick it up.");
	if(avatarHasItem(Torch)) hasTorch;

:hasTorch
	#narratorTitle("Using Equipment");
	ch = character(Targoss);
	sentientFollowAvatar(ch);
	char.sayAudio(Targoss, hasTorch.wav, "Good job. You can keep the torch; it'll be handy when it gets dark.");
	if(secondsElapsed() >= 5) selectItem;

:selectItem
	char.sayAudio(Targoss, selectItem.wav, "Use the mouse wheel to select the torch as your current item. "
			"The current item is shown as a picture at the bottom corner of the scene. "
			"That's the item that gets used when you click the mouse.");
	if(avatarCurrentItem(Torch)) lightTorch;

:lightTorch
	char.sayAudio(Targoss, lightTorch.wav, "Notice that the torch can be selected twice. "
			"When the picture of a fire appears next to the torch, it is wielded as a weapon. "
			"To attack with a weapon, sight on a target and hold the left mouse button to attack repeatedly.");
	if(secondsElapsed() >= 14) lightTorch2;

:lightTorch2
	char.sayAudio(Targoss, lightTorch2.wav, "Select the other torch position, the one without fire, to use the torch for light. "
			"Press the U key to 'use' the torch; this will turn the torch light on and off. "
			"A single click with the mouse wheel is another way to use the current item.");
	if(secondsElapsed() >= 15) getwork;

:getwork
	#narratorTitle("Jobs");
	char.sayAudio(Targoss, getwork.wav, "One last thing and then I'll be finished.");
	if(secondsElapsed() >= 4) getwork2;

:getwork2
	char.sayAudio(Targoss, getwork2.wav, "As you travel from place to place, use the Comm to ask about jobs in the area. "
			"There is growing demand out there for travellers willing to take a risk.");
	if(secondsElapsed() >= 10) getwork3;

:getwork3
	char.stay(Targoss);
	char.sayAudio(Targoss, getwork3.wav, "My job is done. That is all I can teach you. You are on your own now.");
	if(char.distanceMore(Targoss, 10)) bye;

:bye
	char.stay(Targoss);
	char.sayAudio(Targoss, bye.wav, "Good bye and good luck in your travels.");
	# The story ends when the current block does not branch.

# Alternate path when help is not wanted.
:helpUnwanted
	T = character(Targoss);
	sentientStay(T);
	torch = sentientDropGearItem(T, Torch);
	itemMakePermanent(torch);
	char.moveBack(Targoss, 4);
	char.sayAudio(Targoss, pickUp.wav, "I have dropped a torch on the ground in front of me. Use your hand to pick it up.");
	if(avatarHasItem(Torch)) hasTorchBye;

:hasTorchBye
	#narratorTitle("Using Equipment");
	ch = character(Targoss);
	sentientFollowAvatar(ch);
	char.sayAudio(Targoss, hasTorch.wav, "Good job. You can keep the torch; it'll be handy when it gets dark.");
	if(secondsElapsed() >= 5) bye;
