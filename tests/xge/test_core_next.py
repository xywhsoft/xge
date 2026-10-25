"""Build/run the optional collision, cache and node API tests against one SDK."""
import argparse
from pathlib import Path
import subprocess
ROOT=Path(__file__).resolve().parents[2]


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('sdk',nargs='?',default='build/core-next/3d')
    p.add_argument('--gpu',action='store_true');a=p.parse_args();out=ROOT/a.sdk
    config=(out/'xge_build_config.h').read_text()
    tests=[name for name,flag in [('nodes','SCENE'),('collision','COLLISION'),('cache','CACHE')]
           if '#define XGE3D_ENABLE_'+flag+' 1' in config]
    if a.gpu:tests=['core_gpu'] if all('#define XGE3D_ENABLE_'+flag+' 1' in config for flag in ['SCENE','COLLISION','CACHE','ASYNC','ANIMATION']) else []
    for name in tests:
        exe=out/('test_3d_'+name+'.exe')
        subprocess.run(['gcc','@config/includes.rsp','-O2','-Wall','-Wextra','-Werror','-Wno-missing-field-initializers',
                        '-DXGE_DLL','-include',str(out/'xge_build_config.h'),'tests/xge/test_3d_'+name+'.c',str(out/'xge.lib'),'-lm','-o',str(exe)],cwd=ROOT,check=True)
        subprocess.run([str(exe)],cwd=ROOT,check=True)


if __name__=='__main__':main()
