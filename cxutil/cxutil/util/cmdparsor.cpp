#include "cxutil/util/cmdparsor.h"
#include<string.h>
namespace cxutil
{
	CmdParsor::CmdParsor(int argc, char* argv[])
		:m_argc(argc)
		, m_argv(argv)
	{
	}

	CmdParsor::~CmdParsor()
	{

	}

	void CmdParsor::registerFunc(const char* cmd, processFunc func)
	{
        if(func) m_funcs.insert(std::map<std::string, processFunc>::value_type(cmd, func));
	}

	void CmdParsor::process()
	{
		char delims[] = "=";
		for (int i = 0; i < m_argc; ++i)
		{
			char* argv = m_argv[i];
			char* token = strtok(argv, delims);
			
			std::string cmd, param;
			if (token)
			{
				cmd = token;
				token = strtok(nullptr, delims);
				if (token) param = token;
			}
			else
			{
				cmd = argv;
			}

            std::map<std::string, processFunc>::iterator it = m_funcs.find(cmd);
			if (it != m_funcs.end())
			{
				(*it).second(param);
			}

		}
	}
}
