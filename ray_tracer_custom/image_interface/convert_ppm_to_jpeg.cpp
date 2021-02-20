#include "bitmap.h"

int main(int argc,char **argv) 
{
  std::string ppm_loc = "images/placeholder.ppm";
  std::string jpg_loc = "images/placeholder.jpg";

  Bitmap bitmap(jpg_loc, ".jpg");
  bitmap.save_ppm(ppm_loc);

  return 0; 
}