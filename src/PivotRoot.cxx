// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <max.kellermann@ionos.com>

#include "PivotRoot.hxx"
#include "lib/fmt/SystemError.hxx"
#include "system/Mount.hxx"
#include "system/linux/pivot_root.h"
#include "io/FileDescriptor.hxx"

#include <sched.h>
#include <unistd.h>
#include <sys/mount.h>

#ifndef __linux
#error This library requires Linux
#endif

static void
UnshareOrThrow(int flags)
{
	if (unshare(flags) < 0)
		throw FmtErrno("unshare({:#x}) failed", flags);
}

static void
ChdirOrThrow(const char *path)
{
	if (chdir(path) < 0)
		throw FmtErrno("chdir({:?}) failed", path);
}

static void
PivotRootOrThrow(const char *new_root, const char *put_old)
{
	if (my_pivot_root(new_root, put_old) < 0)
		throw FmtErrno("pivot_root({:?}, {:?}) failed", new_root, put_old);
}

void
PivotRoot(const char *new_root, const char *put_old)
{
	UnshareOrThrow(CLONE_NEWUSER|CLONE_NEWNS);

	/* convert all "shared" mounts to "private" mounts */
	MountSetAttr(FileDescriptor::Undefined(), "/",
		     AT_RECURSIVE|AT_SYMLINK_NOFOLLOW|AT_NO_AUTOMOUNT,
		     0, 0, MS_PRIVATE);

	/* first bind-mount the new root onto itself to "unlock" the
	   kernel's mount object (flag MNT_LOCKED) in our namespace;
	   without this, the kernel would not allow an unprivileged
	   process to pivot_root to it */
	BindMount(new_root, new_root);
	MountSetAttr(FileDescriptor::Undefined(), new_root,
		     AT_SYMLINK_NOFOLLOW|AT_NO_AUTOMOUNT,
		     MOUNT_ATTR_NOSUID|MOUNT_ATTR_NOEXEC|MOUNT_ATTR_NODEV,
		     0);

	/* release a reference to the old root */
	ChdirOrThrow(new_root);

	/* enter the new root */
	PivotRootOrThrow(new_root, put_old);

	/* get rid of the old root */
	Umount(put_old, MNT_DETACH);
}
