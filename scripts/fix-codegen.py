#!/usr/bin/env python3
"""Apply manual fixes to codegen output for unresolved branches.

Codegen sometimes fails to resolve branch targets that do exist in the
generated code, emitting REX_FATAL instead. This script patches those
sites after codegen runs. It's idempotent - safe to run multiple times.
"""
import os
import sys

def patch_file(path, old, new, desc):
    with open(path) as f:
        content = f.read()
    if old in content:
        content = content.replace(old, new)
        with open(path, 'w') as f:
            f.write(content)
        print(f"  Fixed: {desc}")
        return True
    elif new.split('\n')[0] in content:
        print(f"  Already fixed: {desc}")
        return True
    else:
        print(f"  WARNING: pattern not found for {desc}", file=sys.stderr)
        return False

def main():
    gen_dir = os.path.join(os.path.dirname(__file__), '..', 'generated', 'default')
    if not os.path.isdir(gen_dir):
        print(f"Generated dir not found: {gen_dir}", file=sys.stderr)
        return 1

    target = os.path.join(gen_dir, 'condemned2recomp_recomp.115.cpp')
    if not os.path.isfile(target):
        print(f"Target file not found: {target}", file=sys.stderr)
        return 1

    # 1. Add forward declaration
    patch_file(target,
        '#include "condemned2recomp_funcs.115.h"\n',
        '#include "condemned2recomp_funcs.115.h"\nDECLARE_REX_FUNC(sub_8217C024);  // for manually resolved branch\n',
        'forward declaration')

    # 2. Fix unconditional branches to 0x8217C024
    patch_file(target,
        '\t// b 0x8217c024\n\t// FATAL: unresolved function 0x8217C024 (no CallTarget in FunctionNode)\n\tREX_FATAL("Unresolved call from 0x8217BF20 to 0x8217C024");\n\treturn;',
        '\t// b 0x8217c024 (manually resolved)\n\tsub_8217C024(ctx, base);\n\treturn;',
        'unconditional branch 0x8217BF20->0x8217C024')

    patch_file(target,
        '\t// b 0x8217c024\n\t// FATAL: unresolved function 0x8217C024 (no CallTarget in FunctionNode)\n\tREX_FATAL("Unresolved call from 0x8217BFEC to 0x8217C024");\n\treturn;',
        '\t// b 0x8217c024 (manually resolved)\n\tsub_8217C024(ctx, base);\n\treturn;',
        'unconditional branch 0x8217BFEC->0x8217C024')

    # 3. Fix conditional branch
    patch_file(target,
        '\t// beq 0x8217c024\n\tif (ctx.cr0.eq) REX_FATAL("Unresolved branch from 0x8217BF0C to 0x8217C024");',
        '\t// beq 0x8217c024 (manually resolved)\n\tif (ctx.cr0.eq) {\n\t\tsub_8217C024(ctx, base);\n\t\treturn;\n\t}',
        'conditional branch 0x8217BF0C->0x8217C024')

    return 0

if __name__ == '__main__':
    sys.exit(main())
