#  HW6

### 408410094 葉X勛
---
1. 使用strace在處理器上追蹤sensors這個指令，列出sensors從哪邊讀取溫度。
2. 寫一隻程式能不斷讀取溫度，直到指定溫度，並印出每個Core的溫度。
---
## 1
![](https://i.imgur.com/kqqQMoe.png)
#### : 以第一顆核心為例，從上圖可得知是從/sys/class/hwmon/hwmon1這個資料夾底下讀取CPU資訊，而其中的temp1_input裡存放的是這顆CPU的溫度。
---
## 2
![](https://i.imgur.com/7eL1C8f.png)
#### :我是連到實驗室的電腦去測溫度的，發現CPU的平均溫度都處於62上下，因此我指定溫度為61，這樣程式才會停下來，否則會一直在迴圈裡出不來。程式最後會將各顆CPU  的溫度列出。