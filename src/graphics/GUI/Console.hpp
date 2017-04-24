#pragma once
#include "Base.hpp"

namespace Graphics::GUI {

class Console : public Base
{
public:
	Console(Game&);

	std::string type() const override;

	void init() override;
	void draw() override;
};

}
