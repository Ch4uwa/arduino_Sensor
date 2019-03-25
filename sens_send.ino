#include "SoftwareSerial.h"
#include "WiFiEsp.h"
#include "dht.h"

#define a_pin A0

dht DHT;
SoftwareSerial Serial1(6, 7); // RX, TX

char ssid[] = "xxxxx";        // your network SSID (name)
char pass[] = "xxxxx";    // your network password
int status = WL_IDLE_STATUS; // the Wifi radio's status

char server[] = "driftier-operations.000webhostapp.com"; // 92.35.102.95
int port = 80;

long prevMillis{}, interval{};
unsigned long currentMillis{};
String t{}, h{};
String data = "";

// Initialize client object
WiFiEspClient client;

void setup()
{
    // initialize serial for debugging
    Serial.begin(115200);
    // initialize serial for ESP module
    Serial1.begin(9600);

    wifiConnect();
    printWifiStatus();

    DHT.read11(a_pin);
    delay(1000);
    h = DHT.humidity;
    t = DHT.temperature;
}

void loop()
{
    readSens();
    serverSend();
    delay(300000);
}

void wifiConnect()
{
    // initialize ESP module
    WiFi.init(&Serial1);

    // check for the presence of the shield
    if (WiFi.status() == WL_NO_SHIELD)
    {
        Serial.println("WiFi shield not present");
        // don't continue
        while (true)
            ;
    }

    // attempt to connect to WiFi network
    while (status != WL_CONNECTED)
    {
        Serial.print("Attempting to connect to WPA SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network
        status = WiFi.begin(ssid, pass);
    }

    // you're connected now, so print out the data
    Serial.println("You're connected to the network");
}

void readSens()
{
    currentMillis = millis();
    if (currentMillis - prevMillis > interval)
    {
        prevMillis = currentMillis;
        DHT.read11(a_pin);
        delay(1000);
        h = DHT.humidity;
        t = DHT.temperature;
    }
    data = "temp1=" + t + "&hum1=" + h + "&name=Martin";
    Serial.println(data);
}

void serverSend()
{
    Serial.println();
    Serial.println("Starting connection to server...");
    // if you get a connection, report back via serial
    if (client.connect(server, port))
    {
        Serial.println("Connected to server");
        // Make a HTTP request
        client.println("POST /add.php HTTP/1.1");
        client.println("Host: driftier-operations.000webhostapp.com"); //tekn.ddns.net
        //client.println("Connection: keep-alive");
        client.println("Content-Type: application/x-www-form-urlencoded");
        client.print("Content-Length: ");
        client.println(data.length());
        client.println();
        client.print(data);
    }
    delay(2000);
    // if there are incoming bytes available
    // from the server, read them and print them
    while (client.available())
    {
        char c = client.read();
        Serial.write(c);
    }

    // if the server's disconnected, stop the client
    if (!client.connected())
    {
        Serial.println();
        Serial.println("Disconnecting from server...");
        client.stop();
    }
}

void printWifiStatus()
{
    // print the SSID of the network you're attached to
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your WiFi shield's IP address
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // print the received signal strength
    long rssi = WiFi.RSSI();
    Serial.print("Signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
}
