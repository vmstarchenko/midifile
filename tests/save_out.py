#!/usr/bin/env python

import sys
import os

TEMPLATE = """\
#include <map>
#include <string>

%(values)s

std::map<std::string, std::string> TEST_OUTS = {
    %(names)s
};
"""

VALUE_TEMPLATE = '''std::string %(name)s = %(value)s;\n\n'''

def format_value(lines):
    return '(\n    ' + '\n   '.join(
        '"%s"' % line.replace('"', '\\"').replace('\n', '\\n')
        for line in lines
    ) + '\n)'

def main():
    out_dir, cur_dir = sys.argv[1:]
    print('Out dir:', out_dir)
    print('Cur dir:', cur_dir)

    values = {}
    outs = os.listdir(out_dir)
    print('Found tests:', outs)
    for fname in outs:
        with open(os.path.join(out_dir, fname)) as f:
            values[fname] = VALUE_TEMPLATE % {
                'name': fname,
                'value': format_value(f.readlines()),
            }

    print('Write test_outs.hpp')
    with open(os.path.join(cur_dir, 'test_outs.hpp'), 'w') as f:
        f.write(TEMPLATE % {
            'values': '\n'.join(sorted(values.values())),
            'names': '\n    '.join(
                '{"%(name)s", %(name)s},' % {'name': name}
                for name in sorted(values)
            )
        })
    


if __name__ == '__main__':
    main()

