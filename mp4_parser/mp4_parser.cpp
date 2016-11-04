#include <iostream>
#include <vector>
#include "mp4_parser.h"


#define FOURCC(c1, c2, c3, c4) \
    (c1 << 24 | c2 << 16 | c3 << 8 | c4)

#define PARSE_DETAIL_INFO(type_box)	do { \
	struct type_box t; \
	fdesc->get_data((u8 *)&t.shape, sizeof(t.shape)); \
	t.print_info(box::tab); \
	t.parse_entry(fdesc, box::tab);\
} while (0)

int box::parse_detail_info(char *type)
{
	int itype = FOURCC(type[0], type[1], type[2], type[3]);
	
	switch (itype)
	{
		case FOURCC('f', 't', 'y', 'p'):
			PARSE_DETAIL_INFO(ftyp_box);
			break;
		case FOURCC('m', 'v', 'h', 'd'):
			PARSE_DETAIL_INFO(mvhd_box);
			break;
		case FOURCC('t', 'k', 'h', 'd'):
			PARSE_DETAIL_INFO(tkhd_box);
			break;
		case FOURCC('m', 'd', 'h', 'd'):
			PARSE_DETAIL_INFO(mdhd_box);
			break;
		case FOURCC('h', 'd', 'l', 'r'):
			PARSE_DETAIL_INFO(hdlr_box);
			break;
		case FOURCC('s', 't', 's', 'c'):
			PARSE_DETAIL_INFO(stsc_box);
			break;
		case FOURCC('s', 't', 'c', 'o'):
			PARSE_DETAIL_INFO(stco_box);
			break;
		case FOURCC('s', 't', 's', 'z'):
			PARSE_DETAIL_INFO(stsz_box);
			break;
		case FOURCC('s', 't', 't', 's'):
			PARSE_DETAIL_INFO(stts_box);
			break;
		default:
			break;
	}

}

vector<chunk_info> chunk_info_list;
vector<u32> chunk_offset_list;
vector<u32> sample_size_list;
vector<time_sampe_info> time_sampe_info_list;

void stsc_box::parse_entry(file_desc *fdesc, char *tab) {
	for (u32 i=0; i<LE(&shape.entry_count); i++) {
		chunk_info ci;
		fdesc->get_data((u8 *)&ci, sizeof(ci));
		ci.first_chunk = LE(&ci.first_chunk);
		ci.samples_per_chunk = LE(&ci.samples_per_chunk);
		ci.sample_description_index = LE(&ci.sample_description_index);
		//save list
		chunk_info_list.push_back(ci);
	}
#if 0
	vector<chunk_info>::iterator it;
	for (it = chunk_info_list.begin(); it != chunk_info_list.end(); ++it)
	{
		cout << "first_chunk: " << (*it).first_chunk;
		cout << " samples_per_chunk: " << (*it).samples_per_chunk;
		cout << " sample_description_index: " << (*it).sample_description_index;
		cout << endl;
	}
#endif
}

void stco_box::parse_entry(file_desc *fdesc, char *tab) {
	for (u32 i=0; i<LE(&shape.entry_count); i++) {
		u32 chunk_offset;
		fdesc->get_data((u8 *)&chunk_offset, sizeof(chunk_offset));
		chunk_offset = LE(&chunk_offset);
		//save list
		chunk_offset_list.push_back(chunk_offset);
	}
#if 0
	vector<u32>::iterator it;
	for (it = chunk_offset_list.begin(); it != chunk_offset_list.end(); ++it)
	{
		cout << "chunk_offset: " << (*it);
		cout << endl;
	}
#endif
}

void stsz_box::parse_entry(file_desc *fdesc, char *tab) {
	for (u32 i=0; i<LE(&shape.sample_count); i++) {
		u32 sample_size;
		fdesc->get_data((u8 *)&sample_size, sizeof(sample_size));
		sample_size = LE(&sample_size);
		//save list
		sample_size_list.push_back(sample_size);
	}
#if 0
	vector<u32>::iterator it;
	for (it = sample_size_list.begin(); it != sample_size_list.end(); ++it)
	{
		cout << "sample_size: " << (*it);
		cout << endl;
	}
#endif
}

void stts_box::parse_entry(file_desc *fdesc, char *tab) {
	for (u32 i=0; i<LE(&shape.entry_count); i++) {
		time_sampe_info tsi;
		fdesc->get_data((u8 *)&tsi, sizeof(tsi));
		tsi.sample_count = LE(&tsi.sample_count);
		tsi.sample_delta = LE(&tsi.sample_delta);
		//save list
		time_sampe_info_list.push_back(tsi);
	}
#if 0
	vector<time_sampe_info>::iterator it;
	for (it = time_sampe_info_list.begin(); it != time_sampe_info_list.end(); ++it)
	{
		cout << "sample_count: " << (*it).sample_count;
		cout << " sample_delta: " << (*it).sample_delta;
		cout << endl;
	}
#endif
}


struct index_table {
	u32 offset;
	u32 size;
	u32 time;
};

vector<index_table> index_table_list;
/*
vector<chunk_info> chunk_info_list;
vector<u32> chunk_offset_list;
vector<u32> sample_size_list;
vector<time_sampe_info> time_sampe_info_list;
*/

chunk_info find_chunk_info(u32 chunk_id)
{
	chunk_info ci;
	
	vector<chunk_info>::iterator it;
	for (it = chunk_info_list.begin(); it != chunk_info_list.end(); ++it)
	{
		if ((*it).first_chunk > chunk_id)
		{
			break;
		}
		ci = *it;
	}

	return ci;
}

void retrieve_index_table(void)
{
	int sample_index = 0;
	
	for (int i=0; i<chunk_offset_list.size(); ++i) {
		index_table t;
		chunk_info ci = find_chunk_info(i);
		for (int s=0; s<ci.samples_per_chunk; ++s) {
			t.offset = chunk_offset_list[i] + s * sample_size_list[s];
			t.size = sample_size_list[s];
			t.time =  sample_index * time_sampe_info_list[0].sample_delta / time_sampe_info_list[0].sample_count;
			index_table_list.push_back(t);
			sample_index++;
		}
	}
	
#if 1
	vector<index_table>::iterator it;
	for (it = index_table_list.begin(); it != index_table_list.end(); ++it)
	{
		cout << "offset: " << (*it).offset;
		cout << " size: " << (*it).size;
		cout << " time: " << (*it).time;
		cout << endl;
	}
#endif
}

void parse_box(file_desc *fdesc, box *father, int level)
{
	while (!fdesc->is_eof()) {
		if (father && fdesc->get_offset() >= father->end_offset)
		{
			break;
		}
		
		box b(fdesc, father, level);
		
		if (!b.init()) {
			break;
		}
		
		if (b.is_parent()) {
			parse_box(fdesc, &b, level + 1);
		}
	}
}


int main(int argc, char **argv) 
{
    if (argc < 2)
    {
        cout << "Usage:" << endl;
        cout << "\t" << argv[0] << " [input_file]" << endl;
        return -1;
    }
	
	file_desc *fdesc = new file_desc(argv[1]);
	
	cout << argv[1] << " size: " << fdesc->get_filesize() << endl;
	
	parse_box(fdesc, NULL, 0);
	
	retrieve_index_table();
	
	delete(fdesc);
}
