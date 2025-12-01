#include <iostream>
#include <carmen/carmen.h>

#include "../camera_hw_interface.h"

#include "9000pro.h"

Webcam9000Pro *webcam;

carmen_camera_image_t *carmen_camera_start(int argc, char **argv)
{
  char *device;
  carmen_camera_image_t *image;


  carmen_param_set_module("camera");
  carmen_param_get_string("dev", &device, NULL);

  carmen_warn("Opening camera\n");

  webcam = new Webcam9000Pro(device, "YUYV");
  webcam->initialize();
  
  image = (carmen_camera_image_t *)calloc(1, sizeof(carmen_camera_image_t));

  image->width = webcam->getImageWidth();
  image->height = webcam->getImageHeight();
  image->bytes_per_pixel = 1;

  image->image_size = image->width*image->height*image->bytes_per_pixel;
  image->is_new = 0;
  image->timestamp = 0;
  image->image = (char *)calloc(image->image_size, sizeof(char));
  carmen_test_alloc(image->image);
  memset(image->image, 0, image->image_size*sizeof(char));

//webcam->start();

  return image;
}

void carmen_camera_shutdown(void)
{
    webcam->stop_reading();
    webcam->uninitialize();
   // webcam->wait();
}

void carmen_camera_grab_image(carmen_camera_image_t *image)
{
  webcam->read_frame((unsigned char *)(image->image));
  
  image->timestamp = carmen_get_time();
  image->is_new = 1;
}


