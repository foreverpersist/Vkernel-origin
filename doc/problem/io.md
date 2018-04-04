# IO Isolation

## Motivation

容器现有的io隔离是通过cgroup设置带宽大小,借助cfq实现的带宽限制.

容器是否有必要监管其内部进程的io带宽? 如容器内进程的io带宽限制

## Point

考虑容器内的io管理