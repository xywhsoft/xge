/* Private single-worker executor. No widget, Document writer or callback is
 * touched on the worker. The owner retires jobs only after releasing renderer
 * references, so expensive candidate reclamation takes place here as well. */
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
        garbage = w->garbage != NULL;
        if (garbage) { job = w->garbage; w->garbage = job->next; }
        else { job = w->queued; w->queued = NULL; w->active = job; }
        if (!job && w->stopping) { xrtMutexUnlock(&w->mutex); break; }
        xrtMutexUnlock(&w->mutex);
        if (!garbage) {
            (void)xuiDocumentPrepareRun(job->prepare);
            xrtMutexLock(&w->mutex); w->active = NULL; garbage = job->retired; xrtMutexUnlock(&w->mutex);
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
    xrtMutexLock(&w->mutex); job->retired = 1;
    if (w->queued == job) w->queued = NULL;
    if (w->active != job) { job->next = w->garbage; w->garbage = job; }
    xrtCondSignal(&w->wake); xrtMutexUnlock(&w->mutex);
}
static void doc_input_schedule(doc_input_worker* w, doc_input_job* job)
{
    xui_doc_prepare_info_t info = {0}; info.iSize = sizeof(info);
    xuiDocumentPrepareGetInfo(job->prepare, &info);
    if (info.iState != XUI_DOC_PREPARE_QUEUED || info.iBufferedUtf8Bytes) return;
    xrtMutexLock(&w->mutex);
    if (w->active != job && !w->queued) w->queued = job;
    xrtCondSignal(&w->wake); xrtMutexUnlock(&w->mutex);
}
static void doc_input_worker_destroy(doc_input_worker* w)
{
    if (!w) return;
    xrtMutexLock(&w->mutex); w->stopping = 1; xrtCondSignal(&w->wake); xrtMutexUnlock(&w->mutex);
    /* Teardown joins before widget/context code can be unloaded. Normal input,
     * cancellation, replacement and publication never wait for the worker. */
    (void)xrtThreadWait(w->thread); xrtThreadDestroy(w->thread);
    xrtCondUnit(&w->wake); xrtMutexUnit(&w->mutex); free(w);
}
