//=============================================================================
//
//     FILE : ESP32_Cam.cpp
//
//  PROJECT : PTCamera (A Pan-n-Tilt WiFi Camera)
//
//   AUTHOR : Bill Daniels
//            Copyright 2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//            Portions Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
//=============================================================================

//--- Includes --------------------------------------------

#include "ESP32_Cam.h"

#define CAMERA_MODEL_ESP32S3_EYE
#include <camera_pins.h>

#include <WiFi.h>

//--- Defines ---------------------------------------------

#define PART_BOUNDARY "123456789000000000000987654321"

//--- Globals ---------------------------------------------

// Access Point Network for this device
const char  *ssid     = "bandbhouse";      // "PTCamera";
const char  *password = "czrrsbFuzzies6";  // "MyCoolGizmo";

const char  *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
const char  *_STREAM_BOUNDARY     = "\r\n--" PART_BOUNDARY "\r\n";
const char  *_STREAM_PART         = "Content-Type: image/jpeg\r\nContent-Length: %u\r\nX-Timestamp: %d.%06d\r\n\r\n";
const char  *webPage              = "<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head><body style=\"margin:0; padding:0\"><img src=\"/stream\" /></body></html>";

//--- C Declarations --------------------------------------

esp_err_t  indexHandler  (httpd_req_t *req);
esp_err_t  streamHandler (httpd_req_t *req);

//--- Constructor -----------------------------------------

ESP32_Cam::ESP32_Cam ()
{
  //-----------------------------------
  //  Init Camera
  //-----------------------------------
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;    // I2C pins for GOOUUU Tech ESP32-S3 CAM board are SDA=4, SCL=5
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 8000000;          // Much better frame rate if set to 8MHz instead of 20MHz  // 20000000;
  config.frame_size   = FRAMESIZE_SVGA;   // See sensor.h file @ https://github.com/espressif/esp32-camera/blob/master/driver/include/sensor.h
  config.pixel_format = PIXFORMAT_JPEG;   // PIXFORMAT_JPEG for streaming, PIXFORMAT_RGB565 for face detection/recognition
  config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location  = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 10;
  config.fb_count     = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if (config.pixel_format == PIXFORMAT_JPEG)
  {
    if (psramFound())
    {
      Serial.println ("PSRAM found.");
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    }
    else
    {
      // Limit the frame size when PSRAM is not available
      Serial.println ("No PSRAM found. Setting frame size to CIF.");
      config.frame_size = FRAMESIZE_CIF;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  }
  else
  {
    // Best option for face detection/recognition
    Serial.println ("Pixel format is not JPEG. Setting frame size to QVGA for good face recognition.");
    config.frame_size = FRAMESIZE_QVGA;

    #if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
    #endif
  }

  // Pullup resistors needed for I2C pins
  pinMode (SIOD_GPIO_NUM, INPUT_PULLUP);  // I2C SDA
  pinMode (SIOC_GPIO_NUM, INPUT_PULLUP);  // I2C SCL

  // camera init
  Serial.println ("esp_camera_init() ...");
  esp_err_t err = esp_camera_init (&config);
  if (err != ESP_OK)
  {
    Serial.printf ("Camera init failed with error 0x%x", err);
    return;
  }

  Serial.println ("esp_camera_sensor_get() ...");
  sensor_t *s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID)
  {
    s->set_vflip(s, 1);        // flip it back
    s->set_brightness(s, 1);   // up the brightness just a bit
    s->set_saturation(s, -2);  // lower the saturation
  }

  //-----------------------------------
  //  Init Web Server
  //-----------------------------------
  Serial.println ("WiFi.begin() ...");

  WiFi.mode  (WIFI_AP_STA);     // Must be both AP and STATION mode to use with ESP-NOW
  WiFi.begin (ssid, password);
  WiFi.setSleep (false);

  Serial.print ("WiFi connecting ");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print (".");
  }
  Serial.println (" WiFi connected");
  Serial.print ("WiFi Channel is "); Serial.println (WiFi.channel());

  Serial.println ("startCameraServer() ...");
  startCameraServer();

  Serial.print   ("Camera Ready! Use 'http://");
  Serial.print   (WiFi.localIP());  // 192.168.1.177
  Serial.println ("' to connect");
}

//--- startCameraServer -----------------------------------

void ESP32_Cam::startCameraServer ()
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_uri_handlers = 16;

  httpd_uri_t index_uri =
  {
    .uri = "/",
    .method = HTTP_GET,
    .handler = indexHandler,
    .user_ctx = NULL
  };

  httpd_uri_t stream_uri =
  {
    .uri = "/stream",
    .method = HTTP_GET,
    .handler = streamHandler,
    .user_ctx = NULL
  };

  Serial.print ("Starting camera server on port: "); Serial.println (config.server_port);
  if (httpd_start (&camera_httpd, &config) == ESP_OK)
  {
    httpd_register_uri_handler (camera_httpd, &index_uri);
    httpd_register_uri_handler (camera_httpd, &stream_uri);
  }
}


//=========================================================
//  Straight 'C' functions
//=========================================================

//--- indexHandler ----------------------------------------

esp_err_t indexHandler (httpd_req_t *req)
{
  httpd_resp_set_type (req, "text/html");

  sensor_t *s = esp_camera_sensor_get ();
  if (s != NULL)
    return httpd_resp_send (req, webPage, strlen(webPage)+1);

  Serial.println ("Camera sensor not found");
  return httpd_resp_send_500 (req);
}

//--- streamHandler ---------------------------------------

esp_err_t streamHandler (httpd_req_t *req)
{
  camera_fb_t     *fb = NULL;
  struct timeval  _timestamp;
  esp_err_t       res = ESP_OK;
  size_t          _jpg_buf_len = 0;
  uint8_t         *_jpg_buf = NULL;
  char            *part_buf[128];
  static int64_t  last_frame = 0;

  if (!last_frame)
    last_frame = esp_timer_get_time ();

  res = httpd_resp_set_type (req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK)
    return res;

  httpd_resp_set_hdr (req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr (req, "X-Framerate", "30");  // or 60

  while (true)
  {
    fb = esp_camera_fb_get();
    if (!fb)
    {
      Serial.println ("Camera capture failed");
      res = ESP_FAIL;
    }
    else
    {
      _timestamp.tv_sec  = fb->timestamp.tv_sec;
      _timestamp.tv_usec = fb->timestamp.tv_usec;
      if (fb->format != PIXFORMAT_JPEG)
      {
        bool jpeg_converted = frame2jpg (fb, 80, &_jpg_buf, &_jpg_buf_len);
        esp_camera_fb_return (fb);
        fb = NULL;

        if (!jpeg_converted)
        {
          Serial.println ("JPEG compression failed");
          res = ESP_FAIL;
        }
      }
      else
      {
        _jpg_buf_len = fb->len;
        _jpg_buf = fb->buf;
      }
    }

    if (res == ESP_OK)
    {
      res = httpd_resp_send_chunk (req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if (res == ESP_OK)
    {
      size_t hlen = snprintf ((char *)part_buf, 128, _STREAM_PART, _jpg_buf_len, _timestamp.tv_sec, _timestamp.tv_usec);
      res = httpd_resp_send_chunk (req, (const char *)part_buf, hlen);
    }
    if (res == ESP_OK)
    {
      res = httpd_resp_send_chunk (req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    if (fb)
    {
      esp_camera_fb_return (fb);
      fb = NULL;
      _jpg_buf = NULL;
    }
    else if (_jpg_buf)
    {
      free (_jpg_buf);
      _jpg_buf = NULL;
    }
    if (res != ESP_OK)
    {
      Serial.println ("res != ESP_OK");
      break;
    }
  }

  Serial.println ("Stream exit!");
  last_frame = 0;
  return res;
}
