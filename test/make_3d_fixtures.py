"""Small reproducible glTF/GLB fixtures, including real PNGs and invalid bounds."""
import base64
import copy
import json
from pathlib import Path
import struct
import zlib

OUT=Path(__file__).resolve().parents[1]/'artifacts/xge-3d/fixtures'

def chunk(kind,data):
    return struct.pack('>I',len(data))+kind+data+struct.pack('>I',zlib.crc32(kind+data)&0xffffffff)

def main():
    OUT.mkdir(parents=True,exist_ok=True)
    png=b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',2,2,8,6,0,0,0))
    png+=chunk(b'IDAT',zlib.compress(b'\0'+bytes([255,0,0,255,255,255,255,255])+b'\0'+bytes([0,0,255,255,0,255,0,255])))+chunk(b'IEND',b'')
    positions=struct.pack('<9f',-1,-1,0,1,-1,0,0,1,0)
    buffer=positions+struct.pack('<3H',0,1,2)+b'\0\0'+struct.pack('<6f',0,1,1,1,.5,0)
    model={'asset':{'version':'2.0'},'scene':0,'scenes':[{'nodes':[0]}],
           'nodes':[{'name':'Parent','translation':[2,0,0],'children':[1]}, {'name':'Triangle','mesh':0,'translation':[0,1,0]}],
           'meshes':[{'primitives':[{'attributes':{'POSITION':0,'TEXCOORD_0':2},'indices':1,'material':0}]}],
           'buffers':[{'byteLength':len(buffer),'uri':'mesh%20data.bin'}],
           'bufferViews':[{'buffer':0,'byteOffset':0,'byteLength':36},{'buffer':0,'byteOffset':36,'byteLength':6},{'buffer':0,'byteOffset':44,'byteLength':24}],
           'accessors':[{'bufferView':0,'componentType':5126,'count':3,'type':'VEC3','min':[-1,-1,0],'max':[1,1,0]},
                        {'bufferView':1,'componentType':5123,'count':3,'type':'SCALAR'},
                        {'bufferView':2,'componentType':5126,'count':3,'type':'VEC2'}],
           'images':[{'uri':'texture%20tile.png'}], 'textures':[{'source':0}],
           'extensionsUsed':['KHR_materials_unlit'],
           'materials':[{'extensions':{'KHR_materials_unlit':{}},'pbrMetallicRoughness':{'baseColorTexture':{'index':0},'metallicFactor':0,'roughnessFactor':1}}]}
    def write(name,value): (OUT/name).write_text(json.dumps(value),encoding='utf-8')
    write('lights.gltf',{'asset':{'version':'2.0'},'scene':0,'scenes':[{'nodes':[0]}],
          'nodes':[{'extensions':{'KHR_lights_punctual':{'light':0}}}],
          'extensionsRequired':['KHR_lights_punctual'],'extensionsUsed':['KHR_lights_punctual'],
          'extensions':{'KHR_lights_punctual':{'lights':[{'type':'directional','intensity':1}]}}})
    (OUT/'mesh data.bin').write_bytes(buffer); (OUT/'texture tile.png').write_bytes(png)
    white=b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',1,1,8,6,0,0,0))
    white+=chunk(b'IDAT',zlib.compress(b'\0\xff\xff\xff\xff'))+chunk(b'IEND',b'')
    (OUT/'white.png').write_bytes(white)
    skin={'asset':{'version':'2.0'},'scene':0,'scenes':[{'nodes':[0]}],
          'nodes':[{'name':'Actor','children':[1,2]},{'name':'SkinMesh','mesh':0,'skin':0,'translation':[2,0,0]},
                   {'name':'Root','children':[3]},{'name':'Tip','translation':[1,0,0]}],
          'skins':[{'joints':[2,3],'skeleton':2,'inverseBindMatrices':3}],
          'materials':[{'extensions':{'KHR_materials_unlit':{}}}],'extensionsUsed':['KHR_materials_unlit'],
          'meshes':[{'primitives':[{'attributes':{'POSITION':0,'JOINTS_0':1,'WEIGHTS_0':2},'material':0}]}],
          'buffers':[],'bufferViews':[],'accessors':[]}
    skin_buffer=bytearray()
    def skin_accessor(values,fmt,component,kind,count):
        skin_buffer.extend(b'\0'*(-len(skin_buffer)%4));offset=len(skin_buffer);packed=struct.pack(fmt,*values);skin_buffer.extend(packed)
        skin['bufferViews'].append({'buffer':0,'byteOffset':offset,'byteLength':len(packed)})
        skin['accessors'].append({'bufferView':len(skin['bufferViews'])-1,'componentType':component,'count':count,'type':kind})
    skin_positions=[-.5,-.5,0,1.5,-.5,0,1.5,.5,0,-.5,-.5,0,1.5,.5,0,-.5,.5,0]
    skin_accessor(skin_positions,'<18f',5126,'VEC3',6)
    skin_accessor([j for bone in [0,1,1,0,1,0] for j in [bone,0,0,0]],'<24H',5123,'VEC4',6)
    skin_accessor([w for i in range(6) for w in [1,0,0,0]],'<24f',5126,'VEC4',6)
    identity=[1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1];tip=identity.copy();tip[12]=-1
    skin_accessor(identity+tip,'<32f',5126,'MAT4',2)
    skin['buffers']=[{'byteLength':len(skin_buffer),'uri':'data:application/octet-stream;base64,'+base64.b64encode(skin_buffer).decode()}]
    write('skin.gltf',skin)
    many=copy.deepcopy(skin);many_buffer=bytearray(skin_buffer)
    for attribute,fmt,component,values in [
        ('JOINTS_1','<24H',5123,[j for i in range(6) for j in [0,0,1,1]]),
        ('WEIGHTS_1','<24f',5126,[w for i in range(6) for w in [.5,.6,.7,.8]])]:
        packed=struct.pack(fmt,*values);offset=len(many_buffer);many_buffer.extend(packed)
        many['bufferViews'].append({'buffer':0,'byteOffset':offset,'byteLength':len(packed)})
        many['accessors'].append({'bufferView':len(many['bufferViews'])-1,'componentType':component,'count':6,'type':'VEC4'})
        many['meshes'][0]['primitives'][0]['attributes'][attribute]=len(many['accessors'])-1
    joint_offset=many['bufferViews'][1]['byteOffset'];weight_offset=many['bufferViews'][2]['byteOffset']
    many_buffer[joint_offset:joint_offset+48]=struct.pack('<24H',*[j for i in range(6) for j in [0,0,1,1]])
    many_buffer[weight_offset:weight_offset+96]=struct.pack('<24f',*[w for i in range(6) for w in [.1,.2,.3,.4]])
    many['buffers']=[{'byteLength':len(many_buffer),'uri':'data:application/octet-stream;base64,'+base64.b64encode(many_buffer).decode()}]
    write('skin-eight.gltf',many)
    primitive_children=copy.deepcopy(skin);primitive_children['meshes'][0]['primitives']*=2;write('skin-primitives.gltf',primitive_children)
    for name,change in [('joint',2),('negative',-1),('zero',0)]:
        bad=copy.deepcopy(skin);raw=bytearray(skin_buffer)
        if name=='joint': struct.pack_into('<H',raw,joint_offset,change)
        else: struct.pack_into('<4f',raw,weight_offset,*([change]*4))
        bad['buffers'][0]['uri']='data:application/octet-stream;base64,'+base64.b64encode(raw).decode()
        write('skin-bad-'+name+'.gltf',bad)
    huge=copy.deepcopy(skin);huge['skins'][0]['joints']=[2]*257;write('skin-large.gltf',huge)
    sparse_skin=copy.deepcopy(skin);sparse_bytes=b'\0\1\0\0'+struct.pack('<32f',*(identity+tip))
    sparse_skin['buffers'].append({'byteLength':len(sparse_bytes),'uri':'data:application/octet-stream;base64,'+base64.b64encode(sparse_bytes).decode()})
    sparse_skin['bufferViews'] += [{'buffer':1,'byteOffset':0,'byteLength':2},{'buffer':1,'byteOffset':4,'byteLength':128}]
    del sparse_skin['accessors'][3]['bufferView'];sparse_skin['accessors'][3]['sparse']={'count':2,'indices':{'bufferView':4,'componentType':5121},'values':{'bufferView':5}}
    write('skin-sparse.gltf',sparse_skin)
    # Separate animation-only sources, with compact node indices and shared names.
    for kind in ['linear','step','cubic','rotation','cubic-rotation']:
        rotation='rotation' in kind;cubic='cubic' in kind
        keys=[0,0,0,1,0,0,1,0] if rotation else [1,0,0,3,0,0]
        if cubic:
            keys=[0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0] if rotation else [0,0,0,1,0,0,2,0,0,0,0,0,3,0,0,0,0,0]
        motion_bytes=struct.pack('<2f',0,1)+struct.pack('<%df'%len(keys),*keys)
        motion={'asset':{'version':'2.0'},'scene':0,'scenes':[{'nodes':[0]}],
                'nodes':[{'name':'Root','children':[1]},{'name':'Tip','translation':[1,0,0]}],
                'buffers':[{'byteLength':len(motion_bytes),'uri':'data:application/octet-stream;base64,'+base64.b64encode(motion_bytes).decode()}],
                'bufferViews':[{'buffer':0,'byteLength':8},{'buffer':0,'byteOffset':8,'byteLength':len(motion_bytes)-8}],
                'accessors':[{'bufferView':0,'componentType':5126,'type':'SCALAR','count':2},
                             {'bufferView':1,'componentType':5126,'type':'VEC4' if rotation else 'VEC3','count':6 if cubic else 2}],
                'animations':[{'name':kind,'samplers':[{'input':0,'output':1,'interpolation':'CUBICSPLINE' if cubic else 'STEP' if kind=='step' else 'LINEAR'}],
                               'channels':[{'sampler':0,'target':{'node':1,'path':'rotation' if rotation else 'translation'}}]}]}
        write('motion-'+kind+'.gltf',motion)
    write('external.gltf',model)
    embedded=copy.deepcopy(model)
    embedded['buffers'][0]['uri']='data:application/octet-stream;base64,'+base64.b64encode(buffer).decode()
    embedded['images'][0]['uri']='data:image/png;base64,'+base64.b64encode(png).decode()
    write('embedded.gltf',embedded)
    # Larger than a frame budget; indexed and nonindexed meshes, shared texture
    # and mip completion exercise every resumable upload stage.
    async_model=copy.deepcopy(embedded);async_bytes=buffer+struct.pack('<9f',*([0,0,1]*3))
    async_model['buffers'][0]={'byteLength':len(async_bytes),'uri':'data:application/octet-stream;base64,'+base64.b64encode(async_bytes).decode()}
    async_model['bufferViews'].append({'buffer':0,'byteOffset':len(buffer),'byteLength':36})
    async_model['accessors'].append({'bufferView':3,'componentType':5126,'count':3,'type':'VEC3'})
    async_model['meshes'][0]['primitives'][0]['attributes']['NORMAL']=3
    async_model['meshes'][0]['primitives'].append(copy.deepcopy(async_model['meshes'][0]['primitives'][0]))
    del async_model['meshes'][0]['primitives'][1]['indices']
    rows=b''.join(b'\0'+bytes(v for x in range(64) for v in [x*4,y*7,160,255]) for y in range(33))
    large_png=b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',64,33,8,6,0,0,0))+chunk(b'IDAT',zlib.compress(rows))+chunk(b'IEND',b'')
    async_model['images'][0]['uri']='data:image/png;base64,'+base64.b64encode(large_png).decode()
    async_model['samplers']=[{'minFilter':9987,'magFilter':9729}];async_model['textures'][0]['sampler']=0
    write('async.gltf',async_model)
    oversized=copy.deepcopy(async_model)
    huge_png=b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',65537,1,8,6,0,0,0))+chunk(b'IDAT',zlib.compress(b'\0'+b'\xff'*(65537*4)))+chunk(b'IEND',b'')
    oversized['images'].append({'uri':'data:image/png;base64,'+base64.b64encode(huge_png).decode()})
    oversized['textures'].append({'source':1,'sampler':0});oversized['materials'].append(copy.deepcopy(oversized['materials'][0]))
    oversized['materials'][1]['pbrMetallicRoughness']['baseColorTexture']['index']=1
    oversized['meshes'][0]['primitives'][1]['material']=1;write('async-oversized.gltf',oversized)
    glb=copy.deepcopy(model);glb['buffers'][0]={'byteLength':len(buffer)+len(png)}
    glb['bufferViews'].append({'buffer':0,'byteOffset':len(buffer),'byteLength':len(png)})
    glb['images'][0]={'bufferView':3,'mimeType':'image/png'}
    text=json.dumps(glb).encode();text+=b' '*(-len(text)%4)
    binary=buffer+png;binary+=b'\0'*(-len(binary)%4)
    data=struct.pack('<III',0x46546c67,2,12+8+len(text)+8+len(binary))
    data+=struct.pack('<II',len(text),0x4e4f534a)+text+struct.pack('<II',len(binary),0x004e4942)+binary
    (OUT/'embedded.glb').write_bytes(data)
    noindex=copy.deepcopy(embedded); del noindex['meshes'][0]['primitives'][0]['indices'];write('nonindexed.gltf',noindex)
    sparse=copy.deepcopy(embedded);sparse_bytes=b'\0\1\2\0'+positions
    sparse['buffers'].append({'byteLength':len(sparse_bytes),'uri':'data:application/octet-stream;base64,'+base64.b64encode(sparse_bytes).decode()})
    sparse['bufferViews'] += [{'buffer':1,'byteOffset':0,'byteLength':3},{'buffer':1,'byteOffset':4,'byteLength':36}]
    a=sparse['accessors'][0];del a['bufferView'];a['sparse']={'count':3,'indices':{'bufferView':3,'componentType':5121},'values':{'bufferView':4}}
    write('sparse.gltf',sparse)
    shared=copy.deepcopy(embedded)
    shared['materials'].append(copy.deepcopy(shared['materials'][0]))
    shared['materials'][1]['pbrMetallicRoughness']['baseColorTexture']['extensions']={'KHR_texture_transform':{'offset':[.25,.5],'scale':[2,3],'rotation':.5,'texCoord':1}}
    shared['extensionsUsed']=['KHR_texture_transform'];shared['extensionsRequired']=['KHR_texture_transform']
    shared['meshes'][0]['primitives'].append(copy.deepcopy(shared['meshes'][0]['primitives'][0]))
    shared['meshes'][0]['primitives'][1]['material']=1
    shared['meshes'][0]['primitives'][1]['attributes']['TEXCOORD_1']=2
    write('shared-materials.gltf',shared)
    cases={}
    cases['overflow']=copy.deepcopy(embedded);cases['overflow']['accessors'][0]['count']=18446744073709551615
    cases['short']=copy.deepcopy(embedded);cases['short']['bufferViews'][0]['byteLength']=4
    cases['badindex']=copy.deepcopy(embedded);bad=positions+struct.pack('<3H',0,1,9)+buffer[42:]
    cases['badindex']['buffers'][0]['uri']='data:application/octet-stream;base64,'+base64.b64encode(bad).decode()
    cases['extension']=copy.deepcopy(embedded);cases['extension']['extensionsRequired']=['EXT_xge_test_unknown']
    cases['cycle']=copy.deepcopy(embedded);cases['cycle']['nodes'][1]['children']=[0]
    cases['missing']=copy.deepcopy(model);cases['missing']['buffers'][0]['uri']='missing.bin'
    cases['badbase64']=copy.deepcopy(embedded);cases['badbase64']['buffers'][0]['uri']='data:application/octet-stream;base64,A'
    cases['missinguv']=copy.deepcopy(embedded);cases['missinguv']['materials'][0]['pbrMetallicRoughness']['baseColorTexture']['texCoord']=1
    for name,value in cases.items(): write(name+'.gltf',value)
    print('glTF fixtures:',OUT)

if __name__=='__main__': main()
