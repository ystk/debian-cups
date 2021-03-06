#include "embed.h"
#include "sfnt.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
 
static void example_outfn(const char *buf,int len,void *context) // {{{
{
  FILE *f=(FILE *)context;
  if (fwrite(buf,1,len,f)!=len) {
    fprintf(stderr,"Short write: %m\n");
    assert(0);
    return;
  }
}
// }}}

#define OBJ \
    xref[xrefpos++]=ftell(f); \
    fprintf(f,"%d 0 obj\n",xrefpos);

#define ENDOBJ \
    fprintf(f,"endobj\n");

#define STREAMDICT \
    OBJ; \
    fprintf(f,"<<\n" \
              "  /Length %d 0 R\n",xrefpos+1);

#define STREAMDATA \
    fprintf(f,">>\n" \
              "stream\n"); \
  stream_len=-ftell(f);

#define STREAM \
  STREAMDICT \
  STREAMDATA

#define ENDSTREAM \
  stream_len+=ftell(f); \
  fprintf(f,"endstream\n" \
            "endobj\n"); \
  OBJ; \
  fprintf(f,"%d\n",stream_len); \
  ENDOBJ;

static inline void write_string(FILE *f,EMB_PARAMS *emb,const char *str) // {{{
{
  assert(f);
  assert(emb);
  OTF_FILE *otf=emb->font->sfnt;
  assert(otf);
  int iA;

  if (emb->plan&EMB_A_MULTIBYTE) {
    putc('<',f); 
    for (iA=0;str[iA];iA++) {
      const unsigned short gid=otf_from_unicode(otf,(unsigned char)str[iA]);
      fprintf(f,"%04x",gid);
      bit_set(emb->subset,gid);
    }
    putc('>',f); 
  } else {
    putc('(',f); 
    for (iA=0;str[iA];iA++) {
      bit_set(emb->subset,otf_from_unicode(otf,(unsigned char)str[iA])); // TODO: emb_set(...) encoding/unicode->gid
    }
    fprintf(f,"%s",str); // TODO
    putc(')',f); 
  }
}
// }}}

int main(int argc,char **argv)
{
  const char *fn="/usr/share/fonts/truetype/microsoft/ARIALN.TTF";
  if (argc==2) {
    fn=argv[1];
  }
  OTF_FILE *otf=otf_load(fn);
  assert(otf);
  FONTFILE *ff=fontfile_open_sfnt(otf);
  EMB_PARAMS *emb=emb_new(ff,
                          EMB_DEST_PDF16,
                          EMB_C_FORCE_MULTIBYTE|
                          EMB_C_TAKE_FONTFILE);

  FILE *f=fopen("test.pdf","w");
  assert(f);
  int xref[100],xrefpos=3;
  int stream_len;

  assert(emb->subset);

  fprintf(f,"%%PDF-1.3\n");
  // content
  STREAM;
  fprintf(f,"BT\n" // content
            "  100 100 Td\n"
            "  /F1 10 Tf\n");
  write_string(f,emb,"Hallo");
  fprintf(f," Tj\n"
            "ET\n");
  ENDSTREAM;

  bit_set(emb->subset,otf_from_unicode(otf,'a'));

  // {{{ do font
  EMB_PDF_FONTDESCR *fdes=emb_pdf_fontdescr(emb);
  assert(fdes);
  EMB_PDF_FONTWIDTHS *fwid=emb_pdf_fontwidths(emb);
  assert(fwid);

  STREAMDICT;
  int ff_ref=xrefpos;
  if (emb_pdf_get_fontfile_subtype(emb)) {
    fprintf(f,"  /SubType /%s\n",
              emb_pdf_get_fontfile_subtype(emb));
  }
  if (emb->outtype&EMB_OUTPUT_T1) {
    fprintf(f,"  /Length1 ?\n"
              "  /Length2 ?\n"
              "  /Length3 ?\n");
  } else {
    fprintf(f,"  /Length1 %d 0 R\n",xrefpos+2);
  }
  STREAMDATA;
  const int outlen=emb_embed(emb,example_outfn,f);
  ENDSTREAM;
  OBJ;
  fprintf(f,"%d\n",outlen);
  ENDOBJ;

  OBJ;
  const int fd_ref=xrefpos;
  char *res=emb_pdf_simple_fontdescr(emb,fdes,ff_ref);
  assert(res);
  fputs(res,f);
  free(res);
  ENDOBJ;

  OBJ;
  int f_ref=xrefpos;
  res=emb_pdf_simple_font(emb,fdes,fwid,fd_ref);
  assert(res);
  fputs(res,f);
  free(res);
  ENDOBJ;

  if (emb->plan&EMB_A_MULTIBYTE) {
    OBJ;
    res=emb_pdf_simple_cidfont(emb,fdes->fontname,f_ref);
    f_ref=xrefpos;
    assert(res);
    fputs(res,f);
    free(res);
    ENDOBJ;
  }

  free(fdes);
  free(fwid);
  // }}}

  int iA;

  xref[2]=ftell(f);
  fprintf(f,"3 0 obj\n"
            "<</Type/Page\n"
            "  /Parent 2 0 R\n"
            "  /MediaBox [0 0 595 842]\n"
            "  /Contents 4 0 R\n"
            "  /Resources <<\n"
            "    /Font <<\n"
            "      /F1 %d 0 R\n"
            "    >>\n"
            "  >>\n"
            ">>\n"
            "endobj\n",
            f_ref);
  xref[1]=ftell(f);
  fprintf(f,"2 0 obj\n"
            "<</Type/Pages\n"
            "  /Count 1\n"
            "  /Kids [3 0 R]"
            ">>\n"
            "endobj\n");
  xref[0]=ftell(f);
  fprintf(f,"1 0 obj\n"
            "<</Type/Catalog\n"
            "  /Pages 2 0 R\n"
            ">>\n"
            "endobj\n");
  // {{{ pdf trailer 
  int xref_start=ftell(f);
  fprintf(f,"xref\n"
            "0 %d\n"
            "%010d 65535 f \n",
            xrefpos+1,0);
  for (iA=0;iA<xrefpos;iA++) {
    fprintf(f,"%010d 00000 n \n",xref[iA]);
  }
  fprintf(f,"trailer\n"
          "<<\n"
          "  /Size %d\n"
          "  /Root 1 0 R\n"
          ">>\n"
          "startxref\n"
          "%d\n"
          "%%%%EOF\n",
          xrefpos+1,xref_start);
  // }}}
  fclose(f);

  emb_close(emb);

  return 0;
}
