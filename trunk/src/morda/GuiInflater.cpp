#include "GuiInflater.hpp"

#include "Container.hpp"
#include "widgets/Label.hpp"
#include "widgets/Button.hpp"
#include "layouts/LinearLayout.hpp"
#include "layouts/FrameLayout.hpp"



using namespace morda;



namespace{

const char* D_Widget = "Widget";
const char* D_Container = "Container";
const char* D_Label = "Label";
const char* D_AbstractButton = "AbstractButton";
const char* D_Button = "Button";

const char* D_LinearLayout = "LinearLayout";
const char* D_FrameLayout = "FrameLayout";



class WidgetFactory : public GuiInflater::WidgetFactory{
public:
	//override
	ting::Ref<morda::Widget> Create(const stob::Node& node)const{
		ASSERT(node == D_Widget)
		return Widget::New(node, false);
	}
	
	inline static ting::Ptr<WidgetFactory> New(){
		return ting::Ptr<WidgetFactory>(new WidgetFactory());
	}
};

class ContainerFactory : public GuiInflater::WidgetFactory{
public:
	//override
	ting::Ref<morda::Widget> Create(const stob::Node& node)const{
		return ContainerFactory::Inflate(node);
	}
	
	inline static ting::Ref<morda::Container> Inflate(const stob::Node& node){
		ASSERT(node == D_Container)
		return Container::New(node, false);
	}
	
	inline static ting::Ptr<ContainerFactory> New(){
		return ting::Ptr<ContainerFactory>(new ContainerFactory());
	}
};

class LabelFactory : public GuiInflater::WidgetFactory{
public:
	//override
	ting::Ref<morda::Widget> Create(const stob::Node& node)const{
		ASSERT(node == D_Label)
		return Label::New(node, false);
	}
	
	inline static ting::Ptr<LabelFactory> New(){
		return ting::Ptr<LabelFactory>(new LabelFactory());
	}
};

class AbstractButtonFactory : public GuiInflater::WidgetFactory{
public:
	//override
	ting::Ref<morda::Widget> Create(const stob::Node& node)const{
		ASSERT(node == D_AbstractButton)
		return AbstractButton::New(node, false);
	}
	
	inline static ting::Ptr<AbstractButtonFactory> New(){
		return ting::Ptr<AbstractButtonFactory>(new AbstractButtonFactory());
	}
};

class ButtonFactory : public GuiInflater::WidgetFactory{
public:
	//override
	ting::Ref<morda::Widget> Create(const stob::Node& node)const{
		ASSERT(node == D_Button)
		return Button::New(node, false);
	}
	
	inline static ting::Ptr<ButtonFactory> New(){
		return ting::Ptr<ButtonFactory>(new ButtonFactory());
	}
};

class LinearLayoutFactory : public GuiInflater::LayoutFactory{
public:
	//override
	ting::Ptr<morda::Layout> Create(const stob::Node& node)const{
		ASSERT(node == D_LinearLayout)
		return LinearLayout::New(node);
	}
	
	inline static ting::Ptr<LinearLayoutFactory> New(){
		return ting::Ptr<LinearLayoutFactory>(new LinearLayoutFactory());
	}
};

class FrameLayoutFactory : public GuiInflater::LayoutFactory{
public:
	//override
	ting::Ptr<morda::Layout> Create(const stob::Node& node)const{
		ASSERT(node == D_FrameLayout)
		return FrameLayout::New(node);
	}
	
	inline static ting::Ptr<FrameLayoutFactory> New(){
		return ting::Ptr<FrameLayoutFactory>(new FrameLayoutFactory());
	}
};

}//~namespace



GuiInflater::GuiInflater(){
	this->AddWidgetFactory(D_Widget, ::WidgetFactory::New());
	this->AddWidgetFactory(D_Container, ContainerFactory::New());
	this->AddWidgetFactory(D_Label, LabelFactory::New());
	this->AddWidgetFactory(D_AbstractButton, AbstractButtonFactory::New());
	this->AddWidgetFactory(D_Button, ButtonFactory::New());
	
	this->AddLayoutFactory(D_LinearLayout, LinearLayoutFactory::New());
	this->AddLayoutFactory(D_FrameLayout, FrameLayoutFactory::New());
}



void GuiInflater::AddWidgetFactory(const std::string& widgetName, ting::Ptr<GuiInflater::WidgetFactory> factory){
	if(!factory){
		throw GuiInflater::Exc("Failed adding factory, passed factory pointer is not valid");
	}
	
	std::pair<T_FactoryMap::iterator, bool> ret = this->widgetFactories.insert(std::pair<std::string, ting::Ptr<GuiInflater::WidgetFactory> >(widgetName, factory));
	if(!ret.second){
		throw GuiInflater::Exc("Failed adding factory, factory with that widget name is already added");
	}
}



bool GuiInflater::RemoveWidgetFactory(const std::string& widgetName)throw(){
	if(this->widgetFactories.erase(widgetName) == 0){
		return false;
	}
	return true;
}



ting::Ref<morda::Container> GuiInflater::Inflate(ting::fs::File& fi)const{
	ting::Ptr<stob::Node> root = stob::Load(fi);
	ASSERT(root)
	root->SetValue(D_Container);
	
	return ContainerFactory::Inflate(*root);
}



ting::Ref<morda::Widget> GuiInflater::Inflate(const stob::Node& gui)const{
	T_FactoryMap::const_iterator i = this->widgetFactories.find(gui.Value());
	
	if(i == this->widgetFactories.end()){
		throw GuiInflater::Exc("Failed to inflate, no matching factory found for requested widget name");
	}
	
	return i->second->Create(gui);
}



void GuiInflater::AddLayoutFactory(const std::string& layoutName, ting::Ptr<LayoutFactory> factory){
	if(!factory){
		throw GuiInflater::Exc("Failed adding factory, passed factory pointer is not valid");
	}
	
	std::pair<T_LayoutMap::iterator, bool> ret = this->layoutFactories.insert(std::pair<std::string, ting::Ptr<GuiInflater::LayoutFactory> >(layoutName, factory));
	if(!ret.second){
		throw GuiInflater::Exc("Failed adding factory, factory with that layout name is already added");
	}
}



bool GuiInflater::RemoveLayoutFactory(const std::string& layoutName)throw(){
	if(this->layoutFactories.erase(layoutName) == 0){
		return false;
	}
	return true;
}



ting::Ptr<Layout> GuiInflater::CreateLayout(const stob::Node& layout)const{
	T_LayoutMap::const_iterator i = this->layoutFactories.find(layout.Value());
	
	if(i == this->layoutFactories.end()){
		throw GuiInflater::Exc("Failed to inflate, no matching factory found for requested layout name");
	}
	
	return i->second->Create(layout);
}
