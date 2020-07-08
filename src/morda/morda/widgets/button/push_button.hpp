#pragma once

#include "button.hpp"

namespace morda{

/**
 * @brief Basic class of a push button.
 * Push button is a button which can be clicked.
 * Clicked means that the button was pressed and then released.
 */
class push_button : virtual public button{
	bool currentlyPressed = false;

protected:
	void on_press_change()override;
	
	/**
	 * @brief Invoked when the button is clicked.
	 * Default implementation emits 'clicked' signal.
	 */
	virtual void on_clicked();
	
	bool on_mouse_button(const  mouse_button_event& event)override;
	
	void on_hover_change(unsigned pointer_id)override;
	
public:
	push_button(std::shared_ptr<morda::context> c, const puu::forest& desc) :
			widget(std::move(c), desc),
			button(nullptr, desc)
	{}
	
	/**
	 * @brief Clicked signal.
	 * Emitted when the button is clicked.
	 */
	std::function<void(push_button&)> click_handler;
};

}
