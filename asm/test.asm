module test
	dim a = 123
	dim b = "hello world"
	print b a

	dim c = [1, 2, 3, a]
	print c

	; push c, 10
	; print c

	; push c, c
	; print c

	; let c += 10

	let b += " rolf"
	print b
