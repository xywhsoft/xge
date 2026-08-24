#define XRT_MODULE_DEFLATE
#define XRT_MODULE_INFLATE
#define XREGEX_IMPLEMENTATION
#include "../../lib/xrt/xrt.h"

#include <stdio.h>
#include <windows.h>

typedef struct bench_item_t {
	unsigned char* source;
	size_t source_size;
	unsigned char* compressed;
	size_t compressed_size;
} bench_item_t;

static double now_seconds(void)
{
	LARGE_INTEGER counter;
	LARGE_INTEGER frequency;
	QueryPerformanceCounter(&counter);
	QueryPerformanceFrequency(&frequency);
	return (double)counter.QuadPart / (double)frequency.QuadPart;
}

static unsigned char* read_file(const char* path, size_t* size)
{
	FILE* file;
	long length;
	unsigned char* data;
	*size = 0;
	file = fopen(path, "rb");
	if ( !file ) return NULL;
	fseek(file, 0, SEEK_END);
	length = ftell(file);
	fseek(file, 0, SEEK_SET);
	if ( length < 0 ) { fclose(file); return NULL; }
	data = (unsigned char*)malloc((size_t)length + 1u);
	if ( !data || fread(data, 1, (size_t)length, file) != (size_t)length ) {
		free(data);
		fclose(file);
		return NULL;
	}
	fclose(file);
	*size = (size_t)length;
	return data;
}

static int load_items(const char* dataset, bench_item_t** items_out, int* count_out,
	unsigned char** corpus_out, size_t* corpus_size_out)
{
	char path[MAX_PATH * 2];
	char line[2048];
	FILE* manifest;
	bench_item_t* items = NULL;
	int count = 0;
	int capacity = 0;
	size_t corpus_size = 0;
	unsigned char* corpus;
	size_t offset = 0;

	snprintf(path, sizeof(path), "%s\\manifest.txt", dataset);
	manifest = fopen(path, "rb");
	if ( !manifest ) return 0;
	while ( fgets(line, sizeof(line), manifest) ) {
		char* first = strchr(line, '|');
		char* second;
		bench_item_t item;
		if ( !first ) continue;
		second = strchr(first + 1, '|');
		if ( !second ) continue;
		*second = '\0';
		if ( count == capacity ) {
			int new_capacity = capacity ? capacity * 2 : 256;
			bench_item_t* grown = (bench_item_t*)realloc(items,
				sizeof(*items) * (size_t)new_capacity);
			if ( !grown ) { fclose(manifest); return 0; }
			items = grown;
			capacity = new_capacity;
		}
		memset(&item, 0, sizeof(item));
		snprintf(path, sizeof(path), "%s\\svg\\%s", dataset, first + 1);
		item.source = read_file(path, &item.source_size);
		if ( !item.source ) { fclose(manifest); return 0; }
		items[count++] = item;
		corpus_size += item.source_size;
	}
	fclose(manifest);

	corpus = (unsigned char*)malloc(corpus_size);
	if ( !corpus ) return 0;
	for ( int i = 0; i < count; ++i ) {
		memcpy(corpus + offset, items[i].source, items[i].source_size);
		offset += items[i].source_size;
	}
	*items_out = items;
	*count_out = count;
	*corpus_out = corpus;
	*corpus_size_out = corpus_size;
	return 1;
}

static void* compress_zlib(const void* source, size_t source_size, size_t* compressed_size)
{
	xdeflateconfig config;
	xbytesview input = { (const uint8*)source, source_size };
	xrtDeflateConfigInit(&config);
	config.Format = XDEFLATE_ZLIB;
	config.Level = 9;
	return xrtDeflateAll(input, &config, compressed_size);
}

static int decompress_zlib(void* output, size_t output_size, const void* source,
	size_t source_size)
{
	xinflateconfig config;
	xbytesview input = { (const uint8*)source, source_size };
	size_t decoded_size = 0;
	bytes decoded;
	xrtInflateConfigInit(&config);
	config.Format = XINFLATE_ZLIB;
	config.OutputLimit = output_size;
	decoded = xrtInflateAll(input, &config, &decoded_size);
	if ( !decoded || decoded_size != output_size ) {
		xrtFree(decoded);
		return 0;
	}
	memcpy(output, decoded, output_size);
	xrtFree(decoded);
	return 1;
}

int main(int argc, char** argv)
{
	bench_item_t* items;
	int item_count;
	unsigned char* corpus;
	size_t corpus_size;
	void* whole_compressed;
	size_t whole_compressed_size;
	unsigned char* output;
	double start;
	double whole_compress_seconds;
	double whole_decode_seconds;
	double per_file_compress_seconds;
	double random_seconds;
	size_t per_file_payload = 0;
	size_t random_bytes = 0;
	uint32_t random_state = 0x584745u;
	int random_passes = 30;
	int random_operations;
	volatile unsigned int checksum = 0;

	if ( argc < 2 ) {
		fprintf(stderr, "usage: benchmark_xrt_deflate DATASET [RANDOM_PASSES]\n");
		return 2;
	}
	if ( argc >= 3 ) random_passes = atoi(argv[2]);
	if ( random_passes <= 0 ) return 2;
	if ( !load_items(argv[1], &items, &item_count, &corpus, &corpus_size) ) {
		fprintf(stderr, "failed to load dataset\n");
		return 1;
	}
	output = (unsigned char*)malloc(corpus_size);
	if ( !output ) return 1;

	start = now_seconds();
	whole_compressed = compress_zlib(corpus, corpus_size, &whole_compressed_size);
	whole_compress_seconds = now_seconds() - start;
	if ( !whole_compressed ) return 1;

	if ( !decompress_zlib(output, corpus_size, whole_compressed, whole_compressed_size) ||
		memcmp(output, corpus, corpus_size) != 0 ) return 1;
	start = now_seconds();
	for ( int i = 0; i < 100; ++i ) {
		if ( !decompress_zlib(output, corpus_size, whole_compressed, whole_compressed_size) ) return 1;
	}
	whole_decode_seconds = (now_seconds() - start) / 100.0;

	start = now_seconds();
	for ( int i = 0; i < item_count; ++i ) {
		items[i].compressed = (unsigned char*)compress_zlib(items[i].source,
			items[i].source_size, &items[i].compressed_size);
		if ( !items[i].compressed ) return 1;
		per_file_payload += items[i].compressed_size;
	}
	per_file_compress_seconds = now_seconds() - start;

	for ( int i = 0; i < item_count; ++i ) {
		if ( !decompress_zlib(output, items[i].source_size, items[i].compressed,
			items[i].compressed_size) || memcmp(output, items[i].source, items[i].source_size) != 0 ) return 1;
	}

	random_operations = item_count * random_passes;
	start = now_seconds();
	for ( int i = 0; i < random_operations; ++i ) {
		int index;
		random_state ^= random_state << 13;
		random_state ^= random_state >> 17;
		random_state ^= random_state << 5;
		index = (int)(random_state % (uint32_t)item_count);
		if ( !decompress_zlib(output, items[index].source_size, items[index].compressed,
			items[index].compressed_size) ) return 1;
		checksum ^= output[0];
		random_bytes += items[index].source_size;
	}
	random_seconds = now_seconds() - start;

	printf("{\n");
	printf("  \"name\": \"xrt-deflate-9\",\n");
	printf("  \"records\": %d,\n", item_count);
	printf("  \"source_bytes\": %zu,\n", corpus_size);
	printf("  \"whole\": {\"payload_bytes\": %zu, \"ratio\": %.9f, \"compress_ms\": %.6f, \"decode_mib_s\": %.6f},\n",
		whole_compressed_size, (double)whole_compressed_size / (double)corpus_size,
		whole_compress_seconds * 1000.0,
		((double)corpus_size / (1024.0 * 1024.0)) / whole_decode_seconds);
	printf("  \"per_file\": {\"payload_bytes\": %zu, \"index_bytes\": %zu, \"total_bytes\": %zu, \"ratio\": %.9f, \"compress_ms\": %.6f, \"operations\": %d, \"operations_per_second\": %.6f, \"average_us\": %.6f, \"mib_s\": %.6f},\n",
		per_file_payload, (size_t)item_count * 8u,
		per_file_payload + (size_t)item_count * 8u,
		(double)(per_file_payload + (size_t)item_count * 8u) / (double)corpus_size,
		per_file_compress_seconds * 1000.0, random_operations,
		(double)random_operations / random_seconds,
		random_seconds * 1000000.0 / (double)random_operations,
		((double)random_bytes / (1024.0 * 1024.0)) / random_seconds);
	printf("  \"checksum\": %u\n", checksum);
	printf("}\n");
	return 0;
}
