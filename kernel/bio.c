// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKETS 13

struct {
  struct buf buckets[NBUCKETS];       // 哈希表
  struct spinlock buclocks[NBUCKETS]; // 每个哈希表的锁
  char lockname[NBUCKETS][20];        // 哈希表锁的名称
  struct buf buf[NBUF];               // 缓冲区
  struct spinlock lock;               // bcache的锁
} bcache;


void
binit(void)
{
  struct buf *b;
  int i;
  
  // 初始化锁
  for (i = 0; i < NBUCKETS; ++i){
    snprintf(bcache.lockname[i], 20, "bcache.bucket-%d", i);
    initlock(&bcache.buclocks[i], bcache.lockname[i]);
  }

  // 为每个哈希桶分配等量缓冲区
  for (b = bcache.buf; b < bcache.buf + NBUF; b++){
    i = (b - bcache.buf) % NBUCKETS;
    acquire(&bcache.buclocks[i]);
    b->next = bcache.buckets[i].next;
    bcache.buckets[i].next = b;
    release(&bcache.buclocks[i]);
    initsleeplock(&b->lock, "buffer");
  }
}


// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b, *cur = 0, *pre = 0;
  uint i, buckectno, sno;
  buckectno = blockno % NBUCKETS;
  acquire(&bcache.buclocks[buckectno]);
  for (b = bcache.buckets[buckectno].next; b != 0; b = b->next){
    if (b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.buclocks[buckectno]);
      acquiresleep(&b->lock);
      return b;
    }
  }
  
  acquire(&bcache.lock);

  for (i = 0; i < NBUCKETS; i++){
    sno = (buckectno + i) % NBUCKETS;
    if (sno != buckectno){
      acquire(&bcache.buclocks[sno]);
    }
    for (b = &bcache.buckets[sno]; b->next != 0; b = b->next){
      if (b->next->refcnt == 0 && (!cur || b->next->timestamp < cur->timestamp)){
        pre = b;
        cur = b->next;
      }
    }
    if (cur){
      cur->dev = dev;
      cur->blockno = blockno;
      cur->valid = 0;
      cur->refcnt = 1;
      pre->next = cur->next;
      release(&bcache.buclocks[sno]);
      acquire(&bcache.buclocks[buckectno]);
      cur->next = bcache.buckets[buckectno].next;
      bcache.buckets[buckectno].next = cur;
      release(&bcache.buclocks[buckectno]);
      release(&bcache.lock);
      acquiresleep(&cur->lock);
      return cur;
    }
    release(&bcache.buclocks[sno]);
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  uint bucketno;

  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  bucketno = b->blockno % NBUCKETS;
  acquire(&bcache.buclocks[bucketno]);
  b->refcnt--;
  if (b->refcnt == 0) {
    b->timestamp = ticks; // 更新时间戳
  }
  release(&bcache.buclocks[bucketno]);
}


void
bpin(struct buf *b) {
  uint bucketno = b->blockno % NBUCKETS;
  acquire(&bcache.buclocks[bucketno]);
  b->refcnt++;
  release(&bcache.buclocks[bucketno]);
}

void
bunpin(struct buf *b) {
  uint bucketno = b->blockno % NBUCKETS;
  acquire(&bcache.buclocks[bucketno]);
  b->refcnt--;
  release(&bcache.buclocks[bucketno]);
}



