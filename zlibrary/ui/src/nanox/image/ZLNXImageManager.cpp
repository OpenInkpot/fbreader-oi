/*
 * Copyright (C) 2008 Alexander Egorov <lunohod@gmx.de>
 *
 * dithering algorithm and jpeg/png handling are from coolreader engine
 * Copyright (C) Vadim Lopatin, 2000-2006
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <ZLImage.h>

#include "ZLNXImageManager.h"

extern "C" {
#include <stdio.h>
#include <jpeglib.h>
#include <setjmp.h>
}

#include <png.h>


static const short dither_2bpp_8x8[] = {
	0, 32, 12, 44, 2, 34, 14, 46, 
	48, 16, 60, 28, 50, 18, 62, 30, 
	8, 40, 4, 36, 10, 42, 6, 38, 
	56, 24, 52, 20, 58, 26, 54, 22, 
	3, 35, 15, 47, 1, 33, 13, 45, 
	51, 19, 63, 31, 49, 17, 61, 29, 
	11, 43, 7, 39, 9, 41, 5, 37, 
	59, 27, 55, 23, 57, 25, 53, 21, 
};

int Dither2BitColor( int color, int x, int y )
{
	int cl = ((((color>>16) & 255) + ((color>>8) & 255) + ((color) & 255)) * (256/3)) >> 8;
	if (cl<5)
		return 0;
	else if (cl>=250)
		return 3<<6;
	int d = dither_2bpp_8x8[(x&7) | ( (y&7) << 3 )] - 1;

	cl = ( cl + d - 32 );
	if (cl<5)
		return 0;
	else if (cl>=250)
		return 3<<6;
	return cl & 0xc0;
}

typedef struct {
	struct jpeg_source_mgr pub;   /* public fields */
	int len;
	JOCTET *buffer;      /* start of buffer */
	bool start_of_file;    /* have we gotten any data yet? */
} my_jpeg_source_mgr;

struct my_error_mgr {
	struct jpeg_error_mgr pub;	/* "public" fields */
	jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

void my_jpeg_error (j_common_ptr cinfo) {
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

	char buffer[JMSG_LENGTH_MAX];

	/* Create the message */
	(*cinfo->err->format_message) (cinfo, buffer);

	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	//my_error_ptr myerr = (my_error_ptr) cinfo->err;

	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	(*cinfo->err->output_message) (cinfo);

	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}

void my_init_source (j_decompress_ptr cinfo)
{
	my_jpeg_source_mgr * src = (my_jpeg_source_mgr*) cinfo->src;
	src->start_of_file = true;
}

boolean my_fill_input_buffer (j_decompress_ptr cinfo)
{
	my_jpeg_source_mgr * src = (my_jpeg_source_mgr *) cinfo->src;

	src->pub.next_input_byte = src->buffer;
	src->pub.bytes_in_buffer = src->len; 

	src->start_of_file = false;

	return true;
}

void my_skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
	my_jpeg_source_mgr * src = (my_jpeg_source_mgr *) cinfo->src;
	src->pub.next_input_byte += num_bytes;
	src->pub.bytes_in_buffer -= num_bytes; 
}

boolean my_resync_to_restart (j_decompress_ptr cinfo, int desired)
{
	return false;
}

void my_term_source (j_decompress_ptr cinfo)
{
	my_jpeg_source_mgr * src = (my_jpeg_source_mgr *) cinfo->src;
	if ( src && src->buffer )
	{
		delete[] src->buffer;
		src->buffer = NULL;
	}
	if(src)
		delete src;
}

	void
my_jpeg_src_free (j_decompress_ptr cinfo)
{
	my_jpeg_source_mgr * src = (my_jpeg_source_mgr *) cinfo->src;
	if ( src && src->buffer )
	{
		delete[] src->buffer;
		src->buffer = NULL;
	}
	delete src;
}

struct s_my_png {
	char *p;
	int size;
};

static void mypng_error_func (png_structp png, png_const_charp msg)
{
	longjmp(png_jmpbuf(png), 1);
}

static void mypng_warning_func (png_structp png, png_const_charp msg)
{
	longjmp(png_jmpbuf(png), 1);
}

static void mypng_read_func(png_structp png, png_bytep buf, png_size_t len)
{
	struct s_my_png *obj = (struct s_my_png *) png_get_io_ptr(png);
	size_t bytesRead = obj->size;

	if(bytesRead >= len)
		bytesRead = len;
	else
		longjmp(png_jmpbuf(png), 1);

	memcpy(buf, (const unsigned char*)obj->p, bytesRead);
	obj->p += bytesRead;
	obj->size -= bytesRead;
}


unsigned int ZLNXImageData::width() const {
	return myWidth;
}

unsigned int ZLNXImageData::height() const {
	return myHeight;
}

void ZLNXImageData::init(unsigned int width, unsigned int height) {
	printf("new image: %d %d\n", width, height);
	myWidth = width;
	myHeight = height;

	myImageData = (char*)malloc(myHeight * myWidth);
}

void ZLNXImageData::setPosition(unsigned int x, unsigned int y) {
	printf("setPosition %d %d\n", x, y);
	myX = x;
	myY = y;

	myPosition = myImageData + x / 4 + myWidth * y / 4;
	myShift = (x & 3) << 1;
}

void ZLNXImageData::moveX(int delta) {
	printf("moveX %d\n", delta);
	setPosition(myX + delta, myY);
}

void ZLNXImageData::moveY(int delta) {
	printf("moveY %d\n", delta);
	setPosition(myX, myY + delta);
}

void ZLNXImageData::setPixel(unsigned char r, unsigned char g, unsigned char b) {
	printf("setPixel %d %d\n", myX, myY); 

	int pixel = (0.299 * r + 0.587 * g + 0.114 * b ) / 64;
	pixel = (~pixel & 3) << 6;

	*myPosition &= ~(0xc0 >> myShift);
	*myPosition |= (pixel >> myShift);
}

void ZLNXImageData::copyFrom(const ZLImageData &source, unsigned int targetX, unsigned int targetY) {
	printf("copyFrom %d %d\n", targetX, targetY);

	char *c;
	char *c_src;
	int s, s_src;
	int sW = source.width();
	int sH = source.height();

	ZLNXImageData *source_image = (ZLNXImageData *)&source;

	char *src = source_image->getImageData();

	for(int j = 0; j < sH; j++)
		for(int i = 0; i < sW; i++) {
			c_src = src + i / 4 + sW * j / 4;
			s_src = (i & 3) << 1;


			c = myImageData + (i + targetX) / 4 + myWidth * (j + (targetY - sH)) / 4;
			s = ((i + targetX) & 3) << 1;

			*c &= ~(0xc0 >> s);
			*c |= (((*c_src << s_src) & 0xc0) >> s);
		}
}

shared_ptr<ZLImageData> ZLNXImageManager::createData() const {
	printf("imagemanager::createData()\n");
	return new ZLNXImageData();
}



void ZLNXImageManager::convertImageDirect(const std::string &stringData, ZLImageData &data) const {
	printf("convertImageDirect\n");

	unsigned char m0, m1;
	m0 = *(stringData.data());
	m1 = *(stringData.data()+1);

	if((m0 == 0xff) && (m1 == 0xd8))
		convertImageDirectJpeg(stringData, data);
	else if(!png_sig_cmp((unsigned char *)stringData.data(), (png_size_t)0, 4) )
		convertImageDirectPng(stringData, data);
	else {
		printf("unsupported image format: %d %d\n", m0, m1);

		/*		FILE *f;
				f = fopen("/tmp/unknown_img", "w+");
				fwrite(stringData.data(), 1, stringData.length(), f);
				fclose(f);

				printf("image dumped to /tmp/unknown_img\n");
				*/		

		data.init(1, 1);

		return;
	}
}

void ZLNXImageManager::convertImageDirectJpeg(const std::string &stringData, ZLImageData &data) const {
	struct jpeg_decompress_struct cinfo;
	JSAMPARRAY buffer;		/* Output row buffer */
	int row_stride;		/* physical row width in output buffer */

	struct my_error_mgr jerr;

	jpeg_create_decompress(&cinfo);

	jpeg_error_mgr errmgr;
	cinfo.err = jpeg_std_error(&errmgr);
	errmgr.error_exit = my_jpeg_error;	

	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		 * We need to clean up the JPEG object, close the input file, and return.
		 */

		my_jpeg_src_free (&cinfo);
		jpeg_destroy_decompress(&cinfo);
		return;
	}

	my_jpeg_source_mgr *src;

	src = (my_jpeg_source_mgr *) new my_jpeg_source_mgr;
	cinfo.src = (struct jpeg_source_mgr *) src;

	src->len = stringData.length() + 2;
	src->buffer = new JOCTET[src->len];

	memcpy(src->buffer, (const unsigned char*)stringData.data(), stringData.length());
	src->buffer[stringData.length()] = (JOCTET) 0xFF;
	src->buffer[stringData.length() + 1] = (JOCTET) JPEG_EOI;

	src->pub.init_source = my_init_source;
	src->pub.fill_input_buffer = my_fill_input_buffer;
	src->pub.skip_input_data = my_skip_input_data;
	src->pub.resync_to_restart = my_resync_to_restart;
	src->pub.term_source = my_term_source;
	src->pub.bytes_in_buffer = src->len;
	src->pub.next_input_byte = src->buffer;

	(void) jpeg_read_header(&cinfo, true);

	data.init(cinfo.image_width, cinfo.image_height);


	cinfo.out_color_space = JCS_RGB;
	//    cinfo.out_color_space = JCS_GRAYSCALE;

	(void) jpeg_start_decompress(&cinfo);

	row_stride = cinfo.output_width * cinfo.output_components;
	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	char *p, *c;	
	int pixel, s, j;
	while (cinfo.output_scanline < cinfo.output_height) {
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);

		p = (char*)buffer[0];
		j = cinfo.output_scanline;

		for(int i = 0; i < cinfo.output_width; i++) {			
			pixel = Dither2BitColor( 
					p[0] << 16 | p[1] << 8 | p[2] << 0,
					i, j);

			c = ((ZLNXImageData&)data).getImageData() + i / 4 + cinfo.output_width * j / 4;
			s = (i & 3) << 1;

			*c &= ~(0xc0 >> s);
			*c |= (pixel >> s);

			p += 3;
		}
	}

	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
}

void ZLNXImageManager::convertImageDirectPng(const std::string &stringData, ZLImageData &data) const {
	struct s_my_png my_png;
	my_png.p = (char*)stringData.data();
	my_png.size = stringData.length();

	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	unsigned int *row = NULL;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
			(png_voidp)&my_png, mypng_error_func, mypng_warning_func);
	if ( !png_ptr )
		return;

	if (setjmp( png_ptr->jmpbuf )) {
		data.init(0, 0);
		if (png_ptr)
		{
			png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
		}
		if ( row )
			delete row;
		return;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		mypng_error_func(png_ptr, "cannot create png info struct");
	png_set_read_fn(png_ptr,
			(voidp)&my_png, mypng_read_func);
	png_read_info( png_ptr, info_ptr );


	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	png_get_IHDR(png_ptr, info_ptr, &width, &height,
			&bit_depth, &color_type, &interlace_type,
			NULL, NULL);

	data.init(width, height);

	row = new unsigned int[ width ];

	// SET TRANSFORMS
	if (color_type & PNG_COLOR_MASK_PALETTE)
		png_set_palette_to_rgb(png_ptr);

	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_gray_1_2_4_to_8(png_ptr);

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);

	if (bit_depth == 16)
		png_set_strip_16(png_ptr);

	png_set_invert_alpha(png_ptr);

	if (bit_depth < 8)
		png_set_packing(png_ptr);

	//if (color_type == PNG_COLOR_TYPE_RGB)
	png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);

	//if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	//    png_set_swap_alpha(png_ptr);

	if (color_type == PNG_COLOR_TYPE_GRAY ||
			color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
		png_set_gray_to_rgb(png_ptr);

	int number_passes = png_set_interlace_handling(png_ptr);
	//if (color_type == PNG_COLOR_TYPE_RGB_ALPHA ||
	//    color_type == PNG_COLOR_TYPE_GRAY_ALPHA)

	//if (color_type == PNG_COLOR_TYPE_RGB ||
	//    color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	png_set_bgr(png_ptr);


	char *c;	
	unsigned int *p;
	int pixel, s;
	for(int pass = 0; pass < number_passes; pass++) {
		for(int y = 0; y < height; y++) {

			png_read_rows(png_ptr, (unsigned char **)&row, png_bytepp_NULL, 1);

			p = row;

			for(int i = 0; i < width; i++) {			
				pixel = Dither2BitColor(*p, i, y);

				c = ((ZLNXImageData&)data).getImageData() + i / 4 + width * y / 4;
				s = (i & 3) << 1;

				*c &= ~(0xc0 >> s);
				*c |= (pixel >> s);

				p++;
			}
		}
	}

	png_read_end(png_ptr, info_ptr);

	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
}
