
#include "ByteSlice.h"

#include <cstdio>

std::shared_ptr<std::vector<uint8_t>> ByteSlice::ReadFile(const char* fullPath)
{
	FILE* file = fopen(fullPath, "rb");

	if (!file) {
		printf("Unable to open file: '%s'.\n", fullPath);
		return {};
	}

	fseek(file, 0, SEEK_END);
	size_t fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	auto r = std::make_shared<std::vector<uint8_t>>(fileSize);

	fread(&(*r)[0], 1, fileSize, file);
	fclose(file);
	return r;
}

ByteSlice ByteSlice::Get(DataBufferPtr buffer, size_t start, std::optional<size_t> size)
{
	return { &(*buffer)[0] + start, size ? buffer->size() - start : *size };
}