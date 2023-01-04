#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <bs_mem.h>
#include <bs_core.h>
#include <bs_shaders.h>

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

    #define GLYF_END_PTS_OF_CONTOURS	 	10  /* 2 | uint16 ARRAY */
    #define GLYF_INSTRUCTION_LENGTH(contours) 	    /* 2 | uint16 	 */ \
	GLYF_END_PTS_OF_CONTOURS + contours * 2

    #define GLYF_INSTRUCTIONS		 	10  /* 1 | uint8  ARRAY */
    #define GLYF_FLAGS(offset, instructions)	    /* 1 | uint8  ARRAY */ \
	offset + instructions + 2
    /* GLYF Flags */
	#define GLYF_ON_CURVE 	0
	#define GLYF_X_SHORT 	1
	#define GLYF_Y_SHORT 	2
	#define GLYF_REPEAT 	3
	#define GLYF_X_SAME 	4
	#define GLYF_Y_SAME 	5
	#define GLYF_OVERLAP 	6

    #define GLYF_XCOORDS(offset, num_flags)         /* 1 | uint8  ARRAY */ \
	offset + num_flags
    #define GLYF_YCOORDS(offset, num_flags, xcoord) /* 1 | uint8  ARRAY */ \
	offset + num_flags + xcoord

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

    struct {
	uint16_t num_points;
	struct {
	    int16_t x;
	    int16_t y;
	} *coords;
    } points[100];
} bs_glyfInfo;

typedef struct {
    void *buf;
    uint_fast32_t offset;

    int num_subtables;
} bs_cmapInfo;

typedef struct {
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

#define BS_FLAGSET(flag, cmp) ((flag >> cmp) & 0x01)

void bs_glyf(bs_glyfInfo *glyf, int id) {
    if(glyf->buf == NULL) {
	glyf->buf = bs_findTable("glyf");
    }

    glyf->buf += bs_loca(&ttf.loca, id);

    int num_contours = bs_memU16(glyf->buf, GLYF_NUMBER_OF_CONTOURS);
    int end_pts[num_contours];

    int num_points = bs_memU16(glyf->buf, GLYF_END_PTS_OF_CONTOURS + num_contours)+1;
    for(int i = 0; i < num_contours; i++) {
	end_pts[i] = bs_memU16(glyf->buf, GLYF_END_PTS_OF_CONTOURS + i*2);
    }

    int instruction_offset = GLYF_INSTRUCTION_LENGTH(num_contours);
    int num_instructions = bs_memU16(glyf->buf, instruction_offset);

    int flag_offset = GLYF_FLAGS(instruction_offset, num_instructions);
    int flag_offset_original = flag_offset;

    /* X COORDINATES */
    int xcoord_offset = GLYF_XCOORDS(flag_offset, num_points);
    int xcoord_size = 0;

    glyf->points[id].coords = malloc(num_points * 2 * sizeof(uint16_t));
    glyf->points[id].num_points = num_points;

    int16_t xcoord_prev = 0;
    for(int i = 0; i < num_points; i++, flag_offset++) {
	int flag = bs_memU8(glyf->buf, flag_offset);
	int16_t xcoord;

	printf(
	    "XSHORT: %d | ONCURVE: %d | REPEAT: %d | XSAME: %d | OVERLAP: %d | =", 
	    BS_FLAGSET(flag, GLYF_X_SHORT), 
	    BS_FLAGSET(flag, GLYF_ON_CURVE),
	    BS_FLAGSET(flag, GLYF_REPEAT),
	    BS_FLAGSET(flag, GLYF_X_SAME),
	    BS_FLAGSET(flag, GLYF_OVERLAP)
	);

	// If xcoord is 8-bit
	if(BS_FLAGSET(flag, GLYF_X_SHORT)) {
	    xcoord = bs_memU8(glyf->buf, xcoord_offset);
	    xcoord_offset += 1;
	    xcoord_size += 1;

	    if(!BS_FLAGSET(flag, GLYF_X_SAME)) {
		xcoord = -xcoord;
	    } else {
		xcoord += xcoord_prev;
	    }
	} else {
	    if(BS_FLAGSET(flag, GLYF_X_SAME)) {
		xcoord = xcoord_prev;
	    } else {
		xcoord = bs_memU16(glyf->buf, xcoord_offset);
		xcoord += xcoord_prev;
		xcoord_offset += 2;
		xcoord_size += 2;
	    }
	}

	printf("%d\n", xcoord);
	xcoord_prev = xcoord;
	glyf->points[id].coords[i].x = xcoord;
    }

    flag_offset = flag_offset_original;

    /* Y COORDINATES */
    int ycoord_offset = GLYF_YCOORDS(flag_offset, num_points, xcoord_size);

    for(int i = 0; i < num_points; i++, flag_offset++) {
	int flag = bs_memU8(glyf->buf, flag_offset);
	int16_t ycoord;


	// If ycoord is 8-bit
	if((flag >> GLYF_Y_SHORT) & 0x01) {
	    ycoord = bs_memU8(glyf->buf, ycoord_offset);
	    ycoord_offset += 1;
	    if(!((flag >> GLYF_Y_SAME) & 0x01))
		ycoord *= -1;
	} else {
	    if((flag >> GLYF_Y_SAME) & 0x01) {
		ycoord = 0;
	    } else {
		ycoord = bs_memU16(glyf->buf, ycoord_offset);
		ycoord_offset += 2;
	    }
	}


	glyf->points[id].coords[i].y = ycoord;
    }
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

int bs_loadFont(char *path) {
    char *vs = "#version 430\n" \
	"layout (location = 0) in vec3 bs_Pos;" \

	"uniform mat4 bs_Proj; uniform mat4 bs_View;" \

	"void main() {" \
	    "gl_Position = bs_Proj * bs_View * vec4(bs_Pos, 1.0);" \
	"}";

    char *fs = "#version 430\n" \
	"out vec4 FragColor;" \
	"void main() {" \
	    "FragColor = vec4(1.0);" \
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

    // Table Gathering
    bs_head(&ttf.head);
    bs_maxp(&ttf.maxp);
    bs_hhea(&ttf.hhea);
    bs_cmap(&ttf.cmap);
    
    bs_glyf(&ttf.glyf, 36);


    bs_batch(&batch00, &shader00);
    for(int i = 0; i < ttf.glyf.points[36].num_points; i++) {
	int16_t x = ttf.glyf.points[36].coords[i].x;
	int16_t y = ttf.glyf.points[36].coords[i].y;
	bs_pushRect((bs_vec3){ (float)x / 10.0 + 650.0, (float)y / 10.0 + 150.0 }, (bs_vec2){ 4.0, 4.0 }, (bs_RGBA){ 255, 0, 0, 255 });
    }

    bs_pushBatch();

    free(buf);
    return 0;
}
