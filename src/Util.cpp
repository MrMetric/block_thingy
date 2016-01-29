#include "Util.hpp"
#include <fstream>
#include <iostream>

std::string Util::read_file(const std::string& path)
{
	std::ifstream inpoot(path, std::ios::ate | std::ios::binary);
	if(!inpoot.is_open())
	{
		std::cerr << "failed to read " << path << "\n";
		exit(1);
	}
	auto fsize_signed = inpoot.tellg();
	if(fsize_signed < 0)
	{
		// TODO: handle this
		std::cerr << "Failed to read " << path << "\n";
		return "";
	}
	uint_fast64_t fsize = uint_fast64_t(fsize_signed);
	inpoot.seekg(0, std::ios::beg);
	char* aaa = new char[fsize];
	inpoot.read(aaa, fsize_signed);
	std::string bbb(aaa, fsize);
	delete[] aaa;
	return bbb;
}