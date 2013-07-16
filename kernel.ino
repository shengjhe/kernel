#include <Ethernet.h>
#include <SPI.h>
int pin =2;
int hpin=3;
int signal=0;
int result=2;
long  updatatime=0;
long starttime=0;
//網路參數宣告
//設定網路卡位址
byte mac[] = { 0x00, 0x51, 0x56, 0xAC, 0x31, 0x21 };

byte ipaddr []={192,168,2,198};
byte netmask[]={255,255,255,0};
byte getway[]={192,168,2,1};
byte googledns[]={168,95,1,1};
char server[] = "192.168.2.1";
String localAddress="shengjhe";

EthernetClient client;
void setup (){
  Serial.begin(9600);
  pinMode(13,OUTPUT);
  while (!Serial); 
  Ethernet.begin(mac,ipaddr,googledns,getway,netmask);
  Serial.println(Ethernet.localIP());
  delay(1000);
  Serial.println("connecting...");
}
void loop(){
  if(Serial.available() >0||millis() > updatatime)
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
    } else if(str ==0x03 && (millis()-starttime)>3000)
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
      updatatime=millis()+120000;
    }
  }
   
  
}


void sendMessage( )
{
  if (client.connect(server, 8080)) {
      Serial.println("connected");
      client.print("GET /TMIProject/api/status/");
      client.print(localAddress);
      client.print("/");
      client.print(result);
      client.println("");
      client.println("Connection: close");
      client.println();
      Serial.print("The message is send");
      Serial.println(result);
  } 
  else {
    Serial.println("connection failed");
  }

  while(client.available()) {
     delay(50);
  }

   while(client.connected()) {
    delay(50);
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }
}

