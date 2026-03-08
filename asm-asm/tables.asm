module tables
	dim a {}

	; set table member
	get a
	int 12
	memb set fart
	get a
	println

	; get table member
	get a
	memb fart
	println

	; unset table member
	get a
	memb unset fart
	get a
	println
