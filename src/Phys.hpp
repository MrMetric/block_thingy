#pragma once

class Phys
{
	public:
		Phys();
		virtual ~Phys();

		void step();

		double gravity;

	private:
};