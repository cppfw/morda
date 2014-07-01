/* The MIT License:

Copyright (c) 2014 Ivan Gagis <igagis@gmail.com>

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

#include <ting/Buffer.hpp>
#include <ting/utf8.hpp>

#include <string>

namespace morda{


class Font{
	Font(const Font&);
	Font& operator=(const Font&);
	
protected:
	Font(){}
	
public:	
	virtual ~Font()throw(){}
	
	//renders the string, returns resulting string advance
	virtual float RenderString(const morda::Matr4f& matrix, ting::utf8::Iterator str)const = 0;
	
	virtual float RenderString(const morda::Matr4f& matrix, const ting::Buffer<ting::u32>& utf32str)const = 0;
	
	float RenderString(const morda::Matr4f& matrix, const char* str)const{
		return this->RenderString(matrix, ting::utf8::Iterator(str));
	}
	
	float RenderString(const morda::Matr4f& matrix, const std::string& str)const{
		return this->RenderString(matrix, str.c_str());
	}
private:

};

}
