#include <iostream>
#include <fstream>
#include <vector>
#include <allegro5/allegro.h>
#include "QuadTree.h"

using namespace std;

vector<unsigned char> encodeQuadTree(ALLEGRO_BITMAP* image, int threshhold) {
	vector<unsigned char> tree;
	int w = al_get_bitmap_width(image);
	int h = al_get_bitmap_height(image);
	tree.push_back((w >> 8) & 0xFF);
	tree.push_back(w & 0xFF);
	tree.push_back((h >> 8) & 0xFF);
	tree.push_back(h & 0xFF);

	al_lock_bitmap(image, al_get_bitmap_format(image), ALLEGRO_LOCK_READONLY);
	encodeSubQuadTree(tree, image, threshhold);
	al_unlock_bitmap(image);

	return tree;
}

void encodeSubQuadTree(vector<unsigned char>& tree, ALLEGRO_BITMAP* image, int threshhold) {
	if (isUniformColor(image, threshhold)) {
		ALLEGRO_COLOR avg = getAverageColor(image);
		unsigned char r, g, b;
		al_unmap_rgb(avg, &r, &g, &b);
		tree.push_back('H');
		tree.push_back(r);
		tree.push_back(g);
		tree.push_back(b);
	}
	else {
		int w = al_get_bitmap_width(image);
		int h = al_get_bitmap_height(image);
		tree.push_back('N');
		encodeSubQuadTree(tree, al_create_sub_bitmap(image, 0, 0, w / 2, h / 2), threshhold);
		encodeSubQuadTree(tree, al_create_sub_bitmap(image, w / 2, 0, w / 2, h / 2), threshhold);
		encodeSubQuadTree(tree, al_create_sub_bitmap(image, 0, h / 2, w / 2, h / 2), threshhold);
		encodeSubQuadTree(tree, al_create_sub_bitmap(image, w / 2, h / 2, w / 2, h / 2), threshhold);
	}
}

void saveEncodedImage(vector<unsigned char> image, string filename) {
	ofstream ofs(filename, ofstream::out);
	for (int i = 0; i < image.size(); i++) {
		ofs << image[i];
	}
}

ALLEGRO_BITMAP* openEncodedImage(string filename, ALLEGRO_DISPLAY* display) {
	ifstream ifs(filename, ifstream::in);
	return decodeQuadTree(ifs, display);
}

ALLEGRO_BITMAP* decodeQuadTree(ifstream& image, ALLEGRO_DISPLAY* display) {
	int w, h;
	w = (image.get() << 8) + image.get();
	h = (image.get() << 8) + image.get();

	ALLEGRO_BITMAP* bitmap = al_create_bitmap(w, h);
	al_set_target_bitmap(bitmap);
	al_lock_bitmap(bitmap, al_get_bitmap_format(bitmap), ALLEGRO_LOCK_READWRITE);
	parseQuadTree(image, bitmap, 0, 0, w, h);
	al_unlock_bitmap(bitmap);
	al_set_target_backbuffer(display);

	return bitmap;
}

void parseQuadTree(ifstream& ifs, ALLEGRO_BITMAP* bitmap, int x, int y, int w, int h) {
	if (ifs.get() == 'H') {
		unsigned char r, g, b;
		r = ifs.get();
		g = ifs.get();
		b = ifs.get();
		ALLEGRO_COLOR fillcolor = al_map_rgb(r, g, b);
		for (int j = 0; j < h; j++) {
			for (int i = 0; i < w; i++) {
				al_put_pixel(x + i, y + j, fillcolor);
			}
		}
	}
	else {
		int halfW = w / 2;
		int halfH = h / 2;
		parseQuadTree(ifs, bitmap, x, y, halfW, halfH);
		parseQuadTree(ifs, bitmap, x + halfW, y, halfW, halfH);
		parseQuadTree(ifs, bitmap, x, y + halfH, halfW, halfH);
		parseQuadTree(ifs, bitmap, x + halfW, y + halfH, halfW, halfH);
	}
}

bool isUniformColor(ALLEGRO_BITMAP* image, int threshhold) {
	int width = al_get_bitmap_width(image);
	int height = al_get_bitmap_height(image);

	if (width == 1 && height == 1) {
		return true;
	}
	else {
		int maxR, maxG, maxB, minR, minG, minB;
		maxR = maxG = maxB = 0;
		minR = minG = minB = 255;
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				ALLEGRO_COLOR pixel = al_get_pixel(image, x, y);
				unsigned char r, g, b;
				al_unmap_rgb(pixel, &r, &g, &b);
				maxR = (r > maxR) ? r : maxR;
				maxG = (g > maxG) ? g : maxG;
				maxB = (b > maxB) ? b : maxB;
				minR = (r < minR) ? r : minR;
				minG = (g < minG) ? g : minG;
				minB = (b < minB) ? b : minB;
			}
		}
		if (maxR - minR + maxG - minG + maxB - minB > threshhold) {
			return false;
		}
		else {
			return true;
		}
	}
}

ALLEGRO_COLOR getAverageColor(ALLEGRO_BITMAP* image) {
	double w = al_get_bitmap_width(image);
	double h = al_get_bitmap_height(image);
	ALLEGRO_COLOR avg;
	double avgR, avgG, avgB;
	avgR = avgG = avgB = 0;
	if (w * h <= 64) {
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				ALLEGRO_COLOR pixel = al_get_pixel(image, x, y);
				unsigned char r, g, b;
				al_unmap_rgb(pixel, &r, &g, &b);
				avgR += r;
				avgG += g;
				avgB += b;
			}
		}
		avg = al_map_rgb(avgR / (w * h), avgG / (w * h), avgB / (w * h));
	}
	else {
		ALLEGRO_COLOR col1 = getAverageColor(al_create_sub_bitmap(image, 0, 0, w / 2, h / 2));
		ALLEGRO_COLOR col2 = getAverageColor(al_create_sub_bitmap(image, w / 2, 0, w / 2, h / 2));
		ALLEGRO_COLOR col3 = getAverageColor(al_create_sub_bitmap(image, 0, h / 2, w / 2, h / 2));
		ALLEGRO_COLOR col4 = getAverageColor(al_create_sub_bitmap(image, w / 2, h / 2, w / 2, h / 2));
		unsigned char r1, r2, r3, r4, g1, g2, g3, g4, b1, b2, b3, b4;
		al_unmap_rgb(col1, &r1, &g1, &b1);
		al_unmap_rgb(col2, &r2, &g2, &b2);
		al_unmap_rgb(col3, &r3, &g3, &b3);
		al_unmap_rgb(col4, &r4, &g4, &b4);

		double avgR = ((double)r1 + r2 + r3 + r4) / 4.0;
		double avgG = ((double)g1 + g2 + g3 + g4) / 4.0;
		double avgB = ((double)b1 + b2 + b3 + b4) / 4.0;
		avg = al_map_rgb(avgR, avgG, avgB);
	}
	return avg;
}