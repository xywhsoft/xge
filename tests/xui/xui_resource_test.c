#include "xui.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_resource_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_resource_destroy_state_t {
	int iCount;
	void* pLastHandle;
} xui_resource_destroy_state_t;

static void __xuiResourceTestDestroy(xui_context pContext, void* pHandle,
	void* pUser)
{
	xui_resource_destroy_state_t* pState;

	(void)pContext;
	pState = (xui_resource_destroy_state_t*)pUser;
	if ( pState != NULL ) {
		pState->iCount++;
		pState->pLastHandle = pHandle;
	}
}

int main(void)
{
	xui_resource_destroy_state_t tDestroyA;
	xui_resource_destroy_state_t tDestroyB;
	xui_resource_desc_t tDescA;
	xui_resource_desc_t tDescB;
	xui_resource_desc_t tDesc;
	xui_context pContext;
	xui_resource pBase;
	xui_resource pMiddle;
	xui_resource pLeaf;
	xui_resource pIsolated;
	xui_resource pOutput;
	uint64_t iRegistryGeneration;
	uint32_t iBaseGeneration;
	uint32_t iMiddleGeneration;
	uint32_t iLeafGeneration;
	int iHandleA;
	int iHandleB;
	int iFailed;
	int iRet;

	memset(&tDestroyA, 0, sizeof(tDestroyA));
	memset(&tDestroyB, 0, sizeof(tDestroyB));
	memset(&tDescA, 0, sizeof(tDescA));
	memset(&tDescB, 0, sizeof(tDescB));
	pContext = NULL;
	pBase = NULL;
	pMiddle = NULL;
	pLeaf = NULL;
	pIsolated = NULL;
	pOutput = NULL;
	iHandleA = 1;
	iHandleB = 2;
	iFailed = 0;

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	pOutput = (xui_resource)&tDescA;
	iRet = xuiResourceSet(pContext, &pOutput, &tDescA);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pOutput == NULL,
		"invalid set clears output");

	tDescA.iSize = sizeof(tDescA);
	tDescA.sName = "resource.base";
	tDescA.iKind = XUI_RESOURCE_USER;
	tDescA.pHandle = &iHandleA;
	tDescA.pUser = &tDestroyA;
	tDescA.onDestroy = __xuiResourceTestDestroy;
	XUI_TEST_CHECK(xuiResourceSet(pContext, &pBase, &tDescA) == XUI_OK &&
		pBase != NULL, "base set");
	iRegistryGeneration = xuiResourceGetRegistryGeneration(pContext);
	iBaseGeneration = xuiResourceGetGeneration(pBase);
	pOutput = NULL;
	XUI_TEST_CHECK(xuiResourceSet(pContext, &pOutput, &tDescA) == XUI_OK &&
		pOutput == pBase && tDestroyA.iCount == 0 &&
		xuiResourceGetRegistryGeneration(pContext) == iRegistryGeneration &&
		xuiResourceGetGeneration(pBase) == iBaseGeneration,
		"identical set is idempotent");

	tDescB = tDescA;
	tDescB.pHandle = &iHandleB;
	tDescB.pUser = &tDestroyB;
	XUI_TEST_CHECK(xuiResourceAddRef(pBase) == XUI_OK &&
		xuiResourceGetRefCount(pBase) == 2, "base add ref");
	pOutput = (xui_resource)&tDescB;
	XUI_TEST_CHECK(xuiResourceSet(pContext, &pOutput, &tDescB) ==
		XUI_ERROR_INVALID_STATE && pOutput == NULL &&
		xuiResourceFind(pContext, tDescA.sName) == pBase &&
		xuiResourceGetRegistryGeneration(pContext) == iRegistryGeneration &&
		tDestroyA.iCount == 0 && tDestroyB.iCount == 0,
		"retained resource replacement is rejected");
	XUI_TEST_CHECK(xuiResourceRemove(pBase) == XUI_ERROR_INVALID_STATE &&
		xuiResourceFind(pContext, tDescA.sName) == pBase,
		"retained resource removal is rejected");
	XUI_TEST_CHECK(xuiResourceRelease(pBase) == XUI_OK &&
		xuiResourceGetRefCount(pBase) == 1, "base release ref");

	tDesc = tDescA;
	tDesc.iKind = XUI_RESOURCE_STYLE;
	tDesc.pUser = &tDestroyB;
	pOutput = (xui_resource)&tDesc;
	XUI_TEST_CHECK(xuiResourceSet(pContext, &pOutput, &tDesc) ==
		XUI_ERROR_INVALID_STATE && pOutput == NULL &&
		tDestroyA.iCount == 0,
		"owned handle reuse is rejected");

	iRegistryGeneration = xuiResourceGetRegistryGeneration(pContext);
	XUI_TEST_CHECK(xuiResourceSet(pContext, &pOutput, &tDescB) == XUI_OK &&
		pOutput != NULL && pOutput != pBase &&
		xuiResourceFind(pContext, tDescB.sName) == pOutput &&
		xuiResourceGetRegistryGeneration(pContext) > iRegistryGeneration &&
		tDestroyA.iCount == 1 && tDestroyA.pLastHandle == &iHandleA,
		"unretained resource replacement commits");
	pBase = pOutput;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iKind = XUI_RESOURCE_USER;
	tDesc.sName = "resource.middle";
	XUI_TEST_CHECK(xuiResourceSet(pContext, &pMiddle, &tDesc) == XUI_OK,
		"middle set");
	tDesc.sName = "resource.leaf";
	XUI_TEST_CHECK(xuiResourceSet(pContext, &pLeaf, &tDesc) == XUI_OK,
		"leaf set");
	tDesc.sName = "resource.isolated";
	XUI_TEST_CHECK(xuiResourceSet(pContext, &pIsolated, &tDesc) == XUI_OK,
		"isolated set");
	XUI_TEST_CHECK(xuiResourceAddDependency(pMiddle, pBase) == XUI_OK &&
		xuiResourceAddDependency(pLeaf, pMiddle) == XUI_OK,
		"dependency chain add");
	iMiddleGeneration = xuiResourceGetGeneration(pMiddle);
	iLeafGeneration = xuiResourceGetGeneration(pLeaf);
	XUI_TEST_CHECK(xuiResourceTouch(pBase) == XUI_OK &&
		xuiResourceGetGeneration(pMiddle) != iMiddleGeneration &&
		xuiResourceGetGeneration(pLeaf) != iLeafGeneration,
		"dependency invalidation is transitive");

	XUI_TEST_CHECK(xuiResourceAddDependency(pBase, pLeaf) == XUI_OK,
		"cyclic dependency add");
	iRegistryGeneration = xuiResourceGetRegistryGeneration(pContext);
	iBaseGeneration = xuiResourceGetGeneration(pBase);
	XUI_TEST_CHECK(xuiResourceAddDependency(pBase, pLeaf) == XUI_OK &&
		xuiResourceGetRegistryGeneration(pContext) == iRegistryGeneration &&
		xuiResourceGetGeneration(pBase) == iBaseGeneration,
		"duplicate dependency is idempotent");
	iRegistryGeneration = xuiResourceGetRegistryGeneration(pContext);
	xuiResourceClearDependencies(pIsolated);
	XUI_TEST_CHECK(xuiResourceGetRegistryGeneration(pContext) ==
		iRegistryGeneration, "clearing empty dependencies is idempotent");

	iMiddleGeneration = xuiResourceGetGeneration(pMiddle);
	iLeafGeneration = xuiResourceGetGeneration(pLeaf);
	XUI_TEST_CHECK(xuiResourceRemove(pBase) == XUI_OK &&
		xuiResourceFind(pContext, tDescB.sName) == NULL &&
		xuiResourceGetDependencyCount(pMiddle) == 0 &&
		xuiResourceGetGeneration(pMiddle) != iMiddleGeneration &&
		xuiResourceGetGeneration(pLeaf) != iLeafGeneration &&
		tDestroyB.iCount == 1 && tDestroyB.pLastHandle == &iHandleB,
		"removal invalidation is transitive");
	pBase = NULL;
	XUI_TEST_CHECK(xuiResourceRemove(pLeaf) == XUI_OK, "leaf remove");
	pLeaf = NULL;
	XUI_TEST_CHECK(xuiResourceRemove(pMiddle) == XUI_OK, "middle remove");
	pMiddle = NULL;
	XUI_TEST_CHECK(xuiResourceRemove(pIsolated) == XUI_OK,
		"isolated remove");
	pIsolated = NULL;

cleanup:
	if ( pContext != NULL ) xuiDestroy(pContext);
	if ( iFailed ) return 1;
	printf("xui_resource_test passed\n");
	return 0;
}
