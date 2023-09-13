# 编译器设置
CC = /home/y/work/app/csi-test/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc
CFLAGS = -Wall -Wextra -g -Wno-unused-function
# CFLAGS += -I/home/y/work/code/d82_411/yocto/build-x9u_ref_b_k414/tmp/work/x9u_ref_b_k414-sdrv-linux/linux-semidrive-dev/4.14.61-r1/recipe-sysroot/usr/include/
VPATH=/home/y/work/code/d82_411/yocto/build-x9u_ref_b_k414/tmp/work/x9u_ref_b_k414-sdrv-linux/linux-semidrive-dev/4.14.61-r1/recipe-sysroot/

# 目标文件和可执行文件设置
TARGET = netlink_demo
OBJS = netlink_demo.o

# 默认目标
all: $(TARGET)

# 链接目标文件生成可执行文件
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ -lpthread

# 编译每个源文件为目标文件
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理生成的文件
clean:
	rm -f $(TARGET) $(OBJS)
