#include <taurix/mm/basic_mm.h>

/* 
void basic_mm_malloc(BasicMM *self, size_t size) {

}
void basic_mm_free(BasicMM *self, void *ptr) {

}

void basic_mm_initialize(BasicMM *self, void *baseptr, size_t size) {
    ((TObject *)self)->vtbl.initialize((TObject *)self);  //调用父类的initialize
    ((TObject *)self)->vptr = &self->vtbl;                  
    self->vtbl.malloc = basic_mm_malloc;
    self->vtbl.free = basic_mm_free;
    self->vtbl.finalize = basic_mm_finalize;
}

void basic_mm_finalize(BasicMM *self) {
    ((TObject *)self)->vtbl.finalize((TObject *)self);
}

void basic_mm_new(BasicMM *self) {
    self->vtbl.initialize = basic_mm_initialize; 
}
*/