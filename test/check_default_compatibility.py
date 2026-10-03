"""Compare the current default DLL/header with the preserved pre-3D baseline."""
import hashlib
import json
from pathlib import Path
import re
import subprocess
ROOT=Path(__file__).resolve().parents[1]
BASE=ROOT/'artifacts/xge-3d/baseline'
def run(args,**kwargs):return subprocess.run([str(a) for a in args],cwd=ROOT,check=True,**kwargs)
def exports(path):
    text=run(['objdump','-p',path],capture_output=True,text=True).stdout
    table=text.split('[Ordinal/Name Pointer] Table',1)[1].split('\n\n',1)[0]
    symbols=set(re.findall(r'^\s*\[\s*\d+\].*\s+(\S+)\s*$',table,re.M))
    assert len(symbols)>5000,('export table not parsed',path,len(symbols))
    return symbols
def layout(name,header_root):
    types={'xge_desc_t':['iWidth','iRunMode'],'xge_texture_t':['iWidth','pBackend'],
           'xge_buffer_t':['iSize','pData'],'xge_image_t':['iStride','pPixels'],
           'xge_render_target_t':['iFlags','tTexture'],'xge_pass_t':['bActive'],
           'xge_vertex_t':['fX','iColor'],'xge_draw_t':['pTexture','tDst','iFlags'],
           'xge_material_t':['pShader','tPipeline'],'xge_shader_t':[],
           'xge_rect_t':['fX','fW'],'xge_rect_i_t':['iX','iW'],'xge_vec2_t':[],
           'xui_proxy_t':['iVersion','surfaceDraw','fontLoadFile'],
           'xui_surface_desc_t':['iWidth','iFlags'],'xui_button_desc_t':['iSize','sText'],
           'xui_layout_t':[]}
    code=f'#include "{(header_root/"xge.h").as_posix()}"\n#include "{(header_root/"xui.h").as_posix()}"\n#include <stddef.h>\n#include <stdio.h>\nint main(void){{\n'
    for typ,fields in types.items():
        code+=f'printf("{typ}:%zu",sizeof({typ}));\n'
        for field in fields:code+=f'printf(",{field}=%zu",offsetof({typ},{field}));\n'
        code+='puts("");\n'
    code+='return 0;}\n';out=ROOT/'artifacts/xge-3d';src=out/f'p6-layout-{name}.c';exe=out/f'p6-layout-{name}.exe'
    src.write_text(code,encoding='utf-8')
    run(['gcc','-O2','-Wall','-Wextra','-Werror','-DXGE_DLL','-DXUI_DLL','-DXGE_DEBUGMODE=0','-I.',src,'-o',exe])
    return run([exe],capture_output=True,text=True).stdout
def main():
    old=BASE/'default/xge.dll';new=ROOT/'build/xge.dll';assert old.exists() and (BASE/'original/xge.h').exists(),'preserved baseline required'
    before,after=exports(old),exports(new);assert not before-after,sorted(before-after)
    previous,current=layout('baseline',BASE/'original'),layout('current',ROOT);assert previous==current,(previous,current)
    data={'baseline_exports':len(before),'current_exports':len(after),'missing':sorted(before-after),'added':sorted(after-before),
          'layout_probes':previous.splitlines(),'baseline_runtime_bytes':(BASE/'default/xge.stripped.dll').stat().st_size,
          'current_runtime_bytes':new.stat().st_size,'current_sha256':hashlib.sha256(new.read_bytes()).hexdigest(),
          'limits':'representative legacy structure sizes and member offsets, not an exhaustive ABI proof'}
    assert data['current_runtime_bytes']==data['baseline_runtime_bytes']
    (ROOT/'artifacts/xge-3d/p6-default-compatibility.json').write_text(json.dumps(data,indent=2))
    print('default compatibility:',len(before),'old exports retained,',len(data['layout_probes']),'legacy layouts identical, runtime bytes unchanged')
if __name__=='__main__':main()
