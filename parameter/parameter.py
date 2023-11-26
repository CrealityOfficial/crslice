import sys
from pathlib import Path

class ParameterCpper():
    '''
    convert json to C++ files
    parameter_wrapper.h
    parameter_wrapper.cpp
    '''
    def __init__(self, sdir, ddir):
        self.source_dir = sdir
        self.dest_dir = ddir
        
        cpp_path = Path(dest_dir + '/parameter_wrapper.cpp')
        h_path = Path(dest_dir + '/parameter_wrapper.h')
        self.cpp_file = cpp_path.open('w', encoding='utf8')
        self.h_file = h_path.open('w', encoding='utf8')
        
    def generate(self, jsons):
        datas = {}
        #.h
        self.generate_h(self.h_file, datas)
        #.cpp
        self.generate_cpp(self.cpp_file, datas)
        
    def generate_h(self, out, datas):
        out.write('#ifndef _PARAMETER_WRAPPER_H_\n')
        out.write('#define _PARAMETER_WRAPPER_H_\n')
        out.write('#include <string>\n')
        out.write('namespace cura52{class Settings;\n') #namespace
        
        out.write('class SceneParamWrapper1{public:\n')  #scene
        out.write('void initialize(Settings* settings);\n')
        out.write('};\n')                                  #scene
        
        out.write('}\n')                                #namespace
        out.write('#endif //_PARAMETER_WRAPPER_H_\n')
        
    def generate_cpp(self, out, datas):
        out.write('#include "parameter_wrapper.h"\n')
        out.write('namespace cura52{')
        
        out.write('void SceneParamWrapper1::initialize(Settings* settings){\n')   #scene
        out.write('}')                                                           #scene
        
        out.write('}')
        
    def __del__(self):
        self.cpp_file.close()
        self.h_file.close()

if __name__ == "__main__":
    source_dir = sys.argv[1]
    dest_dir = sys.argv[2]
    jsons = sys.argv[3]
    
    print('source_dir {}'.format(source_dir))
    print('dest_dir {}'.format(dest_dir))
    print('jsons {}'.format(jsons))
    
    pcc = ParameterCpper(source_dir, dest_dir)
    pcc.generate(jsons)