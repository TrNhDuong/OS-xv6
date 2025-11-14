#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Ta sẽ dùng đệ quy để mô phỏng pipeline pipeline
// Mỗi pipeline là 1 tiến trình riêng biệt

// left_pipe_read: là đầu ống "NGHE" 
// thêm dòng _attribute__((noreturn)) để tránh cảnh báo biên dịch
void sieve(int left_pipe_read) __attribute__((noreturn));
void sieve(int left_pipe_read) {
  int my_prime;
  int num;
  int right_pipe[2];

  // Bước 1: Đọc số nguyên tố đầu tiên từ bên trái
  if (read(left_pipe_read, &my_prime, sizeof(my_prime)) == 0) {
    close(left_pipe_read); // Dọn dẹp sau khi hết số
    exit(0);
  }

  // Bước 2: In ra số nguyên tố của mình
  printf("prime %d\n", my_prime);

  // Bước 3: Tạo new pipe cho tiến trình tiếp theo bên phải
  pipe(right_pipe);

  // Bước 4. Dùng fork() để tạo tiến trình con
  if (fork() == 0) {

    // TIẾN TRÌNH CON TIẾP THEO (BÊN PHẢI)

    // Đóng những pipe không cần thiết
    close(left_pipe_read);
    close(right_pipe[1]);
    // Thực hiện nhiệm vụ tiếp theo: sàng lọc từ ống bên trái và gửi sang ống bên phải
    sieve(right_pipe[0]);

  } else {

    // TIẾN TRÌNH CHA (BÊN TRÁI)

    // Đóng những pipe không cần thiết
    close(right_pipe[0]);
    // Bắt đầu lọc: Đọc các số còn lại từ bên trái
    while (read(left_pipe_read, &num, sizeof(num)) != 0) {
      // Nếu số đó không chia hết cho số nguyên tố thì chuyển nó sang bên phải
      if (num % my_prime != 0) {
        write(right_pipe[1], &num, sizeof(num));
      }
    }

    // Đóng tất cả các ống khi xong việc
    close(left_pipe_read);  
    close(right_pipe[1]);  

    // Đợi cả pipeline bên phải kết thúc
    wait(0);
    exit(0);
  }
}

int
main(int argc, char *argv[])
{
  int p[2];

  // Tạo pipe đầu tiên
  pipe(p);

  if (fork() == 0) {

    // TIẾN TRÌNH CON
    
    close(p[1]); 
    sieve(p[0]);

  } else {

    // TIẾN TRÌNH CHA

    close(p[0]); 
    // Cho các số từ 2 đến 280 vào pipeline
    for (int i = 2; i <= 280; i++) {
      write(p[1], &i, sizeof(i));
    }
    // Đóng băng chuyền khi đã gửi xong
    close(p[1]);
    // Đợi toàn bộ pipeline kết thúc
    wait(0); 
    exit(0);
  }
  
  return 0; 
}