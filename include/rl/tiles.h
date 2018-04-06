#ifndef _TILES_H_
#define _TILES_H_

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_NUM_VARS 20        // Maximum number of variables in a grid-tiling      
#define MAX_NUM_COORDS 100     // Maximum number of hashing coordinates      
#define MaxLONGINT 2147483647  

void tiles(
	int the_tiles[],               // provided array contains returned tiles (tile indices)
	int num_tilings,           // number of tile indices to be returned in tiles       
    int memory_size,           // total number of possible tiles
	float floats[],            // array of floating point variables
    int num_floats,            // number of floating point variables
    int ints[],				  // array of integer variables
    int num_ints);             // number of integer variables

class collision_table {
public:
    collision_table(int,int);
    ~collision_table();
    long m;
    long *data;
    int safe;
    long calls;
    long clearhits;
    long collisions;
    void reset();
    int usage();
    void print();
    void save(int);
    void restore(int);
};

	
void tiles(
	int the_tiles[],               // provided array contains returned tiles (tile indices)
	int num_tilings,           // number of tile indices to be returned in tiles       
    collision_table *ctable,   // total number of possible tiles
	float floats[],            // array of floating point variables
    int num_floats,            // number of floating point variables
    int ints[],				  // array of integer variables
    int num_ints);             // number of integer variables

int hash_UNH(int *ints, int num_ints, long m, int increment);
int hash(int *ints, int num_ints, collision_table *ctable);

// no ints
void tiles(int the_tiles[],int nt,int memory,float floats[],int nf);
void tiles(int the_tiles[],int nt,collision_table *ct,float floats[],int nf);


// one int
void tiles(int the_tiles[],int nt,int memory,float floats[],int nf,int h1);
void tiles(int the_tiles[],int nt,collision_table *ct,float floats[],int nf,int h1);

// two ints
void tiles(int the_tiles[],int nt,int memory,float floats[],int nf,int h1,int h2);
void tiles(int the_tiles[],int nt,collision_table *ct,float floats[],int nf,int h1,int h2);

// three ints
void tiles(int the_tiles[],int nt,int memory,float floats[],int nf,int h1,int h2,int h3);
void tiles(int the_tiles[],int nt,collision_table *ct,float floats[],int nf,int h1,int h2,int h3);

// one float, no ints
void tiles1(int the_tiles[],int nt,int memory,float f1);
void tiles1(int the_tiles[],int nt,collision_table *ct,float f1);

// one float, one int
void tiles1(int the_tiles[],int nt,int memory,float f1,int h1);
void tiles1(int the_tiles[],int nt,collision_table *ct,float f1,int h1);

// one float, two ints
void tiles1(int the_tiles[],int nt,int memory,float f1,int h1,int h2);
void tiles1(int the_tiles[],int nt,collision_table *ct,float f1,int h1,int h2);

// one float, three ints
void tiles1(int the_tiles[],int nt,int memory,float f1,int h1,int h2,int h3);
void tiles1(int the_tiles[],int nt,collision_table *ct,float f1,int h1,int h2,int h3);

// two floats, no ints
void tiles2(int the_tiles[],int nt,int memory,float f1,float f2);
void tiles2(int the_tiles[],int nt,collision_table *ct,float f1,float f2);

// two floats, one int
void tiles2(int the_tiles[],int nt,int memory,float f1,float f2,int h1);
void tiles2(int the_tiles[],int nt,collision_table *ct,float f1,float f2,int h1);

// two floats, two ints
void tiles2(int the_tiles[],int nt,int memory,float f1,float f2,int h1,int h2);
void tiles2(int the_tiles[],int nt,collision_table *ct,float f1,float f2,int h1,int h2);

// two floats, three ints
void tiles2(int the_tiles[],int nt,int memory,float f1,float f2,int h1,int h2,int h3);
void tiles2(int the_tiles[],int nt,collision_table *ct,float f1,float f2,int h1,int h2,int h3);

void tileswrap(
	int the_tiles[],               // provided array contains returned tiles (tile indices)
	int num_tilings,           // number of tile indices to be returned in tiles       
    int memory_size,           // total number of possible tiles
	float floats[],            // array of floating point variables
    int num_floats,            // number of floating point variables
    int wrap_widths[],         // array of widths (length and units as in floats)
    int ints[],				  // array of integer variables
    int num_ints);             // number of integer variables
	
void tileswrap(
	int the_tiles[],               // provided array contains returned tiles (tile indices)
	int num_tilings,           // number of tile indices to be returned in tiles       
    collision_table *ctable,   // total number of possible tiles
	float floats[],            // array of floating point variables
    int num_floats,            // number of floating point variables
    int wrap_widths[],         // array of widths (length and units as in floats)
    int ints[],				  // array of integer variables
    int num_ints);             // number of integer variables



#endif

