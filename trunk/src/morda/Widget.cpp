#include "Widget.hpp"

#include "Container.hpp"
#include "util/util.hpp"

using namespace morda;



void Widget::ApplyDescription(const stob::Node& description){
	if(const stob::Node* p = description.Child("prop").second){
		this->prop = p->Clone();
		this->prop->SetValue();//clear value of the prop node, we don't need it
	}

	if(const stob::Node* p = description.GetProperty("pos")){
		this->rect.p = morda::Vec2fFromSTOB(p);
	}else{
		this->rect.p.SetToZero();
	}

	if(const stob::Node* p = description.GetProperty("dim")){
		this->rect.d = morda::Vec2fFromSTOB(p);
	}

	if(const stob::Node* p = description.GetProperty("name")){
		this->name = p->Value();
	}
}



void Widget::RemoveFromParent(){
	if(ting::Ref<Container> p = this->parent){
#ifdef DEBUG
		bool res =
#endif
		p->Remove(ting::Ref<Widget>(this));
		ASSERT(res)
	}
}
