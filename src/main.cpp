#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureAxTLS.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiServerSecureAxTLS.h>
#include <WiFiServerSecureBearSSL.h>
#include <WiFiUdp.h>
#include <LiquidCrystal_I2C.h>
//#include <LiquidCrystal_I2C.h>
//#define ESP32
#ifdef ESP8266 || ESP32
#define ISR_PREFIX ICACHE_RAM_ATTR
#else
#define ISR_PREFIX
#endif
#include <SocketIOClient.h>
#include <ArduinoJson.h>
//#include <WiFi.h>
//#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//LiquidCrystal_I2C //////////////lcd(0x27, 20, 4);

//LiquidCrystal_I2C //////////////lcd(0x27, 20, 4);
SocketIOClient client;
//const char *ssid = "SHC-INT-1";
//const char *password = "20182019";

//char host[] = "192.168.137.9";

const char *ssid = "SHC-INT";
const char *password = "20182019";
char host[] = "10.4.0.57";

int port = 3484;

extern String RID;
extern String Rname;
extern String Rcontent;

long durMinute;
long durSecond;

long rssi = WiFi.RSSI();
unsigned long previousMillis = 0;
unsigned long interval = 1000;
unsigned long startMilli;

const byte proximitySensor = 12;
const byte reflectiveSensor = 13;

//const byte dc5vRelay = 13;
// static const uint8_t D0   = 16;
// static const uint8_t D1   = 5;
// static const uint8_t D2   = 4;
// static const uint8_t D3   = 0;
// static const uint8_t D4   = 2;
// static const uint8_t D5   = 14;
// static const uint8_t D6   = 12;
// static const uint8_t D7   = 13;
// static const uint8_t D8   = 15;
// static const uint8_t D9   = 3;
// static const uint8_t D10  = 1;

bool movedDown = false; //trạng thái máy đã dập xuống trước đó
bool movedOut = false;  //trạng thái máy đã xoay ra trước đó
bool movedIn = false;   //trạng thái máy đã xoay vào trước đó

long timePointDown = 0; //thời điểm máy dập xuống
long timePointOut = 0;  //thời điểm máy xoay ra
long timePointIn = 0;   //thời điểm máy xoay vào
long timePointUp = 0;

int durDownOut = 0; //khoảng thời gian từ lúc máy dập xuống đến lúc máy xoay ra
int durOutIn = 0;   //khoảng thời gian từ lúc máy xoay ra đến lúc máy xoay vào
int durInDown = 0;  //khoảng thời gian từ lúc máy xoay vào đến lúc máy dập xuống
int durInOut = 0;   //khoảng thời gian từ lúc máy xoay vào, không dập, đến lúc máy xoay ra
int durDownUp = 0;

int sumdurOutIn = 0; //tổng các khoảng thời gian từ lúc máy xoay ra đến lúc máy xoay vào
int sumdurInOut = 0; //tổng các khoảng thời gian từ lúc máy xoay vào, không dập, đến lúc máy xoay ra

int sequence = 0; //biến đếm chu kỳ

StaticJsonDocument<200> doc;
JsonObject root = doc.to<JsonObject>();

// volatile unsigned int pulses; //Biến lưu giá trị thời điểm trước đó bắt đầu tính vat can.
// // volatile có nghĩa là ra lệnh cho trình biên dịch biết rằng giá trị của biến có thể bị thay đổi bởi một lời gọi hàm ngầm định nào đó
// long rpm;//Biến lưu giá trị tốc độ tính được: số vòng trên phút.
// unsigned long timeOld;

// void counter()
// {
//   pulses++;
// }

//hàm thực hiện khi cảm biến gương bắt tín hiệu
//code chưa hoàn chỉnh, chưa tính trường hợp có thể máy xoay ra xoay vào nhiều lần mà không có dập xuống
ISR_PREFIX void doReflectiveTask()
{ // khi gương bắt tín hiệu

  if (movedOut == true) //(4)
  {                     //nếu lần xoay trước đó là xoay ra

    movedIn = true;   //thì lần này máy xoay vào, bật trạng thái xoay vào để vào đoạn (3)
    movedOut = false; //tắt đi trạng thái xoay ra

    movedDown = false; //tắt trạng thái máy dập để sẵn sàng vào bước (1)

    timePointIn = millis();                //lấy thời điểm hiện tại
    durOutIn = timePointIn - timePointOut; //trừ cho thời điểm lúc máy xoay ra, thành dur
    sumdurOutIn += durOutIn;               //tính tổng thời gian các lần ra-vào trong cùng một sequence

    //    //////////////lcd.clear();
    //    //////////////lcd.setCursor(0, 0);
    //    //////////////lcd.print("durOutIn");
    //    //////////////lcd.setCursor(0, 1);
    //    ////////////lcd.print("Reconnecting... ");

    //explain root//
    //a = arduinoId
    //s = signalStart
    //e = signalEnd
    //d = duration
    //sq = sequence
    //c = createdTime

    String JSON;
    StaticJsonDocument<200> root;
    root["a"] = "M01";
    root["s"] = "O";
    root["e"] = "I";
    root["d"] = sumdurOutIn;
    root["sq"] = sequence;
    root["c"] = "a";

    serializeJson(doc, Serial);
    // Serial.println();
    serializeJson(root, JSON);

    client.send("command2", JSON);
    JSON = "";

    // // Serial.println(""); //in
    // // Serial.println("I");
    // // Serial.print("OutIn: ");
    // // Serial.println(sumdurOutIn);
    // // Serial.print("Seq: ");
    // // Serial.println(sequence);
  }

  else if (movedIn == true) //(3)
  {                         //nếu trước đó máy xoay vào
    movedOut = true;        // thì lần này máy xoay ra, trạng thái này dùng cho đoạn (4)
    movedIn = false;        // tắt trạng thái xoay vào

    if (movedDown == true) //(3.1)
    {                      //nếu trước khi xoay ra mà máy có dập xuống

      timePointOut = millis();                   //lấy thời điểm hiện tại
      durDownOut = timePointOut - timePointDown; //trừ cho thời điểm lúc máy dập xuống, thành dur

      String JSON;
      StaticJsonDocument<200> root;

      root["a"] = "M01";
      root["s"] = "D";
      root["e"] = "O";
      root["d"] = durDownOut;
      root["sq"] = sequence;
      root["c"] = "a";

      serializeJson(root, Serial);
      // Serial.println();
      serializeJson(root, JSON);

      client.send("command2", JSON);
      JSON = "";

      // // Serial.println("");
      // // Serial.println("O");
      // // Serial.print("DownOut: ");
      // // Serial.println(durDownOut);
      // // Serial.print("Seq: ");
      // // Serial.println(sequence);
    }
    else if (movedDown == false) //(3.2)
    {                            //còn nếu trước khi xoay ra mà máy không có dập xuống, nghĩa là trước đó máy xoay vào,
      //chưa dập mà đã xoay ra

      timePointOut = millis();               //lấy thời điểm hiện tại
      durInOut = timePointOut - timePointIn; //tính khoảng thời gian vào-ra mà không dập
      sumdurInOut += durInOut;               //tính tổng các lần như vậy

      String JSON;
      StaticJsonDocument<200> root;

      root["a"] = "M01";
      root["s"] = "I";
      root["e"] = "O";
      root["d"] = sumdurInOut;
      root["sq"] = sequence;
      root["c"] = "a";

      serializeJson(root, Serial);
      // Serial.println();
      serializeJson(root, JSON);

      client.send("command2", JSON);
      JSON = "";

      // // Serial.println("");
      // // Serial.println("O");
      // // Serial.print("InOut: ");
      // // Serial.println(sumdurInOut);
      // // Serial.print("Seq: ");
      // // Serial.println(sequence);
    }
  }
}

//hàm thực hiện khi cảm biến tiệm cận bắt tín hiệu
ISR_PREFIX void doProximityTask()
{
  if (movedOut == true)
  { //nếu máy đang xoay ra mà dập, nghĩa là quá trình xoay ra-vào bị lỗi, thực chất máy vừa xoay vào
    
    //đặt lại logic để cho phép tính in-down và cycle time
    movedOut = false;
    movedIn = true;
    movedDown = false;

    //thời điểm xoay ra gần nhất chính là thời điểm xoay vào
    timePointIn = timePointOut;

    //chuyển đến vòng tính bình thường bên dưới
  }

  if (movedOut == false)
  { //nếu máy không xoay ra thì mới tính
    //**Khi mới khởi động, các trạng thái movedIn và movedOut đều là false, nên nếu đặt điều kiện (movedIn == true),
    //thì sẽ không bao giờ vào được code,
    //cho nên đặt điều kiện (movedOut == false) để loại bỏ trường hợp máy dập khi đang xoay ra, đồng thời để code chạy được khi
    //vừa khởi động code lúc (movedIn = false)

    if (movedDown == false) //(1)
    {                       //nếu ngay trước đó không có lần dập nào khác,
      //nghĩa là đây là lần dập đầu tiên từ khi máy xoay vào

      timePointDown = millis(); //lấy hiện tại

      durInDown = timePointDown - timePointIn; //tính dur từ lúc xoay vào

      int cycleTime = durInDown + durDownOut + sumdurOutIn + sumdurInOut; //tính tổng cycle time

      String JSON;
      StaticJsonDocument<200> root;

      root["a"] = "M01";
      root["s"] = "I";
      root["e"] = "D";
      root["d"] = durInDown;
      root["sq"] = sequence;
      root["c"] = "a";

      serializeJson(root, Serial);
      // Serial.println();
      serializeJson(root, JSON);

      client.send("command2", JSON);
      JSON = "";

      // // Serial.println("");
      // // Serial.println("D");
      // // Serial.print("InDown: ");
      // // Serial.println(durInDown);
      // // Serial.print("Seq: ");
      // // Serial.println(sequence);

      String JSON2;
      StaticJsonDocument<200> root2;

      root2["a"] = "M01";
      root2["s"] = "D";
      root2["e"] = "D";
      root2["d"] = cycleTime;
      root2["sq"] = sequence;
      root2["c"] = "a";

      serializeJson(root2, Serial);
      // Serial.println();
      serializeJson(root2, JSON2);

      client.send("command2", JSON2);
      JSON2 = "";

      // // Serial.println("");
      // // Serial.print("Cycle Time: ");
      // // Serial.println(cycleTime);
      // // Serial.print("Seq: ");
      // // Serial.println(sequence);

      sequence++; //tăng biến đếm phiên làm việc

      movedIn = true; //bật trạng thái này lên để báo hiệu lần xoay tiếp theo sẽ là xoay ra, để vào đoạn (3)

      sumdurOutIn = 0; //reset biến
      sumdurInOut = 0; //reset biến
    }
    else if (movedDown == true) //(2)
    {                           //Nếu đã có dập lần đầu thì các lần dập liên tiếp không tính toán gì
    }
  }
}

ISR_PREFIX void doProximityTask2()
{
  if (movedDown == false)
  {
    timePointUp = millis();
    durDownUp = timePointUp - timePointDown;

    String JSON;
    StaticJsonDocument<200> root;

    root["a"] = "M01";
    root["s"] = "D";
    root["e"] = "U";
    root["d"] = durDownUp;
    root["sq"] = sequence;
    root["c"] = "a";

    serializeJson(root, Serial);
    // Serial.println();
    serializeJson(root, JSON);

    client.send("command2", JSON);
    JSON = "";
  }
  movedDown = true; //báo hiệu rằng đã dập lần đầu rồi để đi vào đoạn (2)
}

ISR_PREFIX void check()
{
  if (digitalRead(proximitySensor) == HIGH)
  {
    doProximityTask();
  }
  else
  {
    doProximityTask2();
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(proximitySensor, INPUT);
  pinMode(reflectiveSensor, INPUT);
  // pulses = 0;
  // timeOld = 0;

  ////////////lcd.init();      //Khởi động màn hình. Bắt đầu cho phép Arduino sử dụng màn hình
  ////////////lcd.backlight(); //Bật đèn nền
  ////////////lcd.clear();
  //////////////lcd.begin();       //Khởi động màn hình. Bắt đầu cho phép Arduino sử dụng màn hình
  //////////////lcd.backlight();   //Bật đèn nền
  //////////////lcd.clear();
  delay(10);
  // We start by connecting to a WiFi network
  // Serial.println();
  // Serial.print("Connecting to ");
  // Serial.println(ssid);
  //// Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    long rssi = WiFi.RSSI();
    Serial.print("RSSI:");
    Serial.println(rssi);
    delay(1000);
    ////////////lcd.setCursor(0, 0);
    ////////////lcd.print("Connecting Wifi ");
    ////////////lcd.setCursor(0, 1);
    ////////////lcd.print("RSSI: ");
    ////////////lcd.print(rssi);
  }
  ////////////lcd.clear();
  ////////////lcd.setCursor(0, 0);
  ////////////lcd.print("Wifi Connected");
  ////////////lcd.setCursor(0, 1);
  ////////////lcd.print(WiFi.localIP());
  // Serial.print("WiFi.status()   :   ");
  // Serial.println(WiFi.status());

  //////////////lcd.clear();
  //////////////lcd.setCursor(0, 0);
  //////////////lcd.print("Wifi Connected");
  //////////////lcd.setCursor(0, 1);
  //////////////lcd.print(WiFi.localIP());
  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  // ArduinoOTA.setHostname("myesp32");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  // Serial.println("Ready");
  // Serial.print("IP address: ");
  // Serial.println(WiFi.localIP());
  // Serial.println("Start connect to socket server . . .");
  bool checkconnect = client.connect(host, port);
  // Serial.print("checkconnect  :  ");
  // Serial.println(checkconnect);
  if (!checkconnect)
  {

    // Serial.println("connection failed");
    delay(60000);
    ESP.restart();
  }
  if (client.connected())
  {
    ////////////lcd.clear();
    ////////////lcd.setCursor(0, 0);
    ////////////lcd.print("Server Connected");
    ////////////lcd.setCursor(0, 1);
    ////////////lcd.print("               ");
    client.send("connection", "message", "Connected !");
    delay(2000);
    ////////////lcd.clear();
    //////////////lcd.clear();
    //////////////lcd.setCursor(0, 0);
    //////////////lcd.print("Server Connected");
    //////////////lcd.setCursor(0, 1);
    //////////////lcd.print("               ");
    client.send("connection", "message", "Connected !");
    delay(2000);
    //////////////lcd.clear();
  }

  attachInterrupt(digitalPinToInterrupt(proximitySensor), check, CHANGE); //liên tục làm 2 cái chuyện trên

  attachInterrupt(digitalPinToInterrupt(reflectiveSensor), doReflectiveTask, FALLING); //liên tục làm 2 cái chuyện trên
  //attachInterrupt(digitalPinToInterrupt(proximitySensor), doProximityTask, FALLING);//phát hiện tín hiệu cảm biến tiệm cận
  //attachInterrupt(digitalPinToInterrupt(reflectiveSensor), doReflectiveTask, FALLING);//phát hiện tín hiệu cảm biến gương
}

void loop()
{
  ArduinoOTA.handle();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis > interval)
  {
    previousMillis = currentMillis;
    //client.heartbeat(0);
    client.send("atime", "message", "Time please?");
  }
  if (client.monitor())
  {
    if (RID == "atime" && Rname == "time")
    {
      // Serial.print("Il est ");
      // Serial.println(Rcontent);
      client.send("revatime", "time", Rcontent);
    }
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    // Serial.print("Mat ket noi wifi");
    delay(1000);
    //ESP.restart();
  }
  //Kết nối lại!
  //// Serial.println("client.connected :  ");
  //// Serial.println(client.connected());
  if (!client.connected())
  {
    // Serial.print("Mat ket noi server");
    //////////////lcd.clear();
    //////////////lcd.setCursor(0, 0);
    //////////////lcd.print("Disconnected Server!");
    //////////////lcd.setCursor(0, 1);
    //////////////lcd.print("Reconnecting... ");
    delay(1000);
    client.reconnect(host, port);
    //////////////lcd.clear();
    //ESP.restart();
  }
}
