#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <thread>
#include <omp.h>

#define RGB_COMPONENT_COLOR 255
#define NUMBER_THREADS 8

static const auto THREADS = std::thread::hardware_concurrency();

struct PPMPixel {
  int red;
  int green;
  int blue;
};

typedef struct {
  int x, y, all;
  PPMPixel *data; // here we store the pixels
} PPMImage;
void readPPM(const char *filename, PPMImage &img) {
    std::ifstream file(filename);
    if (file) {
        std::string s;
        int rgb_comp_color;
        file >> s;
        if (s != "P3") {
            std::cout << "error in format" << std::endl;
            exit(9);
        }
        
        file >> img.x >> img.y >> rgb_comp_color;
        img.all = img.x * img.y;
        
        img.data = new PPMPixel[img.all];
        for (int i = 0; i < img.all; i++) {
            file >> img.data[i].red 
                 >> img.data[i].green 
                 >> img.data[i].blue;
        }
    } else {
        std::cout << "error opening file: " << filename << std::endl;
        exit(9);
    }
}
void writePPM(const char *filename, PPMImage &img) {
    std::ofstream file(filename, std::ofstream::out);
    file << "P3" << std::endl;
    file << img.x << " " << img.y << " " << std::endl;
    file << RGB_COMPONENT_COLOR << std::endl;

    for (int i = 0; i < img.all; i++) {
        file << img.data[i].red << " " << img.data[i].green << " "
             << img.data[i].blue << (((i + 1) % img.x == 0) ? "\n" : " ");
    }
    file.close();
}
void move_picture_left(PPMImage &img, int move_times) {
    PPMPixel* temp_col = new PPMPixel[img.y];
    
    #pragma omp parallel

    #pragma omp for 
    for (int m = 0; m < move_times; m++) {
        int tid = omp_get_thread_num();
        for (int r = 0; r < img.y; r++) {
            temp_col[r] = img.data[r * img.x + (img.x - 1)];
        }
        
        for (int r = 0; r < img.y; r++) {
            for (int c = img.x - 1; c > 0; c--) {
                img.data[r * img.x + c] = img.data[r * img.x + c - 1];
            }
        }
         
        for (int r = 0; r < img.y; r++) {
            img.data[r * img.x] = temp_col[r];
        }
    }
    
    delete[] temp_col;
}
int main(int argc, char *argv[]) {
    int moveeee= 100; //number pixels 
    PPMImage image;
    readPPM("car.ppm", image);
    double start_time = omp_get_wtime();
    omp_set_num_threads(NUMBER_THREADS);
    move_picture_left(image,moveeee);
    writePPM("new_car.ppm", image);
    printf("Time taken: %10.6f\n", omp_get_wtime()-start_time) ;
    return 0;
}




