#!/usr/bin/env python3
"""Build CRISP on macOS with clang-cl + lld-link against the VS2008 headers and libraries.

The source lists come from the VS2008 .vcproj files, so both builds compile the same files.
Compiler and linker flags mirror the VS2008 Release configuration.

Usage: mac/build.py [--config Release|Debug] [-j N] [--clean]
"""
import argparse
import json
import re
import os
import shutil
import subprocess
import sys
import xml.etree.ElementTree as ET
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path, PureWindowsPath

ROOT = Path(__file__).resolve().parent.parent
VS = ROOT / 'mac' / 'vs2008'
LLVM = Path(os.environ.get('LLVM_BIN', '/opt/homebrew/opt/llvm@22/bin'))
LLD = Path(os.environ.get('LLD_BIN', '/opt/homebrew/opt/lld@22/bin'))
ASM = ROOT / 'mac' / 'tools' / 'bin' / 'jwasm'

SYSTEM_INCLUDES = [VS / 'vc/include', VS / 'vc/atlmfc/include', VS / 'sdk/include', ROOT / 'external_libs']
LIB_PATHS = [VS / 'vc/lib', VS / 'vc/atlmfc/lib', VS / 'sdk/lib']
WIN_LIBS = 'kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib'.split()

COMMON_DEFINES = ['WIN32', '_MBCS', '_VC80_UPGRADE=0x0600', '_SECURE_SCL=0', '_HAS_ITERATOR_DEBUGGING=0',
                  # clang rejects old boost's own redeclaration of Sleep(); MSVC let it slide
                  'BOOST_USE_WINDOWS_H']

PROJECTS = {
    'Math': dict(
        vcproj='Math.Src/Math.vcproj', kind='lib',
        includes=['Math.Src', 'CRISP2.Src'], defines=['_LIB'], cflags=['/EHa', '/Zp8'],
    ),
    'SpinStar': dict(
        vcproj='SpinStar.vc60/SpinStar.vcproj', kind='dll',
        includes=['Math.Src', 'CRISP2.Src'], defines=['_WINDOWS', '_USRDLL', '_WINDLL'], cflags=['/EHa', '/Zp8'],
        rc_lang='0x41d', deffile='SpinStar.vc60/SpinStar.def',
        libs=['nafxcw.lib', 'libcpmt.lib'] + WIN_LIBS, deps=['Math'],
    ),
    'PeakFit': dict(
        vcproj='PeakFit.Src/PeakFit.vcproj', kind='dll',
        includes=[], defines=['_WINDOWS', '_USRDLL', '_WINDLL'], cflags=['/EHsc', '/GF', '/Gy'],
        rc_lang='0x419', deffile='PeakFit.Src/PeakFit.def',
        libs=['nafxcw.lib', 'libcpmt.lib', 'delayimp.lib'] + WIN_LIBS, ldflags=['/delayload:oleacc.dll'],
    ),
    'CRISP2': dict(
        vcproj='CRISP2.vc60/CRISP2.vcproj', kind='exe',
        includes=['Math.Src', 'SpinStar.Src'], defines=['_WINDOWS'], cflags=['/EHa', '/Zp8'],
        rc_lang='0x409', rc_includes=['CRISP2.Src'],
        libs=['comctl32.lib', 'winmm.lib'] + WIN_LIBS, deps=['Math', 'SpinStar'],
    ),
}
ORDER = ['Math', 'SpinStar', 'PeakFit', 'CRISP2']

# These use macros that close an _asm { } block, emit single-line _asm statements and reopen
# the block. After expansion everything lands on one line, which MSVC accepts but clang's
# inline assembly parser does not, so they are preprocessed first and split into lines.
ASM_MACRO_SOURCES = {'CRISP2.Src/Complex/BM_templ.cpp', 'CRISP2.Src/nfft.cpp'}


def compiler_tool(element):
    return next((t for t in element.iter('Tool') if t.get('Name') == 'VCCLCompilerTool'), None)


def vcproj_files(vcproj, config):
    """Files of a .vcproj built in the given configuration.

    Returns (sources, asm, rc, libs) where sources are (path, pch_mode, pch_header) and
    pch_mode follows UsePrecompiledHeader: 0 none, 1 create (/Yc), 2 use (/Yu).
    """
    path = ROOT / vcproj
    tree = ET.parse(path)
    cfg_name = f'{config}|Win32'
    cfg = next(c for c in tree.getroot().iter('Configuration') if c.get('Name') == cfg_name)
    tool = compiler_tool(cfg)
    default_mode = int(tool.get('UsePrecompiledHeader') or 0)
    default_header = tool.get('PrecompiledHeaderThrough') or 'stdafx.h'

    sources, asm, rc, libs = {}, [], None, []
    for f in tree.iter('File'):
        file_cfg = next((fc for fc in f.findall('FileConfiguration') if fc.get('Name') == cfg_name), None)
        if file_cfg is not None and file_cfg.get('ExcludedFromBuild') == 'true':
            continue
        p = (path.parent / PureWindowsPath(f.get('RelativePath')).as_posix()).resolve()
        ext = p.suffix.lower()
        if ext in ('.c', '.cpp', '.cxx'):
            mode, header = default_mode, default_header
            file_tool = compiler_tool(file_cfg) if file_cfg is not None else None
            if file_tool is not None:
                mode = int(file_tool.get('UsePrecompiledHeader') or mode)
                header = file_tool.get('PrecompiledHeaderThrough') or header
            sources[p] = (p, mode, header)
        elif ext in ('.asm', '.ass'):
            asm.append(p)
        elif ext == '.rc':
            rc = p
        elif ext == '.lib':
            # .lib files added to a project are linked by VS
            libs.append(p)
    return sorted(sources.values()), sorted(set(asm)), rc, libs


VERBOSE = False


def run(cmd, cwd=None):
    if VERBOSE:
        print(' '.join(f'"{c}"' if ' ' in str(c) else str(c) for c in cmd))
    r = subprocess.run([str(c) for c in cmd], cwd=cwd, capture_output=True, text=True, errors='replace')
    return r.returncode, (r.stdout + r.stderr).strip()


def needs_rebuild(target, depfile, sources):
    if not target.exists():
        return True
    t = target.stat().st_mtime
    deps = list(sources)
    if depfile and depfile.exists():
        text = depfile.read_text(errors='replace').replace('\\\n', ' ')
        deps += [Path(d.replace('\\ ', ' ')) for d in text.split(':', 1)[1].split() if d]
    return any(not Path(d).exists() or Path(d).stat().st_mtime > t for d in deps)


def pch_overlay(sources, objdir):
    """VFS overlay that makes every /Yu file see the project's own precompiled header.

    MSVC's /Yu matches '#include "stdafx.h"' by name, so e.g. CRISP2.Src/trace.cpp compiled in
    SpinStar uses SpinStar's stdafx.h, and CRISP2.Src/Complex/*.cpp use CRISP2.Src/StdAfx.h
    although there is none in their folder. Clang resolves the include by path instead, so
    place a virtual copy of the right header next to each such source.
    """
    def find(directory, name):
        return next((f for f in directory.iterdir() if f.name.lower() == name.lower()), None)

    creator = next((src for src, mode, _ in sources if mode == 1), None)
    if creator is None:
        return None
    dirs = {}
    for src, mode, header in sources:
        own = find(creator.parent, header)
        if mode == 2 and own and src.parent != creator.parent:
            existing = find(src.parent, header)
            dirs.setdefault(str(src.parent), {})[existing.name if existing else header] = str(own)
    if not dirs:
        return None
    overlay = objdir / 'pch-overlay.yaml'
    overlay.write_text(json.dumps({
        'version': 0, 'case-sensitive': 'false',
        'roots': [{'name': d, 'type': 'directory',
                   'contents': [{'name': n, 'type': 'file', 'external-contents': t} for n, t in files.items()]}
                  for d, files in dirs.items()],
    }, indent=1))
    return overlay


def compile_one(src, obj, pch_mode, pch_header, pch, project, config, projdir, overlay, pch_dir):
    depfile = obj.with_suffix('.d')
    extra_deps = [pch] if pch_mode == 2 else []
    if not needs_rebuild(obj, depfile, [src] + extra_deps):
        return src, 0, ''
    debug = config == 'Debug'
    cmd = [LLVM / 'clang-cl', '--target=i686-pc-windows-msvc', '-fms-compatibility-version=15.00',
           '/nologo', '/c', '/TP', '/Od', '/W3', '/MTd' if debug else '/MT', '/Z7',
           '-Wno-everything',
           '/clang:-MD', f'/clang:-MF{depfile}', f'/Fo{obj}']
    if pch_mode == 1:
        cmd += [f'/Yc{pch_header}', f'/Fp{pch}']
    elif pch_mode == 2:
        cmd += [f'/Yu{pch_header}', f'/Fp{pch}']
    if overlay:
        cmd.append(f'/clang:-ivfsoverlay{overlay}')
    cmd += project['cflags']
    for d in COMMON_DEFINES + project['defines'] + (['_DEBUG'] if debug else ['NDEBUG']):
        cmd.append(f'/D{d}')
    for inc in SYSTEM_INCLUDES:
        cmd += ['/imsvc', inc]
    # cl.exe ran from the project folder, so headers there were found as well
    cmd += [f'/I{src.parent}', f'/I{projdir}'] + [f'/I{ROOT / i}' for i in project['includes']]
    if src.relative_to(ROOT).as_posix() in ASM_MACRO_SOURCES:
        # Without the precompiled header, the redirected stdafx.h needs its own folder for its includes
        return compile_split_asm(cmd + ([f'/I{pch_dir}'] if pch_dir else []), src, obj, projdir)
    # '--' so the absolute /Users/... path is not parsed as a clang-cl option
    cmd += ['--', src]
    code, out = run(cmd, cwd=projdir)
    return src, code, out


def compile_split_asm(cmd, src, obj, projdir):
    cmd = [c for c in cmd if not str(c).startswith(('/Yu', '/Yc', '/Fp'))]
    pp = [c if c != '/c' else '/E' for c in cmd if not str(c).startswith('/Fo')] + ['--', src]
    if VERBOSE:
        print(' '.join(map(str, pp)))
    r = subprocess.run([str(c) for c in pp], cwd=projdir, capture_output=True, text=True, errors='replace')
    if r.returncode:
        return src, r.returncode, r.stderr
    split = re.sub(r'(?<!\w)(_{1,2}asm)(?!\w)', r'\n\1', r.stdout)
    # Clang compiles every _asm statement separately, so labels cannot be shared between them.
    # Turn "} _asm a _asm b _asm {" back into plain lines a, b of one continuous block.
    split = re.sub(r'\}\s*((?:\n_{1,2}asm(?![ \t]*\{)[^\n]*)*)\n_{1,2}asm[ \t]*\{',
                   lambda m: re.sub(r'\n_{1,2}asm', '\n', m.group(1)) + '\n', split)
    ii = obj.with_suffix('.ii')
    ii.write_text(split)
    # At /Od clang computes operand addresses into registers that these blocks use themselves
    # ("inline assembly requires more registers than available"); /O1 addresses them directly
    cc = ['/O1' if c == '/Od' else c for c in cmd if not str(c).startswith('/clang:-M')] + ['--', ii]
    code, out = run(cc, cwd=projdir)
    return src, code, out


def assemble_one(src, obj):
    if not needs_rebuild(obj, None, [src]):
        return src, 0, ''
    code, out = run([ASM, '-nologo', '-Cp', '-coff', '-safeseh', f'-Fo{obj}', src.name], cwd=src.parent)
    return src, code, '' if code == 0 else out


def obj_path(objdir, src):
    # Disambiguate same-named files from different folders (e.g. Math/MarqMin.cpp)
    stem = src.relative_to(ROOT).as_posix().replace('/', '_').rsplit('.', 1)[0]
    return objdir / f'{stem}.obj'


def wait_all(futures):
    failed = False
    for fut in futures:
        src, code, out = fut.result()
        if out:
            print(out)
        if code:
            failed = True
            print(f'FAILED: {src.relative_to(ROOT)}')
    return failed


def portable_rc(rc, objdir):
    """Copy of an .rc file that llvm-rc on macOS can compile.

    - MFC's TOOLBAR statement is unsupported; emit the same binary layout as a custom
      resource of type RT_TOOLBAR (241): version 1, width, height, count, then the IDs.
    - "dir\\file.bmp" paths only work on Windows hosts; use forward slashes.
    """
    text = rc.read_bytes().decode('latin-1')

    def toolbar(m):
        name, width, height, body = m.group(1), m.group(2), m.group(3), m.group(4)
        items = []
        for line in body.splitlines():
            parts = line.split()
            if parts and parts[0] == 'BUTTON':
                items.append(parts[1])
            elif parts and parts[0] == 'SEPARATOR':
                items.append('0')
        return f'{name} 241\nBEGIN\n    {", ".join(["1", width, height, str(len(items))] + items)}\nEND'

    text = re.sub(r'^(\w+)\s+TOOLBAR\s+(\d+)\s*,\s*(\d+)\s*\nBEGIN\n(.*?)^END', toolbar, text,
                  flags=re.M | re.S)
    text = re.sub(r'"([^"\n]*\\\\[^"\n]*\.\w+)"', lambda m: '"' + m.group(1).replace('\\\\', '/') + '"', text)
    out = objdir / rc.name
    out.write_bytes(text.encode('latin-1'))
    return out


def crt_member(lib, member, out):
    """Extract one object file from a CRT library (cached next to the build output)."""
    if not out.exists():
        out.parent.mkdir(parents=True, exist_ok=True)
        full = next(m for m in run([LLVM / 'llvm-ar', 't', VS / 'vc/lib' / lib])[1].splitlines()
                    if m.lower().endswith('\\' + member))
        data = subprocess.run([str(LLVM / 'llvm-ar'), 'p', str(VS / 'vc/lib' / lib), full],
                              capture_output=True, check=True).stdout
        out.write_bytes(data)
    return out


def build_project(name, config, jobs, outdir):
    project = PROJECTS[name]
    objdir = ROOT / 'mac' / 'build' / config / name
    objdir.mkdir(parents=True, exist_ok=True)
    projdir = (ROOT / project['vcproj']).parent
    sources, asm, rc, project_libs = vcproj_files(project['vcproj'], config)
    pch = objdir / f'{name}.pch'
    overlay = pch_overlay(sources, objdir)
    pch_dir = next((src.parent for src, mode, _ in sources if mode == 1), None)

    objs = [obj_path(objdir, src) for src, _, _ in sources] + [obj_path(objdir, a) for a in asm]
    with ThreadPoolExecutor(jobs) as pool:
        # The precompiled header must exist before the files that use it
        creators = [s for s in sources if s[1] == 1]
        failed = wait_all([pool.submit(compile_one, src, obj_path(objdir, src), mode, header, pch, project, config, projdir, overlay, pch_dir)
                           for src, mode, header in creators])
        if not failed:
            failed = wait_all(
                [pool.submit(compile_one, src, obj_path(objdir, src), mode, header, pch, project, config, projdir, overlay, pch_dir)
                 for src, mode, header in sources if mode != 1] +
                [pool.submit(assemble_one, a, obj_path(objdir, a)) for a in asm])
    if failed:
        sys.exit(f'{name}: compilation failed')

    if rc:
        res = objdir / f'{name}.res'
        if needs_rebuild(res, None, [rc]):
            cmd = [LLVM / 'llvm-rc', '/nologo', '/C', '1252', '/L', project['rc_lang'],
                   '/D', '_DEBUG' if config == 'Debug' else 'NDEBUG', '/D', '_VC80_UPGRADE=0x0600', '/D', 'WIN32',
                   '/I', rc.parent]
            for inc in [ROOT / i for i in project.get('rc_includes', [])] + SYSTEM_INCLUDES:
                cmd += ['/I', inc]
            # relative name: llvm-rc would parse an absolute /Users/... path as an option
            cmd += ['/FO', res, portable_rc(rc, objdir).name]
            code, out = run(cmd, cwd=objdir)
            if code:
                # llvm-rc can leave a partial .res behind, which would look up to date next time
                res.unlink(missing_ok=True)
                sys.exit(f'{name}: resource compilation failed\n{out}')
        objs.append(res)

    if project['kind'] == 'lib':
        target = objdir / f'{name}.lib'
        code, out = run([LLVM / 'llvm-lib', '/nologo', f'/out:{target}'] + objs)
    else:
        ext = 'dll' if project['kind'] == 'dll' else 'exe'
        target = outdir / f'{name}.{ext}'
        cmd = [LLD / 'lld-link', '/nologo', '/machine:x86', '/subsystem:windows', '/debug',
               f'/pdb:{outdir / (name + ".pdb")}', '/dynamicbase:no', '/opt:ref',
               '/manifest:embed', "/manifestuac:level='asInvoker' uiAccess='false'",
               f'/out:{target}']
        if project['kind'] == 'dll':
            # lld-link rejects the obsolete DESCRIPTION statement that link.exe ignores
            deffile = objdir / f'{name}.def'
            lines = (ROOT / project['deffile']).read_text(errors='replace').splitlines()
            deffile.write_text('\n'.join(l for l in lines if not l.strip().upper().startswith('DESCRIPTION')) + '\n')
            # Explicit entry so the DLL startup object (dllcrt0) is chosen over crt0 from libcmt
            cmd += ['/dll', f'/def:{deffile}', f'/implib:{objdir / (name + ".lib")}']
        # lld-link can resolve CRT globals from the console startup object (crt0) before it gets
        # to the real entry point, which then clashes with dllcrt0/wincrt0; put the right one first
        crtlib = 'libcmtd.lib' if config == 'Debug' else 'libcmt.lib'
        startup = 'dllcrt0.obj' if project['kind'] == 'dll' else 'wincrt0.obj'
        cmd.append(crt_member(crtlib, startup, ROOT / 'mac' / 'build' / config / f'{startup}-{crtlib}.obj'))
        libs = project['libs']
        if config == 'Debug':
            debug_libs = {'nafxcw.lib': 'nafxcwd.lib', 'libcpmt.lib': 'libcpmtd.lib'}
            libs = [debug_libs.get(l, l) for l in libs]
        # MFC must be linked before the CRT, so take over the default library order
        for lib in ('nafxcw.lib', 'nafxcwd.lib', 'libcpmt.lib', 'libcpmtd.lib'):
            if lib in libs:
                cmd.append(f'/nodefaultlib:{lib}')
        cmd += [f'/libpath:{p}' for p in LIB_PATHS]
        cmd += project.get('ldflags', []) + objs + libs + project_libs
        cmd += [ROOT / 'mac' / 'build' / config / dep / f'{dep}.lib' for dep in project.get('deps', [])]
        # Relative #pragma comment(lib) paths are relative to the project folder, as with link.exe
        code, out = run(cmd, cwd=projdir)
    if out:
        print(out)
    if code:
        sys.exit(f'{name}: link failed')
    print(f'{name}: {target.relative_to(ROOT)}')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--config', default='Release', choices=['Release', 'Debug'])
    parser.add_argument('-j', '--jobs', type=int, default=4)
    parser.add_argument('--clean', action='store_true')
    parser.add_argument('-v', '--verbose', action='store_true', help='print commands')
    parser.add_argument('projects', nargs='*', default=ORDER)
    args = parser.parse_args()
    global VERBOSE
    VERBOSE = args.verbose

    if not (VS / 'vc/include').exists():
        sys.exit(f'Missing VS2008 headers/libraries in {VS}')
    if args.clean:
        shutil.rmtree(ROOT / 'mac' / 'build' / args.config, ignore_errors=True)
    outdir = ROOT / 'mac' / 'build' / args.config / 'bin'
    outdir.mkdir(parents=True, exist_ok=True)
    for name in ORDER:
        if name in args.projects:
            build_project(name, args.config, args.jobs, outdir)


if __name__ == '__main__':
    main()
