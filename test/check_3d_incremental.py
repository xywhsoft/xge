"""Verify a 3D implementation-only change rebuilds only its owning TU."""
import json
import os
from pathlib import Path
import subprocess
import sys
import time
ROOT=Path(__file__).resolve().parents[1]
def build():
    subprocess.run([sys.executable,'tools/build_profile.py','3d'],cwd=ROOT,check=True)
    return json.loads((ROOT/'build/3d/build-report.json').read_text())
def main():
    before=build();assert not before['compiled'] and not before['linked']
    source=ROOT/'src/xge3d_async.c';stamp=source.stat()
    try:
        os.utime(source,ns=(stamp.st_atime_ns,max(time.time_ns(),stamp.st_mtime_ns+1000000)))
        changed=build();assert changed['compiled']==['src/xge3d_async.c'] and changed['linked'],changed
    finally:
        os.utime(source,ns=(stamp.st_atime_ns,stamp.st_mtime_ns))
        restored=build()
    assert restored['compiled']==['src/xge3d_async.c']
    hot=build();assert not hot['compiled'] and not hot['linked']
    data={'hot':hot,'single_TU':changed,'restored':restored,'method':'mtime change without source-content mutation; restored in finally'}
    (ROOT/'artifacts/xge-3d/p6-incremental.json').write_text(json.dumps(data,indent=2))
    print('3D incremental:',changed['seconds'],'s,',len(changed['reused']),'other objects reused; hot',hot['seconds'],'s')
if __name__=='__main__':main()
