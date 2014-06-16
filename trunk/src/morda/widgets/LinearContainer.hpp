/* The MIT License:

Copyright (c) 2014 Ivan Gagis

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

#include "Container.hpp"
#include "PaddedWidget.hpp"
#include "LinearWidget.hpp"

#include <ting/types.hpp>


namespace morda{


class LinearContainer :
		public Container,
		public PaddedWidget,
		public LinearWidget
{
	LinearContainer(const LinearContainer&);
	LinearContainer& operator=(const LinearContainer&);

	LinearContainer(){}
	LinearContainer(const stob::Node& desc);	
public:

	void OnResize() OVERRIDE;	
	
	morda::Vec2f ComputeMinDim()const throw() OVERRIDE;
	
	static ting::Ref<LinearContainer> New(){
		return ting::Ref<LinearContainer>(new LinearContainer());
	}
	
	static ting::Ref<LinearContainer> New(const stob::Node& desc){
		return ting::Ref<LinearContainer>(new LinearContainer(desc));
	}
private:

};


}
