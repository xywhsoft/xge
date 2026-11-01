"""Offline two-module XRT identity/allocation/lifetime gate, GCC O0 and O2."""
from pathlib import Path
import argparse, hashlib, json, os, shutil, subprocess, sys, time
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'build'))
import host_xrt
from check_xui_lifetime import SOURCES


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--cache', type=Path, required=True)
    parser.add_argument('--cc', default='gcc')
    parser.add_argument('--sanitize', action='store_true')
    args = parser.parse_args()
    root, out, cache = args.root.resolve(), args.out.resolve(), args.cache.resolve()
    cc, nm = shutil.which(args.cc), shutil.which('nm')
    assert cc and nm
    out.mkdir(parents=True, exist_ok=False); cache.mkdir(parents=True, exist_ok=True)
    (out/'tmp').mkdir()
    env = dict(os.environ, TEMP=str(out/'tmp'), TMP=str(out/'tmp'), TMPDIR=str(out/'tmp'))
    if args.sanitize:
        assert os.name != 'nt'
        env.update(ASAN_OPTIONS='detect_leaks=1:halt_on_error=1', UBSAN_OPTIONS='halt_on_error=1')
    identity = hashlib.sha256(Path(cc).read_bytes()).hexdigest()
    tool_inputs = [Path(__file__), Path(host_xrt.__file__), Path(__file__).with_name('check_xui_lifetime.py')]
    tool_hashes = {str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in tool_inputs}
    report = dict(complete=False, passed=False, networkTests=False, sanitize=args.sanitize,
        compiler=cc, compilerSha256=identity, tools=tool_hashes, rows=[], objects=[], profiles=[])
    def save(): (out/'results.json').write_bytes((json.dumps(report, indent=2)+'\n').encode())
    def run(name, command, required=True):
        start = time.monotonic()
        p = subprocess.run(list(map(str,command)), cwd=root, env=env, capture_output=True, timeout=300)
        (out/(name+'.log')).write_bytes(p.stderr if name.endswith('-pre') else p.stdout+p.stderr)
        report['rows'].append(dict(name=name, command=list(map(str,command)), exitCode=p.returncode,
            seconds=round(time.monotonic()-start,3))); save()
        if required: assert p.returncode == 0, out/(name+'.log')
        return p
    generated = out/'generated'
    metadata = host_xrt.generate(root/'extlibs/xrt/xrt.h', generated)
    report['headerSha256'] = metadata['headerSha256']
    common = ['-std=c11', '-g', '-Wall', '-Wextra', '-Werror', '-Wno-unused-parameter',
        '-Wno-unused-function', '-Wno-cast-function-type', '-DXGE_DEBUGMODE=0',
        '-DXRT_MODULE_MEMORY_DEBUG', '-DXGE_XRT_HOST=1', '@config/includes.rsp', '-I'+generated.as_posix()]
    if os.name != 'nt': common += ['-fPIC']
    if args.sanitize: common += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer']
    force = ['-include','extlibs/xrt/xrt_config.h','-include',str(generated/'xge_host_xrt_imports.h')]
    libraries = ['-lm', '-lws2_32', '-liphlpapi', '-lgdi32', '-luser32', '-lshell32', '-lopengl32',
        '-lole32', '-loleaut32', '-lwinmm', '-lavrt'] if os.name == 'nt' else ['-lm','-ldl','-lpthread']
    snapshots = []
    for opt in ('O0','O2'):
        flags = [*common, '-'+opt]
        def compile_source(source, extra=(), key_name=None):
            all_flags = [*flags,*extra]
            name = key_name or Path(source).stem
            pre = run(name+'-'+opt+'-pre',[cc,*all_flags,'-E',source]).stdout
            pre_hash = hashlib.sha256(pre).hexdigest()
            key = hashlib.sha256(json.dumps([identity,str(root),all_flags,str(source),pre_hash]).encode()).hexdigest()
            obj, record = cache/(key+'.o'), cache/(key+'.json')
            reused = obj.exists() and record.exists() and json.loads(record.read_text())['sha256'] == hashlib.sha256(obj.read_bytes()).hexdigest()
            if not reused:
                run(name+'-'+opt+'-build',[cc,*all_flags,'-c',source,'-o',obj])
                record.write_bytes(json.dumps(dict(sha256=hashlib.sha256(obj.read_bytes()).hexdigest())).encode())
            report['objects'].append(dict(source=str(source),flags=all_flags,preprocessedSha256=pre_hash,
                sha256=hashlib.sha256(obj.read_bytes()).hexdigest(),reused=reused))
            snapshots.append((source,all_flags,pre_hash)); save()
            print(opt,name,'reuse' if reused else 'compile',flush=True)
            return obj
        native_flags = [*force,'-DXGE_DLL','-DXGE_BUILD_DLL','-DXUI_DLL','-DXUI_BUILD_DLL']
        objects = [compile_source(s,native_flags) for s in SOURCES if not s.endswith('xui_test_xrt_impl.c')]
        objects.append(compile_source('tests/xui/xui_host_xrt_probe.c',native_flags))
        undefined = run('imports-'+opt,[nm,'-u',*objects]).stdout.decode()
        names = host_xrt.required_symbols(undefined,metadata)
        host_xrt.write_slots(generated,names)
        provider = compile_source('src/xge/core/xge_host_xrt.c',native_flags)
        library = out/('xge-host-'+opt+('.dll' if os.name == 'nt' else '.so'))
        implib = out/('xge-host-'+opt+'.a')
        link = [cc,*flags,'-shared','-o',library,*objects,provider,*libraries]
        link += ['-Wl,--out-implib,'+str(implib)] if os.name == 'nt' else ['-Wl,--no-undefined']
        run('native-'+opt+'-link',link)
        symbols = run('native-'+opt+'-symbols',[nm,'-g','--defined-only',library]).stdout.decode()
        defined = set(__import__('re').findall(r'\b((?:__)?xrt[A-Z]\w*)\s*$',symbols,__import__('re').M))
        assert not defined, 'Hosted module contains XRT implementation: '+str(defined)
        report['profiles'].append(dict(optimization=opt,requiredSymbols=names,xrtDefinitions=sorted(defined),library=str(library)))
        host_flags = ['-DXGE_DLL','-DXUI_DLL'] if os.name == 'nt' else []
        # XRT itself must not see consumer DLL import macros.
        runtime = compile_source('tests/xui/xui_test_xrt_impl.c')
        test = compile_source('tests/xui/xui_host_xrt_test.c',host_flags)
        lifetime = compile_source('tests/xui/xui_lifetime_observer_test.c',host_flags+['-Dmain=xuiHostLifetimeMain',
            '-DxuiInternalOperationEnter=xgeHostXrtTestEnter','-DxuiInternalOperationLeave=xgeHostXrtTestLeave'])
        exe = out/('host-'+opt+('.exe' if os.name == 'nt' else '.out'))
        run('host-'+opt+'-link',[cc,*flags,*(['-no-pie'] if args.sanitize else []),runtime,test,lifetime,
            implib if os.name == 'nt' else library,*libraries,*([] if os.name == 'nt' else ['-Wl,-rpath,'+str(out)]),'-o',exe])
        run('host-'+opt+'-run',[exe],required=False)
    report['inputsUnchanged'] = all(hashlib.sha256(subprocess.check_output([cc,*f,'-E',s],cwd=root,env=env)).hexdigest()==h for s,f,h in snapshots)
    report['inputsUnchanged'] &= all(hashlib.sha256(Path(p).read_bytes()).hexdigest()==h for p,h in tool_hashes.items())
    report['inputsUnchanged'] &= hashlib.sha256((root/'extlibs/xrt/xrt.h').read_bytes()).hexdigest()==metadata['sourceSha256']
    report['complete'] = True
    report['passed'] = report['inputsUnchanged'] and all(r['exitCode']==0 for r in report['rows'])
    save(); assert report['passed'],out/'results.json'


if __name__ == '__main__': main()
