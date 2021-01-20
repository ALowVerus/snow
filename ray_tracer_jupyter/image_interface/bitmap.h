#include <iostream>
#include <fstream>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

class Bitmap {
  public:
    int h, w;
    unsigned char *data;
    Bitmap(int h, int w, unsigned char *data) : h(h), w(w), data(data) { }
    Bitmap(std::string file_location, std::string file_extension) { 
      if (file_extension.compare(".jpg") == 0) {
        int n;
        data = stbi_load(file_location.c_str(), &w, &h, &n, 0);
      } else if (file_extension.compare(".ppm") == 0) {
        std::string line;
        std::string word;
        std::ifstream input_ppm_file(file_location);
        if (!input_ppm_file.is_open()) {
          std::cout << "Unaeble to open file"; 
          exit(32);
        }
        getline(input_ppm_file, line); // Kill the P3
        getline(input_ppm_file, line); // Get w/h
        std::stringstream str_strm(line);
        getline(str_strm, word, ' ');
        w = std::stoi(word);
        getline(str_strm, word, ' ');
        h = std::stoi(word);
        getline(input_ppm_file, line); // Get max value
        int max_val = std::stoi(word);
        std::stringstream total_ss;
        while (getline(input_ppm_file, line)) {
          std::stringstream str_strm(line);
          while (getline(str_strm, word, ' ')) {
            total_ss << word << " ";
          }
        }
        unsigned char *ppm_array = (unsigned char *) malloc(h * w * 3);
        for (int jy = 0; jy < h; jy ++) {
          for (int jx = 0; jx < w; jx ++) {
            for (int jc = 0; jc < 3; jc ++) {
              getline(total_ss, word, ' '); 
              ppm_array[(jy * w + jx) * 3 + jc] = (unsigned char) std::stoi(word);
            }
          }
        }
        input_ppm_file.close();
        data = ppm_array;
      } else {
        exit(32);
      }
    }

    int get_pixel(int y, int x, char p) {
      return (int)(data[(y * w + x) * 3 + p]);
    }
    void set_pixel(int y, int x, char p, int val) {
      data[(y * w + x) * 3 + p] = (char) val;
    }

    void save_jpg(std::string file_location) {
      int success_1 = stbi_write_jpg(file_location.c_str(), w, h, 3, data, 100);
    }
    void save_ppm(std::string file_location) {
      std::ofstream output_ppm_file;
      output_ppm_file.open(file_location.c_str());
      output_ppm_file 
        << "P3\n" 
        << w << " " << h << "\n"
        << 256 << "\n";
      for (int iy = 0; iy < h; iy ++) {
        for (int ix = 0; ix < w; ix ++) {
          int k = ((iy * w + ix)) * 3;
          output_ppm_file 
            << (int) data[k + 0] << " "
            << (int) data[k + 1] << " "
            << (int) data[k + 2] << " ";
        }
        output_ppm_file << "\n";
      }
      output_ppm_file.close();
    }
};