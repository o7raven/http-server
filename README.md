# HTTP Server in C

Simple HTTP Server coded in C using winsocks. So far only usable for absolute basic needs. No support for UNIX compiling yet.


### Notes
- Because the server is programmed in winsock there is no support for UNIX systems for now. Once the Windows version is stable and actually usable, UNIX support will be added as soon as possible
- I'm not a professional programmer, there are probably plenty of bugs and security vulnerabilities I don't know about. I plan to fix it, but I don't recommend using it as a main server, rather as a temporary solution.
- Any feedback is greatly appreciated
### TODO
 - Real-time requested file update
 - Custom request redirects
 - Opening files in folders
 - More methods than just get
 - Code optimization
 - Get rid of vulnerabilities (that's not really possible is it)
 - Add documentation
### Installation

Clone this repository and then cd into it.

```bash
  git clone https://github.com/o7raven/http-server/tree/main
  cd http-server
```
The repository contains a Makefile, so just type `make` and the program will compile. If you want to name the output, add the `-o fileName` option to the end of the Makefile.
## Acknowledgements
I would never have programmed this if I had nothing to learn from. Here are links to sites that I have used in my programming that have helped me
 - [J Marshall's HTTP Guide](https://jmarshall.com/easy/http/)
 - [String manipulations in C](https://en.wikibooks.org/wiki/C_Programming/String_manipulation)
 - [Windows sockets](https://learn.microsoft.com/en-us/windows/win32/winsock/windows-sockets-start-page-2)
 - [Windows CreateThread function](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createthread?redirectedfrom=MSDN) 
### These appliations during the development really helped me a lot
- [ARC for receiving and creating custom requests](https://github.com/advanced-rest-client/arc-electron)
- [Fiddler Classic for debugging server responses](https://www.telerik.com/download/fiddler)
*I can't think of more, but there are 100% more.*
## Authors

- [@raven](https://www.github.com/o7raven)

