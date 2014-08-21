/* The MIT License:

Copyright (c) 2012-2014 Ivan Gagis <igagis@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. */

// Home page: http://morda.googlecode.com

/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once

#include <ting/config.hpp>


#if M_OS == M_OS_WINDOWS
#	ifdef DELETE
#		undef DELETE
#	endif
#endif


namespace morda{

//WARNING: do not add new key codes in the middle and do not change order. Add new key codes to the end of the enumeration.
enum class EKey{
	UNKNOWN,
	SPACE,
	ENTER,
	ZERO,
	ONE,
	TWO,
	THREE,
	FOUR,
	FIVE,
	SIX,
	SEVEN,
	EIGHT,
	NINE,
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	LEFT,
	RIGHT,
	UP,
	DOWN,
	COMMA,
	SEMICOLON,
	APOSTROPHE,
	PERIOD,
	SLASH,
	BACKSLASH,
	TAB,
	LEFT_SHIFT,
	RIGHT_SHIFT,
	END,
	LEFT_SQUARE_BRACKET,
	RIGHT_SQUARE_BRACKET,
	TICK,
	MINUS,
	EQUALS,
	BACKSPACE,
	CAPSLOCK,
	ESCAPE,
	LEFT_CONTROL,
	LEFT_ALT,
	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,
	RIGHT_CONTROL,
	PRINT_SCREEN,
	RIGHT_ALT,
	HOME,
	PAGE_UP,
	PAGE_DOWN,
	INSERT,
	DELETE,
	PAUSE,
	WINDOWS,
	WINDOWS_MENU,
	
	ENUM_SIZE
};



}//~namespace
