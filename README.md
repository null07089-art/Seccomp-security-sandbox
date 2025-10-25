# Seccomp安全执行器适用于各种Linux发行版、Android&Root
```
功能：使用 seccomp 来限制系统调用并防止对系统进行有害操作的 Linux 安全执行实现。

# 🛡️数据保护
    - **文件删除保护**：阻止与文件删除相关的系统调用，例如“unlink”、“unlinkat”、“rmdir”、“rename”
    - **文件创建保护**：防止通过“mknod”、“mknodat”创建设备节点
    - **文件属性保护**：限制文件修改系统调用，包括`chmod`、`chown`、`utime`
    - **IO 控制保护**：完全阻止 `ioctl` 系统调用

# 🛡️存储保护
    - 自动将所有块设备设置为只读模式（`/dev/block/sd*`、`/dev/block/mmcblk*`、...）
    - 防止对存储设备进行写操作
```

### 安装&构建
```bash
    # 安装 libseccomp 开发库
    # 在 Linux 上安装 libseccomp 开发库 的方法取决于你使用的发行版。以下是主流系统的安装方式：

    # Debian/Ubuntu
    sudo apt-get update
    sudo apt-get install libseccomp-dev

    # CentOS / RHEL / openEuler
    sudo yum install libseccomp-devel

    # Fedora
    sudo dnf install libseccomp-devel

    # 或者下载源码编译
    wget https://github.com/seccomp/libseccomp/releases/download/v2.5.5/libseccomp-2.5.5.tar.gz
    tar -xvf libseccomp-2.5.5.tar.gz
    cd libseccomp-2.5.5
    ./configure
    make
    sudo make install
    sudo ldconfig

    # 编译程序
    gcc main.c -lseccomp
```

### 用法
启动交互式shell
```bash
./a.out
```

运行特定命令
```bash
./a.out <命令> [参数...]
```

### 🔧 技术细节
```
被阻止的系统调用

类别 系统调用 错误代码
删除操作 unlink、unlinkat、rmdir、rename、renameat、renameat2 EPERM
创建操作 mknod、mknodat EPERM
修改操作 chmod、chown、utime、utimes、utimensat EPERM
IO 控制 ioctl EPERM

工作流程

1. 设备保护：启动时将所有块设备设置为只读模式
2. 过滤器初始化：使用默认的允许所有策略创建 seccomp 上下文
3.规则添加：添加特定系统调用的阻止规则
4. Filter Loading：将配置好的filter加载到内核中
5.命令执行：在受保护的环境中执行目标命令或启动shell

⚠️重要提示

· ioctl 被完全阻止，这可能会导致依赖终端功能的程序出现意外行为
· 标准 I/O 操作（文件描述符 0-2）不受影响
· 需要足够的权限才能将块设备设置为只读
· 非常适合运行不受信任代码的安全执行场景

🐛 错误处理

在以下情况下，程序将退出并显示错误消息：

· Seccomp初始化失败
· 系统调用规则添加失败
· Seccomp过滤器加载失败
· 块设备只读设置失败
```

### 📝 代码示例

```c
// 代码中的基本用法
#include“seccomp.h” // 去掉main函数

int main() {
    setup_seccomp_filter();
    // 并安全地执行你的命令
    system(“./a.out <argv>”)；
    return 0；
}
```

### 🔍 调试，如果您遇到问题：

1. 检查您是否拥有所需的权限：
```bash
sudo ./a.out
```

2. 验证 libseccomp 是否已安装：
```bash
ldconfig -p | ldconfig -p | grep libseccomp
```

3. 先用简单的命令测试一下：
```bash
./a.out chmod -x a.out
chmod: 'a.out': 不允许的操作
```

### 🤝 贡献
欢迎贡献！请随时提交拉取请求或针对错误和功能请求提出问题。

### 📄 许可证
该项目是开源的，可根据 MIT 许可证使用。
