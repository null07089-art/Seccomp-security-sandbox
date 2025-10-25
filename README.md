```markdown
# Seccomp security sandbox is used in various Linux and Android

A Linux security sandbox implementation that uses seccomp to restrict system calls and prevent harmful operations on the system.

## 🛡️ Features

### Security Protection
- **File Deletion Protection**: Blocks system calls related to file deletion, such as "unlink", "unlinkat", "rmdir", "rename"
- **File Creation Protection**: Prevent device nodes from being created via "mknod", "mknodat"
- **File Attribute Protection**: Restrict file modification system calls, including `chmod`, `chown`, `utime`
- **IO Control Protection**: Completely block `ioctl` system calls

### Storage Protection
- Automatically set all block devices to read-only mode (`/dev/block/sd*`, `/dev/block/mmcblk*`, ...)
- Prevent writes to storage devices

## 🚀 Quick Start

### Prerequisites
- GCC compiler
- libseccomp development library
```

### Install
```bash
# Clone the repository
git clone <repository-url>
cd seccomp-sandbox

# Compiler
gcc seccomp.c -lseccomp -o seccomp_sandbox
```

### Usage
# Start interactive shell
```bash
./seccomp_sandbox
```

#Run specific commands
```bash
./seccomp_sandbox <command> [parameters...]
```

### Example:
```bash
./seccomp_sandbox ls -la
./seccomp_sandbox python3 script.py
./seccomp_sandbox bash -c "your_script.sh"
```

### 🔧 Technical details
```
blocked system call

Category System Call Error Code
Delete operations unlink, unlinkat, rmdir, rename, renameat, renameat2 EPERM
Create operations mknod, mknodat EPERM
Modification operations chmod, chown, utime, utimes, utimensat EPERM
IO control ioctl EPERM

Workflow

1. Device protection: Set all block devices to read-only mode on startup
2. Filter initialization: Create a seccomp context with the default allow all policy
3. Rule addition: Add blocking rules for specific system calls
4. Filter Loading: Load the configured filter into the kernel
5. Command execution: execute the target command or start a shell in a protected environment

⚠️Important Tips

· ioctl is completely blocked, which may cause unexpected behavior in programs that rely on terminal functionality
· Standard I/O operations (file descriptors 0-2) are not affected
· Requires sufficient permissions to set a block device to read-only
· Ideal for secure sandbox scenarios where untrusted code runs

🐛 Error handling

The program will exit with an error message if:

· Seccomp initialization failed
· Failed to add system call rules
· Seccomp filter failed to load
· Block device read-only setting failed
```

###📝 Code Example

```c
//Basic usage in code
#include "seccomp.h" // Remove the main function

int main() {
    setup_seccomp_filter();
    // and execute your commands safely
    system(“./seccomp_sandbox your_app”);
    return 0;
}
```

###🔍 Debugging, if you have problems:

# 1. Check that you have the required permissions:
```bash
sudo ./seccomp_sandbox
```

# 2. Verify that libseccomp is installed:
```bash
ldconfig -p | ldconfig -p | grep libseccomp
```

# 3. Test it with a simple command first:
```bash
./seccomp_sandbox chmod +x seccomp_sandbox
chmod: 'seccomp_sandbox': not allowed operation
```

### 🤝 Contribute
# Contributions welcome! Please feel free to submit pull requests or raise issues for bugs and feature requests.

### 📄 License
# This project is open source and available under the MIT license.
