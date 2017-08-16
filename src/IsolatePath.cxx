/*
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "IsolatePath.hxx"
#include "system/BindMount.hxx"
#include "system/pivot_root.h"
#include "system/Error.hxx"
#include "io/WriteFile.hxx"
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
        throw FormatErrno("unshare(0x%x) failed", flags);
}

static void
ChdirOrThrow(const char *path)
{
    if (chdir(path) < 0)
        throw FormatErrno("chdir('%s') failed", path);
}

static void
PivotRootOrThrow(const char *new_root, const char *put_old)
{
    if (my_pivot_root(new_root, put_old) < 0)
        throw FormatErrno("pivot_root('%s', '%s') failed", new_root, put_old);
}

static void
LazyUmountOrThrow(const char *path)
{
    if (umount2(path, MNT_DETACH) < 0)
        throw FormatErrno("umount('%s') failed", path);
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

static void
write_file(const char *path, const char *data)
{
    if (TryWriteExistingFile(path, data) == WriteFileResult::ERROR)
        throw FormatErrno("write('%s') failed", path);
}

static void
setup_uid_map(int uid)
{
    char buffer[64];
    sprintf(buffer, "%d %d 1", uid, uid);
    write_file("/proc/self/uid_map", buffer);
}

static void
setup_gid_map(int gid)
{
    char buffer[64];
    sprintf(buffer, "%d %d 1", gid, gid);
    write_file("/proc/self/gid_map", buffer);
}

/**
 * Write "deny" to /proc/self/setgroups which is necessary for
 * unprivileged processes to set up a gid_map.  See Linux commits
 * 9cc4651 and 66d2f33 for details.
 */
static void
deny_setgroups()
{
    TryWriteExistingFile("/proc/self/setgroups", "deny");
}

void
IsolatePath(const char *path)
{
    assert(path != nullptr);
    assert(*path == '/');

    const int uid = geteuid(), gid = getegid();

    UnshareOrThrow(CLONE_NEWUSER|CLONE_NEWNS);

    deny_setgroups();
    setup_gid_map(gid);
    setup_uid_map(uid);

    /* convert all "shared" mounts to "private" mounts */
    mount(nullptr, "/", nullptr, MS_PRIVATE|MS_REC, nullptr);

    /* create an empty tmpfs as the new filesystem root */
    const char *const new_root = "/tmp";
    if (mount("none", new_root, "tmpfs", MS_NODEV|MS_NOEXEC|MS_NOSUID,
              "size=256k,nr_inodes=1024,mode=755") < 0)
        throw FormatErrno("mount(tmpfs, '%s') failed", new_root);

    /* first bind-mount the new root onto itself to "unlock" the
       kernel's mount object (flag MNT_LOCKED) in our namespace;
       without this, the kernel would not allow an unprivileged
       process to pivot_root to it */
    BindMount(new_root, new_root, MS_NOSUID|MS_NOEXEC|MS_NODEV);

    /* release a reference to the old root */
    ChdirOrThrow(new_root);

    const char *const put_old = "/old";
    mkdir(put_old + 1, 0700);

    MakeDirs(path);
    BindMount(path, path + 1, MS_NOSUID|MS_NOEXEC|MS_NODEV);

    /* enter the new root */
    PivotRootOrThrow(new_root, put_old + 1);

    /* get rid of the old root */
    LazyUmountOrThrow(put_old);

    rmdir(put_old);

    if (mount(nullptr, "/", nullptr,
              MS_REMOUNT|MS_BIND|MS_NODEV|MS_NOEXEC|MS_NOSUID|MS_RDONLY,
              nullptr) < 0)
        throw FormatErrno("Failed to remount read-only");
}
