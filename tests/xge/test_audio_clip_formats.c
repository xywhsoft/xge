#include "xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
static uint64_t be64(const unsigned char *p){uint64_t n=0;for(int i=0;i<8;++i)n=(n<<8)|p[i];return n;}
static uint64_t le64(const unsigned char *p){uint64_t n=0;for(int i=7;i>=0;--i)n=(n<<8)|p[i];return n;}
int main(int argc,char **argv){if(argc!=4)return 2;int failures=0;printf("{\"formats\":[");for(int i=1;i<argc;++i){FILE *f=fopen(argv[i],"rb");if(!f)return 2;fseek(f,0,SEEK_END);long length=ftell(f);rewind(f);unsigned char *data=length>0?malloc((size_t)length):NULL;if(!data||fread(data,1,(size_t)length,f)!=(size_t)length){fclose(f);free(data);return 2;}fclose(f);
    uint64_t expected_frames=0;uint32_t rate=0,channels=0;
    if(length>=42&&!memcmp(data,"fLaC",4)){uint64_t word=be64(data+18);rate=(uint32_t)(word>>44);channels=(uint32_t)((word>>41)&7)+1;expected_frames=word&UINT64_C(0xfffffffff);}
    else if(length>=27&&!memcmp(data,"OggS",4)){size_t page=0;while(page+27<=(size_t)length&&!memcmp(data+page,"OggS",4)){size_t segments=data[page+26],size=27+segments;if(page+size>(size_t)length)break;for(size_t k=0;k<segments;++k)size+=data[page+27+k];if(page+size>(size_t)length)break;if(data[page+5]&4)expected_frames=le64(data+page+6);page+=size;}}
    xge_audio_clip *clip=NULL;xge_audio_clip_decode_t desc={data,(size_t)length,128u*1024u*1024u,NULL,NULL};int error=xgeAudioClipDecodeMemory(&desc,&clip);xge_audio_clip_info_t info=xgeAudioClipInfo(clip);int pass=!error&&info.frames>1000&&info.sample_rate>0&&info.channels>0&&info.bytes==info.frames*info.channels*4;
    if(expected_frames&&info.frames!=expected_frames)pass=0;if(rate&&info.sample_rate!=rate)pass=0;if(channels&&info.channels!=channels)pass=0;
    const float *samples=xgeAudioClipSamples(clip);for(uint64_t k=0;samples&&k<info.frames*info.channels;k+=1031)if(!isfinite(samples[k]))pass=0;
    if(!pass)++failures;printf("%s{\"input\":\"%s\",\"error\":%d,\"frames\":%llu,\"container_frames\":%llu,\"sample_rate\":%u,\"channels\":%u,\"bytes\":%llu,\"passed\":%s}",i==1?"":",",strrchr(argv[i],'/')?strrchr(argv[i],'/')+1:argv[i],error,(unsigned long long)info.frames,(unsigned long long)expected_frames,info.sample_rate,info.channels,(unsigned long long)info.bytes,pass?"true":"false");xgeAudioClipFree(clip);free(data);
    }printf("],\"failures\":%d}\n",failures);return failures?1:0;}
