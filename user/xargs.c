#include "kernel/types.h"
#include "user/user.h"

#define MAXBUF 512
#define MAXARGS 20

int main(int argc, char *argv[]) {
    if(argc < 2){
        fprintf(2, "Usage: xargs command\n");
        exit(1);
    }

    char buf[MAXBUF];
    char *words[MAXARGS];
    int word_count = 0;

    // Đọc input từ stdin
    int n;
    while((n = read(0, buf, sizeof(buf)-1)) > 0){
        buf[n] = '\0';
        char *p = buf;

        // Tách input thành từng từ
        while(*p){
            // Bỏ space hoặc newline
            while(*p == ' ' || *p == '\n') *p++ = 0;
            if(*p == 0) break;
            // Lưu từ
            if(word_count < MAXARGS) words[word_count++] = p;
            // Đi tới cuối từ
            while(*p && *p != ' ' && *p != '\n') p++;
        }
    }

    if(word_count == 0){
        exit(0); // không có từ nào
    }

    // Chuẩn bị mảng argument cho exec
    char *cmd_args[MAXARGS];
    int j = 0;
    cmd_args[j++] = argv[1]; // lệnh chính
    for(int i=0; i<word_count; i++){
        cmd_args[j++] = words[i]; // thêm argument
    }
    cmd_args[j] = 0; // kết thúc mảng

    // Fork và exec
    int pid = fork();
    if(pid == 0){
        exec(cmd_args[0], cmd_args);
        fprintf(2, "xargs: exec failed\n");
        exit(1);
    } else {
        wait(0);
    }

    exit(0);
}
