// 构建：gcc seccomp.c -lseccomp
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

    // 初始化全部允许为了黑名单模式
    ctx = seccomp_init(SCMP_ACT_ALLOW);
    if (ctx == NULL) {
        fprintf(stderr, "seccomp初始化失败😨\n");
        exit(EXIT_FAILURE);
    }

    int delete_syscalls[] = {
        SCMP_SYS(unlink), // 删除文件
        SCMP_SYS(unlinkat), // 删除文件泛类型
        SCMP_SYS(rmdir), // 删除目录
        // SCMP_SYS(remove), // 命令行程序调不到，而且这也无法加入secc...
        SCMP_SYS(rename), // 重命名（可能用于删除）
        SCMP_SYS(renameat), // 重命名泛类型
        SCMP_SYS(renameat2), // 重命名泛类型2
    };
    
    size_t delete_count = sizeof(delete_syscalls) / sizeof(delete_syscalls[0]);
    for (size_t i = 0; i < delete_count; i++) {
        if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), delete_syscalls[i], 0) != 0) {
            fprintf(stderr, "无法阻止“删除类”系统调用：%d，这很危险👿\n", delete_syscalls[i]);
            exit(EXIT_FAILURE);
        }
    }

    int create_syscalls[] = {
        SCMP_SYS(mknod), // 创建设备节点
        SCMP_SYS(mknodat), // 创建设备节点泛类型
    };
    
    size_t create_count = sizeof(create_syscalls) / sizeof(create_syscalls[0]);
    for (size_t i = 0; i < create_count; i++) {
        if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), create_syscalls[i], 0) != 0) {
            fprintf(stderr, "无法阻止“创建类”系统调用：%d，这很危险👿\n", create_syscalls[i]);
            exit(EXIT_FAILURE);
        }
    }
    

    int modify_syscalls[] = {
        // SCMP_SYS(blockdev), // 块设备读写开关，无这个类
        SCMP_SYS(chmod), // 修改权限
        SCMP_SYS(chown), // 修改所有者
        // SCMP_SYS(chattr), // 无这个类，懒得实现（true）
        SCMP_SYS(utime), // 修改时间
        SCMP_SYS(utimes), // 修改时间
        SCMP_SYS(utimensat), // 修改纳秒时间
    };
    
    size_t modify_count = sizeof(modify_syscalls) / sizeof(modify_syscalls[0]);
    for (size_t i = 0; i < modify_count; i++) {
        if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), modify_syscalls[i], 0) != 0) {
            fprintf(stderr, "无法阻止“修改类”系统调用：%d，这很危险👿\n", modify_syscalls[i]);
            exit(EXIT_FAILURE);
        }
    }

    // int write_syscalls[] = {
        // SCMP_SYS(write), // 基础写入
        // SCMP_SYS(pwrite64), // 定位写入
        // SCMP_SYS(writev), // 向量写入
        // SCMP_SYS(pwritev), // 定位向量写入
        // // 移除了 pwritev2，保持兼容性
        // SCMP_SYS(sendfile), // 文件发送
        // SCMP_SYS(copy_file_range), // 文件范围拷贝
    // };
    
    // size_t write_count = sizeof(write_syscalls) / sizeof(write_syscalls[0]);
    // for (size_t i = 0; i < write_count; i++) {
        // // 只拦截文件描述符 > 2 的情况（排除stdin/stdout/stderr避免炸交互）
        // if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), write_syscalls[i], 1, SCMP_CMP(0, SCMP_CMP_GT, 2)) != 0) {
            // fprintf(stderr, "无法阻止“写入类”系统调用：%d，这很危险👿\n", write_syscalls[i]);
            // exit(EXIT_FAILURE);
        // }
    // }

    // int write_flags = O_WRONLY | O_RDWR | O_CREAT | O_TRUNC;
    // if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), SCMP_SYS(open), 1, SCMP_CMP(1, SCMP_CMP_MASKED_EQ, write_flags, write_flags)) != 0) {
        // fprintf(stderr, "有条件阻止但打开失败？\n");
        // exit(EXIT_FAILURE);
    // }
    
    // if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), SCMP_SYS(openat), 1, SCMP_CMP(2, SCMP_CMP_MASKED_EQ, write_flags, write_flags)) != 0) {
        // fprintf(stderr, "无法阻止openat调用😨\n");
        // exit(EXIT_FAILURE);
    // }
    
    // 低于5.1内核无法精细处理，索性全ban了，里面包含chattr、blockdev类命令失效的实现
    // 终端驱动会加载失败（readline / bash）根本活不了，表现就是：PS1 prompt出不来，方向键、Tab 补全失效，但无伤大雅
    if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EPERM), SCMP_SYS(ioctl), 0) != 0) {
        fprintf(stderr, "无法ban掉inctl类😢\n");
        exit(EXIT_FAILURE);
    }


    if (seccomp_load(ctx) != 0) {
        fprintf(stderr, "seccomp加载失败😢\n");
        exit(55);
    } else {
        fprintf(stdout, "seccomp加载成功😋\n");
    }
    
    seccomp_release(ctx);
}

int main(int argc, char *argv[]) {

    int status = system("for b in /dev/block/sd* /dev/block/mmcblk* /dev/sd* /dev/mmcblk* /dev/vd*; do if blockdev --setro \"$b\"; then echo \"设置设备：$b，只读属性成功\"; else echo \"设置设备：$b，只读属性失败😨\"; fi; done");
    if (status != 0) {
        return(EXIT_FAILURE);
    }

    setup_seccomp_filter();

    if (argc < 2) {
        system("exec /bin/sh");
    } else {
        execvp(argv[1], &argv[1]);
        perror("执行失败🥴");
    }

    return(EXIT_FAILURE);
}
