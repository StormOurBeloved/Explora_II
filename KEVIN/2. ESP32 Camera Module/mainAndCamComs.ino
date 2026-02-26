#include "FS.h"          // manipulation de fichiers
#include "SD_MMC.h"      // carte SD
#include "esp_camera.h"  // caméra!

//Simon adding errorchecking
bool error = false;

#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;

bool BTenabled = true;
#include <time.h>

static bool SDCardPresent = false;
int file_number = 0; // numéro de la photo (nom du fichier)


//*fonction  prise de la photo et enregistrement sur la carte SD*//
bool enregistrer_photo ()
{
  char adresse[20] = ""; // chemin d'accès du fichier .jpeg
  camera_fb_t * fb = NULL; // frame buffer

  // prise de la photo
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Failed to take photo");
    return false;
  }

  file_number = file_number + 1;

  // enregitrement du fichier sur la carte SD
  sprintf (adresse, "/%d.jpg", file_number);

  fs::FS &fs = SD_MMC;
  File file = fs.open(adresse, FILE_WRITE);

  if (!file) {
    Serial.println("Failed to create file.");
    return false;
  }
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.printf("File saved: %s\n", adresse);
  }
  file.close();
  esp_camera_fb_return(fb);

  return true;

}

void setup()
{
  delay(5000);

  Serial.begin(115200);
  SerialBT.begin("ESPCAM1"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");

  Serial.println("Awaiting Bluetooth connection...");
  while(!SerialBT.connected() && millis() < 180000){
  }

  // définition des broches de la caméra pour le modèle AI Thinker - ESP32-CAM
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;

  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;     //YUV422|GRAYSCALE|RGB565|JPEG
  config.frame_size = FRAMESIZE_SXGA;       // QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
  config.jpeg_quality = 10;                // 0-63 ; plus bas = meilleure qualité
  config.fb_count = 1;                     // nombre de frame buffers

  // initialisation de la caméra
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera initialization failed, error 0x%x", err);
    error = true;
    return;
  }

  sensor_t * s = esp_camera_sensor_get();

  // initialisation de la carte micro SD
  // en mode 1 bit: plus lent, mais libère des broches
  if (SD_MMC.begin("/sdcard",true)) { 
    uint8_t cardType = SD_MMC.cardType();
    if (cardType != CARD_NONE) {
      Serial.println("SD-card initialized.");
      SDCardPresent = true;
    }
    else{
      error = true;
    }
  }
  else{
    error = true;
  }

  //Diagnostics with bluetooth
  if(SerialBT.connected()){
    Serial.println("Connected to external device.");
  }
  else{
    Serial.println("Could not secure connection within timeframe.");
  }

  Serial.println("Running system diagnostics...");

  if(!error){
    Serial.println("Found no errors when setting up esp32cam.");
    SerialBT.print("Found no errors when setting up esp32cam.\n");

  }
  else{
    Serial.println("Error when setting up ESPCAM.");
    SerialBT.print("Error when setting up ESPCAM.\n");
  }

}

void loop() 
{
  //Await request to take picture
  if(BTenabled){
    if(SerialBT.available()){
      String recieved = SerialBT.readStringUntil('\n');
      if(recieved == "BTStop"){
        SerialBT.print("\nESPCAM1 Terminating Bluetooth...");
        SerialBT.disconnect();
        delay(200);
        SerialBT.end();
        BTenabled = false;
        Serial.println("Disabled Bluetooth.");
      }
    }
  }

  if(enregistrer_photo()){
    Serial.print("New photo!  ");
    Serial.println(file_number);
  
    if(BTenabled){
      SerialBT.print("\nNew photo!  ");
      SerialBT.print(file_number);
    }
  }
  else{
    if(BTenabled){
      SerialBT.print("\nFailed to take picture.");
    }
  }
  delay(2000);

}
