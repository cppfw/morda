#include "List.hpp"

#include "../App.hpp"


using namespace morda;




namespace{

class StaticProvider : public List::ItemsProvider{
	std::vector<std::shared_ptr<Widget>> widgets;
public:

	size_t count() const NOEXCEPT override{
		return this->widgets.size();
	}
	
	std::shared_ptr<Widget> getWidget(size_t index)override{
		return this->widgets[index];
	}
	
	void add(std::shared_ptr<Widget> w){
		this->widgets.push_back(std::move(w));
	}
};

}




List::List(bool isVertical, const stob::Node* chain):
		Widget(chain),
		isVertical(isVertical)
{
	if(!chain){
		return;
	}
	
	const stob::Node* n = chain->ThisOrNextNonProperty().node();
	
	if(!n){
		return;
	}
	
	std::shared_ptr<StaticProvider> p = ting::New<StaticProvider>();
	
	for(; n; n = n->NextNonProperty().node()){
		p->add(morda::App::Inst().inflater.Inflate(*n));
	}
	
	this->setItemsProvider(std::move(p));
}



void List::OnResize() {
	//TODO:
	
	this->Container::OnResize();
}


void List::setItemsProvider(std::shared_ptr<ItemsProvider> provider){
	if(this->provider){
		this->provider->list = nullptr;
	}
	this->provider = std::move(provider);
	if(this->provider){
		this->provider->list = this;
	}
	this->notifyDataSetChanged();
}



real List::scrollFactor()const NOEXCEPT{
	//TODO:
	return 0;
}


void List::setScrollPosAsFactor(real factor){
	//TODO:
}
