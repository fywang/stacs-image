/**
 * Copyright (C) 2016 Felix Wang
 *
 * Simulation Tool for Asynchronous Cortical Streams (stacs)
 */

#include "image-yarp.h"

/**************************************************************************
* Main entry point
**************************************************************************/

// Code taken from:
// https://stackoverflow.com/questions/8286668/how-to-read-mnist-data-in-c
int reverseInt(int i) {
  unsigned char c1, c2, c3, c4;
  c1 = i & 255, c2 = (i >> 8) & 255, c3 = (i >> 16) & 255, c4 = (i >> 24) & 255;
  return ((int)c1 << 24) + ((int)c2 << 16) + ((int)c3 << 8) + c4;
};

// Main
//
int main(int argc, char ** argv) {
  // Open the network
  yarp::os::Network yarp;

  // Open the output port
  yarp::os::Port p;
  p.open("/image/send");
  // Open an input port
  yarp::os::BufferedPort<yarp::os::Bottle> pRequest;
  pRequest.open("/image/request");

  // Connect to yarp port
  yarp::os::Network::connect("/image/send", "/stacs/image/recv");
  yarp::os::Network::connect("/stacs/image/request", "/image/request");

  // Get the filename
  std::string fnameimages;
  std::string fnamelabels;
  if (argc == 3) {
    fnameimages = argv[1];
    fnamelabels = argv[2];
  }
  else {
    printf("Usage: ./stacs-mnist <images.txt> <labels.txt>\n");
    return -1;
  }

  // Load mnist images (indexing appropriately)
  std::ifstream fileimages(fnameimages.c_str(), std::ios::binary);
  std::ifstream filelabels(fnamelabels.c_str(), std::ios::binary);
  std::vector<std::vector<unsigned char> > images;
  std::vector<unsigned char> labels;

  int n_rows = 0;
  int n_cols = 0;
  int image_size = 0;

  if (fileimages.is_open() && filelabels.is_open()) {
		int magic_number = 0;
    int number_of_images = 0;
    int number_of_labels = 0;
    
    fileimages.read((char *)&magic_number, sizeof(magic_number));
		magic_number = reverseInt(magic_number);
	  if(magic_number != 2051) {
      printf("%s is not a valid mnist image file (%d)\n", fnameimages.c_str(), magic_number);
      return -1;
    }
    filelabels.read((char *)&magic_number, sizeof(magic_number));
		magic_number = reverseInt(magic_number);
	  if(magic_number != 2049) {
      printf("%s is not a valid mnist label file (%d)\n", fnamelabels.c_str(), magic_number);
      return -1;
    }

    // Number of images
		fileimages.read((char *)&number_of_images, sizeof(number_of_images)), number_of_images = reverseInt(number_of_images);
		filelabels.read((char *)&number_of_labels, sizeof(number_of_labels)), number_of_labels = reverseInt(number_of_labels);
    if(number_of_images != number_of_labels) {
      printf("Number of images does not match number of labels\n");
      return -1;
    }

    // Rows/columns
    fileimages.read((char *)&n_rows, sizeof(n_rows)), n_rows = reverseInt(n_rows);
    fileimages.read((char *)&n_cols, sizeof(n_cols)), n_cols = reverseInt(n_cols);
    image_size = n_rows * n_cols;

    images.resize(number_of_images);
    for(int i = 0; i < number_of_images; i++) {
      images[i].resize(image_size);
      fileimages.read((char *)images[i].data(), image_size);
    }
    labels.resize(number_of_labels);
    filelabels.read((char *)labels.data(), number_of_labels);
  }
  // Close files
  fileimages.close();
  filelabels.close();

  printf("Number of images: %lu\n", images.size());
  
  // Random ordering
  std::vector<int> perms;
  perms.resize(images.size());
  for (int i = 0; i < perms.size(); ++i) {
    perms[i] = i;
  }
  std::random_shuffle (perms.begin(), perms.end());

  bool finish = false;

  // Stream until receive stop signal
  for (std::size_t i = 0;; ++i) {
    std::size_t ii = i % perms.size();
    if (ii == 0) {
      std::random_shuffle (perms.begin(), perms.end());
    }
    printf("Sending digit %d\n", labels[perms[ii]]);

    // Prepare image to send to stacs
    yarp::sig::ImageOf<yarp::sig::PixelMono> image;
    image.resize(n_cols, n_rows);
    memcpy(image.getRawImage(), images[perms[ii]].data(), image_size);
  
    p.write(image);
      
    // Check if requested
    yarp::os::Bottle *b = pRequest.read();
    printf("Requesting images: %d entries left\n", b->get(0).asInt());
    if (b->get(0).asInt() < 0) {
      finish = true;
      break;
    }
    
    if (finish) {
      printf("Stacs no longer requesting data\n");
      break;
    }
  }
  
  // exit successfully
  return 0;
}
