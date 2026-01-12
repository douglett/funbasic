module arrays
	dim a []

	get a
	int 321
	push

	get a
	println

	get a
	int 0
	indx
	println

	; should fail
	; get a
	; str "hello"
	; push

	; should work
	get a
	pop
	drop
	get a
	str "hello"
	push
	get a
	int 0
	indx
	println
