"""Build and run the actual profile DLL's focused 3D checks."""
import argparse
from pathlib import Path
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]

def run(args):
    subprocess.run([str(a) for a in args],cwd=ROOT,check=True)

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('subset',choices=['cpu','spatial','terrain','async','culling','instancing','fog','gpu','import','material','sky','light','shadow','ibl','skin','animation','motion','retarget','all'],nargs='?',default='cpu')
    p.add_argument('profile',choices=['3d','full-dev','ui-min'],nargs='?',default='3d')
    a=p.parse_args()
    if a.subset in ('motion','retarget'):
        run([sys.executable,'test/test_3d_'+a.subset+'.py',a.profile]);return
    extra=['--define','XGE_ENABLE_3D=1'] if a.profile=='ui-min' else []
    run([sys.executable,'tools/build_profile.py',a.profile,*extra])
    out=Path('build')/a.profile
    tests=['cpu','spatial','terrain','import','animation','async','gpu','import_gpu','material_gpu','sky_gpu','light_gpu','shadow_gpu','ibl_gpu','skin_gpu','culling_gpu','instancing_gpu','terrain_gpu','async_gpu','fog_gpu'] if a.subset=='all' else ['gpu','import_gpu','material_gpu','sky_gpu','light_gpu','shadow_gpu','ibl_gpu','skin_gpu','culling_gpu','instancing_gpu','terrain_gpu','async_gpu','fog_gpu'] if a.subset=='gpu' else ['terrain','terrain_gpu'] if a.subset=='terrain' else ['async','async_gpu'] if a.subset=='async' else [a.subset+'_gpu'] if a.subset in ('material','sky','light','shadow','ibl','skin','culling','instancing','fog') else [a.subset]
    config=(ROOT/out/'xge_build_config.h').read_text()
    if '#define XGE3D_ENABLE_MODEL 0' in config:
        tests=[t for t in tests if not t.startswith('import')]
    if '#define XGE3D_ENABLE_LIGHTING 0' in config:
        tests=[t for t in tests if t not in ('light_gpu','culling_gpu','instancing_gpu','terrain_gpu')]
    if '#define XGE3D_ENABLE_SHADOW 0' in config:
        tests=[t for t in tests if t not in ('shadow_gpu','culling_gpu','instancing_gpu','terrain_gpu')]
    if '#define XGE3D_ENABLE_IBL 0' in config:
        tests=[t for t in tests if t!='ibl_gpu']
    if '#define XGE3D_ENABLE_TERRAIN 0' in config:
        tests=[t for t in tests if t not in ('terrain','terrain_gpu')]
    if '#define XGE3D_ENABLE_ASYNC 0' in config:
        tests=[t for t in tests if t not in ('async','async_gpu')]
    if '#define XGE3D_ENABLE_ANIMATION 0' in config:
        tests=[t for t in tests if t not in ('skin_gpu','animation','culling_gpu')]
    if any(t.startswith('import') or t in ('light_gpu','ibl_gpu','skin_gpu','animation','spatial','culling_gpu','async','async_gpu') for t in tests):
        run([sys.executable,'test/make_3d_fixtures.py'])
    if 'ibl_gpu' in tests:
        tool=out/'xge3d_ibl.exe'
        run(['gcc','-O2','-Wall','-Wextra','-Werror','-Wno-missing-field-initializers',
             '-DXGE_DLL','-include',out/'xge_build_config.h','tools/xge3d_ibl.c',out/'xge.lib','-lm','-o',tool])
        run([tool.resolve(),'artifacts/xge-3d/fixtures/ibl',*(['artifacts/xge-3d/fixtures/white.png']*6),'4','64'])
    for test in tests:
        exe=out/f'test_3d_{test}.exe'
        run(['gcc','-O2','-Wall','-Wextra','-Werror','-Wno-missing-field-initializers',
             '-DXGE_DLL','-DXUI_DLL','-include',out/'xge_build_config.h',
             f'test/test_3d_{test}.c',out/'xge.lib','-lm','-o',exe])
        run([exe.resolve()])

if __name__=='__main__':
    main()
