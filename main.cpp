#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <poppler-document.h>
#include <poppler-page.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <cerrno>

int main()
{
	int64_t rc = 0;
	uint64_t const pagesz = sysconf(_SC_PAGESIZE);
	uint64_t len_mmap = (pagesz << 1);

        errno = 0;
        int fd = -1;
        int64_t of = 0;
        void *buf = mmap(NULL, len_mmap, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, fd, of);
	if (MAP_FAILED == buf) {
		if (errno) {
			fprintf(stderr, "%s\n", strerror(errno));
		}
		exit(EXIT_FAILURE);
	}

	rc = madvise(buf, len_mmap, MADV_WILLNEED);
	if (-1 == rc) {
		fprintf(stderr, "%s", "error: madvise fast access failed\n");
		if (errno) {
			fprintf(stderr, "%s\n", strerror(errno));
		}
		exit(EXIT_FAILURE);
	}


	std::unique_ptr<poppler::document> doc(poppler::document::load_from_file("doc.pdf"));
	if (!doc) {
		fprintf(stderr, "%s", "error: failed to load document from file\n");
		exit(EXIT_FAILURE);
	}

	// NOTE: we are assuming that the important is on the first page
	std::unique_ptr<poppler::page> page(doc->create_page(0));
	if (!page) {
		fprintf(stderr, "%s", "error: failed to create page\n");
		exit(EXIT_FAILURE);
	}


	uint64_t count = 0;
	char unsigned *data = (char unsigned*) buf;
	poppler::byte_array bytes = page->text().to_utf8();
	for (uint64_t i = 0; i != bytes.size(); ++i, ++count, ++data) {
		*data = bytes[i];
	}

	data = (char unsigned*) buf;
	fprintf(stdout, "%s", data);
	return 0;
}
