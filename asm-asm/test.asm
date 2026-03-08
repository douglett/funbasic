module test
	; set a and b to the same number (ref)
	dim a 111
	get a
	println

	dim b 0
	get a
	set b

	get a
	print
	get b
	println

	; set b to 333. what happens?
	int 333
	set b

	get a
	print
	get b
	println
	