#!/usr/bin/python3

import argparse
import os
import sys

def write_dir(outfile, root):
    for dirpath, dirnames, filenames in os.walk(root):
        if not filenames:
            continue
        dirpath = os.path.abspath(dirpath)
        outfile.write('  - name: "{}"\n'.format(dirpath))
        outfile.write('    type: directory\n')
        outfile.write('    contents:\n')
        for filename in filenames:
            filepath = os.path.join(dirpath, filename)
            outfile.write('      - name: {}\n'.format(filename))
            outfile.write('        type: file\n')
            outfile.write('        external-contents: "{}"\n'.format(filepath))

def main(argv):
    parser = argparse.ArgumentParser(
        description='Generate a case-insensitive Clang VFS overlay.'
    )
    parser.add_argument(
        'dirs',
        metavar='DIR',
        nargs='+',
        help='search directories'
    )
    parser.add_argument(
        '-o',
        dest='output_file',
        metavar='PATH',
        help='output file'
    )
    args = parser.parse_args(argv)

    outfile = open(args.output_file, 'w') if args.output_file else sys.stdout
    outfile.write('version: 0\n')
    outfile.write('case-sensitive: false\n')
    outfile.write('roots:\n')
    for d in args.dirs:
        write_dir(outfile, d)
    outfile.close()

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
