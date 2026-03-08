module arrays
	dim a []

	; push
	get a
	int 321
	push

	get a
	println

	; get at index
	get a
	int 0
	indx
	println

	; set at index
	get a
	int 0
	int 123
	indx set

	get a
	int 0
	indx
	println

	; should fail
	; get a
	; str "hello"
	; push

	; pop
	get a
	pop
	drop

	; should work
	get a
	str "hello"
	push
	get a
	int 0
	indx
	println
