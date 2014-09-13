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

class Writer {
    was_simple *const w;

public:
    explicit Writer(was_simple *_w)
        :w(_w) {}

    Writer(const Writer &) = delete;

    bool Write(const void *p, size_t size) {
        return was_simple_write(w, p, size);
    }

    bool Write(const char *s) {
        return was_simple_puts(w, s);
    }

    template<typename... Args>
    bool Format(const char *fmt, Args... args) {
        return was_simple_printf(w, fmt, std::forward<Args>(args)...);
    }
};

#endif
