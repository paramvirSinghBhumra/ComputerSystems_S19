#include <stdio.h>
#include "cs1300bmp.h"
#include <iostream>
#include <fstream>
#include "Filter.h"

using namespace std;

#include "rdtsc.h"

//
// Forward declare the functions
//
Filter * readFilter(string filename);
double applyFilter(Filter *filter, cs1300bmp *input, cs1300bmp *output);

int
main(int argc, char **argv)
{

  if ( argc < 2) {
    fprintf(stderr,"Usage: %s filter inputfile1 inputfile2 .... \n", argv[0]);
  }

  //
  // Convert to C++ strings to simplify manipulation
  //
  string filtername = argv[1];

  //
  // remove any ".filter" in the filtername
  //
  string filterOutputName = filtername;
  string::size_type loc = filterOutputName.find(".filter");
  if (loc != string::npos) {
    //
    // Remove the ".filter" name, which should occur on all the provided filters
    //
    filterOutputName = filtername.substr(0, loc);
  }

  Filter *filter = readFilter(filtername);

  double sum = 0.0;
  int samples = 0;

  for (int inNum = 2; inNum < argc; inNum++) {
    string inputFilename = argv[inNum];
    string outputFilename = "filtered-" + filterOutputName + "-" + inputFilename;
    struct cs1300bmp *input = new struct cs1300bmp;
    struct cs1300bmp *output = new struct cs1300bmp;
    int ok = cs1300bmp_readfile( (char *) inputFilename.c_str(), input);

    if ( ok ) {
      double sample = applyFilter(filter, input, output);
      sum += sample;
      samples++;
      cs1300bmp_writefile((char *) outputFilename.c_str(), output);
    }
    delete input;
    delete output;
  }
  fprintf(stdout, "Average cycles per sample is %f\n", sum / samples);

}

struct Filter *
readFilter(string filename)
{
  ifstream input(filename.c_str());

  if ( ! input.bad() ) {
    int size = 0;
    input >> size;
    Filter *filter = new Filter(size);
    int div;
    input >> div;
    filter -> setDivisor(div);
    for (int i=0; i < size; i++) {
      for (int j=0; j < size; j++) {
	int value;
	input >> value;
	filter -> set(i,j,value);
      }
    }
    return filter;
  } else {
    cerr << "Bad input in readFilter:" << filename << endl;
    exit(-1);
  }
}


double applyFilter(struct Filter *filter, cs1300bmp *input, cs1300bmp *output){

     int cycStart, cycStop;

  cycStart = rdtscll();
    
    output->height = input->height;
    output->width = input->width;

    int col, row, newHeight, newWidth, divisor;
    newHeight = (input->height)-1;
    newWidth = (input->width)-1;
    divisor = filter->getDivisor();
    int color1, color2, color3;
    
    int counter = 0;
    int data[9];
//     for(int c = 0; c<3; c++){
//         for(int d = 0; d<3; d++){
//             data[counter++] = filter->get(c,d);
//         }
//     }
    data[counter++] = filter->get(0,0);
    data[counter++] = filter->get(0,1);
    data[counter++] = filter->get(0,2);
    data[counter++] = filter->get(1,0);
    data[counter++] = filter->get(1,1);
    data[counter++] = filter->get(1,2);
    data[counter++] = filter->get(2,0);
    data[counter++] = filter->get(2,1);
    data[counter++] = filter->get(2,2);
    
    //pragma = calling omp parrallel stuff
    //omp = open multi processing 
    //parallel = doing operations in parallel
    //for = modifies the inside of the for loop below (specifies what thread does what automatically)
    //-fopenmp allows us to use open mp (flag tells the compiler to do something)
    
    #pragma omp parallel for
    for(row = 1; row < newHeight; row++){
        for(col = 1; col < newWidth; col++){
            color1 = (input -> color[0][row - 1][col - 1]) * data[0]
              + (input -> color[0][row - 1][col]) * data[1]
              + (input -> color[0][row - 1][col + 1]) * data[2] 
            + (input -> color[0][row][col - 1]) * data[3]
              + (input -> color[0][row][col]) * data[4]
              + (input -> color[0][row][col + 1]) * data[5] 
            + (input -> color[0][row + 1][col - 1]) * data[6]
              + (input -> color[0][row + 1][col]) * data[7]
              + (input -> color[0][row + 1][col + 1]) * data[8];

            color2 = (input -> color[1][row - 1][col - 1]) * data[0]
              + (input -> color[1][row - 1][col]) * data[1]
              + (input -> color[1][row - 1][col + 1]) * data[2]
            + (input -> color[1][row][col - 1]) * data[3]
              + (input -> color[1][row][col]) * data[4]
              + (input -> color[1][row][col + 1]) * data[5] 
            + (input -> color[1][row + 1][col - 1]) * data[6]
              + (input -> color[1][row + 1][col]) * data[7]
              + (input -> color[1][row + 1][col + 1]) * data[8];
            
            color3 = (input -> color[2][row - 1][col - 1]) * data[0]
              + (input -> color[2][row - 1][col]) * data[1]
              + (input -> color[2][row - 1][col + 1]) * data[2]
            + (input -> color[2][row][col - 1]) * data[3]
              + (input -> color[2][row][col]) * data[4]
              + (input -> color[2][row][col + 1]) * data[5]
            + (input -> color[2][row + 1][col - 1]) * data[6]
              + (input -> color[2][row + 1][col]) * data[7]
              + (input -> color[2][row + 1][col + 1]) * data[8];

            color1 /= divisor;
            color2 /= divisor;
            color3 /= divisor;
            
            if (color1 < 0 ) {
              color1 = 0;
            }
            else if (color1 > 255 ) { 
              color1 = 255;
            }
            
            if (color2 < 0 ) {
              color2 = 0;
            }
            else if (color2 > 255 ) { 
              color2 = 255;
            }
            
            if (color3 < 0 ) {
              color3 = 0;
            }
            else if (color3 > 255 ) { 
              color3 = 255;
            }
            
            output->color[0][row][col] = color1;
            output->color[1][row][col] = color2;
            output->color[2][row][col] = color3;
        }
    }
    
//   for(int col = 1; col < (input -> width) - 1; col = col + 1) {
//     for(int row = 1; row < (input -> height) - 1 ; row = row + 1) {
//       for(int plane = 0; plane < 3; plane++) {

// 	output -> color[plane][row][col] = 0;

// 	for (int j = 0; j < filter -> getSize(); j++) {
// 	  for (int i = 0; i < filter -> getSize(); i++) {	
// 	    output -> color[plane][row][col]
// 	      = output -> color[plane][row][col]
// 	      + (input -> color[plane][row + i - 1][col + j - 1] 
// 		 * filter -> get(i, j) );
// 	  }
// 	}
	
// 	output -> color[plane][row][col] = 	
// 	  output -> color[plane][row][col] / filter -> getDivisor();

// 	if ( output -> color[plane][row][col]  < 0 ) {
// 	  output -> color[plane][row][col] = 0;
// 	}

// 	if ( output -> color[plane][row][col]  > 255 ) { 
// 	  output -> color[plane][row][col] = 255;
// 	}
//       }
//     }
//   }

  cycStop = rdtscll();
  double diff = cycStop - cycStart;
  double diffPerPixel = diff / (input -> width * input -> height);
  fprintf(stderr, "Took %f cycles to process, or %f cycles per pixel\n",
	  diff, diff / (input -> width * input -> height));
  return diffPerPixel;
}

