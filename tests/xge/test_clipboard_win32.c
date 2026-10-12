#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>

#define XGE_CLIPBOARD_TEST_SIZE ((2u * 1024u * 1024u) + 17u)

/* A clipboard listener may request the test's delayed PNG format while it
 * holds the clipboard open. Dispatch that request during retry waits. */
static void __testPumpMessages(void)
{
	MSG message;
	while ( PeekMessageA(&message, NULL, 0u, 0u, PM_REMOVE) ) {
		TranslateMessage(&message);
		DispatchMessageA(&message);
	}
}

static int __testOpenClipboard(HWND owner)
{
	int attempt;
	for ( attempt = 0; attempt < 50; attempt++ ) {
		if ( OpenClipboard(owner) ) return 1;
		if ( attempt + 1 < 50 ) { __testPumpMessages(); Sleep(2); }
	}
	return 0;
}

static int __testGetClipboardDataWithRetry(const char* format, void* data, size_t capacity)
{
	int attempt, result = XGE_ERROR_BACKEND_FAILED;
	for ( attempt = 0; attempt < 8; attempt++ ) {
		result = xgeClipboardGetData(format, data, capacity);
		if ( result != XGE_ERROR_BACKEND_FAILED ) return result;
		__testPumpMessages();
		if ( attempt + 1 < 8 ) Sleep(2);
	}
	return result;
}

static int __testSetClipboardWideFromAscii(HWND owner, const char* sText, size_t iLength)
{
	HGLOBAL hMemory;
	WCHAR* sWide;
	size_t i;

	hMemory = GlobalAlloc(GMEM_MOVEABLE, (iLength + 1u) * sizeof(WCHAR));
	if ( hMemory == NULL ) return 0;
	sWide = (WCHAR*)GlobalLock(hMemory);
	if ( sWide == NULL ) {
		GlobalFree(hMemory);
		return 0;
	}
	for ( i = 0; i < iLength; i++ ) sWide[i] = (WCHAR)(unsigned char)sText[i];
	sWide[iLength] = L'\0';
	GlobalUnlock(hMemory);
	if ( !__testOpenClipboard(owner) ) {
		GlobalFree(hMemory);
		return 0;
	}
	if ( !EmptyClipboard() || SetClipboardData(CF_UNICODETEXT, hMemory) == NULL ) {
		CloseClipboard();
		GlobalFree(hMemory);
		return 0;
	}
	CloseClipboard();
	return 1;
}

static int __testSystemClipboardMatchesAscii(HWND owner, const char* sText, size_t iLength)
{
	HANDLE hData;
	const WCHAR* sWide;
	size_t i;
	int bMatch;

	if ( !__testOpenClipboard(owner) ) return 0;
	hData = GetClipboardData(CF_UNICODETEXT);
	if ( hData == NULL ) {
		CloseClipboard();
		return 0;
	}
	sWide = (const WCHAR*)GlobalLock(hData);
	if ( sWide == NULL ) {
		CloseClipboard();
		return 0;
	}
	bMatch = 1;
	for ( i = 0; i < iLength; i++ ) {
		if ( sWide[i] != (WCHAR)(unsigned char)sText[i] ) {
			bMatch = 0;
			break;
		}
	}
	if ( bMatch && sWide[iLength] != L'\0' ) bMatch = 0;
	GlobalUnlock(hData);
	CloseClipboard();
	return bMatch;
}

static int __testSetClipboardDibV5(HWND owner, int corrupt)
{
	static const unsigned char pixels[8] = {
		64u, 128u, 255u, 64u, 0u, 255u, 0u, 255u
	};
	HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPV5HEADER) + sizeof(pixels));
	BITMAPV5HEADER* header;
	if ( memory == NULL ) return 0;
	header = (BITMAPV5HEADER*)GlobalLock(memory);
	if ( header == NULL ) { GlobalFree(memory); return 0; }
	memset(header, 0, sizeof(*header));
	header->bV5Size = sizeof(*header);
	header->bV5Width = corrupt == 1 ? 1000 : 2;
	header->bV5Height = -1;
	header->bV5Planes = 1;
	header->bV5BitCount = 32;
	header->bV5Compression = BI_BITFIELDS;
	header->bV5SizeImage = sizeof(pixels);
	header->bV5RedMask = 0x00ff0000u;
	header->bV5GreenMask = corrupt == 2 ? 0x00ff0000u : 0x0000ff00u;
	header->bV5BlueMask = 0x000000ffu;
	header->bV5AlphaMask = 0xff000000u;
	memcpy((unsigned char*)header + sizeof(*header), pixels, sizeof(pixels));
	GlobalUnlock(memory);
	if ( !__testOpenClipboard(owner) ) { GlobalFree(memory); return 0; }
	if ( !EmptyClipboard() || SetClipboardData(CF_DIBV5, memory) == NULL ) {
		CloseClipboard(); GlobalFree(memory); return 0;
	}
	CloseClipboard();
	return 1;
}

static int __testSetClipboardDib24(HWND owner, int delayed)
{
	/* Two bottom-up rows, each padded to a four-byte DIB scan line. */
	static const unsigned char pixels[16] = {
		255u, 0u, 0u, 255u, 255u, 255u, 0u, 0u,
		0u, 0u, 255u, 0u, 255u, 0u, 0u, 0u
	};
	HGLOBAL memory = GlobalAlloc(GMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + sizeof(pixels));
	BITMAPINFOHEADER* header;
	UINT png = 0u;
	if ( memory == NULL ) return 0;
	header = (BITMAPINFOHEADER*)GlobalLock(memory);
	if ( header == NULL ) { GlobalFree(memory); return 0; }
	memset(header, 0, sizeof(*header));
	header->biSize = sizeof(*header);
	header->biWidth = 2;
	header->biHeight = 2;
	header->biPlanes = 1;
	header->biBitCount = 24;
	header->biCompression = BI_RGB;
	header->biSizeImage = sizeof(pixels);
	memcpy((unsigned char*)header + sizeof(*header), pixels, sizeof(pixels));
	GlobalUnlock(memory);
	if ( !__testOpenClipboard(owner) ) { GlobalFree(memory); return 0; }
	if ( !EmptyClipboard() || SetClipboardData(CF_DIB, memory) == NULL ) {
		CloseClipboard(); GlobalFree(memory); return 0;
	}
	if ( delayed ) {
		png = RegisterClipboardFormatA("PNG");
		if ( png == 0u ) { CloseClipboard(); return 0; }
		/* The owner does not render PNG; Windows may synthesize DIBV5. */
		SetClipboardData(CF_DIBV5, NULL);
		SetClipboardData(png, NULL);
	}
	CloseClipboard();
	/* Windows may synthesize CF_DIBV5 from CF_DIB, so the test only requires
	 * a delayed PNG advertisement and verifies the decoded bitmap below. */
	return 1;
}

int main(void)
{
	xge_desc_t tDesc;
	const char* sCurrent;
	const char* sRead;
	char* sOriginal;
	char* sLong;
	size_t iOriginalSize;
	size_t i;
	int iRet;
	int iFailed;
	HWND owner;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iRunMode = XGE_RUN_MANUAL;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		fprintf(stderr, "xgeInit failed: %d\n", iRet);
		return 1;
	}
	owner = CreateWindowExA(0, "STATIC", "XGE clipboard test", WS_POPUP,
		0, 0, 0, 0, NULL, NULL, GetModuleHandleA(NULL), NULL);
	if ( owner == NULL ) { xgeUnit(); return 1; }
	sCurrent = xgeClipboardGetText();
	iOriginalSize = strlen(sCurrent);
	sOriginal = (char*)malloc(iOriginalSize + 1u);
	sLong = (char*)malloc(XGE_CLIPBOARD_TEST_SIZE + 1u);
	if ( sOriginal == NULL || sLong == NULL ) {
		free(sOriginal);
		free(sLong);
		DestroyWindow(owner);
		xgeUnit();
		return 1;
	}
	memcpy(sOriginal, sCurrent, iOriginalSize + 1u);
	for ( i = 0; i < XGE_CLIPBOARD_TEST_SIZE; i++ ) sLong[i] = (char)('A' + (i % 26u));
	sLong[XGE_CLIPBOARD_TEST_SIZE] = '\0';
	iFailed = 0;
	if ( !__testSetClipboardWideFromAscii(owner, sLong, XGE_CLIPBOARD_TEST_SIZE) ) {
		fprintf(stderr, "failed to prepare native 2 MiB clipboard payload\n");
		iFailed = 1;
	} else {
		sRead = xgeClipboardGetText();
		if ( strlen(sRead) != XGE_CLIPBOARD_TEST_SIZE || memcmp(sRead, sLong, XGE_CLIPBOARD_TEST_SIZE) != 0 ) {
			fprintf(stderr, "XGE truncated native 2 MiB clipboard payload\n");
			iFailed = 1;
		}
	}
	if ( !iFailed ) {
		xgeClipboardSetText(sLong);
		if ( !__testSystemClipboardMatchesAscii(owner, sLong, XGE_CLIPBOARD_TEST_SIZE) ) {
			fprintf(stderr, "XGE truncated 2 MiB clipboard output\n");
			iFailed = 1;
		}
	}
	if ( !iFailed ) {
		const unsigned char pixels[16] = {
			255u, 128u, 64u, 64u, 10u, 20u, 30u, 255u,
			40u, 50u, 60u, 128u, 90u, 100u, 110u, 200u
		};
		xge_clipboard_item_t items[2];
		void* png = NULL;
		size_t png_size = 0u;
		unsigned char* copied = NULL;
		UINT format = RegisterClipboardFormatA("PNG");
		DWORD sequence = 0u;
		int stage = 1;
		int fault = 0;
		int size_result = -1;
		int read_result = -1;
		DWORD native_error = 0u;
		SIZE_T native_size = 0u;
		char owner_title[64] = {0};
		iRet = xgeImageEncodePNG(2, 2, pixels, 8, &png, &png_size);
		if ( iRet != XGE_OK || format == 0u ) iFailed = 1;
		if ( !iFailed ) {
			stage = 2;
			items[0] = (xge_clipboard_item_t){XGE_CLIPBOARD_FORMAT_TEXT_UTF8, "image", 5u};
			items[1] = (xge_clipboard_item_t){XGE_CLIPBOARD_FORMAT_IMAGE_PNG, png, png_size};
			if ( xgeClipboardSetItems(items, 2) != XGE_OK || !IsClipboardFormatAvailable(format) ) iFailed = 1;
			sequence = GetClipboardSequenceNumber();
		}
		if ( !iFailed && __testOpenClipboard(owner) ) {
			stage = 3;
			HANDLE data = GetClipboardData(format);
			native_error = GetLastError();
			native_size = data != NULL ? GlobalSize(data) : 0u;
			const void* native = data != NULL ? GlobalLock(data) : NULL;
			if ( native == NULL || native_size != png_size || memcmp(native, png, png_size) != 0 ) {
				fault = data == NULL ? 1 : (native == NULL ? 2 : (native_size != png_size ? 3 : 4));
				iFailed = 1;
			}
			if ( native != NULL ) GlobalUnlock(data);
			if ( !iFailed ) {
				stage = 4;
				static const unsigned char expected_bgra[16] = {
					64u, 128u, 255u, 64u, 30u, 20u, 10u, 255u,
					60u, 50u, 40u, 128u, 110u, 100u, 90u, 200u
				};
				HANDLE bitmap = GetClipboardData(CF_DIBV5);
				const BITMAPV5HEADER* header = bitmap != NULL ?
					(const BITMAPV5HEADER*)GlobalLock(bitmap) : NULL;
				if ( header == NULL || GlobalSize(bitmap) != sizeof(*header) + sizeof(expected_bgra) ||
				     header->bV5Size != sizeof(*header) ||
				     header->bV5Width != 2 || header->bV5Height != -2 ||
				     header->bV5BitCount != 32 || header->bV5Compression != BI_BITFIELDS ||
				     header->bV5SizeImage != sizeof(expected_bgra) ||
				     header->bV5CSType != 0x73524742u ||
				     header->bV5RedMask != 0x00ff0000u ||
				     header->bV5GreenMask != 0x0000ff00u ||
				     header->bV5BlueMask != 0x000000ffu ||
				     header->bV5AlphaMask != 0xff000000u ||
					memcmp((const unsigned char*)header + sizeof(*header),
					expected_bgra, sizeof(expected_bgra)) != 0 ) {
					fault = header == NULL ? 1 : (GlobalSize(bitmap) != sizeof(*header) + sizeof(expected_bgra) ? 2 :
						(header->bV5Size != sizeof(*header) || header->bV5Width != 2 ||
						 header->bV5Height != -2 ? 3 : 4));
					iFailed = 1;
				}
				if ( header != NULL ) GlobalUnlock(bitmap);
			}
			CloseClipboard();
		} else if ( !iFailed ) iFailed = 1;
		if ( !iFailed ) {
			stage = 5;
			copied = (unsigned char*)malloc(png_size);
			if ( copied != NULL ) {
				size_result = __testGetClipboardDataWithRetry(XGE_CLIPBOARD_FORMAT_IMAGE_PNG, NULL, 0u);
				read_result = __testGetClipboardDataWithRetry(XGE_CLIPBOARD_FORMAT_IMAGE_PNG, copied, png_size);
			}
			if ( copied == NULL || size_result != (int)png_size ||
				read_result != (int)png_size || memcmp(copied, png, png_size) != 0 ) iFailed = 1;
		}
		if ( iFailed ) {
			GetWindowTextA(GetClipboardOwner(), owner_title, sizeof(owner_title));
			fprintf(stderr,
			"PNG clipboard format or payload mismatch: stage=%d fault=%d size=%d read=%d "
			"native_error=%lu native_size=%llu sequence=%lu now=%lu "
			"owner=%p title=%s open=%p png=%d dibv5=%d\n",
			stage, fault, size_result, read_result,
			(unsigned long)native_error, (unsigned long long)native_size,
			(unsigned long)sequence, (unsigned long)GetClipboardSequenceNumber(),
			(void*)GetClipboardOwner(), owner_title, (void*)GetOpenClipboardWindow(),
			IsClipboardFormatAvailable(format), IsClipboardFormatAvailable(CF_DIBV5));
		}
		free(copied);
		xrtFree(png);
	}
	if ( !iFailed ) {
		const unsigned char expected[8] = {
			255u, 128u, 64u, 64u, 0u, 255u, 0u, 255u
		};
		xge_image_t image = {0};
		unsigned char* png = NULL;
		int bytes;
		int locked = 0;
		if ( !__testSetClipboardDibV5(owner, 0) ) iFailed = 1;
		bytes = iFailed ? -1 : __testGetClipboardDataWithRetry(XGE_CLIPBOARD_FORMAT_IMAGE_PNG, NULL, 0u);
		if ( bytes <= 8 ) iFailed = 1;
		if ( !iFailed ) {
			/* A size query should retain the converted bytes for the read even
			 * while another owner has the system clipboard open. */
			locked = __testOpenClipboard(owner) ? 1 : 0;
			if ( !locked ) iFailed = 1;
			png = (unsigned char*)malloc((size_t)bytes);
			if ( !iFailed && (png == NULL || __testGetClipboardDataWithRetry(XGE_CLIPBOARD_FORMAT_IMAGE_PNG,
				png, (size_t)bytes) != bytes ||
				xgeImageLoadMemoryEx(&image, png, bytes,
					XGE_IMAGE_STRAIGHT_ALPHA) != XGE_OK ||
				image.iWidth != 2 || image.iHeight != 1 ||
				memcmp(image.pPixels, expected, sizeof(expected)) != 0) ) iFailed = 1;
			if ( locked ) CloseClipboard();
		}
		if ( iFailed ) fprintf(stderr, "CF_DIBV5 PNG fallback mismatch\n");
		xgeImageFree(&image); free(png);
	}
	if ( !iFailed ) {
		if ( !__testSetClipboardDibV5(owner, 1) ||
		     __testGetClipboardDataWithRetry(XGE_CLIPBOARD_FORMAT_IMAGE_PNG, NULL, 0u) >= 0 ||
		     !__testSetClipboardDibV5(owner, 2) ||
		     __testGetClipboardDataWithRetry(XGE_CLIPBOARD_FORMAT_IMAGE_PNG, NULL, 0u) >= 0 ) {
			fprintf(stderr, "malformed CF_DIBV5 was accepted\n");
			iFailed = 1;
		}
	}
	if ( !iFailed ) {
		const unsigned char expected[16] = {
			255u, 0u, 0u, 255u, 0u, 255u, 0u, 255u,
			0u, 0u, 255u, 255u, 255u, 255u, 255u, 255u
		};
		xge_image_t image = {0};
		unsigned char* png = NULL;
		int bytes;
		/* Changing the native clipboard after a size-only read must discard
		 * the previous DIB conversion before the next image is queried. */
		if ( !__testSetClipboardDibV5(owner, 0) ||
		     __testGetClipboardDataWithRetry(XGE_CLIPBOARD_FORMAT_IMAGE_PNG, NULL, 0u) <= 8 ||
		     !__testSetClipboardDib24(owner, 0) ) iFailed = 1;
		bytes = iFailed ? -1 : __testGetClipboardDataWithRetry(XGE_CLIPBOARD_FORMAT_IMAGE_PNG, NULL, 0u);
		if ( bytes <= 8 ) iFailed = 1;
		if ( !iFailed ) {
			png = (unsigned char*)malloc((size_t)bytes);
			if ( png == NULL || __testGetClipboardDataWithRetry(XGE_CLIPBOARD_FORMAT_IMAGE_PNG,
				png, (size_t)bytes) != bytes ||
				xgeImageLoadMemoryEx(&image, png, bytes,
					XGE_IMAGE_STRAIGHT_ALPHA) != XGE_OK ||
				image.iWidth != 2 || image.iHeight != 2 ||
				memcmp(image.pPixels, expected, sizeof(expected)) != 0 ) iFailed = 1;
		}
		if ( iFailed ) fprintf(stderr, "CF_DIB 24-bit PNG fallback mismatch\n");
		xgeImageFree(&image); free(png);
	}
	if ( !iFailed ) {
		const unsigned char expected[16] = {
			255u, 0u, 0u, 255u, 0u, 255u, 0u, 255u,
			0u, 0u, 255u, 255u, 255u, 255u, 255u, 255u
		};
		xge_image_t image = {0};
		unsigned char* png = NULL;
		int bytes;
		int read_result = -1;
		int decode_result = -1;
		int locked = 0;
		DWORD sequence = 0u;
		if ( !__testSetClipboardDib24(owner, 1) ) iFailed = 1;
		sequence = GetClipboardSequenceNumber();
		bytes = iFailed ? -1 : __testGetClipboardDataWithRetry(XGE_CLIPBOARD_FORMAT_IMAGE_PNG, NULL, 0u);
		if ( bytes <= 8 ) iFailed = 1;
		if ( !iFailed ) {
			locked = __testOpenClipboard(owner);
			if ( !locked ) iFailed = 1;
			png = (unsigned char*)malloc((size_t)bytes);
			if ( !iFailed && png != NULL ) read_result = __testGetClipboardDataWithRetry(
				XGE_CLIPBOARD_FORMAT_IMAGE_PNG, png, (size_t)bytes);
			if ( read_result == bytes ) decode_result = xgeImageLoadMemoryEx(
				&image, png, bytes, XGE_IMAGE_STRAIGHT_ALPHA);
			if ( png == NULL || read_result != bytes || decode_result != XGE_OK ||
				image.iWidth != 2 || image.iHeight != 2 ||
				memcmp(image.pPixels, expected, sizeof(expected)) != 0 ) iFailed = 1;
			if ( locked ) CloseClipboard();
		}
		if ( iFailed ) fprintf(stderr,
			"delayed PNG/DIBV5 CF_DIB fallback mismatch: sequence=%lu now=%lu "
			"formats=%d/%d/%d bytes=%d read=%d decode=%d dimensions=%dx%d\n",
			(unsigned long)sequence, (unsigned long)GetClipboardSequenceNumber(),
			IsClipboardFormatAvailable(RegisterClipboardFormatA("PNG")),
			IsClipboardFormatAvailable(CF_DIBV5), IsClipboardFormatAvailable(CF_DIB),
			bytes, read_result, decode_result, image.iWidth, image.iHeight);
		xgeImageFree(&image); free(png);
	}
	xgeClipboardSetText(sOriginal);
	DestroyWindow(owner);
	free(sOriginal);
	free(sLong);
	xgeUnit();
	if ( iFailed ) return 1;
	printf("xge_clipboard_win32_test passed\n");
	return 0;
}

#else
int main(void)
{
	printf("xge_clipboard_win32_test skipped\n");
	return 0;
}
#endif
