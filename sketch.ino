#include <Wire.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <ESP32Servo.h>
#include <SPI.h>

// Define arrays foor temp. and light intensity
char temp_arr[10];
char l_light_arr[8];
char r_light_arr[8];

// define 2 LDRs
#define LDR_L 36
#define LDR_R 39

// define servo
#define SERVMO 17
int pos = 0;
Servo servo;

// define given and initial values
float gamma_value = 0.75;
int theta_offset = 30;
float intensity = 0;

// initialize a MQTT client with WiFi client for network communication
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// definitions for time over wi-fi
#define NTP_SERVER "pool.ntp.org"
int UTC_OFFSET ; //UK time
#define UTC_OFFSET_DST 0

// define 4 push buttons
#define CANCEL 34
#define UP 35
#define DOWN 33
#define OK 32

//define buzzer,led and dht22
#define BUZZER 18
#define LED 15
#define DHT_PIN 12

// OLED screen definitions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C


// object Declarations
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHTesp dhtSensor;

unsigned long timeNow = 0;
unsigned long timeLast = 0;

// initialize alarm variables
bool alarm_enabled=true;
int n_alarms=3;
int alarm_hours[]={0,0,0};
int alarm_minutes[] = {0,0,0};
bool alarm_triggered[] = {false, false};

// buzzer sound notes
int n_notes=8;
int C=262;
int D=294;
int E=330;
int F=349;
int G=392;
int A=440;
int B=494;
int C_H=523;

int notes[]={C,D,E,F,G,A,B,C_H};

//initialize time variables
int days=0;
int hours=0;
int minutes=0;
int seconds=0;

// declare modes for push button selections
int current_mode = 0;
int max_modes = 5;
String options[] = {"1 - Set Time Zone", "2 - Set Alarm 1", "3 - Set Alarm 2", "4 - Set Alarm 3","5 - Disable Alarm"};

void setup() {
  // pin modes
  Serial.begin(9600);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);

  pinMode(CANCEL, INPUT);
  pinMode(UP, INPUT);
  pinMode(DOWN, INPUT);
  pinMode(OK, INPUT);

  pinMode(LDR_L, INPUT);
  pinMode(LDR_R, INPUT);

  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed..."));
    for (;;) {} // Infinite loop if display initialization fails
  }

  display.display();
  delay(2000);

  setup_Mqtt();

  servo.attach(17);
  
  display.clearDisplay();
  print_line("Welcome to Medibox", 0, 0, 2);
  delay(3000);
  display.clearDisplay();

  // Connect to WiFi
  WiFi.begin("Wokwi-GUEST", ""); // Enter your SSID and password here
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    display.clearDisplay();
    print_line("Connecting to WiFi...", 0, 0, 2);
  }
  display.clearDisplay();
  print_line("Connected to WiFi", 0, 0, 2);
  configTime(UTC_OFFSET * 3600, UTC_OFFSET_DST, NTP_SERVER);
}

void loop() {
  if (!mqttClient.connected()) {
    Serial.println("Reconnecting to MQTT Broker");
    connect_Broker();
  }

  mqttClient.loop();

  update_light();

  Serial.println(temp_arr);
  mqttClient.publish("210490L_LIGHT_L", l_light_arr);
  delay(50);
  mqttClient.publish("210490L_LIGHT_R", r_light_arr);
  delay(50);
  mqttClient.publish("210490L_TEMP", temp_arr);
  delay(50);

  update_time_with_check_alarm();
  if (digitalRead(OK) == LOW) {
    delay(2000);
    go_to_menu();
  }
  check_temp();
}

// setting up MQTT client with server connection and callback function.
void setup_Mqtt() {
  mqttClient.setServer("test.mosquitto.org", 1883);
  mqttClient.setCallback(recieveCallback);
}

// function to connect to the MQTT broker and subscribe to specific topics.
void connect_Broker() {
  while(!mqttClient.connected()){
    Serial.println("Attempting MQTT connection...");
    if(mqttClient.connect("ESP32-12345645454")){
      Serial.println("connected");
      mqttClient.subscribe("210490L_ANGLE");
      mqttClient.subscribe("210490L_SERVO_CTRL_FACTOR");
      mqttClient.subscribe("210490L_ON_OFF");
      mqttClient.subscribe("210490L_SCH_ON");
    }else{
      Serial.println("failed");
      Serial.println(mqttClient.state());
      delay(5000);
    }
  }
}

// MQTT message receive callback for handling incoming messages and updating device state.
void recieveCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Arrived msg: [");
  Serial.print(topic);
  Serial.print("] ");

  char payloadCharAr[length];
  Serial.print("Message Recieved: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    payloadCharAr[i] = (char)payload[i];
  }
  Serial.println();
  if (strcmp(topic, "210490L_ON_OFF") == 0) {
    if (payloadCharAr[0] == '1') {
      digitalWrite(15, HIGH);
    } else {
      digitalWrite(15, LOW);
    }
  } else if (strcmp(topic, "210490L_ANGLE") == 0) {
    theta_offset = String(payloadCharAr).toInt();

  } else if (strcmp(topic, "210490L_SERVO_CTRL_FACTOR") == 0) {
    gamma_value = String(payloadCharAr).toFloat();
  }
}

//update light sensor values and calculate adjustments for angle update.
void update_light() {
  float l_sv = analogRead(LDR_L) * 1.00;
  float r_sv = analogRead(LDR_R) * 1.00;

  float left_sv_ch = (float)(l_sv - 4063.00) / (32.00 - 4063.00);
  float right_sv_ch = (float)(r_sv - 4063.00) / (32.00 - 4063.00);

  update_angle(left_sv_ch, right_sv_ch);

  String(left_sv_ch).toCharArray(l_light_arr, 8);
  String(right_sv_ch).toCharArray(r_light_arr, 8);
}

// update servo angle based on light sensor readings and configured parameters.
void update_angle(float lsv, float r_sv) {
  float max_intensity = lsv;
  float D = 1.5;

  if (r_sv > max_intensity) {
    max_intensity = r_sv;
    D = 0.5;
  }

  int theta = theta_offset * D + (180 - theta_offset) * max_intensity * gamma_value;
  theta = min(theta, 180);

  servo.write(theta);
}

// control servo position based on intensity and parameters.
void servo_motor(){
  pos = theta_offset + (180 - theta_offset) * intensity * gamma_value;
  servo.write(pos);
}

// print line function
void print_line(String text, int row, int column, int textSize) {
  display.setTextSize(textSize);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(column, row);
  display.println(text);
  display.display();
}

// current time print function
void print_time_now(void) {
  display.clearDisplay();
  print_line(String(days)+ ":" + String(hours) + ":" + String(minutes) + ":" + String(seconds), 0, 0, 2);
  delay(500);
}

// function to update time with alarm checking
void update_time_with_check_alarm(void){
    display.clearDisplay();
    update_time();
    print_time_now();
    if (alarm_enabled) {
      for (int i = 0; i < n_alarms ; i++) {
        if (alarm_triggered[i] == false && hours == alarm_hours[i] && minutes == alarm_minutes[i]) {
          ring_alarm(); 
          alarm_triggered[i] = true;
        }
      }
    }
}

// ring alarm function
void ring_alarm() {
  display.clearDisplay();
  print_line("Medicine Time",0, 0, 2);
  digitalWrite(LED, HIGH);
  bool break_happen=false;
  while(break_happen==false && digitalRead(CANCEL)==HIGH){
    for (int i = 0; i < n_alarms; i++) {
      if (digitalRead(CANCEL) == LOW){
        delay(200);
        break_happen=true;
        break;
      }
      tone(BUZZER, notes[i]);
      delay(500);
      noTone(BUZZER);
      delay(2);

    }
  }
  digitalWrite(LED, LOW);
  display.clearDisplay();
}

// return values for button pressings
int wait_for_button_press() {
  while (true) {
    if (digitalRead(UP) == LOW) {
      delay(200);
      return UP;
    }
    else if (digitalRead(DOWN) == LOW) {
      delay(200);
      return DOWN;
    }
    else if (digitalRead(CANCEL) == LOW) {
      delay(200);
      return CANCEL;
    }
    else if (digitalRead(OK) == LOW) {
      delay(200);
      return OK;
    }
    update_time();
 }
}

// go to menu function
void go_to_menu(void) {
  while (digitalRead(CANCEL) == HIGH) {
    display.clearDisplay();
    print_line(options[current_mode], 2, 0, 0);
    int pressed = wait_for_button_press();
    if (pressed == UP) {
      current_mode += 1;
      current_mode %= max_modes;
      delay(200);
    }
    else if (pressed == OK) {
      delay(200);
      run_mode(current_mode);
    }
    else if (pressed == DOWN) {
      delay(200);
      current_mode -= 1;
      if (current_mode < 0) {
      current_mode = max_modes - 1;
      }
    }
  }
}

// run mode function
void run_mode(int mode) {
  if (mode == 0) {
    set_time_zone();
  }
  else if (mode == 1 || mode == 2|| mode==3) {
    set_alarm(mode - 1);
  }
  else if (mode == 4) {
    alarm_enabled = false;
  }
}

void set_alarm(int alarm) {
  int temp_hour = alarm_hours[alarm];
  while (true) {
    display.clearDisplay();
    print_line("Enter hour: " + String(temp_hour), 0, 0, 2);
    int pressed = wait_for_button_press();
    if (pressed == UP) {
      delay(200);
      temp_hour += 1;
      temp_hour = temp_hour % 24;
    }
    else if (pressed == DOWN) {
      delay(200);
      temp_hour -= 1;
      if (temp_hour < 0) {
      temp_hour = 23;
      }
    }
    else if (pressed == OK) {
      delay(200);
      alarm_hours[alarm] = temp_hour;
      break;
    }
    else if (pressed == CANCEL){
      delay(200);
      break;
    }
  }

  int temp_minute = alarm_minutes[alarm];
  while (true) {
    display.clearDisplay();
    print_line("Enter minute: " + String(temp_minute), 0, 0, 2);
    int pressed = wait_for_button_press();
    if (pressed == UP) {
      delay(200);
      temp_minute += 1;
      temp_minute = temp_minute % 60;
    }
    else if (pressed == DOWN) {
      delay(200);
      temp_minute -= 1;
      if (temp_minute < 0) {
        temp_minute = 59;
      }
    }
    else if (pressed == OK) {
      delay(200);
      alarm_minutes[alarm] = temp_minute;
      break;
    }
    else if (pressed == CANCEL) {
      delay(200);
      break;
    }
  }
  display.clearDisplay();
  print_line("Alarm is set", 0, 0, 2);
  delay(1000);
}

void check_temp(void) {
    TempAndHumidity data = dhtSensor.getTempAndHumidity();
    String(data.temperature, 2).toCharArray(temp_arr, 10);
    if (data.temperature > 32) {
       // display.clearDisplay();
        print_line("TEMP HIGH", 40, 0, 1);
    } else if (data.temperature < 26) {
       // display.clearDisplay();
        print_line("TEMP LOW", 40, 0, 1);
    }

    if (data.humidity > 80) {
       // display.clearDisplay();
        print_line("HUMD HIGH", 50, 0, 1);
    } else if (data.humidity < 60) {
       // display.clearDisplay();
        print_line("HUMD LOW", 50, 0, 1);
    }  
    delay(1000); 
}

// function to automatically update the current time over WIFI
void update_time(void) {
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  char day_str[3];
  char hour_str[3];
  char min_str[3];
  char sec_str[3];
  strftime(day_str, 3, "%d", &timeinfo);
  strftime(sec_str, 3, "%S", &timeinfo);
  strftime(hour_str, 3, "%H", &timeinfo);
  strftime(min_str, 3, "%M", &timeinfo);
  hours = atoi(hour_str);
  minutes = atoi(min_str);
  days = atoi(day_str);
  seconds = atoi(sec_str);
}

void set_time_zone() {
  int temp_hour=0;
  while (true) {
    display.clearDisplay();
    print_line("Enter offset hour: " + String(temp_hour), 0, 0, 2);
    int pressed = wait_for_button_press();
    if (pressed == UP) {
      delay(200);
      temp_hour += 1;
      temp_hour = temp_hour % 24;
    }
    else if (pressed == DOWN) {
      delay(200);
      temp_hour -= 1;
      if (temp_hour < 0) {
      temp_hour = 23;
      }
    }
    else if (pressed == OK) {
      delay(200);
      hours = hours + temp_hour;
      break;
    }
    else if (pressed == CANCEL){
      delay(200);
      break;
    }
  }
  int temp_minute = 0;
  while (true) {
    display.clearDisplay();
    print_line("Enter offset minute: " + String(temp_minute), 0, 0, 2);
    int pressed = wait_for_button_press();
    if (pressed == UP) {
      delay(200);
      temp_minute += 1;
      temp_minute = temp_minute % 60;
    }
    else if (pressed == DOWN) {
      delay(200);
      temp_minute -= 1;
      if (temp_minute < 0) {
        temp_minute = 59;
      }
    }
    else if (pressed == OK) {
      delay(200);
      minutes = minutes + temp_minute;
      break;
    }
    else if (pressed == CANCEL) {
      delay(200);
      break;
    }
  }
  UTC_OFFSET = temp_hour*3600 + temp_minute*60;
  configTime(UTC_OFFSET,UTC_OFFSET_DST,NTP_SERVER);
  display.clearDisplay();
  print_line("Time is set", 0, 0, 2);
  delay(1000);
}