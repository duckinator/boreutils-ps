#//\
    rm -f ps; clang -std=c11 -Wall -Weverything -Wno-comment -DDEFAULT_SOURCE main.c -lkvm -o ps && ./ps && exit $?

#include <fcntl.h>
#include <kvm.h>
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/user.h>
#include <unistd.h>

// I can't find any docs, so here's info on struct kinfo_proc:
// https://github.com/freebsd/freebsd/blob/master/sys/sys/user.h#L121

static int display(struct kinfo_proc *kp, int nentries) {
    puts("USER \tPID \tVSZ \tRSS \tCOMMAND");
    for (int i = 0; i < nentries; i++) {
        struct kinfo_proc *proc = kp + i;

        // Empty ki_login seems to mean it's a kernel process.
        if (strlen(proc->ki_login) == 0) { continue; }

        // ki_rssize doesn't match with what FreeBSD `ps` prints, but
        // also seems to be the correct thing, so I have no idea what's up.
        printf("%s \t%4d \t%lu \t%ld \t%s\n",
                proc->ki_login,
                proc->ki_pid,
                proc->ki_size / 1000,
                proc->ki_rssize,
                proc->ki_comm);
    }
    return 0;
}

int main(int argc, char **argv) {
    (void)argc; // we only use argv[0], so mark this as unused.

    kvm_t *kd = kvm_open(NULL, "/dev/null", NULL, O_RDONLY, argv[0]);
    if (kd == NULL) {
        // Printing the error is handled by kvm_open()
        return 1;
    }

    int nentries = 0; // Holds the number of processes returned.
    struct kinfo_proc *kp = kvm_getprocs(kd, KERN_PROC_PROC, 0, &nentries);

    if (kp == NULL) {
        fputs("ps: failed to get process information.\n", stderr);
        return 1;
    }

    display(kp, nentries);

    return 0;
}
