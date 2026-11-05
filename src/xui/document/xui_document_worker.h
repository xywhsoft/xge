/* Private single-worker executor. No widget, Document writer or callback is
 * touched on the worker. The owner retires jobs only after releasing renderer
 * references, so expensive candidate reclamation takes place here as well. */
#include <assert.h>
typedef struct doc_input_job {
    xui_document_prepare prepare;
    uint64_t group; /* Owner-side undo unit; never read or changed by worker. */
    struct doc_input_job* next;
    int retired;
} doc_input_job;
typedef struct doc_input_worker {
    xmutex mutex;
    xcond wake;
    xthread* thread;
    doc_input_job *queued, *active, *garbage;
    int stopping;
} doc_input_worker;
static void doc_input_job_free(doc_input_job* job)
{
    xuiDocumentPrepareRelease(job->prepare); free(job);
}
static int32 doc_input_work(void* user)
{
    doc_input_worker* w = user;
    for (;;) {
        doc_input_job* job; int garbage;
        xrtMutexLock(&w->mutex);
        while (!w->garbage && !w->queued && !w->stopping) (void)xrtCondWait(&w->wake, &w->mutex);
        xrtMutexUnlock(&w->mutex);
        /* Never hold the graph mutation domain while waiting for work. Recheck
         * after entering Mutation -> mutex: a creator may have cancelled or
         * transferred the selected borrowed job in the intervening interval. */
        xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))abort();
        xrtMutexLock(&w->mutex);
        garbage = w->garbage != NULL;
        if (garbage) { job = w->garbage; w->garbage = job->next; }
        else { job = w->queued; w->queued = NULL; w->active = job; }
        if (!job && w->stopping) { xrtMutexUnlock(&w->mutex);doc_model_mutation_end(&mutation);break; }
        xrtMutexUnlock(&w->mutex);
        doc_model_mutation_end(&mutation);
        if(!job)continue;
        if (!garbage) {
            (void)xuiDocumentPrepareRun(job->prepare);
            if(!xrtOwnershipMutationBegin(&mutation))abort();
            xrtMutexLock(&w->mutex); w->active = NULL; garbage = job->retired; xrtMutexUnlock(&w->mutex);
            doc_model_mutation_end(&mutation);
        }
        if (garbage) doc_input_job_free(job);
    }
    return 0;
}
static doc_input_worker* doc_input_worker_create(void)
{
    doc_input_worker* w = calloc(1, sizeof(*w)); if (!w) return NULL;
    if (!xrtMutexInit(&w->mutex)) { free(w); return NULL; }
    if (!xrtCondInit(&w->wake)) { xrtMutexUnit(&w->mutex); free(w); return NULL; }
    w->thread = xrtThreadCreate(doc_input_work, w, 0);
    if (!w->thread) { xrtCondUnit(&w->wake); xrtMutexUnit(&w->mutex); free(w); return NULL; }
    return w;
}
static void doc_input_retire(doc_input_worker* w, doc_input_job* job)
{
    if (!job) return;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))abort();
    xrtMutexLock(&w->mutex); job->retired = 1;
    if (w->queued == job) w->queued = NULL;
    if (w->active != job) { job->next = w->garbage; w->garbage = job; }
    xrtCondSignal(&w->wake); xrtMutexUnlock(&w->mutex);
    doc_model_mutation_end(&mutation);
}
static void doc_input_schedule(doc_input_worker* w, doc_input_job* job)
{
    xui_doc_prepare_info_t info = {0}; info.iSize = sizeof(info);
    xuiDocumentPrepareGetInfo(job->prepare, &info);
    if (info.iState != XUI_DOC_PREPARE_QUEUED || info.iBufferedUtf8Bytes) return;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))abort();
    xrtMutexLock(&w->mutex);
    if (!w->stopping && w->active != job && !w->queued) w->queued = job;
    xrtCondSignal(&w->wake); xrtMutexUnlock(&w->mutex);
    doc_model_mutation_end(&mutation);
}
/* Pure frozen trace. queued/active normally borrow the UI's same job; only a
 * retired active job or the garbage list has transferred its one owner here. */
static bool doc_input_worker_trace(const doc_input_worker* worker,xrtownershipvisitor visit,void* user)
{
    if(!worker)return true;
    for(const doc_input_job* job=worker->garbage;job;job=job->next){
        xrtownershipref ref=xuiDocumentPrepareOwnership(job->prepare);if(!ref.Data || !visit(ref,user))return false;
    }
    if(worker->active && worker->active->retired){
        xrtownershipref ref=xuiDocumentPrepareOwnership(worker->active->prepare);if(!ref.Data || !visit(ref,user))return false;
    }
    return true;
}
static bool doc_input_worker_finished(const doc_input_worker* worker)
{
    if(!worker)return true;
    xthreadstate state;return xrtThreadStateTry(worker->thread,&state) && state==XTHREAD_FINISHED;
}
static int doc_input_worker_prepare(doc_input_worker* worker,bool wait,bool* complete)
{
    if(!complete)return XUI_ERROR_INVALID_ARGUMENT;
    *complete=!worker;if(!worker)return XUI_OK;
    xrtownershipscope mutation={0};if(!xrtOwnershipMutationBegin(&mutation))return XUI_ERROR_INVALID_STATE;
    xrtMutexLock(&worker->mutex);worker->stopping=1;
    if(worker->queued)xuiDocumentPrepareCancel(worker->queued->prepare);
    if(worker->active)xuiDocumentPrepareCancel(worker->active->prepare);
    xrtCondSignal(&worker->wake);xrtMutexUnlock(&worker->mutex);doc_model_mutation_end(&mutation);
    if(!wait && !doc_input_worker_finished(worker))return XUI_STATUS_DEFERRED;
    /* Only the explicit legacy void destruction mode may wait. No graph
     * Mutation/Freeze or worker mutex is held while the native proc finishes. */
    xwaitresult waited=xrtThreadWaitFor(worker->thread,wait ? -1:0);
    if(waited==XWAIT_TIMEOUT)return XUI_STATUS_DEFERRED;
    if(waited!=XWAIT_OK)return XUI_ERROR_BACKEND_FAILED;
    *complete=true;return XUI_OK;
}
static void doc_input_worker_destroy(doc_input_worker* w)
{
    if (!w) return;
    /* Type Prepare already observed the real proc/XRT-context completion.
     * No UI boundary waits for an active parse or unloads from its worker tail. */
    assert(doc_input_worker_finished(w));
    (void)xrtThreadWaitFor(w->thread,0);xrtThreadDestroy(w->thread);
    /* Subtype destruction can transfer its final UI jobs after requestStop
     * has already allowed the worker to return. They still have one exact
     * owner and must be reclaimed here rather than enqueued to a dead proc. */
    while(w->garbage){doc_input_job* job=w->garbage;w->garbage=job->next;doc_input_job_free(job);}
    xrtCondUnit(&w->wake); xrtMutexUnit(&w->mutex); free(w);
}
