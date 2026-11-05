"""Keep compiler compatibility code separate from GNU PE import metadata."""
from pathlib import Path
import hashlib,re,struct,subprocess


def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def members(path):
    data=path.read_bytes()
    if data[:8]!=b"!<arch>\n":raise RuntimeError('Expected normal archive: '+str(path))
    offset=8;names=b''
    while offset<len(data):
        h=data[offset:offset+60]
        if len(h)!=60 or h[58:60]!=b'`\n':raise RuntimeError('Malformed archive')
        name=h[:16].decode('ascii').strip();size=int(h[48:58]);body=data[offset+60:offset+60+size];offset+=60+size+(size&1)
        if name=='//':names=body;continue
        if name in ('/','/SYM64/'):continue
        if name.startswith('/')and name[1:].isdigit():name=names[int(name[1:]):].split(b'/\n')[0].decode('ascii')
        else:name=name.rstrip('/')
        if '/'in name or '\\'in name or name in ('.','..'):raise RuntimeError('Unsafe member name')
        yield name,body

def sections(body):
    if body[:4]==b'\0\0\xff\xff':raise RuntimeError('Short import records require explicit importer support')
    machine,count,_,_,_,optional,_=struct.unpack_from('<HHLLLHH',body)
    if machine!=0x8664:raise RuntimeError('Expected x64 COFF')
    result=[]
    for i in range(count):
        row=struct.unpack_from('<8sLLLLLLHHL',body,20+optional+40*i)
        name=row[0].rstrip(b'\0').decode('ascii');size,offset=row[3:5]
        result.append((name,body[offset:offset+size]))
    return result

def externals(body):
    _,_,_,offset,count,_,_=struct.unpack_from('<HHLLLHH',body)
    strings=offset+18*count;index=0;result=[]
    while index<count:
        at=offset+18*index;raw=body[at:at+8];_,section,kind,storage,aux=struct.unpack_from('<LhHBB',body,at+8)
        if raw[:4]==b'\0'*4:
            name=body[strings+int.from_bytes(raw[4:],'little'):].split(b'\0')[0].decode('ascii')
        else:name=raw.split(b'\0')[0].decode('ascii')
        if storage==2:result.append((name,section,kind))
        index+=1+aux
    return result

def windows_support(cc,ar,out,run,inputs,notices,libraries):
    """Preserve all native-code members. Never emit raw .idata as static code.

    libmsvcrt.a also contains genuine MinGW compatibility implementations.
    Those are retained byte for byte, while its import members become a
    declarative .def from their real DLL/name metadata. No C shim is created.
    """
    artifacts=[];archives=[];dlltool=str(Path(cc).with_name('dlltool.exe'))
    support=out/'compiler-support';support.mkdir(exist_ok=True)
    for name in ['libgcc.a','libmingw32.a','libmingwex.a','libmsvcrt.a',*['lib'+v+'.a'for v in libraries]]:
        original=Path(run('find-'+name,[cc,'-print-file-name='+name]).strip()).resolve()
        if not original.is_file():raise RuntimeError('Missing compiler runtime '+name)
        inputs[str(original)]=sha(original);folder=support/name.replace('.a','');folder.mkdir(exist_ok=True)
        code=[];imports=[];import_rows=[];allrows=[];seen=set()
        for member,body in members(original):
            if member in seen:raise RuntimeError('Duplicate compiler archive member: '+member)
            seen.add(member);rows=sections(body);imported=any(k.startswith('.idata$')for k,v in rows)
            allrows.append(dict(member=member,sha256=hashlib.sha256(body).hexdigest(),role='import'if imported else'code'))
            if imported:
                for section,raw in rows:
                    if section=='.idata$6'and len(raw)>2:
                        symbol=raw[2:].split(b'\0')[0].decode('ascii')
                        if not re.fullmatch(r'[A-Za-z_?@$][A-Za-z0-9_?@$]*',symbol):raise RuntimeError('Unsupported import name')
                        kind='function'if any(k=='.text'and v for k,v in rows)else'data'
                        local_names=[name[6:]for name,section,_ in externals(body)if name.startswith('__imp_')and section>0 and rows[section-1][0]=='.idata$5']
                        if len(local_names)!=1:raise RuntimeError('Ambiguous import-local identity')
                        local=local_names[0]
                        imports.append((local,symbol,kind));import_rows.append(dict(member=member,name=symbol,localName=local,kind=kind,hint=int.from_bytes(raw[:2],'little'),ordinal=None,memberSha256=hashlib.sha256(body).hexdigest()))
                    if section in ('.idata$4','.idata$5')and len(raw)>=8 and int.from_bytes(raw[:8],'little')&(1<<63):
                        raise RuntimeError('Ordinal-only import requires explicit declaration support')
                continue
            target=folder/member;target.write_bytes(body);code.append(target)
        archive=support/(name.replace('.a','')+'-code.a')
        rsp=support/(name+'.rsp');rsp.write_text('\n'.join('"'+v.as_posix()+'"'for v in code)+'\n',encoding='utf-8')
        archive.unlink(missing_ok=True)
        if code:run('support-'+name,[ar,'rcs',archive,'@'+str(rsp)])
        archives.append(dict(path=str(archive)if code else None,sha256=sha(archive)if code else None,role='compiler-compatibility-code',sourceArchive=str(original),sourceSha256=sha(original),members=allrows,importMembers=import_rows,codeMembers=len(code),excludedImportMembers=len(allrows)-len(code)))
        if code:artifacts.append(dict(path=str(archive),sha256=sha(archive)))
        if imports:
            dll=run('identify-'+name,[dlltool,'-I',original]).strip()
            if not re.fullmatch(r'[A-Za-z0-9_.-]+\.dll',dll,re.I):raise RuntimeError('Ambiguous import DLL identity')
            definition=support/(name.replace('.a','')+'-imports.def')
            kinds={}
            for local,symbol,kind in imports:
                if local in kinds and kinds[local]!=(symbol,kind):raise RuntimeError('Conflicting import identity')
                kinds[local]=(symbol,kind)
            definition.write_text('LIBRARY '+dll+'\nEXPORTS\n'+''.join(local+('='+symbol if local!=symbol else'')+(' DATA'if kind=='data'else'')+'\n'for local,(symbol,kind)in sorted(kinds.items())),encoding='ascii')
            artifacts.append(dict(path=str(definition),sha256=sha(definition),role='declarative-imports',dll=dll,exports=len(set(imports)),exportKinds={local:kind for local,symbol,kind in sorted(set(imports))},exportAliases={local:symbol for local,symbol,kind in sorted(set(imports))if local!=symbol},sourceArchive=str(original),sourceSha256=sha(original)))
    macros=run('crt-macros',[cc,'-dM','-E','-x','c','-'],input=b'#include <stdio.h>\n#include <stdlib.h>\n')
    if re.search(r'^#define _UCRT(?:\s|$)',macros,re.M)or '#define __MSVCRT_VERSION__ 0x600'not in macros:
        raise RuntimeError('This Windows static SDK currently qualifies the MSVCRT toolchain only')
    notice_paths=[Path(cc).resolve().parents[1]/'COPYING.MinGW-w64-runtime.txt',Path(cc).resolve().parents[1]/'README.md',*notices]
    notice_rows=[]
    for path in notice_paths:
        if not path.is_file():raise RuntimeError('Missing compiler runtime notice '+str(path))
        inputs[str(path)]=sha(path);notice_rows.append(dict(path=str(path.resolve()),sha256=sha(path)))
    return dict(artifacts=artifacts,archives=archives,notices=notice_rows,crt='msvcrt.dll',ansiStdio=True,
        licenses={'libgcc':'GPL-3.0-or-later WITH GCC-exception-3.1','mingw':'COPYING.MinGW-w64-runtime.txt'},
        upstream={'gcc':'https://gcc.gnu.org/releases.html','toolchain':'https://github.com/skeeto/w64devkit'},
        boundary='All CRT FILE pointers and CRT allocation/free pairs must use the same MSVCRT instance. No UCRT FILE/object exchange. XRT allocation remains the sole program XRT allocator.')


def linux_support(cc,out,run,inputs,notices):
    original=Path(run('find-libgcc.a',[cc,'-print-file-name=libgcc.a']).strip()).resolve()
    if not original.is_file():raise RuntimeError('Missing compiler runtime libgcc.a')
    inputs[str(original)]=sha(original)
    macros=run('crt-macros',[cc,'-dM','-E','-x','c','-'],input=b'#include <features.h>\n')
    def number(name):
        match=re.search(r'^#define '+name+r' (\d+)$',macros,re.M)
        return int(match[1])if match else None
    major,minor=number('__GLIBC__'),number('__GLIBC_MINOR__')
    if major is None:raise RuntimeError('Linux static archive currently qualifies glibc only')
    notice_rows=[]
    for path in notices:
        if not path.is_file():raise RuntimeError('Missing compiler runtime notice '+str(path))
        inputs[str(path)]=sha(path);notice_rows.append(dict(path=str(path.resolve()),sha256=sha(path)))
    return dict(artifacts=[dict(path=str(original),sha256=sha(original),role='compiler-runtime-ELF-archive')],archives=[],notices=notice_rows,
        crt='glibc',glibcHeaders=[major,minor],licenses={'libgcc':'GPL-3.0-or-later WITH GCC-exception-3.1'},
        upstream={'gcc':'https://gcc.gnu.org/releases.html'},
        boundary='Archive is built against the recorded glibc headers, including actual __isoc23_* and fortified *_chk symbol dependencies. Older libc availability is not implied.')
