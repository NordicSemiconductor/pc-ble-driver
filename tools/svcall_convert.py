#!/usr/bin/python

import re
import argparse
import sys

parser = argparse.ArgumentParser(description='Convert SoftDevice header files SVCALL macros to functions used by pc-ble-driver')
parser.add_argument('infile', nargs='?', type=argparse.FileType('r'), default=sys.stdin)
parser.add_argument('outfile', nargs='?', type=argparse.FileType('w', 0), default=sys.stdout)

args = parser.parse_args()

lines = []

with args.infile as file:
    lines = file.readlines()

args.infile.close()

svc = re.compile('SVCALL\(([A-Za-z0-9\_]+)\,\s(?P<retval>[a-zA-Z0-9\_\[\]\*]+)\,\s((?P<func>[a-zA-Z0-9\_]+)\((?P<func_args>.*)\))\)\;')

func_count = 0

for line in lines:
    result = svc.match(line)

    if result:
        func = result.group('func')
        func_args = result.group('func_args')
        retval = result.group('retval')

        if func_args == 'void':
            args.outfile.write('SD_RPC_API %s %s(adapter_t *adapter);\n' % (retval, func))
        else:
            args.outfile.write('SD_RPC_API %s %s(adapter_t *adapter, %s);\n' % (retval, func, func_args))

        func_count += 1
    else:
        args.outfile.write(line)

args.outfile.close()

print 'Converted %s SVCALL functions.' % func_count