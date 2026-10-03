"""Verify async=0 removes implementation, declarations and DLL exports."""
import ctypes
import json
from pathlib import Path
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'build/3d-async-off'
def run(args,**kwargs):
    return subprocess.run([str(a) for a in args],cwd=ROOT,check=True,**kwargs)
def main():
    run([sys.executable,'test/make_3d_fixtures.py'])
    run([sys.executable,'tools/build_profile.py','3d',OUT,'--define','XGE3D_ENABLE_ASYNC=0'])
    report=json.loads((OUT/'build-report.json').read_text())
    assert 'src/xge3d_async.c' not in report['sources']
    assert 'XGE3D_ENABLE_ASYNC' not in report['features']
    dll=ctypes.CDLL(str(OUT/'xge.dll'))
    args=['gcc','-O2','-Wall','-Wextra','-Werror','-Wno-missing-field-initializers','-DXGE_DLL','-DXUI_DLL','-I.',
          '-include',OUT/'xge_build_config.h']
    names=['Create','Free','Request','Status','Pump','Cancel','Release','Take']
    for name in names:
        symbol='xge3dLoader'+name
        assert not getattr(dll,symbol,None),symbol
        probe='#include "xge.h"\nvoid *entry=(void*)&'+symbol+';\n'
        result=subprocess.run([*map(str,args),'-fsyntax-only','-x','c','-'],input=probe,cwd=ROOT,text=True,capture_output=True)
        assert result.returncode!=0,('declaration remains',symbol)
    for test in ['import','import_gpu']:
        exe=OUT/f'test_3d_{test}.exe'
        run([*args,'test/test_3d_'+test+'.c',OUT/'xge.lib','-lm','-o',exe]);run([exe])
    print(f"3D async-off: TU, 8 declarations and 8 exports removed; actual model CPU/GPU passed; {report['runtime_bytes']} runtime bytes")
if __name__=='__main__':main()
