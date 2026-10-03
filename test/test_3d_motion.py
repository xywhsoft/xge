"""Exercise C conversion tools, then load/play their files through the real DLL."""
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
    out=Path('build')/profile
    run([sys.executable,'tools/build_3d_tools.py'])
    run([sys.executable,'test/fetch_3d_motion_assets.py'])
    run([sys.executable,'tools/build_profile.py',profile])
    tool=ROOT/'build/3d-tools/xge3d_motion.exe'
    FIXTURES.mkdir(parents=True,exist_ok=True)
    bvh='''HIERARCHY
ROOT Hips
{
 OFFSET 0 0 0
 CHANNELS 6 Xposition Yposition Zposition Zrotation Xrotation Yrotation
 JOINT Tip
 {
  OFFSET 100 0 0
  CHANNELS 3 Zrotation Xrotation Yrotation
  End Site { OFFSET 100 0 0 }
 }
}
MOTION
Frames: 3
Frame Time: 0.5
0 100 0 0 0 0 0 0 0
100 100 0 90 0 0 90 0 0
200 100 0 180 0 0 0 0 0
'''
    source=FIXTURES/'test.bvh';source.write_text(bvh)
    for up in ['y','z']:
        run([tool,'convert',source,FIXTURES/f'bvh-{up}.gltf','--fps','4','--unit','.01','--up',up])
    for name,text in {'truncated':bvh[:-4],'huge':bvh.replace('Frames: 3','Frames: 999999999999'),
                      'channel':bvh.replace('Zrotation Xrotation Yrotation','Zrotation Zrotation Yrotation'),
                      'nan':bvh.replace('Time: 0.5','Time: nan')}.items():
        bad=FIXTURES/f'bvh-bad-{name}.bvh';bad.write_text(text);output=FIXTURES/f'bvh-bad-{name}.gltf'
        run([tool,'convert',bad,output],False);assert not output.exists()
    fbx=ROOT/'artifacts/xge-3d/external/kaykit/Knight.fbx'
    actions={'idle':'Idle','walk':'Walking_A','run':'Running_A','jump':'Jump_Full_Long',
             'slash':'1H_Melee_Attack_Slice_Horizontal','gather':'PickUp'}
    hashes={}
    for name,clip in actions.items():
        output=FIXTURES/f'kaykit-{name}.gltf';run([tool,'convert',fbx,output,'--clip',clip,'--fps','30'])
        before=hashlib.sha256(output.read_bytes()).hexdigest()
        run([tool,'convert',fbx,output,'--clip',clip,'--fps','30'])
        assert hashlib.sha256(output.read_bytes()).hexdigest()==before;hashes[name]=before
    cut=FIXTURES/'kaykit-walk-cut.gltf';run([tool,'convert',fbx,cut,'--clip','Walking_A','--start','.25','--end','.75','--fps','12'])
    data=json.loads(cut.read_text());assert data['accessors'][0]['count']==7 and data['accessors'][0]['max']==[.5]
    run([tool,'convert',fbx,FIXTURES/'missing-clip.gltf','--clip','missing'],False)
    exe=ROOT/out/'test_3d_motion.exe'
    run(['gcc','-O2','-Wall','-Wextra','-Werror','-DXGE_DLL','-include',out/'xge_build_config.h',
         'test/test_3d_motion.c',out/'xge.lib','-lm','-o',exe])
    run([exe]);(ROOT/'artifacts/xge-3d/p4-motion-hashes.json').write_text(json.dumps(hashes,indent=2))

if __name__=='__main__': main()
