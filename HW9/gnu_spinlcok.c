
//for hw9's problem 1
#include <atomic.h>
#include "pthreadP.h"
#include <shlib-compat.h>

int __pthread_spin_lock (pthread_spinlock_t *lock)
{
  int val = 0; //先將val的值設為0
  if (__glibc_likely (atomic_exchange_acquire (lock, 1) == 0)) //將兩參數的值進行對調，然後回傳lock對調前本來的值。
    return 0; //如果lock原本為0，表示還沒被上鎖，那麼就將lock設為1，並會return 0，表示成功獲得lock。
  do
    {
      do
        {
          /* TODO Back-off.  */ //隨機等待一段時間，避免兩個以上的thread同時lock。
          atomic_spin_nop (); //讓while不要跑的那麼快，可以有效降溫CPU。
          val = atomic_load_relaxed (lock); //將lock的值load到val，而由於不用在乎前後順序，只要確保有成功讀取lock的值就好，所以用relaxed。
        }
      while (val != 0); //當val為1時，表示目前有人先lock了，故會繼續在這個迴圈裡面跑，直到發現val為0(代表目前為unlock的狀態)，才會跳出此迴圈。
    }
  while (!atomic_compare_exchange_weak_acquire (lock, &val, 1)); //如果lock=val，將lock=1(上鎖)，回傳True跳出此迴圈。否則會將value=lock，並回傳False，繼續跑迴圈。
  return 0; //成功獲得lock
}

int __pthread_spin_unlock(pthread_spinlock_t *lock)
{
    atomic_store_release(lock, 0); // 將lock設為0，並透過release將結果傳遞出去。
    return 0; // 成功unlock
}