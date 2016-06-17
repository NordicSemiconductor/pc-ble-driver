!/usr/bin/python
#
# Copyright (c) 2016 Nordic Semiconductor ASA
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
#
#   2. Redistributions in binary form must reproduce the above copyright notice, this
#   list of conditions and the following disclaimer in the documentation and/or
#   other materials provided with the distribution.
#
#   3. Neither the name of Nordic Semiconductor ASA nor the names of other
#   contributors to this software may be used to endorse or promote products
#   derived from this software without specific prior written permission.
#
#   4. This software must only be used in or with a processor manufactured by Nordic
#   Semiconductor ASA, or in or with a processor manufactured by a third party that
#   is used in combination with a processor manufactured by Nordic Semiconductor.
#
#   5. Any software provided in binary or object form under this license must not be
#   reverse engineered, decompiled, modified and/or disassembled.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

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
