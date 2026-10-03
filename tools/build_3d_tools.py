"""Build offline C tools with a small persistent object cache."""
import hashlib
import json
from pathlib import Path
import subprocess
import os
from build_profile import dependencies,stamp

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'build/3d-tools'

def main():
    OUT.mkdir(parents=True,exist_ok=True)
    os.chdir(ROOT)
    flags=['gcc','-std=c11','-O2','-Wall','-Wextra','-Werror','-Wno-missing-braces','-Wno-misleading-indentation']
    compiler=subprocess.check_output(['gcc','--version']).decode().splitlines()[0]
    sources=['tools/xge3d_motion.c','tools/xge3d_motion_load.c','tools/xge3d_motion_write.c','tools/xge3d_motion_retarget.c','tools/xge3d_motion_xrt.c','lib/ufbx/ufbx.c']
    objects=[]
    for source in sources:
        obj=OUT/(Path(source).stem+'.o');record=obj.with_suffix('.json');dep=obj.with_suffix('.d')
        digest=hashlib.sha256((compiler+json.dumps(flags)).encode()).hexdigest()
        try:
            cached=json.loads(record.read_text())
            valid=obj.exists() and cached['key']==digest and all(stamp(p)==s for p,s in cached['deps'].items())
        except (OSError,KeyError,ValueError): valid=False
        if not valid:
            print('[compile]',source,flush=True)
            subprocess.run([*flags,'-MD','-MF',str(dep),'-MT',str(obj),'-c',source,'-o',str(obj)],cwd=ROOT,check=True)
            record.write_text(json.dumps({'key':digest,'deps':{p:stamp(p) for p in dependencies(dep)}}))
        objects.append(str(obj))
    exe=OUT/'xge3d_motion.exe'
    if not exe.exists() or any(Path(o).stat().st_mtime>exe.stat().st_mtime for o in objects):
        subprocess.run(['gcc',*objects,'-lm','-lws2_32','-o',str(exe)],cwd=ROOT,check=True)
    print(exe)

if __name__=='__main__': main()
