#include "CImg.h"
#include <random>
#include <string>
#include <sys/stat.h>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>

bool IsPathExist(const std::string &s) {
  struct stat buffer;
  return (stat (s.c_str(), &buffer) == 0);
}

using namespace cimg_library;

int width = 500;
int height = 500;
float gravity = 30.0;
int triangle_height = 200;
float dt = 0.1;
int num_rand = 3;

struct Mass {
  float x = 0;
  float y = 0;
  Mass(float ix, float iy) {
    x = ix;
    y = iy;
  }
};

std::vector<Mass> masses;
int nmasses = 3;

unsigned char red[] = { 167, 38, 8 }, green[] = { 122, 179, 131 }, blue[] = {118, 120, 219}, color[] = {0,0,0};
unsigned char *colors[] = {red, green, blue};

enum Mass_Layout {
  TRIANGLE,
  LINE,
  RANDOM,
  NRANDOM
};

void init_masses(Mass_Layout l) {
  float mid_height = height / 2.0;
  float mid_width = width / 2.0;
  float third = triangle_height / 3.0;
  float half = triangle_height / 2.0;

  switch(l) {
  case TRIANGLE: {
    masses.push_back(Mass(mid_width, mid_height - (2.0 * third)));
    masses.push_back(Mass(mid_width - half, mid_height + third));
    masses.push_back(Mass(mid_width + half, mid_height + third));
    // masses[0][0] = mid_width;
    // masses[0][1] = mid_height - (2.0 * third);
    // masses[1][0] = mid_width - half;
    // masses[1][1] = mid_height + third;
    // masses[2][0] = mid_width + half;
    // masses[2][1] = mid_height + third;    
    break;
  }
  case LINE: {
    masses.push_back(Mass(mid_width, mid_height));
    masses.push_back(Mass(mid_width - half, mid_height));
    masses.push_back(Mass(mid_width + half, mid_height));
    // masses[0][0] = mid_width;
    // masses[0][1] = mid_height;
    // masses[1][0] = mid_width - half;
    // masses[1][1] = mid_height;    
    // masses[2][0] = mid_width + half;
    // masses[2][1] = mid_height;
    
    break;
  }
  case RANDOM:
  case NRANDOM: {
    nmasses = num_rand;
    std::random_device g;
    std::uniform_int_distribution<int> rw(0,width-1);
    std::uniform_int_distribution<int> rh(0,height-1);

    for (int i = 0; i < num_rand; ++i) {
      masses.push_back(Mass(rw(g), rh(g)));
    }
    // masses[0][0] = rw(g);
    // masses[1][0] = rw(g);
    // masses[2][0] = rw(g);
    
    // masses[0][1] = rh(g);
    // masses[1][1] = rh(g);
    // masses[2][1] = rh(g);

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

void print_help() {
  printf("Gravity Snapshot options:\n"
	 "   -size [int w] [int h]     the width and height of the frames\n"
	 "   -frames [int]        the number of frames to render, default is 1\n"
	 "                        if followed by \"inf\" the program will contnue indefinitely\n"
	 "   -shape [type]        can be triangle, line, or random\n"
	 "   -shape-size [int]     height for triangle, width for line\n"
	 "   -dt [float]          the time step between frames\n"
	 "   -gravity [float]     the force of gravity\n"
	 "   -i [int]             the initial number of iterations per frame, default is 100\n"
	 "   -step [int]          by how much the number of iterations increases per frame, default is 10\n"
	 "\n   -ns                No save. Don't save the frames\n"
	 "   -name [filename]     basefile name\n"
	 "   -save-in [directory]       save directory\n"
	 "   -g                   generate directory from timestamp\n"
	 "                        when the -o flag is also present the generated directory\n"
	 "                        will be a child of the given directory\n\n"
	 "   -interactive         show the gravity simulation\n"
	 "\n   -help, --help        show this help info\n"
    );
  exit(1);
}

void Point::update() {
  x += xv * dt;
  y += yv * dt;
  xv += xa * dt;
  yv += ya * dt;
  float xacc = 0;
  float yacc = 0;
  for (auto m : masses) {
    float dx = m.x - x;
    float dy = m.y - y;
    float d = (dx * dx) + (dy * dy);
    float f = gravity / (d + .1);
    xacc += dx * f;
    yacc += dy * f;
  }
  xa = xacc;
  ya = yacc;
}

void interactive_mode() {  
  CImg<unsigned char> visu(width,height,1,3,0);
  CImgDisplay disp(visu,"Gravity Snapshot");
  color[0] = 255;   color[1] = 255;   color[2] = 255;
  visu.fill(0);

  int i = 0;
  for (auto m : masses) {
    visu.draw_circle(m.x, m.y, 15, colors[i % 3]);
    ++i;
  }
  
  visu.display(disp);

  Point p = Point();
  bool mouse_pressed = false;
  bool begin = false;
  while (!begin) {
    if (disp.button()&1) {
      begin = true;
      p.reset(disp.mouse_x(), disp.mouse_y());
    }
  }
  
  while (true) {
    visu.fill(0);
    if (disp.is_closed()) {
      printf("Window Closed\n");
      exit(1);
    }    
    if(!mouse_pressed && disp.button()&1) {
      mouse_pressed = true;
      p.reset(disp.mouse_x(), disp.mouse_y());
    } else if (mouse_pressed && !disp.button()&1) {
      mouse_pressed = false;
    }

    i = 0;
    for (auto m : masses) {
      visu.draw_circle(m.x, m.y, 15, colors[i % 3]);
      ++i;
    }

    p.update();
    visu.draw_circle(p.x, p.y, 5, color);
    visu.display(disp);
  }
}

void calc_closest(Point *p) {
  int c = 0;
  float dist = INFINITY;
  // for (int m = 0; m < nmasses; ++m) {
  int i = 0;
  for (auto m : masses) {
    // float d = hypotf(p->x - masses[m][0], p->y - masses[m][1]);
    float d = abs(p->x - m.x) + abs(p->y - m.y);
    if (d < dist) {
      dist = d;
      c = i % 3;
    }
    ++i;
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

  // for (int m = 0; m < nmasses; ++m) {
  for (int i = 0; i < nmasses; ++i) {
    auto m = masses[i];
    float d = hypotf(p->x - m.x, p->y - m.y);
    // printf("%f\n",d);
    if (d < dist) {
      c = i % 3;
      dist = d;
    } else {
      total += d;
    }
    // total += d;
    rgb[i % 3] = d;
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
#define TAKES_PARAMS(flag, num) if(i+num >= argc){printf("Error: " flag " flag requires an argument\n");} else {i += num;}

int main(int argc, char *argv[]) {
  Mass_Layout shape = TRIANGLE;
  int iterations = 0;
  int step = 1;
  int frames = 1;
  bool save = true;
  bool timestamp_dir = false;
  std::string directory = "./";
  bool directory_set = false;
  std::string filename = "gravity-snapshot.png";
  bool interactive = false;
  bool verbose = false;
  for (int i = 1; i < argc; i += 1) {
    if (FLAG_IS("-shape-size")) {
      TAKES_PARAM("-shape-size")
      triangle_height = std::stoi(argv[i]);
    } else if (FLAG_IS("-shape")) {
      TAKES_PARAM("-shape")
	if (strcmp(argv[i],"line") == 0) {
	  shape = LINE;
	} else if (strcmp(argv[i],"random") == 0) {
	  shape = RANDOM;
	} else if (strcmp(argv[i], "nrandom") == 0) {
	  TAKES_PARAM("nrandom")
	  shape = NRANDOM;
	  num_rand = std::stoi(argv[i]);
	}
    } else if (FLAG_IS("-i")) {
      TAKES_PARAM("-i")
      iterations = std::stoi(argv[i]);
    } else if (FLAG_IS("-frames")) {
      TAKES_PARAM("-frames")
      if (strcmp(argv[i], "inf") == 0) {
	frames = 0;
      } else {
	frames = std::stoi(argv[i]);
      }
    } else if (FLAG_IS("-step")) {
      TAKES_PARAM("-step")
      step = std::stoi(argv[i]);
    } else if (FLAG_IS("-size")) {
      TAKES_PARAMS("-size", 2)
      width = std::stoi(argv[i-1]);
      height = std::stoi(argv[i]);      
    } else if (FLAG_IS("-dt")) {
      TAKES_PARAM("-dt")
      dt = std::stof(argv[i]);
    } else if (FLAG_IS("-gravity")) {
      TAKES_PARAM("-gravity")
      gravity = std::stof(argv[i]);
    } else if (FLAG_IS("-ns")) {
      save = false;
    } else if(FLAG_IS("-g")) {
      timestamp_dir = true;
    } else if (FLAG_IS("-save-in")) {
      TAKES_PARAM("-save-in")
      directory_set = true;
      directory = argv[i];
    } else if (FLAG_IS("-name")) {
      TAKES_PARAM("-name")
      filename = argv[i];
    } else if (FLAG_IS("-interactive")) {
      interactive = true;
    } else if (FLAG_IS("-v")) {
      verbose = true;
    }

    else if (FLAG_IS("-help") || FLAG_IS("--help")) {
      print_help();
    }
    else {
      printf("Unrecognized argument %s\n\n", argv[i]);
      print_help();
    }
  }
  
  init_masses(shape);
  if (interactive) {
    interactive_mode();
    return 0;
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
    oss << std::put_time(&tm, "%Y-%m-%d_%H:%M:%S/");
    directory += oss.str();
    mkdir(directory.c_str(), 0777);
  }  
  std::string fullname = directory + filename;
  const std::string::size_type size = fullname.size();
  char *savename = new char[size + 1];
  memcpy(savename, fullname.c_str(), size + 1);  

  if (verbose) {
    printf("Shape size: %d\n"
	   "Frames: %d\n"
	   "Base iterations per frame: %d\n"
	   "Steps per frame: %d\n"
	   "dt: %f\n",
	   triangle_height, frames, iterations, step, dt);
  }
  if (!save) {
    printf("Not Saving\n");
  }  
  if (!save && directory_set) {
    printf("\033[31mWARNING: save directory is set, but so is the no-save flag.\n"
	   "         Output will not be saved!\n\033[39m\n");
  }

  init_masses(shape);
  CImg<unsigned char> visu(width,height,1,3,0);
  CImgDisplay main_disp(visu,"Gravity Snapshot");
  visu.fill(0);

  auto s = std::to_string((width * height * sizeof(Point)));
  int n = s.length() - 3;
  while (n > 0) {
    s.insert(n, ",");
    n -= 3;
  }
  // printf("Total size required: %s bytes\n", s.c_str());
  
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

  if (frames == 0) {
    int i = 0;
    while (true) {
      if (main_disp.is_closed()) {
	printf("Window Closed\n");
	exit(1);
      }
      render_frame(p, &visu, step);
      visu.display(main_disp);
      if (save) {
	visu.save(savename, i);
      }
      ++i;
    }
  }
  
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
