# 嵌入式Linux

## 第一章 系统编程概念

### 一、系统调用

* 系统调用是软件上由用户态进入内核态的唯一方式，保证用户在用户空间安全的访问内核。
* 系统调用的实现方式为软中断

```c
用户程序首先将系统调用号填充到某个寄存器--->执行软中断指令--->该指令导致处理器收到软中断异常--->导致跳转到异常向量表中执行--->从异常向量表跳转到真正的软中断异常处理代码执行--->在软中断异常处理代码中找到系统调用号--->根据系统调用号调用内核中对应的函数--->并把函数的执行结果返回给用户空间
```

* 系统调用号：arch/arm/include/asm/unistd.h

### 二、库函数

### 三、标准C语言库函数；GUNC语言库函数（glibc）

### 四、系统调用和库函数的错误

### 五、在内核中增加一个新的系统调用

## 第二章 Linux中的文件IO

### 一、文件操作的主要API

#### 1.文件操作的主要API

* API是一些由Linux系统提供支持的函数，有应用程序来使用。
* 应用程序通过调用API来调用操作系统的API。
* 学习一个操作系统，就是学习使用这个操作系统的API。

#### 2.Linux常用的文件API

```c
open | close | write | read | lseek
```

#### 3.文件操作的一般步骤

* open打开一个文件，得到一个文件描述符，然后对文件进行读写操作（或其他操作），最后close关闭文件即可。
* 文件平时是存放在文件系统中的块设备，我们把这种文件叫做静态文件。当我们去open打开一个文件时，Linux的内核操作包括：在进程中建立一个打开文件的数据结构，记录下打开的这个文件，然后申请一块内存，将静态文件的内容从块设备读取到内存中的特定地址管理存放，成为动态文件。
* 文件打开后，针对这份文件的读写操作，都是针对这份动态文件的。当我们close关闭动态文件时，内核将内存中的动态文件更新到块设备中静态文件。
* 常见现象：打开一个大文件时比较慢；写了一半的文件如果没保存直接关机，重启后文件内容丢失。
* 为什么要这么设及？因为块设备本身有读写限制，对块设备的操作非常不灵活。而内存可以按字节为单位操作，而且可以随时操作，很灵活。

#### 4.重要概念：文件描述符

* 文件描述符实际上是一个数字，在进程中表示一个特定的含义，当我们open打开一个文件时，操作系统在内存中构建了一些数据结构来表示这个动态文件，然后返回给应用程序一个数字作为文件描述符，即该进程中该文件的标识。
* 注意文件描述符的作用域就是该进程，出了当前进程这个文件描述符就没有意义了。

### 二、一个简单的文件读写实例

#### 1. 文件的打开与文件的关闭

* Linux中的文件描述符fd的合法范围是0或则一个正整数，不可能是一个负数。
* open返回的fd必须用记录好，对文件的所有操作都离不开fd。

#### 2. 实时查询man手册

* man 1 xxx查询Linux shell命令、man 2 xxx查询API、man 3 xxx 查询库函数

#### 3. 读取文件内容

* ssize_t read(int fd, void *buf, size_t count);ssize_t是内核重定义的的一个类型，其实就是int，返回值表示成功读取的字节数；fd表示文件描述符，buf是应用程序自己提供的一段缓冲区，用来存储读出的内容；count是要读取的字节数。

#### 4. 向文件中写入内容

* ssize_t write(int fd, const void *buf, size_t count)

### 三、open函数的flags（第二个参数）详解

#### 1. 读写权限

* ```c
  O_RDONLY | O_WRONLY | O_RDWR
  ```

#### 2. 打开存在并有内容的文件时，对原内容的处理：O_APPEND、O_TRUNC

* O_APPEND：原有内容保留，将新内容添加在后面
* O_TRUNC：原内容全部删除，新内容重头开始添加
* 如果O_APPEND | O_TRUNC 则为O_TRUNC的效果。
* 不使用这两个的时候，若不读不写或则读，则原内容保持不变，若写，则原内容被逐个替代。

#### 3. 退出程序：exit(库函数)、_exit(API)、\_Exit(API)

* 当我们的程序在前面的某个操作失败导致后面的操作都无法进行的时候，应该及时退出整个程序。
* 如何退出程序：在main中用return，一般正常return 0 ，异常return -1。正式终止进程应该使用exit\\_exit\\\_Exit三者之一。

#### 4.打开不存在的文件时：O_CREAT、O_EXCL

* 当我们打开一个不存在的文件时，一般会报错，若此时加上O_CREAT，则无论该文件存在与否，都会直接创建一个新文件。
* 但O_CREAT会带来一个新问题，当我们打错文件名字的时候，会导致别的文件被创建，即原内容被清空。
* 若此时加上O_EXCL，则对于文件已经存在的情况下，会报错而不是重新创建。
* 当使用O_CREAT去创建一个新文件时，可以使用open函数的第三个参数mode来指定该文件的权限。

#### 5. 以非阻塞的方式打开设备文件：O_NONBLOCK

* 阻塞和非阻塞：如果一个函数是阻塞式的，则调用这个函数时可能因为函数执行条件未具备而被卡住一直等待。如果他是非阻塞的，那么调用函数后会立即返回，但函数有没有完成任务不确定。
* 阻塞式的结果有保障但时间没有保障，非阻塞式的时间有保障但结果没有保障。
* 操作系统提供的API和由API封装而成的库函数，有很多本身被设计为阻塞式或则非阻塞式的，使用我们调用这些函数时要心里有数。
* 该flag只能用于设备文件，不能用于普通文件。

#### 6.写阻塞等待：O_SYNC

* write阻塞等待底层完成写入才返回到应用层。
* 无O_SYNC时write只是将内容写入底层缓冲区则返回，然后底层再合适的时候再将内容一次性同步到硬盘中。这种设计是为了提高硬件操作的效率以及硬件的寿命，但有时我们需要不到等待立即写入，则可用O_SYNC。

### 四、文件读写的一些细节

#### 1.errno和perror

* errno就是error number，即错误码
* errno实质上就是一个数字，每个数字对应一种错误。
* perror就是print error 即打印错误。

#### 2. read和write的count

* count表示我们想要读或则写的字节数，返回值表示实际完成的字节数。
* count在和阻塞、非阻塞结合起来，就会更加复杂，如果一个函数是阻塞式的，我们想要读取30个字节，而实际只完成了20个字节，就会导致阻塞。
* 有时候我们想要写正是程序时，要读出或则写入的往往是一个很大的文件（eg:2M）我们不可能把count设置成2\*1024\*1024，而是把count设置成为一个合适的数字（eg:2048）然后通过多次读写来完成任务。

#### 3.文件IO和标准IO的效率

* 文件IO就是指open、read、write、close等API函数构成的一套用来读写文件的体系，他能完成读写，但效率不高。
* 应用层c语言库函数提供了一套用来读写文件的函数列表，叫标准IO，标准IO由一系列C库函数（fopen、fread、fwrite、fclose）构成，这些标准IO都是由文件IO封装而来，其实就是再应用层加了一个缓冲机制，这样我们通过fwrite写入的内容不是直接进入内核中的buf，而是进入了应用层标准IO库自己维护的buf中，然后标准IO库更具操作系统单次write的最佳count来选择合适的时候将内容写入内核的buf中。

### 五、Linux如何管理文件

#### 1. 硬盘中的静态文件和inode（i节点）

* 硬盘（块设备）-> block -> 扇区 -> 字节
* 一块硬盘可以分为两个区域：硬盘内容管理表、真正的存储内容的区域。操作系统访问硬盘时先去读取硬盘的管理表，从中找到对应文件的扇区信息，然后再通过这个信息去查询真正存储内容的区域。
* 管理表中每个文件对应一个inode，每个inode有一个数字编号，对应一个结构体，结构体中记录了该文件的各种信息。
* 格式化u盘，一般有两种方法，一种是删除内容管理表，一种是删除所有内容。

#### 2.内存中的动态文件和vnode(v节点)

* 一个程序的运行就是一个进程，进程中打开的文件就是该进程。每个进程都有一个进程信息表记录了这个进程的所有信息，表中有一个指针会指向一个文件管理表，文件管理表记录了这个进程的所有信息，通过文件信息中的文件描述符fd就可以找到特定文件的vnode。
* 一个vnode记录了一个被打开的文件的各种信息。

#### 3. 文件与流的概念

* 文件操作中，一个文件中很多个字符的数据被挨个读出或写入时，就构成了一个字符流。
* 流的概念是动态的。
* 编程中提到流这个概念，一般都是IO相关的，所以经常叫IO流，文件操作时就构成了一个IO流。

### 六、lseek函数详解

#### 1.lseek函数介绍

* 文件指针：动态文件在内存中是文件流的形式，文件流很长，有很多个字节，文件指针指出了当前正在操作的位置，就如GUI下的光标一样。
* lseek函数：文件指针是vnode的一个元素，不能直接访问，需要用lseek函数来访问这个指针。
* 当我们打开一个文件时，文件指针默认指向文件流的开始，我们也可以通过lseek来移动文件指针的位置。
* 如果先对一个文件写入若干个字节的内容后立即去读取的话，是读取不到刚写入的内容的，因为此时文件指针已经被write移动到了该内容的后面。

#### 2.用lseek计算文件长度

#### 3.用lseek构建空洞文件

* 空洞文件就是这个文件中有一段是空的，而普通文件是不能有空的，因为我们write时文件指针是依次从前往后移动的。
* 打开一个文件后，用lseek往后移动一段距离，在write写入，就会构成一个空洞文件。
* 空洞文件对多线程操作文件是及其有用的，有时候需要创建一个很大的文件，如果从头开始构建的时间很长，就可以用多个线程来同时对文件进行写入构建。

### 七、多次打开同一文件与O_APPEND

#### 1.重复打开同一文件读取

* 一个进程中两次打开同一文件，然后分别读取，结果是fd1和fd2分别读，两个文件指针相互独立。
* 文件指针是包含在动态文件的文件管理表中的，可以看出Linux系统的进程中不同的fd对应着不同的独立的文件管理表。

#### 2. 重复打开同一文件写入

* 一个进程中两次打开同一文件，然后分别写入，结果是：两个文件指针相互独立。
* 若希望接续写而不是分别写，在open时加入O_APPEND就可以了，O_TRUNC | O_APPEND也可以生效。

#### 3.O_APPEND的实现原理和其原子操作性说明

* O_APPEND可以让write多做一件事，就是移动自己的文件指针的同时也移动别人的文件指针。
* O_APPEND对文件指针的影响是原子性的。
* 原子操作的含义：这个操作一旦开始就不会被打断，必须等待其操作结束其他代码才能运行。

### 八、文件共享的实现方式

#### 1.什么是文件共享

* 文件共享就是同一个文件（同一个inode、vnode）被多个独立的读写体去同时操作。
* 文件共享的意义：可以通过文件共享来实现多线程同时操作一个大文件，以减少文件的读写时间，提升效率。

#### 2.文件共享的三种情况

* 文件共享的核心是如何实现多个文件描述符指向同一个文件。
* 1.同一个进程多次使用（同时）open打开同一个文件；2.在不同的进程中分别使用（同时）open打开同一个文件；3.使用Linux提供的dup和dup2来复制文件描述符。
* 分析文件共享的核心在于：分别写/读还是接续写/读。

#### 3.再论文案件描述符

* 文件描述符本质上是一个数字，这个数字是进程表中的文件描述表（数组）的一个表项，通过该表项可查找得到文件管理表指针，进而可以访问这个文件对应的文件管理表。
* 操作系统规定：fd从0开始分配，内核会从文件描述表中挑选一个未经使用的最小fd返回。
* fd中的0、1、2已经被默认被系统用占用了，分别对应三个文件stdin、stdout、stderr。因此用户进程得到的最小文件描述符是3。
* printf函数默认输出到stdout上，fprintf可以指定输出到那个文件描述符上。

### 九、文件描述符的复制--dup和dup2

#### 1.使用dup进行文件描述的复制

#### 2.使用dup的缺陷

* 不能制定新的文件描述符数字，只能由系统自动分配。

#### 3.使用dup2进行文件描述的复制

* dup2支持指定新的文件描述符数字。
* 利用dup2复制的文件描述符更原来的描述符进行交叉读写时，结果为接续读写而不是分别读写。

#### 4.重定位命令 > 

### 十、fcntl系统调用介绍

#### 1.fcntl的原型和作用

* fcntl是一个多功能文件管理工具，接收2个参数加1个变参。第一个参数是所需要操作文件描述符。第二个参数是cmd，表示要进行哪些操作。第三个arg变参是用来配合cmd使用的。

#### 2.fcntl常见的cmd



### 十一、标准IO库介绍

#### 1.标准IO和文件IO的区别

* 标准IO是C库函数,文件IO是API
* C库函数由API封装而来，比API好用。
* C库函数具有可移植性而API不具备。
* 性能和易用性上，C库函数更好一些，文件IO不带缓存，标准IO带缓存，标准IO比文件IO性能更高。

#### 2.常见标准IO介绍

* fopen、fread、fwrite、fclose、fseek

#### 3.一个简单的标准IO读写实例

## 第三章 文件属性  

### 一、Linux中各种文件类型

### 二、常用文件属性获取

### 三、stat函数应用实例

### 四、文件权限管理

### 五、读取目录文件


## 第四章 获取系统和进程信息

### 一、关于时间的概念

### 二、Linux系统中的时间

### 三、Linux中使用随机数

### 四、proc文件系统介绍

### 五、系统标识：uname()


## 第五章 进程
### 一、程序的开始和结束

#### 1.main函数由谁调用

* 编译链接时的引导代码：操作系统其实在执行main之前也需要执行一段引导程序，编译链接时由链接器将编译器中事先准备好的引导代码给链接进去，跟应用程序构成最终的可执行程序。

* 运行时的加载器：当我们执行一个程序时，加载器负责将代码加载到内存中去执行。

* 程序在编译链接时用连接器（连接引导代码），运行时用加载器（加载到内存）。

* argc和argv的传参的实现涉及到引导代码、加载器。

#### 2.程序如何结束

* 正常终止：return、exit、_exit。
* 非正常终止：自己或他人发送非正常信号终止进程。

#### 3.atexit注册进程终止处理函数

### 二、进程环境

#### 1.环境变量

* export命令可查看环境变量
* 进程环境表：每一个进程都有一份环境变量构成的表格，当前进程中可以直接使用这些环境变量。进程环境表其实就是一个字符串数组，用environ变量指向它。
* 程序中通过environ全局变量使用环境变量。
* 我们写的程序可以无条件使用环境变量，一旦使用到了环境变量，那么程序就和操作系统有关了。
* 获取指定环境变量函数：getenv()。

#### 2.进程运行的虚拟地址空间

* 操作系统中每个进程早独立的地址空间中运行。
* 每个进程的逻辑地址空间均为4GB（32位操作系统）,其中0~1G位OS占用，1~4为应用程序占用。
* 操作系统会完成虚拟地址到物理地址的映射。
* 意义：进程隔离；提供多进程同时运行的环境。

### 三、进程的正式引入

#### 1.什么是进程

* 进程就是程序的一次运行过程，他是一个动态过程而不是静态实物。
* 进程控制块（PCB）是内核中专门用来管理一个进程的数据结构。

#### 2.进程ID

* 进程ID用来唯一标识一个进程，便于管理，位于PCB中。

* 常用获取进程ID的函数：

  ```c
  getpid()//获取当前进程ID
  getppid()//获取当前进程的父进程ID
  getuid()//获取用户ID
  geteuid()//获取有效用户ID
  getgid()//获取组ID
  getefid()//获取有效组ID
  ```

  

#### 3.多进程调度原理

* 操作系统通过合理调节同时运行多个进程，宏观上看是并行，微观上是串行。
* 实际上现代操作系统的最小调度单元是线程而不是进程。

### 四、fork系统调用创建子进程

#### 1.为什么要创建子进程

* 每一次程序的运行都需要一个进程，需要创建多个进程实现宏观上的并行

#### 2.fork系统调用的内部原理

* 进程的分裂生长模式：如果操作系统需要一个新进程来运行一个程序，那么操作系统就会用一个现有的进程来复制生成一个新进程，老进程叫父进程，新进程叫子进程。

* fork系统调用一次会返回两次，使用fork后要用if判断返回值，返回值等于0的就是子进程，返回值大于0的就是父进程。

* fork的返回值在父进程中等于本次创建的子进程ID，在子进程中等于0。

  ``` c
  #include<sys/type.h>
  #include<unistd.h>
  
  int main()
  {
      pid_t p = -1;
      p = fork();//返回两次
      if(p == 0)
      {
          //子进程
      }
      if(p > 0)
      {
          //父进程
      }
      if(p < 0)
      {
          //出错
      }
  
      return 0;
  }
  ```

  

#### 3.关于子进程

* 子进程由父进程复制而来，由自己独立的PCB，被内核同等调度。

### 五、父子进程对文件的操作

#### 1.子进程继承父进程中打开的文件

* 测试上下文：父进程先打开一个文件得到fd，然后在创建子进程，之后在父子进程中各自用write向fd写入内容。结果为接续写，原因是父子进程之间的fd对应的文件指针是彼此关联的（类似O_APPEND）。
* 实际测试时有时候会只看到某一进程写入的内容，原因时是该进程开始之前，另一进程已经结束了，导致文件被强制关闭，该进程打开后又因为O_TRUNC将内容清空了。

#### 2.父子进程各自打开文件实现文件共享

* 测试上下文：父进程open打开1.txt然后写入，子进程open打开1.txt然后写入。结果为分别写，原因是父子进程分离后才各自打开文件，这时候两个PCB已经相互独立，文件表也独立了，因此两次读写是完全独立的。
* 若在上诉测试中open时加入O_APPEND则可以把父子进程的文件指针关联起来，实现接续写。

#### 3.总结

* 父子进程会有关联：父进程在没有fork之前自己做的事情对子进程有很大影响，但是父进程fork之后自己在if里面做的事情对子进程就没有影响了。本质是fork内部实际上已经复制了父进程的PCB生成了子进程的PCB，两个进程已经独立被OS调度运行。
* 我们创建子进程的目的就是要去独立运行其他程序，否则创建子进程没有意义。

### 六、进程的诞生和消亡

#### 1.进程的诞生

* 进程0和进程1：进程0是由内核构建的，进程1是由进程0fork创建来的。
* 其他进程：都是由fork或则vfork创建而来的

#### 2.进程的消亡

* 正常终止和异常终止：进程正常运行然后由自己主动结束和被其他程序被动结束。
* 进程运行时需要消耗系统资源（内存、IO），进程终止时里应完全释放这些资源，如果没有释放这些资源，就丢失了。
* Linux系统设计时规定：每一个进程退出时，操作系统将自动回收这个进程涉及到的所有资源（malloc、open）,但是只回收了内存和IO，并没有回收这个进程本身占用的内存（主要是task_struct和栈内存）
* 每一个进程都需要一个来帮他释放本身占用的内存的过程，这个进程就是他的父进程。

#### 3.僵尸进程

* 僵尸进程即子进程先于父进程结束，但是父进程暂时还没有帮其释放资源，此时子进程成为僵尸进程。但子进程出task_struct和栈内存外其他空间皆已被清理。
* 父进程可以使用wait和waitpid以显示的方式回收子进程剩余的资源并获取子进程退出状态。
* 若父进程不显示回收子进程，当父进程结束时一样会回收已消亡的子进程的剩余资源。

#### 4.孤儿进程

* 父进程先于子进程结束，此时子进程成为一个孤儿进程。
* Linux系统规定：所有的孤儿进程都成为一个特殊的进程（进程1，也就是init进程）的子进程。

### 七、父进程wait（系统调用）回收子进程

#### 1.wait工作原理

* 子进程结束时，系统向其父进程发送SIGCHILD信号，父进程被唤醒后去回收僵尸子进程资源。
* 父进程调用wait后就进入阻塞状态，直到进程资源回收完成。但若父进程没有任何子进程则wait返回错误。
* 父子进程之间是异步的，SIGCHILD信号机制就是为了解决异步的父子进程之间的通信问题。

#### 2.wait编程实战

* wait原型pid_t wait(int *wstatus),参数\*wstatus用来返回子进程结束时的状态，父进程得到wait后再结合一些宏定义就可以得到关于子进程结束时的一些信息。
* wait的返回值pid_t就是本次回收的子进程的PID。即父进程wait回收子进程后可以得到子进程的PID以及子进程结束时的状态。

### 八、waitpid系统调用介绍

#### 1.waitpid和wait差别

* 基本功能一样，都是用来回收子进程。
* wait可以回收指定PID的子进程，也可以选择阻塞式或非阻塞式两种模式。

#### 2.wait原型介绍

* pid_t waitpid(pid_t pid, int *wstatus, int options),返回的pid_t为回收的子进程的PID。
* 输入的pid_t为指定的要回收的子进程ID，若为-1则不指定PID。
* 输入的\*wstatus用于接收子进程状态，options则为阻塞或非阻塞选项，0表示默认的阻塞方式，WNOHANG则表示非阻塞式。

#### 3.代码实例

```c

```

#### 4.竞态初步映入

* 竞态即竞争状态，多进程环境下，多个进程同时抢占系统资源（内存、CPU、文件IO）。
* 竞争状态对OS来说是很危险的，如果没有处理好就会造成结果不稳定。
* 操作系统提供了一系列消灭竞态的机制，我们需要在合适的地方使用合适的方式来消灭竞态。

### 九、exec函数族

#### 1.为什么需要exec族函数

* fork系统调用是为了执行新的程序，但需要直接在子进程的if里写入新的程序代码，这样不够灵活，
* 使用exec族函数可以运行新的可执行程序，即把一个编译好的可执行程序直接加载运行。

### 十、进程状态和system函数

#### 1.进程的5种状态

* 就绪态
* 运行态
* 僵尸态
* 等待态
* 暂停态

#### 2.进程各种状态之间的转换

#### 3.system函数简介

* system函数 = fork + exec系列函数，其原型为int system(const char *string)。

* system函数是原子操作，即整个过程一旦开始就不能被打断（没有竞争状态），直到执行完毕。但若时间过长可能会影响操作系统整体的实时性。

* 使用system调用ls

  ```shell
  system("ls -al /etc/passwd/etc/shadow");
  ```

### 十一、进程关系

* 无关系
* 父子关系
* 进程组（group）:由若干个进程构成一个进程组，组内共享一些文件。
* 会话（session）：会话就是进程组的组。

### 十二、守护进程的引入

#### 1.进程查看命令ps

```shell
ps -ajx #偏向于显示与进程有关的各种ID号
ps -aux #偏向于显示进程占用的各种资源。
```

#### 2.向进程发送信号指令：kill

* kill-信号编号 进程ID：向特定进程发送一个特定信号。
* kill -9 XXX：向XXX这个进程发送9信号，即结束进程。

#### 3. 守护进程

* 守护进程：用daemon表示守护进程，简称为d，进程名后带d的进本上就是守护进程。
* 守护进程的特征：
  * 长期运行：一般都是开机运行到关机。
  * 与中控台脱离（终端被关闭）
  * 服务器一般都为守护进程：服务器程序就是一个一直运行的程序，可以为我们提供某种服务。

#### 4.常见守护进程

* syslogd:系统日志守护进程，提供syslog功能。
* cron：用来实现操作系统的时间管理，Linux中实现定时功能就需要用到cron。

### 十三、编写简单守护进程

### 十四、使用syslog来记录调试信息
### 十五、让程序单列运行
### 十六、Linux的进程间通信介绍
### 十七、管道
### 十八、SystemV IPC介绍

### 十九、进程状态一览

### 二十、Linux进程调度

### 二十一、Linux如何实现进程调度

## 第六章、进程间通信

 		![进程间通信](https://github.com/Sarainco/yuji/blob/main/img_tool/Linux/%E8%BF%9B%E7%A8%8B%E9%97%B4%E9%80%9A%E4%BF%A1.png?raw=true)

### 一、共享内存

### 二、管道和FIFO

### 三、消息队列

#### 1.System V消息队列

#### 2.POSIX消息队列

POSIX消息队列与System V消息队列的相似之处在于数据的交换单位是整个消息，它允许进程之间以消息的形式交换数据。

* POSIX消息队列是引用式计数的。只有当所有当前使用队列的进程都关闭了队列之后才会对队列进行标记以便删除。
* 每个System V消息都有一个整数类型，并且通过msgrcv()可以以各种方式类选择消息。与之形成鲜明对比的是，POSIX消息有一个关联的优先级，并且消息之间是严格按照优先级顺序排队的。
* POSIX消息队列提供了一个特性允许在队列中的一条消息可用异步通知进程。

##### 2.1POSIX消息队列API中主要函数

* mq_open()函数创建一个新消息队列或者打开一个既有队列，返回后续调用中会用到的消息队列描述符。
* mq_send()函数像队列写入一条消息。
* mq_receive()函数从队列中读取一条消息。
* mq_close()函数关闭进程之前打开的一个消息队列。
* mq_unlink()函数删除一个消息队列名并当所有进程关闭该队列时对队列标记以便删除。
* 每个消息队列都有一组关联的特性，其中一些特性可以在使用mq_open()船舰或打开队列时进行设置。获取和修改队列特性的工作则由两个函数来完成：mq_getattr()和mq_setattr()。
* mq_notify()函数允许一个进程向一个队列注册接收消息通知。在注册完之后，当一条消息可用是会通过发送一个信号或则在一个单独的线程中调用一个函数来进行通知进程。

##### 2.2 打开、关闭和断开链接消息队列

##### 2.3 描述符和消息队列之间的关系

##### 2.4 消息队列特性

##### 2.5 交换消息

##### 2.6 消息通知

##### 2.7 Linux特有的特性

### 四、信号量

### 五、IPC对象和IPC Key


## 第七章、Linux中的信号

### 一、什么是信号

#### 1.信号的本质

信号是在软件层面上对中断机制的一种模拟，在原理上，一个进程收到一个信号与处理器收到一个中断请求是一样的。信号是异步的，一个进程不必通过任何操作来等待信号的到达，事实上，进程也不知道信号什么时候到达。

信号是进程间通信中唯一的异步通信机制，可以看作异步通知，通知接收信号的进程有哪些事件发生了。信号机制经过POSIX实时扩展后，功能更加强大，除了基本通知功能外，还可以传递附加信息。

#### 2.信号的来源

信号事件的发生有两个来源

* 硬件来源（比如按下了键盘或者其他硬件故障）；
* 软件来源，最常用发送信号的系统函数是kill、raise、alarm、setitimer,软件来源还包括一些非法运算操作等。

### 二、常见信号介绍

可以从两个不同的分类角度对信号进行划分

* 可靠性方面，可分为可靠信号和不可靠信号
* 时间关系上，可分为实时信号和非实时信号

### 三、进程对信号的处理

#### 1.信号发送

#### 2.自定义信号处理方式

#### 3.信号集操作


## 第八章、定时器与休眠

### 一、间隔式定时器

### 二、定时器的调度与精度

### 三、为阻塞操作设置超时

### 四、暂停运行（休眠）一段固定时间

### 五、POSIX时钟

### 六、POSIX间隔式定时器

### 七、利用文件描述符进行通知的定时器：timerfd API

## 第九章、Linux线程全解

### 一、再论进程
#### 1.多进程实现同实读取键盘和鼠标
* 创建子进程，在父子进程中分别进行读鼠标和键盘的工作

#### 2.使用进程的优势
* CPU分时复用,单核心CPU可以宏观上实现的多任务并行

#### 3.使用进程技术的劣势
* 进程间切换开销大进程间通信麻烦且效率低下

#### 4.解决方案----线程技术
* 线程技术保留进程实现多任务的特性线程的改进就是线程切间切换和通信上提升效率，继承线程的优点，克服线程的缺点多线程中多核心CPU更具优势，因为每个核心可以处理一个线程

### 二、线程的引入

#### 1.使用线程技术同时读取键盘和鼠标

#### 2. Linux中的线程简介

* 线程是一种轻量级进程
* 线程是参与内核调度的最小单元
* 一个进程中可以有多个线程

#### 3.线程技术的优势

* 像进程一样被OS调度
* 同一进程的多个线程之间很容易进行高效通信
* 在多核心CPU架构下效率最大化

#### 4.线程间通信方式

* 信号：用pthread_kill对线程发送信号，目标线程用sigaction来处理。
* 信号量：用于同步，一个线程阻塞等待另一个线程给他发信号量才会继续执行。
* 锁机制：互斥锁（用于保护共享数据或保持互斥操作）；条件变量（与信号量类似，与互斥锁一起使用）；自旋锁；读写锁；

### 三、线程常见函数

#### 1.线程的创建和回收

* pthread_create：主线程用来创建子线程
* pthread_join：主线程用来阻塞等待回收子线程
* pthread_detach：主线程用来分离线程，分离后主线程不必再去回收子线程

#### 2.线程取消

* pthread_cancel：线程调用该函数去取消（结束）子线程
* pthread_setcancelstate：子线程用来设置自己是否允许被取消
* pthread_setcanceltype：子线程用来设置自己取消类型（立即取消or等执行到cancellation point的函数时才会取消）

#### 3. 线程函数退出

* pthread_exit | return：都可以用于子线程退出，注意exit是用于进程退出的。
* pthread_cleanup_push和pthread_cleanup_pop：线程退出时需要调用的清理函数

#### 4.获取线程ID

* pthread_self

#### 5.发送信号

* pthraed_kill

### 四、线程同步之信号量

### 五、线程同步之互斥锁

### 六、线程同步之条件变量

#### 1.什么是条件变量

* 条件变量是利用线程间共享的全局变量进行同步的一种机制，主要包括两个动作：一个线程阻塞等待条件成立而挂起，另一个线程使条件成立
* 为了防止竞争，条件变量的使用总是和互斥锁结合在一起。

#### 2.相关函数

* pthread_cond_init
* pthread_cond_destroy
* pthread_cond_wait
* pthread_cond_signal/pthread_cond_boardcast

## 第十章、Linux网络编程

### 一、Linux网络编程框架

#### 1.网络是分层的

* OSI七层模型：应用层、表示层、会话层、传输层、数据链路层、物理层
* 互联网极其复杂，需要分层以便更好地实现网络通信。

#### 2.TCP/IP协议的引入

* TCP/IP协议是用的最多的网络协议。
* TCP/IP协议分为4层，对迎着OSI的7层：应用层（应用层、表示层、会话层），传输层、网络层、数据链路层（链路层、物理层）
* 网络编程最应该关注应用层，了解传输层，网络层和链路层不用管。

#### 3.BS和CS

* BS架构：broswer-server。浏览器-服务器架构
* CS架构：client-server。客户端-服务器架构

### 二、TCP协议的学习

#### 1.关于TCP理解的重点

* TCP协议工作在传输层，对上服务socket接口，对下调用IP层。
* TCP协议面向连接，通信前必须三次握手建立连接关系。
* TCP协议提供可靠传输，不怕丢包、乱序问题

#### 2.TCP如何保证可靠传输

###  一、网络基本常识

#### 1.七层网络协议模型

* 应用层 - 主要用于将数据交给应用程序
* 表示层 - 主要用于按照统一的格式进行数据的封装
* 会话层 - 主要用于控制会话的建立、关闭等操作
* 传输层 - 主要用于数据的检查和重新排序等
* 网络层 - 主要用于选择具体的网络协议再次封装和发送
* 数据链路层 - 主要用从将数据转换为高低电平信号等
* 物理层 - 主要用于交换机等设备网络

#### 2.常见的网络协议

* tcp协议 - 传输控制协议，是一种面向连接的协议
* udp协议 - 用户数据报协议，是一种非面向连接的协议
* ip协议 - 互联网协议，是上述两种协议的底层协议

#### 3.IP地址和子网掩码

* IP地址 - 是互联网中的唯一标识，其本质上就是一个由32位二进制组成的整数，也有128位二进制组成的整数，日常生活中采用点分十进制表示法来描述IP地址，也就是将每一个字节的二进制转换为一个十进制的整数，不同的整数之间采用小数点分割。

* ```c
  A类：0 + 7位网络地址 + 24位主机地址
  B类：10 + 14位网络地址 + 16位主机地址
  C类：110 + 21位网络地址 +8位主机地址
  D类：1110 + 28位多播地址
  ```

#### 4.子网掩码

* 主要用于划分IP地址中的网络地址和主机地址，以及判断两个IP地址是否在同一个子网中，具体的划分方法为：IP地址 & 子网掩码 = 网络地址+ 主机地址

#### 5.端口号和字节序

* IP地址 - 定位到具体的某一台主机/设备
* 端口号 - 定位到主机上的具体某个进程
* 网络编程中需要提供：IP地址 + 端口号；端口号本质上是unsigned short类型，范围是0~65535，其中0~1024之间的端口被系统调用占用，因此编程从1025开始使用。

#### 5.字节序

* 小端系统：主要指将地位数据存放到地位内存地址的系统
* 大端系统：主要是指将低位数据存放在高位内存地址的系统
* 一般来说，对于所有发送到网络中的多字节整数来说，先转化为网络字节序在发送，而对于所有从网络中接收到的多字节整数来说，需要先转化为主机字节序在解析，二网络字节序本质上就是大端字节序
* htons 主机字节序转化为网络字节序

## 第十一章、嵌入式程序的安全

### 一、千奇百怪的BUG

#### 1.数组越界

```c
void buf_overflow(int n, char val)
{
    volatile char buf[5];//内存分配的时遵循地址对齐原则，分配8个字节
    volatile int a = 0x55;
    buf[0] = 0x5a;
    a++;
    buf[n] = val;
    printf("buf[0] =  0x%x a = 0x%x\n", buf[0], a);//buf[0] =  0x5a a = 0x56 || 段错误 (核心已转储)
}//函数返回地址也在栈空间
```

* 变量被修改（不一定，分配内存存在内存对齐）
* 返回地址被修改，可能会执行到别的函数（返回值刚好是另一个函数的地址）  
* 远在天边的函数变量被修改

#### 2.未赋值的函数指针

```c
int (*f) (int a, int b)//定义函数指针 f = 0x00000000
```

#### 3.加不加打印效果不一样

* 一个直接用变量值，一个去栈里面拿值用

#### 4.加上for循环就没问题了

* 主要是因为定义了一个新的变量，在栈区

### 二、栈溢出

* 栈溢出是最常见、危害最大的软件漏洞之一；要理解栈溢出攻击的原理，需要对计算机程序中函数的调用和返回过程的底层细节有清晰的理解，尤其时这样的一个问题：计算机是如何保存某个函数调用的吓一跳指令地址，实现函数执行完成之后跳回函数调用处，继续往下执行的？

#### 1.什么是栈溢出

* 在这里存在漏洞的关键在于，函数返回后，继续执行的下一条指令地址是从内存的某一个位置读出来的，如果内存中的这个值在函数返回之前被修改，那么程序的执行流程就会被修改。
* 而造成这个值被修改的最常见的原因就是对函数内某个局部数组的操作，发生了越界，如果越界范围超过了当前函数栈帧的尾部，就会吧之前爆粗不在栈中的RBP（基址指针）和RIP（指令指针）值给修改掉。
* 在大多数情况下，栈溢出会导致层序在函数返回后崩溃，但是，如果精心构造栈溢出数据，把保存在RIP值的内存位置修改成某个特定的值，就可以实现控制代码执行过程，让他执行自己定于的一段代码的效果
* 例如，如下的一段代码就是典型的栈溢出漏洞的程序，她把外部的输入复制到一个局部缓冲区，燃火执行一些自己的逻辑，但是并没有严格的检查数据输入的长度，就使得外部精心构造的输入数据可以控制check_param函数返回之后的执行流程

```c
void check_param(char *param)
{
    char buffer[128];
    strcpy(buffer,param);
    ......
}

int main()
{
    check_param(argv[1]);
    ......
}
```

#### 2.如何防止栈溢出

* 从上面的分析可以看出，只要对栈内数组操作的边界执行严格仔细的检查，杜绝任何数组越界访问的行为，就可以阻止栈溢出攻击
* 在上面的程序中，只需要把strcpy改成具有边界检查功能的strncpy，就可以堵住这个漏洞

```c
void check_param(char *param)
{
    char buffer[128];
    strncpy(buffer, param, sizeof(buffer));
    ......
}
```

### 三、堆溢出

#### 1.什么是堆溢出

* 与栈溢出相似，分配在堆上的内存也会发生溢出，溢出之后也会修改掉一些本不该被修改的数据。只是利用堆溢出控制程序执行流程的过程更加复杂

* 这是最常见的对内存溢出的攻击方式，要理解这种攻击方式的实现原理，需要先理解一下glibc的堆内存管理策略。当用户通过glibc的动态内存分配函数申请一块内存时，库函数不会内茨都去向操作系统请求同样大小的内存，而实会一次性申请一块相对较大的内存，然后把这块内存做些分割，并把合适大小的一块返回给应用程序。当应用程序再次申请时，库函数会先在已经从操作系统拿到的那些内存块中查找是否已经有能满足要求的内存块，如果有就直接返回。

* 应用程序持续不断的申请和释放内存块，库函数还要尽量避免内存碎片的产生，所以，当用户释放一块内存时，他会检查该块内存的前后两块相邻内存的空闲状态，如果他们也是空闲的，就把这几块连续的空闲内存合并。而这个合并操作，就是删除双向链表中的某个节点。如下

  ```c
  void unlink(malloc_chunk *P, malloc_chunk *BK, malloc_chunk *FD)
  {
      FD = P->fd;
      BK = P->bk;
      FD->bk = bk;//内存写操作
      BK->fd = FD;//内存写操作
  }
  ```

* 上面是一个典型的从双向链表中删除某个节点的操作，其中P时要删除的中间节点，而BK和FD分别是P节点的后一个和前一个结点。他有两个内存写操作，而写内存的地址和写入的内容，都来至于相邻内存中的数据。所以，如果程序中存在堆溢出缺陷，攻击则就可以用设计的数据，去操作malloc_chunk结构中的指针值，使得在执行这个内存写操作的时候，去修改你希望修改的数据。
* 比如，可以把内存地址修改成计算得到的GOT飙中某个库函数的地址，而写入内存的值修改成某段shellcode的入口地址。这样，当程序在后面调用到这函数之后，就会实际去执行自己提前准备的shellcode，原理很容易理解，但是操作起来就需要高超的技巧和精确的计算。而且，并不是任意大小的堆内存溢出都能够内利用来实现程序的1流程控制，因为在glibc的内存管理策略中，为了加快小块内存的分配和释放效率，大小不超过64字节的内存块使用fastbin组织管理的，而不会被链表在双向链表中，他们在释放之后不会只想前后内存块的合并操作

#### 2.如何防止堆溢出

* 与栈溢出相似，封堵堆溢出的关键在于层序中用到的所有内存操作，都要严格检查操作边界，比如，任何用到memcpy、strcpy等函数的地方，都要对输入数据执行严格的长度检查，保证其不会操作曹处预定义的缓冲区的内存。对于复杂计算得到的数组下标，也要小心处理，确保落在有效范围内。

### 三、格式化字符串漏洞

#### 1.什么是格式化字符串漏洞

* 格式化字符串漏洞产生的原因在于程序没有对外部输入的内容执行严格的检查和过滤，当这样的数据作为参数传递给某些格式化操作函数，如printf、fprintf等时，就可能被恶意利用，比如，一个简单的存在格式化字符串漏洞程序如下

  ```CQL
  int main()
  {
  	char buffer[1024];
  	strcpy(buffer, argv[1], sizeof(buffer) - 1);
  	prinf(buffer);
  	return 0;
  }
  ```

* 在这个程序中，虽然使用了带有边界检查功能的strncpy函数执行内存的复制，没有溢出漏洞，但是在使用printf函数输出buffer内容时，并没有对外部输入内容执行任何形式的验证和检查。这样，外部输入的数据中如果带了%s或者%x等格式符时，就会意外的输出一些本不该显示出来的内容。甚至，还可以精心构造带有%n格式化的输入，实现向某个内存地址写入数据，让程序返回时，去执行某段自定义的shellcode，达到目的。

#### 2.如何防止格式化字符串漏洞

* 要防范这种漏洞，只要对输入数据执行严格的检查就可以了。

## 第十二章、Linux内核框架

![This is the frame photo](https://raw.githubusercontent.com/Sarainco/yuji/refs/heads/main/img_tool/Linux/Linux%E5%86%85%E6%A0%B8%E6%A1%86%E6%9E%B6.png)

### 一、Linux内核源码的目录结构

[Linux内核源码目录结构](https://blog.csdn.net/weixin_38715577/article/details/101365039)

### 二、Linux内核的组成部分

	Linux内核主要是由进程调度（SCHED）、内存管理（MM）、虚拟文件系统（VFS）、网络接口（NET）、和进程间通信（IPC）5个子系统组成。组成部分与关系如下图：
	
											![](https://github.com/Sarainco/yuji/blob/main/img_tool/Linux/Linux%E5%86%85%E6%A0%B8%E7%BB%84%E6%88%90%E9%83%A8%E5%88%86%E5%8F%8A%E5%85%B3%E7%B3%BB.png?raw=true)

#### 1.进程调度

#### 2.内存管理

#### 3.虚拟文件系统

#### 4.网络接口

#### 5.进程间通信

### 三、Linux内核空间与用户空间

## 第十三章、驱动开发环境搭建

### 一、Linux内核的编译和加载

#### 1.开发环境搭建

* nfs、tftp

#### 2.Linux内核的引导

#### 3.Linux内核编译

```c
#make config //基于文本的最为传统的配置界面
#make menuconfig //基于文本菜单的配置界面
#make xconfig //要求QT被安装
#make gconfig //要求GTK+被安装
```

##### 3.1Linux内核的配置文件系统有以下三个部分组成

* Makefile：分布在Linux内核源码中，定义Linux内核的编译规则。

* 配置文件（Kconfig）:给用户提供配置选择的功能。

* 配置工具：包括配置命令解释器（对脚本中使用的配置命令进行解释）和配置用户界面（提供字符界面和图形界面）。这些配置工具使用的都是脚本语言，如用Tcl/TK/Perl等。

  使用make menuconfig 、make config等命令后，会生成一个.config配置文件，记录那部分被编译进内核、那部分被编译进内核模块。

  运行make meuconfig等时，配置工具首先分析与体系结构对应的/arch/xxx/Kconfig文件（xxx即为传入的ARCH参数），/arch/xxx/Kconfig文件中出除本身包含的一些与体系结构相关的配置项和配置菜单外，还通过soure语句引入了一系列Kconfig文件，而这些Kconfig又可能再次通过source引入下一层的Kconfig，配置工具依据Kconfig包含的菜单和条目即可。描绘出下图所示的分层结构。

  ![kconfig](https://github.com/Sarainco/yuji/blob/main/img_tool/Linux/Kconfig%E8%8F%9C%E5%8D%95%E9%A1%B9.png?raw=true)

##### 3.2 Kconfig和Makefile

在Linux内核中增加程序需要完成以下3项工作。

* 将编写的源代码复制到Linux内核源码的相应目录中。
* 在目录的Kconfig文件中增加关于新源代码对应项目的编译配置选项。
* 在目录的Makefile文件中增加对新源代码的编译条目。

Kconfig

Makefile

#### 4.根文件系统制作

根文件系统首先是内核启动时所mount(挂载)的第一个文件系统，内核代码映像文件保存在文件系统中，而系统引导启动程序会在根文件系统挂载之后从中把一些基本的初始化脚本和服务等加载到内存中去运行。

##### 4.1 BusyBox构建根文件系统

##### 4.2 Yocto构建根文件系统

##### 4.3 Buildroot构建根文件系统

## 第十四章、Linux内核驱动

### 一、什么是驱动

##### 1.理解驱动的概念

* 内核驱动即软件层面的驱动，指的是这段代码操作了硬件使其动了起来，所以叫硬件的驱动程序。

#### 2.Linux体系架构

* 分层思想，大致可以分为两层，上层是应用，下层是操作系统，驱动、文件系统等程序包含于OS中，上层应用通过API调用OS中中的驱动程序。
* 驱动上面是系统调用API，下面是硬件。
* 驱动本身也是分层的。

### 二、模块化设计

#### 1.微内核和宏内核

* 宏内核又称单内核，将内核从总体上作为一个大过程实现，运行在一个独立的地址空间，所有内核服务程序都在一个地址空间运行，相互之间调用函数，简单高效。
* 微内核各个功能被划分成独立的进程，进程间通过IPC通信，模块化程度高，一个服务失效不会影响到另一个程序。

#### 2.Linux本质上是宏内核，但是又吸收了微内核的模块化特性

* 静态模块化：在编译时实现可裁剪，特征是想要裁剪功能必须重新编译。
* 动态模块化：zImage可以不重新编译烧录。

### 三、Linux设备驱动分类

字符设备、块设备、网络设备（设备本身读写操作的特征差异）

* 字符设备
* 块设备
* 网络设备

### 四、Linux驱动子系统

#### 1.DMA

DMA是一种无需CPU参与就可以让外设与系统内存之间进行双向数据传输的硬件机制。使用DMA可以使系统CPU从实际的I/O数据传输过程中摆脱出来，从而大大提高系统的吞吐率。DMA通常与硬件体系结构，特别是外设总线技术密切相关。

DMA方式数据传输由DMA控制器（DMAC）控制，在传输期间，CPU可以并发的执行其他任务。当DMA结束后，DMAC通过中断通知CPU数据传输已经结束，然后由CPU执行相应的中断服务程序进行后续处理。

##### 1.1 DMA与Cache一致性

##### 1.2 Linux下的DMA编程

#### 2.以太网

##### 2.1 以太网控制及其接口

![](https://github.com/Sarainco/yuji/blob/main/img_tool/Linux/%E7%BD%91%E7%BB%9C%E7%A1%AC%E4%BB%B6%E6%8E%A5%E5%8F%A3%E7%A4%BA%E6%84%8F%E5%9B%BE.png?raw=true)

以太网组成框架

![](https://github.com/Sarainco/yuji/blob/main/img_tool/Linux/%E4%BB%A5%E5%A4%AA%E7%BD%91%E7%BB%84%E6%88%90%E6%A1%86%E6%9E%B6.png?raw=true)

* MAC：通常集成在ARM芯片中，功能类似于一个控制器，以太网协议层数据传送给MAC，由MAC通过DMA发送到外部接口。或者接收从PHY传过来的信号，DMA搬运到内存中存储。
* PHY：通常是一个独立芯片，由数字和模拟两部分，也可以集成在ARM芯片内部，负责把从MAC传送过来的数据转换成可以在网线上传输的信号，或者接受网线上传输过来的信号，转化成数字信号回传给MAC。分为百兆PHY和千兆PHY。
* MAC和PHY间的控制接口为MDIO（Management Data Input/Output）,用于读写每个PHY的控制寄存器和状态寄存器，以达到控制PHY行为和监控PHY状态的目的。其中有两根线，分别为双向的MDIO和单向的MDC。

##### 2.2 相关通信接口

* MII
* RMII
* RGMII
* SGMII

##### 2.3 MDIO接口

![](https://github.com/Sarainco/yuji/blob/main/img_tool/Linux/MDIO%E6%97%B6%E5%BA%8F.png?raw=true)

![](https://github.com/Sarainco/yuji/blob/main/img_tool/Linux/MDIO%E6%97%B6%E5%BA%8F1.png?raw=true)

##### 2.5 RJ45接口

#### 3.PHY芯片详解

##### 3.1 PHY基础知识简介

PHY是IEEE802.3规定的一个标准模块，SOC可以对PHY进行配置或则读取PHY相关状态，这个就需要PHY内部寄存器去实现。PHY芯片寄存器地址空间为5位，地址0~31共32个寄存器，IEEE定义了0~15这16个寄存器功能，16~31这16个寄存器由厂商自行实现。

##### 3.2 LAN8720A详解

这里以ALPHA开发板所使用的LAN8720A这个PHY为例，详细的分析一下PHY芯片的寄存器。

## 第十五章、Linux文件系统

## 第十六章、Linux网络系统

#### 1.Linux网络设备驱动的结构

Linux网络设备驱动程序的体系结构可以分为4层，依次为网络协议接口层、网络设备接口层、提供实际功能的设备驱动功能层以及网络设备与媒介层。

* 网络协议接口层向网络层协议提供统一的数据包收发接口，不论上层协议是ARP还是IP，都通过dev_queue_xmit()函数发送数据，并通过netif_rx()函数接收数据。这一层的存在使得上层协议独立于具体的设备。
* 网络设备接口层向协议接口层提供统一的用于描述具体网络设备属性和操作结构体net_device，该结构体是设备驱动功能层中个函数的容器。实际上，网络设备接口层从宏观上规划了具体操作硬件的设备驱动功能层的结构。
* 设备驱动功能层的个函数是网络设备接口层net_device数据结构的具体成员，是驱使网络设备硬件完成相应动作的程序，它通过hard_start_xmit()函数启动发送操作，并通过网络设备上的中断出发接收操作。
* 网络设备与媒介层是完成数据包发送和接收的物理实体，包括网络适配器和具体的传输媒介，网络适配器被设备驱动功能层中的函数在物理上驱动。对于Linux系统而言，网络设备和媒介都可以是虚拟的。

在设计具体的网络设备驱动是，我们需要完成的主要工作是编写设备驱动功能层的相关函数以填充net_device数据结构的内容并将net_device注册入内核。

##### 1.1 网络协议接口层

网络协议接口层主要的功能是给上层协议提供透明的数据包发送和接收接口。当上层ARP或IP需要发送数据包时，他将调用网络协议接口层的dev_queue_xmit()函数发送该数据包，同时需传递给该函数一个指向struct sk_buff数据结构的指针。dev_queue_xmit()函数的原型为：

```c
int dev_queue_xmit(struct sk_buff *skb);
```

同样的，上层对数据包的接收也通过向netif_rx()函数传递一个struct sk_buff数据结构的指针来完成。netif_rx()函数的原型为：

```c
int netif_rx(struct sk_buff *skb);
```

sk_buff结构体非常重要，它定义于include/linux/skbuff.h文件中，含义为“套接字缓冲区”，用于在Linux网络子系统中的各层之间传递数据，是Linux网络传递的“中枢神经”。

当发送数据包时，Linux内核的网络处理模块必须建立一个包含要传输的数据包的sk_buff，然后将sk_buff递交给下层，各层在sk_buff中添加不同的协议头直至交给网络设备发送。同样地，当网络设备从网络媒介上接收到数据包后，他必须将收到的数据转换为sk_buff数据结构并传递给上层，各层剥去相对应的协议直至交给用户。

```CQL
/**sk_buff结构体中的几个关键的数据成员**/
struct sk_buff {
	union {
		struct {
			/* These two members must be first. */
			struct sk_buff		*next;
			struct sk_buff		*prev;

			union {
				ktime_t		tstamp;
				struct skb_mstamp skb_mstamp;
			};
		};
		struct rb_node	rbnode; /* used in netem & tcp stack */
	};
	struct sock		*sk;
	struct net_device	*dev;

	/*
	 * This is the control buffer. It is free to use for every
	 * layer. Please put your private variables there. If you
	 * want to keep them across layers you have to do a skb_clone()
	 * first. This is owned by whoever has the skb queued ATM.
	 */
	char			cb[48] __aligned(8);

	unsigned long		_skb_refdst;
	void			(*destructor)(struct sk_buff *skb);
#ifdef CONFIG_XFRM
	struct	sec_path	*sp;
#endif
#if defined(CONFIG_NF_CONNTRACK) || defined(CONFIG_NF_CONNTRACK_MODULE)
	struct nf_conntrack	*nfct;
#endif
#if IS_ENABLED(CONFIG_BRIDGE_NETFILTER)
	struct nf_bridge_info	*nf_bridge;
#endif
	unsigned int		len,
				data_len;
	__u16			mac_len,
				hdr_len;

	/* Following fields are _not_ copied in __copy_skb_header()
	 * Note that queue_mapping is here mostly to fill a hole.
	 */
	kmemcheck_bitfield_begin(flags1);
	__u16			queue_mapping;
	__u8			cloned:1,
				nohdr:1,
				fclone:2,
				peeked:1,
				head_frag:1,
				xmit_more:1;
	/* one bit hole */
	kmemcheck_bitfield_end(flags1);

	/* fields enclosed in headers_start/headers_end are copied
	 * using a single memcpy() in __copy_skb_header()
	 */
	/* private: */
	__u32			headers_start[0];
	/* public: */

/* if you move pkt_type around you also must adapt those constants */
#ifdef __BIG_ENDIAN_BITFIELD
#define PKT_TYPE_MAX	(7 << 5)
#else
#define PKT_TYPE_MAX	7
#endif
#define PKT_TYPE_OFFSET()	offsetof(struct sk_buff, __pkt_type_offset)
...
#ifdef CONFIG_NET_SCHED
	__u16			tc_index;	/* traffic control index */
#ifdef CONFIG_NET_CLS_ACT
	__u16			tc_verd;	/* traffic control verdict */
#endif
#endif

	union {
		__wsum		csum;
		struct {
			__u16	csum_start;
			__u16	csum_offset;
		};
	};
	__u32			priority;
	int			skb_iif;
	__u32			hash;
	__be16			vlan_proto;
	__u16			vlan_tci;
#if defined(CONFIG_NET_RX_BUSY_POLL) || defined(CONFIG_XPS)
	union {
		unsigned int	napi_id;
		unsigned int	sender_cpu;
	};
#endif
#ifdef CONFIG_NETWORK_SECMARK
	__u32			secmark;
#endif
	union {
		__u32		mark;
		__u32		dropcount;
		__u32		reserved_tailroom;
	};
...
	/* private: */
	__u32			headers_end[0];
	/* public: */

	/* These elements must be at the end, see alloc_skb() for details.  */
	sk_buff_data_t		tail;
	sk_buff_data_t		end;
	unsigned char		*head,
				*data;
	unsigned int		truesize;
	atomic_t		users;
};
```

尤其值得注意的是head和end指向缓冲区的头部和尾部，而data和tail指向实际数据的头部和尾部。每一层会在head和data之间填充协议头，或在tail和end之间添加新的协议数据。

套接字缓冲区设计的操作函数，Linux套接字缓冲区支持分配、释放、变更等功能函数。

```c
/**Linux内核中用于分配套接字缓冲区的函数**/
struct sk_buff *alloc_skb(unsigned int len, gfp_t priority);
struct sk_buff *dev_alloc_skb(unsigned int len);
/**
*alloc_skb()函数随机分配一个套接字缓冲区和一个数据缓冲区，参数len为数据缓冲区的空间大小，通常以L1_CACHE_BYTES字节对齐，参数priority为内存分配的优先级；dev_alloc_skb()函数以GFP_ATOMIC优先级进行skp的分配，原因是该函数经常在设备驱动的接收中断里被调用**/
```

```c
/**Linux内核中释放套接字缓冲区的函数**/
void kfree_skb(struct sk_buff *skb);
void dev_kfree_skb(struct sk_buff *skb);
void dev_kfree_skb_irq(struct sk_buff *skb);
void dev_kfree_skb_any(struct sk_buff *skb);
/**上述函数用于释放被alloc_skb()函数分配的套接字缓冲区和数据缓冲区**/
/**Linux内核内部使用kfree_skb()函数，而在网络驱动设备驱动程序中最好用dev_kfree_skb()、dev_kfree_skb_irq()或dev_kfree_skb_irq()函数进行套接字缓冲区的释放。其中dev_kfree_skb()函数用于非中断上下文，dev_kfree_skb_irq()函数用于中断上下文，而dev_kfree_skb_any()函数在中断上下文和非中断上下文都可以被采用。**/
void __dev_kfree_skb_any(struct sk_buff *skb, enum skb_free_reason reason)
{
    if(in_irq() || irqs_disabled())
        __dev_kfree_skb_irq(skb, reason);
    else
        dev_kfree_skb(skb);
}
```

```c
/**在Linux内核中在缓冲区尾部增加数据**/
unsigned char *skb_put(struct sk_buff *skb, unsigned int len);
/**它会导致skb->tail后移len，而skb->len会增加len的大小；通常，在设备驱动接收数据处理中会调用该函数。**/

/**在缓冲区开头增加数据**/
unsigned char *skb_push(struct sk_buff *skb, unsigned int len);

/**调整缓冲区的头部**/
static inline void skb_reserve(struct sk_buff *skb, int len);
```

##### 1.2 网络设备接口层

网络设备接口层主要功能是为千变万化的网络设备定义统一、抽象的数据结构net_device结构体，以不变应万变，实现多种硬件在软件层面的统一。

net_device结构体在内核中指代一个网络设备，它定义与include/linux/netdevice.h文件中，网络设备驱动程序只需要通过填充net_device的集体成员并注册net_device即可实现硬件操作函数与内核挂接。

net_device是一个巨大的结构体，包含网络设备的属性描述和操作接口。

```c
/**（1）全局信息**/
char name[IFNAMESIZE];//name是网络设备的名称。
/**（2）硬件信息**/
unsigned long mem_end;
unsigned long mem_start;
/**mem_start和mem_end定义了设备所使用共享内存的起始和结束地址**/
unsigned long base_addr;//网络设备I/O基地址
unsigned char irq;//中断号
unsigned if_port;//指定多端口设备使用哪一个端口，该字段仅针对多端口设备。
unsigned char dma;//指定分配给设备的DMA通道。
/**（3）接口信息**/
unsigned short hard_header_len;//网络设备的硬件头长度
unsigned short type;//硬件类型
unsigned short mtu;//最大传输单元
unsigned char *dev_addr;//存放设备的硬件地址
unsigned short flags;//网络接口标志
/**（4）设备操作函数**/
const struct net_device_ops *netdev_ops;//该结构体是网络设备的一系列硬件操作的集和
/**（5）辅助成员**/
unsigned long trans_start;
unsigned long last_rx;//时间戳
```

##### 1.3 设备驱动功能层

net_device结构体的成员（属性和net_device_ops结构体中的函数指针）需要被设备驱动功能层赋予集体的数值和函数。对于具体的设备xxx，应该编写相应的设备驱动功能层的函数。

由于网络数据包的接受可有中断引发，设备驱动功能层中的另一个主体部分将是中断处理函数，她负责读取硬件上接收到的数据包并传送给上层协议，因此可能需要xxx_interrupt()和xxx_rx()函数，前者完成中断类型判断等基本工作，后者需要完成数据包生成及降级递交给上层等复杂工作。

## 第十七章、Linux内存管理

内存管理的主要作用是控制多个进程安全的共享主内存区域。当CPU提供内存管理单元（MMU）时，Linux内存管理对每个进程完成从虚拟内存到物理内存的转化。

Linux内核的内存管理总体比较庞大，包含底层的Buddy算法，它用于管理每个页的占用情况，内核空间的slab以及用户空间的C库二次管理。另外，内核也提供了页缓存的支持，用内存来缓存磁盘，per-BDI flusher线程用于刷回脏的页缓存到磁盘。Kswapd(交换进程)则是Linux中用于页面回收（包括file-backed的页和匿名页）的内核线程，它采用最近采用最少（LRU）算法进行内存回收。

#### 1.程序和进程的概念

* 程序：存放在磁盘或硬盘上的可执行文件
* 进程：运行在内存中的程序叫做进程；同一个程序可以同时对应多个线程

#### 2.进程中内存区域的划分

#### 3.存放常量字符串不同形式的比较

```c
//pc指向只读常量区，pc本身在栈区
char *pc = "hello";//pc里面记录的是字符串的首地址
//ps指向栈区本身也在栈区
char ps[6] = "hello";//把hello这个字符串复制了一份放在了数组里面
/**
*对于一个记录常量字符串的指针来说，指针指向的内容不可以改变，但是指针的指向可以改变
*对于一个记录常量字符串的字符数组来说，数组指向的内容可以改变，但是指向不可以改变
*对于一个记录动态内存的字符指针来说，指针指向和指针指向的内容都可以改变
**/
```

#### 4.虚拟内存管理技术

在Linux系统中，一般采用虚拟内存管理技术来进行内存空间的管理，即每个进程都可以由0~4G-1的地址空间（虚拟内存），由操作系统负责建立虚拟内存到物理内存的映射，因此不同进程中的虚拟地址看起来是一样的，但是所对应的真是物理内存不一样。其中0~3G-1的地址空间叫做用户空间，3G~4G-1叫做内核空间。一般用户程序都运行在用户空间，不能直接访问内核空间，但是操作系统的内核提供了相关的API含税可以访问内核空间。内核空间对常规内存、I/O设备内存以及高端内存有不同的处理方式。当然，内核空间和用户空间的界限是可以调整的，在内核配置选项Kernel Features->Memory split下可以设置界限为2GB或则3GB。

内存地址的基本单位是字节，而内存映射的基本单位是页，目前主流的操作系统中的一个内存页的大小是4kb

#### 5.段错误的由来

* 试图操作没有操作权限的内存空间（如修改只读常量区中的数据）
* 试图操作没有经过映射的虚拟地址（如给任意一个虚拟地址赋值）

#### 6.内存管理的相关函数

#### 7.Linux缓存系统

缓存是指将经常访问或新写入的数据从称作缓存的更快的小存储器读出或写入的过程。

赃内存是数据支持（如文件支持）的内存，其内容被修改（通常在缓存中）但尚未写回磁盘。数据的缓存版本比磁盘上的新，这就意味着两个版本不同步。缓存数据写回磁盘（后端存储）的机制称为回写。最终更新磁盘版本，使两者同步。干净内存是文件支持的内存，其内容与磁盘同步。

## 第十八章、Linux系统调试、调优

### 一、Linux内核的打印

### 二、内核调用栈

### 三、Linux性能工具

#### 1.Linux 性能工具全景图

当今时代，绝大多数企业的应用都是运行在 Linux 操作系统上，所以对应用进行性能诊断和性能优化时，离不开 Linux 的各种性能观测工具和性能优化工具。

笔者使用过的常见的Linux 性能观测和性能优化工具有：

- top/uptime
- ps/pstree
- df/du/free/lsblk
- ip/ifconfig/ping/telnet
- route/dig/nslookup
- lsof/netstat/ss
- tcpdump/tshark/wireshark
- netstat/vmstat/iostat/pidstat/dstat/mpstat
- sar/sysctl/ethtool

从著名的 LINUX 性能专家 Brendan Gregg 的个人博客和技术书籍，摘抄了如下九张图，一览 Linux 性能工具全景图

- linux performance observability tools![Image](https://mmbiz.qpic.cn/mmbiz_png/P0X0ia1B3wEZ7LcTMUZBfibwyfG7oTt3k5WGDc0UiamLVsckw1cO4EoEUZ3mQdwoeUQ8RadjiaAIuwc2iab2TOLlL1g/640?wx_fmt=png&wxfrom=5&wx_lazy=1&wx_co=1)
- linux static performance tools![Image](https://mmbiz.qpic.cn/mmbiz_png/P0X0ia1B3wEZ7LcTMUZBfibwyfG7oTt3k5YT6gNM3YlbjrCYC0pCtzdE3yhYlcjNRSNiciaOkAqa9nC7mAGw4Niavmg/640?wx_fmt=png&wxfrom=5&wx_lazy=1&wx_co=1)
- linux performance benchmark tools

![Image](https://mmbiz.qpic.cn/mmbiz_png/P0X0ia1B3wEZ7LcTMUZBfibwyfG7oTt3k5aoibpgZKgNz57tH3iaumpoyPvDrJZr9SSxWRZuPyON4w1U89x2w5cBRg/640?wx_fmt=png&wxfrom=5&wx_lazy=1&wx_co=1)

- linux performance tuning tools![Image](https://mmbiz.qpic.cn/mmbiz_png/P0X0ia1B3wEZ7LcTMUZBfibwyfG7oTt3k5pEDUUVc6kmf7Icj3nwib4M716aU5mEXVfXjnCUGB1aBBsXecXhiaygPw/640?wx_fmt=png&wxfrom=5&wx_lazy=1&wx_co=1)

- linux performance observability: sar![Image](https://mmbiz.qpic.cn/mmbiz_png/P0X0ia1B3wEZ7LcTMUZBfibwyfG7oTt3k5sXOaF2kibIq7fywFSOhm6ATCkb8a4IkU5ibzTvmGLUTPibMuqYPORm2Qg/640?wx_fmt=png&wxfrom=5&wx_lazy=1&wx_co=1)

- linux performance observability: perf-tools![Image](https://mmbiz.qpic.cn/mmbiz_png/P0X0ia1B3wEZ7LcTMUZBfibwyfG7oTt3k5iaHNsGuFhrmFq6tdGhmtbVibHScZoNPuHbGpRSB4tkicRIBE5eGSsmjHQ/640?wx_fmt=png&wxfrom=5&wx_lazy=1&wx_co=1)

- linux bcc/BPF Tracing tools![Image](https://mmbiz.qpic.cn/mmbiz_png/P0X0ia1B3wEZ7LcTMUZBfibwyfG7oTt3k5rNK70CibujphEiajb2aKOqeaseeZFo6pMeicmKZ3yV6IuQZgR657ccQZw/640?wx_fmt=png&wxfrom=5&wx_lazy=1&wx_co=1)

- bpftrace/eBPF Tools

  ![Image](https://mmbiz.qpic.cn/mmbiz_png/P0X0ia1B3wEZ7LcTMUZBfibwyfG7oTt3k5DdGbBS2dKyzVRIIgq2LXYmKnLcWognjro6ic4mcYwIbGhOtJ32uvcLg/640?wx_fmt=png&wxfrom=5&wx_lazy=1&wx_co=1)

- BPF Performance Tools: Linux System and Application Observability

  (![Image](https://mmbiz.qpic.cn/mmbiz_png/P0X0ia1B3wEZ7LcTMUZBfibwyfG7oTt3k5Xcg7goNDAvY4BX7dvia2Mk23bTDzemPKj4BW3h230icQJrBmKxSPHxWg/640?wx_fmt=png&wxfrom=5&wx_lazy=1&wx_co=1)

#### 2.CPU性能工具

#### 3.内存性能工具

	内存的主要性能指标就是系统内存的分配和使用、进程内存的分配和使用以及SWAP（内存交换分区）的用量。



从这些指标出发，可以得到内存性能工具表

| 性能指标                         | 性能工具                                           | 说明                                                         |
| -------------------------------- | -------------------------------------------------- | ------------------------------------------------------------ |
| 系统以用、可用、剩余内存         | free、vmstat、sar、/proc/meminfo                   | free最为简单，vmstat、sar更为全面；/proc/meminfo常用于监控系统中 |
| 进程虚拟内存、常驻内存、共享内存 | ps、top、pidstat、/proc/pid/stat、/proc/pid/status |                                                              |
|                                  |                                                    |                                                              |
|                                  |                                                    |                                                              |
|                                  |                                                    |                                                              |
|                                  |                                                    |                                                              |
|                                  |                                                    |                                                              |
|                                  |                                                    |                                                              |
|                                  |                                                    |                                                              |
|                                  |                                                    |                                                              |
|                                  |                                                    |                                                              |

 

#### 4.磁盘I/O性能工具

#### 5.网络性能工具

### 四、应用内存泄露排查

Valgrind是一个开源的内存调试和性能分析工具，用于帮助开发者找出程序中的内存错误，如内存泄漏、使用未初始化的内存、非法内存访问等问题。它在Linux平台上广泛使用，支持多种架构。

## 第十九章、驱动实验案例

### 一、字符设备驱动开发

#### 1.字符设备驱动简介

字符设备是Linux驱动中最基本的一类设备，字符设备就是一个一个字节，按照字节流进行读写设备，读写数据是分先后顺序的。

* 应用程序调用open()函数------>c库中的open()函数------>open()系统调用------>驱动的open()函数

其中关于C库以及如何通过系统调用陷入到内核中这个我们不用管，我们重点关注的是应用程序和具体的驱动，应用程序使用到的函数在具体的驱动程序都有与之对应的函数。每一个系统调用，在驱动中都有与之对应的一个驱动函数，在Linux内核文件include/linux/fs.h中有个file_operations的结构体，此结构体就是linux内核驱动操作函数集和，内容如下：

```c
struct file_operations {
	struct module *owner; //拥有该结构体的模块的指针，一般设置为THIS_MODULE
	loff_t (*llseek) (struct file *, loff_t, int);//修改文件当前读写位置
	ssize_t (*read) (struct file *, char __user *, size_t, loff_t *);//读取设备文件
	ssize_t (*write) (struct file *, const char __user *, size_t, loff_t *);//想设备文件发送/写入数据
	ssize_t (*read_iter) (struct kiocb *, struct iov_iter *);
	ssize_t (*write_iter) (struct kiocb *, struct iov_iter *);
	int (*iterate) (struct file *, struct dir_context *);
	int (*iterate_shared) (struct file *, struct dir_context *);
	unsigned int (*poll) (struct file *, struct poll_table_struct *);//轮询函数，用于查询设备是否可以进行非阻塞读写
	long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);//对设备的控制功能，和ioctl函数对应。
	long (*compat_ioctl) (struct file *, unsigned int, unsigned long);
	int (*mmap) (struct file *, struct vm_area_struct *);//将设备的内存映射到进程空间（用户空间）
	int (*open) (struct inode *, struct file *);//打开设备文件
	int (*flush) (struct file *, fl_owner_t id);
	int (*release) (struct inode *, struct file *);//关闭/释放设备文件
	int (*fsync) (struct file *, loff_t, loff_t, int datasync);
	int (*fasync) (int, struct file *, int);//刷新待处理的数据
	int (*lock) (struct file *, int, struct file_lock *);
	ssize_t (*sendpage) (struct file *, struct page *, int, size_t, loff_t *, int);
	unsigned long (*get_unmapped_area)(struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
	int (*check_flags)(int);
	int (*flock) (struct file *, int, struct file_lock *);
	ssize_t (*splice_write)(struct pipe_inode_info *, struct file *, loff_t *, size_t, unsigned int);
	ssize_t (*splice_read)(struct file *, loff_t *, struct pipe_inode_info *, size_t, unsigned int);
	int (*setlease)(struct file *, long, struct file_lock **, void **);
	long (*fallocate)(struct file *file, int mode, loff_t offset,
			  loff_t len);
	void (*show_fdinfo)(struct seq_file *m, struct file *f);
#ifndef CONFIG_MMU
	unsigned (*mmap_capabilities)(struct file *);
#endif
	ssize_t (*copy_file_range)(struct file *, loff_t, struct file *,
			loff_t, size_t, unsigned int);
	int (*clone_file_range)(struct file *, loff_t, struct file *, loff_t,
			u64);
	ssize_t (*dedupe_file_range)(struct file *, u64, u64, struct file *,
			u64);
};
```

#### 2.字符设备驱动开发步骤

##### 2.1 驱动模块的加载和卸载

Linux驱动有两种运行模式，第一种是将驱动编进内核中，这样当linux内核启动的时候就会自动运行驱动程序，第二种就是将驱动编译成模块，在内核启动以后将其加载进内核。

##### 2.2 字符设备注册与注销

##### 2.3 实现设备的具体操作函数

##### 2.4 添加LICENSE和作者信息

#### 3.Linux设备号

##### 3.1 设备号的组成

为了方便管理，Linux中每个设备都有一个设备号，设备好友主设备号和次设备号两个部分组成，主设备号表示某一个具体的设备，次设备号表示使用这个驱动的各个设备。Linux提供了一个名为dev_t的数据类型表示设备号，dev_t定义在文件include/linux/types.h中。

```c
typedef __u32 __kernel_dev_t;
typedef __kernel_dev_t dev;
//高12位表示主设备号，低20位为次设备号。
```

##### 3.2 设备号的分配

###### 3.2.1 静态分配

###### 3.2.2 动态分配

#### 4.字符设备驱动开发实例

```c
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>

#define CHRDEVBASE_MAJOR 200
#define CHRDEVBASE_NAME "chrdevbase"
static char readbuf[100] = {0};
static char writebuf[100] = {0};
static char kerneldata[] = {"kernel data!"};

static int chrdevbase_open(struct inode *inode, struct file *filp)
{
    return 0;
}

static ssize_t chrdevbase_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
    int retvalue = 0;
    memcpy(readbuf, kerneldata, sizeof(kerneldata));
    retvalue = copy_to_user(buf, readbuf, cnt);
    if(retvalue == 0) {
        printk("chrdevbase_read ok\r\n");
    } else {
        printk("chrdevbase_read failed\r\n");
    }

    return 0;
}

static ssize_t chrdevbase_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    int retvalue = 0;
    retvalue = copy_from_user(writebuf, buf, cnt);
    if(retvalue == 0) {
    printk("kernel receive data :%s\r\n", writebuf);
    } else {
        printk("kernel receive data failed!");
    }
    return 0;
}

static int chrdevbase_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static struct file_operations chrdevbase_fops = {
    .owner = THIS_MODULE,
    .open = chrdevbase_open,
    .read = chrdevbase_read,
    .write = chrdevbase_write,
    .release = chrdevbase_release,
};

/****/
static int __init chrdevbase_init(void)
{
    int retvalue = 0;
    retvalue = register_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME, &chrdevbase_fops);
    if(retvalue < 0) {
        printk("chrdevbase driver register failed!\r\n");
    }
    printk("chrdevbase_init()\r\n");

    return 0;
}

static void __exit chrdevbase_exit(void)
{
    unregister_chrdev(CHRDEVBASE_MAJOR, CHRDEVBASE_NAME);
    printk("chrdevbase_exit()\r\n");
}

module_init(chrdevbase_init);
module_exit(chrdevbase_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sarainco");
```

### 二、Linux设备树

#### 1.什么是设备树

设备树分开就是设备和树，描述设备树的文件叫做DTS，这个DTS文件采用树形结构描述板级设备，也就是开发板信息，比如CPU数量、内存基地址、IIC接口上接了那些设备、SPI接口上接了那些设备等等。

#### 2.DTS、DTB和DTC

设备树源文件扩展名为.dts，DTS是设备源文件，DTB是将DTS编译以后得到的二进制文件，DTC为编译工具。

#### 3.DTS语法

### 三、LED驱动开发实验

#### 1.将IO作为GPIO驱动步骤

* 使能GPIO对应的时钟:0x20C406C[27:26]
* 设置IO复用寄存器0x20E0068[3:0]0101
* 设置IO功能寄存器，设置上下拉、速度等。0x20E02F4[]0x10b0
* 配置GPIO，设置输入输出、是否使用中断、默认输出电平等。

#### 1.1 地址映射

内存管理单元MMU(memory manage unit)，主要功能如下：

* 完成虚拟空间到物理空间的映射。
* 内存保护，设置存储器的访问权限，设置虚拟存储空间的缓冲特性。

虚拟空间到物理空间的映射也叫地址映射。对于32位处理器来讲，虚拟地址的范围是2^32=4GB  ，笔者使用的开发板上有512MB的DDR3，这512就是物理内存，经过MMU可以将其映射到整个4GB的虚拟空间。

```c
ioremap  ||  iounmap
```

#### 2.设备树下的LED驱动

* 在.dts文件中创建相应的设备节点

  ```c
  	alphaled {
  		#address-cells = <1>;
  		#size-cells = <1>;
  		compatible = "atkalpha-led";
  		status = "okay";
  		reg = <	0X020C406C	0X04
  				0X020E0068	0X04
  				0X020E02F4	0X04
  				0X0209C000	0X04
  				0X0209C004	0X04 >;
  	};
  ```

* 编写驱动程序，获取设备树中的相关属性值。

* 使用获取到的有关属性值来初始化LED所使用的GPIO。

#### 3.将驱动编译进内核

### 四、pinctl和gpio子系统

Linux驱动讲究驱动分离和分层，pinctl和gpio子系统就是驱动分离和分层思想下的产物，驱动的分离和分层就是按照面向对象的设计思想而设计的设备驱动框架。

#### 1.pinctl子系统

* 获取设备树中的pin信息
* 更具获取到的pin信息来设置pin的复用功能。
* 更具获取到的pin信息来设置pin的电气属性，如上下拉、速度、驱动能力等。

对于使用者来讲，只需要在设备树里面配置好某个pin的相关属性即可，其他的初始化工作均有pinctl子系统来完成，pinctl子系统的源码目录为drivers/pinctl

#### 2.gpio子系统

* 方便开发者使用gpio,提供相应的API函数