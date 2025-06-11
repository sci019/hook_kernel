#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/mm.h>       // struct vm_area_struct, struct page, page_to_pfn
#include <linux/mm_types.h> // struct vm_fault, vma->vm_file のために必要
#include <linux/pgtable.h>  // PAGE_SIZE のために必要
#include <asm/current.h>    // current マクロのために必要 (タスク名取得用)

// Kprobeの事前ハンドラ (handle_mm_fault が呼び出される直前)
// handle_mm_fault のシグネチャ: vm_fault_t handle_mm_fault(struct vm_fault *vmf)
static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    // RDIレジスタからvmf (vm_fault構造体) へのポインタを取得
    struct vm_fault *vmf = (struct vm_fault *)regs->di;

    // vmfとvmf->vmaが有効であることを確認
    if (vmf && vmf->vma) {
        printk(KERN_INFO "Kprobe: [PRE][PID:%d, Comm:%s] handle_mm_fault called. VirtAddr: 0x%lx, VMA: 0x%lx-0x%lx, vm_flags: 0x%lx, vm_file: %s\n",
               current->pid, current->comm, vmf->address, vmf->vma->vm_start, vmf->vma->vm_end,
               vmf->vma->vm_flags, (vmf->vma->vm_file == NULL ? "NULL" : "NON-NULL"));

        if (vmf->vma->vm_file == NULL) {
            // これはファイルにマップされていないメモリ領域 (匿名メモリ) です。
            printk(KERN_INFO "Kprobe: [PRE][PID:%d, Comm:%s] Anonymous memory access suspected. Expected page fault.\n",
                   current->pid, current->comm);
        }
    }
    return 0; // 元の関数(handle_mm_fault)の実行を続行
}

// Kprobeの事後ハンドラ (handle_mm_fault が実行を終えた後)
static void handler_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags)
{
    // RDIレジスタからvmf (vm_fault構造体) へのポインタを取得
    struct vm_fault *vmf = (struct vm_fault *)regs->di;
    // RAXレジスタは関数の戻り値を保持します。handle_mm_fault の戻り値は vm_fault_t です。
    vm_fault_t ret_val = (vm_fault_t)regs->ax;

    // vmfとvmf->vmaが有効であることを確認
    if (vmf && vmf->vma) {
        printk(KERN_INFO "Kprobe: [POST][PID:%d, Comm:%s] handle_mm_fault returned: %d (0x%x). VirtAddr: 0x%lx, vmf->page: %s\n",
               current->pid, current->comm, ret_val, ret_val, vmf->address, (vmf->page ? "NOT NULL" : "NULL"));

        // 匿名メモリのフォールトであり、かつページが正常に割り当てられた場合
        // vmf->page が NULL でない場合、新しい物理ページが割り当てられたことを意味します。
        // vm_fault_t の戻り値も確認するとより詳細な情報が得られます。
        if (vmf->vma->vm_file == NULL) { // 匿名メモリであると判断
            if (vmf->page) { // vmf->page が設定されているか
                struct page *allocated_page = vmf->page;
                unsigned long pfn = page_to_pfn(allocated_page);
                unsigned long physical_address = pfn * PAGE_SIZE;

                printk(KERN_INFO "Kprobe: [POST][PID:%d, Comm:%s] Anonymous memory: Physical page created. Virt: 0x%lx, Phys: 0x%lx, RetVal: %d\n",
                       current->pid, current->comm, vmf->address, physical_address, ret_val);
            } else {
                printk(KERN_INFO "Kprobe: [POST][PID:%d, Comm:%s] Anonymous memory: vmf->page is NULL after handle_mm_fault for VirtAddr 0x%lx. Return value: %d\n",
                       current->pid, current->comm, vmf->address, ret_val);
            }
        }
    }
}

// Kprobe構造体を定義
static struct kprobe kp = {
    .symbol_name = "handle_mm_fault", // フックするカーネル関数名を handle_mm_fault に設定
    .pre_handler = handler_pre,         // 事前ハンドラを設定
    .post_handler = handler_post,       // 事後ハンドラを設定
};

// モジュール初期化関数
static int __init kprobe_init(void)
{
    int ret;
    ret = register_kprobe(&kp); // Kprobeを登録
    if (ret < 0) {
        printk(KERN_ERR "Kprobeの登録に失敗しました (エラーコード: %d)。'%s' シンボルがエクスポートされていない可能性があります。\n", ret, kp.symbol_name);
        return ret;
    }
    printk(KERN_INFO "Kprobeを '%s' に登録しました。\n", kp.symbol_name);
    return 0;
}

// モジュール終了関数
static void __exit kprobe_exit(void)
{
    unregister_kprobe(&kp); // Kprobeを登録解除
    printk(KERN_INFO "Kprobeを '%s' から登録解除しました。\n", kp.symbol_name);
}

// モジュール初期化関数と終了関数を登録
module_init(kprobe_init);
module_exit(kprobe_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("メモリフォールト (匿名メモリ) の物理ページ作成をログに記録するKprobeモジュール");
MODULE_VERSION("0.7"); // バージョンを更新
