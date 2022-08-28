###### tags: OS
# HW7

### 408410094 資工三 葉X勛
---
1. 執行make，將產生的四個執行檔案的執行結果截圖。
2. 「確實的」解釋「為什麼」peterson_trival-O3的執行結果是錯的。
3. 請問在你的電腦上「peterson_trival-g」的速度比「peterson_correct-O3」快或者是慢?上述兩個程式的正確與否?
4. 「確實的」解釋「3.」，某個程式比另一個程式快或者慢的理由。
---
執行環境:Win11的WSL
CPU:Intel(R) Core(TM) i5-8265U CPU @ 1.60GHz(8 CPUs), ~1.8GHz

---
## 1.執行make，將產生的四個執行檔案的執行結果截圖。
### peterson_trival-g
![](https://i.imgur.com/kz5Thm4.png)
:執行到後期會一直有p0,p1同時在critical section的情況發生。
### peterson_trival-O3
![](https://i.imgur.com/7uBggJX.png)
:產生deadlock。
### peterson_correct-g
![](https://i.imgur.com/AvGWFSN.png)
### peterson_correct-O3
![](https://i.imgur.com/btcLlZ2.png)

---
## 2.「確實的」解釋「為什麼」peterson_trival-O3的執行結果是錯的。
由1.的截圖，可發現當p1進來的時候，p0、p1同時進入deadlock。
以下為反組譯後p0的組語:
![](https://i.imgur.com/VNNKLbU.png)
:可以發現在<+24>那行，gcc將turn是否=1的判斷優化掉了，只判斷flag1是否=1，其原因在於原本程式在進行判斷前就已經先將turn=1，而由於gcc不會知道這個程式為multithreading的程式，因此在優化時gcc會將turn=1進行優化，只剩下判斷flag1=1。在反組譯的組語中得知，當flag1為1的時候，會跑到<+56>: jmp 0x1398 <p0+56>這行，會一直跳回自己這行，因此發生了deadlock的現象。

---
## 3.請問在你的電腦上「peterson_trival-g」的速度比「peterson_correct-O3」快或者是慢?上述兩個程式的正確與否?
由1.的截圖很明顯的可以發現前者的速度比後者還要快得多，但是peterson_trival-g是個錯誤的程式，因為他沒有使用atomic operation 來保護global varaible，導致turn在不同的thread中有不同的值，p0看到的turn可能是1，而p1看到的turn可能是0。而在peterson_correct-O3中，因為有使用了atomic operation對turn進行寫入，因此在不同的thread中看到的turn的值是一致的，確保程式的正確性。

---
## 4.「確實的」解釋「3.」，某個程式比另一個程式快或者慢的理由。
peterson_correct-O3使用了atomic operation來指定turn的值，如此每個thread(每顆CPU)看到的turn都會一樣。若是沒有用atomic operation的話，當CPU<sub>0</sub>剛修改完turn的值後，會先將最新的數值寫到自己的cache，等待cache line被標註為invalidate後才會寫到主記憶體，若同時間有另一顆CPU<sub>1</sub>欲存取主記憶體中同一位置的資料，由於自己的cache以及主記憶體中都沒有最新的資料，因此會讀到舊的turn值，造成程式錯誤。因此需要用atomic operation，在某顆CPU<sub>0</sub>更改某個位置的資料時，都要先確保其他CPU已經invalid同一位置的cache(欲寫入的CPU<sub>0</sub>廣播invalidate，其他CPU回invalidate ack)，清除掉其他CPU cache內的值後，才能將資料寫入自己的cache，稍後補寫回主記憶體。如此一來就能確保其他CPU能讀到更新後的值，但這樣的保護機制同時會使得程式的負擔加大(像是CPU<sub>0</sub>要廣播invalidate，並且等其他CPU回ack)，造成執行速度變慢。
以下為CPU<sub>0</sub>欲寫入時，需等待CPU<sub>1</sub>回ack，中間產生了stall，讓CPU<sub>0</sub>閒置了一段時間，導致程式執行變慢。
 <img src="https://i.imgur.com/hpiKULk.png" width = "400" height = "400"  align=center />

---
反組譯後，可以看到peterson_correct-O3多了一些mfence的指令，來確保gcc不會打亂程式執行的順序，但由於使用了mfence的關係，在mfence指令前的讀寫操作必須在mfence指令後的讀寫操作前完成，讓gcc不能對這部分的程式進行memory reordering，限制了gcc對其的優化，造成程式的執行速度下降。
![](https://i.imgur.com/XHtXJlA.png)
