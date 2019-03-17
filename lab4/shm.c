#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct shm_page {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id =0;
    shm_table.shm_pages[i].frame =0;
    shm_table.shm_pages[i].refcnt =0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {

  // lab4 added here
  // case 1
  acquire(&(shm_table.lock));
  int i;
  for (i = 0; i < 64; i++) {
    if (id == shm_table.shm_pages[i].id) {
      uint va = PGROUNDUP( myproc()->sz );
      // mappages(myproc()->pgdir, (char *)va, PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U);
      if (mappages(myproc()->pgdir, (void *)va, PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U|PTE_P) < 0) {
        cprintf("attach failed!\n");
        return -1;
      }
      shm_table.shm_pages[i].refcnt += 1;
      myproc()->sz += PGSIZE;
      *pointer=(char *)va;
      release(&(shm_table.lock));
      return 0;
    }
  }
 release(&(shm_table.lock)); 
// case 2
  acquire(&(shm_table.lock));
  for (i = 0; i < 64; i++) {
    if (shm_table.shm_pages[i].id == 0 && shm_table.shm_pages[i].frame == 0 && shm_table.shm_pages[i].refcnt == 0) {
      shm_table.shm_pages[i].id = id;
      shm_table.shm_pages[i].frame = kalloc();
      shm_table.shm_pages[i].refcnt = 1;
      uint va = PGROUNDUP( myproc()->sz );
      memset(shm_table.shm_pages[i].frame, 0, PGSIZE);
      if (mappages(myproc()->pgdir, (void *)va, PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U|PTE_P) < 0) {
        cprintf("init failed!\n");
        release(&(shm_table.lock));
        return -1;
      }
      myproc()->sz += PGSIZE;
      *pointer=(char *)va;
      break;
    }
  }
  release(&(shm_table.lock));
  return 0; //added to remove compiler warning -- you should decide what to return
}


int shm_close(int id) {
  //you write this too!
  // lab4 added here
  acquire(&(shm_table.lock));
  int i = 0;
  for (i = 0; i < 64; i++) {
    if (id == shm_table.shm_pages[i].id) {
      shm_table.shm_pages[i].id = 0;
      shm_table.shm_pages[i].frame = 0;
      if (shm_table.shm_pages[i].refcnt == 0) {
      break;
      }else {
        shm_table.shm_pages[i].refcnt -= 1;
      break;
      } 
    }
  }
  release(&(shm_table.lock));
  return 0; //added to remove compiler warning -- you should decide what to return
}
