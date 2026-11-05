/* One typed whole-object snapshot. A single backend publication sees either
 * every requested representation or none. Lazy providers are run twice on
 * the same pinned generation; publication/replacement during either pass is
 * rejected. Plain bytes borrow their one pinned generation, without copying.
 * Only Owned factories enter this safe path; raw providers stay C interop. */
typedef int (*DS_FN(Consumer))(const DS_ITEM*,int,void*);
static int DS_FN(Copy)(DS_HANDLE object,DS_FN(Consumer) consume,void* user)
{
    if(!consume || !DS_IS_OWNED(object))return DS_STATUS(ERROR_INVALID_ARGUMENT);
    DS_OPERATION call={0};int status=DS_PREFIX(OperationBegin)(object,&call);
    if(status!=DS_STATUS(OK))return status;
    DS_GENERATION* generations[64]={0};DS_ITEM items[64]={0};void* copies[64]={0};
    int count=object->iCount,held=0;uint64_t revision=object->iRevision;
    if(count<=0 || count>64)status=DS_STATUS(ERROR_INVALID_ARGUMENT);
    for(int i=0;status==DS_STATUS(OK) && i<count;++i){
        DS_GENERATION* generation=object->pSlots[i];
        if(!DS_PREFIX(GenerationHold)(generation)){status=DS_STATUS(ERROR_INVALID_STATE);break;}
        generations[held++]=generation;items[i].sFormat=generation->sFormat;
    }
    for(int i=0;status==DS_STATUS(OK) && i<count;++i){
        DS_GENERATION* generation=generations[i];
        if(!(generation->iFlags&DS_PROVIDER) && generation->tRead.raw==NULL){
            items[i].pData=generation->pValue;items[i].iDataSize=generation->tSize.valueBytes;
        }else{
            size_t required=0,written=0;
            status=DS_PREFIX(GenerationRead)(generation,NULL,0,&required);
            if(status==DS_STATUS(OK) && object->iRevision!=revision)status=DS_STATUS(ERROR_INVALID_STATE);
            if(status!=DS_STATUS(OK))break;
            copies[i]=xrtMalloc(required ? required:1u);
            if(!copies[i]){status=DS_STATUS(ERROR_OUT_OF_MEMORY);break;}
            status=DS_PREFIX(GenerationRead)(generation,copies[i],required,&written);
            if(status==DS_STATUS(OK) && (written!=required || object->iRevision!=revision))status=DS_STATUS(ERROR_INVALID_STATE);
            if(status!=DS_STATUS(OK))break;
            items[i].pData=copies[i];items[i].iDataSize=written;
        }
    }
    if(status==DS_STATUS(OK))status=DS_PREFIX(OwnedAdmit)(DS_PREFIX(ObjectRoot)(object));
    if(status==DS_STATUS(OK) && object->iRevision!=revision)status=DS_STATUS(ERROR_INVALID_STATE);
    if(status==DS_STATUS(OK))status=consume(items,count,user);
    xge_data_outcome_t result={0};DS_PREFIX(OwnedOutcomeBegin)(&result,status);
    for(int i=held;i>0;--i){
        xrtFree(copies[i-1]);DS_PREFIX(OwnedOutcomeAdd)(&result,DS_PREFIX(GenerationSnapshotEnd)(generations[i-1]));
    }
    status=result.iStatus;xrtSetErrorTake(result.pError);
    return DS_PREFIX(OperationEnd)(&call,status);
}
