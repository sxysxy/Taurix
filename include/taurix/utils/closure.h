/* include/taurix/utils/closure.h
        手动实现的闭包
        author: hfcloud(sxysxygm@gmail.com)
          date: 2019.08.26
*/

#ifndef TAURIX_CLOSURE_H
#define TAURIX_CLUSURE_H 

#ifdef __cplusplus
extern "C" {
#endif

struct tagClosure {
    union {
        const char *name;
        const char *name_wchar;
    };
    void *pdata; //指向数据
    struct tagClosure *next;  //组成链表
};
typedef struct tagClosure Closure;

#define cls_pack(vardata, varname, var) Closure var; var.name = varname, var.pdata = vardata, var.next = (void*)0;

#define cls_unpack(closure, receiver) receiver = (typeof(receiver))((Closure*)closure)->pdata;

#define cls_pack_boxed(vardata, varname, var)   struct {                              \
                                                    union {                           \
                                                        const char *name;             \
                                                        const char *name_wchar;       \
                                                    };                                \
                                                    typeof(vardata) data;             \
                                                    void *next;                       \
                                                }var;                                 \
                                                var.name = (const char*)varname;      \
                                                var.data = vardata;                   \
                                                var.next = (void*)0;                

#define cls_unpack_boxed(closure, receiver)            { struct {                                 \
                                                            union {                               \
                                                                const char *name;                 \
                                                                const char *name_wchar;           \
                                                            };                                    \
                                                            data_type data;                       \
                                                            void *next;                           \
                                                        }*__cls_tmp_var___, *__cls_tmp_var__ = (typeof(__cls_tmp_var__))closure; \
                                                        receiver = __cls_tmp_var__->data;   }           

#ifdef __cplusplus
}
#endif

#endif