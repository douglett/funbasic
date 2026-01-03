module doug1
	dim str = ""
	dim dir = 0
	dim a = 0

	print "You are standing in a strange room."
	print "There are doors leading north(1), south(2), east(3), and west(4)."
	print "Please pick a number to proceed: "
	input str
	let dir = toint(str)

door1:
	let a = dir
	let a == 1
	ifn a then goto door2
		print "You open the north door... there was a bottomless pit there!"
		print "You fall into the pit and die."
		goto theend
door2:
	let a = dir
	let a == 2
	ifn a then goto door3
		print "You open the south door... a yeti jumps out at you!"
		print "The yeti gobbles you up. You taste disgusting."
		goto theend
door3:
	let a = dir
	let a == 3
	ifn a then goto door4
		print "You open the east door... a samurai stands dramatically, framed by the rising sun."
		print "He chops your head off! Blood sprays out everywhere like a cool movie."
		goto theend
door4:
	let a = dir
	let a == 4
	ifn a then goto nodoor
		print "You open the west door... a cowboy is here holding a six-shooter. He tips his hat, 'Howdy pardner!'"
		print "Oops, the safety was off! He shoots you by accident and you die stupidly."
		goto theend
nodoor:
	print "Wracked with indecision, you stand around helplessly."
	print "You starve to death!"
	goto theend

theend:
	print "THE END!"
	