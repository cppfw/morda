#include "Tab.hpp"
#include "../../util/util.hpp"


using namespace morda;

bool Tab::maskOverlaps(Vec2r pos) {
	auto borders = this->getActualBorders();

	if(pos.x < borders.left()){
		if(pos.y != 0 && this->rect().d.y != 0){
			if(pos.x / (this->rect().d.y - pos.y) < borders.left() / this->rect().d.y){
				return false;
			}
		}

		if(this->parent() && this->parentIter() != this->parent()->children().begin()){ //if this is not the first widget in the parent
			auto prevIter = this->parentIter();
			ASSERT(prevIter != this->parent()->children().end())
			--prevIter;

			if(auto pt = dynamic_cast<Tab*>(prevIter->get())){ //previous tab
				if(pt->isPressed()){
					if(pt->maskOverlaps(pos + this->rect().p - pt->rect().p)){
						return false;
					}
				}
			}
		}
	}else{
		auto rb = this->rect().d.x - borders.right();
		auto dx = pos.x - rb;
		if(dx > 0){
			if(pos.y != 0 && this->rect().d.y != 0){
				if(dx / pos.y > borders.right() / this->rect().d.y){
					return false;
				}
			}
		}
	}
	return true;
}


bool Tab::onMouseButton(bool isDown, const morda::Vec2r& pos, MouseButton_e button, unsigned pointerId) {
	if(isDown){
		if(!maskOverlaps(pos)){
			return false;
		}
	}
	
	return this->ChoiceButton::onMouseButton(isDown, pos, button, pointerId);
}

void Tab::onPressedChanged() {
	this->ChoiceButton::onPressedChanged();
	this->NinePatchToggle::onPressedChanged();
}


Tab::Tab(const stob::Node* chain) :
		Widget(chain),
		Button(chain),
		ToggleButton(chain),
		ChoiceButton(chain),
		NinePatchToggle(chain)
{
	if(!this->pressedNinePatch()){
		this->setPressedNinePatch(morda::inst().resMan.load<ResNinePatch>("morda_npt_tab_active"));
	}
	if(!this->unpressedNinePatch()){
		this->setUnpressedNinePatch(morda::inst().resMan.load<ResNinePatch>("morda_npt_tab_inactive"));
	}
	
	this->onPressedChanged();//initialize nine-patch
}
