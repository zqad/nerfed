nerfed
======

Purpose
-------
Some archiving applications (tar) lacks options for not performing privileged
operations (e.g. mknod), and to enforce archive permissions (chmod) when
unpacking an archive. In the case where you are looking to automating unpacking
and would like to avoid enforcing some calls due to looking at OS archives, or
if you want to automate extracting content of broken archives (containing e.g.
folders with the wrong mode), you can just nerf these libc functions: Make them
happily return success without action.

The features and inner workings are similar to fakeroot, but where fakeroot
takes care to serve a consistent view of anything written to the file system,
nerfed will just throw this information away.

Variants
--------
nerfed comes in three flavors:
- `nerfed-embedded`: Embeds the library within the executable and writes it
  to `/tmp` before using it for `LD_PRELOAD`.
- `nerfed-separate`: Expects to find the library on a static location in the
  file system, typically `/usr/libexec/nerfed/libeat-function.so`. This path
  can be changed using the `PREFIX` or `PRELOAD_SO_PATH` arguments to `make`.
- `libeat-function.so`: Library used by the above commands, but with a stable
  interface that can be used without the helper executables.

Usage
-----
To use with e.g. `tar` and ignore mknod and chmod:
```
nerfed --chmod --mknod tar -zxf archive.tar.gz
```

To accomplish the same action using the library directly:
```
env LD_PRELOAD=/path/to/libeat-function.so LIBEAT_FUNCTION_CHMOD=1 LIBEAT_FUNCTION_MKNOD=1 tar -zxf archive.tar.gz
```
