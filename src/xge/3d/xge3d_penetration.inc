/* Bounded EPA on support-mapped affine primitives/triangles. At the storage
 * limit the support plane gives a conservative separating displacement. */
#if XGE3D_ENABLE_MOTION
static cv cvcross(cv a,cv b){return (cv){a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
typedef struct epa_face {int a,b,c;cv normal;double distance;} epa_face;
static int epa_face_make(epa_face *f,const support_point *v,int a,int b,int c,cv inside)
{
    cv n=cvcross(cvsub(v[b].p,v[a].p),cvsub(v[c].p,v[a].p));double length=sqrt(cvdot(n,n));if(length<1e-18){return 0;}n=cvscale(n,1/length);
    if(cvdot(n,cvsub(v[a].p,inside))<0){int t=b;b=c;c=t;n=cvscale(n,-1);}
    *f=(epa_face){a,b,c,n,fmax(0,cvdot(n,v[a].p))};return 1;
}
static int epa_tetra(const support_point *v,int a,int b,int c,int d,double *score)
{
    cv x=cvsub(v[b].p,v[a].p),y=cvsub(v[c].p,v[a].p),z=cvsub(v[d].p,v[a].p),o=cvscale(v[a].p,-1);
    double det=cvdot(x,cvcross(y,z));if(fabs(det)<1e-20){return 0;}
    double w1=cvdot(o,cvcross(y,z))/det,w2=cvdot(x,cvcross(o,z))/det,w3=cvdot(x,cvcross(y,o))/det;
    *score=fmin(fmin(w1,w2),fmin(w3,1-w1-w2-w3));return *score>=-1e-9;
}
static int uniform_sphere(const convex *a,cv *center,double *radius)
{
    if(a->tri||a->shape->type!=XGE3D_SHAPE_SPHERE){return 0;}const float *m=a->world.m;
    cv x={m[0],m[1],m[2]},y={m[4],m[5],m[6]},z={m[8],m[9],m[10]};double r=cvdot(x,x);
    if(fabs(cvdot(y,y)-r)>1e-7*fmax(1,r)||fabs(cvdot(z,z)-r)>1e-7*fmax(1,r)||fabs(cvdot(x,y))+fabs(cvdot(x,z))+fabs(cvdot(y,z))>1e-7*fmax(1,r)){return 0;}
    *center=cvadd(cvpoint(&a->world,cvfrom(a->shape->center)),a->offset);*radius=sqrt(r)*a->shape->radius;return 1;
}
static int convex_penetration(const convex *a,const convex *b,cv *normal,cv *point,double *depth)
{
    support_point v[96];int count=0;double upper,lower;int result=convex_distance_seed(a,b,normal,point,&upper,&lower,v,&count);if(result){return result;}
    if(upper>1e-5){return XGE_ERROR_NOT_FOUND;}if(upper>=1e-9){*depth=0;return XGE_OK;}
    support_point touch=minkowski(a,b,cvscale(*normal,-1));double touching=-cvdot(touch.p,*normal);
    if(touching<=1e-5&&touching>=-1e-5){*depth=fmax(0,touching);*point=touch.b;return XGE_OK;}
    cv ca,cb;double ra,rb;if(uniform_sphere(a,&ca,&ra)&&uniform_sphere(b,&cb,&rb)){
        cv delta=cvsub(ca,cb);double distance=sqrt(cvdot(delta,delta));*normal=distance>1e-12?cvscale(delta,1/distance):(cv){1,0,0};
        *depth=fmax(0,ra+rb-distance);*point=cvadd(cb,cvscale(*normal,rb));return XGE_OK;
    }
    for(int x=-1;x<=1;++x){for(int y=-1;y<=1;++y){for(int z=-1;z<=1;++z){if(!x&&!y&&!z){continue;}
        support_point p=minkowski(a,b,(cv){x,y,z});int duplicate=0;for(int i=0;i<count;++i){if(cvdot(cvsub(v[i].p,p.p),cvsub(v[i].p,p.p))<1e-24){duplicate=1;break;}}if(!duplicate){v[count++]=p;}
    }}}
    int ids[4]={0},found=0;double best=-DBL_MAX;
    for(int i=0;i<count-3&&!found;++i){for(int j=i+1;j<count-2&&!found;++j){for(int k=j+1;k<count-1&&!found;++k){for(int l=k+1;l<count;++l){double score;
        if(epa_tetra(v,i,j,k,l,&score)&&score>best){best=score;ids[0]=i;ids[1]=j;ids[2]=k;ids[3]=l;if(score>1e-5){found=1;break;}}
    }}}}
    if(best< -1e-9){return XGE_ERROR_UNSUPPORTED;}
    support_point seed[4];cv inside={0};for(int i=0;i<4;++i){seed[i]=v[ids[i]];inside=cvadd(inside,cvscale(seed[i].p,.25));}memcpy(v,seed,sizeof(seed));count=4;
    epa_face faces[192];int faces_count=0;const int triangles[4][3]={{0,1,2},{0,3,1},{0,2,3},{1,3,2}};
    for(int i=0;i<4;++i){if(epa_face_make(faces+faces_count,v,triangles[i][0],triangles[i][1],triangles[i][2],inside)){++faces_count;}}
    if(faces_count<4){return XGE_ERROR_UNSUPPORTED;}
    for(int iteration=0;iteration<128;++iteration){int at=0;for(int i=1;i<faces_count;++i){if(faces[i].distance<faces[at].distance){at=i;}}
        epa_face f=faces[at];support_point next=minkowski(a,b,f.normal);double distance=cvdot(next.p,f.normal);
        int duplicate=0;for(int i=0;i<count;++i){if(cvdot(cvsub(v[i].p,next.p),cvsub(v[i].p,next.p))<1e-22){duplicate=1;break;}}
        if(distance-f.distance<=1e-5*fmax(1,distance)||duplicate||count>=96||faces_count>180||iteration==127){
            *normal=cvscale(f.normal,-1);*depth=fmax(0,distance);support_point tri[]={v[f.a],v[f.b],v[f.c]};int n=3;cv pa,closest;
            if(!simplex_reduce(tri,&n,&pa,point,&closest)){return XGE_ERROR_INVALID_STATE;}return XGE_OK;
        }
        int edges[576][2],edge_count=0,n=0;
        for(int i=0;i<faces_count;++i){epa_face old=faces[i];if(cvdot(old.normal,cvsub(next.p,v[old.a].p))>1e-10){
                int links[3][2]={{old.a,old.b},{old.b,old.c},{old.c,old.a}};
                for(int k=0;k<3;++k){int reverse=-1;for(int e=0;e<edge_count;++e){if(edges[e][0]==links[k][1]&&edges[e][1]==links[k][0]){reverse=e;break;}}
                    if(reverse>=0){edges[reverse][0]=edges[--edge_count][0];edges[reverse][1]=edges[edge_count][1];}
                    else{edges[edge_count][0]=links[k][0];edges[edge_count++][1]=links[k][1];}
                }
            }else faces[n++]=old;
        }
        if(n+edge_count>192||!edge_count){return XGE_ERROR_INVALID_STATE;}faces_count=n;v[count]=next;
        for(int i=0;i<edge_count;++i){if(epa_face_make(faces+faces_count,v,edges[i][0],edges[i][1],count,inside)){++faces_count;}}
        ++count;
    }return XGE_ERROR_INVALID_STATE;
}
#endif
