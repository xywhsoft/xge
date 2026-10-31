"""Build a procedural C light-effects comparison with independent runtime switches."""
import argparse
from pathlib import Path
import subprocess
import sys
ROOT=Path(__file__).resolve().parents[3];OUT=ROOT/'build/light-effects/demo'
def run(args):subprocess.run([str(x) for x in args],cwd=ROOT,check=True)
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--run',action='store_true');p.add_argument('--frames',type=int)
    p.add_argument('--capture');p.add_argument('--effects',choices=['all','none'],default='all');p.add_argument('--levels',type=int,default=4);a=p.parse_args()
    if a.frames is not None and a.frames<=0:p.error('--frames must be positive')
    if not 1<=a.levels<=6:p.error('--levels must be 1..6')
    flags=['MODEL','ANIMATION','ASYNC','IBL','TERRAIN','FOG','SCENE','COLLISION','CACHE']
    run([sys.executable,'tools/build/build.py','3d',OUT,*[x for f in flags for x in ['--define','XGE3D_ENABLE_'+f+'=0']]])
    exe=OUT/'xge_3d_light_effects.exe'
    run(['gcc','@config/includes.rsp','-O2','-Wall','-Wextra','-Werror','-Wno-missing-field-initializers','-DXGE_DLL','-include',OUT/'xge_build_config.h',
         'examples/xge/xge_3d_light_effects/main.c',OUT/'xge.lib','-lm','-o',exe])
    if a.run:
        args=[exe,'--effects',a.effects,'--levels',a.levels]
        if a.frames:args+=['--frames',a.frames]
        if a.capture:
            capture=(ROOT/a.capture).resolve();capture.parent.mkdir(parents=True,exist_ok=True);args+=['--capture',capture]
        run(args)
if __name__=='__main__':main()
