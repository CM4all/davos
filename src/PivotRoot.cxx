/*
 * author: Max Kellermann <mk@cm4all.com>
 */

#include "PivotRoot.hxx"
#include "system/BindMount.hxx"
#include "system/pivot_root.h"
#include "system/Error.hxx"

#include <sched.h>
#include <unistd.h>
#include <sys/mount.h>

#ifndef __linux
#error This library requires Linux
#endif

void
PivotRoot(const char *new_root, const char *put_old)
{
    constexpr int flags = CLONE_NEWUSER|CLONE_NEWNS;
    if (unshare(flags) < 0)
        throw FormatErrno("unshare(0x%x) failed", flags);

    /* convert all "shared" mounts to "private" mounts */
    mount(nullptr, "/", nullptr, MS_PRIVATE|MS_REC, nullptr);

    /* first bind-mount the new root onto itself to "unlock" the
       kernel's mount object (flag MNT_LOCKED) in our namespace;
       without this, the kernel would not allow an unprivileged
       process to pivot_root to it */
    BindMount(new_root, new_root, MS_NOSUID|MS_NOEXEC|MS_NODEV);

    /* release a reference to the old root */
    if (chdir(new_root) < 0)
        throw FormatErrno("chdir('%s') failed", new_root);

    /* enter the new root */
    if (my_pivot_root(new_root, put_old) < 0)
        throw FormatErrno("pivot_root('%s', '%s') failed", new_root, put_old);

    /* get rid of the old root */
    if (umount2(put_old, MNT_DETACH) < 0)
        throw FormatErrno("umount('%s') failed", put_old);
}
