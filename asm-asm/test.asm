; begin
module test
	; basic maths
	int 23
	int 12
	add
	dup
	println
	int 2
	sub
	println

	; strings
	str "hello world"
	println

	; variables
	dim a 123
	get a
	println

	int 456
	set a
	get a
	println

	int 1
	int 2
	int 3
	call myfunc

	yield

; test function - 3 arguments. test argument call structure
myfunc:
	dim $a 0
	dim $b 0
	dim $c 0
	set $c
	set $b
	set $a

	get $a
	print
	get $b
	print
	get $c
	println

	jmp myfunc_end
	str "< no! >"
	println

myfunc_end:
	ret
