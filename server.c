#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <arpa/inet.h>	//for inet_pton in c++
#include <strings.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <math.h>

#define PORT 28781
#define MAX 1024

char ctos_buff[MAX];
char stoc_buff[MAX];
int new_socket;

int caesar_key;
FILE *plaintext;
int server_fd;
struct sockaddr_in address;
int addrlen = sizeof(address);
char encoding_scheme[]={' ','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',',','.','?','0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','!'};

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


char decrypt(char s)
{
        for(int j=0;j<66;j++)
        {
            if(s==encoding_scheme[j])
            {   
                s=encoding_scheme[(j - caesar_key + 66) % 66];
                break;
            }      
        }   
    return s;
}




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


void connect1()
{
	
	int opt=1;
	int s_port=PORT;
	server_fd = socket(AF_INET , SOCK_STREAM , 0);	//AF_INEt for ipv4,sock_stream for TCP,0 fpr internet protocol

	if(setsockopt(server_fd , SOL_SOCKET , SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
			//helps in reuse of address and port. optional
	{
		printf("setsockopt\n");
		exit(0);
	}

	address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;   //INADDR_ANY for localhos
    address.sin_port = htons( PORT );   //The htons() function converts the unsigned short integer hostshort from host byte order to network byte order.
      
    
    if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
    {
        printf("bind failed");
        exit(0);
    }
    listen(server_fd, 3);
    
   
}

void send_to_client(char *s)           //send data to client
{
    strncpy(stoc_buff,s,strlen(s));
    send(new_socket,stoc_buff,strlen(stoc_buff),0);
} 

int receive_from_client(int flag) 
{
    int i = 0;
    if(flag)
    {    
        while (i < MAX) {
            int temp;
            if ((temp = read(new_socket, ctos_buff + i, MAX - i)) <= 0) {
                if (temp == 0)
                    break;
                perror("Error ");
                exit(-1);
            }
            i += temp;
        }
        for (int j = 0; j < i; j++) 
            ctos_buff[j] = decrypt(ctos_buff[j]);

        fwrite(ctos_buff, sizeof(char), i, plaintext);
    }        
    else
    {
        i=read( new_socket , ctos_buff, MAX);
        ctos_buff[i]='\0';           
        return i;
    }    
    return i;
}

int main()
{
    srand(time(NULL));
    
    printf("Waiting for client to connect\n");
   
	connect1();

	while(1)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,(socklen_t*)&addrlen))<0)
        {
            printf("accept");
            exit(0);
        }
        printf("--------------------------------------------------------\n");
        printf("Connection established\n\n\n");    

        
        receive_from_client(0);        
        int i=0,j=0;
        char temp[MAX];

        while(ctos_buff[i]!='$')
        {
            temp[j++]=ctos_buff[i++];
        }    
        int prime_num=atoi(temp);

        bzero(temp,strlen(temp)+2);
        j=0;
        i++;
        while(ctos_buff[i]!='$')
        {
            temp[j++]=ctos_buff[i++];
        }
        int primitive_root=atoi(temp);

        bzero(temp,strlen(temp) +2);
        j=0;
        i++;
        while(ctos_buff[i]!='\0')
        {
            temp[j++]=ctos_buff[i++];
        }    
        int c_public_key=atoi(temp);

        printf("Prime number            :%d\n",prime_num);
        printf("Primitive root          :%d\n\n",primitive_root);
        

        int s_private_key = rand() % (prime_num - 1) + 1;
        printf("Private key of Server   :%d\n",s_private_key);

        int s_public_key = power(primitive_root,s_private_key,prime_num);
        printf("Public key of Server    :%d\n\n",s_public_key);

        printf("Public key of Client    :%d\n\n",c_public_key);

        sprintf(stoc_buff,"%d",s_public_key);
        send_to_client(stoc_buff);

        int shared_key = power(c_public_key, s_private_key , prime_num);
        printf("Shared key is           :%d\n\n",shared_key);
        
    
        caesar_key = shared_key%66;
        printf("caesar key is           :%d\n\n",caesar_key);


        if((plaintext = fopen("plaintext.txt","w")) < 0) {
                perror("Error ");
                exit(-1);
        }
        printf("Receiving data from client....\n\n");
       
        int n;    
        while ((n = receive_from_client(1)) > 0){} 
            
        printf("Data received successfully!\n");
        printf("--------------------------------------------------------\n\n");
        
        fclose(plaintext);
        close(new_socket);
        new_socket=0;
}
/*    
    int flag=0;
    printf("--------------------------------------------------------\n");
    printf("                    LET'S CHAT                      \n");
    printf("--------------------------------------------------------\n");
    while(1)
    {    
        receive_from_client();
        if(strcmp("exit",decrypt(ctos_buff))==0)
            break;
        printf("Cipher Text     :%s\n",ctos_buff);
        printf("Decrypt Text    :%s\n                          Enter text     :",decrypt(ctos_buff));
        //scanf("%s",stoc_buff);
        scanf("%[^\n]%*c",stoc_buff);
    	if(strcmp("exit",stoc_buff)==0)
            flag=1;
        send_to_client(encrypt(stoc_buff));

        if(flag)
            break;
    }    
	printf("Connection Terminated\n");
    printf("------------------------------------------------------\n");
*/
	return 0;
}