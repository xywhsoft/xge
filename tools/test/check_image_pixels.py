"""Pure read-only Image pixel spans/copies, without XRT, engine or GUI.

C11/C++11/C++17 clients at O0/O2; capacity, padded rows, format, overlap
and failure preservation, plus optional Linux ASan/UBSan/leak checks.
No decoder/parser/network/security tests.
"""
from pathlib import Path
import argparse,hashlib,json,os,re,shutil,subprocess,time


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root',type=Path,default=Path(__file__).resolve().parents[2])
    p.add_argument('--out',type=Path,required=True)
    p.add_argument('--sanitize',action='store_true')
    a=p.parse_args();root,out=a.root.resolve(),a.out.resolve();win=os.name=='nt'
    assert not(win and a.sanitize)
    tools={n:Path(shutil.which(n)).resolve()for n in ('gcc','g++','nm','objdump')}
    unit=root/'src/xge/text/xge_image_pixels.c';case=root/'tests/xge/xge_image_pixels_test.c'
    files=[Path(__file__).resolve(),unit,case,root/'src/xge/core/xge_byte_rows.h',
        root/'include/xge.h',root/'include/xge_config.h',root/'include/xnative_transfer.h',
        root/'extlibs/xrt/xrt.h',root/'extlibs/xrt/xrt_config.h',*tools.values()]
    sha=lambda v:hashlib.sha256(Path(v).read_bytes()).hexdigest()
    inputs={str(v):sha(v)for v in files};out.mkdir(parents=True,exist_ok=False)
    report=dict(complete=False,passed=False,shippingDelivery=False,scope=__doc__,inputs=inputs,rows=[],programs=[],objects=[])
    def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    def run(name,command):
        command=list(map(str,command));start=time.monotonic()
        z=subprocess.run(command,cwd=out,capture_output=True,timeout=120)
        log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr)
        report['rows'].append(dict(name=name,command=command,exitCode=z.returncode,
            seconds=time.monotonic()-start,logSha256=sha(log)))
        save();print(name,z.returncode,flush=True);assert not z.returncode,log
        return z.stdout+z.stderr
    flags=['-Wall','-Wextra','-Werror','-DXGE_STATIC_BUILD=1','-I'+str(root),'-I'+str(root/'include')]
    try:
        for level in ('O0','O2'):
            obj=out/(level+'.o')
            run(level+'-object',[tools['gcc'],'-std=c11','-'+level,*flags,'-ffunction-sections','-fdata-sections','-c',unit,'-o',obj])
            undefined=run(level+'-undefined',[tools['nm'],'-u',obj])
            symbols=[line.split()[-1].decode()for line in undefined.splitlines()if line.strip()]
            # Distribution GCC may insert its stack protector even in this
            # allocator-free leaf component. These are compiler/CRT hooks.
            assert set(symbols)<={'memmove','memcpy','__stack_chk_fail','__stack_chk_fail_local'},symbols
            report['objects'].append(dict(path=str(obj),sha256=sha(obj),undefinedSymbols=symbols));save()
            for standard in ('c11','c++11','c++17'):
                tool=tools['g++']if '+'in standard else tools['gcc'];name=level+'-'+standard.replace('+','p')
                binary=out/(name+('.exe'if win else''))
                run(name+'-build',[tool,'-std='+standard,'-'+level,*flags,case,obj,'-Wl,--gc-sections','-o',binary])
                data=run(name+'-run',[binary]);assert b'overlap and failure preservation passed'in data
            table=run(level+'-imports',[tools['objdump'],'-p',binary])
            imports=re.findall(rb'DLL Name:\s*(\S+)',table)if win else re.findall(rb'\bNEEDED\s+(\S+)',table)
            assert not any(b'xge'in v.lower()or b'xrt'in v.lower()or b'xlc'in v.lower()for v in imports)
            report['programs'].append(dict(path=str(binary),sha256=sha(binary),bytes=binary.stat().st_size,imports=[v.decode()for v in imports]));save()
        if a.sanitize:
            binary=out/'sanitized'
            run('sanitized-build',[tools['gcc'],'-std=c11','-O1','-g','-fsanitize=address,undefined','-fno-sanitize-recover=all',*flags,case,unit,'-o',binary])
            run('sanitized-run',[binary])
        report.update(complete=True,inputsUnchanged=all(sha(v)==h for v,h in inputs.items()))
        report['passed']=report['inputsUnchanged'];assert report['passed']
    except Exception as e:report.update(complete=True,passed=False,error=str(e));raise
    finally:save()


if __name__=='__main__':main()
