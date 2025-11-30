# Terminal calculator
A calculator in the terminal, similar to just running the python or node interpreters without any file

## Features
 - Correct left-assosiative precedence parsing
 - Multiple output formats (hexadecimal, binary and decimal)
 - Aliases for constants (true, false and PI)
 - Expression history with up arrow and down arrow
 - Common operators like bit shifting, remainder, etc
 - Strings and chars. Can be provided as arguments to functions, eg `len("Hello, world") // expected output: 12`
 - Constant functions

## Functions
 - sin(x) // x is in degrees
 - cos(x) // x is in degreess
 - tan(x) // x is in degrees
 - atan(x) // x is in degrees
 - deg(rad)
 - rad(deg)
 - fah(x) // x is in celcius
 - cel(x) // x is in fahrenheit
 - dec(x) // x is a number or character eg dec("A") // expected output: 65
 - hex(x) // x is a number or character eg hex(0b1000001) // expected output: 0x41
 - bin(x) // x is a number or character eg bin(0xF) // expected output: 0b1111
 - round(x) // x is a number
 - floor(x) // x is a number
 - ceil(x) // x is a number
 - abs(x) // x is a number
 - len(str) // str is a string eg "Hello, буржуй"
 - chr|char(x) // x is a number of the ascii character code eg chr(65) + "n" + char(100) // expected output: "And"
