#include "cache.h"
#include <stdlib.h>
#include <string.h>

/**
 * @description: 初始化缓存单元，动态分配内存
 * @param {LRU_cache} *cache 提前声明好，要被初始化的缓存变量
 * @return {*}
 */
// int LRU_cache_init(LRU_cache **cptr) {
//   LRU_cache* cache=*cptr;
//   cache = (LRU_cache *)malloc(sizeof(LRU_cache));
//   INIT_MY_LIST_HEAD(&cache->head);
//   cache->length = 0;
//   return LRU_OP_SUCCESS;
// }

/**
 * @description: 析构缓存单元，释放动态分配好的内存
 * @param {LRU_cache} *cache
 * @return {*}
 */
int LRU_cache_free(LRU_cache **cptr) {
  LRU_cache *cache = *cptr;
  free(cache);
  return LRU_OP_SUCCESS;
}

/**
 * @description: 查找链表中是否有含特定域名的一条文，为内部使用的函数
 * @param {LRU_cache} *cache
 * @param {char} *domain_name
 * @return {*}
 */
DNS_entry *__LRU_list_find(LRU_cache **cptr, const char *domain_name) {
  LRU_cache* cache = *cptr;
  mylist_head *p;
  mylist_for_each(p, &cache->head) {
    DNS_entry *entry = mylist_entry(p, DNS_entry, node);
    if (strcmp(entry->domain_name, domain_name) == 0) {
      return entry;
    }
  }
  return NULL;
}

/**
 * @description: 向条文链表中加入新一条，写入指定的内存位置中，为内部使用的函数
 * @param {LRU_cache} *cache
 * @param {DNS_entry} *entry 要被加入的新条文
 * @param {DNS_entry} *location 指定好的指定内存位置
 * @return {*}
 */
int __LRU_list_add(LRU_cache **cptr, DNS_entry *entry, DNS_entry *location) {
  LRU_cache *cache = *cptr;
  memcpy(location, entry, sizeof(DNS_entry));
  mylist_add_head(&location->node, &cache->head);

    return LRU_OP_SUCCESS;
}
/**
 * @description: 删除链表中的指定一个节点，为内部使用的函数
 * @param {LRU_cache} *
 * @param {DNS_entry} *entry
 * @return {*}
 */
int __LRU_list_del(LRU_cache **cptr, DNS_entry *entry) {
  LRU_cache *cache = *cptr;
  mylist_del_init(&entry->node);

  return LRU_OP_SUCCESS;
}
/**
 * @description: 将互联网上查到的新报文加入缓存系统中，为外部调用的接口
 * @param {LRU_cache} *cache
 * @param {DNS_entry} *entry
 * 互联网上查询到的内容组织成的新报文，事先应封装好对应内容
 * @return {*}
 */
int LRU_entry_add(LRU_cache **cptr, DNS_entry *entry) {
  LRU_cache *cache = *cptr;
  if (cache->length <
      LRU_CACHE_LENGTH) { //如果缓存空间仍未满，直接在内存空闲位置加入新条文，新条文会位于链表头
    __LRU_list_add(&cache, entry, &cache->list[cache->length]);
    cache->length++;
  } else { //如果缓存空间位置已满，将链表最后的一条文的内存位置腾出给新条文，新条文会位于链表头
    DNS_entry *tail = mylist_entry(cache->head.prev, DNS_entry, node);
    //printf("tail->dn:%s\n",tail->domain_name);
    if (mylist_is_last(&tail->node, &cache->head)) {
      __LRU_list_del(&cache, tail);
      __LRU_list_add(&cache, entry, tail);

    }

  }
  return LRU_OP_SUCCESS;
}

/**
 * @description: 检查客户端请求的某域名是否在缓存中
 * @param {LRU_cache} *cache
 * @param {DNS_entry} *entry
 * 请求的域名。事先应声明一个条文变量，并将域名赋给条文变量，将条文变量作为参数传入。
 * 如果查到，该条文变量里面会有查到的ip、类型等参数
 * @return {*} 返回值为int，通过检查返回值可知缓存是否查到该域名
 */
int LRU_cache_find(LRU_cache **cptr, DNS_entry *entry) {
  LRU_cache *cache = *cptr;
  DNS_entry *temp = __LRU_list_find(&cache, entry->domain_name);
  if (temp == NULL) {
    return LRU_OP_FAILED;
  } else {
    mylist_rotate_node_head(&temp->node, &cache->head);
    entry->ip = temp->ip;
    entry->type = temp->type;
    return LRU_OP_SUCCESS;
  }
}