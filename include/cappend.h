#ifndef CAPPEND_H
#define CAPPEND_H

#include <string.h>
#include <stdlib.h>

typedef struct {
	int left;
	int right;
	int bottom;
	int top;

	int w;
	int h;
} cappend_ImgInfo;

#define cappend_MAX(x, y) (((x) > (y)) ? (x) : (y))
#define cappend_MIN(x, y) (((x) < (y)) ? (x) : (y))
#include <stdio.h>
// Appends image data to another image
void cappend_append(unsigned char *dest, int dest_w, int dest_h, 
					unsigned char *src, int src_w, int src_h,
					int x_offset, int y_offset) {

	// For each horizontal row in src image
	for(int i = 0; i < src_h; i++) {
		
		int write_offset = 0;
		write_offset += i * 4 * dest_w;
		write_offset += 4 * x_offset;
		write_offset += 4 * y_offset * dest_w;

		int read_offset = 0;
		read_offset += i * 4 * src_w;

		memcpy(dest + write_offset, src + read_offset, 4 * src_w);
	}

}

// Gets a textures smallest possible size without losing quality
// Returns 1 if texture is completely transparent
int cappend_getMinimumTextureSize(unsigned char *data, int old_w, int old_h, cappend_ImgInfo *img_info) {
    int left   = old_w;
    int right  = 0;
    int bottom = 0;
    int top    = old_h;

    // For each pixel in texture
    for(int y = 0; y < old_h * 4; y += 4) {
        for(int x = 0; x < old_w * 4; x += 4) {
            int index = x + (y * old_w);
            unsigned char A = data[index+3];

            // If pixel is not transparent
            if(A != 0) {
                left   = cappend_MIN(left,   x / 4);
                right  = cappend_MAX(right,  x / 4);
                bottom = cappend_MAX(bottom, y / 4);
                top    = cappend_MIN(top,    y / 4);
            }
        }
    }

    // If texture is completely transparent
    if(right == 0 &&
        bottom == 0 &&
        left == old_w &&
        top == old_h) {

        left = right = top = bottom = 0;
        *img_info = (cappend_ImgInfo){ left, right, bottom, top, 0, 0 };
        return 1;
    }

    right++;
    bottom++;

    *img_info = (cappend_ImgInfo){ left, right, bottom, top, (right - left), (bottom - top) };

    return 0;
}

void cappend_removePixelsByExtent(unsigned char **data, int data_w, int data_h, cappend_ImgInfo *img_info) {
	unsigned char *new_data = malloc(4 * img_info->w * img_info->h);

	// For each horizontal row in image
	for(int i = img_info->top; i < img_info->bottom; i++) {
		int write_offset = 4 * (i - img_info->top) * img_info->w;

		int read_offset = 4 * i * data_w;
		read_offset += 4 * img_info->left;

		memcpy(new_data + write_offset, *data + read_offset, 4 * img_info->w);
	}

	free(*data);
	*data = new_data;
}

#endif /* CAPPEND_H */