#include <unistd.h>
#include <stdio.h>
#include <string.h>
int main(int argc, char** argv) {
    char hello[1024];
    int len = sizeof(hello);
    long ret;

    printf("使用 'syscall' 呼叫system call\n");
    __asm__ volatile ( 
        "mov $0, %%rax\n"   //system call number:sys_read
        "mov $0, %%rdi\n"   //stdin
        "mov %1, %%rsi\n"   //buffer
        "mov %2, %%rdx\n"   //max_buffer_size
        "syscall\n"
        "mov %%rax, %0"     //save return to %0
        :  "=m"(ret)
        : "g" (hello), "g" (len)
        : "rax", "rbx", "rcx", "rdx");

    printf("輸入的字元為:%c\n",hello[0]);
    printf("回傳值是：%ld\n", ret);
}
