# UI

* https://zh.wikipedia.org/zh-tw/SDL
* https://www.itread01.com/content/1549641630.html

摘要：

關於SDL,在簡介裡面，有一些概念，但是很多人還是留言，不清楚到底是個什麼。這節，我簡單總結下:

我們如何將一張圖顯示在螢幕上。這裡簡單的分為幾個部分，硬體螢幕，驅動程式，軟體部分。SDL不直接關注硬體螢幕，而是關注每個平臺下的螢幕驅動程式。比如window下的DirectX,linux下的x11 ,以及android下的opengl es。SDL通過將這三個平臺，當然不止這三個平臺的螢幕驅動，封裝成一套對外統一的API呼叫，讓使用者可以不關注具體某個平臺，可以快速開發影象的繪製操作。SDL的核心，便是如此。