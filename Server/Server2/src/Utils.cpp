#include "Utils.h"


std::vector<std::string> utils::SplitString(std::string str, const std::string& delim)
{
	std::vector<std::string> res;

	if (str.empty() || delim.empty())
		return res;

	size_t pos;
	while ((pos = str.find(delim)) != std::string::npos)
	{
		res.emplace_back(str.substr(0, pos));
		str.erase(0, pos + delim.length());
	}

	if (res.empty())
		res.emplace_back(str);

	return res;
}
