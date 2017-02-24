#include<stdio.h>
#include<string.h>
#include "DES.h"

int key[64];            
char InputKey[8];       
int keyPC1[56];         
int A[28];              
int B[28];
int keyAB[56];          
unsigned char K[16][48];
int text_out[65][64];   
char InitIV[8] = {0}; 
char Plaintext[512] = {0};      
char target[8];         
int text[64];           
int text_ip[64];        
int text_end[64];       
int L0[32],Li[32];      
int R0[32],Ri[32];
int RE0[48];            
int RK[48];             
int RKS[8];             
int SP[32];             
int RKSP[32];           
int result[64][64];     
int H[1024];             
char Ciphertext[1024];        
int C[4096];             
int M[64][8];           
int k,l,m,n; 
int k1,l1,m1,n1;
int r[8],c[8];        

int IP[64]={         
   58, 50, 42, 34, 26, 18, 10, 2, 
   60, 52, 44, 36, 28, 20, 12, 4, 
   62, 54, 46, 38, 30, 22, 14, 6, 
   64, 56, 48, 40, 32, 24, 16, 8, 
   57, 49, 41, 33, 25, 17,  9, 1, 
   59, 51, 43, 35, 27, 19, 11, 3, 
   61, 53, 45, 37, 29, 21, 13, 5, 
   63, 55, 47, 39, 31, 23, 15, 7
};

int IPN[64]={       
   40,  8, 48, 16, 56, 24, 64, 32,
   39,  7, 47, 15, 55, 23, 63, 31,
   38,  6, 46, 14, 54, 22, 62, 30,
   37,  5, 45, 13, 53, 21, 61, 29,
   36,  4, 44, 12, 52, 20, 60, 28,
   35,  3, 43, 11, 51, 19, 59, 27,
   34,  2, 42, 10, 50, 18, 58, 26,
   33,  1, 41,  9, 49, 17, 57, 25
};

int E[48]={           
   32,  1,  2,  3,  4,  5, 
    4,  5,  6,  7,  8,  9, 
    8,  9, 10, 11, 12, 13, 
   12, 13, 14, 15, 16, 17, 
   16, 17, 18, 19, 20, 21, 
   20, 21, 22, 23, 24, 25, 
   24, 25, 26, 27, 28, 29, 
   28, 29, 30, 31, 32,  1 
};

int PC1[56]={          
   57, 49, 41, 33, 25, 17,  9, 
    1, 58, 50, 42, 34, 26, 18, 
   10,  2, 59, 51, 43, 35, 27, 
   19, 11,  3, 60, 52, 44, 36, 
   63, 55, 47, 39, 31, 23, 15, 
    7, 62, 54, 46, 38, 30, 22, 
   14,  6, 61, 53, 45, 37, 29, 
   21, 13,  5, 28, 20, 12,  4 
};

int move[16]={          
   1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1
};

int PC2[48]={           
   14, 17, 11, 24,  1,  5, 
    3, 28, 15,  6, 21, 10, 
   23, 19, 12,  4, 26,  8, 
   16,  7, 27, 20, 13,  2, 
   41, 52, 31, 37, 47, 55, 
   30, 40, 51, 45, 33, 48, 
   44, 49, 39, 56, 34, 53, 
   46, 42, 50, 36, 29, 32 
}; 

int S1[4][16]={      
   14,  4, 13, 1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9, 0,  7, 
    0, 15,  7, 4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5, 3,  8, 
    4,  1, 14, 8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10, 5,  0, 
   15, 12,  8, 2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0, 6, 13
};
int S2[4][16]={
   15,  1,  8, 14,  6, 11,  3,  4,  9, 7,  2, 13, 12, 0,  5, 10, 
    3, 13,  4,  7, 15,  2,  8, 14, 12, 0,  1, 10,  6, 9, 11,  5, 
    0, 14,  7, 11, 10,  4, 13,  1,  5, 8, 12,  6,  9, 3,  2, 15, 
   13,  8, 10,  1,  3, 15,  4,  2, 11, 6,  7, 12,  0, 5, 14,  9
};
int S3[4][16]={
   10,  0,  9, 14, 6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8, 
   13,  7,  0,  9, 3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1, 
   13,  6,  4,  9, 8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7, 
    1, 10, 13,  0, 6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12
};
int S4[4][16]={
    7, 13, 14, 3,  0,  6,  9, 10,  1, 2, 8,  5, 11, 12,  4, 15, 
   13,  8, 11, 5,  6, 15,  0,  3,  4, 7, 2, 12,  1, 10, 14,  9, 
   10,  6,  9, 0, 12, 11,  7, 13, 15, 1, 3, 14,  5,  2,  8,  4, 
    3, 15,  0, 6, 10,  1, 13,  8,  9, 4, 5, 11, 12,  7,  2, 14
};
int S5[4][16]={
    2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13, 0, 14,  9,
   14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3, 9,  8,  6, 
    4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6, 3,  0, 14, 
   11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10, 4,  5,  3
};
int S6[4][16]={
   12,  1, 10, 15, 9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11, 
   10, 15,  4,  2, 7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8, 
    9, 14, 15,  5, 2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6, 
    4,  3,  2, 12, 9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13
};
int S7[4][16]={
    4, 11,  2, 14, 15, 0,  8, 13,  3, 12, 9,  7,  5, 10, 6,  1, 
   13,  0, 11,  7,  4, 9,  1, 10, 14,  3, 5, 12,  2, 15, 8,  6, 
    1,  4, 11, 13, 12, 3,  7, 14, 10, 15, 6,  8,  0,  5, 9,  2, 
    6, 11, 13,  8,  1, 4, 10,  7,  9,  5, 0, 15, 14,  2, 3, 12
};
int S8[4][16]={
   13,  2,  8, 4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7, 
    1, 15, 13, 8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2, 
    7, 11,  4, 1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8, 
    2,  1, 14, 7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11 
};  

int P[32]={                   
   16, 7, 20, 21, 29, 12, 28, 17,  1, 15, 23, 26,  5, 18, 31, 10,
    2, 8, 24, 14, 32, 27,  3,  9, 19, 13, 30,  6, 22, 11,  4, 25  
};

static void KeyByteToBit();

static void CreatKeys();

static void Encode();

static void PrintCiphertext();

static int PrintCiphertextEx();

static void Decode();

static void PrintPlaintext();

int DESEncode(IN char * pKey, IN char * pIV, IN char * pPlaintext, OUT char * pCiphertext)
{
   int iRet = 0;
   if(pKey == NULL || pIV == NULL || pPlaintext == NULL || pCiphertext == NULL) return iRet = 1;
   if(strlen(pKey) != 8 || strlen(pIV) != 8) return iRet = 2;
   if(strlen(pPlaintext) <= 0) return iRet = 3;
   memset(InputKey, 0, sizeof(InputKey));
   memcpy(InputKey, pKey, strlen(pKey));
   memset(InitIV, 0, sizeof(InitIV));
   memcpy(InitIV, pIV, strlen(pIV));
   memcpy(Plaintext, pPlaintext, strlen(pPlaintext) + 1);
   memset(Ciphertext, 0, sizeof(Ciphertext));
   KeyByteToBit();            
   CreatKeys();               
   Encode();              
   PrintCiphertext();  
   if(strlen(Ciphertext) <= 0) return iRet = 4;
   memcpy(pCiphertext, Ciphertext, strlen(Ciphertext) + 1);
   return iRet;
}

int DESEncodeEx(IN char * pKey, IN char * pIV, IN char * pPlaintext, OUT char * pCiphertext, OUT int * cipherLen)
{
   int iRet = 0;
	int textLen = 0;
   if(pKey == NULL || pIV == NULL || pPlaintext == NULL || pCiphertext == NULL) return iRet = 1;
   if(strlen(pKey) != 8 || strlen(pIV) != 8) return iRet = 2;
   if(strlen(pPlaintext) <= 0) return iRet = 3;
   memset(InputKey, 0, sizeof(InputKey));
   memcpy(InputKey, pKey, strlen(pKey));
   memset(InitIV, 0, sizeof(InitIV));
   memcpy(InitIV, pIV, strlen(pIV));
   memcpy(Plaintext, pPlaintext, strlen(pPlaintext) + 1);
   memset(Ciphertext, 0, sizeof(Ciphertext));
   KeyByteToBit();            
   CreatKeys();               
   Encode();              
   textLen = PrintCiphertextEx();  
   if(textLen <= 0) return iRet = 4;
   memcpy(pCiphertext, Ciphertext, textLen + 1);
	*cipherLen = textLen;
   return iRet;
}

int DESDecode(IN char * pKey, IN char * pIV, IN char * pCiphertext, OUT char * pPlaintext)
{
   int iRet = 0;
   if(pKey == NULL || pIV == NULL || pPlaintext == NULL || pCiphertext == NULL) return iRet = 1;
   if(strlen(pKey) != 8 || strlen(pIV) != 8) return iRet = 2;
   memset(InputKey, 0, sizeof(InputKey));
   memcpy(InputKey, pKey, strlen(pKey));
   memset(InitIV, 0, sizeof(InitIV));
   memcpy(InitIV, pIV, strlen(pIV));
   {
      int i = 0, j = 0;
      int chCnt = 0;
      int ptLen = strlen(pCiphertext);
      if(ptLen <= 0 || ptLen % 16 != 0) return iRet = 3;
      memset(Ciphertext, 0, sizeof(Ciphertext));
      memcpy(Ciphertext, pCiphertext, strlen(pCiphertext));
      memset(H, 0, sizeof(H));

      for(i=0;Ciphertext[i]!='\0';i++)  
      {
         chCnt++;
         if(Ciphertext[i]>='0'&&Ciphertext[i]<='9')
            H[i]=Ciphertext[i]-'0';
         else if(Ciphertext[i]>='A'&&Ciphertext[i]<='F')
            H[i]=Ciphertext[i]-'A'+10;
         else if(Ciphertext[i]>='a'&&Ciphertext[i]<='f')
            H[i]=Ciphertext[i]-'a'+10;
         else 
         {     
            return iRet = 4;
         }
      }

      n = chCnt;
      for(i=0;i<chCnt;i++)         
      {   
         int he[4]={0,0,0,0};
         for(j=3;H[i]!=0;j--)
         {   
            he[j] = H[i]%2;
            H[i] = H[i]/2;
         }
         for(j=0;j<4;j++)
            C[j+(i*4)]=he[j];
      } 
   }
   KeyByteToBit();            
   CreatKeys(); 
   Decode();                
   PrintPlaintext();          
   if(strlen(Plaintext) <= 0) return iRet = 4;
   memcpy(pPlaintext, Plaintext, strlen(Plaintext) + 1);
   return iRet;
}

int DESDecodeEx(IN char * pKey, IN char * pIV, IN char * pCiphertext, IN int cipherLen, OUT char * pPlaintext)
{
	int iRet = 0;	
	//int textLen = 0;
	if(pKey == NULL || pIV == NULL || pPlaintext == NULL || pCiphertext == NULL) return iRet = 1;
	if(strlen(pKey) != 8 || strlen(pIV) != 8) return iRet = 2;
	memset(InputKey, 0, sizeof(InputKey));
	memcpy(InputKey, pKey, strlen(pKey));
	memset(InitIV, 0, sizeof(InitIV));
	memcpy(InitIV, pIV, strlen(pIV));
	{
		int chCnt = 0;
		int i = 0, j = 0;
		int ptLen = cipherLen;
		//if(ptLen <= 0 || ptLen % 16 != 0) return iRet = 3;
		if(ptLen <= 0 || ptLen % 8 != 0) return iRet = 3;
		memset(Ciphertext, 0, sizeof(Ciphertext));
		memcpy(Ciphertext, pCiphertext, ptLen);
		memset(H, 0, sizeof(H));

		for(i=0; i<ptLen; i++)
		{
			chCnt += 2;
			H[i*2] = 0xF & (Ciphertext[i] >> 4);
			H[i*2+1] = 0xF & Ciphertext[i];
		}

		n = chCnt;
		for(i=0;i<chCnt;i++)         
		{   
			int he[4]={0,0,0,0};
			for(j=3;H[i]!=0;j--)
			{   
				he[j] = H[i]%2;
				H[i] = H[i]/2;
			}
			for(j=0;j<4;j++)
				C[j+(i*4)]=he[j];
		} 
	}
	KeyByteToBit();            
	CreatKeys(); 
	Decode();                
	PrintPlaintext();          
	if(strlen(Plaintext) <= 0) return iRet = 4;
	memcpy(pPlaintext, Plaintext, strlen(Plaintext) + 1);
	return iRet;
}

static void KeyByteToBit()
{            
   int i,j;                       
   for(i=0;i<8;i++)               
   {
      int a[8]={0,0,0,0,0,0,0,0};
      int x=InputKey[i];         

      for(j=0;x!=0;j++)          
      {  
         a[j]=x%2;
         x=x/2;  
      }

      for(j=0;j<8;j++)           
         key[(i*8)+j]=a[7-j];
   }  
   memset(InputKey, 0, sizeof(InputKey));  
}

static void CreatKeys()
{
   int i,j,x,y,t;
   for(i=0;i<56;i++)    
      keyPC1[i]=key[PC1[i]-1];

   for(i=0;i<28;i++)    
   {  
      A[i]=keyPC1[i];
      B[i]=keyPC1[i+28];
   } 

   for(t=0;t<16;t++)       
   {  
      if(move[t]==1)     
      {   
         x=A[0];         
         for(i=0;i<27;i++)
            A[i]=A[i+1];
         A[27]=x;
         x=B[0];
         for(i=0;i<28;i++)
            B[i]=B[i+1];
         B[27]=x;   
      }
      else
      {   
         x=A[0];  
         y=A[1];
         for(i=0;i<26;i++)
            A[i]=A[i+2];
         A[26]=x;
         A[27]=y;

         x=B[0];
         y=B[1];
         for(i=0;i<26;i++)
            B[i]=B[i+2];
         B[26]=x;
         B[27]=y;   
      }

      for(i=0;i<28;i++)   
      {   
         keyAB[i]=A[i];
         keyAB[i+28]=B[i];  
      }

   for(i=0;i<48;i++)    
      K[t][i]=keyAB[PC2[i]-1];
   }

   for(i=0;i<8;i++)
   {  
      int a[8]={0,0,0,0,0,0,0,0};
      x=InitIV[i];
      for(j=0;x!=0;j++)
      { 
         a[j]=x%2;
         x=x/2;  
      }
      for(j=0;j<8;j++)
         text_out[0][(i*8)+j]=a[7-j]; 
   }
}

static void Encode()
{
   int i = 0, j = 0;             
   n=0;
   while(Plaintext[i]!='\0')
   {  
      n++;
      i++;  
   }
   k=n%8;
   n=(n-1)/8+1;

   for(l=0;l<n;l++)
   {   
      if(l==(n-1)&&k!=0)
      {  
         for(i=0;i<k;i++)  
         {
            target[i]=Plaintext[i+(8*l)];
         }

         for(i=k;i<8;i++)    //PKCS7
         {
            char padCh = 8-k;
            target[i]=padCh;
         }
      }
      else
      {
         for(i=0;i<8;i++) 
         {
            target[i]=Plaintext[i+(8*l)];
         }
      }

      for(i=0;i<8;i++)                     
      {          
         int a[8]={0,0,0,0,0,0,0,0};    
         
         m=target[i];
         {
            int j = 0;
            for(j=0;m!=0;j++)
            {  
               a[j]=m%2;
               m=m/2;
            } 
         }
 
         for(j=0;j<8;j++)
         {
            text[(i*8)+j]=a[7-j];
         }
      }

      for(i=0;i<64;i++)       
      {
         text[i]=text_out[l][i]^text[i];
      }

      for(i=0;i<64;i++)                
      {
         text_ip[i]=text[IP[i]-1];
      }

      for(i=0;i<32;i++)                     
      {  
         L0[i]=text_ip[i];
         R0[i]=text_ip[i+32];  
      }

      {
         int t = 0;
         for(t=0;t<16;t++)
         {
            for(i=0;i<48;i++)     
               RE0[i]=R0[E[i]-1];

            for(i=0;i<48;i++)         
               RK[i]=RE0[i]^K[t][i];

            for(i=0;i<8;i++)          
            {                         
               r[i]=RK[(i*6)+0]*2+RK[(i*6)+5];                             
               c[i]=RK[(i*6)+1]*8+RK[(i*6)+2]*4+RK[(i*6)+3]*2+RK[(i*6)+4];  
            }
            RKS[0]=S1[r[0]][c[0]]; 
            RKS[1]=S2[r[1]][c[1]];
            RKS[2]=S3[r[2]][c[2]];
            RKS[3]=S4[r[3]][c[3]];
            RKS[4]=S5[r[4]][c[4]];
            RKS[5]=S6[r[5]][c[5]];
            RKS[6]=S7[r[6]][c[6]];
            RKS[7]=S8[r[7]][c[7]]; 

            for(i=0;i<8;i++)          
            {                         
               int b[4]={0,0,0,0};
               m=RKS[i];
               for(j=3;m!=0;j--)
               {
                  b[j]=m%2;
                  m=m/2;
               }
               for(j=0;j<4;j++)
                  SP[j+(i*4)]=b[j];
            }

            for(i=0;i<32;i++)       
               RKSP[i]=SP[P[i]-1];  

            for(i=0;i<32;i++)       
               Ri[i]=L0[i]^RKSP[i];

            for(i=0;i<32;i++)        
            {              
               L0[i]=R0[i];
               R0[i]=Ri[i];
            }
         }
      }

      for(i=0;i<32;i++)   
         Li[i]=R0[i];    
      for(i=0;i<32;i++)
         R0[i]=L0[i];
      for(i=0;i<32;i++)
         L0[i]=Li[i];

      for(i=0;i<32;i++)   
         text_end[i]=L0[i];                  
      for(i=32;i<64;i++)
         text_end[i]=R0[i-32];

      for(i=0;i<64;i++)   
         text_out[l+1][i]=text_end[IPN[i]-1];

      for(i=0;i<64;i++)
         result[l][i]=text_out[l+1][i];
   }
}

static void PrintCiphertext()
{
   int i,j;
   for(j=0;j<n;j++)    
      for(i=0;i<16;i++)
         H[i+(j*16)]=result[j][0+(i*4)]*8+result[j][1+(i*4)]*4+
         result[j][2+(i*4)]*2+result[j][3+(i*4)];

   for(i=0;i<n*16;i++)
   {
      if(H[i]<10)
         Ciphertext[i]=H[i]+48;
      else if(H[i]==10)
         Ciphertext[i]='a';
      else if(H[i]==11)
         Ciphertext[i]='b';
      else if(H[i]==12)
         Ciphertext[i]='c';
      else if(H[i]==13)
         Ciphertext[i]='d';
      else if(H[i]==14)
         Ciphertext[i]='e';
      else if(H[i]==15)
         Ciphertext[i]='f';
   }

   for(i=l*16;i<sizeof(Ciphertext);i++)
      Ciphertext[i]='\0';
}

static int PrintCiphertextEx()
{
   int i, j, iRet,cipherStrLen = 0;
   for(j=0;j<n;j++)    
      for(i=0;i<16;i++)
         H[i+(j*16)]=result[j][0+(i*4)]*8+result[j][1+(i*4)]*4+
         result[j][2+(i*4)]*2+result[j][3+(i*4)];

   for(i=0;i<n*16/2;i++)
   {
      char tmpCh = 0;
      tmpCh |= (H[i*2] << 4);
      tmpCh |= H[i*2+1];
      Ciphertext[i] = tmpCh;
		cipherStrLen++;
   }

   for(i = cipherStrLen; i<sizeof(Ciphertext); i++)
      Ciphertext[i]='\0';
	return iRet = cipherStrLen;
}

static void Decode()
{
   int i,j,t; 	      
   k=n/16;
   for(l=0;l<k;l++)
   {   
      for(i=0;i<64;i++)  
      text_out[l+1][i]=C[i+(l*64)];             

      for(i=0;i<64;i++)  
         text_ip[i]=text_out[l+1][IP[i]-1];

      for(i=0;i<32;i++)  
      {   
         L0[i]=text_ip[i];
         R0[i]=text_ip[i+32];
      }

      for(t=0;t<16;t++)
      {
         for(i=0;i<48;i++)  
            RE0[i]=R0[E[i]-1];

         for(i=0;i<48;i++)     
            RK[i]=RE0[i]^K[15-t][i];

         for(i=0;i<8;i++) 
         {   
            r[i]=RK[(i*6)+0]*2+RK[(i*6)+5];
            c[i]=RK[(i*6)+1]*8+RK[(i*6)+2]*4+RK[(i*6)+3]*2+RK[(i*6)+4];
         }
         RKS[0]=S1[r[0]][c[0]]; 
         RKS[1]=S2[r[1]][c[1]];
         RKS[2]=S3[r[2]][c[2]];
         RKS[3]=S4[r[3]][c[3]];
         RKS[4]=S5[r[4]][c[4]];
         RKS[5]=S6[r[5]][c[5]];
         RKS[6]=S7[r[6]][c[6]];
         RKS[7]=S8[r[7]][c[7]]; 

         for(i=0;i<8;i++)       
         { 
            int b[4]={0,0,0,0};
            m=RKS[i];
            for(j=3;m!=0;j--)
            {
               b[j]=m%2;
               m=m/2;
            }
            for(j=0;j<4;j++)
               SP[j+(i*4)]=b[j];
         }

         for(i=0;i<32;i++)        
            RKSP[i]=SP[P[i]-1];  

         for(i=0;i<32;i++)        
            Ri[i]=L0[i]^RKSP[i];

         for(i=0;i<32;i++)        
         {          
            L0[i]=R0[i];
            R0[i]=Ri[i];
         }
      }
      for(i=0;i<32;i++)    
         Li[i]=R0[i];     
      for(i=0;i<32;i++)
         R0[i]=L0[i];
      for(i=0;i<32;i++)
         L0[i]=Li[i];

      for(i=0;i<32;i++)    
         text_end[i]=L0[i];
      for(i=32;i<64;i++)
         text_end[i]=R0[i-32];

      for(i=0;i<64;i++)       
         text[IP[i]-1]=text_end[i];

      for(i=0;i<64;i++)      
         result[l][i]=text_out[l][i]^text[i];
   }
} 

static void PrintPlaintext()
{    
   int i,j;
   for(i=0;i<(n/16);i++)           
      for(j=0;j<8;j++)
         M[i][j]=result[i][(j*8)+0]*128+result[i][(j*8)+1]*64+
         result[i][(j*8)+2]*32+result[i][(j*8)+3]*16+
         result[i][(j*8)+4]*8+result[i][(j*8)+5]*4+
         result[i][(j*8)+6]*2+result[i][(j*8)+7];
   {
      int k = 0;
      int iFlag = 1;
      for(i=0;i<(n/16);i++)
      {
         for(j=0;j<8;j++)
         {
            if(M[i][j]>8) 
            {
               Plaintext[k] = M[i][j];
               k++;
            }
            else
            {
               Plaintext[k] = '\0';
               iFlag = 0;
               break;
            }
         }
         if(!iFlag) break;
      }
   }
}
