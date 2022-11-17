#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_SIZE 1024

int main()
{
    FILE *file = fopen("../user_dir/test.jpg", "rb");
    char buf[MAX_SIZE];
    int send_time = 0;
    int size;
    // while (!feof(file))
    // {
    //     bzero(buf, sizeof(buf));
    //     printf("正在进行第%d次发送...\n", ++send_time);
    //     size = fread(buf, 1, MAX_SIZE, file);
    //     printf("读取长度: %d\n", size);
    //     printf("读取内容: %x\n", buf);
    // }
    // fclose(file);
    // if(-1){
    //     printf("11");
    // }
    char str[] = "log.txt.lfd";
    int index = -1;
    for (int i = 0; i < sizeof(str); i++)
    {
        if (str[i] == '.')
            index = i;
    }
    char new[MAX_SIZE];
    strcpy(new, str + index);
    printf("%s", new);
}