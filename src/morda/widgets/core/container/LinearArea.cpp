#include "LinearArea.hpp"

#include "../../../util/util.hpp"

#include <cmath>



using namespace morda;



LinearArea::LayoutParams::LayoutParams(const stob::Node* chain) :
		Container::LayoutParams(chain)
{
	if(auto n = getProperty(chain, "weight")){
		this->weight = n->asFloat();
	}else{
		this->weight = 0;
	}
}




LinearArea::LinearArea(bool isVertical, const stob::Node* chain) :
		Widget(chain),
		Container(chain),
		isVertical_v(isVertical)
{}



namespace{

class Info{
public:
	Vec2r measuredDim;
};

}



void LinearArea::layOut(){
	unsigned longIndex = this->GetLongIndex();
	unsigned transIndex = this->GetTransIndex();
	
	std::vector<Info> infoArray(this->children().size());
	
	//Calculate rigid size, net weight and store weights
	real rigid = 0;
	real netWeight = 0;
	
	{
		auto info = infoArray.begin();
		for(auto i = this->children().cbegin(); i != this->children().cend(); ++i, ++info){
			auto& lp = this->getLayoutParamsDuringLayoutAs<LayoutParams>(**i);
			
			netWeight += lp.weight;
			
			ASSERT(lp.dim[longIndex] != LayoutParams::max_c)
			ASSERT(lp.dim[longIndex] != LayoutParams::fill_c)
			
			Vec2r d = this->dimForWidget(**i, lp);
			info->measuredDim = d;
			
			rigid += d[longIndex];
		}
	}
	
	//arrange widgets
	{
		real flexible = this->rect().d[longIndex] - rigid;
		
		real pos = 0;
		
		auto info = infoArray.begin();
		for(auto i = this->children().begin(); i != this->children().end(); ++i, ++info){
			auto& lp = this->getLayoutParamsDuringLayoutAs<LayoutParams>(**i);
			
			if(lp.weight != 0){
				ASSERT(lp.weight > 0)
				Vec2r d;
				d[longIndex] = info->measuredDim[longIndex];
				if(flexible > 0){
					ASSERT(netWeight > 0)
					d[longIndex] += flexible * lp.weight / netWeight;
				}
				
				if(lp.dim[transIndex] == LayoutParams::max_c || lp.dim[transIndex] == LayoutParams::fill_c){
					d[transIndex] = this->rect().d[transIndex];
				}else{
					if(lp.dim[transIndex] == LayoutParams::min_c){
						d[transIndex] = -1;
					}else{
						d[transIndex] = lp.dim[transIndex];
					}
					if(d.x < 0 || d.y < 0){
						Vec2r md = (*i)->measure(d);
						for(unsigned i = 0; i != md.size(); ++i){
							if(d[i] < 0){
								d[i] = md[i];
							}
						}
					}
				}
				(*i)->resize(d.rounded());
			}else{
				(*i)->resize(info->measuredDim);
			}
			
			Vec2r newPos;

			if(this->isVertical_v){
				newPos[longIndex] = this->rect().d[longIndex] - pos - (*i)->rect().d[longIndex];
			}else{
				newPos[longIndex] = pos;
			}
			
			pos += (*i)->rect().d[longIndex];
			
			newPos[transIndex] = std::round((this->rect().d[transIndex] - (*i)->rect().d[transIndex]) / 2);
			
			(*i)->moveTo(newPos);
		}
	}
}



morda::Vec2r LinearArea::measure(const morda::Vec2r& quotum)const{
	unsigned longIndex = this->GetLongIndex();
	unsigned transIndex = this->GetTransIndex();
	
	std::vector<Info> infoArray(this->children().size());
	
	//calculate rigid length
	real rigidLength = 0;
	real height = quotum[transIndex] >= 0 ? quotum[transIndex] : 0;
	real netWeight = 0;
	
	{
		auto info = infoArray.begin();
		for(auto i = this->children().begin(); i != this->children().end(); ++i, ++info){
			auto& lp = this->getLayoutParamsDuringLayoutAs<LayoutParams>(**i);

			netWeight += lp.weight;

			if(lp.dim[longIndex] == LayoutParams::max_c || lp.dim[longIndex] == LayoutParams::fill_c){
				throw morda::Exc("LinearContainer::onMeasure(): mistake: 'max' or 'fill' in longitudional direction");
			}

			Vec2r d;
			if(lp.dim[transIndex] == LayoutParams::max_c){
				if(quotum[transIndex] >= 0){
					d[transIndex] = quotum[transIndex];
				}else{
					d[transIndex] = -1;
				}
			}else if(lp.dim[transIndex] == LayoutParams::min_c){
				d[transIndex] = -1;
			}else if(lp.dim[transIndex] == LayoutParams::fill_c){
				if(quotum[transIndex] >= 0){
					d[transIndex] = quotum[transIndex];
				}else{
					d[transIndex] = 0;
				}
			}else{
				d[transIndex] = lp.dim[transIndex];
			}

			ASSERT(lp.dim[longIndex] != LayoutParams::max_c)
			ASSERT(lp.dim[longIndex] != LayoutParams::fill_c)
			if(lp.dim[longIndex] == LayoutParams::min_c){
				d[longIndex] = -1;
			}else{
				d[longIndex] = lp.dim[longIndex];
			}

			d = (*i)->measure(d);
			info->measuredDim = d;

			rigidLength += d[longIndex];

			if(lp.weight == 0){
				if(quotum[transIndex] < 0){
					utki::clampBottom(height, d[transIndex]);
				}
			}
		}
	}
	
	Vec2r ret;
	
	real flexLen;
	
	if(quotum[longIndex] < 0){
		ret[longIndex] = rigidLength;
		flexLen = 0;
	}else{
		ret[longIndex] = quotum[longIndex];
		flexLen = quotum[longIndex] - rigidLength;
	}
	
	{
		auto info = infoArray.begin();
		for(auto i = this->children().begin(); i != this->children().end(); ++i, ++info){
			auto& lp = this->getLayoutParamsDuringLayoutAs<LayoutParams>(**i);
			ASSERT(lp.weight >= 0)
			if(lp.weight == 0){
				continue;
			}

			ASSERT(netWeight > 0)

			Vec2r d;
			d[longIndex] = info->measuredDim[longIndex];
			
			if(flexLen > 0){
				d[longIndex] += flexLen * lp.weight / netWeight;
			}
			
			if(lp.dim[transIndex] == LayoutParams::max_c){
				if(quotum[transIndex] >= 0){
					d[transIndex] = quotum[transIndex];
				}else{
					d[transIndex] = -1;
				}
			}else if(lp.dim[transIndex] == LayoutParams::min_c){
				d[transIndex] = -1;
			}else if(lp.dim[transIndex] == LayoutParams::fill_c){
				if(quotum[transIndex] >= 0){
					d[transIndex] = quotum[transIndex];
				}else{
					d[transIndex] = 0;
				}
			}else{
				d[transIndex] = lp.dim[transIndex];
			}
			
			d = (*i)->measure(d);
			if(quotum[transIndex] < 0){
				utki::clampBottom(height, d[transIndex]);
			}
		}
	}
	
	ret[transIndex] = height;
	return ret;
}


