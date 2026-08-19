# 推荐的目录组织

建议按下面的方式组织移植层文件：

```text
libcpu/<ISA>/port_<compiler>.c
libcpu/<ISA>/port_<compiler>.s
```

例如：

- `libcpu/CM3/port_gcc.c`
- `libcpu/CM3/port_gcc.s`

如果你要移植到 Cortex-M4 + GCC，可以参考：

```text
libcpu/CM4/port_gcc.c
libcpu/CM4/port_gcc.s
```

如果你要移植到 Cortex-M3 + ARMCC，也可以参考：

```text
libcpu/CM3/port_armcc.c
libcpu/CM3/port_armcc.s
```
