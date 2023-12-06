#include <stdio.h>
#include <stdlib.h>
// argv[1]을 받아오기 위함
#include <string.h>
#include <time.h>

#define PNG_SETJMP_NOT_SUPPORTED
#include <png.h>

#define WIDTH 32
#define HEIGHT 32
#define COLOR_DEPTH 8

struct Pixel {
	png_byte r, g, b, a;
};

png_bytep *input_ptr = NULL;	// input png의 픽셀 정보를 담을 포인터 변수

// input png의 픽셀 정보 불러오기
void read_input_png(const char *filename) {
	FILE *fp = fopen(filename, "rb");

	// png 파일에서 필요한 정보 읽고 저장하기
	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png) abort();

	png_infop info = png_create_info_struct(png);
	if(!info) abort();

	// png가 올바르지 않은 경우 중단
	if(setjmp(png_jmpbuf(png))) abort();

	// png 이미지의 정보(info)를 읽어옴
	png_init_io(png, fp);
	png_read_info(png, info);
	png_read_update_info(png, info);

	if (input_ptr) abort();

	// 읽어온 info를 토대로 2차원 동적 할당
	input_ptr = (png_bytep*)malloc(sizeof(png_bytep) * HEIGHT);
	for(int y = 0; y < HEIGHT; y++) {
		input_ptr[y] = (png_byte*)malloc(png_get_rowbytes(png,info));
	}

	png_read_image(png, input_ptr);

	fclose(fp);

	// 22, 25번째 줄에서 읽어온 정보를 저장한 메모리를 해제(free) 함
	png_destroy_read_struct(&png, &info, NULL);
}

void reverse(Pixel *row_pointers[], char* option) {
	// 1. 그대로 출력
	if(!strcmp(option, "1")) {
		for(int col=0; col<WIDTH; col++) {
			for(int row=0; row<HEIGHT; row++) {
				row_pointers[row][col].r = input_ptr[row][col * 4];
				row_pointers[row][col].g = input_ptr[row][col * 4 + 1]; 
				row_pointers[row][col].b = input_ptr[row][col * 4 + 2]; 
				row_pointers[row][col].a = input_ptr[row][col * 4 + 3];
			}
		}
	}
	else if(!strcmp(option, "2")) { 	// 2. 위아래 뒤집기
		for(int col=0; col<WIDTH; col++) {
			for(int row=0; row<HEIGHT; row++) {
				row_pointers[HEIGHT-row-1][col].r = input_ptr[row][col * 4];
				row_pointers[HEIGHT-row-1][col].g = input_ptr[row][col * 4 + 1]; 
				row_pointers[HEIGHT-row-1][col].b = input_ptr[row][col * 4 + 2]; 
				row_pointers[HEIGHT-row-1][col].a = input_ptr[row][col * 4 + 3];
			}
		}
	}
	else if(!strcmp(option, "3")) {	// 3. 좌우 뒤집기
		for(int col=0; col<WIDTH; col++) {
			for(int row=0; row<HEIGHT; row++) {
				row_pointers[row][WIDTH-col-1].r = input_ptr[row][col * 4];
				row_pointers[row][WIDTH-col-1].g = input_ptr[row][col * 4 + 1]; 
				row_pointers[row][WIDTH-col-1].b = input_ptr[row][col * 4 + 2]; 
				row_pointers[row][WIDTH-col-1].a = input_ptr[row][col * 4 + 3];
			}
		}
	}
	else if(!strcmp(option, "4")) { 	//4. 대각선 뒤집기
		for(int col=0; col<WIDTH; col++) {
			for(int row=0; row<HEIGHT; row++) {
				row_pointers[col][row].r = input_ptr[row][col * 4];
				row_pointers[col][row].g = input_ptr[row][col * 4 + 1]; 
				row_pointers[col][row].b = input_ptr[row][col * 4 + 2]; 
				row_pointers[col][row].a = input_ptr[row][col * 4 + 3];
			}
		}
	}
}

int main(int argc, char *argv[]) {
	srand(time(NULL));

	/* open PNG file for writing */
	FILE *f = fopen("out.png", "wb");
	if (!f) {
		fprintf(stderr, "could not open out.png\n");
		return 1;
	}

	/* initialize png data structures */
	png_structp png_ptr;
	png_infop info_ptr;

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		fprintf(stderr, "could not initialize png struct\n");
		return 1;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		fclose(f);
		return 1;
	}

	/* begin writing PNG File */
	png_init_io(png_ptr, f);
	png_set_IHDR(png_ptr, info_ptr, WIDTH, HEIGHT, COLOR_DEPTH,
	             PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
	             PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);

	/* allocate image data */
	struct Pixel *row_pointers[HEIGHT];
	for (int row = 0; row < HEIGHT; row++) {
		row_pointers[row] = (Pixel *)calloc(WIDTH*2, sizeof(struct Pixel));
	}
    
    const char *ptr = "target.png";
    read_input_png(ptr);

	// row_pointers: 출력할 이미지
	// input_ptr: 입력받은 이미지

	srand(time(NULL));

	reverse(row_pointers, argv[1]);

	/* write image data to disk */
	png_write_image(png_ptr, (png_byte **)row_pointers);

	/* finish writing PNG file */
	png_write_end(png_ptr, NULL);

	/* clean up PNG-related data structures */
	png_destroy_write_struct(&png_ptr, &info_ptr);

	/* close the file */
	fclose(f);
	f = NULL;

	return 0;
}
