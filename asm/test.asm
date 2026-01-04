module test
	dim a = 123
	dim b = "hello world"
	print b a

	dim c = [1, 2, 3, a, 99]
	print c

	let c += [ 10 ]
	print c
	; let c -= 10
	; let c = erase(c, 1, 10)
	; let c = pop(c, 10)

	let b += " rolf"
	print b

	dim sprite1 = [0 0 0, 4 4,  1 1 1 1,  1 0 0 1,  1 0 0 1,  1 1 1 1]
	sprreg sprite1
	sprdereg sprite1

	; let sprite1[3] = 2
	pop a = c
	print c a
	dim t = []
	pop t = c, 2
	print c
