import re

stubsyscalls = open("syscalls.h").readlines()
kimplsyscalls = open("kernel.c").readlines()
syscallnums = open("syscall_numbers.h").readlines()

def uniqueAndRemoveEmpty(a):
    b = set(a)
    b.remove("")
    return list(b)

temp = []

for stubsyscall in stubsyscalls:
    a = re.match("^inline\ [_a-zA-z0-9]*\ (sys_[^\ (]*)[\ \t]*\(.*$", stubsyscall)
    if(a):
        temp.append(a.group(1))

stubsyscalls = temp[:]
temp = []

for syscallnum in syscallnums:
    a = re.match("^#define\ (SYSCALL_[^\ \t(]*)[\ \t]*\([0-9]*\).*$", syscallnum)
    if(a):
        temp.append(a.group(1))

syscallnums = temp[:]
temp = []

for kimplsyscall in kimplsyscalls:
    a = re.search("case\ (SYSCALL_[^\ :]*):", kimplsyscall)
    if(a):
        temp.append(a.group(1))

kimplsyscalls = temp[:]

implemented = set([x.lower().replace("syscall", "sys") for x in kimplsyscalls])
stubbed = set(stubsyscalls)
listed = set([x.lower().replace("syscall", "sys") for x in syscallnums])

listedNotStubbed = listed.difference(stubbed)
listedNotImplemented = listed.difference(implemented)
implementedNotStubbed = implemented.difference(stubbed)
stubbedNotImplemented = stubbed.difference(implemented)

if(len(listedNotStubbed)):
    print "WARNING: No stubs for the following listed system calls:"
    print "\n".join(list(listedNotStubbed))

if(len(listedNotImplemented)):
    print "WARNING: No kernel implementation for the following listed system calls:"
    print "\n".join(list(listedNotImplemented))

if(len(implementedNotStubbed)):
    print "WARNING: No stubs for the following implemented system calls:"
    print "\n".join(list(implementedNotStubbed))

if(len(stubbedNotImplemented)):
    print "WARNING: No kernel implementation for the following stubbed system calls:"
    print "\n".join(list(stubbedNotImplemented))


