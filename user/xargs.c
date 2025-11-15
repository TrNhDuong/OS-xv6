#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// Giới hạn số đối số và độ dài dòng
#define MAXARG 32
#define MAX_INPUT_SIZE 512

void xargs_run(char *cmd[], int n_args);
int readline(char *buf, int max_len);

// Hàm chính của xargs
int main(int argc, char *argv[]) {
    int n_mode = 0; // 0: chế độ mặc định (tất cả cùng lúc), 1: chế độ -n 1
    int cmd_start_idx = 1; // Vị trí bắt đầu của lệnh mục tiêu trong argv[]

    if (argc > 1 && strcmp(argv[1], "-n") == 0) {
        if (argc < 3 || strcmp(argv[2], "1") != 0) {
            fprintf(2, "xargs: -n chỉ hỗ trợ -n 1\n");
            exit(1);
        }
        n_mode = 1;
        cmd_start_idx = 3; // Lệnh bắt đầu từ argv[3]
    }
    
    if (argc <= cmd_start_idx) {
        fprintf(2, "usage: xargs [-n 1] command [initial-args...]\n");
        exit(1);
    }
    
    // Lưu các đối số cố định của lệnh mục tiêu
    char *base_cmd[MAXARG];
    int base_argc = 0;
    for (int i = cmd_start_idx; i < argc; i++) {
        base_cmd[base_argc++] = argv[i];
    }
    
    // 2. Vòng lặp đọc và thực thi
    char input_buf[MAX_INPUT_SIZE];
    int n;
    
    while ((n = readline(input_buf, MAX_INPUT_SIZE)) > 0) {
        // Phân tách chuỗi input_buf thành các tham số
        char *p = input_buf;
        char *last_space = input_buf;
        
        // Chuẩn bị mảng đối số cho exec
        char *exec_args[MAXARG];
        int exec_argc = 0;
        
        // Copy các đối số cố định vào mảng exec_args
        for (int i = 0; i < base_argc; i++) {
            exec_args[exec_argc++] = base_cmd[i];
        }

        // Tách các từ trong input_buf (dựa trên khoảng trắng)
        while (*p != '\0' && exec_argc < MAXARG) {
            // Bỏ qua khoảng trắng
            while (*p == ' ' || *p == '\t' || *p == '\n') p++;
            if (*p == '\0') break;

            // Bắt đầu một từ mới
            
            int cnt = 0;
            while (*p != ' ' && *p != '\t' && *p != '\n' && *p != '\0') {p++; cnt++; }
            
            char* new_char = malloc(cnt + 1);
            for (int i = 0; i < cnt; i++) {
                new_char[i] = *(p - cnt + i);
            }
            new_char[cnt] = '\0';
            last_space = new_char;
            // Thêm tham số mới
            exec_args[exec_argc++] = last_space;

            

            // Nếu ở chế độ -n 1, thực thi ngay lập tức
            if (n_mode == 1) {
                exec_args[exec_argc] = 0; // Kết thúc mảng bằng NULL
                xargs_run(exec_args, exec_argc);
                
                // Chuẩn bị cho lần thực thi tiếp theo
                exec_argc = base_argc; 
            }
        }
        
        // Nếu không phải chế độ -n 1, thực thi tất cả các tham số đã thu thập
        if (n_mode == 0 && exec_argc > base_argc) {
            exec_args[exec_argc] = 0;
            xargs_run(exec_args, exec_argc);
        }
    }

    exit(0);
}

// Hàm đơn giản để đọc toàn bộ input từ STDIN cho đến EOF hoặc '\n'
// Rất đơn giản, không hoàn hảo như các hệ thống thực tế
int readline(char *buf, int max_len) {
    char c;
    int i = 0;
    while(read(0, &c, 1) == 1) {
        if (i < max_len - 1) {
            buf[i++] = c;
            if (c == '\n') break; // Dừng nếu gặp ký tự xuống dòng
        }
    }
    buf[i] = '\0';
    return i;
}


// Hàm thực thi lệnh con
void xargs_run(char *cmd[], int n_args) {
    // 1. Dùng fork() để tạo tiến trình con
    int pid = fork();

    if (pid == 0) {
        // 2. Tiến trình con: Dùng exec() để thay thế bằng lệnh mục tiêu
        exec(cmd[0], cmd);
        // Nếu exec() thất bại
        fprintf(2, "xargs: exec %s failed\n", cmd[0]);
        exit(1);
    } else if (pid > 0) {
        // 3. Tiến trình cha: Đợi tiến trình con kết thúc
        wait(0);
    } else {
        // fork thất bại
        fprintf(2, "xargs: fork failed\n");
    }
}