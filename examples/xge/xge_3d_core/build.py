"""Build the three generic engine mechanisms example with its matching SDK."""
import argparse
from pathlib import Path
import shutil
import subprocess
import sys
ROOT=Path(__file__).resolve().parents[3]
OUT=ROOT/'build/core-next/demo'


def run(args):subprocess.run([str(v) for v in args],cwd=ROOT,check=True)


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--run',action='store_true');p.add_argument('--frames',type=int)
    p.add_argument('--automatic',action='store_true');p.add_argument('--capture');a=p.parse_args()
    if a.frames is not None and a.frames<=0:p.error('--frames must be positive')
    run([sys.executable,'tools/build/build.py','3d',OUT,'--define','XGE3D_ENABLE_ASYNC=0','--define','XGE3D_ENABLE_IBL=0','--define','XGE3D_ENABLE_TERRAIN=0'])
    assets=OUT/'assets';assets.mkdir(exist_ok=True)
    shutil.copy2(ROOT/'examples/xge/xge_3d_walk/assets/explorer.gltf',assets/'explorer.gltf')
    exe=OUT/'xge_3d_core.exe'
    run(['gcc','@config/includes.rsp','-O2','-Wall','-Wextra','-Werror','-Wno-missing-field-initializers','-DXGE_DLL','-include',OUT/'xge_build_config.h',
         'examples/xge/xge_3d_core/main.c',OUT/'xge.lib','-lm','-o',exe])
    if a.run:
        args=[exe]
        if a.frames:args+=['--frames',a.frames]
        if a.capture:args+=['--capture',a.capture]
        if a.automatic:args+=['--automatic']
        run(args)


if __name__=='__main__':main()
