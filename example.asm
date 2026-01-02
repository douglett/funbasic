; blah: 	mov a b
; 		add a b
; 		sub a b
; 		mov r a
; 		load a b
; 		save a b
; 		ret

; main: 	call blah
; 		callm std strcat
; 		callx print
; 		dim b "hello world"
; 		dim a 123

dim a = 123
dim b = "hello world"
dim c = []

let a = num(b)
let b = string(a)
if a = b then goto blah
let z = blah