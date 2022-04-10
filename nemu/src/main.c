int init_monitor(int, char *[]);
void ui_mainloop(int);

int main(int argc, char *argv[]) {
  //初始化
  int is_batch_mode = init_monitor(argc, argv);
  //用户界面
  ui_mainloop(is_batch_mode);

  return 0;
}
