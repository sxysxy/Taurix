/* include/taurix/linklist.h 
        基本的链表的实现
        author: hfcloud
          date: 2019.08.26
*/

#ifndef TAURIX_LINKLIST_H
#define TAURIX_LINKLIST_H

//在链表头部插入新元素item，这个会修改head，故head必须为左值
#define ll_insert_front(head, item) (item)->next = head; head = (item);

//在链表尾部插入新元素item
#define ll_insert_back(head, item)  {                                       \
                                      typeof(head) __ll_tmp_p__ = head;     \
                                      while(__ll_tmp_p__->next)             \
                                        __ll_tmp_p__ = __ll_tmp_p__->next;  \
                                      __ll_tmp_p__->next = item;            \
                                    }

//遍历链表
#define ll_traverse(head, function, closure) {  typeof(head) __ll_tmp_p__ = head;       \
                                                while(__ll_tmp_p__) {                   \
                                                    function(__ll_tmp_p__, closure);    \
                                                    __ll_tmp_p__ = __ll_tmp_p__->next;  \
                                                }                                       \
                                             }

//移除链表中的元素item，可能会修改head，故head必须为左值。 ！！！                                
#define ll_remove(head, item) { if(head != NULL) {                                      \
                                    if(head == (item)) head = head->next;               \
                                    else {                                              \
                                        typeof(head) __ll_tmp_p__ = head;               \
                                        while(__ll_tmp_p__->next != (item))             \
                                            __ll_tmp_p__ = __ll_tmp_p__->next;          \
                                        __ll_tmp_p__->next = (item)->next;              \
                                    }                                                   \
                                }                                                       \
                              }

#endif