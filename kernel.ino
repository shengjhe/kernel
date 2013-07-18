#include <Ethernet.h>
#include <SPI.h>
#include <MD5.h>
/******************************************************
**              主機參數設定                        **
******************************************************/
int   result=2;
long  updatatime=0;
long starttime=0;
String HDAddress="shengjhe";

/******************************************************
**               網路參數設定                        **
******************************************************/

//設定網路卡位址
byte mac[] = { 0x00, 0x51, 0x56, 0xAC, 0x31, 0x37 };
byte ipaddr []={192,168,2,114};
byte netmask[]={255,255,255,0};
byte getway[]={192,168,2,1};
byte googledns[]={168,95,1,1};
char server[] = "192.168.2.1";
EthernetClient client;



/******************************************************
**               加密參數設定                        **
******************************************************/
String md5String="";
int md5Stringlen=0;
String sendMD5String="";
String myhashString="SJProject";


/******************************************************
**              伺服器連線相關參數設定             **
******************************************************/

String cookie="";
String randString="";
String responseMessage="";



/******************************************************
**                 初始化設定                        **
******************************************************/

void setup (){
  Serial.begin(9600);
  pinMode(13,OUTPUT);
  while (!Serial); 
  Ethernet.begin(mac,ipaddr,googledns,getway,netmask);
  Serial.println(Ethernet.localIP());
  delay(1000);
  Serial.println("connecting...");
  
  Serial.println("Geting cookie");
  login();
}



/******************************************************
**           Arduino 循環函數                        **
******************************************************/

void loop()
{
  if(Serial.available() >0 || millis() > updatatime)
  {
    char str =Serial.read();
    if(str ==0)
    {
      //代表門窗被開啟
      digitalWrite(13,HIGH);
      result=1;
      sendMessage();
      updatatime=millis()+180000;
    }
    else if(str ==1)
    {
        //代表門窗是關閉的狀態
       digitalWrite(13,LOW);
       result=0;
       sendMessage();
       updatatime=millis()+180000;
    } 
    else if(str ==0x03 && (millis()-starttime)>3000)
    {
        //代表端點剛開機
       result=3;
       sendMessage();
       starttime=millis();
       updatatime=millis()+180000;
    }
    
    else if (millis() > updatatime)
    {
      //兩分鐘沒收到端點傳來訊息就送出遺失訊號
      result=2;
      sendMessage();
      updatatime=millis()+30000;
    }
  }
  
}




/******************************************************
**           登入伺服器主機函數                      **
******************************************************/

void login()
{
  delay(1000);
  if (client.connect(server, 8080))
 {
              client.print("GET /TMIProject/APP/Users/login/shengjhe");
              client.println(" HTTP/1.1");
              client.println("Host:192.168.2.1");
              client.println("");
              client.println("Connection: close");
              delay(1000);
              while (client.available() >0) 
              {
                    char c = client.read();
                    randString += c; 
                    
                        if(randString.endsWith("Set-Cookie: "))
                        {
                           randString="";
                        }
                        if(randString.endsWith("; Path"))
                        {
                            cookie=randString;
                            randString="";
                            cookie.replace("; Path", "");
                        }

                       if(randString.endsWith("\r\n"))
                        {
                            randString="";
                        }
  
              }
  } 
  
            stopHttpConnection();
            Serial.print("cookie:");
            Serial.println(cookie);
            Serial.print("randString:");
            Serial.println(randString);
            encryptMD5();

}


/******************************************************
**              MD5加密函數                          **
******************************************************/
void encryptMD5()
{
  //判斷伺服器回傳回來的亂數不為空
    if(!randString.equals(""))
    {
        //將主機ID + 伺服器回傳的亂數 + 團隊自訂字串彙整
        md5String = HDAddress+randString+myhashString;
        //取得加密前字串長度
        md5Stringlen=md5String.length()+1;
        char md5char[md5Stringlen];
        //將彙整完畢的完整字串進行16 byte  加密
        md5String.toCharArray(md5char,md5Stringlen);
        unsigned char* hash=MD5::make_hash(md5char);
        char *md5str= MD5::make_digest(hash, 16);
        sendMD5String=md5str;
        Serial.println(sendMD5String);
    }
    else
    {
        login();
    
    }

}


/******************************************************
**           發送警訊通知伺服器函數                  **
******************************************************/

void sendMessage( )
{
      if (client.connect(server, 8080))
     {
                   //判斷Cookie不為空才發送Request
                 if(!cookie.equals(""))
                 {
                          client.print("PUT /TMIProject/APP/Status/");
                          client.print(sendMD5String);  // 傳送加密字串
                          client.print("/");
                          client.print(result);   //傳送偵測狀態結果
                          client.println(" HTTP/1.1");
                          client.println("Host:192.168.2.1");
                          client.print("Cookie:");
                          client.println(cookie);  //傳送Cookie
                          client.println("");
                          client.println("Connection: close");
                          Serial.print("The message is send");
                          Serial.println(result);
                 }
                 else 
                 {
                     login();
                 }
               
      } 
      else
      {
        //連線失敗
        Serial.println("connection failed");
      }
      delay(1000);
      
      
      //讀取Http Response Result
      while (client.available() >0) 
      {
          char ch = client.read();
          responseMessage+=ch;
  
          //過濾Http Header
          if(responseMessage.endsWith("\r\n"))
          {
                responseMessage="";
          }
      
      }
      
    //判斷如果Http回傳的網頁結果是 Not succesfully，就把cookie 還有亂數清空，讓主機重新進行登入
    if (responseMessage.equals("Not succesfully"))
    {
        responseMessage="";
        cookie="";
        randString="";
    }
   
    Serial.println(responseMessage);
    
    //關閉Http 連線，並清空 responseMessage 變數內容
   stopHttpConnection();
   responseMessage="";
}


/******************************************************
**                Http 關閉連線函數                  **
******************************************************/
void stopHttpConnection()
{
    //判斷http 連線是存在的，而且已經把 http response 結果讀取完畢，才進行連線關閉
     if (!client.connected())
     {
          Serial.println();
          Serial.println("disconnecting.");
          client.stop();
    }
}
