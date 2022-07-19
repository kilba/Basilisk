#include <cappend.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct {
	int w, h;

	int x, y;
	float tex_x, tex_y;

	int id;
} rectpacker_Rect;

int cmpHeight(const void *s1, const void *s2) {
    rectpacker_Rect *r1 = (rectpacker_Rect *)s1;
    rectpacker_Rect *r2 = (rectpacker_Rect *)s2;
    return r2->h - r1->h;
}

int cmpId(const void *s1, const void *s2) {
    rectpacker_Rect *r1 = (rectpacker_Rect *)s1;
    rectpacker_Rect *r2 = (rectpacker_Rect *)s2;
    return r1->id - r2->id;
}

void rectpacker_packRect(rectpacker_Rect *rects, int rect_count, int atlas_width, int atlas_height) {
	// Setting default values
	for(int i = 0; i < rect_count; i++) {
		rects[i].x = 0;
		rects[i].y = 0;
		rects[i].tex_x = 0.0;
		rects[i].tex_y = 0.0;
		rects[i].id = i;
	}

	// Sort by a heuristic
	qsort(rects, rect_count, sizeof(rectpacker_Rect), cmpHeight);

	int x_pos = 0;
	int y_pos = 0;

	int largest_h_this_row = rects[0].h;
	for(int i = 0; i < rect_count; i++) {
		if(rects[i].w == 0 || rects[i].h == 0)
			continue;

		// If outside range of current row
		if((x_pos + rects[i].w) > atlas_width) {
			y_pos += largest_h_this_row;
			x_pos = 0;

			largest_h_this_row = rects[i].h;
		}

		// If outside the range of the atlas
		if((y_pos + rects[i].h) > atlas_height)
			break;


		rects[i].x = x_pos;
		rects[i].y = y_pos;
		rects[i].tex_x = x_pos / (float)atlas_width;
		rects[i].tex_y = y_pos / (float)atlas_height;

		// Optimization test
		// if(i != 0) {
		// 	if((rects[i].h * 2) < rects[i-1].h) {
		// 	}
		// }

		x_pos += rects[i].w;
	}
	
	qsort(rects, rect_count, sizeof(rectpacker_Rect), cmpId);
}