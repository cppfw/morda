#include "click_drop_down_box.hpp"

#include "../group/overlay.hpp"

#include "../proxy/mouse_proxy.hpp"

#include "../label/color.hpp"

using namespace morda;

namespace{
const auto drop_down_box_layout = puu::read(R"qwertyuiop(
	layout{dx{max} dy{max}}

	@row{
		layout{dx{max}}
		@pile{
			id{morda_dropdown_selection}
			layout{dx{min}dy{max} weight{1}}
		}
		@widget{layout{dx{3dp}}}
		@image{
			image{morda_img_divider_vert}
			layout{dy{fill}}
		}
		@widget{layout{dx{3dp}}}
		@image{
			image{morda_img_dropdown_arrow}
		}
	}
)qwertyuiop");
}

namespace{
const auto item_layout = puu::read(R"qwertyuiop(
		@pile{
			layout{
				dx{max}
			}
			@mouse_proxy{
				id{morda_dropdown_mouseproxy}
				layout{
					dx{fill} dy{fill}
				}
			}
			@color{
				id{morda_dropdown_color}
				color{${morda_color_highlight}}
				visible{false}
				layout{
					dx{fill} dy{fill}
				}
			}
		}
	)qwertyuiop");
}

namespace{
const auto drop_down_menu_layout = puu::read(R"qwertyuiop(
		@pile{
			@widget{
				id{morda_min_size_forcer}
			}
			@nine_patch{
				layout{
					dx{max}
				}
				image{morda_npt_contextmenu_bg}
				@column{
					layout{
						dx{max}
					}
					id{morda_contextmenu_content}
				}
			}
			@mouse_proxy{
				layout{
					dx{fill} dy{fill}
				}
				id{morda_drop_down_menu_mouse_proxy}
			}
		}
	)qwertyuiop");
}

click_drop_down_box::click_drop_down_box(std::shared_ptr<morda::context> c, const puu::forest& desc) :
		widget(std::move(c), desc),
		button(this->context, drop_down_box_layout),
		nine_patch_push_button(this->context, drop_down_box_layout),
		drop_down_box(this->context, desc, this->get_widget_as<pile>("morda_dropdown_selection"))
{
	this->press_handler = [this](button& b){
		if(!b.is_pressed()){
			return;
		}

		this->show_drop_down_menu();
	};
}

bool click_drop_down_box::on_mouse_button(bool is_down, const morda::vector2& pos, mouse_button button, unsigned pointer_id){
	if(!is_down){
		this->mouse_button_up_handler(true);
	}

	return this->nine_patch_push_button::on_mouse_button(is_down, pos, button, pointer_id);
}

void click_drop_down_box::show_drop_down_menu(){
	if(!this->get_provider()){
		return;
	}

	auto olay = this->find_ancestor<overlay>();
	if(!olay){
		throw std::logic_error("drop_down_box: no overlay parent found");
	}

	auto np = this->context->inflater.inflate(drop_down_menu_layout);
	ASSERT(np)

	// force minimum horizontal size of the drop down menu to be the same as the drop down box horizontal size
	{
		auto min_size_forcer = np->try_get_widget("morda_min_size_forcer");

		auto& lp = min_size_forcer->get_layout_params();
		lp.dims.x = this->rect().d.x;
	}

	auto va = np->try_get_widget_as<morda::column>("morda_contextmenu_content");
	ASSERT(va)

	for(size_t i = 0; i != this->get_provider()->count(); ++i){
		va->push_back(this->wrap_item(this->get_provider()->get_widget(i), i));
	}

	this->hovered_index = -1;

	np->get_widget_as<mouse_proxy>("morda_drop_down_menu_mouse_proxy").mouse_button_handler =
			[this](bool is_down, const vector2 pos, mouse_button button, unsigned id) -> bool{
				if(!is_down){
					this->mouse_button_up_handler(false);
				}

				return true;
			};

	this->current_drop_down_menu = olay->show_context_menu(np, this->pos_in_ancestor(vector2(0), olay) + vector2(0, this->rect().d.y));
}

void click_drop_down_box::mouse_button_up_handler(bool is_first_button_up_event){
	auto ddm = this->current_drop_down_menu.lock();
	if(!ddm){
		return;
	}

	if(this->hovered_index < 0){
		if(!is_first_button_up_event){
			this->context->run_from_ui_thread([ddm](){
				ddm->remove_from_parent(); // close drop down menu
			});
		}
		return;
	}
	this->set_selection(this->hovered_index);

	auto ddb = utki::make_shared_from_this(*this);

	this->context->run_from_ui_thread([ddb, ddm](){
		ddm->remove_from_parent(); // close drop down menu
		if(ddb->selection_handler){
			ddb->selection_handler(*ddb);
		}
	});
}

std::shared_ptr<widget> click_drop_down_box::wrap_item(std::shared_ptr<widget>&& w, size_t index){
	auto wd = std::dynamic_pointer_cast<pile>(this->context->inflater.inflate(item_layout));
	ASSERT(wd)

	auto mp = wd->try_get_widget_as<mouse_proxy>("morda_dropdown_mouseproxy");
	ASSERT(mp)

	auto cl = wd->try_get_widget_as<color>("morda_dropdown_color");
	ASSERT(cl)
	auto cl_weak = utki::make_weak(cl);

	wd->push_back(w);

	mp->hover_changed_handler = [this, cl_weak, index](bool is_hovered, unsigned id){
		if(auto c = cl_weak.lock()){
			c->set_visible(is_hovered);
		}
		if(is_hovered){
			this->hovered_index = int(index);
		}else{
			if(this->hovered_index >= 0 && decltype(index)(this->hovered_index) == index){
				this->hovered_index = -1;
			}
		}
	};

	return wd;
}
