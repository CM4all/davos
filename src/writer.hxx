/*
 * Response body writer.
 *
 * author: Max Kellermann <mk@cm4all.com>
 */

#ifndef DAVOS_WRITER_HXX
#define DAVOS_WRITER_HXX

extern "C" {
#include <was/simple.h>
}

#include <utility>

#include <string.h>
#include <stdio.h>

class Writer {
	was_simple *const w;
	uint64_t written;

	char buffer[16384];
	size_t n_buffer;

public:
	explicit Writer(was_simple *_w)
		:w(_w), written(0), n_buffer(0) {}

	Writer(const Writer &) = delete;

	~Writer() {
		if (was_simple_set_length(w, written + n_buffer))
			Flush();
	}

	bool Flush() {
		bool success = was_simple_write(w, buffer, n_buffer);
		if (success) {
			written += n_buffer;
			n_buffer = 0;
		}

		return success;
	}

	bool WantWrite(size_t size) {
		return n_buffer + size <= sizeof(buffer) || Flush();
	}

	bool Write(const void *p, size_t size) {
		if (!WantWrite(size))
			return false;

		if (size > sizeof(buffer)) {
			assert(n_buffer == 0);
			if (!was_simple_write(w, p, size))
				return false;

			written += size;
			return true;
		} else {
			memcpy(buffer + n_buffer, p, size);
			n_buffer += size;
			return true;
		}
	}

	bool Write(const char *s) {
		return Write(s, strlen(s));
	}

	template<typename... Args>
	bool Format(const char *fmt, Args... args) {
		if (!Flush())
			return false;

		n_buffer = snprintf(buffer, sizeof(buffer), fmt,
				    std::forward<Args>(args)...);
		return true;
	}
};

#endif
