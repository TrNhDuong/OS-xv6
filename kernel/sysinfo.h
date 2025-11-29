struct sysinfo {
  uint64 freemem;   // Số lượng bộ nhớ trống (tính bằng bytes)
  uint64 nproc;     // Số lượng tiến trình đang hoạt động (state != UNUSED)
};