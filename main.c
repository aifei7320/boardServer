/*************************************************************************
  > File Name: main.c
  > Author: zxf
  > Mail: zhengxiaofeng333@163.com 
  > Created Time: Fri 07 Apr 2017 06:52:06 PM PDT
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <termios.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>

#define GPS_DEV "/dev/ttyUSB0"

int UART0_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity);

int main(int argc, char *argv[])
{
    int fd;
    int ret;
    int count;
    FILE *uart = NULL;
    fd_set gpsfds;
    char buf[100];
    char temp[100];
    struct termios usb;
    struct timeval wait_time;

    fd = open(GPS_DEV, O_RDONLY);
    if (fd < 0){
        printf("open GPS device failed!ret:%d\n", fd);
        return fd;
    }

    wait_time.tv_sec = 1;
    wait_time.tv_usec = 0;

    if (UART0_Set(fd, 9600, 0, 8, 1, 'N') == 0){
    }
    
    FD_ZERO(&gpsfds);
    FD_SET(fd + 1, &gpsfds);

    uart = fdopen(fd, "r");
    while(1){
        ret = select(fd + 1, &gpsfds, NULL, NULL, &wait_time);
        if(ret < 0)
            perror("select()\n");
        else {
            ret = fread(buf, 7, sizeof(char), uart);
            if (strstr(buf, "$GNRMC") != NULL){
                fread(temp, 61, sizeof(char), uart);
                if (strchr(temp, 'V'))
                    continue;
                strncat(buf, temp, 59);
                printf("read:%s\n", buf);
                memset(buf, 0, 100);
                memset(temp, 0, 100);
            }
            //strncat(temp, buf, ret);
            //if ()
            memset(buf, 0, 100);
        }
    }
   return 0;
}

int UART0_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity)
{

      int   i;
         int   status;
         int   speed_arr[] = { B115200, B19200, B9600, B4800, B2400, B1200, B300};
     int   name_arr[] = {115200,  19200,  9600,  4800,  2400,  1200,  300};

    struct termios options;

    /*tcgetattr(fd,&options)得到与fd指向对象的相关参数，并将它们保存于options,该函数还可以测试配置是否正确，该串口是否可用等。若调用成功，函数返回值为0，若调用失败，函数返回值为1.
    */
    if  ( tcgetattr( fd,&options)  !=  0)
       {
          perror("SetupSerial 1");
          return(0);
       }

    //设置串口输入波特率和输出波特率
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
                {
                     if  (speed == name_arr[i])
                            {
                                 cfsetispeed(&options, speed_arr[i]);
                                 cfsetospeed(&options, speed_arr[i]);
                            }
              }

    //修改控制模式，保证程序不会占用串口
    options.c_cflag |= CLOCAL;
    //修改控制模式，使得能够从串口中读取输入数据
    options.c_cflag |= CREAD;

    //设置数据流控制
    switch(flow_ctrl)
    {

       case 0 ://不使用流控制
              options.c_cflag &= ~CRTSCTS;
              break;

       case 1 ://使用硬件流控制
              options.c_cflag |= CRTSCTS;
              break;
       case 2 ://使用软件流控制
              options.c_cflag |= IXON | IXOFF | IXANY;
              break;
    }
    //设置数据位
    //屏蔽其他标志位
    options.c_cflag &= ~CSIZE;
    switch (databits)
    {
       case 5    :
                     options.c_cflag |= CS5;
                     break;
       case 6    :
                     options.c_cflag |= CS6;
                     break;
       case 7    :
                 options.c_cflag |= CS7;
                 break;
       case 8:
                 options.c_cflag |= CS8;
                 break;
       default:
                 fprintf(stderr,"Unsupported data size\n");
                 return (0);
    }
    //设置校验位
    switch (parity)
    {
       case 'n':
       case 'N': //无奇偶校验位。
                 options.c_cflag &= ~PARENB;
                 options.c_iflag &= ~INPCK;
                 break;
       case 'o':
       case 'O'://设置为奇校验
                 options.c_cflag |= (PARODD | PARENB);
                 options.c_iflag |= INPCK;
                 break;
       case 'e':
       case 'E'://设置为偶校验
                 options.c_cflag |= PARENB;
                 options.c_cflag &= ~PARODD;
                 options.c_iflag |= INPCK;
                 break;
       case 's':
       case 'S': //设置为空格
                 options.c_cflag &= ~PARENB;
                 options.c_cflag &= ~CSTOPB;
                 break;
        default:
                 fprintf(stderr,"Unsupported parity\n");
                 return (0);
    }
    // 设置停止位
    switch (stopbits)
    {
       case 1:
                 options.c_cflag &= ~CSTOPB; break;
       case 2:
                 options.c_cflag |= CSTOPB; break;
       default:
                       fprintf(stderr,"Unsupported stop bits\n");
                       return (0);
    }

  //修改输出模式，原始数据输出
  options.c_oflag &= ~OPOST;

  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);//我加的
//options.c_lflag &= ~(ISIG | ICANON);

    //设置等待时间和最小接收字符
    options.c_cc[VTIME] = 1; /* 读取一个字符等待1*(1/10)s */
    options.c_cc[VMIN] = 1; /* 读取字符的最少个数为1 */

    //如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读
    tcflush(fd,TCIFLUSH);

    //激活配置 (将修改后的termios数据设置到串口中）
    if (tcsetattr(fd,TCSANOW,&options) != 0)
           {
               perror("com set error!\n");
              return (0);
           }
    return (1);
} 
