#Author: persist

# Structure

## Logical Structure

vkernel
|--resource view (cgroup, namespace)
|--resource allocation (host kernel)
|--...

proxy
|--register/unregister (container, vkernel)
|--intercept/parse/transfer (container, vkernel)

## Physical Structure

vkernel
|--resource view update daemon
|--resource allocation daemon

proxy
|--register daemon
|--transfer daemon

## Resource Subsystem

system
|--memory
|--cpu
|--blkio
|--device

# Container Lifecycle

created 
	container registers container-vkernel in proxy.

running
	apps in container invoke syscall to request resource.
	proxy intercepts the request, then parse it and find specific vkernel.
	proxy transfers the request to specific vkernel.
	vkernel checks the legality of the request to decide dening it or allowing it by invoking syscall about hardware driver.
	vkernel updates resource usage.

destroy
	container unregisters container-vkernel in proxy.

# Key Points

## How to create proxy

## Communication between container and proxy

## Communication between proxy and vkernel

## Communication between vkernel and host 

# Knowledge

## How to create a daemon process

	fork	- run daemon in a child process.
	setsid	- start a new session, create a new process group and leave tty.
	chdir	- change current work directory.
	close 	- close unused file descriptors.

## How to intercept syscalls by hook

	LD_PRELOAD - use LD_PRELOAD to load your own functions to replace the old functions.

### memory syscalls

### cpu syscalls

### blkio syscalls

### device syscalls

