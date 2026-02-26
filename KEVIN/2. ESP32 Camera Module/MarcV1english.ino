#include "FS.h"          // manipulation de fichiers
#include "SD_MMC.h"      // carte SD
#include "esp_camera.h"  // caméra!


static bool SDCardPresent = false;
int file_number = 0; // numéro de la photo (nom du fichier)


//*fonction  prise de la photo et enregistrement sur la carte SD*//
void enregistrer_photo ()
{
  char adresse[20] = ""; // chemin d'accès du fichier .jpeg
  camera_fb_t * fb = NULL; // frame buffer

 // prise de la photo
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Failed to take photo");
    return;
  }

  file_number = file_number + 1;

// enregitrement du fichier sur la carte SD
    sprintf (adresse, "/%d.jpg", file_number);

  fs::FS &fs = SD_MMC;
  File file = fs.open(adresse, FILE_WRITE);

  if (!file) {
    Serial.println("Failed to create file.");
  }
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.printf("File saved: %s\n", adresse);
  }
  file.close();
  esp_camera_fb_return(fb);

}

void setup()
{
Serial.begin(115200);
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
  }

}

void loop() 
{
  enregistrer_photo();
  Serial.print("New photo!  ");
  Serial.println(file_number);
  delay(5000);

}
