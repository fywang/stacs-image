/**
 * Copyright (C) 2016 Felix Wang
 *
 * Simulation Tool for Asynchronous Cortical Streams (stacs)
 */

#include "image-yarp.h"

/**************************************************************************
* Main entry point
**************************************************************************/

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
  if (argc == 2) {
    fnameimages = argv[1];
  }
  else {
    printf("Usage: ./stacs-usps <images.txt>\n");
    return -1;
  }

  // Load mnist images (indexing appropriately)
  std::ifstream fileimages(fnameimages.c_str(), std::ios::in);
  std::vector<std::vector<unsigned char> > images;
  std::vector<int> labels;

  int n_rows = 16;
  int n_cols = 16;
  int image_size = 256;
  int number_of_images = 0;

  images.clear();

  if (fileimages.is_open()) {
    std::vector<unsigned char> image;
    image.resize(image_size);
    std::string line;

    // Read line by line
    while (std::getline(fileimages, line)) {
      std::istringstream iss(line);
      int label;
      float value;
      std::string token;

      // first digit is the label
      iss >> label;
      label -= 1; // usps labels 1 to 10 for digits 0 through 9
      //printf("label: %d\n", label);
      // remaining are the values
      for(int i = 0; i < image_size; ++i) {
        iss >> token;
        value = std::strtof(token.substr(token.find(':')+1, token.size()).c_str(), NULL);
        //printf("token: %s, value: %f\n", token.c_str(), value);
        // Float is from -1 to 1
        // Convert to 0 to 255
        image[i] = (unsigned char) round((value+1.0)*127.5);
      }
      images.push_back(image);
      labels.push_back(label);
    }

    // Close files
    fileimages.close();
  }

  printf("Number of images: %lu\n", images.size());
  
  // Writing order of digits
  std::ofstream uspsorder;
  uspsorder.open("uspsorder.txt");
  
  // Random ordering
  std::vector<int> perms;
  perms.resize(images.size());
  for (int i = 0; i < perms.size(); ++i) {
    perms[i] = i;
  }
  std::srand(std::time(0));
  std::random_shuffle (perms.begin(), perms.end());

  bool finish = false;

  // Stream until receive stop signal
  for (std::size_t i = 0;; ++i) {
    std::size_t ii = i % perms.size();
    if (ii == 0) {
      std::random_shuffle (perms.begin(), perms.end());
    }
    if (labels[perms[ii]] != 1 && labels[perms[ii]] != 9) {
      continue;
    }
    printf("Sending digit %d\n", labels[perms[ii]]);
      
    // Write what was played to file
    uspsorder << labels[perms[ii]] << std::endl;


    // Prepare image to send to stacs
    yarp::sig::ImageOf<yarp::sig::PixelMono> image;
    image.resize(n_cols, n_rows);
    std::memcpy(image.getRawImage(), images[perms[ii]].data(), image_size);
  
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
