#include <unistd.h>  
#include <stdio.h>
#include <stdlib.h>

void ls()
{
    char *argv[ ]={"ls", NULL};   
    char *envp[ ]={"PATH=/bin", NULL};
    execve("/bin/ls", argv, envp);   
}
 
int main()   
{   
    printf("%p\n", ls);
    getchar();
	ls();

}  