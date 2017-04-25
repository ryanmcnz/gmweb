# GMWeb: GameMaker + Chromium extension (via Awesomium)

GMWeb is an extension for GameMaker Studio that enables displaying and interacting with web content on surfaces in-game:

[Watch trailer demonstrating GMWeb](https://www.youtube.com/watch?v=_9TkpzlJIzw)

This is achieved via integration of Awesomium, which contains Chromium.. allowing for HTML/CSS/JS game UI's along the lines of what can be achieved in WebKit.

![](https://raw.githubusercontent.com/ryanmcnz/gmweb/master/screenshot1.jpg)
![](https://raw.githubusercontent.com/ryanmcnz/gmweb/master/screenshot2.jpg)
![](https://raw.githubusercontent.com/ryanmcnz/gmweb/master/screenshot3.jpg)

*NOTE:* This project is very unfinished. Bugs will exist, and wrapping of Awesomium is still a bit glitchy (especially in fullscreen support). Features such as support for special keys during input are currently lacking.. and the version of Chromium in Awesomium is also very outdated.
This means that quite a few APIs are missing from it, eg. gamepad support (but of course you can still achieve this anyway via handling it in GameMaker with extensions such as the brilliant "InputDog" by Messhof LLC.

This is open to pull requests.

**Credits:**
- Awesomium belongs to Adam Simmons
- Chromium belongs to Google
- A big thanks to [Liam](https://github.com/LiamKarlMitchell) for developing the GMWeb DLL in C++, as well as figuring out how to generate that pesky GM extension XML.

**Disclaimer:**

This is a free proof-of-concept project and is provided unfinished, as-is. GMWeb is not commercial, is created solely for fun and is not affiliated with Awesomium or Chromium. If you use this extension in your game, please make sure you follow the licenses for both Awesomium and Chromium!

**Donations:**

If you found this code useful, consider chucking some $$ at us via PayPal!
- ryanmc.nz@gmail.com (GML, UI demos)
- LiamKarlMitchell@gmail.com (engineering, god-like C++ for DLL)
