#ifndef CX_INPUTCACHE_1604026967709_H
#define CX_INPUTCACHE_1604026967709_H
#include <vector>

namespace cxutil
{
	class DLPInput;
	class InputCache
	{
	public:
		InputCache();
		~InputCache();

		void save(const char* file, std::vector<float*>& vData, std::vector<int>& vNr);
		void save(const char* file);
		void load(const char* file);
		void fill(DLPInput* input);
	protected:
		std::vector<std::vector<float>> m_data;
	};
}

#endif // CX_INPUTCACHE_1604026967709_H