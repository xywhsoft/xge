"""Compile the exact search source with regex disabled and call its public stub.

This qualifies the source branch, not an entire disabled-feature SDK. Windows
links the matched static SDK/full runtime to resolve uncollected PE sections;
Linux linker GC needs neither. No window or network is opened.
"""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess


def main():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('root','test-root','static','runtime','out'):p.add_argument('--'+name,type=Path,required=True)
 a=p.parse_args();root,tests,static,runtime,out=[v.resolve()for v in(a.root,a.test_root,a.static,a.runtime,a.out)]
 win=os.name=='nt';cc=Path(shutil.which('gcc')).resolve();sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
 meta=json.loads((static/'build-report.json').read_text(encoding='utf-8'))
 assert meta['passed']and meta['sourceUnchanged']and meta['xrt_mode']=='direct-external'and Path(meta['source_root']).resolve()==root
 archive=static/meta['archive'];source=root/'src/xui/code/xui_code_search.c';test=tests/'tests/xui/xui_code_search_no_regex_test.c'
 files=[Path(__file__).resolve(),test,source,cc,archive,runtime,static/'build-report.json',*sorted((root/'include').glob('*.h'))]
 files.extend(v for folder in ('src/xui/core','src/xui/code')for v in (root/folder).glob('*.h'))
 inputs={str(v):sha(v)for v in files};out.mkdir(parents=True,exist_ok=False)
 report=dict(complete=False,passed=False,scope=__doc__,shippingDelivery=False,networkTests=False,inputs=inputs,rows=[])
 env=dict(os.environ)
 if win:env['PATH']=str(runtime.parent)+os.pathsep+env.get('PATH','')
 def run(name,command):
  command=list(map(str,command));z=subprocess.run(command,cwd=root,env=env,capture_output=True,timeout=60)
  log=out/(name+'.log');log.write_bytes(z.stdout+z.stderr);report['rows'].append(dict(name=name,command=command,exitCode=z.returncode,logSha256=sha(log)))
  print(name,z.returncode,flush=True);assert not z.returncode,log;return z.stdout+z.stderr
 try:
  for level in ('O0','O2'):
   binary=out/(level+('.exe'if win else''));links=[archive,runtime,*['-l'+n for n in meta['system_libraries']]]if win else[]
   run(level+'-build',[cc,'-'+level,'-std=c11','-Wall','-Wextra','-Werror','-Wno-unused-function','-Wno-unused-parameter','-Wno-cast-function-type',
    '-ffunction-sections','-fdata-sections','-DXRT_NO_REGEX=1',*['-I'+str(v)for v in (root,root/'include',root/'src/xui/core',root/'src/xui/code')],
    test,source,*links,'-Wl,--gc-sections','-o',binary])
   data=run(level+'-run',[binary]);assert b'MarkAll disabled-regex source branch: explicit unsupported and zero count passed'in data
  report.update(complete=True,inputsUnchanged=all(sha(v)==h for v,h in inputs.items()));report['passed']=report['inputsUnchanged'];assert report['passed']
 except Exception as error:report.update(complete=True,passed=False,error=str(error));raise
 finally:(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')


if __name__=='__main__':main()
