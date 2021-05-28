#include "CImg.h"
#include <string>
#include <sys/stat.h>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>

bool IsPathExist(const std::string &s)
{
  struct stat buffer;
  return (stat (s.c_str(), &buffer) == 0);
}

using namespace cimg_library;

int width = 500;
int height = 500;
int triangle_height = 200;
float dt = 0.1;

float masses[3][2] = {{200.0, 150.0}, {400.0, 200.0}, {300,400}};
int nmasses = sizeof(masses)/sizeof(masses[0]);

unsigned char red[] = { 167, 38, 8 }, green[] = { 122, 179, 131 }, blue[] = {118, 120, 219}, color[] = {0,0,0};
unsigned char *colors[] = {red, green, blue};

enum Mass_Layout {
  TRIANGLE,
  LINE
};

void init_masses(Mass_Layout l) {
  float mid_height = height / 2.0;
  float mid_width = width / 2.0;
  float third = triangle_height / 3.0;
  float half = triangle_height / 2.0;

  switch(l) {
  case TRIANGLE: {
    masses[0][0] = mid_width;
    masses[0][1] = mid_height - (2.0 * third);
    masses[1][0] = mid_width - half;
    masses[1][1] = mid_height + third;
    masses[2][0] = mid_width + half;
    masses[2][1] = mid_height + third;    
    break;
  }
  case LINE: {
    masses[0][0] = mid_width;
    masses[0][1] = mid_height;
    masses[1][0] = mid_width - half;
    masses[1][1] = mid_height;    
    masses[2][0] = mid_width + half;
    masses[2][1] = mid_height;
    
    break;
  }

  }
}

struct Point {
  float x = 0;
  float y = 0;
  float xv = 0;
  float yv = 0;
  float xa = 0;
  float ya = 0;  

  Point() {}
  
  Point(float xpos, float ypos) {
    x = xpos;
    y = ypos;
  }
  void update();
  void reset(float xpos, float ypos) {
    x = xpos;
    y = ypos;
    xv = 0;
    yv = 0;
    xa = 0;
    ya = 0;
  }
};

void Point::update() {
  x += xv * dt;
  y += yv * dt;
  xv += xa * dt;
  yv += ya * dt;
  float xacc = 0;
  float yacc = 0;
  for (auto m : masses) {
    float dx = m[0] - x;
    float dy = m[1] - y;
    float d2 = (dx * dx) + (dy * dy);
    float f = 50.0 / (d2 * sqrt(d2 + 0.15));
    xacc += dx * f;
    yacc += dy * f;
  }
  xa = xacc;
  ya = yacc;
}

void calc_closest(Point *p) {
  int c = 0;
  float dist = INFINITY;
  for (int m = 0; m < nmasses; ++m) {
    // float d = hypotf(p->x - masses[m][0], p->y - masses[m][1]);
    float d = abs(p->x - masses[m][0]) + abs(p->y - masses[m][1]);
    if (d < dist) {
      dist = d;
      c = m;
    }
  }
  color[0] = colors[c][0];
  color[1] = colors[c][1];
  color[2] = colors[c][2];
}

void calc_weighted_closest(Point *p) {
  int c = 0;
  float rgb[3];
  float total = 0;
  float dist = INFINITY;

  for (int m = 0; m < nmasses; ++m) {
    float d = hypotf(p->x - masses[m][0], p->y - masses[m][1]);
    // printf("%f\n",d);
    if (d < dist) {
      c = m;
      dist = d;
    } else {
      total += d;
    }
    // total += d;
    rgb[m] = d;        
  }
  for (int i = 0; i < 3; ++i) {
    if (i == c) {
      color[i] = 255;
    } else {
      color[i] = (char)(255 * (total-rgb[i])/total);
    }
  }
}

CImg<unsigned char> *render_frame(Point **p, CImg<unsigned char> *img, int steps) {
  img->fill(0);
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      for (int i = 0; i < steps; ++i) {
	p[y][x].update();
      }
      calc_weighted_closest(&(p[y][x]));
      // if (x == 0 && y == 0) {
      // 	printf("color: %d %d %d\n", color[0], color[1], color[2]);
      // }
      img->draw_point(x,y,0,color);
    }
  }
  return img;
}


#define FLAG_IS(flag) (strcmp(flag, argv[i]) == 0)
#define TAKES_PARAM(flag) if(i+1 >= argc){printf("Error: " flag " flag requires an argument\n");} else {++i;}

int main(int argc, char *argv[]) {
  int iterations = 100;
  int step = 10;
  int frames = 1;
  bool save = true;
  bool timestamp_dir = false;
  std::string directory = "./";
  bool directory_set = false;
  std::string filename = "gravity-snapshot.bmp";
  for (int i = 1; i < argc; i += 1) {
    if (FLAG_IS("-t")) {
      TAKES_PARAM("-t")
      triangle_height = std::stoi(argv[i]);
    } else if (FLAG_IS("-i")) {
      TAKES_PARAM("-i")
      iterations = std::stoi(argv[i]);
    } else if (FLAG_IS("-f")) {
      TAKES_PARAM("-f")
      frames = std::stoi(argv[i]);
    } else if (FLAG_IS("-s")) {
      TAKES_PARAM("-s")
      step = std::stoi(argv[i]);
    } else if (FLAG_IS("-w")) {
      TAKES_PARAM("-w")
      width = std::stoi(argv[i]);
    } else if (FLAG_IS("-h")) {
      TAKES_PARAM("-h")
      height = std::stoi(argv[i]);
    } else if (FLAG_IS("-dt")) {
      TAKES_PARAM("-dt")
      dt = std::stof(argv[i]);
    } else if (FLAG_IS("-ns")) {
      save = false;
    } else if(FLAG_IS("-g")) {
      timestamp_dir = true;
    } else if (FLAG_IS("-o")) {
      TAKES_PARAM("-o")
      directory_set = true;
      directory = argv[i];
    } else if (FLAG_IS("-help") || FLAG_IS("--help")) {
      printf("Gravity Snapshot options:\n"
	     "   -f [int]        the number of frames to render\n"
	     "   -i [int]        the initial number of iterations per frame\n"
	     "   -s [int]        the step size, how much the number of iterations increases per frame\n"
	     "   -w,-h [int]     the width/height of the image\n"
	     "   -ns             don't save the frames\n"
	     "   -o [directory]  save directory\n"
	     "   -g              generate directory from timestamp\n"
	     "   -b [filename]   basefile name\n"
	     "   -t [int]        the height of the the equilateral triangle layout for the attractor masses\n"
	     "   -dt [float]     the time step between frames\n"
	     "   -help, --help      show this help info\n"
	     );
      exit(1);
    }
    else {
      printf("Unrecognized argument %s. Exiting.\n", argv[i]);
      exit(1);
    }
  }
  
  // Setup save directory string
  if (directory[directory.size()-1] != '/') {
    directory += "/";
  }
  if (directory_set) {
    struct stat dirstat;
    if (stat(directory.c_str(), &dirstat) != 0) {
      printf("Error: save directory `%s` does not exist\n", directory.c_str());
      exit(1);
    } else if (!S_ISDIR(dirstat.st_mode)) {
      printf("Error: `%s` is not a directory\n", directory.c_str());
      exit(1);
    }
  }  
  if (timestamp_dir && save) {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H-%M-%S/");
    directory += oss.str();
    mkdir(directory.c_str(), 0777);
  }  
  std::string fullname = directory + filename;
  const std::string::size_type size = fullname.size();
  char *savename = new char[size + 1];
  memcpy(savename, fullname.c_str(), size + 1);  
  
  printf("Triangle height: %d\n"
	 "Frames: %d\n"
	 "Base iterations per frame: %d\n"
	 "Iteration increase step per frame: %d\n"
	 "dt: %f\n",
	 triangle_height, frames, iterations, step, dt);
  if (!save) {
    printf("Not Saving\n");
  }
  if (!save && directory_set) {
    printf("\033[31mWARNING: save directory is set, but so is the no-save flag.\n"
	   "         Output will not be saved!\n\033[39m");
  }

  init_masses(LINE);
  CImg<unsigned char> visu(width,height,1,3,0);
  CImgDisplay main_disp(visu,"Gravity Snapshot");
  
  auto s = std::to_string((width * height * sizeof(Point)));
  int n = s.length() - 3;
  while (n > 0) {
    s.insert(n, ",");
    n -= 3;
  }
  printf("Total size required: %s bytes\n", s.c_str());
  
  int num_digits = (int)(std::floor(std::log10(frames))) + 1;

  const size_t w = width;  // with the power of c++, everything that should be built in  
  const size_t h = height; // instead gets to be another few lines or a library
  Point **p = new Point*[h];
  for(size_t i = 0; i < h; ++i) {
    p[i] = new Point[w];
  }

  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      p[i][j].reset(j, i);
    }
  }

  render_frame(p, &visu, iterations);
  
  for (int i = 0; i < frames; i += 1) {
    if (main_disp.is_closed()) {
      printf("Window Closed\n");
      exit(1);
    }
    render_frame(p, &visu, step);
    visu.display(main_disp);
    if (save) {
      visu.save(savename, i, num_digits);
    }
  }
  printf("Frame Rendering Complete\n");

  while (!main_disp.is_closed()) {
    main_disp.wait();
  }
  return 0;
}
