#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  // Bước 1: Tạo 2 pipe với
  // p_to_c: pipe từ Parent (cha) -> Child (con)
  // c_to_p: pipe từ Child (con) -> Parent (cha)
  // cấp phát mảng 2 phần tử cho mỗi pipe vì mỗi pipe có 2 đầu: [0] đọc, [1] viết
  int p_to_c[2];
  int c_to_p[2];
  pipe(p_to_c);
  pipe(c_to_p);

  // Bước 2: Tạo một buffer 1 byte để gửi tin
  char buf[1];
  buf[0] = 'a'; // Giả sử gửi ký tự 'a' (có thể gửi bất kỳ ký tự nào)

  // Bước 3. Tạo tiến trình con (dùng fork() để nhân bản tiến trình)
  int pid = fork();

  if (pid < 0) {
    // Trường hợp fork() thất bại
    fprintf(2, "fork failed\n");
    exit(1);
    // Children sẽ có pid == 0
    // Parent sẽ có pid > 0 (là PID của tiến trình con)

  } else if (pid == 0) {
    
    // TIẾN TRÌNH CON
    int child_pid = getpid();

    // Đóng các đầu ống khi không dùng:
    // 1. Con KHÔNG gửi vào pipe p_to_c, chỉ đọc
    close(p_to_c[1]); 
    // 2. Con KHÔNG đọc từ c_to_p, chỉ viết
    close(c_to_p[0]);

    // Dùng read() để đọc từ cha 
    read(p_to_c[0], buf, 1); 

    // In thông báo ping khi nhận được byte từ cha
    printf("%d: received ping\n", child_pid); 

    // Dùng write() để gửi byte nhận được về cho cha 
    write(c_to_p[1], buf, 1); 

    // Đóng các đầu ống còn lại và thoát
    close(p_to_c[0]);
    close(c_to_p[1]);
    exit(0); 

  } else {

    // TIẾN TRÌNH CHA
    int parent_pid = getpid();

    // Đóng các đầu ống khi không dùng:
    // 1. Cha KHÔNG đọc từ p_to_c, chỉ viết 
    close(p_to_c[0]);
    // 2. Cha KHÔNG viết vào c_to_p, chỉ đọc
    close(c_to_p[1]);

    // Dùng write() để gửi 1 byte cho con 
    write(p_to_c[1], buf, 1); 

    // Dùng read() để chờ con gửi lại 1 byte
    read(c_to_p[0], buf, 1); 

    // In thông báo pong khi nhận được byte từ con
    printf("%d: received pong\n", parent_pid); 

    // Đóng các đầu ống còn lại và thoát
    close(p_to_c[1]);
    close(c_to_p[0]);
    exit(0); 
  }
}