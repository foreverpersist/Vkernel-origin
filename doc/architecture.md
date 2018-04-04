# Architecture

	Divide vkernel into 3 subsytems:

> * resource view
> * resource allocation
> * permission control


## Resource view

	Container resource can be divided into logical resource (such as: fd, kernel-module) and physical resource (such as: memory, cpu).

	                Resource View
	                      |
	           |---------------------|
	    Logical Resource     Physical Resource
	           |                     |
	    |--------------|      |--------------|
	namespace       others cgroup         others

### Logical Resource

	implemented by namespace and others

### Physical Resource

	implemented by cgroup and others 


## Resource Allocation

	Both logical resource and physical resource are surpposed to be allocated following some rules.

	             Resource Allocation
	                      |
	           |--------------------|
	   Physical Resource     Logical Resource
	           |                    |
	   |---------------|     |--------------|
	cgroup          others known         unknown

### Physical Resource Allocation

	implemented by cgroup and others

### Logical Resouce Allocation

	includes known resouces and unknown resouces, both of them are not implemented.


## Permission Control

	Root, Mount, SELinux and etc.

	       Permission Control
	               |
	|---------|---------|---------|
	Root    Mount    SELinux   others

### Root

	Isolate container root permission and host root permission.

### Mount

	Limit or prevent mounting sensitive directories.

### SELinux

	Compatible for SELinux for access controll.


### Others

	...