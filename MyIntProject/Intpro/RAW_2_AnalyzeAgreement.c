#include<stdio.h>
#include<string.h>

#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netinet/ether.h>
#include<stdlib.h>

#include <stdio.h>

#include <netinet/ip.h>

#include <netinet/udp.h>

#include <sys/socket.h>

#include <netpacket/packet.h>

#include <net/if_arp.h>

#include <arpa/inet.h>

#include <netinet/ether.h>

#include <sys/ioctl.h>

#include <unistd.h>

#include <netpacket/packet.h>

#include <net/if.h>

#include <string.h>

#include <pthread.h>

#include <net/ethernet.h>

#include <ifaddrs.h>

#define NuLL -1
#define ICMP 0
#define IGMP 1
#define TCP 2
#define UDP 3
#define ARP_GO 4
#define ARP_BACK 5
#define RARP_GO 6
#define RARP_BACK 7
#define UNICAST 0
#define BROADCAST 1

typedef struct
{

  int eth;

  int type;

  unsigned short src_port;

  unsigned short dst_port;

  unsigned char src_ip[4];

  unsigned char dst_ip[4];

  unsigned char src_mac[6];

  unsigned char dst_mac[6];

} MYBUF;

typedef struct interface{
	char name[20];		//�ӿ�����
	unsigned char ip[4];		//IP��ַ
	unsigned char mac[6];		//MAC��ַ
	unsigned char netmask[4];	//��������
	unsigned char br_ip[4];		//�㲥��ַ
	int  flag;			//״̬
}INTERFACE;

int interface_num=0;//接口数量

MYBUF mybuf;

INTERFACE net_interface[16];//接口数据
int get_interface_num();

int MyMacCmp(char *buf);

void getinterface();

int AnalyzeAgreement(char* buf);

int AND(unsigned char * first,unsigned char *secend);

int IsSameSegment();

int SendTo(int len,char *buf,int eth,int fd);



int main()
{

		
		char pc_mac[6] = {0x54,0xEE,0x75,0x95,0x8B,0x6F};
		char Arm_mac[6]= {0x00,0x53,0x50,0x00,0x09,0x2B};
    	getinterface(); // 获取自身网卡信息

		int fd = socket(PF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
		printf("fd = %d\n",fd);
	
	while(1){

		bzero(&mybuf,sizeof(mybuf));

		unsigned char buf[1500] = "";

		int len = recvfrom(fd,buf,sizeof(buf),0,NULL,NULL);
				
		if(AnalyzeAgreement(buf) == -1)
		{
			continue;
		}
		//是否是同一网段
		int  Ethnum;
		
		Ethnum = IsSameSegment();

		if(Ethnum == -1)
		{
			//没有同一网段
			continue;
			//路由表查表
		}
		//有同一网段，返回所出去的网卡

		//查ARP表
		
		//如果有，返回网卡

		//如果没有，那就广播发ARP

		//广播后，获得目的MAC

		//查防火墙过滤表

		//固定ARM——MAC

		//固定PC———MAC

		if(Ethnum  == 2)
		{
		memcpy(buf,Arm_mac,6);
		memcpy(buf+6,net_interface[Ethnum].mac,6);
		SendTo(len,buf,Ethnum,fd);
		printf("to ens39\n");
		printf("协议类型：%d\n",mybuf.type);
		}else if(Ethnum == 1)
		{
			memcpy(buf,pc_mac,6);
			memcpy(buf+6,net_interface[Ethnum].mac,6);
		SendTo(len,buf,Ethnum,fd);
		printf("to ens33\n");
		printf("协议类型：%d\n",mybuf.type);

		}	
	}
	close(fd);
return 0;
}


int get_interface_num(){
	return interface_num;
}



void getinterface(){
	struct ifreq buf[16];    /* ifreq结构数组 */
	struct ifconf ifc;                  /* ifconf结构 */
	
	int sock_raw_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	 /* 初始化ifconf结构 */
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = (caddr_t) buf;
 
    /* 获得接口列表 */
    if (ioctl(sock_raw_fd, SIOCGIFCONF, (char *) &ifc) == -1){
        perror("SIOCGIFCONF ioctl");
        return ;
    }
    interface_num = ifc.ifc_len / sizeof(struct ifreq); /* 接口数量 */
    printf("interface_num=%d\n\n", interface_num);
 	char buff[20]="";
	int ip;
	int if_len = interface_num;
    while (if_len-- > 0){ /* 遍历每个接口 */
        printf("%s\n", buf[if_len].ifr_name); /* 接口名称 */
        sprintf(net_interface[if_len].name, "%s", buf[if_len].ifr_name); /* 接口名称 */
		printf("-%d-%s--\n",if_len,net_interface[if_len].name);
        /* 获得接口标志 */
        if (!(ioctl(sock_raw_fd, SIOCGIFFLAGS, (char *) &buf[if_len]))){
            /* 接口状态 */
            if (buf[if_len].ifr_flags & IFF_UP){
                printf("UP\n");
				net_interface[if_len].flag = 1;
            }
            else{
                printf("DOWN\n");
				net_interface[if_len].flag = 0;
            }
        }else{
            char str[256];
            sprintf(str, "SIOCGIFFLAGS ioctl %s", buf[if_len].ifr_name);
            perror(str);
        }
 
        /* IP地址 */
        if (!(ioctl(sock_raw_fd, SIOCGIFADDR, (char *) &buf[if_len]))){
			printf("IP:%s\n",(char*)inet_ntoa(((struct sockaddr_in*) (&buf[if_len].ifr_addr))->sin_addr));
			bzero(buff,sizeof(buff));
			sprintf(buff, "%s", (char*)inet_ntoa(((struct sockaddr_in*) (&buf[if_len].ifr_addr))->sin_addr));
			inet_pton(AF_INET, buff, &ip);
			memcpy(net_interface[if_len].ip, &ip, 4);
		}else{
            char str[256];
            sprintf(str, "SIOCGIFADDR ioctl %s", buf[if_len].ifr_name);
            perror(str);
        }
 
        /* 子网掩码 */
        if (!(ioctl(sock_raw_fd, SIOCGIFNETMASK, (char *) &buf[if_len]))){
            printf("netmask:%s\n",(char*)inet_ntoa(((struct sockaddr_in*) (&buf[if_len].ifr_addr))->sin_addr));
			bzero(buff,sizeof(buff));
			sprintf(buff, "%s", (char*)inet_ntoa(((struct sockaddr_in*) (&buf[if_len].ifr_addr))->sin_addr));
			inet_pton(AF_INET, buff, &ip);
			memcpy(net_interface[if_len].netmask, &ip, 4);
        }else{
            char str[256];
            sprintf(str, "SIOCGIFADDR ioctl %s", buf[if_len].ifr_name);
            perror(str);
        }
 
        /* 广播地址 */
        if (!(ioctl(sock_raw_fd, SIOCGIFBRDADDR, (char *) &buf[if_len]))){
            printf("br_ip:%s\n",(char*)inet_ntoa(((struct sockaddr_in*) (&buf[if_len].ifr_addr))->sin_addr));
			bzero(buff,sizeof(buff));
			sprintf(buff, "%s", (char*)inet_ntoa(((struct sockaddr_in*) (&buf[if_len].ifr_addr))->sin_addr));
			inet_pton(AF_INET, buff, &ip);
			memcpy(net_interface[if_len].br_ip, &ip, 4);
        }else{
            char str[256];
            sprintf(str, "SIOCGIFADDR ioctl %s", buf[if_len].ifr_name);
            perror(str);
        }

        /*MAC地址 */
		if (!(ioctl(sock_raw_fd, SIOCGIFHWADDR, (char *) &buf[if_len]))){
			printf("MAC:%02x:%02x:%02x:%02x:%02x:%02x\n\n",
					(unsigned char) buf[if_len].ifr_hwaddr.sa_data[0],
					(unsigned char) buf[if_len].ifr_hwaddr.sa_data[1],
					(unsigned char) buf[if_len].ifr_hwaddr.sa_data[2],
					(unsigned char) buf[if_len].ifr_hwaddr.sa_data[3],
					(unsigned char) buf[if_len].ifr_hwaddr.sa_data[4],
					(unsigned char) buf[if_len].ifr_hwaddr.sa_data[5]);
			memcpy(net_interface[if_len].mac, (unsigned char *)buf[if_len].ifr_hwaddr.sa_data, 6);
		}else{
            char str[256];
            sprintf(str, "SIOCGIFHWADDR ioctl %s", buf[if_len].ifr_name);
            perror(str);
        }
    }//–while end
    close(sock_raw_fd);   //关闭socket
}




int AnalyzeAgreement(char* buf)
{
	mybuf.eth = -1;

	for(int i = 0;i < interface_num; i++)
	{
		
			if(memcmp(buf,net_interface[i].mac,6) == 0)
			{
				mybuf.eth = i;

				break;
			}
	}
	if(mybuf.eth == -1) return -1;

		memcpy(mybuf.dst_mac,buf,6);

		memcpy(mybuf.src_mac,buf+6,6);

		unsigned short type = 0;
		type = ntohs(*(unsigned short *)(buf+12));

		//printf("协议类型：type = %#x\n",type);

		if(type == 0x0800)
		{
			unsigned char *ip = buf+14;

			memcpy(mybuf.src_ip,ip+12,4);

			memcpy(mybuf.dst_ip,ip+16,4);


			if(buf[14+8+1] == 0x01)
			{
					unsigned char *icmp = buf+14+(ip[0]&0x0f)*4;

					mybuf.type = ICMP;
			}
			else if(buf[14+8+1] == 0x06)
			{
				unsigned char *tcp = buf+14+(ip[0]&0x0f)*4;

				mybuf.src_port = ntohs(*(unsigned short *)tcp);

				mybuf.dst_port = ntohs(*(unsigned short *)(tcp+2));
	
				mybuf.type = TCP;
			}
			else if(buf[14+8+1] == 0x11)
			{
				
				unsigned char *udp = buf+14+(ip[0]&0x0f)*4;
				
				mybuf.src_port = ntohs(*(unsigned short *)udp);

                mybuf.dst_port = ntohs(*(unsigned short *)(udp+2));

				mybuf.type = UDP;
			}
			else
			{
				mybuf.type = NuLL;
			}
		}
		else if(type == 0x0806)
		{
			unsigned char *arp = buf+14;
			// int c  = 0;
			//  int d = 0;
			//  c  =  (c | 0x01) << 24;
			//  char *a = (char *)c;
			//  d  = (d | 0x02) << 24;
			//  char *b = (char *)d;
		
			
			
			memcpy(mybuf.src_ip,arp+14,4);

			memcpy(mybuf.dst_ip,arp+24,4);
			// if(memcmp(arp+7,a,1) == 0)mybuf.type = ARP_GO;
			// if(memcmp(arp+7,b,1) == 0)mybuf.type = ARP_BACK;

			
			
		}
		else if(type == 0x8035)
		{
			//  int a  = 0;
			//  int b = 0;
			//  a  =  (a | 0x03) << 24;
			//  char *c = (char *)a;
			// b  =  (b | 0x04) << 24;
			//  char *d = (char *)a;
			
			 unsigned char *rarp = buf+14;

			memcpy(mybuf.src_ip,rarp+14,4);

			memcpy(mybuf.dst_ip,rarp+24,4);

			// if(memcmp(rarp+7,c,1) == 0)mybuf.type = RARP_GO;
			// if(memcmp(rarp+7,d,1) == 0)mybuf.type = RARP_BACK;
		}
		else
		{
			mybuf.type = NuLL;
		}
		return mybuf.eth;
}

int SendTo(int len,char *buf,int eth,int fd)
{
    
	struct sockaddr_ll sll;

	struct ifreq ethreq;

	strncpy(ethreq.ifr_name,net_interface[eth].name,IFNAMSIZ);

	ioctl(fd,SIOCGIFINDEX,&ethreq);
		
	bzero(&sll,sizeof(sll));

	sll.sll_ifindex = ethreq.ifr_ifindex;
			
	sendto(fd,buf,len,0,(struct sockaddr *)&sll,sizeof(sll));
}

int IsSameSegment()
{
	if(mybuf.type == TCP || mybuf.type == UDP || mybuf.type == ICMP)
	{
		for(int i = 0;i < interface_num ; i++)
		{
			if(AND(net_interface[i].netmask , net_interface[i].ip) == AND(net_interface[i].netmask , mybuf.dst_ip))
			{
				
				return i;
				
			}
		}
	}
	return -1;
}

int AND(unsigned char * first,unsigned char *secend)
{

 	unsigned int * first1 = (unsigned char *)first;
	unsigned int * secend1 = (unsigned char *)secend;

	
	return *first1 & *secend1;
}

int SendArp(int eth,int flag,int fd)
{
unsigned char dst_mac[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
unsigned char srt_mac[6] = {0x00,0x00,0x00,0x00,0x00,0x00};
unsigned char dst_ip[6] = {10,0,121,237};
unsigned char srt_ip[6] = {10,0,121,253};
unsigned char msg[1500] = "";
struct ether_header *eth_hd  = (struct ether_header *)msg;
memcpy(eth_hd->ether_dhost,dst_mac,6);
memcpy(eth_hd->ether_shost,srt_mac,6);
eth_hd->ether_type = htons(0x0806);
struct arphdr *arp_hd = (struct arphdr*)(msg+14);
arp_hd->ar_hrd = htons(1);
arp_hd->ar_pro = htons(0x0800);
arp_hd->ar_hln = 6;
arp_hd->ar_pln = 4;
arp_hd->ar_op = htons(1);
memcpy(arp_hd->__ar_sha,srt_mac,6);
memcpy(arp_hd->__ar_sip,net_interface[eth].ip,4);
memcpy(arp_hd->__ar_tha,dst_mac,6);
memcpy(arp_hd->__ar_tip,mybuf.dst_ip,4);

	
	int len = 42;
	struct ifreq ethreq;
	strncpy(ethreq.ifr_name,net_interface[eth].name,IFNAMSIZ);
	ioctl(fd,SIOCGIFINDEX,&ethreq);
	struct sockaddr_ll sll;
	bzero(&sll,sizeof(sll));
	sll.sll_ifindex = ethreq.ifr_ifindex;
    
	if(flag == 0)
	{
		//单播
		SendTo(len,msg,eth,fd);
	}
	else if(flag == 1)
	{
		for(unsigned int i =BinaryAnd(GetIpNet(eth),1);i != i | (~((unsigned int)(net_interface[eth].netmask)));i = BinaryAnd(i,1))
		{
			memcpy(msg+41,&i,4);
			sendto(fd,msg,len,0,(struct sockaddr *)&sll,sizeof(sll));
		}
		
		

	}
	
		
	return 0;
}
//获取网络地址(网段地址)
unsigned int GetIpNet(int eth)
{
		unsigned int a= (unsigned int)(net_interface[eth].ip);
		unsigned int b= (unsigned int)(net_interface[eth].netmask);
		return a & b;
}

unsigned int BinaryAnd(unsigned int first,unsigned int second)
{
	unsigned int c;
	while(second != 0)
	{
		c = (first & second) << 1;
		first = first ^ second;
		second = c;
	}
	return first;
}