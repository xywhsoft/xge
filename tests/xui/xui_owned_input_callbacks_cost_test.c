/* Native client cost comparison, same SDK, exact raw/owned registration work. */
#define main owned_input_functional_main
#include "xui_owned_input_callbacks_test.c"
#undef main
static void report(const char* name,xmemdebugsnapshot* before)
{
 xmemdebugsnapshot after;xrtMemDebugSnapshot(&after);
 unsigned long long allocations=after.AllocCount-before->AllocCount,frees=after.FreeCount-before->FreeCount,reallocations=after.ReallocCount-before->ReallocCount;
 printf("InputOwners %s alloc=%llu free=%llu realloc=%llu\n",name,allocations,frees,reallocations);
 if(!strcmp(name,"owned-hotkey")||!strcmp(name,"owned-error"))assert(allocations==1000&&frees==1000&&!reallocations);
 else assert(!allocations&&!frees&&!reallocations);
}
int main(void)
{
#if defined(XGE_TEST_STATIC)
 const xge_host_xrt_contract_t contract=XGE_STATIC_XRT_LOCAL_CONTRACT_INIT;assert(!strcmp(contract.headerSha256,xgeStaticXrtContractV1.headerSha256));
#else
 const xge_host_xrt_contract_t contract=XGE_HOST_XRT_LOCAL_CONTRACT_INIT;assert(xgeHostXrtBind(&contract,resolve,NULL,NULL)==XGE_HOST_XRT_OK);
#endif
 assert(xrtMemDebugEnable(true));Fixture f;create(&f);Owner owner=makeOwner(&f,f.a);xmemdebugsnapshot before;
 registerHot(&owner);xrtMemDebugSnapshot(&before);
 for(int i=0;i<1000;++i)registerHot(&owner);
 report("owned-hotkey",&before);
 assert(xuiHotKeyRegister(f.context,f.a,'A',0,NULL,NULL)==XUI_OK);xrtMemDebugSnapshot(&before);
 for(int i=0;i<1000;++i)assert(xuiHotKeyRegister(f.context,f.a,'A',0,NULL,NULL)==XUI_OK);
 report("raw-hotkey",&before);
 registerError(&owner);xrtMemDebugSnapshot(&before);
 for(int i=0;i<1000;++i)registerError(&owner);
 report("owned-error",&before);
 assert(xuiSetErrorCallback(f.context,NULL,NULL)==XUI_OK);xrtMemDebugSnapshot(&before);
 for(int i=0;i<1000;++i)assert(xuiSetErrorCallback(f.context,NULL,NULL)==XUI_OK);
 report("raw-error",&before);
 printf("InputOwners layout context=%zu hotkey=%zu record=%zu\n",sizeof(xui_context_t),sizeof(xui_hotkey_t),sizeof(xui_callback_record_t));
 closeFixture(&f);assert(drop(&owner));xmemdebugsnapshot end;xrtMemDebugSnapshot(&end);assert(!end.LiveCount&&!end.InvalidFreeCount&&!end.DoubleFreeCount);return 0;
}
