# WebcamProtector
![user main](https://github.com/clavis0x/WebcamProtector/blob/master/screenshot/1.png)
## Introduction
Link: [커널 후킹을 이용한 웹캠 보호 프로그램 제작(pdf)](http://clavis.mireene.com/projects/WebcamProtector.pdf)

## Develop Environment
* Visual Studio Community 2015 Update 3
* Windows Driver Kit Version 7.1.0

## Platform
* Windows 7 x86

## Mechanism
* Windows Device Driver
* SSDT(System Service Descriptor Table) Hooking
* Kernel-User Communication : Global event object, DeviceIoControl    
![Mechanism](https://github.com/clavis0x/WebcamProtector/blob/master/screenshot/5.png)    
![Mechanism](https://github.com/clavis0x/WebcamProtector/blob/master/screenshot/6.png)

## Reference
* Greg Hoglund,James Butler, "Rootkits: Subverting the Windows Kernel", Addison-Wesley, 2006
* Mark E. Russinovich,David A. Solomon,Alex Ionescu, "Windows Internals (6th edition)", Pearson, 2012

## Screenshots
- Exception settings  
![exception settings](https://github.com/clavis0x/WebcamProtector/blob/master/screenshot/2.png)  

- Attack blocking notification (Metasploit)  
![blocked attack](https://github.com/clavis0x/WebcamProtector/blob/master/screenshot/3.png)  
![blocked attack](https://github.com/clavis0x/WebcamProtector/blob/master/screenshot/4.png)
