// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
} kmem;

struct page pages[PHYSTOP/PGSIZE];
struct page *page_lru_head = 0;
struct spinlock lru_lock;
int num_free_pages;
int num_lru_pages;
int num = 1;


// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
  initlock(&lru_lock,"lru");
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;
  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP){
    if((uint)v % PGSIZE !=0){
      cprintf("kfree v is not size aligned!\n");
    }
    if(v<end){
      cprintf("v < end\n");
    }
    if(V2P(v) >= PHYSTOP){
      cprintf("V2P(v) >= PHYSTOP\n");
    }
    panic("kfree");
  }
  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = (struct run*)v;
  r->next = kmem.freelist;
  kmem.freelist = r;
  if(kmem.use_lock)
    release(&kmem.lock);
}

void lru_link(pde_t *pgdir, char *vaddr,uint pa)
{
  acquire(&lru_lock);
  pages[pa/PGSIZE].pgdir = pgdir;
  pages[pa/PGSIZE].vaddr = vaddr;
  if (page_lru_head == 0){
    page_lru_head = &pages[pa/PGSIZE];
    pages[pa/PGSIZE].prev = &pages[pa/PGSIZE];
    pages[pa/PGSIZE].next = &pages[pa/PGSIZE]; 
  }
  pages[pa/PGSIZE].prev = page_lru_head->prev;
  pages[pa/PGSIZE].next = page_lru_head;
  page_lru_head->prev->next = &pages[pa/PGSIZE];
  page_lru_head->prev = &pages[pa/PGSIZE];
  release(&lru_lock);
}

void lru_unlink(uint pa)
{
  acquire(&lru_lock);
  struct page *r;
  r = &pages[pa/PGSIZE];
  if (r->next == r){  //only one member at lru_list
    page_lru_head = 0;
  }
  else{
    r->next->prev = r->prev;
    r->prev->next = r->next;  //unlink r
  }
  release(&lru_lock);
}

int reclaim(void)
{
  acquire(&lru_lock);
  struct page *r;
  char *mem;	  //corresponding Physical Page
  uint *pte;
  r = page_lru_head;
  if (!r){		
    cprintf("OOM(out of memory) error!\n");
    return 0;
  }
  for(;;){
    r = page_lru_head;
    if (!(*(uint *)pgdir2pte(r->pgdir,r->vaddr)& PTE_A)){
      break;
    }
    *(uint *)pgdir2pte(r->pgdir,r->vaddr)  &= ~PTE_A;  //clear PTE_A
    page_lru_head = r -> next;
  }
  page_lru_head = r->next;
  page_lru_head->prev = r->prev;
  r->prev->next = page_lru_head;
  if (page_lru_head == r){  // only one page was at linked list
    page_lru_head = 0;
  }
  mem = P2V(PTE_ADDR(*(uint *)pgdir2pte(r->pgdir,r->vaddr)));
  pte = (uint *)pgdir2pte(r->pgdir,r->vaddr);
  *pte &= ~0xFFFFF000;	//bic
  *pte |= (uint)num<<12;
  *pte &= (~PTE_P);
  flushtlb();		// call lcr3 to flush TLB
  num++;				//blkno
  release(&lru_lock);
  release(&kmem.lock);			//kfree need kmemlock
  swapwrite(mem,num);
  kfree(mem);
  return 1;
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;
try_again:
  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(!r && reclaim()){
    goto try_again;
  }
  if(r)
    kmem.freelist = r->next;
  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)r;
}

