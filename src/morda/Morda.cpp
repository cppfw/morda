#include "Morda.hpp"

#include "resources/ResSTOB.hpp"

#include "widgets/slider/Slider.hpp"

#include "widgets/button/SimpleButton.hpp"
#include "widgets/button/CheckBox.hpp"
#include "widgets/button/RadioButton.hpp"
#include "widgets/button/ChoiceGroup.hpp"

#include "widgets/label/ColorLabel.hpp"
#include "widgets/label/TextLabel.hpp"
#include "widgets/label/GreyscaleGlass.hpp"
#include "widgets/label/BlurGlass.hpp"

#include "widgets/TextField.hpp"
#include "widgets/List.hpp"
#include "widgets/TreeView.hpp"
#include "widgets/DropDownSelector.hpp"
#include "widgets/Window.hpp"
#include "widgets/MouseCursor.hpp"

using namespace morda;


Morda::T_Instance Morda::instance;



void Morda::initStandardWidgets(papki::File& fi) {
	
	//mount default resource pack
	
	std::vector<std::string> paths;

#if M_OS == M_OS_WINDOWS
#	ifdef DEBUG
	paths.push_back("../../morda_res/");
#	else
	paths.push_back("morda_res/");
#	endif
#elif M_OS_NAME == M_OS_NAME_IOS || M_OS_NAME == M_OS_NAME_ANDROID
	paths.push_back("morda_res/");
#else //linux or macosx
#	ifdef DEBUG
	paths.push_back("../../morda_res/");
#	else
	
	unsigned soname =
#		include "../soname.txt"
	;
	
	{
		std::stringstream ss;
		ss << "/usr/local/share/morda/res" << soname << "/";
		paths.push_back(ss.str());
	}
	{
		std::stringstream ss;
		ss << "/usr/share/morda/res" << soname << "/";
		paths.push_back(ss.str());
	}
#	endif
#endif

	bool mounted = false;
	for(const auto& s : paths){
		try{
//			TRACE(<< "s = " << s << std::endl)
			fi.setPath(s);
//			TRACE(<< "fi.path() = " << fi.path() << std::endl)
			this->resMan.mountResPack(fi);
		}catch(papki::Exc&){
			continue;
		}
		mounted = true;
		break;
	}

	if(!mounted){
		throw morda::Exc("Morda::initStandardWidgets(): could not mount default resource pack");
	}
	
	//add standard widgets to inflater
	
	this->inflater.addWidget<TextLabel>("TextLabel");
	this->inflater.addWidget<TextInput>("TextInput");
	this->inflater.addWidget<VerticalSlider>("VerticalSlider");
	this->inflater.addWidget<HorizontalSlider>("HorizontalSlider");
	this->inflater.addWidget<ImageLabel>("ImageLabel");
	this->inflater.addWidget<Window>("Window");
	this->inflater.addWidget<NinePatch>("NinePatch");
	this->inflater.addWidget<SimpleButton>("SimpleButton");
	this->inflater.addWidget<ColorLabel>("ColorLabel");
	this->inflater.addWidget<TextField>("TextField");
	this->inflater.addWidget<CheckBox>("CheckBox");
	this->inflater.addWidget<GreyscaleGlass>("GreyscaleGlass");
	this->inflater.addWidget<BlurGlass>("BlurGlass");
	this->inflater.addWidget<HorizontalList>("HorizontalList");
	this->inflater.addWidget<VerticalList>("VerticalList");
	this->inflater.addWidget<TreeView>("TreeView");
	this->inflater.addWidget<DropDownSelector>("DropDownSelector");
	this->inflater.addWidget<RadioButton>("RadioButton");
	this->inflater.addWidget<ChoiceGroup>("ChoiceGroup");
	this->inflater.addWidget<MouseCursor>("MouseCursor");
	
	
	try{
		auto t = morda::Morda::inst().resMan.load<ResSTOB>("morda_gui_defs");
		
		if(t->chain()){
			this->inflater.inflate(*t->chain());
		}
		
	}catch(ResourceManager::Exc&){
		//ignore
		TRACE(<< "Morda::initStandardWidgets(): could not load morda_gui_definitions" << std::endl)
	}
}

void Morda::setViewportSize(const morda::Vec2r& size){
	this->viewportSize = size;
	
	if(!this->rootWidget){
		return;
	}
	
	this->rootWidget->resize(this->viewportSize);
}



void Morda::setRootWidget(const std::shared_ptr<morda::Widget>& w){
	this->rootWidget = w;

	this->rootWidget->moveTo(morda::Vec2r(0));
	this->rootWidget->resize(this->viewportSize);
}

void Morda::render(const Matr4r& matrix)const{
	if(!this->rootWidget){
		TRACE(<< "Morda::render(): root widget is not set" << std::endl)
		return;
	}
	
	Render::setCullEnabled(true);
	
	morda::Matr4r m(matrix);
	m.translate(-1, -1);
	m.scale(Vec2r(2.0f).compDivBy(this->viewportSize));
	
	ASSERT(this->rootWidget)
	
	if(this->rootWidget->needsRelayout()){
		TRACE(<< "root widget re-layout needed!" << std::endl)
		this->rootWidget->relayoutNeeded = false;
		this->rootWidget->layOut();
	}
	
	this->rootWidget->renderInternal(m);
}



void Morda::onMouseMove(const morda::Vec2r& pos, unsigned id){
	if(!this->rootWidget){
		return;
	}
	
	if(this->rootWidget->isInteractive()){
		this->rootWidget->setHovered(this->rootWidget->rect().overlaps(pos), id);
		this->rootWidget->onMouseMove(pos, id);
	}
}



void Morda::onMouseButton(bool isDown, const morda::Vec2r& pos, Widget::MouseButton_e button, unsigned pointerID){
	if(!this->rootWidget){
		return;
	}

	if(this->rootWidget->isInteractive()){
		this->rootWidget->setHovered(this->rootWidget->rect().overlaps(pos), pointerID);
		this->rootWidget->onMouseButton(isDown, pos, button, pointerID);
	}
}



void Morda::onMouseHover(bool isHovered, unsigned pointerID){
	if(!this->rootWidget){
		return;
	}
	
	this->rootWidget->setHovered(isHovered, pointerID);
}

void Morda::onKeyEvent(bool isDown, Key_e keyCode){
//		TRACE(<< "HandleKeyEvent(): is_down = " << is_down << " is_char_input_only = " << is_char_input_only << " keyCode = " << unsigned(keyCode) << std::endl)

	if(auto w = this->focusedWidget.lock()){
//		TRACE(<< "HandleKeyEvent(): there is a focused widget" << std::endl)
		w->onKeyInternal(isDown, keyCode);
	}else{
//		TRACE(<< "HandleKeyEvent(): there is no focused widget, passing to rootWidget" << std::endl)
		if(this->rootWidget){
			this->rootWidget->onKeyInternal(isDown, keyCode);
		}
	}
}


void Morda::setFocusedWidget(const std::shared_ptr<Widget> w){
	if(auto prev = this->focusedWidget.lock()){
		prev->isFocused_v = false;
		prev->onFocusChanged();
	}
	
	this->focusedWidget = w;
	
	if(w){
		w->isFocused_v = true;
		w->onFocusChanged();
	}
}

void Morda::onCharacterInput(const UnicodeProvider& unicode, Key_e key){
	if(auto w = this->focusedWidget.lock()){
		//			TRACE(<< "HandleCharacterInput(): there is a focused widget" << std::endl)
		if(auto c = dynamic_cast<CharInputWidget*>(w.operator->())){
			c->onCharacterInput(unicode.get(), key);
		}
	}
}
