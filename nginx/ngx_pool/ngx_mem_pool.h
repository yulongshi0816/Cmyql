//
// Created by 施玉龙 on 2024/8/15.
//

#ifndef NGX_POOL_NGX_MEM_POOL_H
#define NGX_POOL_NGX_MEM_POOL_H
// 移植ngxin内存池项目
// 非配小块内存池的头部信息数据

#include <cwchar>

using u_char = unsigned  char;
using ngx_uint_t = unsigned int;

// 清理函数的类型
typedef void (*ngx_pool_cleanup_pt)(void *data);
struct ngx_pool_cleanup_s {
    ngx_pool_cleanup_pt   handler; // 定义函数指针，保存清理操作的函数
    void                 *data;
    ngx_pool_cleanup_s   *next; // 所有cleanup操作都在链表上
};

struct ngx_pool_s; // 类型潜质声明

struct ngx_pool_data_t{
    u_char               *last; // 小块内存的可用内存的起始地址
    u_char               *end; // 小块内存池可用内存的末尾地址
    ngx_pool_s           *next; // 所有小块内存池被串在链表上
    ngx_uint_t            failed; // 记录分配失败的次数
};

// 大块内存的头部信息
struct ngx_pool_large_s {
    ngx_pool_large_s     *next; //所有大块内存分配串在链表上
    void                 *alloc; // 保存分配出去的地址
};
// ngx内存池头部信息和管理成员信息
struct ngx_pool_s {
    ngx_pool_data_t       d; // 头信息
    size_t                max; // 存储的小块内存和大块内存的分界线
    ngx_pool_s           *current; // 指向第一个提供小块内存分配的小块内存池
    ngx_pool_large_s     *large; // 指向大块内存的入口地址
    ngx_pool_cleanup_s   *cleanup; // 指向所有的预制清理操作的回调函数 链表的入口
};

// 把数值d调整到临近a的倍数
#define ngx_align(d, a)     (((d) + (a - 1)) & ~(a - 1))
// 默认一个物理页面的大小是4096
const int ngx_pagesize = 4096;
// 小块内存池可分配的最大空间
const int NGX_MAX_ALLOC_FROM_POOL  = ngx_pagesize - 1;// 4k

const int NGX_DEFAULT_POOL_SIZE = 16 * 1024; // 16k

const int NGX_POOL_ALIGNMENT = 16; // 内存池大小按照16个字节对齐
// 小块内存池最小的size调整成NGX_POOL_ALIGGNMENT的临近倍数
const int NGX_MIN_POOL_SIZE =
        ngx_align((sizeof(ngx_pool_s) + 2 * sizeof(ngx_pool_large_s)), NGX_POOL_ALIGNMENT);

class ngx_mem_pool {
public:
    // 创建指定size的内存池，但是小块内存池不超过一个页面大小
    bool ngx_create_pool(size_t size);
    // 考虑内存字节对齐，从内存池申请size大小的内存；
    void *ngx_palloc(size_t size);
    void *ngx_pnalloc(size_t size); //    不考虑内存的对齐
    // 调用ngx_palloc实现内存分配，但是会初始化0
    void *ngx_pcalloc(size_t size);
    // 释放大块内存
    void ngx_pfree(ngx_pool_s *pool, void *p);
    void ngx_reset_pool(); // 充值
    void ngx_destroy_pool(); // 内存池的销毁
    // 添加回调清理操作函数
    ngx_pool_cleanup_s *ngx_pool_cleanup_add(size_t size);
private:
    ngx_pool_s * pool_; // 指向ngx内存池的指针
    // 小内存分配
    void *ngx_palloc_small(size_t size, bool align);
    void *ngx_palloc_large(ngx_pool_s *pool, size_t size);
    // 分配新的小块内存池
    void *ngx_palloc_block(ngx_pool_s *pool, size_t size);
};

#endif //NGX_POOL_NGX_MEM_POOL_H
