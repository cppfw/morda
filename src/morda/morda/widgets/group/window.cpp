#include "window.hpp"

#include "../../context.hpp"

#include "../../util/util.hpp"

using namespace morda;

namespace{

const auto windowDesc_c = puu::read(R"qwertyuiop(
		@column{
			layout{
				dx{max} dy{max}
			}

			@row{
				layout{dx{fill}}

				@mouse_proxy{
					id{morda_lt_proxy}
				}

				@mouse_proxy{
					id{morda_t_proxy}
					layout{
						dx{0} dy{fill} weight{1}
					}
				}

				@mouse_proxy{
					id{morda_rt_proxy}
				}
			}

			@row{
				layout{
					weight{1}
					dx{max}
				}

				@mouse_proxy{
					id{morda_l_proxy}
					layout{dy{fill}}
				}

				// middle
				@column{
					layout{
						weight{1}
						dy{max}
					}

					// caption
					@pile{
						layout{
							dx{max}
						}

						clip{true}

						@mouse_proxy{
							id{morda_caption_proxy}
							layout{
								dx{max} dy{max}
							}
						}

						@color{
							id{morda_window_title_bg}
							layout{
								dx{max} dy{max}
							}
						}

						@row{
							layout{
								dx{max} dy{max}
							}

							@margins{
								left{3dp}
								top{2dp}
								bottom{2dp}

								layout{
									dx{0}
									weight{1}
								}

								@text{
									id{morda_title}
								}
							}
						}
					}

					@pile{
						id{morda_content}
						clip{true}
						layout{
							dx{fill} dy{0}
							weight{1}
						}
					}
				}

				@mouse_proxy{
					id{morda_r_proxy}
					layout{dy{fill}}
				}
			}

			@row{
				layout{
					dx{fill}
				}
				@mouse_proxy{
					id{morda_lb_proxy}
				}

				@mouse_proxy{
					id{morda_b_proxy}
					layout{
						dy{fill}
						dx{0}
						weight{1}
					}
				}

				@mouse_proxy{
					id{morda_rb_proxy}
				}
			}
		}
	)qwertyuiop");
}

void morda::window::set_background(std::shared_ptr<widget> w){
	ASSERT(this->children().size() == 1 || this->children().size() == 2)
	if(this->children().size() == 2){
		this->erase(this->children().begin());
	}

	ASSERT(this->children().size() == 1)

	if(w){
		this->insert(std::move(w), this->children().begin());
	}
}

morda::window::window(std::shared_ptr<morda::context> c, const puu::forest& desc) :
		widget(std::move(c), desc),
		pile(nullptr, windowDesc_c)
{
	this->setup_widgets();

	const real default_border_size_px = 5;
	sides<real> borders(default_border_size_px);

	for(const auto& p : desc){
		if(!is_property(p)){
			continue;
		}

		if(p.value == "title"){
			this->set_title(get_property_value(p).to_string());
		}else if(p.value == "look"){
			for(const auto& pp : p.children){
				if(!is_property(pp)){
					continue;
				}

				if(pp.value == "title_color_active"){
					this->titleBgColorTopmost = get_property_value(pp).to_uint32();
				}else if(pp.value == "title_color_inactive"){
					this->titleBgColorNonTopmost = get_property_value(pp).to_uint32();
				}else if(pp.value == "background"){
					this->set_background(this->context->inflater.inflate(pp.children));
				}else if(pp.value == "left"){
					borders.left() = parse_dimension_value(get_property_value(pp), this->context->units);
				}else if(pp.value == "top"){
					borders.top() = parse_dimension_value(get_property_value(pp), this->context->units);
				}else if(pp.value == "right"){
					borders.right() = parse_dimension_value(get_property_value(pp), this->context->units);
				}else if(pp.value == "bottom"){
					borders.bottom() = parse_dimension_value(get_property_value(pp), this->context->units);
				}
			}
		}
	}
	this->set_borders(borders);

	// this should go after initializing borders
	this->emptyMinDim = this->measure(vector2(-1));

	this->contentArea->push_back_inflate(desc);
}

void morda::window::setup_widgets(){
	this->contentArea = this->try_get_widget_as<pile>("morda_content");
	ASSERT(this->contentArea)

	this->title = this->try_get_widget_as<text>("morda_title");
	ASSERT(this->title)

	this->titleBg = this->try_get_widget_as<color>("morda_window_title_bg");
	ASSERT(this->titleBg);

	std::function<decltype(mouse_proxy::mouse_button_handler)()> make_mouse_button_handler = [this](){
		return decltype(mouse_proxy::mouse_button_handler)([this](mouse_proxy& mp, const mouse_button_event& e){
			if(e.button != mouse_button::left){
				return false;
			}

			this->mouse_captured = e.is_down;

			if(e.is_down){
				this->capturePoint = e.pos;
			}else{
				if(!mp.is_hovered()){
					mp.context->set_mouse_cursor(morda::mouse_cursor::arrow);
				}
			}
			return true;
		});
	};

	std::function<decltype(morda::mouse_proxy::hover_change_handler)(morda::mouse_cursor)> make_hover_change_handler = [this](morda::mouse_cursor cursor){
		return [this, cursor](mouse_proxy& mp, unsigned pointer_id){
			// LOG("hover = " << mp.is_hovered() << std::endl)
			// LOG("this->mouse_captured = " << this->mouse_captured << std::endl)
			if(this->mouse_captured){
				return;
			}
			if(mp.is_hovered()){
				// defer setting hovered cursor to make sure that hovered cursor is set after the unhovered cursor
				mp.context->run_from_ui_thread([context = mp.context, cursor](){
					context->set_mouse_cursor(cursor);
				});
			}else{
				mp.context->set_mouse_cursor(morda::mouse_cursor::arrow);
			}
		};
	};

	{
		auto caption = this->try_get_widget_as<mouse_proxy>("morda_caption_proxy");
		ASSERT(caption)

		caption->mouse_button_handler = make_mouse_button_handler();

		caption->mouse_move_handler = [this](mouse_proxy&, const mouse_move_event& e){
			if(this->mouse_captured){
				this->move_by(e.pos - this->capturePoint);
				return true;
			}
			return false;
		};
		caption->hover_change_handler = make_hover_change_handler(morda::mouse_cursor::grab);
	}

	{
		auto w = this->try_get_widget_as<mouse_proxy>("morda_lt_proxy");
		ASSERT(w)
		w->mouse_button_handler = make_mouse_button_handler();
		w->mouse_move_handler = [this](mouse_proxy&, const mouse_move_event& e){
			if(this->mouse_captured){
				using std::min;
				morda::vector2 d = e.pos - this->capturePoint;
				d = min(d, this->rect().d - this->emptyMinDim); // clamp top
				this->move_by(d);
				this->resize_by(-d);
			}
			return false;
		};
		w->hover_change_handler = make_hover_change_handler(morda::mouse_cursor::top_left_corner);
		this->ltBorder = w;
	}

	{
		auto w = this->try_get_widget_as<mouse_proxy>("morda_lb_proxy");
		ASSERT(w)
		w->mouse_button_handler = make_mouse_button_handler();
		w->mouse_move_handler = [this](mouse_proxy&, const mouse_move_event& e){
			if(this->mouse_captured){
				using std::min;
				using std::max;
				morda::vector2 d = e.pos - this->capturePoint;
				d.x() = min(d.x(), this->rect().d.x() - this->emptyMinDim.x()); // clamp top
				d.y() = max(d.y(), -(this->rect().d.y() - this->emptyMinDim.y())); // clamp bottom
				this->move_by(morda::vector2(d.x(), 0));
				this->resize_by(morda::vector2(-d.x(), d.y()));
			}
			return false;
		};
		w->hover_change_handler = make_hover_change_handler(morda::mouse_cursor::bottom_left_corner);
		this->lbBorder = w;
	}

	{
		auto w = this->try_get_widget_as<mouse_proxy>("morda_rt_proxy");
		ASSERT(w)
		w->mouse_button_handler = make_mouse_button_handler();
		w->mouse_move_handler = [this](mouse_proxy&, const mouse_move_event& e){
			if(this->mouse_captured){
				using std::min;
				using std::max;
				morda::vector2 d = e.pos - this->capturePoint;
				d.x() = max(d.x(), -(this->rect().d.x() - this->emptyMinDim.x())); // clamp bottom
				d.y() = min(d.y(), this->rect().d.y() - this->emptyMinDim.y()); // clamp top
				this->move_by(morda::vector2(0, d.y()));
				this->resize_by(morda::vector2(d.x(), -d.y()));
			}
			return false;
		};
		w->hover_change_handler = make_hover_change_handler(morda::mouse_cursor::top_right_corner);
		this->rtBorder = w;
	}

	{
		auto w = this->try_get_widget_as<mouse_proxy>("morda_rb_proxy");
		ASSERT(w)
		w->mouse_button_handler = make_mouse_button_handler();
		w->mouse_move_handler = [this](mouse_proxy&, const mouse_move_event& e){
			if(this->mouse_captured){
				using std::max;
				morda::vector2 d = e.pos - this->capturePoint;
				d = max(d, -(this->rect().d - this->emptyMinDim)); // clamp bottom
				this->resize_by(d);
			}
			return false;
		};
		w->hover_change_handler = make_hover_change_handler(morda::mouse_cursor::bottom_right_corner);
		this->rbBorder = w;
	}

	{
		auto w = this->try_get_widget_as<mouse_proxy>("morda_l_proxy");
		ASSERT(w)
		w->mouse_button_handler = make_mouse_button_handler();
		w->mouse_move_handler = [this](mouse_proxy&, const mouse_move_event& e){
			if(this->mouse_captured){
				using std::min;
				morda::vector2 d = e.pos - this->capturePoint;
				d.x() = min(d.x(), this->rect().d.x() - this->emptyMinDim.x()); // clamp top
				this->move_by(morda::vector2(d.x(), 0));
				this->resize_by(morda::vector2(-d.x(), 0));
			}
			return false;
		};
		w->hover_change_handler = make_hover_change_handler(morda::mouse_cursor::left_side);
		this->lBorder = w;
	}

	{
		auto w = this->try_get_widget_as<mouse_proxy>("morda_r_proxy");
		ASSERT(w)
		w->mouse_button_handler = make_mouse_button_handler();
		w->mouse_move_handler = [this](mouse_proxy&, const mouse_move_event& e){
			if(this->mouse_captured){
				using std::max;
				morda::vector2 d = e.pos - this->capturePoint;
				d.x() = max(d.x(), -(this->rect().d.x() - this->emptyMinDim.x())); // clamp bottom
				this->resize_by(morda::vector2(d.x(), 0));
			}
			return false;
		};
		w->hover_change_handler = make_hover_change_handler(morda::mouse_cursor::right_side);
		this->rBorder = w;
	}

	{
		auto w = this->try_get_widget_as<mouse_proxy>("morda_t_proxy");
		ASSERT(w)
		w->mouse_button_handler = make_mouse_button_handler();
		w->mouse_move_handler = [this](mouse_proxy&, const mouse_move_event& e){
			if(this->mouse_captured){
				using std::min;
				morda::vector2 d = e.pos - this->capturePoint;
				d.y() = min(d.y(), this->rect().d.y() - this->emptyMinDim.y()); // clamp top
				this->move_by(morda::vector2(0, d.y()));
				this->resize_by(morda::vector2(0, -d.y()));
			}
			return false;
		};
		w->hover_change_handler = make_hover_change_handler(morda::mouse_cursor::top_side);
		this->tBorder = w;
	}

	{
		auto w = this->try_get_widget_as<mouse_proxy>("morda_b_proxy");
		ASSERT(w)
		w->mouse_button_handler = make_mouse_button_handler();
		w->mouse_move_handler = [this](mouse_proxy&, const mouse_move_event& e){
			if(this->mouse_captured){
				using std::max;
				morda::vector2 d = e.pos - this->capturePoint;
				d.y() = max(d.y(), -(this->rect().d.y() - this->emptyMinDim.y())); // clamp bottom
				this->resize_by(morda::vector2(0, d.y()));
			}
			return false;
		};
		w->hover_change_handler = make_hover_change_handler(morda::mouse_cursor::bottom_side);
		this->bBorder = w;
	}
}

void morda::window::set_title(const std::string& str){
	this->title->set_text(utki::to_utf32(str));
}

void morda::window::set_borders(sides<real> borders) {
	this->lBorder->get_layout_params().dims.x() = borders.left();
	this->tBorder->get_layout_params().dims.y() = borders.top();
	this->rBorder->get_layout_params().dims.x() = borders.right();
	this->bBorder->get_layout_params().dims.y() = borders.bottom();

	this->lbBorder->get_layout_params().dims.x() = borders.left();
	this->lbBorder->get_layout_params().dims.y() = borders.bottom();

	this->rbBorder->get_layout_params().dims.x() = borders.right();
	this->rbBorder->get_layout_params().dims.y() = borders.bottom();

	this->ltBorder->get_layout_params().dims.x() = borders.left();
	this->ltBorder->get_layout_params().dims.y() = borders.top();

	this->rtBorder->get_layout_params().dims.x() = borders.right();
	this->rtBorder->get_layout_params().dims.y() = borders.top();
}

bool morda::window::on_mouse_button(const mouse_button_event& e){
	if(e.is_down && !this->is_topmost()){
		this->context->run_from_ui_thread(
				[this](){
					this->move_to_top();
					this->focus();
				}
			);
	}

	this->container::on_mouse_button(e);

	return true;
}

bool morda::window::on_mouse_move(const mouse_move_event& e){
	this->container::on_mouse_move(e);
	return true;
}

void window::move_to_top(){
	if(!this->parent()){
		return;
	}

	ASSERT(this->parent()->children().size() != 0)

	if(this->parent()->children().size() == 1){
		return;
	}

	auto prevTopmost = this->parent()->children().rbegin()->get();
	ASSERT(prevTopmost)
	if(prevTopmost == this){
		return;//already topmost
	}

	container* p = this->parent();

	p->change_child_z_position(p->find(*this), p->children().end());

	this->updateTopmost();

	if(auto pt = dynamic_cast<window*>(prevTopmost)){
		pt->updateTopmost();
	}
}

bool window::is_topmost()const noexcept{
	if(!this->parent()){
		return false;
	}

	ASSERT(this->parent()->children().size() != 0)

	return this->parent()->children().back().get() == this;
}

void window::updateTopmost(){
	this->titleBg->set_color(
			this->is_topmost() ? this->titleBgColorTopmost : this->titleBgColorNonTopmost
		);
}

void window::lay_out(){
	this->updateTopmost();
	this->pile::lay_out();
}
