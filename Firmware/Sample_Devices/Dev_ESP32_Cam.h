//=============================================================================
//
//     FILE : ESP32_Cam.h
//
//  PROJECT : PTCamera (A Pan-n-Tilt WiFi Camera)
//
//    BOARD : GOOUUU_ESP32-S3-CAM with OV2640 Camera
//            https://github.com/profharris/GOOUUU_ESP32-S3-CAM/blob/main/README.md
//            https://github.com/espressif/arduino-esp32/tree/master/libraries/ESP32/examples/Camera/CameraWebServer
//            https://github.com/espressif/esp32-camera
//
//      CAM : OV2640, CAMERA_MODEL_ESP32S3_EYE
//   ISSUES : https://www.reddit.com/r/esp32/comments/r7kqtt/esp32cam_super_fast_frame_rate_when_putting_the/
//            ∙ Had to lift the camera's ribbon cable connector latch and re-insert the ribbon cable
//            ∙ Applying a loop of aluminum foil to the PCB antenna helped
//            ∙ Reducing the xclk_freq_hz config value to 8MHz from 20MHz was a game changer for frame rate and stability!
//
//            Frame Sizes: config.frame_size =
//              FRAMESIZE_QQVGA : ( 160 x  120)
//              FRAMESIZE_QVGA  : ( 320 x  240)
//              FRAMESIZE_CIF   : ( 400 x  296)
//              FRAMESIZE_VGA   : ( 640 x  480)
//              FRAMESIZE_SVGA  : ( 800 x  600)
//              FRAMESIZE_XGA   : (1024 x  768)
//              FRAMESIZE_SXGA  : (1280 x 1024)
//              FRAMESIZE_UXGA  : (1600 x 1067)
//
//            JPEG Quality: 0 (High Quality) ... 63 (Low Quality)
//              0 - 10 : High quality, less compression (larger files)
//             11 - 63 : Low quality, more compression (smaller files)
//
//   AUTHOR : Bill Daniels
//            Copyright 2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//            Portions Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
//=============================================================================

//--- Includes --------------------------------------------

#include "esp_camera.h"
#include "esp_http_server.h"

//--- class ESP32_Cam -------------------------------------

class ESP32_Cam
{
  protected:
    camera_config_t  config;
    httpd_handle_t   camera_httpd = NULL;

    void  startCameraServer ();

  public:
    ESP32_Cam ();
};
