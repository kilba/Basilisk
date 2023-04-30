#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <bs_mem.h>
#include <bs_math.h>
#include <bs_core.h>
#include <bs_shaders.h>
#include <bs_debug.h>
#include <lodepng.h>

#include <cglm/cglm.h>

/* --------------------------- TABLE OFFSETS -------------------------- */
/* HEAD Table Offsets */
    #define HEAD_VERSION                 	0   /* 4 | fixed        */
    #define HEAD_FONT_REVISION           	4   /* 4 | fixed        */
    #define HEAD_CHECK_SUM_ADJUSTMENT    	8   /* 4 | uint32       */
    #define HEAD_MAGIC_NUMBER            	12  /* 4 | uint32       */
    #define HEAD_FLAGS                   	16  /* 2 | uint16       */
    #define HEAD_UNITS_PER_EM            	18  /* 2 | uint16       */
    #define HEAD_CREATED                 	20  /* 8 | longDateTime */
    #define HEAD_MODIFIED                	28  /* 8 | longDateTime */
    #define HEAD_X_MIN                   	36  /* 2 | fWord        */
    #define HEAD_Y_MIN                   	38  /* 2 | fWord        */
    #define HEAD_X_MAX                   	40  /* 2 | fWord        */
    #define HEAD_Y_MAX                   	42  /* 2 | fWord        */
    #define HEAD_MAC_STYLE               	44  /* 2 | uint16       */
    #define HEAD_LOWEST_REC_PPEM        	46  /* 2 | uint16       */
    #define HEAD_FONT_DIRECTION_HINT    	48  /* 2 | int16        */
    #define HEAD_INDEX_TO_LOC_FORMAT     	50  /* 2 | int16        */
    #define HEAD_GLYPH_DATA_FORMAT       	52  /* 2 | int16        */

/* MAXP Table Offsets */
    #define MAXP_VERSION			0   /* 4 | fixed	*/
    #define MAXP_NUM_GLYPHS			4   /* 2 | uint16	*/
    #define MAXP_MAX_POINTS			6   /* 2 | uint16	*/
    #define MAXP_MAX_CONTOURS			8   /* 2 | uint16	*/
    #define MAXP_MAX_COMPONENT_POINTS		10  /* 2 | uint16 	*/
    #define MAXP_MAX_COMPONENT_CONTOURS		12  /* 2 | uint16	*/
    #define MAXP_MAX_ZONES			14  /* 2 | uint16	*/
    #define MAXP_MAX_TWILIGHT_POINTS		16  /* 2 | uint16	*/
    #define MAXP_MAX_STORAGE			18  /* 2 | uint16	*/
    #define MAXP_MAX_FUNCTION_DEFS		20  /* 2 | uint16	*/
    #define MAXP_MAX_INSTRUCTION_DEFS		22  /* 2 | uint16	*/
    #define MAXP_MAX_STACK_ELEMENTS		24  /* 2 | uint16	*/
    #define MAXP_MAX_SIZE_OF_INSTRUCTIONS	26  /* 2 | uint16	*/
    #define MAXP_MAX_COMPONENT_ELEMENTS		28  /* 2 | uint16	*/
    #define MAXP_MAX_COMPONENT_DEPTH		30  /* 2 | uint16	*/

/* HHEA Table Offsets */
    #define HHEA_VERSION		 	0   /* 4 | fixed 	 */
    #define HHEA_ASCENT			 	4   /* 2 | fWord 	 */
    #define HHEA_DESCENT		 	6   /* 2 | fWord 	 */
    #define HHEA_LINE_GAP		 	8   /* 2 | fWord 	 */
    #define HHEA_ADVANCE_WIDTH_MAX	 	10  /* 2 | ufWord	 */
    #define HHEA_MIN_LEFT_SIDE_BEARING	 	12  /* 2 | fWord 	 */
    #define HHEA_MIN_RIGHT_SIDE_BEARING  	14  /* 2 | fWord	 */
    #define HHEA_X_MAX_EXTENT		 	16  /* 2 | fWord 	 */
    #define HHEA_CARET_SLOPE_RISE	 	18  /* 2 | int16	 */
    #define HHEA_CARET_SLOPE_RUN	 	20  /* 2 | int16	 */
    #define HHEA_CARET_OFFSET		 	22  /* 2 | fWord	 */
    /* 8 bytes reserved here */
    #define HHEA_METRIC_DATA_FORMAT	 	32  /* 2 | int16	 */
    #define HHEA_NUM_OF_LONG_HOR_METRICS 	34  /* 2 | uint16	 */

/* GLYF Table Offsets */
    #define GLYF_NUMBER_OF_CONTOURS	 	0   /* 2 | int16	 */
    #define GLYF_XMIN			 	2   /* 2 | fWord 	 */
    #define GLYF_YMIN			 	4   /* 2 | fWord 	 */
    #define GLYF_XMAX				6   /* 2 | fWord 	 */
    #define GLYF_YMAX			 	8   /* 2 | fWord	 */

    #define GLYF_END_PTS_OF_CONTOURS	 	10  /* 2 | uint16 ARRAY  */
    #define GLYF_INSTRUCTION_LENGTH(contours) 	    /* 2 | uint16 	 */ \
	GLYF_END_PTS_OF_CONTOURS + contours * 2

    #define GLYF_INSTRUCTIONS		 	10  /* 1 | uint8  ARRAY  */
    #define GLYF_FLAGS(offset, instructions)	    /* 1 | uint8  ARRAY  */ \
	offset + instructions + 2
    /* GLYF Flags */
	#define GLYF_ON_CURVE 	0
	#define GLYF_X_SHORT 	1
	#define GLYF_Y_SHORT 	2
	#define GLYF_REPEAT 	3
	#define GLYF_X_SAME 	4
	#define GLYF_Y_SAME 	5
	#define GLYF_OVERLAP 	6

    #define GLYF_XCOORDS(offset, num_flags)         /* 1 | uint8  ARRAY  */ \
	offset + num_flags
    #define GLYF_YCOORDS(offset, num_flags, coord) /* 1 | uint8  ARRAY   */ \
	offset + num_flags + coord

/* CMAP Table Offsets */
    #define CMAP_VERSION		 	0   /* 2 | uint16	 */
    #define CMAP_NUMBER_SUBTABLES	  	2   /* 2 | uint16	 */
    /* CMAP Subtable Fields */
	#define CMAP_SUBTABLE_FIELD_BEGIN     	4
	#define CMAP_SUBTABLE_FIELD_END       	8

	/* Field Variables */
	#define CMAP_PLATFORM_ID	      	4   /* 2 | uint16	 */
	#define CMAP_PLATFORM_SPECIFIC_ID     	6   /* 2 | uint16	 */
	#define CMAP_OFFSET		      	8   /* 4 | uint32	 */

typedef struct {
    int16_t x, y;
    bool on_curve;
} bs_glyfPt;

typedef struct {
    uint16_t num_points;
    uint16_t num_contours;

    int16_t x_min, x_max;
    int16_t y_min, y_max;

    uint16_t *contours;
    bs_glyfPt *coords;
} bs_glyph;

/* Table Structs */
typedef struct {
    void *buf;
    int units_per_em;
    int16_t index_to_loc_format;
} bs_headInfo;

typedef struct {
    void *buf;
    uint16_t num_glyphs;
} bs_maxpInfo;

typedef struct {
    void *buf;
} bs_hheaInfo;

typedef struct {
    void *buf;
} bs_locaInfo;

typedef struct {
    void *buf;

    bs_glyph glyphs[200];
} bs_glyfInfo;

typedef struct {
    void *buf;
    uint_fast32_t offset;

    int num_subtables;
} bs_cmapInfo;

typedef struct {
    int detail;

    void *buf;
    uint_fast32_t offset;
    int data_len;

    /* Metadata */
    uint_fast16_t table_count;

    /* Tables */
    bs_headInfo head;
    bs_maxpInfo maxp;
    bs_hheaInfo hhea;
    bs_locaInfo loca;
    bs_cmapInfo cmap;
    bs_glyfInfo glyf;
} bs_ttf;

bs_ttf ttf;

void *bs_findTable(char tag[4]) {
    void *table_ptr = bsearch(tag, ttf.buf+12, ttf.table_count, 16, bs_memcmpU32);
    
    uint_fast32_t offset = bs_memU32(table_ptr, 8);

    table_ptr = ttf.buf + offset;

    return table_ptr;
}

void bs_head(bs_headInfo *head) {
    head->buf = bs_findTable("head");
    if(head->buf == NULL)
        return;

    head->units_per_em = bs_memU16(head->buf, HEAD_UNITS_PER_EM);
    head->index_to_loc_format = bs_memU16(head->buf, HEAD_INDEX_TO_LOC_FORMAT);
}

void bs_maxp(bs_maxpInfo *maxp) {
    maxp->buf = bs_findTable("maxp");
    if(maxp->buf == NULL)
	return;

    maxp->num_glyphs = bs_memU16(maxp->buf, MAXP_NUM_GLYPHS);
}

void bs_hhea(bs_hheaInfo *hhea) {
    hhea->buf = bs_findTable("hhea");
    if(hhea->buf == NULL)
	return;

//    int val = bs_memU16(hhea->buf, HHEA_NUM_OF_LONG_HOR_METRICS);
}

uint32_t bs_loca(bs_locaInfo *loca, int id) {
    loca->buf = bs_findTable("loca");
    if(loca->buf == NULL)
	return 0;

    int offset_mul = (ttf.head.index_to_loc_format == 1) ? 1 : 2;
    uint32_t offset = bs_memU32(loca->buf, id * sizeof(uint32_t));

    return offset;
}

int bs_glyfCoords(bool y, uint8_t *flags, bs_glyfPt *pts, int offset, int num_points) {
    bs_glyfInfo *glyf = &ttf.glyf;
    int16_t coord_prev = 0;
    for(int i = 0; i < num_points; i++) {
	uint8_t flag = flags[i];
	int16_t coord;

	// If coord is 8-bit
	if(BS_FLAGSET(flag, (GLYF_X_SHORT + y))) {
	    coord = bs_memU8(glyf->buf, offset);
	    offset += 1;

	    if(!BS_FLAGSET(flag, (GLYF_X_SAME + y)))
		coord = -coord;

	    coord += coord_prev;
	} else {
	    if(BS_FLAGSET(flag, (GLYF_X_SAME + y))) {
		coord = coord_prev;
	    } else {
		coord = bs_memU16(glyf->buf, offset);
		coord += coord_prev;
		offset += 2;
	    }
	}

	coord_prev = coord;
	char *data = (char *)(pts + i);
	int16_t *val = (int16_t *)(data + y * sizeof(int16_t));

	*val = coord;
    }
    
    return offset;
}

void bs_glyf(bs_glyfInfo *glyf, int id) {
    if(glyf->buf == NULL)
	glyf->buf = bs_findTable("glyf");

    // Get bounding boxes
    glyf->glyphs[id].x_min = bs_memU16(glyf->buf, GLYF_XMIN);
    glyf->glyphs[id].y_min = bs_memU16(glyf->buf, GLYF_YMIN);
    glyf->glyphs[id].x_max = bs_memU16(glyf->buf, GLYF_XMAX);
    glyf->glyphs[id].y_max = bs_memU16(glyf->buf, GLYF_YMAX);

    glyf->glyphs[id].x_min = 111;
    glyf->glyphs[id].y_min = -431;
    glyf->glyphs[id].x_max = 2005;
    glyf->glyphs[id].y_max = 1493;

    // Get location of glyph in ttf buffer
    glyf->buf += bs_loca(&ttf.loca, id);

    // Parse contours
    int num_contours = bs_memU16(glyf->buf, GLYF_NUMBER_OF_CONTOURS);
    int end_pts[num_contours];
    for(int i = 0; i < num_contours; i++)
	end_pts[i] = bs_memU16(glyf->buf, GLYF_END_PTS_OF_CONTOURS + i*2);

    // Num points is equal to end of last contour
    int num_points = end_pts[num_contours - 1] + 1;

    int instruction_offset = GLYF_INSTRUCTION_LENGTH(num_contours);
    int num_instructions = bs_memU16(glyf->buf, instruction_offset);

    int flag_offset = GLYF_FLAGS(instruction_offset, num_instructions);
    int flag_offset_original = flag_offset;

    int coord_offset = GLYF_XCOORDS(flag_offset, num_points);

    glyf->glyphs[id].coords = malloc(num_points * sizeof(bs_glyfPt));
    glyf->glyphs[id].contours = malloc(num_contours * sizeof(uint16_t));
    glyf->glyphs[id].num_contours = num_contours;
    glyf->glyphs[id].num_points = num_points;

    for(int i = 0; i < num_contours; i++)
	glyf->glyphs[id].contours[i] = end_pts[i];

    int num_repeats_total = 0;
    uint8_t flags[num_points];

    // Parse flags
    for(int i = 0; i < num_points; i++) {
	flags[i] = bs_memU8(glyf->buf, flag_offset++);

	// Add repeated flags
	if(BS_FLAGSET(flags[i], GLYF_REPEAT)) {
	    int num_repeats = bs_memU8(glyf->buf, flag_offset++);
	    int flag_repeated = flags[i];

	    num_repeats_total += num_repeats - 1;

	    for(int j = 0; j < num_repeats; j++)
		flags[++i] = flag_repeated;
	}
    }
    
    coord_offset -= num_repeats_total;

    for(int i = 0; i < num_points; i++)
	glyf->glyphs[id].coords[i].on_curve = BS_FLAGSET(flags[i], GLYF_ON_CURVE);

    // Get X and Y coords
    coord_offset = bs_glyfCoords(false, flags, glyf->glyphs[id].coords, coord_offset, num_points);
    coord_offset = bs_glyfCoords(true , flags, glyf->glyphs[id].coords, coord_offset, num_points);
}

void bs_cmap(bs_cmapInfo *cmap) {
    cmap->buf = bs_findTable("cmap");
    if(cmap->buf == NULL)
	return;

    cmap->num_subtables = bs_memU16(cmap->buf, CMAP_NUMBER_SUBTABLES);
}

bs_Batch batch00;
bs_Shader shader00;
void bs_pushText() {
    bs_selectBatch(&batch00);
    bs_renderBatch(0, bs_batchSize());
}

bs_ivec2 bmp_dim = BS_IV2(64, 64);
unsigned char bmp[64 * 64];

int bs_cmpFloats(const void *a, const void *b) {
    float f_a = *(float *)a;
    float f_b = *(float *)b;

    if(f_a < f_b) return -1;
    else if(f_a > f_b) return 1;
    return 0;
}

void bs_rasterizeGlyph(bs_glyph *glyph, bs_vec2 *pts, int num_pts) {
    float scale = (float)bmp_dim.y / (float)(glyph->y_max - glyph->y_min);

    for(int i = 0; i < num_pts; i++) {
	pts[i].x = (pts[i].x - glyph->x_min) * scale;
	pts[i].y = (pts[i].y - glyph->y_min) * scale;
    }

    float intersections[256];
    int num_intersections = 0;
    for(int i = 0; i < bmp_dim.y; i++) {
	num_intersections = 0;
	float scanline = i;
	
	for(int j = 0; j < num_pts-1; j++) {
	    bs_vec2 pt0 = pts[j + 0];
	    bs_vec2 pt1 = pts[j + 1];

	    if(scanline <= glm_min(pt0.y, pt1.y)) continue;
	    if(scanline >  glm_max(pt0.y, pt1.y)) continue;

	    float dx = pt1.x - pt0.x;
	    float dy = pt1.y - pt0.y;

	    if(dy == 0) continue;

	    if(dx == 0) {
		intersections[num_intersections++] = pt0.x;
	    } else {
		intersections[num_intersections++] = (scanline - pt0.y) * (dx / dy) + pt0.x;
	    }
	}
    
	qsort(intersections, num_intersections, sizeof(float), bs_cmpFloats);
	if(num_intersections > 1) {
	    for(int m = 0; m < num_intersections; m += 2) {
		float start_intersection = intersections[m + 0];
		int   start_idx = intersections[m + 0];
		float start_covered = (start_idx + 1) - start_intersection;

		float end_intersection = intersections[m + 1];
		int   end_idx = intersections[m + 1];
		float end_covered = end_intersection - end_idx;

		if(start_idx == end_idx) {
		}

		for(int j = start_idx; j <= end_idx; j++)
		    bmp[j + i * bmp_dim.x] = 255;
	    }
	}
    }
    
    lodepng_encode_file("test.png", bmp, bmp_dim.x, bmp_dim.y, LCT_GREY, 8);
}

int bs_loadFont(char *path) {
    memset(bmp, 0, bmp_dim.x * bmp_dim.y * sizeof(char));
    char *vs = "#version 430\n" \
	"layout (location = 0) in vec3 bs_Pos;" \
	"layout (location = 1) in vec4 bs_Col;" \

	"uniform mat4 bs_Proj; uniform mat4 bs_View;" \
	"out vec4 fcol;"\

	"void main() {" \
	    "fcol = bs_Col;"\
	    "gl_Position = bs_Proj * bs_View * vec4(bs_Pos, 1.0);" \
	"}";

    char *fs = "#version 430\n" \
	"out vec4 FragColor;" \
	"in vec4 fcol;"\
	"void main() {" \
	    "FragColor = fcol;" \
	"}";

    bs_shaderMem(&shader00, vs, fs, 0);

    int err;
    void *buf = bs_fileContents(path, &ttf.data_len, &err);
    if(err != 0)
        return 1;

    ttf.buf = buf;
    ttf.glyf.buf = NULL;

    // Metadata Gathering
    ttf.table_count = bs_memU16(ttf.buf, 4);
    ttf.detail = 4;

    // Table Gathering
    bs_head(&ttf.head);
    bs_maxp(&ttf.maxp);
    bs_hhea(&ttf.hhea);
    bs_cmap(&ttf.cmap);
    
    int idx = 35;
    bs_glyf(&ttf.glyf, idx);

    bs_batch(&batch00, &shader00);

    int num_pts = ttf.glyf.glyphs[idx].num_points;
    bs_glyph *gi = ttf.glyf.glyphs + idx;

    bs_vec2 pts[512];
    int num_raster_pts = 0;
    for(int i = 0; i < gi->num_contours; i++) {
	uint16_t first = (i == 0) ? 0 : gi->contours[i - 1] + 1;
	uint16_t last = gi->contours[i] + 1;

	// Check winding order
	bs_vec2 firstPt, nextPt;
	firstPt = bs_v2(gi->coords[first + 0].x, gi->coords[first + 0].y);
	nextPt  = bs_v2(gi->coords[first + 1].x, gi->coords[first + 1].y);

	bs_v2print(bs_v2sub(firstPt, nextPt));

	for(int j = first; j < last; j++) {
	    bs_glyfPt curr = gi->coords[j];
	    bs_glyfPt curr_off, next_off;

	    curr_off = gi->coords[((j + 1) >= last) ? ((j + 1) - last + first) : (j + 1)];

	    if(!curr.on_curve)
		continue;

	    bs_vec2 curr_off_v, next_off_v, curr_v = bs_v2(curr.x, curr.y);

	    if(curr_off.on_curve) {
		pts[num_raster_pts++] = curr_v;
		continue;
	    }

	    while(!(curr_off = gi->coords[((j + 1) >= last) ? ((j + 1) - last + first) : (j + 1)]).on_curve) {
		next_off = gi->coords[((j + 2) >= last) ? ((j + 2) - last + first) : (j + 2)];
		if(next_off.on_curve) break;

		curr_off_v = bs_v2(curr_off.x, curr_off.y);
		next_off_v = bs_v2(next_off.x, next_off.y);

		bs_vec2 mid = bs_v2mid(curr_off_v, next_off_v);

		bs_vec2 elems[ttf.detail + 1];
		bs_v2QuadBez(curr_v, curr_off_v, mid, elems, ttf.detail);
		elems[ttf.detail] = mid;
		for(int i = 0; i < ttf.detail; i++)
		    pts[num_raster_pts++] = elems[i];

		curr_v = mid;
		j++;
	    }

	    curr_off = gi->coords[((j + 1) >= last) ? ((j + 1) - last + first) : (j + 1)];
	    next_off = gi->coords[((j + 2) >= last) ? ((j + 2) - last + first) : (j + 2)];
	    curr_off_v = bs_v2(curr_off.x, curr_off.y);
	    next_off_v = bs_v2(next_off.x, next_off.y);
	    double t = 0.0;
	    double incr;

	    incr = 1.0 / (double)ttf.detail;

	    for(int k = 0; k < ttf.detail; k++, t += incr) {
		bs_vec2 v;
		v.x = (1 - t) * (1 - t) * curr_v.x + 2 * (1 - t) * t * curr_off_v.x + t * t * next_off_v.x;
		v.y = (1 - t) * (1 - t) * curr_v.y + 2 * (1 - t) * t * curr_off_v.y + t * t * next_off_v.y;
		pts[num_raster_pts++] = v;
	    }
	}
	pts[num_raster_pts++] = bs_v2(gi->coords[first].x, gi->coords[first].y);
    }

    bs_RGBA rgba = BS_RGBA(bs_randRangeI(0, 255), bs_randRangeI(0, 255), bs_randRangeI(0, 255), 255);
    for(int k = 0; k < num_raster_pts; k++) {
	if(k % 2 == 0)
	    rgba = BS_RGBA(bs_randRangeI(0, 255), bs_randRangeI(0, 255), bs_randRangeI(0, 255), 255);

	rgba.a = 255;
	bs_pushRect(bs_v3(pts[k].x / 8.0, pts[k].y / 8.0, 0.0), bs_v2(4.0, 4.0), rgba);
    }
    bs_rasterizeGlyph(gi, pts, num_raster_pts);

    bs_pushBatch();

    free(buf);
    return 0;
}
