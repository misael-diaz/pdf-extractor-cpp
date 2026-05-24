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

#define BUFFER_SIZE 256

int main()
{
	int64_t rc = 0;
	uint64_t const pagesz = sysconf(_SC_PAGESIZE);
	uint64_t len_mmap = (pagesz << 1);

        errno = 0;
        void *buf = mmap(NULL, len_mmap, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
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

	// NOTE: we are assuming that the important data is on the first page
	std::unique_ptr<poppler::page> page(doc->create_page(0));
	if (!page) {
		fprintf(stderr, "%s", "error: failed to create page\n");
		exit(EXIT_FAILURE);
	}

	// NOTE: Even though the code is returning an error, this can be used to signal
	//       that the medical document must be processed manually probably because
	//       of poor resolution. This is the right way to respond to that case.
	poppler::byte_array bytes = page->text().to_utf8();
	if (!bytes.size()) {
		fprintf(stderr, "%s", "error: poppler failed to extract text data\n");
		exit(EXIT_FAILURE);
	}

	uint64_t bytes_written = 0;
	char unsigned *data = (char unsigned*) buf;
	for (uint64_t i = 0; i != bytes.size(); ++i, ++bytes_written) {
		data[bytes_written] = bytes[i];
		if ((len_mmap - bytes_written) <= pagesz) {
			buf = mremap(buf, len_mmap, (len_mmap << 1), MREMAP_MAYMOVE);
			if (MAP_FAILED == buf) {
				if (errno) {
					fprintf(stderr, "%s\n", strerror(errno));
				}
				exit(EXIT_FAILURE);
			}
			len_mmap <<= 1;
			data = (char unsigned*) buf;
		}
	}

	errno = 0;
	void *dstbuf = mmap(NULL, len_mmap, PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (MAP_FAILED == dstbuf) {
		fprintf(stderr, "%s", "error: destination chat memory mapping failed\n");
		if (errno) {
			fprintf(stderr, "%s\n", strerror(errno));
		}
		_exit(1);
	}

	rc = madvise(dstbuf, len_mmap, MADV_WILLNEED);
	if (-1 == rc) {
		fprintf(stderr, "%s", "error: dest mmap sequential access failed\n");
		if (errno) {
			fprintf(stderr, "%s\n", strerror(errno));
		}
		_exit(1);
	}

	uint64_t len = 0;
	uint64_t count = 0;
	char unsigned *srcbuf = (char unsigned*) buf;
	char unsigned *dst = (char unsigned*) dstbuf;
	char unsigned *txt = (char unsigned*) srcbuf;
	// excludes non-ASCII characters from the content
	while (bytes_written > count) {
		if (0x80u > (*txt)) {
			if (((*txt) < 0x0au)) {
				*dst = 0x20u;
			}
			else if (((*txt) >= 0x0bu) && ((*txt) < 0x20u)) {
				*dst = 0x20u;
			}
			else if (((*txt) == 0x22u) || ((*txt) == 0x27u)) {
				*dst = 0x20u;
			}
			else if (((*txt) >= 0x41u) && ((*txt) < 0x5bu)) {
				*dst = (((*txt) - 0x41u) + 0x61u);
			}
			else if ((0x7fu == (*txt))) {
				*dst = 0x20u;
			}
			else {
				*dst = *txt;
			}
			txt += 1;
			dst += 1;
			count += 1;
			len += 1;
		}
		else if (0xc2 > (*txt))  {
			fprintf(stderr, "%s", "error: invalid utf-8 prefix\n");
			_exit(1);
		}
		else if (0xe0u > (*txt)) {
			uint16_t const value = ((txt[1] << 8) | txt[0]);
			if ((value >= 0x80c3u) && (value < 0x86c3u)) {
				*dst = 'a';
				dst += 1;
				len += 1;
			}
			else if ((value >= 0x88c3u) && (value < 0x8cc3u)) {
				*dst = 'e';
				dst += 1;
				len += 1;
			}
			else if ((value >= 0x8cc3u) && (value < 0x90c3u)) {
				*dst = 'i';
				dst += 1;
				len += 1;
			}
			else if ((value >= 0x92c3u) && (value < 0x97c3u)) {
				*dst = 'o';
				dst += 1;
				len += 1;
			}
			else if ((value >= 0x99c3u) && (value < 0x9ec3u)) {
				*dst = 'u';
				dst += 1;
				len += 1;
			}
			else if ((value >= 0xa0c3u) && (value < 0xa6c3u)) {
				*dst = 'a';
				dst += 1;
				len += 1;
			}
			else if ((value >= 0xa8c3u) && (value < 0xacc3u)) {
				*dst = 'e';
				dst += 1;
				len += 1;
			}
			else if ((value >= 0xacc3u) && (value < 0xb0c3u)) {
				*dst = 'i';
				dst += 1;
				len += 1;
			}
			else if ((value == 0xb1c3u)) {
				*dst = 'n';
				dst += 1;
				len += 1;
			}
			else if ((value >= 0xb2c3u) && (value < 0xb7c3u)) {
				*dst = 'o';
				dst += 1;
				len += 1;
			}
			else if ((value >= 0xb9c3u) && (value < 0xbdc3u)) {
				*dst = 'u';
				dst += 1;
				len += 1;
			}
			txt += 2;
			count += 2;
		}
		else if (0xf0u > (*txt)) {
			txt += 3;
			count += 3;
		}
		else {
			txt += 4;
			count += 4;
		}
	}
	if (bytes_written != count) {
		fprintf(stderr, "%s", "error: bytes read and filesize mismatch\n");
		exit(EXIT_FAILURE);
	}
	else {
		fprintf(stdout, "%s %lu\n", "bytes-raw:", bytes_written);
		fprintf(stdout, "%s %lu\n", "bytes-kept:", len);
	}

	// filters garbage from some medical records that use watermarks
	data = (char unsigned*) dstbuf;
	for (uint64_t i = 0; i != len; ++i) {
		if (('e' == data[i]) && ('p' == data[i + 1]) && ((' ' == data[i + 2]) || ('\n' == data[i + 2]))) {
			data[i] = ' ';
			data[i + 1] = ' ';
		}
		if (('s' == data[i]) && ('u' == data[i + 1]) && ((' ' == data[i + 2]) || ('\n' == data[i + 2]))) {
			data[i] = ' ';
			data[i + 1] = ' ';
		}
		if (((' ' == data[i]) || ('\n' == data[i])) && ('a' == data[i + 1]) && ((' ' == data[i + 2]) || ('\n' == data[i + 2]))) {
			data[i + 1] = ' ';
		}
		if (((' ' == data[i]) || ('\n' == data[i])) && ('r' == data[i + 1]) && ((' ' == data[i + 2]) || ('\n' == data[i + 2]))) {
			data[i + 1] = ' ';
		}
		if (((' ' == data[i]) || ('\n' == data[i])) && ('s' == data[i + 1]) && ((' ' == data[i + 2]) || ('\n' == data[i + 2]))) {
			data[i + 1] = ' ';
		}
		if (((' ' == data[i]) || ('\n' == data[i])) && ('e' == data[i + 1]) && ((' ' == data[i + 2]) || ('\n' == data[i + 2]))) {
			data[i + 1] = ' ';
		}
	}

	fprintf(stdout, "%s", data);


	// extracts data based on provider
	char placeholder[BUFFER_SIZE];
	char patient_name[BUFFER_SIZE];
	char patient_document[BUFFER_SIZE];
	char physician_name[BUFFER_SIZE];
	char *csi = strstr((char*) dstbuf, "clinica san ignacio");
	if (csi) {
		fprintf(stdout, "%s", "processing: clinica san ignacio document\n");
		char paciente[] = "paciente";
		char *patient = strstr((char*) dstbuf, paciente);
		if (!patient)  {
			fprintf(stderr, "%s", "error: missing patient info\n");
			exit(EXIT_FAILURE);
		}
		char *type = strstr((char*) dstbuf, "tipo paciente");
		if (!type)  {
			fprintf(stderr, "%s", "error: missing patient-type info\n");
			exit(EXIT_FAILURE);
		}
		if (patient >= type) {
			fprintf(stderr, "%s", "error: unexpected layout\n");
			exit(EXIT_FAILURE);
		}
		if ((type - patient) >= BUFFER_SIZE) {
			fprintf(stderr, "%s", "error: would overrun buffer\n");
			exit(EXIT_FAILURE);
		}
		patient += sizeof(paciente);
		uint64_t sz = (type - patient);
		memset(patient_name, 0, BUFFER_SIZE);
		memcpy(patient_name, patient, sz);

		// truncates patient name so that it does not look weird on the console
		char *str = patient_name;
		while (*str) {
			if (
				((' ' == str[0]) || ('\n' == str[0])) &&
				((' ' == str[1]) || ('\n' == str[1]))
			   ) {
				str[0] = 0;
				break;
			}
			++str;
		}

		char numero[] = "numero";
		char *document = strstr((char*) dstbuf, numero);
		if (!document)  {
			fprintf(stderr, "%s", "error: missing patient-document info\n");
			exit(EXIT_FAILURE);
		}

		char *age = strstr((char*) dstbuf, "edad");
		if (!age)  {
			fprintf(stderr, "%s", "error: missing patient-age info\n");
			exit(EXIT_FAILURE);
		}

		if (document >= age) {
			fprintf(stderr, "%s", "error: unexpected layout\n");
			exit(EXIT_FAILURE);
		}
		if ((age - document) >= BUFFER_SIZE) {
			fprintf(stderr, "%s", "error: would overrun buffer\n");
			exit(EXIT_FAILURE);
		}

		document += sizeof(numero);
		sz = (age - document);
		memset(patient_document, 0, BUFFER_SIZE);
		memcpy(patient_document, document, sz);

		// extracts the physician name
		char *physician = strstr((char*) dstbuf, "t. profesional");
		if (!physician)  {
			fprintf(stderr, "%s", "error: missing physician-id info\n");
			exit(EXIT_FAILURE);
		}

		str = physician;
		char *beg = physician;
		char *end = physician;
		int end_found = 0;
		while (*str) {
			if (!end_found) {
				if (
					((' ' == str[ 0]) || ('\n' == str[ 0])) &&
					((' ' == str[-1]) || ('\n' == str[-1])) &&
					((str[-2] >= 0x61) && (str[-2] < 0x7B)) &&
					((str[-3] >= 0x61) && (str[-3] < 0x7B))
				   ) {
					end_found = 1;
					end = &str[-1];
				}
			}
			else if (
					((str[ 0] >= 0x61) && (str[ 0] < 0x7B)) &&
					((str[-1] >= 0x61) && (str[-1] < 0x7B)) &&
					((' ' == str[-2]) || ('\n' == str[-2])) &&
					((' ' == str[-3]) || ('\n' == str[-3]))
				   ) {
				beg = &str[-1];
				break;
			}
			--str;
		}

		if ((physician == end) || (physician == beg)) {
			fprintf(stderr, "%s", "error: failed to extract physician name\n");
			exit(EXIT_FAILURE);
		}

		if ((end - beg) >= BUFFER_SIZE) {
			fprintf(stderr, "%s", "error: would overrun buffer\n");
			exit(EXIT_FAILURE);
		}

		sz = (end - beg);
		memset(physician_name, 0, BUFFER_SIZE);
		memcpy(physician_name, beg, sz);

		fprintf(stdout, "name: %s\n", patient_name);
		fprintf(stdout, "id: %s\n", patient_document);
		fprintf(stdout, "physician: %s\n", physician_name);
		exit(EXIT_SUCCESS);
	}

        char *cs = strstr((char*) dstbuf, "coopsana");
        if (cs) {
		fprintf(stdout, "%s", "processing: coopsana document\n");
		char paciente[] = "paciente";
		char *patient = strstr((char*) dstbuf, paciente);
		if (!patient)  {
			fprintf(stderr, "%s", "error: missing patient info\n");
			exit(EXIT_FAILURE);
		}
		char *pid = strstr((char*) dstbuf, "cedula");
		if (!pid)  {
			fprintf(stderr, "%s", "error: missing patient-cedula info\n");
			exit(EXIT_FAILURE);
		}
		if (patient >= pid) {
			fprintf(stderr, "%s", "error: unexpected layout\n");
			exit(EXIT_FAILURE);
		}
		if ((pid - patient) >= BUFFER_SIZE) {
			fprintf(stderr, "%s", "error: would overrun buffer\n");
			exit(EXIT_FAILURE);
		}
		patient += sizeof(paciente);
		uint64_t sz = (pid - patient);
		memset(placeholder, 0, BUFFER_SIZE);
		memcpy(placeholder, patient, sz);

		// truncates patient name so that it does not look weird on the console
		int name_found = 0;
		char *str = placeholder;
		char *beg = placeholder;
		char *end = placeholder;
		while (*str) {
			if (!name_found) {
				if ((*str >= 0x61) && (*str < 0x7B)) {
					name_found = 1;
					beg = str;
				}
			}
			else if (
				((' ' == str[0]) || ('\n' == str[0])) &&
				((' ' == str[1]) || ('\n' == str[1]))
			   ) {
				str[0] = 0;
				end = str;
				break;
			}
			++str;
		}

		memset(patient_name, 0, BUFFER_SIZE);
		memcpy(patient_name, beg, (end - beg));

		char cedula[] = "cedula";
		char *document = strstr((char*) dstbuf, cedula);
		if (!document)  {
			fprintf(stderr, "%s", "error: missing patient-document info\n");
			exit(EXIT_FAILURE);
		}

		char *address = strstr((char*) document, "direccion");
		if (!address)  {
			fprintf(stderr, "%s", "error: missing patient-address info\n");
			exit(EXIT_FAILURE);
		}

		if (document >= address) {
			fprintf(stderr, "%s", "error: unexpected layout\n");
			exit(EXIT_FAILURE);
		}
		if ((address - document) >= BUFFER_SIZE) {
			fprintf(stderr, "%s", "error: would overrun buffer\n");
			exit(EXIT_FAILURE);
		}

		document += sizeof(cedula);
		sz = (address - document);
		memset(patient_document, 0, BUFFER_SIZE);
		memcpy(patient_document, document, sz);

		// extracts the physician name
		char profesional[] = "profesional";
		char *physician = strstr((char*) dstbuf, profesional);
		if (!physician)  {
			fprintf(stderr, "%s", "error: missing physician info\n");
			exit(EXIT_FAILURE);
		}

		document = strstr(physician, cedula);
		if (!document)  {
			fprintf(stderr, "%s", "error: missing physician id\n");
			exit(EXIT_FAILURE);
		}
		if (physician >= document) {
			fprintf(stderr, "%s", "error: unexpected layout\n");
			exit(EXIT_FAILURE);
		}
		if ((document - physician) >= BUFFER_SIZE) {
			fprintf(stderr, "%s", "error: would overrun buffer\n");
			exit(EXIT_FAILURE);
		}

		physician += sizeof(profesional);
		sz = (document - physician);
		memset(physician_name, 0, BUFFER_SIZE);
		memcpy(physician_name, physician, sz);

		fprintf(stdout, "name: %s\n", patient_name);
		fprintf(stdout, "id: %s\n", patient_document);
		fprintf(stdout, "physician: %s\n", physician_name);

		exit(EXIT_SUCCESS);
	}

        char *ste = strstr((char*) dstbuf, "salud total eps");
	if (ste) {
		fprintf(stdout, "%s", "processing: salud total document\n");
		char anchor[] = "tipo documento";
		char *doctype = strstr((char*) dstbuf, anchor);
		if (!doctype) {
			fprintf(stderr, "%s", "error: missing doctype info\n");
			exit(EXIT_FAILURE);
		}
		char nombre[] = "nombre:";
		char *patient = strstr((char*) doctype, nombre);
		if (!patient)  {
			fprintf(stderr, "%s", "error: missing patient info\n");
			exit(EXIT_FAILURE);
		}
		char *date = strstr((char*) doctype, "fecha");
		if (!date)  {
			fprintf(stderr, "%s", "error: missing date info\n");
			exit(EXIT_FAILURE);
		}
		if (patient >= date) {
			fprintf(stderr, "%s", "error: unexpected layout\n");
			exit(EXIT_FAILURE);
		}
		if ((date - patient) >= BUFFER_SIZE) {
			fprintf(stderr, "%s", "error: would overrun buffer\n");
			exit(EXIT_FAILURE);
		}
		uint64_t sz = (date - (patient + sizeof(nombre)));
		memset(patient_name, 0, BUFFER_SIZE);
		memcpy(patient_name, patient + sizeof(nombre), sz);

		char documento[] = "documento:";
		char *document = strstr(doctype + sizeof(anchor), documento);
		if (!document)  {
			fprintf(stderr, "%s", "error: missing document info\n");
			exit(EXIT_FAILURE);
		}
		if (document >= patient) {
			fprintf(stderr, "%s", "error: unexpected layout\n");
			exit(EXIT_FAILURE);
		}
		if ((patient - document) >= BUFFER_SIZE) {
			fprintf(stderr, "%s", "error: would overrun buffer\n");
			exit(EXIT_FAILURE);
		}

		document += sizeof(documento);
		sz = (patient - document);
		memset(patient_document, 0, BUFFER_SIZE);
		memcpy(patient_document, document, sz);

		// extracts physician name
		char prescritor[] = "prescritor";
		char *section = strstr((char*) dstbuf, prescritor);
		if (!section)  {
			fprintf(stderr, "%s", "error: missing physician-section info\n");
			exit(EXIT_FAILURE);
		}

		char *physician = strstr(section, nombre);
		if (!section)  {
			fprintf(stderr, "%s", "error: missing physician-name info\n");
			exit(EXIT_FAILURE);
		}

		char *tel = strstr(section, "telefono");
		if (!tel)  {
			fprintf(stderr, "%s", "error: missing physician-telephone\n");
			exit(EXIT_FAILURE);
		}
		if (physician >= tel) {
			fprintf(stderr, "%s", "error: unexpected layout\n");
			exit(EXIT_FAILURE);
		}
		if ((tel - physician) >= BUFFER_SIZE) {
			fprintf(stderr, "%s", "error: would overrun buffer\n");
			exit(EXIT_FAILURE);
		}

		physician += sizeof(nombre);
		sz = (tel - physician);
		memset(physician_name, 0, BUFFER_SIZE);
		memcpy(physician_name, physician, sz);

		fprintf(stdout, "name: %s\n", patient_name);
		fprintf(stdout, "id: %s\n", patient_document);
		fprintf(stdout, "physician: %s\n", physician_name);
		exit(EXIT_SUCCESS);
	}

	char *se = strstr((char*) dstbuf, "sanitas");
	if (se) {
		fprintf(stdout, "%s", "processing: sanitas document\n");
		char nombre[] = "nombre";
		char *patient = strstr((char*) dstbuf, nombre);
		if (!patient)  {
			fprintf(stderr, "%s", "error: missing patient info\n");
			exit(EXIT_FAILURE);
		}
		char *type = strstr((char*) dstbuf, "tipo");
		if (!type)  {
			fprintf(stderr, "%s", "error: missing user-type info\n");
			exit(EXIT_FAILURE);
		}
		if (patient >= type) {
			fprintf(stderr, "%s", "error: unexpected layout\n");
			exit(EXIT_FAILURE);
		}
		if ((type - patient) >= BUFFER_SIZE) {
			fprintf(stderr, "%s", "error: would overrun buffer\n");
			exit(EXIT_FAILURE);
		}
		patient += sizeof(nombre);
		uint64_t sz = (type - patient);
		memset(patient_name, 0, BUFFER_SIZE);
		memcpy(patient_name, patient, sz);


		char identificacion[] = "identificacion: c.c";
		char *number = strstr((char*) dstbuf, identificacion);
		if (!number)  {
			fprintf(stderr, "%s", "error: missing patient number info\n");
			exit(EXIT_FAILURE);
		}

		char *sex = strstr((char*) dstbuf, "- sex");
		if (!number)  {
			fprintf(stderr, "%s", "error: missing patient gender info\n");
			exit(EXIT_FAILURE);
		}

		if (number >= sex) {
			fprintf(stderr, "%s", "error: unexpected layout\n");
			exit(EXIT_FAILURE);
		}
		if ((sex - number) >= BUFFER_SIZE) {
			fprintf(stderr, "%s", "error: would overrun buffer\n");
			exit(EXIT_FAILURE);
		}

		number += sizeof(identificacion);
		sz = (sex - number);
		memset(patient_document, 0, BUFFER_SIZE);
		memcpy(patient_document, number, sz);

		fprintf(stdout, "name: %s\n", patient_name);
		fprintf(stdout, "id: %s\n", patient_document);
		exit(EXIT_SUCCESS);
	}

	char *sure = strstr((char*) dstbuf, "sura");
	if (sure) {
		fprintf(stdout, "%s", "processing: sura document\n");

		char ccpattern[] = "cc -";
		char *cc = strstr((char*) dstbuf, ccpattern);
		if (!cc) {
			fprintf(stderr, "%s", "error: missing patient id\n");
			exit(EXIT_FAILURE);
		}

		// finds the initial part of the patient name
		int sw = 0;
		char *str = cc;
		str += sizeof(ccpattern);
		while (*str) {
			if ((*str >= 0x61) && (*str < 0x7B)) {
				sw = 1;
				break;
			}
			++str;
		}
		if (!sw) {
			fprintf(stderr, "%s", "error: missing patient name\n");
			exit(EXIT_FAILURE);
		}

		if ((str - cc) >= BUFFER_SIZE) {
			fprintf(stderr, "%s", "error: would overrun buffer\n");
			exit(EXIT_FAILURE);
		}

		// extracts the document id (cedula)
		uint64_t sz = (str - (cc + sizeof(ccpattern)));
		memset(patient_document, 0, BUFFER_SIZE);
		memcpy(patient_document, cc + sizeof(ccpattern), sz);

		char afiliado[] = "afiliado";
		char *patient = strstr((char*) dstbuf, afiliado);
		if (!patient)  {
			fprintf(stderr, "%s", "error: missing patient info\n");
			exit(EXIT_FAILURE);
		}

		if (str >= patient) {
			fprintf(stderr, "%s", "error: surprising layout\n");
			exit(EXIT_FAILURE);
		}

		if ((str - patient) >= BUFFER_SIZE) {
			fprintf(stderr, "%s", "error: would overrun buffer\n");
			exit(EXIT_FAILURE);
		}

		// copy the first part of the name
		sz = (patient - str);
		memset(patient_name, 0, BUFFER_SIZE);
		memcpy(patient_name, str, sz);

		char *truncate = patient_name;
		while (*truncate) {
			if (
				((' ' == truncate[0]) || ('\n' == truncate[0])) &&
				((' ' == truncate[1]) || ('\n' == truncate[1]))
			   )	{
				truncate[0] = ' ';
				truncate[1] = 0;
				break;
			}
			++truncate;
		}

		patient += sizeof(afiliado);
		while (*patient) {
			if ((*patient >= 0x61) && (*patient < 0x7B)) {
				break;
			}
			++patient;
		}

		char *type = strstr((char*) dstbuf, "ips afiliado");
		if (!type)  {
			fprintf(stderr, "%s", "error: missing patient-type info\n");
			exit(EXIT_FAILURE);
		}
		if (patient >= type) {
			fprintf(stderr, "%s", "error: unexpected layout\n");
			exit(EXIT_FAILURE);
		}
		if ((type - patient) >= BUFFER_SIZE) {
			fprintf(stderr, "%s", "error: would overrun buffer\n");
			exit(EXIT_FAILURE);
		}

		sz = (type - patient);
		if ((strlen(patient_name) + sz + 1) >= BUFFER_SIZE) {
			fprintf(stderr, "%s", "error: would overrun buffer\n");
			exit(EXIT_FAILURE);
		}

		strncat(patient_name, patient, sz);

		// truncates patient name so that it does not look weird on the console
		str = patient_name;
		while (*str) {
			if (
				((' ' == str[0]) || ('\n' == str[0])) &&
				((' ' == str[1]) || ('\n' == str[1]))
			   ) {
				str[0] = 0;
				break;
			}
			++str;
		}

		fprintf(stdout, "name: %s\n", patient_name);
		fprintf(stdout, "id: %s\n", patient_document);
		exit(EXIT_SUCCESS);
	}

	return 0;
}
