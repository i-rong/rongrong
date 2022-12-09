.data
msg:
    .ascii "Hello world\n"
    len = .-msg  #"."表示当前地址
.text      # 代码段
.global _start  #汇编程序的入口 如同C的main函数
_start:
    movl $len, %edx
    movl $msg, %ecx
    movl $1, %ebx
    movl $4, %eax
    int $0x80

    movl $0, %ebx
    movl $1, %eax
    int $0x80
