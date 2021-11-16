#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

// double-linked, circular list. double-linked makes remove
// fast. circular simplifies code, because don't have to check for
// empty list in insert and remove.

void
lst_init(struct list *lst)
{
  lst->next = lst;
  lst->prev = lst;
}

int
lst_empty(struct list *lst) {
  return lst->next == lst;
}

void
lst_remove(struct list *e) {
  e->prev->next = e->next;
  e->next->prev = e->prev;
}

void*
lst_pop(struct list *lst) {
  if(lst->next == lst)
    panic("lst_pop");
  struct list *p = lst->next;
  lst_remove(p);
  return (void *)p;
}

void
lst_push(struct list *lst, void *p)
{
  struct list *e = (struct list *) p;
  e->next = lst->next;
  e->prev = lst;
  lst->next->prev = p;
  lst->next = e;
}

void
lst_print(struct list *lst)
{
  for (struct list *p = lst->next; p != lst; p = p->next) {
    printf(" %p", p);
  }
  printf("\n");
}
