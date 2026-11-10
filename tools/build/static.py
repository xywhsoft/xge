#!/usr/bin/env python3
"""Build a direct-XRT static XGE archive without changing the normal DLL build.

The existing profile/configuration guards select the same physical sources.
single/xge.c remains one aggregate member; XUI/3D and optional HarfBuzz retain
their normal translation-unit boundaries. Every compiled function/data item
gets its compiler section. Actual final reachability is the linker's job.
No XRT implementation or generated host import slots enter this archive.
"""
from pathlib import Path
from static_support import windows_support,linux_support
import egl
import argparse,hashlib,json,os,re,shlex,shutil,subprocess,sys,time


def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def reuse_dependencies(dependencies, source_root, target_root):
    """Validate the files used by the target build, not the parent's checkout."""
    original_root=Path(source_root)
    source_root,target_root=original_root.resolve(),Path(target_root).resolve()
    rebound={}
    for name,digest in dependencies.items():
        path=Path(name)
        if path.is_relative_to(original_root):path=target_root/path.relative_to(original_root)
        else:
            path=path.resolve()
            if path.is_relative_to(source_root):path=target_root/path.relative_to(source_root)
        if not path.is_file()or sha(path)!=digest:return None
        rebound[str(path)]=digest
    return rebound

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root',type=Path,default=Path(__file__).resolve().parents[2])
    p.add_argument('--out',type=Path,required=True)
    p.add_argument('--profile',default='default')
    p.add_argument('--reuse-build',type=Path,help='Verified same-profile object reuse from a previous static build')
    p.add_argument('--define',action='append',default=[])
    p.add_argument('--debug-api',action='store_true')
    p.add_argument('--runtime-notice',type=Path,action='append',default=[],help='Additional exact compiler runtime license texts to preserve')
    a=p.parse_args();root,out=a.root.resolve(),a.out.resolve();windows=os.name=='nt'
    if out==root or out.is_relative_to(root/'include') or out.is_relative_to(root/'src'):
        p.error('Keep generated static materials outside source/include directories')
    profiles=json.loads((root/'config/profiles.json').read_text(encoding='utf-8'))['profiles']
    if a.profile not in profiles:p.error('Unknown profile: '+a.profile)
    for value in a.define:
        if not re.fullmatch(r'(?:XGE|XUI|XGE3D)_ENABLE_[A-Z0-9_]+=[01]',value):
            p.error('Feature overrides must be exact NAME=0/1 feature switches: '+value)
    if len({v.split('=')[0] for v in a.define})!=len(a.define):p.error('Duplicate feature override')
    cc=shutil.which(os.environ.get('CC','gcc'));ar=shutil.which(os.environ.get('AR','ar'));nm=shutil.which(os.environ.get('NM','nm'))
    if not all((cc,ar,nm)):p.error('GCC, ar and nm are required')
    out.mkdir(parents=True,exist_ok=True);objects=out/'obj';objects.mkdir(exist_ok=True)
    started=time.monotonic();inputs={};commands=[];compiled=[];reused=[];rows=[]
    parent={}
    if a.reuse_build:
        parent_path=a.reuse_build.resolve()/'build-report.json';parent=json.loads(parent_path.read_text(encoding='utf-8'))
        if not parent['complete']or not parent['passed']or parent['profile']!=a.profile:raise RuntimeError('Invalid same-profile reuse parent')
        inputs[str(parent_path)]=sha(parent_path)
    parent_rows={row['source']:row for row in parent.get('objects',[])}
    for folder in ('include','single','src','modules','extlibs','config','platform','res'):
        for path in (root/folder).rglob('*'):
            if path.is_file() and '__pycache__'not in path.parts and path.suffix.lower()in('.c','.h','.cc','.cpp','.inc','.inl','.def','.json','.rsp','.ico','.rc','.txt'):
                inputs[str(path)]=sha(path)
    for path in (Path(__file__).resolve(),Path(__file__).with_name('static_support.py').resolve(),Path(cc),Path(ar),Path(nm)):inputs[str(path)]=sha(path)
    report=dict(complete=False,passed=False,scope=__doc__,source_root=str(root),profile=a.profile,
        defines=a.define,debug_api=a.debug_api,xrt_mode='direct-external',xrt_implementation=False,
        host_xrt_import_slots=False,public_abi_schema=2,inputs=inputs,commands=commands)
    def save():(out/'build-report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    def run(name,args,**kwargs):
        args=[v.as_posix() if isinstance(v,Path) else str(v)for v in args];start=time.monotonic();r=subprocess.run(args,cwd=root,capture_output=True,**kwargs)
        log=out/(name+'.log');log.write_bytes(r.stdout+r.stderr)
        commands.append(dict(name=name,command=args,exitCode=r.returncode,seconds=round(time.monotonic()-start,3),logSha256=sha(log)))
        save()
        if r.returncode:raise RuntimeError('Failed '+name+': '+str(log))
        return r.stdout.decode(errors='replace').replace('\r\n','\n')
    version=run('compiler-version',[cc,'--version'])
    includes=shlex.split((root/'config/includes.rsp').read_text(encoding='utf-8'))
    flags=['-std=c11','-O2','-Wall','-Wextra','-Wno-unused-parameter','-Wno-unused-function',
        '-Wno-cast-function-type','-ffunction-sections','-fdata-sections','-DXGE_XRT_HOST=1',
        '-DXGE_STATIC_BUILD=1',f'-DXGE_DEBUGMODE={int(a.debug_api)}',f'-DXGE_BUILD_PROFILE={profiles[a.profile]["id"]}',
        *includes,*['-D'+v for v in a.define]]
    if not windows:flags+=['-fPIC','-D_GNU_SOURCE']
    if a.profile=='ide':
        flags+=['-DXGE_XRT_PROFILE_IDE','-include','extlibs/xrt/xrt_config.h','-Iextlibs/xrt','-Iextlibs/xrt/include']
        flags += [f'-Iextlibs/xrt/extlibs/{v}/include'for v in ('xllm','xllm-session','xwork')]
    names=sorted(set(re.findall(r'(?:XGE|XUI|XGE3D)_ENABLE_\w+',
        (root/'include/xge_config.h').read_text(encoding='utf-8')+(root/'include/xui_config.h').read_text(encoding='utf-8'))))
    def probe(suffix):
        text='#include "xui_config.h"\n'+''.join(f'#if {name}\nXGE_SELECTED_{name}\n#endif\n'for name in names)
        result=run('features-'+suffix,[cc,*flags,'-E','-P','-x','c','-'],input=text.encode())
        return sorted(set(re.findall(r'^XGE_SELECTED_(\w+)$',result,re.M)))
    features=probe('initial')
    if 'XGE_ENABLE_TEXT'in features and not any(v.startswith('XGE_ENABLE_HARFBUZZ=')for v in a.define):
        flags+=['-DXGE_ENABLE_HARFBUZZ=1'];features=probe('text')
    unknown={v.split('=')[0]for v in a.define}-set(names)-{'XUI_ENABLE_WEBVIEW2'}
    if unknown:raise RuntimeError('Unknown feature switches: '+str(sorted(unknown)))
    egl_sdk=egl.configure(features,windows,cc);flags+=egl_sdk['cflags']
    if egl_sdk['metadata']:inputs.update(egl_sdk['metadata']['inputs'])
    external_links=[];runtime_dependencies=[]
    webview2=windows and 'XUI_ENABLE_WEBVIEW'in features and (
        os.environ.get('XUI_ENABLE_WEBVIEW2')=='1' or 'XUI_ENABLE_WEBVIEW2=1'in a.define)
    if webview2:
        sdk=Path(os.environ.get('WEBVIEW2_SDK_DIR',''));include=sdk/'build/native/include'
        loader=sdk/'build/native/x64/WebView2Loader.dll.lib';runtime=sdk/'build/native/x64/WebView2Loader.dll'
        if not(include/'WebView2.h').is_file()or not loader.is_file()or not runtime.is_file():
            raise RuntimeError('The selected WebView2 backend requires its complete WEBVIEW2_SDK_DIR')
        if not any(v.startswith('XUI_ENABLE_WEBVIEW2=')for v in a.define):flags+=['-DXUI_ENABLE_WEBVIEW2=1']
        flags+=['-isystem',include.as_posix()];external_links.append(loader);runtime_dependencies.append(runtime)
        for path in [loader,runtime,*include.rglob('*.h')]:inputs[str(path.resolve())]=sha(path)
    modules=json.loads((root/'config/modules.json').read_text(encoding='utf-8'));sources=['single/xge.c']
    for group,header,prefix in [('xui_sources','xui_config.h','XGE_ENABLE_XUI && '),('3d_sources','xge_config.h','')]:
        choices=list(modules.get(group,{}).items())
        text='#include "'+header+'"\n'+''.join(f'#if {prefix}({guard})\nXGE_SOURCE_{i}\n#endif\n'for i,(_,guard)in enumerate(choices))
        selected=run('sources-'+group,[cc,*flags,'-E','-P','-x','c','-'],input=text.encode())
        sources += [choices[int(i)][0]for i in re.findall(r'^XGE_SOURCE_(\d+)$',selected,re.M)]
    if a.profile=='ide':
        for product in ('xllm','xllm-session','xwork'):
            extension=json.loads((root/f'extlibs/xrt/extlibs/{product}/config/modules.json').read_text(encoding='utf-8'))
            sources += ['extlibs/xrt/'+s for module in extension['modules']for s in module['sources']]
    if len(sources)!=len(set(sources)):raise RuntimeError('Duplicate physical source selection')
    api=int(re.search(r'#define\s+XGE_VERSION_MAJOR\s+(\d+)',(root/'include/xge.h').read_text(encoding='utf-8')).group(1))
    config='#ifndef XGE_BUILD_CONFIG_H\n#define XGE_BUILD_CONFIG_H\n'
    config+=f'#define XGE_BUILD_API_MAJOR {api}\n#define XGE_BUILD_PROFILE {profiles[a.profile]["id"]}\n'
    config+=''.join(f'#define {name} {int(name in features)}\n'for name in names)
    config+='#define XGE_XRT_HOST 1\n#define XGE_STATIC_BUILD 1\n#define XL6_XGE_STATIC 1\n'
    if a.debug_api:config+='#define XGE_DEBUGMODE 1\n'
    if webview2:config+='#define XUI_ENABLE_WEBVIEW2 1\n'
    public=sorted((root/'include').glob('*.h'));abi_inputs='XGE_PUBLIC_ABI_V2\n'+''.join(path.relative_to(root).as_posix()+'\0'+path.read_text(encoding='utf-8')+'\0'for path in [*public,root/'extlibs/xrt/xrt.h'])
    fingerprint=hashlib.sha256((config+abi_inputs+version+str(int(a.debug_api))).encode()).hexdigest()[:16]
    config+=f'#define XGE_BUILD_ABI_FINGERPRINT UINT64_C(0x{fingerprint})\n#endif\n'
    (out/'xge_build_config.h').write_text(config,encoding='utf-8',newline='\n')
    flags+=[f'-DXGE_BUILD_ABI_FINGERPRINT=0x{fingerprint}ULL']
    xrt_sha=hashlib.sha256((root/'extlibs/xrt/xrt.h').read_bytes().replace(b'\r\n',b'\n')).hexdigest()
    contract='''#ifndef XGE_STATIC_XRT_CONTRACT_H
#define XGE_STATIC_XRT_CONTRACT_H
#include "xge_host_xrt.h"
#include <wchar.h>
struct XgeStaticScalarAlignment {char c;void*p;double d;};
struct XgeStaticViewAlignment {char c;xstrview v;};
#if defined(_WIN64) && defined(__x86_64__)
#define XGE_STATIC_XRT_TARGET 1u
#elif defined(__linux__) && defined(__x86_64__)
#define XGE_STATIC_XRT_TARGET 2u
#else
#error "Static XGE target ABI has not been qualified"
#endif
#define XGE_STATIC_XRT_LOCAL_CONTRACT_INIT {sizeof(xge_host_xrt_contract_t),1u, \\
 ((uint64_t)XGE_STATIC_XRT_TARGET|((uint64_t)sizeof(void*)<<8)|((uint64_t)sizeof(size_t)<<12)| \\
 ((uint64_t)sizeof(long)<<16)|((uint64_t)sizeof(wchar_t)<<20)|((uint64_t)sizeof(double)<<24)| \\
 ((uint64_t)sizeof(int)<<28)|((uint64_t)offsetof(struct XgeStaticScalarAlignment,p)<<32)| \\
 ((uint64_t)offsetof(struct XgeStaticScalarAlignment,d)<<40)), \\
 ((uint64_t)sizeof(xstrview)|((uint64_t)offsetof(struct XgeStaticViewAlignment,v)<<16)| \\
 ((uint64_t)sizeof(xrtresourcelimits)<<32)),XGE_STATIC_XRT_HEADER_SHA256}
extern const xge_host_xrt_contract_t xgeStaticXrtContractV1;
#endif
'''
    contract=contract.replace('#include <wchar.h>','#include <wchar.h>\n#define XGE_STATIC_XRT_HEADER_SHA256 "'+xrt_sha+'"')
    (out/'xge_static_xrt_contract.h').write_text(contract,encoding='utf-8',newline='\n')
    meta=out/'xge_static_contract.c';meta.write_text('#include "xge.h"\n#include "xge_static_xrt_contract.h"\nconst xge_host_xrt_contract_t xgeStaticXrtContractV1=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;\n',encoding='utf-8',newline='\n')
    cpp=None;link_objects=[]
    def compile_one(source,compiler,options,label=None):
        source=Path(source);name=label or source.as_posix().replace('/','_').replace(':','_')
        obj=objects/(name+'.o');dep=obj.with_suffix('.d');record=obj.with_suffix('.json')
        command=[compiler,*options,'-MD','-MF',dep,'-MT',obj,'-c',source,'-o',obj]
        key=hashlib.sha256(json.dumps([str(v)for v in command]).encode()).hexdigest();cache=None
        try:cache=json.loads(record.read_text())
        except (OSError,ValueError):pass
        if cache and cache.get('sourceRoot')!=str(root):cache=None
        prior=parent_rows.get(str(source))
        if cache is None and prior and prior['compiler']==str(compiler)and prior['flags']==options:
            prior_cache=json.loads(Path(prior['dependencyRecord']).read_text())
            rebound=reuse_dependencies(prior_cache['dependencies'],parent['source_root'],root)
            if rebound is not None and prior_cache['compilerSha256']==sha(compiler)and sha(prior['object'])==prior['sha256']:
                shutil.copy2(prior['object'],obj);cache=dict(prior_cache,key=key,sourceRoot=str(root),dependencies=rebound);record.write_text(json.dumps(cache,indent=2)+'\n')
        if cache and obj.exists()and cache['key']==key and cache['compilerSha256']==sha(compiler)and all(Path(v).is_file()and sha(v)==h for v,h in cache['dependencies'].items()):
            reused.append(str(source))
        else:
            record.unlink(missing_ok=True);print('[compile] '+str(source),flush=True);run('compile-'+str(len(link_objects)),command)
            text=dep.read_text().replace('\\\n',' ');text=re.split(r':\s',text,maxsplit=1)[1]
            deps=[re.sub(r'\\([ #\\])',r'\1',v)for v in re.findall(r'(?:\\.|[^\s])+',text)]
            paths=[Path(v)if Path(v).is_absolute()else root/v for v in deps]
            record.write_text(json.dumps(dict(key=key,sourceRoot=str(root),compilerSha256=sha(compiler),dependencies={str(v):sha(v)for v in paths}),indent=2)+'\n')
            compiled.append(str(source))
        link_objects.append(obj);rows.append(dict(source=str(source),object=str(obj),sha256=sha(obj),compiler=str(compiler),flags=options,dependencyRecord=str(record)))
    for source in sources:compile_one(source,cc,flags)
    compile_one(meta,cc,[*flags,'-I'+str(out)],'static-contract')
    if 'XGE_ENABLE_HARFBUZZ'in features:
        cpp=shutil.which(os.environ.get('CXX','g++'))
        if not cpp:raise RuntimeError('The selected text configuration requires its existing HarfBuzz C++ dependency')
        hb=['-std=c++17','-O2','-fno-exceptions','-fno-rtti','-ffunction-sections','-fdata-sections','-DNDEBUG',
            '-DHB_NO_BUFFER_SERIALIZE','-DHB_NO_BUFFER_VERIFY','-DHB_NO_DRAW','-DHB_NO_OT_FETCH','-DHB_NO_MMAP','-DHB_NO_COLOR']
        if not windows:hb+=['-fPIC','-DHAVE_PTHREAD=1']
        compile_one('extlibs/harfbuzz/src/harfbuzz.cc',cpp,hb)
    archive=out/'libxge3_static.a';temporary=out/'libxge3_static.pending.a';temporary.unlink(missing_ok=True)
    response=out/'archive.rsp';response.write_text('\n'.join('"'+v.as_posix()+'"'for v in link_objects)+'\n')
    run('archive',[ar,'rcs',temporary,'@'+str(response)])
    defined=run('defined-symbols',[nm,'-g','--defined-only',temporary]);undefined=run('undefined-symbols',[nm,'-u',temporary])
    forbidden=re.findall(r'\b(?:__imp_)?((?:__)?xrt[A-Z]\w*)\s*$',defined,re.M)
    if forbidden or re.search(r'\b(?:XgeHostXrtSlot_|xgeHostXrtBind|xgeHostXrtDescribe|xgeHostXrtRuntimeIdentity)',defined+undefined):
        raise RuntimeError('Static archive unexpectedly embeds XRT or hosted import-slot machinery: '+str(forbidden[:8]))
    temporary.replace(archive)
    symbols=sorted(set(re.findall(r'\b(?:__imp_)?((?:__)?xrt[A-Z]\w*)\s*$',undefined,re.M)))
    libraries=(['kernel32','advapi32','ws2_32','iphlpapi','gdi32','user32','shell32','ole32','oleaut32','uuid','imm32','winmm','avrt','dwrite']if windows else['m','X11','Xi','Xcursor','GL','dl','pthread'])
    libraries+=egl_sdk['libraries']
    global_definitions=set(re.findall(r'^\S*\s+[A-Za-z]\s+(\S+)\s*$',defined,re.M))
    cpp_runtime=sorted({v for v in re.findall(r'^\s+U\s+(\S+)\s*$',undefined,re.M)
        if v.startswith(('_Z','__cxa','__gxx')) and v not in global_definitions})
    if cpp_runtime:libraries+=['stdc++']
    support=windows_support(cc,ar,out,run,inputs,a.runtime_notice,libraries)if windows else linux_support(cc,out,run,inputs,a.runtime_notice)
    resource=None
    if windows:
        windres=shutil.which('windres')
        if not windres:raise RuntimeError('windres required for the existing Windows icon/version resources')
        resource=out/'xge_static_res.o';run('resource',[windres,'-O','coff','-i','platform/windows/xge.rc','-o',resource])
    sections=run('object-sections',['objdump','-h',link_objects[0]])
    section_count=len(re.findall(r'^\s*\d+\s+\.(?:text|data|bss|rdata)(?:\$|\.)',sections,re.M))
    if section_count<2:raise RuntimeError('Compiler did not emit independent function/data sections')
    unchanged=all(sha(path)==digest for path,digest in inputs.items())
    report.update(complete=True,passed=unchanged,sourceUnchanged=unchanged,api_major=api,compiler=[cc,sha(cc),version],
        flags=flags,features=features,egl_sdk=egl_sdk['metadata'],sources=sources,objects=rows,compiled=compiled,reused=reused,
        linkage='static-archive',platform='win_x64'if windows else'linux_x64',archive=archive.name,
        archive_sha256=sha(archive),archive_bytes=archive.stat().st_size,xrt_header_sha256=xrt_sha,
        xrt_undefined_symbols=symbols,system_libraries=libraries,resource=resource.name if resource else None,
        resource_sha256=sha(resource)if resource else None,resource_role='Optional DLL-shell icon; not an engine functional resource'if resource else None,compiler_support=support,section_count_aggregate=section_count,
        external_link_libraries=[dict(path=str(v.resolve()),sha256=sha(v))for v in external_links],
        cxx_runtime_undefined=cpp_runtime,
        runtime_dependencies=[dict(path=str(v.resolve()),sha256=sha(v),deployment='Must remain available to the final application loader')for v in runtime_dependencies],
        reuse_parent=str(a.reuse_build.resolve()/'build-report.json')if a.reuse_build else None,
        granularity='single/xge.c aggregate member plus configured physical XUI/3D TUs; compiler section reachability, not function-source splitting',
        seconds=round(time.monotonic()-started,3))
    save()
    if not unchanged:raise RuntimeError('Native source changed during static build')
    print('[static] '+str(archive)+' '+str(archive.stat().st_size)+' bytes',flush=True)

if __name__=='__main__':
    try:main()
    except (OSError,ValueError,RuntimeError,subprocess.SubprocessError)as error:
        print('[ERROR] '+str(error),file=sys.stderr);sys.exit(1)
