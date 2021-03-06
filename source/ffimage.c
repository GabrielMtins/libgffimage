#include "ffimage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
    Copyright (C) 2020  Gabriel Martins

    This file is part of gffimage.

    gffimage is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    gffimage is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with gffimage.  If not, see <https://www.gnu.org/licenses/>
    Contact: gabrielmartinsthe@gmail.com
*/

ffimage* ffimage_create(int width, int height){
	ffimage* self = malloc(sizeof(ffimage));
	self->width = width;
	self->height = height;
	self->buffer = malloc(width*height*4*2);
	return self;
}

ffimage* ffimage_load(const char* filename){
	FILE* file = fopen(filename, "rb");
	if(file == NULL) return NULL;

	char farbfeld[9] = "";
	fread(farbfeld, 8, 1, file);
	farbfeld[8] = '\0';

	if(strcmp(farbfeld, "farbfeld")){
		fclose(file);
		return NULL; // check magic number
	}

	ffimage* self = malloc(sizeof(ffimage));

	uint8_t width[4];
	uint8_t height[4];

	fread(width, 1, 4, file);
	self->width = (width[0]<<24) + (width[1]<<16) + (width[2]<<8) + (width[3]);

	fread(height, 1, 4, file);
	self->height = (height[0]<<24) + (height[1]<<16) + (height[2]<<8) + (height[3]);

	uint32_t buffer_size = self->width*self->height*4*2;

	self->buffer = malloc(buffer_size);
	fread(self->buffer, 1, buffer_size, file);

	return self;
}

void ffimage_clear(ffimage* self, ffcolor c){
	if(self == NULL) return;
	for(int i = 0; i < self->width; i++){
		for(int j = 0; j < self->height; j++){
			ffimage_drawPixel(self, i, j, c);
		}
	}
}

ffcolor ffimage_getPixel(ffimage* self, int i, int j){
	if(self == NULL || i < 0 || j < 0 || i >= self->width || j >= self->height){
		ffcolor c = {0, 0, 0, 255};
		return c;
	}

	ffcolor c = {
		self->buffer[8*(i+j*self->width)+0],
		self->buffer[8*(i+j*self->width)+2],
		self->buffer[8*(i+j*self->width)+4],
		self->buffer[8*(i+j*self->width)+6]
	};
	return c;
}

void ffimage_drawPixel(ffimage* self, int i, int j, ffcolor c){
	if(self == NULL) return;
	if(i < 0 || j < 0 || i >= self->width || j >= self->height) return;

	self->buffer[8*(i+j*self->width)+7] = 0;
	self->buffer[8*(i+j*self->width)+6] = c.a;
	self->buffer[8*(i+j*self->width)+5] = 0;
	self->buffer[8*(i+j*self->width)+4] = c.b;
	self->buffer[8*(i+j*self->width)+3] = 0;
	self->buffer[8*(i+j*self->width)+2] = c.g;
	self->buffer[8*(i+j*self->width)+1] = 0;
	self->buffer[8*(i+j*self->width)+0] = c.r;
}

void ffimage_drawLine(ffimage* self, int x1, int y1, int x2, int y2, int thickness, ffcolor c){
	if(self == NULL) return;
	double dx = (x2-x1);
	double dy = (y2-y1);
	double steps = 0;

	if(fabs(dx) > fabs(dy)) steps = fabs(dx);
	else steps = fabs(dy);

	double start_x = x1, start_y = y1;

	for(int i = 0; i < steps; i++){
		start_x+=(double)dx/steps;
		start_y+=(double)dy/steps;
		if(start_x < 0 || start_y < 0 || start_x > self->width || start_y > self->height) return;
		ffimage_drawRect(self, start_x, start_y, thickness, thickness, c);
	}
}

void ffimage_drawRect(ffimage* self, int x, int y, int w, int h, ffcolor c){
	if(self == NULL) return;
	for(int i = 0; i < w; i++){
		for(int j = 0; j < h; j++){
			ffimage_drawPixel(self, x+i, y+j, c);
		}
	}
}

void ffimage_saveAs(ffimage* self, const char* filename){
	if(self == NULL) return;
	FILE* file = fopen(filename, "wb");
	if(file == NULL){
		printf("failed to open file\n");
		exit(-1);
	}
	uint8_t width[] = {
		255 & (self->width>>24),
		255 & (self->width>>16),
		255 & (self->width>>8),
		255 & self->width,
	};
	uint8_t height[] = {
		255 & (self->height>>24),
		255 & (self->height>>16),
		255 & (self->height>>8),
		255 & self->height,
	};
	fwrite("farbfeld", 8, 1, file);
	fwrite(width, 4, 1, file);
	fwrite(height, 4, 1, file);
	fwrite(self->buffer, 2*4*self->width*self->height, 1, file);
	fclose(file);
}

void ffimage_saveToOutput(ffimage* self){
	if(self == NULL) return;
	FILE* file = stdout;
	uint8_t width[] = {
		255 & (self->width>>24),
		255 & (self->width>>16),
		255 & (self->width>>8),
		255 & self->width,
	};
	uint8_t height[] = {
		255 & (self->height>>24),
		255 & (self->height>>16),
		255 & (self->height>>8),
		255 & self->height,
	};
	fwrite("farbfeld", 8, 1, file);
	fwrite(width, 4, 1, file);
	fwrite(height, 4, 1, file);
	fwrite(self->buffer, 2*4*self->width*self->height, 1, file);
}

void ffimage_destroy(ffimage* self){
	if(self == NULL) return;
	free(self->buffer);
	free(self);
}
