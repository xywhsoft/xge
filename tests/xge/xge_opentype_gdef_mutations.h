/* Test-only address of an original fixture's GDEF caret field. Each mutation
 * is loaded into a fresh native face; immutable HB caches are never edited. */
static unsigned fixture_u16(const unsigned char* p){return ((unsigned)p[0]<<8)|p[1];}
static uint32_t fixture_u32(const unsigned char* p){return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];}
static unsigned char* fixture_caret(unsigned char* data,size_t bytes,unsigned glyph,unsigned number)
{
    unsigned i,n=fixture_u16(data+4);size_t gdef=0,list,coverage,ligature;unsigned index=UINT_MAX;
    CHECK(bytes>=12 && (size_t)n*16<=bytes-12);
    for(i=0;i<n;i++){unsigned char* table=data+12+i*16;if(!memcmp(table,"GDEF",4)){gdef=fixture_u32(table+8);break;}}
    CHECK(gdef && gdef+12<=bytes);list=gdef+fixture_u16(data+gdef+8);
    coverage=list+fixture_u16(data+list);CHECK(fixture_u16(data+coverage)==1);
    for(i=0;i<fixture_u16(data+coverage+2);i++)if(fixture_u16(data+coverage+4+i*2)==glyph)index=i;
    CHECK(index!=UINT_MAX && index<fixture_u16(data+list+2));ligature=list+fixture_u16(data+list+4+index*2);
    CHECK(number<fixture_u16(data+ligature));ligature+=fixture_u16(data+ligature+2+number*2);
    CHECK(ligature+4<=bytes);return data+ligature;
}
static void fixture_put16(unsigned char* p,unsigned value){p[0]=(unsigned char)(value>>8);p[1]=(unsigned char)value;}
