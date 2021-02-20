#include "bitmap.h"

int main(int argc,char **argv) 
{
  std::string ppm_loc = "images/placeholder.ppm";
  std::string jpg_loc = "images/placeholder.jpg";

  Bitmap bitmap(ppm_loc, ".ppm");
  bitmap.save_jpg(jpg_loc);

  return 0; 
}