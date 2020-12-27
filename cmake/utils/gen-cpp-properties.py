import argparse
import json
import itertools
parser = argparse.ArgumentParser(prefix_chars='-')
parser.add_argument('--file', required=True, help='Path to c_cpp_properties file')
parser.add_argument('--name', help='Config name', default='')
parser.add_argument('-D', action='append', nargs='*',
                    help='Preprocessor defines', default=[])
parser.add_argument('-i', action='append', nargs='*',
                    help='Header include dir', default=[])
parser.add_argument('-f', action='append', nargs='*',
                    help='Force include source/headers', default=[])
parser.add_argument('-cc', nargs='?', help='Path to compilec_commands.json')


def jsonTemplate():
    template = '''{
                "name": "",
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
    # parse
    newJson = jsonTemplate()
    newJson['name'] = namespace.name if namespace.name else 'Generated config'
    newJson['includePath'] = list(itertools.chain.from_iterable(namespace.i))
    newJson['defines'] = list(itertools.chain.from_iterable(namespace.D))
    newJson['forcedInclude'] = list(itertools.chain.from_iterable(namespace.f))
    newJson['compileCommands'] = namespace.cc if namespace.cc else ''
    # find config in file
    try:
        with open(namespace.file, "r") as f:
            cpptoolsConfig = json.loads(f.read())
        matching = [x for x in cpptoolsConfig['configurations']
                    if x['name'] == newJson['name']]
    except Exception as e:
        cpptoolsConfig = jsonTemplateEmpty()
        matching = None
    # write config to file
    if matching:
        configs = cpptoolsConfig['configurations']
        configs[configs.index(matching[0])] = newJson
    else:
        list.append(cpptoolsConfig['configurations'], newJson)

    with open(namespace.file, "w+") as f:
        f.write(json.dumps(cpptoolsConfig, indent=2))
