#include "LinearContainer.hpp"

#include "../../util/util.hpp"
#include "../../context.hpp"

#include <cmath>

using namespace morda;

LinearContainer::layout_params::layout_params(const puu::forest& desc, const morda::units& units) :
		Container::layout_params(desc, units)
{
	for(const auto& p : desc){
		if(!is_property(p)){
			continue;
		}

		if(p.value == "weight"){
			this->weight = get_property_value(p).to_float();
		}
	}
}

std::unique_ptr<widget::layout_params> LinearContainer::create_layout_params(const puu::forest& desc)const{
	return utki::make_unique<LayoutParams>(desc, this->context->units);
}

LinearContainer::LinearContainer(std::shared_ptr<morda::context> c, const puu::forest& desc, bool vertical) :
		widget(std::move(c), desc),
		container(this->context, desc),
		oriented_widget(this->context, puu::forest(), vertical)
{}

namespace{
class Info{
public:
	Vec2r measuredDim;
};
}

void LinearContainer::lay_out(){
	unsigned longIndex = this->get_long_index();
	unsigned transIndex = this->get_trans_index();

	std::vector<Info> infoArray(this->children().size());

	// calculate rigid size, net weight and store weights
	real rigid = 0;
	real netWeight = 0;

	{
		auto info = infoArray.begin();
		for(auto i = this->children().cbegin(); i != this->children().cend(); ++i, ++info){
			auto& lp = this->getLayoutParamsAs<LayoutParams>(**i);

			netWeight += lp.weight;

			ASSERT(lp.dim[longIndex] != LayoutParams::max_c)
			ASSERT(lp.dim[longIndex] != LayoutParams::fill_c)

			Vec2r d = this->dimForWidget(**i, lp);
			info->measuredDim = d;

			rigid += d[longIndex];
		}
	}

	// arrange widgets
	{
		real flexible = this->rect().d[longIndex] - rigid;

		real pos = 0;

		real remainder = 0;

		auto info = infoArray.begin();
		for(auto i = this->children().begin(); i != this->children().end(); ++i, ++info){
			auto& lp = this->getLayoutParamsAs<LayoutParams>(**i);

			if(lp.weight != 0){
				ASSERT(lp.weight > 0)
				Vec2r d;
				d[longIndex] = info->measuredDim[longIndex];
				if(flexible > 0){
					ASSERT(netWeight > 0)
					real dl = flexible * lp.weight / netWeight;
					real floored = std::floor(dl);
					ASSERT(dl >= floored)
					d[longIndex] += floored;
					remainder += (dl - floored);
					if(remainder >= real(1)){
						d[longIndex] += real(1);
						remainder -= real(1);
					}
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
				(*i)->resize(d);
			}else{
				(*i)->resize(info->measuredDim);
			}

			Vec2r newPos;

			newPos[longIndex] = pos;

			pos += (*i)->rect().d[longIndex];

			newPos[transIndex] = std::round((this->rect().d[transIndex] - (*i)->rect().d[transIndex]) / 2);

			(*i)->move_to(newPos);
		}

		if(remainder > 0){
			Vec2r d;
			d[transIndex] = 0;
			d[longIndex] = std::round(remainder);
			this->children().back()->resize_by(d);
			this->children().back()->move_by(-d);
		}
	}
}

morda::Vec2r LinearContainer::measure(const morda::Vec2r& quotum)const{
	unsigned longIndex = this->get_long_index();
	unsigned transIndex = this->get_trans_index();

	std::vector<Info> infoArray(this->children().size());

	// calculate rigid length
	real rigidLength = 0;
	real height = quotum[transIndex] >= 0 ? quotum[transIndex] : 0;
	real netWeight = 0;

	{
		auto info = infoArray.begin();
		for(auto i = this->children().begin(); i != this->children().end(); ++i, ++info){
			auto& lp = this->getLayoutParamsAs<LayoutParams>(**i);

			netWeight += lp.weight;

			if(lp.dim[longIndex] == LayoutParams::max_c || lp.dim[longIndex] == LayoutParams::fill_c){
				throw std::logic_error("LinearContainer::measure(): 'max' or 'fill' in longitudional direction specified in layout parameters");
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
		real remainder = 0;

		auto lastChild = this->children().size() != 0 ? this->children().back().get() : nullptr;

		auto info = infoArray.begin();
		for(auto i = this->children().begin(); i != this->children().end(); ++i, ++info){
			auto& lp = this->getLayoutParamsAs<LayoutParams>(**i);
			ASSERT(lp.weight >= 0)
			if(lp.weight == 0){
				continue;
			}

			ASSERT(netWeight > 0)

			Vec2r d;
			d[longIndex] = info->measuredDim[longIndex];

			if(flexLen > 0){
				real dl = flexLen * lp.weight / netWeight;
				real floored = std::floor(dl);
				ASSERT(dl >= floored)
				d[longIndex] += floored;
				remainder += (dl- floored);
				if(remainder >= real(1)){
					d[longIndex] += real(1);
					remainder -= real(1);
				}
				if((*i).get() == lastChild){
					if(remainder > 0){
						Vec2r correction;
						correction[transIndex] = 0;
						correction[longIndex] = std::round(remainder);
						d += correction;
					}
				}
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
