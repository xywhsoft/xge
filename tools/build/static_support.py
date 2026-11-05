"""Keep compiler compatibility code separate from GNU PE import metadata."""
from pathlib import Path
import hashlib,os,re,struct,subprocess


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
            start=strings+int.from_bytes(raw[4:],'little');end=body.find(b'\0',start)
            if end<0:raise RuntimeError('Unterminated COFF symbol name')
            name=body[start:end].decode('ascii')
        else:name=raw.split(b'\0')[0].decode('ascii')
        if storage==2:result.append((name,section,kind))
        index+=1+aux
    return result


def import_thunk(body):
    """Prove an AMD64 executable jmp-through-IAT relocation, not a name guess."""
    _,count,_,symbols,symbol_count,optional,_=struct.unpack_from('<HHLLLHH',body)
    strings=symbols+18*symbol_count
    for i in range(count):
        header=20+optional+40*i
        row=struct.unpack_from('<8sLLLLLLHHL',body,header)
        raw=body[row[4]:row[4]+row[3]]
        if not row[-1]&0x20000000:continue
        begin=4 if raw.startswith(b'\xf3\x0f\x1e\xfa')else 0
        if raw[begin:begin+2]!=b'\xff\x25' or len(raw)<begin+6:continue
        for j in range(row[7]):
            offset,index,kind=struct.unpack_from('<LLH',body,row[5]+10*j)
            if offset!=begin+2 or kind!=4 or index>=symbol_count:continue
            at=symbols+18*index
            value,section,_,storage,_=struct.unpack_from('<LhHBB',body,at+8)
            if storage!=2 or not 0<section<=count:continue
            if body[20+optional+40*(section-1):][:8].rstrip(b'\0')!=b'.idata$5':continue
            name=body[at:at+8]
            if name[:4]==b'\0'*4:
                start=strings+int.from_bytes(name[4:],'little');end=body.find(b'\0',start)
                if end<0:continue
                name=body[start:end]
            else:name=name.split(b'\0')[0]
            if name.startswith(b'__imp_'):
                return dict(kind='function',known=True,
                    reason='actual-AMD64-executable-jmp-IAT-thunk',
                    section=row[0].rstrip(b'\0').decode('ascii'),
                    sectionFlags=hex(row[-1]),instructionOffset=begin,
                    relocationType=kind,iatSymbol=name.decode('ascii'),chain=[])
    return None

def windows_support(cc,ar,out,run,inputs,notices,libraries):
    """Preserve all native-code members. Never emit raw .idata as static code.

    libmsvcrt.a also contains genuine MinGW compatibility implementations.
    Those are retained byte for byte, while its import members become a
    declarative .def from their real DLL/name metadata. No C shim is created.
    """
    artifacts=[];archives=[];classifier=WindowsExportKinds();unknown_inventory=[]
    dlltool=str(Path(cc).with_name('dlltool.exe'))
    inputs[str(Path(dlltool).resolve())]=sha(dlltool)
    support=out/'compiler-support';support.mkdir(exist_ok=True)
    for name in ['libgcc.a','libmingw32.a','libmingwex.a','libmsvcrt.a',*['lib'+v+'.a'for v in libraries]]:
        original=Path(run('find-'+name,[cc,'-print-file-name='+name]).strip()).resolve()
        if not original.is_file():raise RuntimeError('Missing compiler runtime '+name)
        inputs[str(original)]=sha(original)
        dll=None
        folder=support/name.replace('.a','');folder.mkdir(exist_ok=True)
        code=[];imports=[];import_rows=[];allrows=[];seen=set()
        for member,body in members(original):
            if member in seen:raise RuntimeError('Duplicate compiler archive member: '+member)
            seen.add(member);rows=sections(body);imported=any(k.startswith('.idata$')for k,v in rows)
            allrows.append(dict(member=member,sha256=hashlib.sha256(body).hexdigest(),role='import'if imported else'code'))
            if imported:
                if dll is None:
                    dll=run('identify-'+name,[dlltool,'-I',original]).strip()
                    # Keep the actual OS image identity even when a real
                    # import thunk already proves the function kind.
                    try:classifier.image(dll)
                    except (OSError,ValueError,struct.error,UnicodeError):pass
                for section,raw in rows:
                    if section=='.idata$6'and len(raw)>2:
                        symbol=raw[2:].split(b'\0')[0].decode('ascii')
                        if not re.fullmatch(r'[A-Za-z_?@$][A-Za-z0-9_?@$]*',symbol):raise RuntimeError('Unsupported import name')
                        if not re.fullmatch(r'[A-Za-z0-9_.-]+\.dll',dll,re.I):
                            raise RuntimeError('Ambiguous import DLL identity')
                        proof=import_thunk(body)or classifier.classify(dll,symbol)
                        kind=proof['kind']
                        local_names=[name[6:]for name,section,_ in externals(body)if name.startswith('__imp_')and section>0 and rows[section-1][0]=='.idata$5']
                        if len(local_names)!=1:raise RuntimeError('Ambiguous import-local identity')
                        local=local_names[0]
                        imports.append((local,symbol,kind))
                        import_rows.append(dict(member=member,name=symbol,localName=local,
                            kind=kind,kindKnown=proof['known'],kindEvidence=proof,
                            hint=int.from_bytes(raw[:2],'little'),ordinal=None,
                            memberSha256=hashlib.sha256(body).hexdigest()))
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
            if not re.fullmatch(r'[A-Za-z0-9_.-]+\.dll',dll,re.I):raise RuntimeError('Ambiguous import DLL identity')
            definition=support/(name.replace('.a','')+'-imports.def')
            kinds={}
            for local,symbol,kind in imports:
                if kind=='unknown':
                    unknown_inventory.extend(dict(dll=dll,sourceArchive=str(original),**row)
                        for row in import_rows if row['localName']==local
                        and row['name']==symbol and row['kind']=='unknown')
                    continue
                if local in kinds and kinds[local]!=(symbol,kind):raise RuntimeError('Conflicting import identity')
                kinds[local]=(symbol,kind)
            definition.write_text('LIBRARY '+dll+'\nEXPORTS\n'+''.join(local+('='+symbol if local!=symbol else'')+(' DATA'if kind=='data'else'')+'\n'for local,(symbol,kind)in sorted(kinds.items())),encoding='ascii')
            artifacts.append(dict(path=str(definition),sha256=sha(definition),
                role='declarative-imports',dll=dll,exports=len(kinds),
                exportKinds={local:kind for local,(symbol,kind)in sorted(kinds.items())},
                exportAliases={local:symbol for local,(symbol,kind)in sorted(kinds.items())if local!=symbol},
                unknownImportsOmitted=True,sourceArchive=str(original),sourceSha256=sha(original)))
    macros=run('crt-macros',[cc,'-dM','-E','-x','c','-'],input=b'#include <stdio.h>\n#include <stdlib.h>\n')
    if re.search(r'^#define _UCRT(?:\s|$)',macros,re.M)or '#define __MSVCRT_VERSION__ 0x600'not in macros:
        raise RuntimeError('This Windows static SDK currently qualifies the MSVCRT toolchain only')
    notice_paths=[Path(cc).resolve().parents[1]/'COPYING.MinGW-w64-runtime.txt',Path(cc).resolve().parents[1]/'README.md',*notices]
    notice_rows=[]
    for path in notice_paths:
        if not path.is_file():raise RuntimeError('Missing compiler runtime notice '+str(path))
        inputs[str(path)]=sha(path);notice_rows.append(dict(path=str(path.resolve()),sha256=sha(path)))
    inputs.update(classifier.files)
    return dict(artifacts=artifacts,archives=archives,notices=notice_rows,
        importClassification=dict(method='Executable import thunk; otherwise readonly System32 PE export RVA/section/forwarder evidence. Unknown never implies DATA.',
            dllInputs=classifier.files,unknownInventory=unknown_inventory),
        crt='msvcrt.dll',ansiStdio=True,
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


class WindowsExportKinds:
    """Read PE exports only; never load a DLL or run initialization code."""
    def __init__(self):
        self.base=Path(os.environ.get('SystemRoot','C:/Windows'))/'System32'
        self.cache={};self.files={}

    def image(self,dll):
        if not re.fullmatch(r'[A-Za-z0-9_.-]+',dll):
            raise ValueError('Invalid forwarder DLL name')
        if not dll.lower().endswith('.dll'):dll+='.dll'
        key=dll.lower()
        if key in self.cache:return self.cache[key]
        path=self.base/dll;data=path.read_bytes()
        self.files[str(path.resolve())]=hashlib.sha256(data).hexdigest()
        def at(offset,size):
            if offset<0 or size<0 or offset+size>len(data):
                raise ValueError('PE range outside file')
            return data[offset:offset+size]
        if at(0,2)!=b'MZ':raise ValueError('Not PE')
        pe=int.from_bytes(at(60,4),'little')
        if at(pe,4)!=b'PE\0\0':raise ValueError('No PE signature')
        machine,count,_,_,_,optional,_=struct.unpack('<HHLLLHH',at(pe+4,20))
        opt=pe+24
        if machine!=0x8664 or optional<120 or int.from_bytes(at(opt,2),'little')!=0x20b:
            raise ValueError('Not AMD64 PE32+')
        export,export_size=struct.unpack('<LL',at(opt+112,8));sections=[]
        for i in range(count):
            raw=at(opt+optional+40*i,40)
            name=raw[:8].rstrip(b'\0').decode('ascii')
            virtual,rva,size,offset=struct.unpack_from('<LLLL',raw,8)
            flags=struct.unpack_from('<L',raw,36)[0]
            sections.append(dict(name=name,rva=rva,mapped=max(virtual,size),
                size=size,offset=offset,flags=flags))
        def section(rva):
            return next((s for s in sections if s['rva']<=rva<s['rva']+s['mapped']),None)
        def read(rva,size):
            sec=section(rva)
            if sec is None or rva-sec['rva']+size>sec['size']:
                raise ValueError('PE RVA has no file bytes')
            return at(sec['offset']+rva-sec['rva'],size)
        def text(rva):
            sec=section(rva)
            if sec is None:raise ValueError('PE string RVA outside section')
            limit=min(4096,sec['size']-(rva-sec['rva']))
            raw=read(rva,limit);end=raw.find(b'\0')
            if end<0:raise ValueError('Unterminated PE export string')
            return raw[:end].decode('ascii')
        values=struct.unpack('<LLHHLLLLLLL',read(export,40))
        ordinal_base,functions,names,func_at,name_at,ord_at=values[5:]
        if functions>1000000 or names>1000000:raise ValueError('PE export table limit')
        table=struct.unpack('<'+'L'*functions,read(func_at,4*functions));lookup={}
        if names:
            ntable=struct.unpack('<'+'L'*names,read(name_at,4*names))
            otable=struct.unpack('<'+'H'*names,read(ord_at,2*names))
            for name_rva,index in zip(ntable,otable):
                if index>=functions:raise ValueError('PE export ordinal index outside table')
                lookup[text(name_rva)]=index
        result=dict(path=str(path.resolve()),sha256=self.files[str(path.resolve())],
            export=export,exportSize=export_size,base=ordinal_base,table=table,
            names=lookup,section=section,text=text)
        self.cache[key]=result
        return result

    def classify(self,dll,name,ordinal=None,seen=(),chain=()):
        identity=(dll.lower(),name,ordinal)
        if identity in seen or len(seen)>=8:
            return dict(kind='unknown',known=False,reason='forwarder-cycle-or-depth',chain=list(chain))
        try:
            image=self.image(dll)
            index=(ordinal-image['base'])if ordinal is not None else image['names'].get(name,-1)
            if not 0<=index<len(image['table']):
                raise ValueError('Export name/ordinal unavailable in actual System32 image')
            rva=image['table'][index]
            if not rva:raise ValueError('Empty export RVA')
            evidence=dict(dll=dll,name=name,ordinal=image['base']+index,
                path=image['path'],sha256=image['sha256'],rva=rva)
            if image['export']<=rva<image['export']+image['exportSize']:
                forward=image['text'](rva);evidence['forwarder']=forward
                target,symbol=forward.rsplit('.',1)
                if not target or not symbol:raise ValueError('Invalid forwarder identity')
                if symbol.startswith('#'):
                    value=int(symbol[1:])
                    if not 0<value<=65535:raise ValueError('Invalid forwarder ordinal')
                    return self.classify(target,None,value,seen+(identity,),chain+(evidence,))
                return self.classify(target,symbol,None,seen+(identity,),chain+(evidence,))
            sec=image['section'](rva)
            if not sec:raise ValueError('Export RVA outside mapped section')
            evidence.update(section=sec['name'],sectionFlags=hex(sec['flags']))
            if sec['flags']&0x20000000:kind='function'
            elif sec['flags']&(0x40000000|0x80000000):kind='data'
            else:raise ValueError('Export RVA section has no code/data memory flags')
            return dict(kind=kind,known=True,reason='System32-PE-export-section',
                chain=list(chain+(evidence,)))
        except (OSError,ValueError,struct.error,UnicodeError)as error:
            return dict(kind='unknown',known=False,reason=str(error),dll=dll,
                name=name,requestedOrdinal=ordinal,chain=list(chain))
