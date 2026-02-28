//
// mountainplots - read a png dem/dsm and write a png slope diagram
//
// (c)2023,6 Mark J. Stock <markjstock@gmail.com>
//

#include "memory.h"
#include "inout.h"
#include "CLI11.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <cmath>

constexpr double pi() { return std::atan(1)*4; }

// finding intersection points of a point-angle combination with the box boundaries

void findIntersection(const float px, const float py, const float alpha,
                      const float nx, const float ny,
                      float& x_intersect, float& y_intersect) {

  const float alpharad = std::fmod(alpha, 360.f) * pi() / 180.0;

  // Check intersection with the right boundary
  if (alpharad < pi() / 2 || alpharad > 3 * pi() / 2) {
    x_intersect = nx;
    y_intersect = py + std::tan(alpharad) * (nx - px);
    //printf("  right bdry, testing %g %g\n", x_intersect, y_intersect);
    if (y_intersect >= 0 && y_intersect <= ny) {
      return;
    }
  } 

  // Check intersection with the left boundary
  if (alpharad > pi() / 2 && alpharad < 3 * pi() / 2) {
    x_intersect = 0;
    y_intersect = py - std::tan(alpharad) * px;
    //printf("  left bdry, testing %g %g\n", x_intersect, y_intersect);
    if (y_intersect >= 0 && y_intersect <= ny) {
      return;
    }
  }

  // Check intersection with the top boundary
  if (alpharad > 0 && alpharad < pi()) {
    y_intersect = ny;
    x_intersect = px + (ny - py) / std::tan(alpharad);
    //printf("  top bdry, testing %g %g\n", x_intersect, y_intersect);
    if (x_intersect >= 0 && x_intersect <= nx) {
      return;
    }
  }

  // Check intersection with the bottom boundary
  if (alpharad > pi() && alpharad < 2 * pi()) {
    y_intersect = 0;
    x_intersect = px - py / std::tan(alpharad);
    //printf("  bottom bdry, testing %g %g\n", x_intersect, y_intersect);
    if (x_intersect >= 0 && x_intersect <= nx) {
      return;
    }
  }

  // If no intersection is found, return the original point (this should not happen with valid inputs)
  x_intersect = px;
  y_intersect = py;
  return;
}


// begin execution here

int main(int argc, char const *argv[]) {

  std::cout << "mountainplots v0.1\n";

  // process command line args
  CLI::App app{"Generate mountain slope image from input dem/dsm"};

  // load a dem from a png file - check command line for file name
  std::string demfile = "in.png";
  app.add_option("-i,--input", demfile, "png DEM for elevations")->required();

  // set output file name and size
  std::string outfile = "out.png";
  app.add_option("-o,--output", outfile, "png profile output");
  int ox = 0;
  app.add_option("-x,--ox", ox, "force number of pixels in horizontal direction, if not given with match input dem");
  int oy = 0;
  app.add_option("-y,--oy", oy, "force number of pixels in vertical direction, if not given, will use elevs or assume 1000");

  // set elevations for black and white, dem resolution in mpp
  std::vector<float> elevs({0.f, -1.f});
  app.add_option("-e,--elevs", elevs, "elevation of black and white pixels, meters, defaults 0 1000")->expected(2);
  float mpp = -1.f;
  app.add_option("-m,--mpp", mpp, "meters per pixel in the dem, default is to assume 1.0");

  // finally parse
  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    return app.exit(e);
  }


  //
  // read a png of elevations
  //

  std::cout << "Reading elevations from file (" << demfile << ")\n";

  // check the resolution first
  int nx, ny;
  {
    int hgt, wdt;
    (void) read_png_res (demfile.c_str(), &hgt, &wdt);
    if (wdt > 0) nx = wdt;
    if (hgt > 0) ny = hgt;
  }
  std::cout << "  input dem is " << nx << " x " << ny << " pixels\n";

  // allocate the space
  float** dem = allocate_2d_array_f((int)nx, (int)ny);

  // read the first channel into the elevation array, scaled as 0..vscale
  (void) read_png (demfile.c_str(), (int)nx, (int)ny, 0, 0, 0.0, 0,
                   dem, 0.0, 1.0, nullptr, 0.0, 1.0, nullptr, 0.0, 1.0);

  // find the pixel with the maximum value (altitude)
  float maxalt = 0.f;
  int mx = 0;
  int my = 0;
  for (int ix=0; ix<nx; ix++) {
  for (int iy=0; iy<ny; iy++) {
    if (dem[ix][iy] > maxalt) {
      maxalt = dem[ix][iy];
      mx = ix;
      my = iy;
    }
  }
  }
  std::cout << "  highest point is at " << mx << " x " << my << " pixels\n";

  // find the largest horizontal distance possible
  const float maxhorizsq = std::pow( (mx > nx/2) ? (float)mx : (float)(nx-mx) ,2)
                         + std::pow( (my > ny/2) ? (float)my : (float)(ny-my) ,2);
  const float maxhoriz = std::sqrt(maxhorizsq);
  std::cout << "  max horizontal distance is " << maxhoriz << " pixels\n";

  // reset horizontal size
  if (ox == 0) {
    ox = int(maxhoriz+0.5f);
  }

  // if mpp was set, use that, otherwise assume 1
  if (mpp <= 0.f) mpp = 1.f;

  // if vertical resolution was not set, use elevs to set it
  if (oy == 0) {
    // if elevs were set
    if (elevs[1] > 0.f) {
      oy = int(elevs[1]/mpp + 0.5f);
    } else {
      // assume 0..5000
      elevs[1] = 5000.f;
      oy = int(elevs[1]/mpp + 0.5f);
    }
  } else {
    // just use given vertical resolution for black-to-white
    if (elevs[1] > 0.f) {
      // keep elevs and use them with existing oy
      mpp = (elevs[1]) / (float)oy;
    } else {
      // adjust elevs to accomodate oy
      elevs[0] = 0.f;
      elevs[1] = mpp * (float)oy;
    }
  }
  std::cout << "  output image will be " << ox << " x " << oy << " pixels\n";
  std::cout << "  elevs are " << elevs[0] << " to " << elevs[1] << " units\n";

  //
  // generate the profile image
  //

  // allocate and zero
  float** profimg = allocate_2d_array_f((int)ox, (int)oy);
  for (int ix=0; ix<ox; ix++) {
  for (int iy=0; iy<oy; iy++) {
    profimg[ix][iy] = 0.f;
  }
  }

  // apply the data to the output image
  float lastmax = 0.f;
  for (int ix=0; ix<nx; ix++) {
  for (int iy=0; iy<ny; iy++) {

    // elevation difference, normalized
    // convert pixel value (0..1) to elevation in meters
    const float elevm = elevs[0] + dem[ix][iy] * (elevs[1]-elevs[0]);
    // then meters to pixels
    const float pfy = 0.5f + elevm/mpp;
    const int py = std::min(oy-1, std::max((int)0, (int)pfy));

    // distance, normalized
    const float distsq = (float)(ix-mx)*(ix-mx) + (float)(iy-my)*(iy-my);
    const float pfx = ox * std::sqrt(distsq) / maxhoriz;
    const int px = std::min(ox-1, std::max((int)0, (int)pfx));

    //if (std::sqrt(distsq) > lastmax+1.f) {
    //  lastmax = std::sqrt(distsq);
    //  std::cout << "    elev " << elevm << " at " << ix << " " << iy << " dist " << lastmax << " given col " << px << "\n";
    //}
    
    // nearest
    //profimg[px][py] += 1.f/(std::sqrt(distsq+1.f));
    //profimg[px][py] += std::pow(distsq+1.f, -0.4f);
    //profimg[px][py] += std::pow(distsq+1.f, -0.3f);

    // bilinear
    const float toadd = std::pow(distsq+1.f, -0.3f);
    const float fracx = pfx - (float)px;
    const float fracy = pfy - (float)py;
    profimg[px][py] += toadd * (1.f-fracx) * (1.f-fracy);
    if (px+1 < ox) profimg[px+1][py] += toadd * (fracx) * (1.f-fracy);
    if (py+1 < oy) profimg[px][py+1] += toadd * (1.f-fracx) * (fracy);
    if (px+1 < ox and py+1 < oy) profimg[px+1][py+1] += toadd * (fracx) * (fracy);
  }
  }

  // free the dem
  free_2d_array_f(dem);

  // find and plot a smoothed mean line
  float* meanalt = (float*)malloc(sizeof(float)*ox);
  if (false) {
    // loop over columns
    for (int ix=0; ix<ox; ix++) {
      float zmom = 0.f;
      float fmom = 0.f;
      for (int iy=0; iy<oy; iy++) {
        zmom += profimg[ix][iy];
        fmom += iy*profimg[ix][iy];
      }
      meanalt[ix] = fmom / (zmom+1.e-5);
      //printf("x %ld  meanalt %g zmom %g fmom %g\n", ix, meanalt[ix], zmom, fmom);
    }
    // run smoothing iterations
    float* buffer = (float*)malloc(sizeof(float)*ox);
    for (int iter=0; iter<10; ++iter) {
      for (int ix=0; ix<ox; ix++) buffer[ix] = meanalt[ix];
      for (int ix=1; ix<ox-1; ix++) meanalt[ix] = 0.5f*(buffer[ix-1]+buffer[ix+1]);
    }
  }

  // invert colors and apply exponent
  for (int ix=0; ix<ox; ix++) {
  for (int iy=0; iy<oy; iy++) {
    const float val = profimg[ix][iy] * 0.6f/(mpp*mpp);
    profimg[ix][iy] = std::pow(1.f - std::min(1.f,val), 6);
  }
  }

  if (false) {
    // draw smoothed centerline (now write it in black)
    for (int ix=0; ix<ox; ix++) {
      profimg[ix][(int)meanalt[ix]] = 0.f;
    }
  }

  //
  // write the profile image
  //

  std::cout << "Writing profile to " << outfile << std::endl;
  (void) write_png (outfile.c_str(), (int)ox, (int)oy, FALSE, TRUE,
                    profimg, 0.0, 1.0, nullptr, 0.0, 1.0, nullptr, 0.0, 1.0);

  free_2d_array_f(profimg);

}
