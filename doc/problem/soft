# Soft Resource Isolation

## Notion

软资源范畴
	进程	pid
	文件	fd, inode
	网络 socket
	内存
	设备

## Motivation

容器内软资源若无限制,将导致主机或其他容器无法正常创建资源, 如fork-bomb

## Point

限制每个容器可用软资源的上限,统计大多数容器的使用情况,结合主机的配置设置默认值,允许调整

