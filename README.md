# Seccomp 安全沙箱使用于各种Linux、Android
```
使用 seccomp 来限制系统调用并防止对系统进行有害操作的 Linux 安全沙箱实现。

## 🛡️ 特点

### 安全保护
- **文件删除保护**：阻止与文件删除相关的系统调用，例如“unlink”、“unlinkat”、“rmdir”、“rename”
- **文件创建保护**：防止通过“mknod”、“mknodat”创建设备节点
- **文件属性保护**：限制文件修改系统调用，包括`chmod`、`chown`、`utime`
- **IO 控制保护**：完全阻止 `ioctl` 系统调用

### 存储保护
- 自动将所有块设备设置为只读模式（`/dev/block/sd*`、`/dev/block/mmcblk*`、...）
- 防止对存储设备进行写操作

## 🚀 快速入门

### 先决条件
- GCC编译器
- libseccomp 开发库
```

### 安装
```bash
# 克隆存储库
git clone <存储库-url>
cd seccomp-沙箱

# 编译程序
gcc seccomp.c -lseccomp -o seccomp_sandbox
```

### 用法
启动交互式shell
```bash
./seccomp_sandbox
```

运行特定命令
```bash
./seccomp_sandbox <命令> [参数...]
```

### 示例：
```bash
./seccomp_sandbox ls -la
./seccomp_sandbox python3 script.py
./seccomp_sandbox bash -c "your_script.sh"
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
· 非常适合运行不受信任代码的安全沙箱场景

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
    system(“./seccomp_sandbox your_app”)；
    return 0；
}
```

### 🔍 调试，如果您遇到问题：

1. 检查您是否拥有所需的权限：
```bash
sudo ./seccomp_sandbox
```

2. 验证 libseccomp 是否已安装：
```bash
ldconfig -p | ldconfig -p | grep libseccomp
```

3. 先用简单的命令测试一下：
```bash
./seccomp_sandbox chmod +x seccomp_sandbox
chmod: 'seccomp_sandbox': 不允许的操作
```

### 🤝 贡献
欢迎贡献！请随时提交拉取请求或针对错误和功能请求提出问题。

### 📄 许可证
该项目是开源的，可根据 MIT 许可证使用。
