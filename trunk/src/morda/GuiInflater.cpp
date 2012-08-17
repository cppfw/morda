#include "GuiInflater.hpp"
#include "Container.hpp"
#include "widgets/Label.hpp"
#include "widgets/Button.hpp"



using namespace morda;



namespace{

class WidgetFactory : public GuiInflater::WidgetFactory{
public:
	//override
	ting::Ref<morda::Widget> Create(ting::Ptr<stob::Node> node)const{
		ASSERT(node->Value() == "Widget")
		return Widget::New(node);
	}
	
	inline static ting::Ptr<WidgetFactory> New(){
		return ting::Ptr<WidgetFactory>(new WidgetFactory());
	}
};

class ContainerFactory : public GuiInflater::WidgetFactory{
public:
	//override
	ting::Ref<morda::Widget> Create(ting::Ptr<stob::Node> node)const{
		ASSERT(node->Value() == "Container")
		return Container::New(node);
	}
	
	inline static ting::Ptr<ContainerFactory> New(){
		return ting::Ptr<ContainerFactory>(new ContainerFactory());
	}
};

class LabelFactory : public GuiInflater::WidgetFactory{
public:
	//override
	ting::Ref<morda::Widget> Create(ting::Ptr<stob::Node> node)const{
		ASSERT(node->Value() == "Label")
		return Label::New(node);
	}
	
	inline static ting::Ptr<LabelFactory> New(){
		return ting::Ptr<LabelFactory>(new LabelFactory());
	}
};

class AbstractButtonFactory : public GuiInflater::WidgetFactory{
public:
	//override
	ting::Ref<morda::Widget> Create(ting::Ptr<stob::Node> node)const{
		ASSERT(node->Value() == "AbstractButton")
		return AbstractButton::New(node);
	}
	
	inline static ting::Ptr<AbstractButtonFactory> New(){
		return ting::Ptr<AbstractButtonFactory>(new AbstractButtonFactory());
	}
};

class ButtonFactory : public GuiInflater::WidgetFactory{
public:
	//override
	ting::Ref<morda::Widget> Create(ting::Ptr<stob::Node> node)const{
		ASSERT(node->Value() == "Button")
		return Button::New(node);
	}
	
	inline static ting::Ptr<ButtonFactory> New(){
		return ting::Ptr<ButtonFactory>(new ButtonFactory());
	}
};

}//~namespace



GuiInflater::GuiInflater(){
	this->AddWidgetFactory("Widget", ::WidgetFactory::New());
	this->AddWidgetFactory("Container", ContainerFactory::New());
	this->AddWidgetFactory("Label", LabelFactory::New());
	this->AddWidgetFactory("AbstractButton", AbstractButtonFactory::New());
	this->AddWidgetFactory("Button", ButtonFactory::New());
	//TODO: add default factories
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



ting::Ref<morda::Widget> GuiInflater::Inflate(ting::fs::File& fi)const{
	ting::Ptr<stob::Node> root = stob::Load(fi);
	root->SetValue("Container");
	
	return this->Inflate(root);
}



ting::Ref<morda::Widget> GuiInflater::Inflate(ting::Ptr<stob::Node> gui)const{
	if(gui.IsNotValid()){
		throw GuiInflater::Exc("Failed to inflate, passed pointer to GUI STOB hierarchy is not valid");
	}
	
	T_FactoryMap::const_iterator i = this->widgetFactories.find(gui->Value());
	
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
