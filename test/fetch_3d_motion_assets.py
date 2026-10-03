"""Fetch only the pinned, hashed CC0 validation inputs; never fetch during DLL build."""
import hashlib
import json
from pathlib import Path
import urllib.request

ROOT=Path(__file__).resolve().parents[1]

def main():
    manifest=json.loads((ROOT/'test/assets/3d_motion/kaykit.json').read_text())
    out=ROOT/'artifacts/xge-3d/external/kaykit';out.mkdir(parents=True,exist_ok=True)
    for name,info in manifest['files'].items():
        path=out/name
        if path.exists() and hashlib.sha256(path.read_bytes()).hexdigest()==info['sha256']: continue
        request=urllib.request.Request(info['url'],headers={'User-Agent':'XGE-3D-asset-validation'})
        with urllib.request.urlopen(request,timeout=60) as response: data=response.read(info['bytes']+1)
        assert len(data)==info['bytes'] and hashlib.sha256(data).hexdigest()==info['sha256'],name
        path.write_bytes(data);print('fetched',name,len(data))
    (out/'SOURCE.json').write_text(json.dumps(manifest,indent=2))

if __name__=='__main__': main()
