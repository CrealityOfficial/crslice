#ifndef CRSLICE_SLICE_H
#define CRSLICE_SLICE_H
#include "crslice/interface.h"
#include "crslice/crscene.h"

namespace crslice
{
    struct SliceResult
    {
        unsigned long int print_time; // Ԥ����ӡ��ʱ����λ����
        double filament_len; // Ԥ���������ģ���λ����
        double filament_volume; // Ԥ��������������λ��g
        unsigned long int layer_count;  // ��Ƭ����
        double x;   // ��Ƭx�ߴ�
        double y;   // ��Ƭy�ߴ�
        double z;   // ��Ƭz�ߴ�
    };

	class CRSLICE_API CrSlice
	{
	public:
		CrSlice();
		~CrSlice();

		void sliceFromFakeArguments(int argc, const char* argv[], ccglobal::Tracer* tracer = nullptr);
		void sliceFromScene(CrScenePtr scene, ccglobal::Tracer* tracer = nullptr);

		void process();

        SliceResult sliceResult;
	};
}
#endif  // MSIMPLIFY_SIMPLIFY_H
