# Minios鼠标相关

1. 在kernel/main.c中使用`init_kb()`初始化键盘主板。

   - 键盘参数的初始化

   - 设置键盘led：`set_led()`

   - 将键盘中断加入中断序列:`put_irq_handler`

   - enable_irq()

   - **初始化鼠标:`init_mouse()`**

     > 话说这里写的好乱

   - 设置鼠标led

2. 进程tty开启，**轮询开始**，其中**tty_mouse**判断鼠标操作

3. 当鼠标中断发来，调用**mouse_handler**处理发来的scan_code

4. 2中的tty_mouse判断handler中的处理结果，并写显存