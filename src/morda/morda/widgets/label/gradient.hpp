#pragma once

#include "../widget.hpp"

#include "../../res/gradient.hpp"

namespace morda{

/**
 * @brief Gradient widget.
 * This is a widget which can display a rectangle with gradient.
 * From GUI script it can be instantiated as "gradient".
 *
 * @param gradient - gradient resource name.
 */
class gradient : public widget{
	std::shared_ptr<res::gradient> res;

public:
	gradient(std::shared_ptr<morda::context> c, const treeml::forest& desc);

	gradient(const gradient&) = delete;
	gradient& operator=(const gradient&) = delete;

	void render(const morda::matrix4& matrix)const override;
};

}
