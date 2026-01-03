module doug1
	dim str = ""
	dim dir = 0
	dim a = 0

	print "You are standing in a strange room."
	print "There are doors leading north(1), south(2), east(3), and west(4)."
	print "Please pick a number to proceed: "
	; input str
	let dir = a
	; let dir = toint(str)

door1:
	; let a = dir
	; let a == 1
	; ifn a then goto door2
; 		print "You open the north door... there was a bottomless pit there!"
; 		print "You fall into the pit and die."
; 		goto theend
; door2:
; 	; let a == 2
; 	; if a then goto door2
; 	; let a == 3
; 	; if a then goto door3
; 	; let a == 4
; 	; if a then goto door4
; 	; goto nodoor

; theend:
; 	print "THE END!"
; 	