"""Run each profile alone, report actual GPU/CPU timing and process memory."""
import argparse
import csv
import ctypes
import hashlib
import json
import math
import os
from pathlib import Path
import platform
import statistics
import subprocess
import sys
from datetime import datetime,timezone
ROOT=Path(__file__).resolve().parents[1]
ART=ROOT/'artifacts/xge-3d'
def run(args,**kwargs):return subprocess.run([str(a) for a in args],cwd=ROOT,check=True,**kwargs)
def stats(values):
    v=sorted(values);assert v
    return {'samples':len(v),'median':statistics.median(v),'p95':v[math.ceil(.95*len(v))-1],'max':max(v)}
def report(profile,cycles):
    path=ART/f'p6-pressure-{profile}.csv';rows=list(csv.reader(path.open()))
    memory=[[int(v) for v in row[1:]] for row in rows if row[0]=='memory']
    assert len(memory)>=cycles//100 and memory[-1][0]==cycles,memory
    private=[m[1] for m in memory];delta=private[-1]-private[0]
    mean_x=statistics.mean(m[0] for m in memory);mean_y=statistics.mean(private)
    slope=sum((m[0]-mean_x)*(m[1]-mean_y) for m in memory)/sum((m[0]-mean_x)**2 for m in memory)
    # A large decrease is reclaimed memory, not a leak. Bound positive growth
    # above the warm baseline and its trend, while retaining the complete data.
    assert delta<=4*1024*1024 and max(private)-private[0]<=4*1024*1024 and slope<=2048,(delta,slope,private)
    heap=[m[4] for m in memory];blocks=[m[5] for m in memory]
    assert heap[-1]-heap[0]<=65536 and max(heap)-min(heap)<=131072 and blocks[-1]-blocks[0]<=16,(heap,blocks)
    steady=[m for m in memory if m[0]>=300]
    assert max(m[4] for m in steady)-min(m[4] for m in steady)<=16384 and max(m[5] for m in steady)-min(m[5] for m in steady)<=4,steady
    frames=[[int(v) for v in row[1:]] for row in rows if row[0]=='frame'];modes={}
    for mode,name in enumerate(['culling+instancing','instancing-only','culling-only','both-disabled','resource-and-animation-pressure']):
        selected=[f for f in frames if f[0]==mode]
        if mode<4:selected=selected[16:]
        assert len(selected)>=60,(name,len(selected))
        gpu=[f[2]/1000 for f in selected if f[2]]
        if mode<4:assert len(gpu)==len(selected),(name,len(gpu),len(selected))
        modes[name]={'cpu_us':stats([f[1] for f in selected]),'frame_interval_us':stats([f[3] for f in selected]),
                     'gpu_us':stats(gpu) if gpu else None,'draw_calls':sorted(set(f[4] for f in selected)),
                     'model_and_pose_upload_bytes':stats([f[5] for f in selected]) if mode<4 else None}
    assert max(modes['culling+instancing']['draw_calls'])<min(modes['both-disabled']['draw_calls']),modes
    loads=[list(map(int,row[1:])) for row in rows if row[0]=='loads'];assert len(loads)==1 and loads[0][0]==cycles//4
    log=ART/f'p6-pressure-{profile}.log';text=log.read_text(encoding='utf-8',errors='replace')
    assert f'pressure: {cycles} cycles' in text,text[-1000:]
    data={'profile':profile,'cycles':cycles,'recorded_utc':datetime.now(timezone.utc).isoformat(),
          'machine':{'platform':platform.platform(),'processor':os.environ.get('PROCESSOR_IDENTIFIER'),
                     'gpu':next(line[5:] for line in text.splitlines() if line.startswith('GPU: '))},
          'memory':{'samples':memory,'final_minus_warm_private_bytes':delta,'private_span_bytes':max(private)-min(private),
                    'slope_bytes_per_cycle':slope,'peak_working_set_bytes':max(m[3] for m in memory),
                    'crt_busy_bytes_span':max(heap)-min(heap),'crt_busy_blocks_span':max(blocks)-min(blocks),
                    'steady_crt_busy_bytes_span':max(m[4] for m in steady)-min(m[4] for m in steady),
                    'steady_crt_busy_blocks_span':max(m[5] for m in steady)-min(m[5] for m in steady)},
          'performance':modes,'complete_model_load_wall_us':{'samples':loads[0][0],'mean':loads[0][1]/loads[0][0],'max':loads[0][2]},
          'source_hashes':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in ['test/test_3d_pressure.c','test/test_3d_pressure.py','examples/xge_3d_integration/main.c']},
          'csv_sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'dll_sha256':hashlib.sha256((ROOT/'build'/profile/'xge.dll').read_bytes()).hexdigest(),
          'limits':['CPU Render timing is submission, GPU elapsed query is real offscreen 3D main+shadow work',
                    'four modes use a fixed camera/pose, 16 warm-up samples per mode omitted',
                    'frame intervals include window scheduling/presentation; individual runs, not simultaneous profiles',
                    'pressure timing includes application geometry queries, animation, resource handling and UI when enabled',
                    'process memory includes driver caches; matching application/DLL msvcrt busy heap checks retained CPU bytes/blocks; not proof of absence of every leak',
                    'load wall time includes polling cadence and GPU upload; not a standalone CPU parser benchmark']}
    (ART/f'p6-pressure-{profile}.json').write_text(json.dumps(data,indent=2),encoding='utf-8')
    print(profile,'pressure passed; private delta',delta,'B; slope',round(slope,2),'B/cycle;',
          'draw calls',modes['both-disabled']['draw_calls'],'->',modes['culling+instancing']['draw_calls'])
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('profiles',nargs='*',default=['3d','full-dev'])
    p.add_argument('--cycles',type=int,default=1200);p.add_argument('--report-existing',action='store_true');a=p.parse_args()
    if any(profile not in ['3d','full-dev'] for profile in a.profiles):p.error('profiles must be 3d or full-dev')
    if a.cycles<400 or a.cycles%100:p.error('--cycles must be >=400 and a multiple of 100')
    ART.mkdir(parents=True,exist_ok=True)
    for profile in a.profiles:
        if not a.report_existing:
            run([sys.executable,'examples/xge_3d_integration/build.py',profile]);out=ROOT/'build'/profile;exe=out/'test_3d_pressure.exe'
            run(['gcc','-O2','-Wall','-Wextra','-Werror','-Wno-missing-field-initializers','-DXGE_DLL','-DXUI_DLL','-I.',
                 '-include',out/'xge_build_config.h','test/test_3d_pressure.c',out/'xge.lib','-lm','-lpsapi','-o',exe])
            with (ART/f'p6-pressure-{profile}.log').open('w',encoding='utf-8') as log:
                run([exe,a.cycles,ART/f'p6-pressure-{profile}.csv'],stdout=log,stderr=subprocess.STDOUT)
        report(profile,a.cycles)
if __name__=='__main__':main()
