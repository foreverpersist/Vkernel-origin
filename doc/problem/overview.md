# Overview

	容器技术目前存在着许多隔离性问题,或者说是安全问题.

## /proc, /sys未隔离

	容器看到的资源视图不正确,如top,free,iostat等指令

## 内核模块

	容器可能需要特别的内核模块 - intercept modprobe & resource view?

## 软资源

	软资源,如pid, inode, fd等没有隔离,如fork-bomb恶意服务 - intercept syscalls?

## 存储隔离

	容器无限写文件,可能会耗尽存储资源 - intercept syscalls & storage account?

## 设备隔离

	存在设备在多个容器中使用的情况, /dev设备需要隔离 - dev namespace?

## 网络隔离

	运行在host网络模式的容器可能会控制主机及其他容器的网络 - intercept syscalls?

## root权限

	容器内root权限对主机及其他容器造成安全威胁 - intercept syscalls & cap permissions?

## Mount安全

	容器随意挂载敏感文件/目录,可能会影响主机及其他容器 - Apparmor?

## SELinux -> Seccomp

	如何将SELinux -> Seccomp用于容器隔离 - seccomp & ptrace & tracer?

## 镜像安全

	有些镜像可能包含恶意软件,或使用有漏洞的软件 - verify & auto-update notify?
