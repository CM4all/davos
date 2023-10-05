/*
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "IsolatePath.hxx"
#include "lib/fmt/SystemError.hxx"
#include "spawn/UserNamespace.hxx"
#include "system/Mount.hxx"
#include "system/pivot_root.h"
#include "io/FileDescriptor.hxx"
#include "util/ScopeExit.hxx"

#include <assert.h>
#include <sched.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>

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
		throw FmtErrno("chdir('{}') failed", path);
}

static void
PivotRootOrThrow(const char *new_root, const char *put_old)
{
	if (my_pivot_root(new_root, put_old) < 0)
		throw FmtErrno("pivot_root('{}', '{}') failed", new_root, put_old);
}

static void
MakeDirs(const char *path)
{
	assert(path != nullptr);
	assert(*path == '/');

	++path; /* skip the slash to make it relative */

	char *allocated = strdup(path);
	AtScopeExit(allocated) { free(allocated); };

	char *p = allocated;
	while (char *slash = strchr(p, '/')) {
		*slash = 0;
		mkdir(allocated, 0755);
		*slash = '/';
		p = slash + 1;
	}

	mkdir(allocated, 0700);
}

void
IsolatePath(const char *path)
{
	assert(path != nullptr);
	assert(*path == '/');

	const int uid = geteuid(), gid = getegid();

	UnshareOrThrow(CLONE_NEWUSER|CLONE_NEWNS);

	DenySetGroups(0);
	SetupGidMap(0, gid, false);
	SetupUidMap(0, uid, false);

	/* convert all "shared" mounts to "private" mounts */
	MountSetAttr(FileDescriptor::Undefined(), "/",
		     AT_RECURSIVE|AT_SYMLINK_NOFOLLOW|AT_NO_AUTOMOUNT,
		     0, 0, MS_PRIVATE);

	/* create an empty tmpfs as the new filesystem root */
	const char *const new_root = "/tmp";
	MountOrThrow("none", new_root, "tmpfs", MS_NODEV|MS_NOEXEC|MS_NOSUID,
		     "size=256k,nr_inodes=1024,mode=755");

	/* first bind-mount the new root onto itself to "unlock" the
	   kernel's mount object (flag MNT_LOCKED) in our namespace;
	   without this, the kernel would not allow an unprivileged
	   process to pivot_root to it */
	BindMount(new_root, new_root);

	/* release a reference to the old root */
	ChdirOrThrow(new_root);

	const char *const put_old = "/old";
	mkdir(put_old + 1, 0700);

	MakeDirs(path);

	/* make the new root tmpfs read-only */
	MountSetAttr(FileDescriptor::Undefined(), new_root,
		     AT_SYMLINK_NOFOLLOW|AT_NO_AUTOMOUNT,
		     MOUNT_ATTR_RDONLY,
		     0);

	BindMount(path, path + 1);
	MountSetAttr(FileDescriptor{AT_FDCWD}, path + 1,
		     AT_SYMLINK_NOFOLLOW|AT_NO_AUTOMOUNT,
		     MOUNT_ATTR_NOSUID|MOUNT_ATTR_NOEXEC|MOUNT_ATTR_NODEV,
		     0);

	/* enter the new root */
	PivotRootOrThrow(new_root, put_old + 1);

	/* get rid of the old root */
	Umount(put_old, MNT_DETACH);

	rmdir(put_old);

	MountSetAttr(FileDescriptor::Undefined(), "/",
		     AT_SYMLINK_NOFOLLOW|AT_NO_AUTOMOUNT,
		     MOUNT_ATTR_NOSUID|MOUNT_ATTR_NOEXEC|MOUNT_ATTR_NODEV|MOUNT_ATTR_RDONLY,
		     0);
}
