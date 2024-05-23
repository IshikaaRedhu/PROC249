#include<WiFi.h>
#include<Adafruit_MQTT.h>
#include<Adafruit_MQTT_Client.h>
#include<DHT.h>

//  rgb led details
byte rpin = 25;
byte gpin = 26;
byte bpin = 27;
byte rchannel = 0;
byte gchannel = 1;
byte bchannel = 2;
byte resolution = 8;
int frequency = 5000;
byte rval , gval , bval = 0;

//  dht details
byte dht_pin = 4;
#define dht_type DHT11
DHT dht(dht_pin , dht_type);

#define WLAN_SSID       "WR3005N3-757E"
#define WLAN_PASS       "70029949"


#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883           
#define AIO_USERNAME  "Tamtap"
#define AIO_KEY       "--"



//  client details
WiFiClient wificlient;
Adafruit_MQTT_Client mqtt(&wificlient , IO_BROKER , IO_PORT , IO_USERNAME , IO_KEY);

//  attaching io feeds named redvalue , greenvalue , bluevalue with out objects red, green, blue
Adafruit_MQTT_Subscribe red = Adafruit_MQTT_Subscribe(&mqtt , IO_USERNAME"/feeds/redvalue");
Adafruit_MQTT_Subscribe green = Adafruit_MQTT_Subscribe(&mqtt , IO_USERNAME"/feeds/greenvalue");
Adafruit_MQTT_Subscribe blue = Adafruit_MQTT_Subscribe(&mqtt , IO_USERNAME"/feeds/bluevalue");

//  attaching feeds to object for publishing
Adafruit_MQTT_Publish dp = Adafruit_MQTT_Publish(&mqtt , IO_USERNAME"/feeds/dew");
Adafruit_MQTT_Publish tc = Adafruit_MQTT_Publish(&mqtt , IO_USERNAME"/feeds/temperature celcius");
Adafruit_MQTT_Publish h = Adafruit_MQTT_Publish(&mqtt , IO_USERNAME"/feeds/humidity");

void setup()
{
  Serial.begin(115200);

  //  connecting with wifi
  Serial.print("Connecting with : ");
  Serial.println(ssid);
  WiFi.begin(ssid , password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  Serial.println();

  //  RGB led setup
  ledcSetup(rchannel , frequency , resolution);
  ledcSetup(gchannel , frequency , resolution);
  ledcSetup(bchannel , frequency , resolution);

  //  attaching pins with channel
  ledcAttachPin(rpin , rchannel);
  ledcAttachPin(gpin , gchannel);
  ledcAttachPin(bpin , bchannel);

  //  dht setup
  dht.begin();

  //  feeds to be subscribed
  mqtt.subscribe(&red);
  mqtt.subscribe(&green);
  mqtt.subscribe(&blue);
}

void loop()
{
  //  connecting with server
  mqttconnect();

  //  reading values from dht sensor
  float tempc = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(tempc) ||  isnan(humidity))
  {
    Serial.println("Sensor not working!");
    delay(1000);
    return;
  }

  //  printing these values on serial monitor
  String val = String(tempc) + " *C" +  String(humidity) + " %RH";
  Serial.println(val);

  //  publishing values on IO : feed object.publish(data)
  if (!tc.publish(tempc) ||  !h.publish(humidity))
  {
    Serial.println("Can't publish!");
  }
  

  //  subscribing feeds, making subscription pointer to get values
  //  only this type of variable/object can catch feed and then we can extract its value.
  Adafruit_MQTT_Subscribe *subscription;
  while (true)
  {
    subscription = mqtt.readSubscription(5000); 
    if (subscription  ==  0)  //  after timeout or 5sec , either feed is returned or 0
    {
      Serial.println("Can't catch feed");
      break;
    }
    else  //  got something
    {
      if (subscription  ==  &red)
      {
        String temp = (char *)red.lastread;
        rval = temp.toInt();
        makecolor(rval , gval , bval);
      }
      
      else if (subscription  ==  &green)
      {
        String temp = (char *)green.lastread;
        gval = temp.toInt();
        makecolor(rval , gval , bval);
      }

      else if (subscription  ==  &blue)
      {
        String temp = (char *)blue.lastread;
        bval = temp.toInt();
        makecolor(rval , gval , bval);
      }
    }
  }

  delay(7000);
}

void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()){
    return;
  }
  
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { 
       mqtt.disconnect();
       delay(5000);  
       retries--;
       if (retries == 0) {
         
         while (1);
       }
  }
  
}

void makecolor(byte r , byte g , byte b)
{
  //  printing values
  Serial.print("RED : ");
  Serial.print(r);
  Serial.print("GREEN : ");
  Serial.print(g);
  Serial.print("BLUE : ");
  Serial.println(b);

  //  writing values
  ledcWrite(rchannel , r);
  ledcWrite(gchannel , g);
  ledcWrite(bchannel , b);
}