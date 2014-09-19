//This file contains implementations of platform dependent methods from App class.

#include <vector>
#include <array>

#include "../../AppFactory.hpp"

#include <ting/WaitSet.hpp>
#include <ting/fs/FSFile.hpp>
#include <ting/utf8.hpp>
#include <ting/util.hpp>



using namespace morda;



App::XDisplayWrapper::XDisplayWrapper(){
	this->d = XOpenDisplay(0);
	if(!this->d){
		throw morda::Exc("XOpenDisplay() failed");
	}
}



App::XDisplayWrapper::~XDisplayWrapper()NOEXCEPT{
	XCloseDisplay(this->d);
}



App::XVisualInfoWrapper::XVisualInfoWrapper(const WindowParams& wp, XDisplayWrapper& xDisplay){
	//TODO: allow configuring depth, stencil buffers via WindowParams
	int attr[] = {
		GLX_RGBA,
		GLX_DOUBLEBUFFER,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_DEPTH_SIZE, 24,
		None
	};

	this->vi = glXChooseVisual(
			xDisplay.d,
			DefaultScreen(xDisplay.d),
			attr
		);
	if(!this->vi){
		throw morda::Exc("glXChooseVisual() failed");
	}
}



App::XVisualInfoWrapper::~XVisualInfoWrapper()NOEXCEPT{
	XFree(this->vi);
}



App::XWindowWrapper::XWindowWrapper(const App::WindowParams& wp, XDisplayWrapper& xDisplay, XVisualInfoWrapper& xVisualInfo) :
		d(xDisplay)
{
	Colormap colorMap = XCreateColormap(
			this->d.d,
			RootWindow(this->d.d, xVisualInfo.vi->screen),
			xVisualInfo.vi->visual,
			AllocNone
		);
	//TODO: check for error

	XSetWindowAttributes attr;
	attr.colormap = colorMap;
	attr.border_pixel = 0;
	attr.event_mask =
			ExposureMask |
			KeyPressMask |
			KeyReleaseMask |
			ButtonPressMask |
			ButtonReleaseMask |
			PointerMotionMask |
			ButtonMotionMask |
			StructureNotifyMask |
			EnterWindowMask |
			LeaveWindowMask
		;

	this->w = XCreateWindow(
			this->d.d,
			RootWindow(this->d.d, xVisualInfo.vi->screen),
			0,
			0,
			wp.dim.x,
			wp.dim.y,
			0,
			xVisualInfo.vi->depth,
			InputOutput,
			xVisualInfo.vi->visual,
			CWBorderPixel | CWColormap | CWEventMask,
			&attr
		);
	//TODO: check for error
	
	{//We want to handle WM_DELETE_WINDOW event to know when window is closed.
		Atom a = XInternAtom(this->d.d, "WM_DELETE_WINDOW", True);
		XSetWMProtocols(this->d.d, this->w, &a, 1);
	}

	XMapWindow(this->d.d, this->w);
}



App::XWindowWrapper::~XWindowWrapper()NOEXCEPT{
	XDestroyWindow(this->d.d, this->w);
}



App::GLXContextWrapper::GLXContextWrapper(XDisplayWrapper& xDisplay, XWindowWrapper& xWindow, XVisualInfoWrapper& xVisualInfo) :
		d(xDisplay),
		w(xWindow)
{
	this->glxContext = glXCreateContext(this->d.d, xVisualInfo.vi, 0, GL_TRUE);
	if(this->glxContext == NULL){
		throw morda::Exc("glXCreateContext() failed");
	}
	glXMakeCurrent(this->d.d, this->w.w, this->glxContext);
	
	TRACE(<< "OpenGL version: " << glGetString(GL_VERSION) << std::endl)
	
	if(glewInit() != GLEW_OK){
		this->Destroy();
		throw morda::Exc("GLEW initialization failed");
	}
}



void App::GLXContextWrapper::Destroy()NOEXCEPT{
//	TRACE(<< "App::GLXContextWrapper::Destroy(): invoked" << std::endl)
	glXMakeCurrent(this->d.d, None, NULL);
//	TRACE(<< "App::GLXContextWrapper::Destroy(): destroying context" << std::endl)
	glXDestroyContext(this->d.d, this->glxContext);
}



App::XInputMethodWrapper::XInputMethodWrapper(XDisplayWrapper& xDisplay, XWindowWrapper& xWindow) :
		d(xDisplay),
		w(xWindow)
{
	this->xim = XOpenIM(this->d.d, NULL, NULL, NULL);
	if(this->xim == NULL){
		throw morda::Exc("XOpenIM() failed");
	}
	
	this->xic = XCreateIC(
			this->xim,
			XNClientWindow, this->w.w,
			XNFocusWindow, this->w.w,
			XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
			NULL
		);
	if(this->xic == NULL){
		this->Destroy();
	}
}



void App::XInputMethodWrapper::Destroy()NOEXCEPT{
	if(this->xic != NULL){
		XUnsetICFocus(this->xic);
		XDestroyIC(this->xic);
	}
	if(this->xim != NULL){
		XCloseIM(this->xim);
	}
}


App::App(const WindowParams& requestedWindowParams) :
		xVisualInfo(requestedWindowParams, xDisplay),
		xWindow(requestedWindowParams, xDisplay, xVisualInfo),
		glxContex(xDisplay, xWindow, xVisualInfo),
		xInputMethod(xDisplay, xWindow),
		curWinRect(0, 0, -1, -1)
{
	//initialize screen density
	{
		int scrNum = 0;
		this->dotsPerCm = ((double(DisplayWidth(this->xDisplay.d, scrNum)) / (double(DisplayWidthMM(this->xDisplay.d, scrNum))/ 10.0))
				+ (double(DisplayHeight(this->xDisplay.d, scrNum)) / (double(DisplayHeightMM(this->xDisplay.d, scrNum)) / 10.0))) / 2;
	}
	
#ifdef DEBUG
	//print GLX version
	{
		int major, minor;
		glXQueryVersion(this->xDisplay.d, &major, &minor);
		TRACE(<< "GLX Version: " << major << "." << minor << std::endl)
	}
#endif
	
	this->UpdateWindowRect(
			morda::Rect2r(
					0,
					0,
					float(requestedWindowParams.dim.x),
					float(requestedWindowParams.dim.y)
				)
		);
	
	this->MountDefaultResPack();
}



namespace{

class XEventWaitable : public ting::Waitable{
	int fd;
	
	int GetHandle() override{
		return this->fd;
	}
public:
	XEventWaitable(Display* d){
		this->fd = XConnectionNumber(d);
	}
	
	inline void ClearCanReadFlag(){
		this->ting::Waitable::ClearCanReadFlag();
	}
};

Widget::EMouseButton ButtonNumberToEnum(int number){
	switch(number){
		case 1:
			return Widget::EMouseButton::LEFT;
		default:
		case 2:
			return Widget::EMouseButton::MIDDLE;
		case 3:
			return Widget::EMouseButton::RIGHT;
		case 4:
			return Widget::EMouseButton::WHEEL_UP;
		case 5:
			return Widget::EMouseButton::WHEEL_DOWN;
	}
}



const std::array<EKey, std::uint8_t(-1) + 1> keyCodeMap = {
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::ESCAPE,//9
	EKey::ONE,//10
	EKey::TWO,//11
	EKey::THREE,//12
	EKey::FOUR,//13
	EKey::FIVE,//14
	EKey::SIX,//15
	EKey::SEVEN,//16
	EKey::EIGHT,//17
	EKey::NINE,//18
	EKey::ZERO,//19
	EKey::MINUS,//20
	EKey::EQUALS,//21
	EKey::BACKSPACE,//22
	EKey::TAB,//23
	EKey::Q,//24
	EKey::W,//25
	EKey::E,//26
	EKey::R,//27
	EKey::T,//28
	EKey::Y,//29
	EKey::U,//30
	EKey::I,//31
	EKey::O,//32
	EKey::P,//33
	EKey::LEFT_SQUARE_BRACKET,//34
	EKey::RIGHT_SQUARE_BRACKET,//35
	EKey::ENTER,//36
	EKey::LEFT_CONTROL,//37
	EKey::A,//38
	EKey::S,//39
	EKey::D,//40
	EKey::F,//41
	EKey::G,//42
	EKey::H,//43
	EKey::J,//44
	EKey::K,//45
	EKey::L,//46
	EKey::SEMICOLON,//47
	EKey::APOSTROPHE,//48
	EKey::GRAVE,//49
	EKey::LEFT_SHIFT,//50
	EKey::BACKSLASH,//51
	EKey::Z,//52
	EKey::X,//53
	EKey::C,//54
	EKey::V,//55
	EKey::B,//56
	EKey::N,//57
	EKey::M,//58
	EKey::COMMA,//59
	EKey::PERIOD,//60
	EKey::SLASH,//61
	EKey::RIGHT_SHIFT,//62
	EKey::UNKNOWN,
	EKey::LEFT_ALT,//64
	EKey::SPACE,//65
	EKey::CAPSLOCK,//66
	EKey::F1,//67
	EKey::F2,//68
	EKey::F3,//69
	EKey::F4,//70
	EKey::F5,//71
	EKey::F6,//72
	EKey::F7,//73
	EKey::F8,//74
	EKey::F9,//75
	EKey::F10,//76
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::F11,//95
	EKey::F12,//96
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::RIGHT_CONTROL,//105
	EKey::UNKNOWN,
	EKey::PRINT_SCREEN,//107
	EKey::RIGHT_ALT,//108
	EKey::UNKNOWN,
	EKey::HOME,//110
	EKey::UP,//111
	EKey::PAGE_UP,//112
	EKey::LEFT,//113
	EKey::RIGHT,//114
	EKey::END,//115
	EKey::DOWN,//116
	EKey::PAGE_DOWN,//117
	EKey::INSERT,//118
	EKey::DELETE,//119
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::PAUSE,//127
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::WINDOWS,//133
	EKey::UNKNOWN,
	EKey::WINDOWS_MENU,//135
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN,
	EKey::UNKNOWN
};



class KeyEventUnicodeResolver{
	XIC& xic;
	XEvent& event;
public:
	KeyEventUnicodeResolver(XIC& xic, XEvent& event) :
			xic(xic),
			event(event)
	{}
	
	std::vector<std::uint32_t> Resolve()const{
#ifndef X_HAVE_UTF8_STRING
#	error "no Xutf8stringlookup()"
#endif
		
		Status status;
		//KeySym xkeysym;
		std::array<char, 32> staticBuf;
		std::vector<char> arr;
		ting::Buffer<char> buf = staticBuf;

		int size = Xutf8LookupString(this->xic, &this->event.xkey, buf.begin(), buf.size() - 1, NULL, &status);
		if(status == XBufferOverflow){
			//allocate enough memory
			arr.resize(size + 1);
			buf = arr;
			size = Xutf8LookupString(this->xic, &this->event.xkey, buf.begin(), buf.size() - 1, NULL, &status);
		}
		ASSERT(size >= 0)
		ASSERT(buf.size() != 0)
		ASSERT(buf.size() > unsigned(size))
		
//		TRACE(<< "KeyEventUnicodeResolver::Resolve(): size = " << size << std::endl)
		
		buf[size] = 0;//null-terminate
		
		switch(status){
			case XLookupChars:
			case XLookupBoth:
				if(size == 0){
					return std::vector<std::uint32_t>();
				}
				
				{
					typedef std::vector<std::uint32_t> T_Vector;
					T_Vector utf32;
					
					for(ting::utf8::Iterator i(buf.begin()); i.IsNotEnd(); ++i){
						utf32.push_back(i.Char());
					}
					
					std::vector<std::uint32_t> ret(utf32.size());
					
					std::uint32_t* dst = &*ret.begin();
					for(T_Vector::iterator src = utf32.begin(); src != utf32.end(); ++src, ++dst){
						*dst = *src;
					}
					
					return ret;
				}
				
				break;
			default:
			case XBufferOverflow:
				ASSERT(false)
			case XLookupKeySym:
			case XLookupNone:
				break;
		}//~switch
		
		return std::vector<std::uint32_t>();
	}
};



}//~namespace



void App::Exec(){
	XEventWaitable xew(this->xDisplay.d);
	
	ting::WaitSet waitSet(2);
	
	waitSet.Add(xew, ting::Waitable::READ);
	waitSet.Add(this->uiQueue, ting::Waitable::READ);
	
	//Sometimes the first Expose event does not come for some reason. It happens constantly in some systems and never happens on all the others.
	//So, render everything for the first time.
	this->Render();
	
	while(!this->quitFlag){
		waitSet.WaitWithTimeout(this->updater.Update());
		
		if(this->uiQueue.CanRead()){
			while(auto m = this->uiQueue.PeekMsg()){
				m();
			}
			ASSERT(!this->uiQueue.CanRead())
		}
		
		if(xew.CanRead()){
			xew.ClearCanReadFlag();
			while(XPending(this->xDisplay.d) > 0){
				XEvent event;
				XNextEvent(this->xDisplay.d, &event);
//				TRACE(<< "X event got, type = " << (event.type) << std::endl)
				switch(event.type){
					case Expose:
//						TRACE(<< "Expose X event got" << std::endl)
						if(event.xexpose.count != 0){
							break;//~switch()
						}
						this->Render();
						break;
					case ConfigureNotify:
//						TRACE(<< "ConfigureNotify X event got" << std::endl)
						this->UpdateWindowRect(morda::Rect2r(0, 0, float(event.xconfigure.width), float(event.xconfigure.height)));
						break;
					case KeyPress:
//						TRACE(<< "KeyPress X event got" << std::endl)
						{
							EKey key = keyCodeMap[std::uint8_t(event.xkey.keycode)];
							this->HandleKeyEvent(true, key);
							this->HandleCharacterInput(KeyEventUnicodeResolver(this->xInputMethod.xic, event), key);
						}
						break;
					case KeyRelease:
//						TRACE(<< "KeyRelease X event got" << std::endl)
						{
							EKey key = keyCodeMap[std::uint8_t(event.xkey.keycode)];

							//detect auto-repeated key events
							if(XEventsQueued(this->xDisplay.d, QueuedAfterReading)){//if there are other events queued
								XEvent nev;
								XPeekEvent(this->xDisplay.d, &nev);

								if(nev.type == KeyPress
										&& nev.xkey.time == event.xkey.time
										&& nev.xkey.keycode == event.xkey.keycode
									)
								{
									//Key wasn't actually released
									this->HandleCharacterInput(KeyEventUnicodeResolver(this->xInputMethod.xic, nev), key);

									XNextEvent(this->xDisplay.d, &nev);//remove the key down event from queue
									break;
								}
							}

							this->HandleKeyEvent(false, key);
						}
						break;
					case ButtonPress:
//						TRACE(<< "ButtonPress X event got, button mask = " << event.xbutton.button << std::endl)
//						TRACE(<< "ButtonPress X event got, x, y = " << event.xbutton.x << ", " << event.xbutton.y << std::endl)
						this->HandleMouseButton(
								true,
								morda::Vec2r(event.xbutton.x, event.xbutton.y),
								ButtonNumberToEnum(event.xbutton.button),
								0
							);
						break;
					case ButtonRelease:
						this->HandleMouseButton(
								false,
								morda::Vec2r(event.xbutton.x, event.xbutton.y),
								ButtonNumberToEnum(event.xbutton.button),
								0
							);
						break;
					case MotionNotify:
//						TRACE(<< "MotionNotify X event got" << std::endl)
						this->HandleMouseMove(
								morda::Vec2r(event.xmotion.x, event.xmotion.y),
								0
							);
						break;
					case EnterNotify:
						this->HandleMouseHover(true);
						break;
					case LeaveNotify:
						this->HandleMouseHover(false);
						break;
					case ClientMessage:
//						TRACE(<< "ClientMessage X event got" << std::endl)
						//probably a WM_DELETE_WINDOW event
						{
							char* name = XGetAtomName(this->xDisplay.d, event.xclient.message_type);
							if(*name == *"WM_PROTOCOLS"){
								this->quitFlag = true;
							}
							XFree(name);
						}
						break;
					default:
						//ignore
						break;
				}//~switch()
			}//~while()
		}//~if there are pending X events
		
		this->Render();
	}//~while(!this->quitFlag)
	
	waitSet.Remove(this->uiQueue);
	waitSet.Remove(xew);
}



namespace morda{

inline void Main(int argc, const char** argv){
	std::unique_ptr<morda::App> app = morda::CreateApp(argc, argv, ting::Buffer<std::uint8_t>(0, 0));

	app->Exec();
}

}//~namespace



int main(int argc, const char** argv){
	morda::Main(argc, argv);

	return 0;
}
