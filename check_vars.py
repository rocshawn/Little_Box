import os
import re

regex = re.compile(r'^\s*([a-zA-Z0-9_:<>]+(?:\s*\*+)?)\s+([a-zA-Z0-9_]+)\s*;\s*$', re.MULTILINE)

src_dir = r"e:\Files\Code\Little_Box\src"
outfile = r"e:\Files\Code\Little_Box\py_out.txt"
with open(outfile, 'w') as out:
    for root, _, files in os.walk(src_dir):
        for file in files:
            if file.endswith('.h'):
                path = os.path.join(root, file)
                with open(path, 'r', encoding='utf-8') as f:
                    content = f.read()
                    matches = regex.findall(content)
                    for Type, Name in matches:
                        if not Type.startswith('return') and not Type.startswith('friend') and not Type.startswith('using') and not Type.startswith('typedef'):
                            out.write(f'{path}: {Type} {Name}\n')
