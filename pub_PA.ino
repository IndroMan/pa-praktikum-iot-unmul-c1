#include <ESP8266WiFi.h>
#include <PubSubClient.h>

bool on = false;

// Pin Ultrasonik
int echoPin = D7;
int trigPin = D8;

// Update these with values suitable for your network.
const char* ssid = "POCO F3";
const char* password = "";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// menerima data
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Pesan diterima [");
  Serial.print(topic);
  Serial.print("] ");
  String data = ""; // variabel untuk menyimpan data yang berbentuk array char
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    data += (char)payload[i]; // menyimpan kumpulan char kedalam string
  }
  Serial.println();
  
  if(data == "on"){
    on = true;
  }else if(data == "off"){
    on = false;
  }

}

// fungsi untuk menghubungkan ke broker
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("iot_unmul/iot_c_1/alat");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup(){
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Inisialisasi pin ultrasonik  
  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
}

float ultrasonik(){  //membuat fungsi ultrasonik untuk mendapatkan jarak dari sensor ultrasonik
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  int durasiPantulan = pulseIn(echoPin, HIGH);
  return durasiPantulan * 0.034 / 2; //membuat nilai dari fungsi ultrasonik menjadi hasil perhitungan durasi*0,034/2
}

void loop(){
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if(on == true){
    float jarak = ultrasonik();
    snprintf (msg, MSG_BUFFER_SIZE, "%s", itoa(jarak,msg,10)); // itoa (konversi integer ke string)
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("iot_unmul/iot_c_1", msg); // fungsi untuk publish ke broker
  }
  delay(10);
}
