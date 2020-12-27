import argparse
import json
parser = argparse.ArgumentParser(prefix_chars='-')
parser.add_argument('--file',required=True,type=argparse.FileType(mode='r', bufsize=-1, encoding=None),help='Path to cpptools file')
parser.add_argument('-D',action='append',help='Preprocessor defines', default=[])
parser.add_argument('-i',action='append',help='Header include dir',default=[])
parser.add_argument('-f',action='append',help='Force include source/headers',default=[])
parser.add_argument('-cc',nargs='?', help='Path to compilec_commands.json')


def jsonTemplate():
    template = '''{
                "name": "STM32_CMake",
                "cStandard": "c99",
                "cppStandard": "c++17",
                "intelliSenseMode": "gcc-x64",
                "includePath": [],
                "defines":[],
                 "forcedInclude": [  ]
            }'''
    return json.loads(template)
    
def jsonTemplateEmpty():
    template = '''{
                "configurations": []
            }'''
    return json.loads(template)

if __name__ == "__main__":
    namespace = parser.parse_args()
    #print (namespace)
    try:
        cpptoolsConfig = json.loads(namespace.file.read())
        matching = [x for x in cpptoolsConfig['configurations'] if x['name'] =='STM32_CMake']
    except:
        matching = None
        cpptoolsConfig = jsonTemplateEmpty()
            

    newJson = jsonTemplate()
    if namespace.i:
        newJson['includePath']=namespace.i[0].split(' -i')
    if namespace.D:
        newJson['defines']=namespace.D[0].split(' -D')
    if namespace.f:
        newJson['forcedInclude']=namespace.f[0].split(' -f')
    if namespace.cc:
        newJson['compileCommands']=namespace.cc
    if matching:
         cpptoolsConfig['configurations'][cpptoolsConfig['configurations'].index(matching[0])] = newJson
    else:
        list.append(cpptoolsConfig['configurations'],newJson)
    fh = open(namespace.file.name, "w")
    fh.write(json.dumps(cpptoolsConfig,indent=2)) 
    fh.close()