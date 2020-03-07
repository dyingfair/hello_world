#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
 #include <unistd.h>
 
int (*realopen)(const char *pathname, int flags);
int (*realclose)(int fd);
int open(const char *pathname, int flags) {

    void *handle;    
    char *error;
	printf("my open\n");
    handle = dlopen("libc.so.6", RTLD_LAZY);//获得libc.so.6的句柄

    if ((error = dlerror()) != NULL) {
        puts(error);
        exit(-1);
    }

    realopen = dlsym(handle, "open");//返回open函数在libc.so.6中的加载时地址

    if ((error = dlerror()) != NULL) {
        puts(error);
        exit(-1);
    }

    fputs("you will open the file: ", stderr);
    puts(pathname);

    return realopen(pathname, flags);//调用实际的open函数
}

int main(int argc, char **argv) {
	printf("fdrwerwe\n");
    int fd = open(__FILE__, 0);
    if (fd != -1) 
		close(fd);
	else
		printf("111\n");
    return 0;
}