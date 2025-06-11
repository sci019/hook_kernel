# kernel API フォールトハンドラのフック手順

```bash
cd hook_kernel_api
make
cd ..

sudo dmesg -c
sudo insmod hook_kernel_api/kprobe_gup.ko

gcc test.c -o test
./test
sudo dmesg > dmseg.log

sudo rmmod kprobe_gup
```
