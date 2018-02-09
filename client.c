#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>	//for inet_pton in c++
#include <time.h>
#include <limits.h>
#include <math.h>

#define MAXSIZE 1000000
#define PORT 28781
#define MAX 1024

char ctos_buff[MAX];
char stoc_buff[MAX];
int sock;


int caesar_key;
char encoding_scheme[]={' ','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',',','.','?','0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','!'};

//------------------------Encryption/Decryption
char* encrypt(char *s)
{
	int n=(int)strlen(s);
	char *s1=(char*)malloc(n*sizeof(char));
	
	for(int i=0;i<n;i++)
	{
		for(int j=0;j<66;j++)
		{
			if(s[i]==encoding_scheme[j])
			{	
				s1[i]=encoding_scheme[(j + caesar_key) % 66];
				break;
			}	
		}	
	}
	return s1;
}

char* decrypt(char *s)
{
	int n=(int)strlen(s);
	char *s1=(char*)malloc(n*sizeof(char));
	
	for(int i=0;i<n;i++)
	{
		for(int j=0;j<66;j++)
		{
			if(s[i]==encoding_scheme[j])
			{	
				s1[i]=encoding_scheme[(j - caesar_key + 66) % 66];
				break;
			}	
		}	
		
	}
	return s1;
}

//------------------------Prime no. and primitive roots
int power(int a,int b,int prime)
{
	unsigned long long x=1;
	unsigned long long y=a;

	while(b)
	{
		if(b & 1)
			x = (x * y) % prime;
		y = (y * y) % prime;
		b/=2;
	}	
	int result = x % prime;
	return result; 

}



int my_miller(int value)
{
	if(value%2==0|| value<2)
		return 0;
	int q = value - 1;
	int k=0;

	while(q%2==0)
	{
		q>>=1;
		k++;
	}	 
	for(int i=0;i<15;i++)
	{	
		long long int a= rand() % (value - 4) + 2;
		long long int result = power(a,q,value);
		int flag=1;
		
		for(;q != value - 1 ; q*=2)
		{
			if(result==1 || result == value - 1)
			{
				flag=0;
				break;
			}
			result = result * result % value;
		}	
		if(flag==1)
				return 0;
	}	
	return 1;

}

int get_prime()
{
	srand(time(NULL));
	while(1)
	{
		int prime= rand() % INT_MAX;
		if(my_miller(prime))
			return prime;
	}	
}

int my_Proot(int n)
{
	
	int prime[MAXSIZE];
	memset(prime,1,sizeof(prime));
	for(int p=2; p*p<MAXSIZE;p++)
	{
		if(prime[p])
		{
			for(int i=p*2;i<MAXSIZE; i+=p)
				prime[i]=0;
		}	
	}	

	int phi=n-1;	
	int flag=1;
	int i = 2;
	while(i++)
	{	
		int j;
		for(j=2;j<=sqrt(phi);j++)
		{
			if(prime[j] && phi%prime[j]==0 && power(i,phi/prime[j],n)==1)
			{	
				flag=0;
				break;
			}	
		}	
		if(flag)
			return i;
	}	
	return -1;
}



//----------------------socket connection
void connect1()
{
	int s_port=PORT;
	struct sockaddr_in serv_addr;
	sock = socket(AF_INET, SOCK_STREAM,0);
	memset(&serv_addr,'0',sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(s_port);

	inet_pton(AF_INET , "127.0.0.1" , &serv_addr.sin_addr); //converts the character string src into a network address structure in the af address family, then copies the network address structure to dst

	if(connect(sock,(struct  sockaddr *)&serv_addr, sizeof(serv_addr)))
	{
		printf("Connection not established\n");
		exit(0);
	}

}

void send_to_server(char *s)
{
    strncpy(ctos_buff,s,strlen(s));
    send(sock,ctos_buff,strlen(ctos_buff),0);

} 

void receive_from_server()
{    
    int i=read(sock,stoc_buff, MAX);
    stoc_buff[i]='\0';
}


//----------------------driver program
int main(int argc, char* argv[])
{
	if(argc!=2)
	{	
		printf("Please provide input file\n");
		return 0;
	}	
	srand(time(NULL));
	printf("Connected to Server\n");
	printf("--------------------------------------------------------\n");
	printf("Connection established\n\n\n");

	int prime_num=get_prime();
	printf("Prime number		:%d\n",prime_num);

	int primitive_root=my_Proot(prime_num);
	printf("Primitive root  	:%d\n\n",primitive_root);

	int c_private_key = rand() % (prime_num - 1) + 1;
	printf("Private key of Client 	:%d\n",c_private_key);

	int c_public_key = power(primitive_root,c_private_key,prime_num);
	printf("Public key of Client 	:%d\n\n",c_public_key);
	
	connect1();

	sprintf(ctos_buff,"%d$%d$%d",prime_num, primitive_root, c_public_key);
	send_to_server(ctos_buff);
	
	receive_from_server();
	int s_public_key = atoi(stoc_buff);
	printf("Public key of Server    :%d\n\n",s_public_key);

	int shared_key = power(s_public_key, c_private_key , prime_num);
	printf("Shared key		:%d\n\n",shared_key);
	
	caesar_key = shared_key%66;
	
	printf("caesar key		:%d\n\n",caesar_key);

	printf("Sending the data to server...\n\n");
	FILE *fp;
	fp=fopen(argv[1],"r");
	if(fp == NULL)
	{
		printf("\nError : No Such File Exists!\n");
		return 1;
	}
	while((fgets(ctos_buff,sizeof(ctos_buff),fp)) != NULL)
		send_to_server(encrypt(ctos_buff));		

	fclose(fp);
	printf("Data sent successfully!\n");
	printf("--------------------------------------------------------\n\n");
/*
	int flag=0;
	 printf("--------------------------------------------------------\n");
    printf("                    LET'S CHAT                      \n");
    printf("--------------------------------------------------------\n");
	while(1)
	{	
		//printf("   							Enter Text	:");
		printf("                          Enter text     :");
		//scanf("%s",ctos_buff);
		scanf("%[^\n]%*c",ctos_buff);
		if(strcmp("exit",ctos_buff)==0)
			flag=1;
		send_to_server(encrypt(ctos_buff));

		if(flag)
            break;
		
		receive_from_server();
		if(strcmp("exit",decrypt(stoc_buff))==0)
            break;
        printf("Cipher Text 	:%s\n",stoc_buff);
		printf("decrypted Text	:%s\n",decrypt(stoc_buff));
	}	
	printf("connection Terminated\n");
	printf("-----------------------------------------------\n");
*/
	return 0;
}