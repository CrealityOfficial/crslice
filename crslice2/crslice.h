#ifndef CRSLICE_SLICE_H_2
#define CRSLICE_SLICE_H_2
#include "crslice2/interface.h"
#include "crslice2/crscene.h"

namespace crslice2
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

	class CRSLICE2_API CrSlice
	{
	public:
		CrSlice();
		~CrSlice();

		void sliceFromScene(CrScenePtr scene, ccglobal::Tracer* tracer = nullptr);

        SliceResult sliceResult;
	};

    CRSLICE2_API void prusaSliceFromFile(const std::string& file, const std::string& out);
}
#endif  // MSIMPLIFY_SIMPLIFY_H
