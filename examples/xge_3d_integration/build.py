"""Build the public-API integration demo with its matching profile DLL."""
import argparse
from pathlib import Path
import subprocess
import sys
ROOT=Path(__file__).resolve().parents[2]
def run(args):subprocess.run([str(a) for a in args],cwd=ROOT,check=True)
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('profile',choices=['3d','full-dev'],nargs='?',default='full-dev')
    p.add_argument('--run',action='store_true');p.add_argument('--frames',type=int,default=360)
    p.add_argument('--capture');a=p.parse_args()
    if a.frames<=0:p.error('--frames must be positive')
    run([sys.executable,'tools/build_profile.py',a.profile]);run([sys.executable,'test/make_3d_fixtures.py'])
    fixtures=ROOT/'artifacts/xge-3d/fixtures'
    required=[fixtures/f'rig-{rig}.gltf' for rig in ['a','b']]
    required += [fixtures/f'retarget-{rig}-{action}.gltf' for rig in ['a','b'] for action in ['idle','walk','run','jump','slash','gather']]
    if any(not f.exists() for f in required):run([sys.executable,'test/test_3d_retarget.py',a.profile])
    out=ROOT/'build'/a.profile;exe=out/'xge_3d_integration.exe'
    run(['gcc','-O2','-Wall','-Wextra','-Werror','-Wno-missing-field-initializers','-DXGE_DLL','-DXUI_DLL','-I.',
         '-include',out/'xge_build_config.h','examples/xge_3d_integration/main.c',out/'xge.lib','-lm','-o',exe])
    if a.run:run([exe,'--frames',a.frames,*(['--capture',a.capture] if a.capture else [])])
if __name__=='__main__':main()
