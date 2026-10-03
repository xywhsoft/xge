"""Run explicit-map retargeting, diagnostics and public-runtime bone checks."""
import hashlib
import json
from pathlib import Path
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]
FIXTURES=ROOT/'artifacts/xge-3d/fixtures'

def run(args,ok=True):
    result=subprocess.run([str(a) for a in args],cwd=ROOT)
    assert (result.returncode==0)==ok,args

def main():
    profile=sys.argv[1] if len(sys.argv)>1 else '3d';assert profile in ('3d','full-dev')
    out=Path('build')/profile;run([sys.executable,'tools/build_profile.py',profile])
    run([sys.executable,'tools/build_3d_tools.py']);run([sys.executable,'test/make_3d_rigs.py'])
    if not (FIXTURES/'kaykit-walk.gltf').exists(): run([sys.executable,'test/test_3d_motion.py',profile])
    tool=ROOT/'build/3d-tools/xge3d_motion.exe';actions=['idle','walk','run','jump','slash','gather']
    run([tool,'convert',FIXTURES/'rig-a.gltf',FIXTURES/'probe-b.gltf','--target',FIXTURES/'rig-b.gltf','--map',FIXTURES/'a-to-b.json'])
    hashes={}
    for rig in ['a','b']:
        for action in actions:
            output=FIXTURES/f'retarget-{rig}-{action}.gltf'
            args=[tool,'convert',FIXTURES/f'kaykit-{action}.gltf',output,'--target',FIXTURES/f'rig-{rig}.gltf','--map',FIXTURES/f'kaykit-to-{rig}.json']
            run(args);before=hashlib.sha256(output.read_bytes()).hexdigest();run(args)
            assert hashlib.sha256(output.read_bytes()).hexdigest()==before;hashes[f'{rig}-{action}']=before
    valid=json.loads((FIXTURES/'a-to-b.json').read_text())
    for case in ['missing','duplicate','semantic','scale']:
        mapping=json.loads(json.dumps(valid))
        if case=='missing': mapping['bones'][5]['target']='absent'
        elif case=='duplicate': mapping['bones'][5]['source']=mapping['bones'][6]['source']
        elif case=='semantic': mapping['bones'].pop()
        else: mapping['translation_scale']=0
        path=FIXTURES/f'map-bad-{case}.json';path.write_text(json.dumps(mapping));output=FIXTURES/f'retarget-bad-{case}.gltf'
        run([tool,'convert',FIXTURES/'rig-a.gltf',output,'--target',FIXTURES/'rig-b.gltf','--map',path],False);assert not output.exists()
    exe=ROOT/out/'test_3d_retarget.exe'
    run(['gcc','-O2','-Wall','-Wextra','-Werror','-DXGE_DLL','-include',out/'xge_build_config.h',
         'test/test_3d_retarget.c',out/'xge.lib','-lm','-o',exe]);run([exe])
    (ROOT/'artifacts/xge-3d/p4-retarget-hashes.json').write_text(json.dumps(hashes,indent=2))

if __name__=='__main__': main()
