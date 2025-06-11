# kernel API フォールトハンドラのフック手順

プロセス1（別ターミナルでdmseg出力をリアルタイム監視）
```bash
dmesg -w
```

プロセス2
```bash
# kprobe利用ドライバビルド
cd hook_kernel_api
make
cd ..

# kprobe利用ドライバロード
sudo insmod hook_kernel_api/kprobe_gup.ko

# テスト実行
gcc test.c -o test
./test

sudo rmmod kprobe_gup
```
