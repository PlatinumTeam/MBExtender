#!/usr/bin/python3

import argparse
import sys

# Only internal variables whose names start with a string in this list are saved
KEEP_PREFIXES = [
    'CMAKE_HAVE_',
    'HAVE_',
    'SIZEOF_',
    'curl_cv_',
]


def main(argv):
    parser = argparse.ArgumentParser(description='Save an MBExtender CMake cache to a script usable with cmake -C.')
    parser.add_argument('-o', dest='output', required=True, help='Path to the output .cmake file')
    parser.add_argument('path', help='Path to the CMakeCache.txt file')
    args = parser.parse_args(argv)

    cache = {}
    with open(args.path, 'r') as f:
        for line in f:
            line = line.lstrip().rstrip('\n')
            if line.startswith('#') or line.startswith('//'):
                continue
            if ':' not in line:
                continue
            name, _, line = line.partition(':')
            if '=' not in line:
                continue
            type, _, value = line.partition('=')
            if type != 'INTERNAL':
                continue
            if not any(name.startswith(prefix) for prefix in KEEP_PREFIXES):
                continue
            cache[name] = value

    with open(args.output, 'w') as f:
        for name, value in sorted(cache.items()):
            print(name + '=' + value)
            value = value.replace('"', '\\"')
            f.write('set({} "{}" CACHE INTERNAL "")\n'.format(name, value))

if __name__ == '__main__':
    main(sys.argv[1:])
