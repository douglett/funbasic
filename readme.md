Fun basic
=========

Small ASM-like basic which can be easily parsed and played with.

Compile target.

TODO:
-----
- reference system
	- testing - what is is doing exactly?
		- ref for heap data - ok
		- ref for numbers - this turns out to be fine!
		- ref for strings
			- ok for read / split / concat
			- ok for array set, but strings become non-const (need clone)
			- alternatively, don't allow array set - like js. becomes no-op (or error?)
	- assume ref? (current)
	- assume clone?
	- explicit clone command? 
- callex system
	- system calls
	- multiple runtimes
- setex / getex (?)
- multiple modules
