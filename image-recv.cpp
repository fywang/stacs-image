/**
 * Copyright (C) 2016 Felix Wang
 *
 * Simulation Tool for Asynchronous Cortical Streams (stacs)
 */

#include "image-yarp.h"

/**************************************************************************
* Main entry point
**************************************************************************/

class ImagePort : public yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelMono> > {
  public:
    ImagePort();
    //using yarp::os::BufferedPort<yarp::sig::Image>::onRead;
    virtual void onRead (yarp::sig::ImageOf<yarp::sig::PixelMono>& img) {
      // Receive image and display
      printf("Image received\n");
      for (std::size_t i = 0; i < img.height(); ++i) {
        for (std::size_t j = 0; j < img.width(); ++j) {
          printf("%3d ", img.getRawImage()[i*img.height() + j]);
        }
        printf("\n");
      }
    }
  private:
    yarp::os::Property conf;
};

ImagePort::ImagePort() {
  // Get an image write device
  printf("Streaming to yarpview\n");
}

// Main
//
int main(int argc, char ** argv) {
  // Open the network
  yarp::os::Network yarp;

  // Open the input port
  ImagePort p;
  p.useCallback();
  p.open("/image/recv");

  // Connect to yarp port
  yarp::os::Network::connect("/image/send", "/image/recv");

  while(true);

  // exit successfully
  return 0;
}
