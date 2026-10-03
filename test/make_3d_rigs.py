"""Two actual test rigs: different proportions, T/A reference poses and local axes."""
import base64
import copy
import json
import math
from pathlib import Path
import struct

OUT=Path(__file__).resolve().parents[1]/'artifacts/xge-3d/fixtures'
SEMANTICS=['root','hips','spine','chest','head','left_upper_arm','left_lower_arm','left_hand',
           'right_upper_arm','right_lower_arm','right_hand','left_upper_leg','left_lower_leg','left_foot',
           'right_upper_leg','right_lower_leg','right_foot']
PARENTS=[None,0,1,2,3,3,5,6,3,8,9,1,11,12,1,14,15]

def mul(a,b):
    x,y,z,w=a;X,Y,Z,W=b
    return [w*X+x*W+y*Z-z*Y,w*Y-x*Z+y*W+z*X,w*Z+x*Y-y*X+z*W,w*W-x*X-y*Y-z*Z]

def inverse(q): return [-q[0],-q[1],-q[2],q[3]]

def rotate(q,v): return mul(mul(q,[*v,0]),inverse(q))[:3]

def axis(k,angle):
    q=[0,0,0,math.cos(angle/2)];q[k]=math.sin(angle/2);return q

def reference(b):
    height=1.2 if b else .95
    shoulder=1.95 if b else 1.5
    upper,lower=(.55,.48) if b else (.38,.32)
    xs=.32 if b else .25
    points=[[0,0,0],[0,height,0],[0,height+.25,0],[0,shoulder-.1,0],[0,shoulder+.4,0]]
    quats=[[0,0,0,1] for _ in range(17)]
    for side in [1,-1]:
        arm=axis(2,(-35 if side==1 else 35)*math.pi/180) if b else [0,0,0,1]
        offset=rotate(arm,[side*upper,0,0]);second=rotate(arm,[side*lower,0,0])
        p=[side*xs,shoulder,0];q=[p[k]+offset[k] for k in range(3)];r=[q[k]+second[k] for k in range(3)]
        points.extend([p,q,r])
    knee=.56 if b else .48
    for side in [1,-1]: points.extend([[side*.18,height-.07,0],[side*.18,knee,0],[side*.18,.06,.03]])
    if b:
        quats=[axis(1,math.pi/2) for _ in range(17)];quats[0]=[0,0,0,1]
        for index,side in [(5,1),(8,-1)]:
            arm=axis(2,(-35 if side==1 else 35)*math.pi/180);twist=axis(0,side*math.pi/2)
            for i in [index,index+1,index+2]: quats[i]=mul(arm,twist)
    return points,quats

def make(b):
    points,world=reference(b);nodes=[];ref_overrides={}
    for i,semantic in enumerate(SEMANTICS):
        parent=PARENTS[i];parent_q=world[parent] if parent is not None else [0,0,0,1]
        delta=[points[i][k]-(points[parent][k] if parent is not None else 0) for k in range(3)]
        n={'name':('B_' if b else 'A_')+semantic,'translation':rotate(inverse(parent_q),delta),'rotation':mul(inverse(parent_q),world[i])}
        children=[j for j,p in enumerate(PARENTS) if p==i]
        if children: n['children']=children
        nodes.append(n)
        if b and semantic in ['left_upper_arm','right_upper_arm']:
            side=1 if semantic.startswith('left') else -1
            ref_overrides[semantic]=mul(inverse(parent_q),axis(0,side*math.pi/2))
    # Rigid bone prisms and head/pelvis cubes, weighted to actual joints.
    vertices=[];normals=[];joints=[];weights=[]
    def box(center,axes,half,joint):
        for face in range(6):
            k=face//2;sign=1 if face%2 else -1;a=(k+1)%3;c=(k+2)%3
            normal=[axes[k][i]*sign for i in range(3)]
            corners=[(-1,-1),(1,-1),(1,1),(-1,-1),(1,1),(-1,1)]
            if sign<0: corners.reverse()
            for u,v in corners:
                signs=[0,0,0];signs[k]=sign;signs[a]=u;signs[c]=v
                vertices.extend([center[i]+sum(axes[j][i]*half[j]*signs[j] for j in range(3)) for i in range(3)])
                normals.extend(normal);joints.extend([joint,0,0,0]);weights.extend([1,0,0,0])
    for child,parent in enumerate(PARENTS):
        if parent is None or child==1: continue
        d=[points[child][i]-points[parent][i] for i in range(3)];length=math.sqrt(sum(x*x for x in d));y=[x/length for x in d]
        x=[y[1],-y[0],0];n=math.sqrt(sum(v*v for v in x));x=[v/n for v in x] if n else [1,0,0]
        z=[x[1]*y[2]-x[2]*y[1],x[2]*y[0]-x[0]*y[2],x[0]*y[1]-x[1]*y[0]]
        center=[(points[child][i]+points[parent][i])/2 for i in range(3)]
        box(center,[x,y,z],[.06,length*.5,.06],parent)
    identity=[[1,0,0],[0,1,0],[0,0,1]]
    box([0,points[4][1]+.12,0],identity,[.15,.18,.13],4)
    box(points[1],identity,[.22,.12,.12],1)
    ibm=[]
    for p,q in zip(points,world):
        qi=inverse(q);columns=[rotate(qi,[1 if i==k else 0 for i in range(3)]) for k in range(3)]
        translation=rotate(qi,[-x for x in p]);ibm.extend([v for c in columns for v in [*c,0]]+[*translation,1])
    data={'asset':{'version':'2.0'},'scene':0,'scenes':[{'nodes':[0,17]}],'nodes':nodes+[{'name':'Mesh','mesh':0,'skin':0}],
          'skins':[{'joints':list(range(17)),'skeleton':0,'inverseBindMatrices':4}],
          'meshes':[{'primitives':[{'attributes':{'POSITION':0,'NORMAL':1,'JOINTS_0':2,'WEIGHTS_0':3},'material':0}]}],
          'materials':[{'pbrMetallicRoughness':{'metallicFactor':0,'roughnessFactor':1},'doubleSided':True}],
          'buffers':[],'bufferViews':[],'accessors':[]}
    binary=bytearray()
    def accessor(values,fmt,kind,count,component=5126):
        packed=struct.pack('<'+str(len(values))+fmt,*values);binary.extend(b'\0'*(-len(binary)%4));offset=len(binary);binary.extend(packed)
        index=len(data['accessors']);data['bufferViews'].append({'buffer':0,'byteOffset':offset,'byteLength':len(packed)})
        data['accessors'].append({'bufferView':index,'componentType':component,'count':count,'type':kind});return index
    count=len(vertices)//3
    accessor(vertices,'f','VEC3',count);accessor(normals,'f','VEC3',count);accessor(joints,'H','VEC4',count,5123);accessor(weights,'f','VEC4',count);accessor(ibm,'f','MAT4',17)
    if not b:
        times=accessor([0,1],'f','SCALAR',2)
        channels=[];samplers=[]
        for node,path,values,kind in [(0,'translation',[0,0,0,0,0,1],'VEC3'),(1,'translation',[0,.95,0,0,1.05,0],'VEC3'),
                                     (5,'rotation',[0,0,0,1,*axis(2,-math.pi/3)],'VEC4')]:
            values=accessor(values,'f',kind,2);channels.append({'sampler':len(samplers),'target':{'node':node,'path':path}});samplers.append({'input':times,'output':values,'interpolation':'LINEAR'})
        data['animations']=[{'name':'axis-probe','channels':channels,'samplers':samplers}]
    data['buffers']=[{'byteLength':len(binary),'uri':'data:application/octet-stream;base64,'+base64.b64encode(binary).decode()}]
    (OUT/('rig-b.gltf' if b else 'rig-a.gltf')).write_text(json.dumps(data),encoding='utf-8')
    source_names=['root','hips','spine','chest','head','upperarm.l','lowerarm.l','hand.l','upperarm.r','lowerarm.r','hand.r','upperleg.l','lowerleg.l','foot.l','upperleg.r','lowerleg.r','foot.r']
    mapping={'bones':[]}
    for semantic,source in zip(SEMANTICS,source_names):
        entry={'semantic':semantic,'source':source,'target':('B_' if b else 'A_')+semantic}
        if semantic in ref_overrides: entry['target_reference_rotation']=ref_overrides[semantic]
        mapping['bones'].append(entry)
    (OUT/('kaykit-to-b.json' if b else 'kaykit-to-a.json')).write_text(json.dumps(mapping),encoding='utf-8')
    if b:
        probe=copy.deepcopy(mapping)
        for entry in probe['bones']: entry['source']='A_'+entry['semantic']
        (OUT/'a-to-b.json').write_text(json.dumps(probe),encoding='utf-8')

def main():
    OUT.mkdir(parents=True,exist_ok=True);make(False);make(True);print('Distinct rigs and explicit semantic/reference maps:',OUT)

if __name__=='__main__': main()
