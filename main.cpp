#include <stdio.h>
#include <stdlib.h>
#include <time.h> // rand()함수의 시드값을 지정하는 데 사용

#define PNG_SETJMP_NOT_SUPPORTED
#include <png.h>

#define WIDTH 290	// target.png의 width
#define HEIGHT 290	// target.png의 height
#define COLOR_DEPTH 8
#define UNIT 30

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

// row_pointers: 출력할 이미지
// input_ptr: 입력받은 이미지
void reverse(Pixel *row_pointers[], int option, int col_location, int row_location) {
	// 1. 그대로 출력
	if(option==0) {
		for(int col=0; col<WIDTH; col++) {
			for(int row=0; row<HEIGHT; row++) {
				row_pointers[row_location+row][col_location+col].r = input_ptr[row][col * 4];
				row_pointers[row_location+row][col_location+col].g = input_ptr[row][col * 4 + 1]; 
				row_pointers[row_location+row][col_location+col].b = input_ptr[row][col * 4 + 2]; 
				row_pointers[row_location+row][col_location+col].a = input_ptr[row][col * 4 + 3];
			}
		}
	}
	else if(option==1) { 	// 2. 위아래 뒤집기
		for(int col=0; col<WIDTH; col++) {
			for(int row=0; row<HEIGHT; row++) {
				row_pointers[(row_location+HEIGHT)-row-1][col_location+col].r = input_ptr[row][col * 4];
				row_pointers[(row_location+HEIGHT)-row-1][col_location+col].g = input_ptr[row][col * 4 + 1]; 
				row_pointers[(row_location+HEIGHT)-row-1][col_location+col].b = input_ptr[row][col * 4 + 2]; 
				row_pointers[(row_location+HEIGHT)-row-1][col_location+col].a = input_ptr[row][col * 4 + 3];
			}
		}
	}
	else if(option==2) {	// 3. 좌우 뒤집기
		for(int col=0; col<WIDTH; col++) {
			for(int row=0; row<HEIGHT; row++) {
				row_pointers[row_location+row][(col_location+WIDTH)-col-1].r = input_ptr[row][col * 4];
				row_pointers[row_location+row][(col_location+WIDTH)-col-1].g = input_ptr[row][col * 4 + 1]; 
				row_pointers[row_location+row][(col_location+WIDTH)-col-1].b = input_ptr[row][col * 4 + 2]; 
				row_pointers[row_location+row][(col_location+WIDTH)-col-1].a = input_ptr[row][col * 4 + 3];
			}
		}
	}
	else if(option==3) { 	//4. 대각선 뒤집기
		for(int col=0; col<WIDTH; col++) {
			for(int row=0; row<HEIGHT; row++) {
				row_pointers[row_location+row][col_location+col].r = input_ptr[col][row * 4];
				row_pointers[row_location+row][col_location+col].g = input_ptr[col][row * 4 + 1]; 
				row_pointers[row_location+row][col_location+col].b = input_ptr[col][row * 4 + 2]; 
				row_pointers[row_location+row][col_location+col].a = input_ptr[col][row * 4 + 3];
			}
		}
	}
}

int main(int argc, char *argv[]) {
	srand(time(NULL));

	// output png file(out.png)를 열기
	FILE *f = fopen(argv[2], "wb");
	if (!f) {
		fprintf(stderr, "could not open out.png\n");
		return 1;
	}

	// out.png structure 초기화 과정
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

	png_init_io(png_ptr, f);
	png_set_IHDR(png_ptr, info_ptr, WIDTH*UNIT, HEIGHT*UNIT, COLOR_DEPTH,
	             PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
	             PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);

	// out.png 메모리 2차원 동적 할당
    struct Pixel **row_pointers;
	row_pointers = (struct Pixel **)malloc(sizeof(struct Pixel *) * HEIGHT * UNIT);
	for (int row = 0; row < HEIGHT * UNIT; row++) {
		row_pointers[row] = (struct Pixel *)malloc(sizeof(struct Pixel) * WIDTH * UNIT);
	}

    const char *ptr = argv[1];
    read_input_png(ptr);

	srand(time(NULL));

	for(int i=0; i<HEIGHT*UNIT; i+=HEIGHT) {
		for(int j=0; j<WIDTH*UNIT; j+=WIDTH) {
			//printf("%d %d - ", i,j);
			reverse(row_pointers, rand()%4, i, j);
		}
	}

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
