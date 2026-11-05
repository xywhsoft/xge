#!/usr/bin/env python3
"""Offline loader identity and strict relink checks; GCC only, no TCC or UI."""
from pathlib import Path
import argparse
import hashlib
import importlib.util
import json
import os
import re
import shutil
import struct
import subprocess
import time


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def pe(path):
    data = Path(path).read_bytes()
    word = lambda at: struct.unpack_from('<H', data, at)[0]
    dword = lambda at: struct.unpack_from('<I', data, at)[0]
    header = dword(0x3c)
    assert data[header:header+4] == b'PE\0\0'
    optional = header + 24
    directories = optional + (112 if word(optional) == 0x20b else 96)
    sections = optional + word(header+20)
    def offset(rva):
        for i in range(word(header+6)):
            at = sections + 40*i
            size, address, raw_size, raw = struct.unpack_from('<IIII', data, at+8)
            if address <= rva < address + max(size, raw_size):
                return raw + rva - address
        raise ValueError('Unmapped PE RVA')
    def text(rva):
        at = offset(rva)
        return data[at:data.index(b'\0', at)].decode('ascii')
    imports, names, image = [], [], None
    if dword(directories):
        table = offset(dword(directories))
        image = text(dword(table+12))
        names = [text(dword(offset(dword(table+32))+4*i)) for i in range(dword(table+24))]
    if dword(directories+8):
        at = offset(dword(directories+8))
        while any(data[at:at+20]):
            imports.append(text(dword(at+12)))
            at += 20
    return dict(image=image, imports=imports, exports=sorted(names))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('root', 'engine', 'previous', 'out'):
        p.add_argument('--'+name, type=Path, required=True)
    a = p.parse_args()
    root, engine, previous, out = [getattr(a, n).resolve() for n in ('root','engine','previous','out')]
    out.mkdir(parents=True, exist_ok=False)
    spec = importlib.util.spec_from_file_location('xge_relink_test', root/'tools/build/relink.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    metadata = json.loads((engine/'build-report.json').read_text())
    old = json.loads((previous/'build-report.json').read_text())
    windows = os.name == 'nt'
    binary = engine / metadata['runtime_image']
    old_binary = previous / ('xge.dll' if windows else 'libxge.so')
    cc = shutil.which('gcc')
    assert cc
    pins = [Path(__file__).resolve(), root/'tools/build/build.py', root/'tools/build/relink.py',
            engine/'build-report.json', previous/'build-report.json', binary, old_binary,
            engine/'xge_build_config.h', previous/'xge_build_config.h', Path(cc)]
    if windows:
        pins.append(engine/metadata['import_library'])
    inputs = {str(path): sha(path) for path in pins}
    report = dict(complete=False, passed=False, scope=__doc__, network=False, gpu=False,
                  actualClipboard=False, tccPeExecuted=False, inputs=inputs, rows=[])
    def save():
        (out/'results.json').write_text(json.dumps(report, indent=2)+'\n')
    def check(name, condition, **facts):
        report['rows'].append(dict(name=name, passed=bool(condition), **facts))
        save()
        assert condition, name
    def run(name, command, expected=0, env=None):
        start = time.monotonic()
        result = subprocess.run(list(map(str,command)), cwd=root, env=env, capture_output=True, timeout=90)
        log = out/(name+'.log')
        log.write_bytes(result.stdout+result.stderr)
        check(name, result.returncode == expected, command=list(map(str,command)),
              exitCode=result.returncode, expected=expected, seconds=time.monotonic()-start,
              logSHA256=sha(log))
        return result.stdout.decode('utf8', errors='replace')
    check('only-qualified-objects', not metadata['compiled'] and set(metadata['reused']) == set(old['compiled']) | set(old['reused']),
          compiled=len(metadata['compiled']), reused=len(metadata['reused']))
    check('abi-configuration-unchanged', (engine/'xge_build_config.h').read_bytes() == (previous/'xge_build_config.h').read_bytes())
    qualified = metadata['qualified_object_build']
    check('qualified-inputs-unchanged', qualified['inputs_unchanged'] and qualified['report_sha256'] == sha(previous/'build-report.json'))
    check('all-object-content-identical', all(sha(row['object']) == row['sha256'] for row in qualified['objects']))
    for host in (False, True):
        for debug in (False, True):
            for win in (False, True):
                value = module.image_names(3,host,debug,win)
                stem = ('xge3_host' if host else 'xge') + ('dbg' if debug else '')
                expected = stem+'.dll' if win else 'lib'+stem+'.so'
                check('names-'+str((host,debug,win)), value['runtime_image'] == expected and value['import_library'] == (stem+'.lib' if win else None))

    # A small, fully synthetic qualified cache exercises fail-closed admission.
    q = out/'qualification';oldroot=q/'old-source';newroot=q/'new-source';base=q/'old-engine';target=q/'new-engine'
    for directory in (oldroot,newroot,base/'obj',target):
        directory.mkdir(parents=True,exist_ok=True)
    compiler = q/'compiler';compiler.write_bytes(b'never executed')
    stamp = lambda path:[Path(path).stat().st_mtime_ns,Path(path).stat().st_size]
    digest = lambda value:hashlib.sha256(json.dumps(value,sort_keys=True).encode()).hexdigest()
    identity = [str(compiler),stamp(compiler),'test compiler']
    flags = ['-O2','-I'+(target/'host-xrt').as_posix()]
    oldflags = ['-O2','-I'+(base/'host-xrt').as_posix()]
    for directory in (oldroot,newroot):
        (directory/'unit.c').write_bytes(b'int unit;\n')
    for directory in (base,target):
        (directory/'xge_build_config.h').write_bytes(b'fixed ABI\n')
    obj = base/'obj/unit.c.o';dep=obj.with_suffix('.d');cache=obj.with_suffix('.json')
    obj.write_bytes(b'qualified object');dep.write_bytes(b'unit.o: unit.c\n')
    command=[str(compiler),*oldflags,'-MD','-MF',str(dep),'-MT',str(obj),'-c','unit.c','-o',str(obj)]
    cache_data=dict(key=digest([identity,str(compiler),stamp(compiler),command]),deps={'unit.c':stamp(oldroot/'unit.c')})
    cache.write_text(json.dumps(cache_data))
    base_report=dict(api_major=3,public_abi_schema=2,profile='default',defines=[],compiler=identity,
                     xrt_mode='host',features=[],linked=True,flags=oldflags,compiled=['unit.c'],reused=[])
    (base/'build-report.json').write_text(json.dumps(base_report))
    def fresh():
        return module.QualifiedRelink(newroot,target,base,oldroot,identity,flags,'default',[],3,[],False,digest,stamp)
    def rejects(name, action):
        try:
            action()
        except (RuntimeError,OSError):
            check(name,True)
        else:
            check(name,False)
    valid=fresh();valid.object('unit.c',str(compiler),flags,False)
    check('qualified-valid-fixture',valid.finish()['inputs_unchanged'])
    (target/'xge_build_config.h').write_bytes(b'wrong ABI\n');rejects('reject-abi-change',fresh)
    (target/'xge_build_config.h').write_bytes(b'fixed ABI\n')
    (newroot/'unit.c').write_bytes(b'int changed;\n')
    rejects('reject-current-dependency-change',lambda:fresh().object('unit.c',str(compiler),flags,False))
    (newroot/'unit.c').write_bytes(b'int unit;\n')
    bad=dict(cache_data,key='wrong-command');cache.write_text(json.dumps(bad))
    rejects('reject-cache-command-change',lambda:fresh().object('unit.c',str(compiler),flags,False))
    cache.write_text(json.dumps(cache_data))
    rejects('reject-output-inside-qualified-source',lambda:module.check_locations(newroot,oldroot/'new-output',base,oldroot))
    rejects('reject-output-inside-qualified-output',lambda:module.check_locations(newroot,base/'new-output',base,oldroot))
    rejects('reject-same-source-tree',lambda:module.check_locations(oldroot,target,base,oldroot))
    dep.write_bytes(b'unit.o: unit.c missing.h\n')
    rejects('reject-dependency-cache-inventory-mismatch',lambda:fresh().object('unit.c',str(compiler),flags,False))
    dep.write_bytes(b'unit.o: unit.c\n')
    rejects('reject-incomplete-inventory',lambda:fresh().finish())
    valid=fresh();valid.object('unit.c',str(compiler),flags,False);obj.write_bytes(b'changed after admit')
    rejects('reject-object-change-during-link',valid.finish)

    probe = out/'probe.c'
    probe.write_text('extern unsigned int xgeGetBuildFeatures(void);\nint main(void){return xgeGetBuildFeatures() ? 0:42;}\n')
    if windows:
        actual=pe(binary);prior=pe(old_binary)
        check('pe-export-image-name',actual['image']==metadata['runtime_image'],image=actual['image'])
        check('pe-export-set-identical',actual['exports']==prior['exports'],exports=len(actual['exports']))
        for kind, library in (('dll',binary),('implib',engine/metadata['import_library'])):
            exe=out/('actual-'+kind+'.exe');run('actual-'+kind+'-link',[cc,'-O2',probe,library,'-o',exe])
            imports=pe(exe)['imports']
            check('actual-'+kind+'-iat',metadata['runtime_image'] in imports and 'xge.dll' not in imports,imports=imports)
    else:
        dynamic=run('elf-dynamic',[shutil.which('readelf'),'-d',binary])
        check('elf-soname',re.search(r'\(SONAME\).*\['+re.escape(metadata['runtime_soname'])+r'\]',dynamic) is not None)
        nm=shutil.which('nm')
        symbols=lambda text:sorted(line.split()[-1] for line in text.splitlines() if line.strip())
        new_symbols=run('elf-new-exports',[nm,'-D','--defined-only',binary]);old_symbols=run('elf-old-exports',[nm,'-D','--defined-only',old_binary])
        check('elf-export-set-identical',symbols(new_symbols)==symbols(old_symbols))
        exe=out/'actual-probe';run('actual-link',[cc,'-O2',probe,binary,'-o',exe])
        needed=run('elf-needed',[shutil.which('readelf'),'-d',exe])
        check('elf-needed-basename',re.search(r'\(NEEDED\).*\['+re.escape(metadata['runtime_image'])+r'\]',needed) is not None and str(engine) not in needed)
        env=dict(os.environ,LD_LIBRARY_PATH=str(engine))
        run('actual-cpu-query',[exe],env=env)

    # Both DLLs deliberately live beside the program. Exact image identity must
    # select the host image, regardless of an embedded GUI image named xge.
    collision=out/'collision';host=out/'fake-old-host';collision.mkdir();host.mkdir()
    fake=out/'fake.c';fake.write_text(('__declspec(dllexport) ' if windows else '__attribute__((visibility("default"))) ')+'unsigned int xgeGetBuildFeatures(void){return FEATURES;}\n')
    probe.write_text('extern unsigned int xgeGetBuildFeatures(void);\nint main(void){return xgeGetBuildFeatures()==222u ? 0:42;}\n')
    old_name='xge.dll' if windows else 'libxge.so';new_name=metadata['runtime_image']
    def fake_build(name,path,features):
        extra=[] if windows else ['-fPIC','-Wl,-soname,'+path.name]
        run(name,[cc,'-shared','-O2',*extra,'-DFEATURES='+str(features),fake,'-o',path])
    fake_build('fake-embedded',collision/old_name,111)
    fake_build('fake-old-host',host/old_name,222)
    fake_build('fake-new-host',collision/new_name,222)
    suffix='.exe' if windows else ''
    for kind,library,expected in (('old',host/old_name,42),('new',collision/new_name,0)):
        exe=collision/(kind+suffix)
        run('fake-'+kind+'-link',[cc,'-O2',probe,library,'-o',exe])
        env=None if windows else dict(os.environ,LD_LIBRARY_PATH=str(collision))
        run('fake-'+kind+'-run',[exe],expected,env)
    report['complete']=True
    report['inputsUnchanged']=all(sha(name)==value for name,value in inputs.items())
    report['passed']=report['inputsUnchanged'] and all(row['passed'] for row in report['rows'])
    save()
    print(len(report['rows']),'checks',report['passed'],flush=True)
    return not report['passed']


if __name__=='__main__':
    raise SystemExit(main())
