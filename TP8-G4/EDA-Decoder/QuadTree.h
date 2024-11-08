#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <allegro5/allegro.h>

using namespace std;

vector<unsigned char> encodeQuadTree(ALLEGRO_BITMAP* image, int threshhold);
void encodeSubQuadTree(vector<unsigned char>& tree, ALLEGRO_BITMAP* image, int threshhold);
void saveEncodedImage(vector<unsigned char> image, string filename);
ALLEGRO_BITMAP* openEncodedImage(string filename, ALLEGRO_DISPLAY* display);
ALLEGRO_BITMAP* decodeQuadTree(std::ifstream& image, ALLEGRO_DISPLAY* display);
void parseQuadTree(ifstream& ifs, ALLEGRO_BITMAP* bitmap, int x, int y, int w, int h);
bool isUniformColor(ALLEGRO_BITMAP* image, int threshhold);
ALLEGRO_COLOR getAverageColor(ALLEGRO_BITMAP* image);