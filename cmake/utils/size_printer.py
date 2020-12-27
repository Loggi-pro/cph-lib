import enum
from pathlib import Path
import re
import argparse
import os
import distutils.dir_util
import filecmp
import sys
import serial
from typing import NamedTuple
from time import sleep
import subprocess
from subprocess import DEVNULL


# ==========================CLI================================
parser = argparse.ArgumentParser(conflict_handler='resolve')
parser.add_argument('--size', type=str, action="store",
                    help='Path to cmake-size')
parser.add_argument('--mcu', action="store", type=str,
                    help='MCU name')
parser.add_argument('--maxflash', action="store", type=int,
                    help='max flash size', default=0)
parser.add_argument('--maxram', action="store", type=int,
                    help='max ram size', default=0)
parser.add_argument('--elf', type=str, action="store",
                    help='Path to elf file')
parser.add_argument('--ezstack', type=str, help='Path to stack analyzer',
                    action="store", default='')

# ==========================MAIN SECTION================================


class SizeStruct:
    """App result data"""

    def __init__(self):
        self.device = ""
        self.program_size = 0
        self.program_percent = 0
        self.data_size = 0
        self.data_percent = 0
        self.ezstack_used = False


class Arguments:
    """App cli arguments"""

    def __init__(self, size, elf, ezstack, mcu, maxflash, maxram):
        self.size = size
        self.elf = elf
        self.ezstack = ezstack
        self.mcu = mcu
        self.maxflash = maxflash
        self.maxram = maxram


def run(args: Arguments) -> SizeStruct:
    """Return the size info of elf file"""
    set_current_working_dir()
    size_path = which(args.size)
    elf_path = which(args.elf)
    assert_path_exist(size_path, "size utility was not found: use --size")
    assert_path_exist(elf_path, "elf file was not found: use --elf")
    mcu: McuType = select_mcu(args.mcu)
    ha = McuHandlerArgs()
    ha.size_path, ha.elf_path, ha.mcu, ha.maxflash, ha.maxram = size_path, elf_path, args.mcu, args.maxflash, args.maxram
    size_result: SizeStruct = handler_selector[mcu](ha)
    parseResult: EzStackStruct = handle_ezstack(which(args.ezstack), elf_path)
    if parseResult.ezstack_used:
        size_result.data_size, size_result.data_percent, size_result.ezstack_used = parseResult.data_size, parseResult.data_percent, parseResult.ezstack_used
    return size_result


# ==========================SPECIFIC MCU HANDLER================================

class McuType(enum.Enum):
    """Mcu type"""
    avr = 1
    stm32 = 2
    unknown = 3


# Allow to select specific mcu handler
handler_selector = {
    McuType.avr: lambda args: handler_wrapper(handle_avr, args),
    McuType.stm32: lambda args: handler_wrapper(handle_stm32, args),
    McuType.unknown: lambda args: (_ for _ in ()).throw(
        TypeError('mcu argument not specified'))
}


class McuHandlerArgs:
    """MCU Handler arguments:
    size_path: str, elf_path: str, mcu: str, maxflash: int, maxram: int"""
    pass


def handler_wrapper(f, args: McuHandlerArgs) -> SizeStruct:
    """Mcu handler wrapper=same exception policy. Call handler(f)"""
    try:
        return f(args)
    except Exception as e:
        raise Exception(
            f'error happened while parse size utility output (possibly wrong mcu): {str(e)}') from e


def handle_avr(args: McuHandlerArgs) -> SizeStruct:
    """Avr handler: call avr size_tool, and parse it output"""
    output = subprocess.Popen(
        [args.size_path, "-C", f"--mcu={args.mcu}", f"{args.elf_path}"], stdout=subprocess.PIPE).communicate()[0]
    output = str(output.decode("utf-8"))
    return parse_avr_output(output)


def handle_stm32(args: McuHandlerArgs) -> SizeStruct:
    """Stm32 handler: call stm32 size_tool, and parse it output"""
    output = subprocess.Popen(
        [args.size_path, "-B", f"{args.elf_path}"], stdout=subprocess.PIPE).communicate()[0]
    output = str(output.decode("utf-8"))
    return parse_stm32_output(args.mcu, output, args.maxflash, args.maxram)


def parse_avr_output(output) -> SizeStruct:
    """Avr handler: parse avr size_tool output to result struct"""
    # output = 'AVR Memory Usage\r\n----------------\r\nDevice: atmega168p\r\n\r\nProgram:     156 bytes (1.0% Full)\r\n(.text + .data + .bootloader)\r\n\r\nData:          0 bytes (0.0% Full)\r\n(.data + .bss + .noinit)\r\n\r\n\r\n'
    # print(parseAvrSize(output).data_size)
    # output  ='Program = 156 bytes; RAM usage = 10 bytes (1.0% Full)'
    res = SizeStruct()
    res.device = re.search(r'Device:\s*(.*)\r', output).group(1)
    res.program_size = float(re.search(
        r'Program:\s*(\d+).*(\d+)\%', output).group(1))
    res.program_percent = float(re.search(
        r'Program:\s*(\d+).*(\d+)\%', output).group(2))
    res.data_size = float(
        re.search(r'Data:\s*(\d+).*(\d+)\%', output).group(1))
    res.data_percent = float(
        re.search(r'Data:\s*(\d+).*(\d+)\%', output).group(2))
    return res


def parse_stm32_output(mcu, output, maxflash, maxram) -> SizeStruct:
    """Stm32 handler: parse stm32 size_tool output to result struct"""
    #    output = """   text	   data	    bss	    dec	    hex	filename
    #   12316	      0	      4	  12320	   3020	path:/to/file/stm32f103_example"""

    l = [x.group() for x in re.finditer(r'(\d+)', output)]
    text, data, bss = float(l[0]), float(l[1]), float(l[2])
    res = SizeStruct()
    res.device = mcu
    res.program_size = text+data
    res.data_size = data+bss
    if (maxflash and maxflash > 0):
        res.program_percent = res.program_size/maxflash*100
    if (maxram and maxram > 0):
        res.data_percent = res.data_size/maxram*100
    return res


def select_mcu(mcu: str) -> McuType:
    """Convert cli mcu to enum"""
    if (mcu is None or mcu == ''):
        return McuType.unknown
    if ("stm32" in mcu.lower()):
        return McuType.stm32
    else:
        return McuType.avr


# ==========================EZSTACK HANDLER================================


class EzStackStruct:
    """Ezstack result:"""

    def __init__(self, used: bool, data_size: float = 0, data_percent: float = 0):
        self.ezstack_used = used
        self.data_size = data_size
        self.data_percent = data_percent


def handle_ezstack(ezstack_path: str, elf_path: str) -> EzStackStruct:
    """EzStack handler"""
    try:
        output = subprocess.Popen(
            [ezstack_path, f"{elf_path}", "-allowCallsFromIsr", "-includeBadInterrupt", "-totalOnly"], stdout=subprocess.PIPE, stderr=DEVNULL).communicate()[0]
        output = str(output.decode("utf-8"))
        data_size = float(
            re.search(r'RAM usage = (\d+)', output).group(1))
        data_percent = float(re.search(
            r'RAM usage = \d+.*\((\d+\.\d+)\%', output).group(1))
        return EzStackStruct(True, data_size, data_percent)
    except Exception:
        return EzStackStruct(False)

# ==========================colorization================================


class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


def print_size(size_result: SizeStruct):
    """Print app result data"""
    program_str = value_to_str(size_result.program_size)
    data_str = value_to_str(size_result.data_size)

    program_color = percent_to_color(size_result.program_percent)
    data_color = percent_to_color(size_result.data_percent)
    program_str = colorize_string(program_str, program_color)
    program_percent_str = colorize_string(
        f'{size_result.program_percent:.1f}%', program_color)
    data_str = colorize_string(data_str, data_color)
    data_percent_str = colorize_string(
        f'{size_result.data_percent:.1f}%', data_color)
    if (not size_result.ezstack_used):
        print(colorize_string(
            'Note: Ezstack can\'t calc RAM usage of stack.', bcolors.WARNING))
    print(
        f'Device: {size_result.device}. Program data: {program_str} ({program_percent_str}). RAM usage: {data_str} ({data_percent_str}).')

    if size_result.program_percent >= 100:
        raise Exception('flash size overflow')
    if size_result.data_percent >= 100:
        raise Exception('ram size overflow')


def value_to_str(x) -> str:
    """Convert value to string with suffix"""
    suffix = 'bytes'
    result = f'{x:.0f} {suffix}'

    if x >= 1024:
        x /= 1024
        suffix = 'Kb'
        result = f'{x:.1f} {suffix}'
    return result


def percent_to_color(percent) -> str:
    """Convert percents to color. Valid percents -> green, else -> red"""
    return bcolors.OKGREEN if percent < 100 else bcolors.FAIL


def colorize_string(s: str, color: str) -> str:
    """Wrap string with specified color"""
    return f'{color}{s}{bcolors.ENDC}'

# ==========================UTILITIES========================


def set_current_working_dir():
    """Change working dir to folder of file"""
    abspath = os.path.abspath(__file__)
    dname = os.path.dirname(abspath)
    os.chdir(dname)


def which(arg: str):
    """Return fullpath of arg, or None if file not exist"""
    import os
    try:
        fpath = os.path.split(arg)
        if not fpath:
            return None
        if os.path.isfile(arg):
            return arg
        for path in os.environ["PATH"].split(os.pathsep):
            f = os.path.join(path, arg)
            if os.path.isfile(f):
                return f
    except Exception:
        pass
    return None


def assert_path_exist(path, message):
    """Throw exception if file not exists"""
    if path is None:
        raise FileNotFoundError(
            f'{message}:{path}')


# ==========================STARTUP SECTION========================
if __name__ == "__main__":
    # pass
    os.system('color')
    namespace = parser.parse_args()
    args = Arguments(size=namespace.size, elf=namespace.elf,
                     ezstack=namespace.ezstack, mcu=namespace.mcu, maxflash=namespace.maxflash, maxram=namespace.maxram)
    try:
        size_result = run(args)
        print_size(size_result)
    except Exception as e:
        print(f'{bcolors.FAIL}Error: {str(e)}.{bcolors.ENDC}')
