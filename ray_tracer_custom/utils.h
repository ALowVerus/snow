#pragma once

#include <fstream>
#include "xeus_image_methods.h"

namespace utils {
    int write_string_to_file(std::string target, std::string data)
    {
        std::ofstream myfile;
        myfile.open (target);
        myfile << data;
        myfile.close();
        return 0;
    }
    
    std::string read_txt_file(std::string target)
    {
        std::string line;
        std::ifstream myfile (target);
        std::vector<std::string> lines;
        if (myfile.is_open())
        {
            while (getline(myfile,line))
            {
                lines.push_back(line + "\n");
            }
            myfile.close();
            std::string s = std::accumulate(lines.begin(), lines.end(), std::string(""));
            return s;
        }
        else std::cout << "Unable to open file"; 
        return "ERROR";
    }
    
    unsigned char* read_in_ppm(std::string file_location, int *h, int *w) {
        std::ifstream input_ppm_file(file_location);
        if (!input_ppm_file.is_open()) {
          std::cout << "Unable to open file"; 
          exit(32);
        }
        std::string line;
        std::string word;
        getline(input_ppm_file, line); // Kill the P3
        getline(input_ppm_file, line); // Get w/h
        std::stringstream str_strm(line);
        getline(str_strm, word, ' ');
        *w = std::stoi(word);
        getline(str_strm, word, ' ');
        *h = std::stoi(word);
        getline(input_ppm_file, line); // Get max value
        int max_val = std::stoi(word);
        std::stringstream total_ss;
        while (getline(input_ppm_file, line)) {
          std::stringstream str_strm(line);
          while (getline(str_strm, word, ' ')) {
            total_ss << word << " ";
          }
        }
        unsigned char* ppm_array = (unsigned char *) malloc((*h) * (*w) * 3);
        for (int jy = 0; jy < *h; jy ++) {
          for (int jx = 0; jx < *w; jx ++) {
            for (int jc = 0; jc < 3; jc ++) {
              getline(total_ss, word, ' '); 
              ppm_array[(jy * (*w) + jx) * 3 + jc] = (unsigned char) std::stoi(word);
            }
          }
        }
        input_ppm_file.close();
        return ppm_array;
    }
    
    im::image display_ppm_stream(std::string s) {
        utils::write_string_to_file("images/placeholder.ppm", s);
        system("c++ -o testrun image_interface/convert_jpeg_to_ppm.cpp && ./testrun && rm ./testrun");
        im::image test_image("images/placeholder.jpg");
        return test_image;
    }
}