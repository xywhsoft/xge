#include "xui.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct tree_work_t {
	unsigned long long iIdLookups;
	unsigned long long iIdInserts;
	unsigned long long iHierarchySteps;
	unsigned long long iVisibleWrites;
	unsigned long long iVisibleMoves;
	unsigned long long iRebuilds;
} tree_work_t;

static tree_work_t g_tWork;
static int g_iFailAllocation = -1;

static int treeFailAllocation(void)
{
	if ( g_iFailAllocation < 0 ) return 0;
	return g_iFailAllocation-- == 0;
}

static void* treeMalloc(size_t iSize)
{
	return treeFailAllocation() ? NULL : xrtMalloc(iSize);
}

static void* treeRealloc(void* pOld, size_t iSize)
{
	return treeFailAllocation() ? NULL : xrtRealloc(pOld, iSize);
}

static void* treeMapGetOrAdd(xmap* pMap, xbytesview tKey, bool* pNew)
{
	return treeFailAllocation() ? NULL : xrtMapGetOrAdd(pMap, tKey, pNew);
}

/* Compile the production data path with local counters, without public test
 * exports or changes to shared widget/internal structures. Unused UI functions
 * are discarded by the whole-program compiler. Hash counts are xrtMap operations,
 * not bucket probes; xrtMap provides the expected constant-time ID lookup. */
#define XUI_TREE_VIEW_TEST_COUNT(field, count) (g_tWork.field += (unsigned long long)(count))
#define xrtMalloc treeMalloc
#define xrtRealloc treeRealloc
#define xrtMapGetOrAdd treeMapGetOrAdd
#include "../src/xui_tree_view.c"
#undef xrtMalloc
#undef xrtRealloc
#undef xrtMapGetOrAdd

#define CHECK(expr) do { if ( !(expr) ) { \
	printf("tree scale failure at line %d: %s\n", __LINE__, #expr); \
	iFailed = 1; goto cleanup; } } while ( 0 )

static int treeId(int iIndex)
{
	return (iIndex == 0) ? 0 : INT_MAX - iIndex * 7919;
}

static xui_tree_view_node_t treeNode(int iId, int iParent, int bExpanded)
{
	xui_tree_view_node_t tNode;
	memset(&tNode, 0, sizeof(tNode));
	tNode.iId = iId;
	tNode.iParent = iParent;
	tNode.sText = "scale node";
	tNode.bEnabled = 1;
	tNode.bExpanded = bExpanded;
	return tNode;
}

static void treePrintWork(const char* sCase, int iCount)
{
	printf("%s N=%d lookup=%llu insert=%llu hierarchy=%llu writes=%llu moves=%llu rebuild=%llu\n",
		sCase, iCount, g_tWork.iIdLookups, g_tWork.iIdInserts,
		g_tWork.iHierarchySteps, g_tWork.iVisibleWrites, g_tWork.iVisibleMoves, g_tWork.iRebuilds);
}

static int treeScale(int iCount, int bChain, int bIncremental)
{
	xui_tree_view_data_t tData;
	xui_tree_view_node_t tNode;
	int i;
	int iFailed = 0;
	__xuiTreeViewDefaults(&tData);
	memset(&g_tWork, 0, sizeof(g_tWork));
	for ( i = 0; i < iCount; i++ ) {
		tNode = treeNode(treeId(i), (i == 0) ? -1 : treeId(bChain ? i - 1 : 0), 1);
		CHECK((bIncremental ? __xuiTreeViewAddNodeData(NULL, &tData, &tNode) : __xuiTreeViewAppendNodeData(&tData, &tNode)) == XUI_OK);
	}
	if ( !bIncremental ) CHECK(__xuiTreeViewRebuildVisible(NULL, &tData, -1) == XUI_OK);
	CHECK(tData.iNodeCount == iCount && tData.iVisibleCount == iCount);
	CHECK(g_tWork.iIdLookups == (unsigned long long)(bIncremental ? 3 : 2) * (iCount - 1));
	CHECK(g_tWork.iIdInserts == (unsigned long long)iCount);
	CHECK(g_tWork.iHierarchySteps == (unsigned long long)(bIncremental ? 1 : 3) * iCount);
	CHECK(g_tWork.iVisibleWrites == (unsigned long long)(bIncremental ? 1 : 2) * iCount);
	CHECK(g_tWork.iVisibleMoves == 0 && g_tWork.iRebuilds == (unsigned long long)!bIncremental);
	treePrintWork(bChain ? (bIncremental ? "add-chain" : "build-chain") : (bIncremental ? "add-wide" : "build-wide"), iCount);
	memset(&g_tWork, 0, sizeof(g_tWork));
	for ( i = 0; i < iCount; i++ ) {
		CHECK(__xuiTreeViewFindNodeData(&tData, treeId(i)) == i);
		CHECK(__xuiTreeViewVisibleIndexData(&tData, treeId(i)) == i);
		CHECK(tData.arrNodes[i].iDepth == (bChain ? i : (i != 0)));
		CHECK(tData.arrNodes[i].bHasChildren == (bChain ? i + 1 < iCount : i == 0));
	}
	CHECK(g_tWork.iIdLookups == (unsigned long long)2 * iCount);
	CHECK(g_tWork.iHierarchySteps == 0 && g_tWork.iVisibleWrites == 0);
	treePrintWork("lookup-all", iCount);
	tData.iSelectedId = treeId(iCount - 1);
	memset(&g_tWork, 0, sizeof(g_tWork));
	CHECK(__xuiTreeViewSetExpandedData(NULL, &tData, 0, 0) == XUI_OK);
	CHECK(tData.iVisibleCount == 1 && tData.iSelectedId == 0 && tData.iFocusVisible == 0);
	CHECK(__xuiTreeViewSetExpandedData(NULL, &tData, 0, 1) == XUI_OK);
	CHECK(tData.iVisibleCount == iCount);
	CHECK(g_tWork.iHierarchySteps == (unsigned long long)5 * (iCount - 1));
	CHECK(g_tWork.iVisibleWrites == (unsigned long long)2 * (iCount - 1));
	CHECK(g_tWork.iVisibleMoves == 0 && g_tWork.iRebuilds == 0);
	treePrintWork("collapse-expand-root", iCount);
cleanup:
	__xuiTreeViewClearNodeStorage(&tData);
	return !iFailed;
}

static int treeLocality(int iCount)
{
	xui_tree_view_data_t tData;
	xui_tree_view_node_t tNode;
	int i;
	int iFailed = 0;
	__xuiTreeViewDefaults(&tData);
	for ( i = 0; i < iCount; i++ ) {
		tNode = treeNode(treeId(i), (i == 0) ? -1 : 0, 0);
		CHECK(__xuiTreeViewAppendNodeData(&tData, &tNode) == XUI_OK);
	}
	tNode = treeNode(INT_MAX, -1, 0);
	CHECK(__xuiTreeViewAppendNodeData(&tData, &tNode) == XUI_OK);
	tNode = treeNode(INT_MAX - 1, INT_MAX, 0);
	CHECK(__xuiTreeViewAppendNodeData(&tData, &tNode) == XUI_OK);
	CHECK(__xuiTreeViewRebuildVisible(NULL, &tData, -1) == XUI_OK);
	memset(&g_tWork, 0, sizeof(g_tWork));
	for ( i = 0; i < 256; i++ ) {
		CHECK(__xuiTreeViewSetExpandedData(NULL, &tData, iCount, 1) == XUI_OK);
		CHECK(__xuiTreeViewSetExpandedData(NULL, &tData, iCount, 0) == XUI_OK);
		CHECK(__xuiTreeViewSetExpandedData(NULL, &tData, 1, i & 1) == XUI_OK);
	}
	CHECK(tData.iVisibleCount == 2);
	CHECK(g_tWork.iHierarchySteps == 5 * 256 && g_tWork.iVisibleWrites == 2 * 256);
	CHECK(g_tWork.iVisibleMoves == 0 && g_tWork.iRebuilds == 0);
	treePrintWork("toggle-small-subtree-256x", iCount);
	CHECK(__xuiTreeViewSetExpandedData(NULL, &tData, 0, 1) == XUI_OK);
	tData.iActiveVisible = iCount;
	memset(&g_tWork, 0, sizeof(g_tWork));
	CHECK(__xuiTreeViewSetExpandedData(NULL, &tData, 0, 0) == XUI_OK);
	CHECK(tData.iActiveVisible == 1 && tData.arrVisible[1] == iCount);
	CHECK(g_tWork.iVisibleMoves == 1 && g_tWork.iVisibleWrites == (unsigned long long)iCount);
	CHECK(__xuiTreeViewSetExpandedData(NULL, &tData, 0, 1) == XUI_OK);
	CHECK(tData.iActiveVisible == iCount);
	tData.iActiveVisible = 1;
	CHECK(__xuiTreeViewSetExpandedData(NULL, &tData, 0, 0) == XUI_OK && tData.iActiveVisible == -1);
	/* Opening a deep, initially collapsed chain one level at a time must not
	 * revisit already visible ancestors. */
	__xuiTreeViewClearNodeStorage(&tData);
	__xuiTreeViewDefaults(&tData);
	for ( i = 0; i < iCount; i++ ) {
		tNode = treeNode(treeId(i), (i == 0) ? -1 : treeId(i - 1), 0);
		CHECK(__xuiTreeViewAppendNodeData(&tData, &tNode) == XUI_OK);
	}
	CHECK(__xuiTreeViewRebuildVisible(NULL, &tData, -1) == XUI_OK);
	memset(&g_tWork, 0, sizeof(g_tWork));
	for ( i = 0; i + 1 < iCount; i++ ) CHECK(__xuiTreeViewSetExpandedData(NULL, &tData, i, 1) == XUI_OK);
	CHECK(tData.iVisibleCount == iCount && tData.arrNodes[iCount - 1].iDepth == iCount - 1);
	CHECK(g_tWork.iHierarchySteps == (unsigned long long)4 * (iCount - 1));
	CHECK(g_tWork.iVisibleWrites == (unsigned long long)iCount - 1);
	CHECK(g_tWork.iVisibleMoves == 0 && g_tWork.iRebuilds == 0);
	treePrintWork("expand-chain-level-by-level", iCount);
	__xuiTreeViewClearNodeStorage(&tData);
	__xuiTreeViewDefaults(&tData);
	tNode = treeNode(INT_MAX, -1, 0);
	CHECK(__xuiTreeViewAppendNodeData(&tData, &tNode) == XUI_OK);
	tNode = treeNode(INT_MAX - 1, INT_MAX, 0);
	CHECK(__xuiTreeViewAppendNodeData(&tData, &tNode) == XUI_OK);
	for ( i = 0; i < iCount; i++ ) {
		tNode = treeNode(treeId(i), -1, 0);
		CHECK(__xuiTreeViewAppendNodeData(&tData, &tNode) == XUI_OK);
	}
	CHECK(__xuiTreeViewRebuildVisible(NULL, &tData, -1) == XUI_OK);
	memset(&g_tWork, 0, sizeof(g_tWork));
	for ( i = 0; i < 8; i++ ) {
		CHECK(__xuiTreeViewSetExpandedData(NULL, &tData, 0, 1) == XUI_OK);
		CHECK(__xuiTreeViewSetExpandedData(NULL, &tData, 0, 0) == XUI_OK);
	}
	CHECK(g_tWork.iHierarchySteps == 6 * 8 && g_tWork.iRebuilds == 0);
	CHECK(g_tWork.iVisibleMoves == (unsigned long long)16 * iCount);
	CHECK(g_tWork.iVisibleWrites == (unsigned long long)16 * (iCount + 1));
	CHECK(__xuiTreeViewVisibleIndexData(&tData, treeId(iCount - 1)) == iCount);
	treePrintWork("toggle-before-visible-suffix-8x", iCount);
cleanup:
	__xuiTreeViewClearNodeStorage(&tData);
	return !iFailed;
}

/* Independent, intentionally scan-based oracle, bounded to small random trees.
 * Its fixed stack preserves the old preorder semantics without recursion. */
static int treeCheckReference(const xui_tree_view_data_t* pData)
{
	int arrStack[256];
	int arrDepth[256];
	int arrExpected[256];
	int iStack = 0;
	int iVisible = 0;
	int i;
	int j;
	int iNode;
	int iDepth;
	for ( i = 0; i < pData->iNodeCount; i++ ) arrExpected[i] = -1;
	for ( i = pData->iNodeCount - 1; i >= 0; i-- ) {
		if ( pData->arrNodes[i].iParent < 0 ) { arrStack[iStack] = i; arrDepth[iStack++] = 0; }
	}
	while ( iStack > 0 ) {
		iNode = arrStack[--iStack];
		iDepth = arrDepth[iStack];
		if ( iVisible >= pData->iVisibleCount || pData->arrVisible[iVisible] != iNode || pData->arrNodes[iNode].iDepth != iDepth ) return 0;
		arrExpected[iNode] = iVisible++;
		if ( pData->arrNodes[iNode].bExpanded ) {
			for ( i = pData->iNodeCount - 1; i >= 0; i-- ) {
				if ( pData->arrNodes[i].iParent == pData->arrNodes[iNode].iId ) {
					arrStack[iStack] = i; arrDepth[iStack++] = iDepth + 1;
				}
			}
		}
	}
	if ( iVisible != pData->iVisibleCount ) return 0;
	for ( i = 0; i < pData->iNodeCount; i++ ) {
		int bChildren = 0;
		for ( j = 0; j < pData->iNodeCount; j++ ) bChildren |= pData->arrNodes[j].iParent == pData->arrNodes[i].iId;
		if ( pData->arrNodes[i].bHasChildren != bChildren || pData->arrIndex[i].iVisible != arrExpected[i] ||
		     __xuiTreeViewFindNodeData(pData, pData->arrNodes[i].iId) != i ||
		     __xuiTreeViewVisibleIndexData(pData, pData->arrNodes[i].iId) != arrExpected[i] ) return 0;
	}
	return 1;
}

static int treeRandomAndFailures(void)
{
	xui_tree_view_data_t tData;
	xui_tree_view_node_t tNode;
	unsigned int iRandom = 0x5eeda11u;
	int iFailed = 0;
	int i;
	int iNode;
	int iFail;
	__xuiTreeViewDefaults(&tData);
	for ( i = 0; i < 4096; i++ ) {
		iRandom = iRandom * 1664525u + 1013904223u;
		if ( (i % 512) == 0 ) {
			__xuiTreeViewClearNodeStorage(&tData);
			__xuiTreeViewDefaults(&tData);
		}
		if ( tData.iNodeCount == 0 || ((iRandom & 3u) == 0 && tData.iNodeCount < 256) ) {
			iNode = (tData.iNodeCount > 0 && iRandom % 7u != 0) ? (int)((iRandom >> 8) % (unsigned int)tData.iNodeCount) : -1;
			tNode = treeNode(treeId(tData.iNodeCount), (iNode < 0) ? -1 : tData.arrNodes[iNode].iId, (iRandom >> 20) & 1);
			CHECK(__xuiTreeViewAddNodeData(NULL, &tData, &tNode) == XUI_OK);
		} else {
			iNode = (int)((iRandom >> 8) % (unsigned int)tData.iNodeCount);
			CHECK(__xuiTreeViewSetExpandedData(NULL, &tData, iNode, (iRandom >> 20) & 1) == XUI_OK);
		}
		CHECK(treeCheckReference(&tData));
	}
	for ( iFail = 0; iFail < 5; iFail++ ) {
		__xuiTreeViewClearNodeStorage(&tData);
		__xuiTreeViewDefaults(&tData);
		for ( i = 0; i < 16; i++ ) {
			tNode = treeNode(i, (i == 0) ? -1 : 0, 1);
			CHECK(__xuiTreeViewAddNodeData(NULL, &tData, &tNode) == XUI_OK);
		}
		tNode = treeNode(16, 0, 1);
		g_iFailAllocation = iFail;
		CHECK(__xuiTreeViewAddNodeData(NULL, &tData, &tNode) == XUI_ERROR_OUT_OF_MEMORY);
		g_iFailAllocation = -1;
		CHECK(tData.iNodeCount == 16 && tData.iVisibleCount == 16 && __xuiTreeViewFindNodeData(&tData, 16) == -1);
		CHECK(treeCheckReference(&tData));
		CHECK(__xuiTreeViewAddNodeData(NULL, &tData, &tNode) == XUI_OK && treeCheckReference(&tData));
	}
	printf("random-oracle: 4096 mutations passed; allocation-failure/retry: 5 sites passed\n");
cleanup:
	g_iFailAllocation = -1;
	__xuiTreeViewClearNodeStorage(&tData);
	return !iFailed;
}

int main(void)
{
	int arrSizes[] = {4096, 16384, 65536};
	int i;
	if ( !treeRandomAndFailures() ) return 1;
	for ( i = 0; i < 3; i++ ) {
		if ( !treeScale(arrSizes[i], 0, 0) || !treeScale(arrSizes[i], 1, 0) ||
		     !treeScale(arrSizes[i], 0, 1) || !treeScale(arrSizes[i], 1, 1) || !treeLocality(arrSizes[i]) ) return 1;
	}
	printf("xui_tree_view_scale_test passed (operation bounds, no timing thresholds)\n");
	return 0;
}
