/* Adapted from accepted World1/QualityPass02/Fountain02/jpeg_block_patch.c.
 * Focused 64x128, RGB 4:4:4 coefficient preservation; no engine/tool changes. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>
#define CHECK(x,msg) do {if(!(x)){fprintf(stderr,"%s\n",msg);return 2;}}while(0)
int main(int argc,char **argv){
 CHECK(argc==5,"usage: block-patch original.jpg candidate.jpg 8x16-mask.bin output.jpg");
 FILE *a=fopen(argv[1],"rb"),*b=fopen(argv[2],"rb"),*mask=fopen(argv[3],"rb");CHECK(a&&b&&mask,"input open failed");
 unsigned char allowed[128];CHECK(fread(allowed,1,128,mask)==128&&fgetc(mask)==EOF,"mask must be128bytes");fclose(mask);
 struct jpeg_decompress_struct original,candidate;struct jpeg_error_mgr ea,eb,eo;
 original.err=jpeg_std_error(&ea);candidate.err=jpeg_std_error(&eb);jpeg_create_decompress(&original);jpeg_create_decompress(&candidate);
 jpeg_stdio_src(&original,a);jpeg_stdio_src(&candidate,b);for(int m=0;m<16;m++)jpeg_save_markers(&original,JPEG_APP0+m,65535);jpeg_save_markers(&original,JPEG_COM,65535);jpeg_read_header(&original,TRUE);jpeg_read_header(&candidate,TRUE);
 CHECK(original.image_width==64&&original.image_height==128&&candidate.image_width==64&&candidate.image_height==128,"requires64x128");
 CHECK(original.num_components==3&&candidate.num_components==3&&original.jpeg_color_space==candidate.jpeg_color_space,"component/colorspace mismatch");
 for(int q=0;q<NUM_QUANT_TBLS;q++){
  CHECK((original.quant_tbl_ptrs[q]==NULL)==(candidate.quant_tbl_ptrs[q]==NULL),"quant table presence mismatch");
  if(original.quant_tbl_ptrs[q])CHECK(!memcmp(original.quant_tbl_ptrs[q]->quantval,candidate.quant_tbl_ptrs[q]->quantval,sizeof(original.quant_tbl_ptrs[q]->quantval)),"quant tables differ");
 }
 jvirt_barray_ptr *ac=jpeg_read_coefficients(&original),*bc=jpeg_read_coefficients(&candidate);
 int copied=0;
 for(int c=0;c<3;c++){
  jpeg_component_info *x=&original.comp_info[c],*y=&candidate.comp_info[c];
  CHECK(x->h_samp_factor==1&&x->v_samp_factor==1&&y->h_samp_factor==1&&y->v_samp_factor==1,"requires4:4:4");
  CHECK(x->width_in_blocks==8&&x->height_in_blocks==16&&y->width_in_blocks==8&&y->height_in_blocks==16,"block dimensions differ");
  CHECK(x->component_id==y->component_id&&x->quant_tbl_no==y->quant_tbl_no,"component IDs/table mapping differ");
  for(int row=0;row<16;row++){
   JBLOCKARRAY dest=(*original.mem->access_virt_barray)((j_common_ptr)&original,ac[c],row,1,TRUE);
   JBLOCKARRAY src=(*candidate.mem->access_virt_barray)((j_common_ptr)&candidate,bc[c],row,1,FALSE);
   for(int col=0;col<8;col++)if(allowed[row*8+col]){memcpy(dest[0][col],src[0][col],sizeof(JBLOCK));copied++;}
  }
 }
 FILE *out=fopen(argv[4],"wb");CHECK(out,"output open failed");struct jpeg_compress_struct enc;enc.err=jpeg_std_error(&eo);jpeg_create_compress(&enc);jpeg_stdio_dest(&enc,out);jpeg_copy_critical_parameters(&original,&enc);jpeg_write_coefficients(&enc,ac);
 for(jpeg_saved_marker_ptr m=original.marker_list;m;m=m->next){
  if(m->marker==JPEG_APP0&&m->data_length>=5&&!memcmp(m->data,"JFIF",5))continue;
  if(m->marker==JPEG_APP0+14&&m->data_length>=5&&!memcmp(m->data,"Adobe",5))continue;
  jpeg_write_marker(&enc,m->marker,m->data,m->data_length);
 }
 jpeg_finish_compress(&enc);jpeg_destroy_compress(&enc);fclose(out);
 jpeg_finish_decompress(&original);jpeg_finish_decompress(&candidate);jpeg_destroy_decompress(&original);jpeg_destroy_decompress(&candidate);fclose(a);fclose(b);
 printf("Copied %d component blocks (%d spatial8x8blocks); all other coefficients retained.\n",copied,copied/3);return 0;
}
