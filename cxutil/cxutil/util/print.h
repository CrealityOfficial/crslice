#ifndef CURA_PRINT_1605157544355_H
#define CURA_PRINT_1605157544355_H
#include <string>
#include <vector>

namespace cxutil
{
	void printHelp();
	void printCall(int argc, char* argv[]);
	void printCall(const std::vector<std::string>& args);
}

#endif // CURA_PRINT_1605157544355_H