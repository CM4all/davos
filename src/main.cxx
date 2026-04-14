// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

#include "frontend.hxx"
#include "PlainBackend.hxx"
#include "PivotRoot.hxx"
#include "IsolatePath.hxx"
#include "util/PrintException.hxx"

#include <cerrno>
#include <stdexcept>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

static void
MaybePivotRoot()
{
	const char *new_root = getenv("DAVOS_PIVOT_ROOT");
	if (new_root == nullptr)
		return;

	const char *put_old = getenv("DAVOS_PIVOT_ROOT_OLD");
	if (put_old == nullptr)
		throw std::runtime_error("DAVOS_PIVOT_ROOT_OLD missing");

	if (*put_old == '/' || *put_old == 0)
		throw std::runtime_error("DAVOS_PIVOT_ROOT_OLD must be a relative path");

	PivotRoot(new_root, put_old);
}

static void
MaybeIsolatePath()
{
	const char *path = getenv("DAVOS_ISOLATE_PATH");
	if (path == nullptr)
		return;

	if (*path != '/')
		throw std::runtime_error("DAVOS_ISOLATE_PATH must be an absolute path");

	IsolatePath(path);
}

int
main(int, const char *const*) noexcept
try {
	MaybePivotRoot();
	MaybeIsolatePath();

	PlainBackend backend;
	run(backend);
	return EXIT_SUCCESS;
} catch (...) {
	PrintException(std::current_exception());
	return EXIT_FAILURE;
}
