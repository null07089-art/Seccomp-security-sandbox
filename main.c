// build: gcc seccomp.c -lseccomp
#define _GNU_SOURCE
#include <seccomp.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

void setup_seccomp_filter() {
    scmp_filter_ctx ctx;

    // Initialize all allowed for blacklist mode
    ctx = seccomp_init(SCMP_ACT_ALLOW);
    if (ctx == NULL) {
        fprintf(stderr, "seccomp initialization failed😨\n");
        exit(EXIT_FAILURE);
    }

    int delete_syscalls[] = {
        SCMP_SYS(unlink), // Delete files
        SCMP_SYS(unlinkat), // Delete file generic type
        SCMP_SYS(rmdir), // Delete directory
        // SCMP_SYS(remove), // The command line program cannot be adjusted, and it cannot be added to secc...
        SCMP_SYS(rename), // Rename (may be used for deletion)
        SCMP_SYS(renameat), // Rename generic type
        SCMP_SYS(renameat2), // Rename generic type 2
    };
    
    size_t delete_count = sizeof(delete_syscalls) / sizeof(delete_syscalls[0]);
    for (size_t i = 0; i < delete_count; i++) {
        if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), delete_syscalls[i], 0) != 0) {
            fprintf(stderr, "Cannot prevent "delete class" system call: %d, this is dangerous👿\n", delete_syscalls[i]);
            exit(EXIT_FAILURE);
        }
    }

    int create_syscalls[] = {
        SCMP_SYS(mknod), // Create device node
        SCMP_SYS(mknodat), //Create device node generic type
    };
    
    size_t create_count = sizeof(create_syscalls) / sizeof(create_syscalls[0]);
    for (size_t i = 0; i < create_count; i++) {
        if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), create_syscalls[i], 0) != 0) {
            fprintf(stderr, "Cannot prevent "Create class" system call: %d, this is dangerous👿\n", create_syscalls[i]);
            exit(EXIT_FAILURE);
        }
    }
    

    int modify_syscalls[] = {
        // SCMP_SYS(blockdev), // Block device read and write switch, no such class
        SCMP_SYS(chmod), // Modify permissions
        SCMP_SYS(chown), //Modify owner
        // SCMP_SYS(chattr), // No such class, too lazy to implement (true)
        SCMP_SYS(utime), //Modification time
        SCMP_SYS(utimes), //Modification time
        SCMP_SYS(utimensat), // Modify nanosecond time
    };
    
    size_t modify_count = sizeof(modify_syscalls) / sizeof(modify_syscalls[0]);
    for (size_t i = 0; i < modify_count; i++) {
        if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), modify_syscalls[i], 0) != 0) {
            fprintf(stderr, "Cannot prevent "modify class" system call: %d, this is dangerous👿\n", modify_syscalls[i]);
            exit(EXIT_FAILURE);
        }
    }

    // int write_syscalls[] = {
        // SCMP_SYS(write), // Basic writing
        // SCMP_SYS(pwrite64), // Positioned write
        // SCMP_SYS(writev), // Vector write
        // SCMP_SYS(pwritev), // Positioning vector write
        // // Removed pwritev2 to maintain compatibility
        // SCMP_SYS(sendfile), // File sending
        // SCMP_SYS(copy_file_range), // File range copy
    // };
    
    // size_t write_count = sizeof(write_syscalls) / sizeof(write_syscalls[0]);
    // for (size_t i = 0; i < write_count; i++) {
        // // Only intercept file descriptors > 2 (exclude stdin/stdout/stderr to avoid exploding interactions)
        // if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), write_syscalls[i], 1, SCMP_CMP(0, SCMP_CMP_GT, 2)) != 0) {
            // fprintf(stderr, "Unable to block "write class" system call: %d, this is dangerous👿\n", write_syscalls[i]);
            //exit(EXIT_FAILURE);
        // }
    // }

    // int write_flags = O_WRONLY | O_RDWR | O_CREAT | O_TRUNC;
    // if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), SCMP_SYS(open), 1, SCMP_CMP(1, SCMP_CMP_MASKED_EQ, write_flags, write_flags)) != 0) {
        // fprintf(stderr, "Conditional blocking but failed to open?\n");
        //exit(EXIT_FAILURE);
    // }
    
    // if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), SCMP_SYS(openat), 1, SCMP_CMP(2, SCMP_CMP_MASKED_EQ, write_flags, write_flags)) != 0) {
        // fprintf(stderr, "Cannot prevent openat call😨\n");
        //exit(EXIT_FAILURE);
    // }
    
    // Kernels below 5.1 cannot handle it in detail and are simply banned. It contains the implementation of invalid commands such as chattr and blockdev.
    // The terminal driver will fail to load (readline/bash) and cannot survive at all. The performance is: the PS1 prompt cannot be displayed, the arrow keys and Tab completion are invalid, but it is harmless.
    if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), SCMP_SYS(ioctl), 0) != 0) {
        fprintf(stderr, "Cannot ban inctl class😢\n");
        exit(EXIT_FAILURE);
    }


    if (seccomp_load(ctx) != 0) {
        fprintf(stderr, "seccomp loading failed😢\n");
        exit(55);
    } else {
        fprintf(stdout, "seccomp loaded successfully😋\n");
    }
    
    seccomp_release(ctx);
}

int main(int argc, char *argv[]) {

    int status = system("for b in /dev/block/sd* /dev/block/mmcblk* /dev/sd* /dev/mmcblk* /dev/vd*; do if blockdev --setro \"$b\"; then echo \"Set device: $b, read-only attribute successful\"; else echo \"Set device: $b, read-only attribute failed😨\"; fi; done");
    if (status != 0) {
        return(EXIT_FAILURE);
    }

    setup_seccomp_filter();

    if (argc < 2) {
        system("exec /bin/sh");
    } else {
        execvp(argv[1], &argv[1]);
        perror("Execution failed🥴");
    }

    return(EXIT_FAILURE);
}
