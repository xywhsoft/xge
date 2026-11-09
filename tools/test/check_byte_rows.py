"""Check production strided-byte arithmetic on the host address width."""
from pathlib import Path
import argparse,hashlib,json,os,shutil,subprocess


def main():
 p=argparse.ArgumentParser(description=__doc__)
 p.add_argument('--root',type=Path,default=Path(__file__).resolve().parents[2])
 p.add_argument('--out',type=Path,required=True)
 p.add_argument('--sanitize',action='store_true')
 a=p.parse_args();root=a.root.resolve();out=a.out.resolve();out.mkdir(parents=True,exist_ok=False)
 cc=shutil.which('gcc');assert cc
 if a.sanitize and os.name=='nt':p.error('Use the Linux sanitizer toolchain')
 sha=lambda path:hashlib.sha256(Path(path).read_bytes()).hexdigest()
 files=[Path(__file__).resolve(),Path(cc),root/'src/xge/core/xge_byte_rows.h',root/'tests/xge/xge_byte_rows_test.c']
 report=dict(passed=False,sanitizers=a.sanitize,networkTests=False,inputs={str(p):sha(p) for p in files},rows=[])
 def save():(out/'results.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
 for opt in ('O0','O2'):
  exe=out/(opt+('.exe' if os.name=='nt' else ''))
  flags=['-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'] if a.sanitize else []
  command=[cc,'-std=c11','-Wall','-Wextra','-Werror','-'+opt,*flags,'-I'+str(root),str(root/'tests/xge/xge_byte_rows_test.c'),'-o',str(exe)]
  for step,argv in [('build',command),('run',[str(exe)])]:
   z=subprocess.run(argv,capture_output=True,timeout=120);log=out/(opt+'-'+step+'.log');log.write_bytes(z.stdout+z.stderr)
   report['rows'].append(dict(optimization=opt,step=step,command=argv,exitCode=z.returncode,logSha256=sha(log)));save();print(opt,step,z.returncode,flush=True);assert not z.returncode,log
 assert all(sha(path)==digest for path,digest in report['inputs'].items())
 report.update(passed=True,inputsUnchanged=True);save()


if __name__=='__main__':main()
