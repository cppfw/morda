/* The MIT License:

Copyright (c) 2012 Ivan Gagis <igagis@gmail.com>

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

#include <ting/Singleton.hpp>
#include <ting/types.hpp>
#include <ting/config.hpp>
#include <ting/Buffer.hpp>

#include "config.hpp"

#ifdef M_MORDA_OGLES2
#	include <GLES2/gl2.h>
#	include <EGL/egl.h>
#else
#	include <GL/glew.h>
#	include <GL/glx.h>
#endif


#include "Exc.hpp"
#include "Widget.hpp"
#include "Container.hpp"
#include "GuiInflater.hpp"
#include "resman/ResourceManager.hpp"

#include "shaders/SimpleSingleColoringShader.hpp"
#include "shaders/SimpleTexturingShader.hpp"


namespace morda{



class App : public ting::IntrusiveSingleton<App>{
	friend class ting::IntrusiveSingleton<App>;
	static ting::IntrusiveSingleton<App>::T_Instance instance;
	
	
public:
	struct DefaultShaders{
		SimpleSingleColoringShader simpleSingleColoring;
		SimpleTexturingShader simpleTexturing;
	};
	
private:
	
#if M_OS == M_OS_LINUX
	
#	ifdef __ANDROID__
	
	EGLDisplay eglDisplay;
	EGLContext eglContext;
	EGLSurface eglSurface;
	
	friend void SetEGLStuff(App* app, EGLDisplay display, EGLContext context, EGLSurface surface);
	friend void UpdateWindowDimensions(App* app, const tride::Vec2f& newWinDim);
	
	//TODO: create shaders when OGL is initialized
	ting::Ptr<DefaultShaders> shaders;

	inline void SwapGLBuffers(){
		eglSwapBuffers(this->eglDisplay, this->eglSurface);
	}

public:
	inline DefaultShaders& Shaders()throw(){
		return *this->shaders;
	}



#	else //generic linux

	struct XDisplayWrapper{
		Display* d;
		XDisplayWrapper();
		~XDisplayWrapper()throw();
	} xDisplay;
	
	struct XVisualInfoWrapper{
		XVisualInfo *vi;
		XVisualInfoWrapper(XDisplayWrapper& xDisplay);
		~XVisualInfoWrapper()throw();
	} xVisualInfo;
	
	struct XWindowWrapper{
		Window w;
		
		XDisplayWrapper& d;

		XWindowWrapper(unsigned width, unsigned height, XDisplayWrapper& xDisplay, XVisualInfoWrapper& xVisualInfo);
		~XWindowWrapper()throw();
	} xWindow;
	
	struct GLXContextWrapper{
		GLXContext glxContext;
		
		XDisplayWrapper& d;
		XWindowWrapper& w;
		
		GLXContextWrapper(XDisplayWrapper& xDisplay, XWindowWrapper& xWindow, XVisualInfoWrapper& xVisualInfo);
		~GLXContextWrapper()throw(){
			this->Destroy();
		}
		
		void Destroy()throw();
	} glxContex;
	
	ting::Inited<volatile bool, false> quitFlag;
	
	friend void Main(int argc, char** argv);
	void Exec();
	
	inline void SwapGLBuffers(){
		glXSwapBuffers(this->xDisplay.d, this->xWindow.w);
	}
	
	DefaultShaders shaders;
	
public:
	inline DefaultShaders& Shaders()throw(){
		return this->shaders;
	}
#	endif

#else
#	error "unsupported OS"
#endif
	
private:
	tride::Vec2f curWinDim;
	
	ting::Ref<morda::Container> rootContainer;
	
	ResourceManager resMan;
	
	GuiInflater inflater;
	
	void SetGLViewport(const tride::Vec2f& dim);
	
	void UpdateWindowDimensions(const tride::Vec2f& dim);
	
	void Render();
	
protected:
	App(unsigned w, unsigned h);

public:
	
	virtual ~App()throw(){}
	
	virtual void Init(const ting::Buffer<const ting::u8>& savedState = ting::Buffer<const ting::u8>(0, 0)) = 0;
	
	inline void SetRootContainer(const ting::Ref<morda::Container>& c){
		this->rootContainer = c;
		this->rootContainer->SetPos(tride::Vec2f(0));
		this->rootContainer->Resize(this->curWinDim);
	}
	
	inline ResourceManager& ResMan()throw(){
		return this->resMan;
	}
	
	inline GuiInflater& Inflater()throw(){
		return this->inflater;
	}
};



/**
 * @brief Create application instance
 * User needs to define this factory function to create his application instance.
 * @return New application instance.
 */
ting::Ptr<App> CreateApp(int argc, char** argv);



}//~namespace
