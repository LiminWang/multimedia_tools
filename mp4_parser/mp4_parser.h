#ifndef _MP4_PARSER_H_
#define _MP4_PARSER_H_

#include <iostream>
#include <iomanip>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


#define is_type(a) (((a) >= 'a' && (a) <= 'z') || \
	((a) >= '0' && (a) <= '9') || ((a) == ' '))

using namespace std;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef	unsigned long long u64;

template <typename T>  
T LE(const T *v)
{
	if (sizeof(T) == 1) {
		return *v;
	} else if (sizeof(T) == 2) {
    	return ((*v & 0xff) << 8) | (*v >> 8);  
	} else if (sizeof(T) == 4) {
		return (*v >> 24) |
        	((*v & 0x00ff0000) >> 8) |
        	((*v & 0x0000ff00) << 8) |
        	(*v << 24); 
	} else if (sizeof(T) == 8) {
		T tmp = *v;
		tmp = ((tmp & 0x00ff00ff00ff00ff) << 8) | ((tmp & 0xff00ff00ff00ff00) >> 8);
	    tmp = ((tmp & 0x0000ffff0000ffff) << 16) | ((tmp & 0xffff0000ffff0000) >> 16);
		return tmp;
	}
  	
  	return *v;
}  

struct file_desc {
public:
	file_desc(char *mfile_name) {
		strcpy(file_name, mfile_name);
		fd = fopen(mfile_name, "rb");
		cout << "fd = " << hex << fd << dec << endl;
		fseek(fd, 0L, SEEK_END);
		size = ftell(fd);
		fseek(fd, 0L, SEEK_SET);
		offset = 0;
		eof = false;
	}
	
	~file_desc() {
		fclose(fd);
	}
	
	int get_data(u8 *mdata, u32 msize) {
		if (fread(mdata, 1, msize, fd) < msize)
		{
			cout << "Read EOF!\n" << endl;
			eof = true;
			return -2;
		}
		offset += msize;
		return 0;
	}
	
	int peek_data(u8 *mdata, u32 msize) {
		if (fread(mdata, 1, msize, fd) < msize)
		{
			cout << "Read EOF!\n" << endl;
			eof = true;
			return -2;
		}
		fseek(fd, offset, SEEK_SET);
		return 0;
	}
	
	int skip_data(u32 msize) {
		fseek(fd, msize, SEEK_CUR);
		offset += msize;
		return 0;
	}
	
	int seek(u32 moffset) {
		fseek(fd, moffset, SEEK_SET);
		offset = moffset;
		return 0;
	}
	
	u32 get_offset(void) {
		return offset;
	}
	
	u32 get_filesize(void) {
		return size;
	}
	
	bool is_eof(void) {
		return eof;
	}
private:
	char file_name[128];
	FILE *fd;
	u32 size;
	u32 offset;
	bool eof;
};

struct box_member {
public:
	u32 size;
	u8 type[4];
	u64 largesize;
};

class box {
public:
	box(file_desc *mfdesc, box *mfather = NULL, int mlevel = 0) {
		start_offset = 0;
		end_offset = 0;
		father = mfather;
		level = mlevel;
		fdesc = mfdesc;
		memset(type, 0, sizeof(type));
		memset(tab, 0, sizeof(tab));
		tab[0] = '|';
	}
	
	~box() {
		fdesc->seek(end_offset);
	}
	
	bool init(void) {
		start_offset = fdesc->get_offset();
		if (fdesc->get_data((u8 *)&m.size, 4) < 0) {
			return false;
		}
		m.size = LE(&m.size);
		if (fdesc->get_data((u8 *)type, 4) < 0) {
			return false;
		}
		m.largesize = 0;
		if (m.size == 1) {
			if (fdesc->get_data((u8 *)&m.largesize, 8) < 0) {
				return false;
			}
			m.largesize = LE(&m.largesize);
		}
		
		u32 size = m.largesize ? m.largesize : m.size;
		end_offset = start_offset + size;
		
		print_info();
		parse_detail_info(type);
		
		return true;
	}
	
	bool is_parent(void) {
		u32 size = 0;
		fdesc->peek_data((u8 *)&size, 4);
		size = LE(&size);
		if (size > fdesc->get_filesize()) {
			return false;
		}
		
		u8 type[8] = {0};
		fdesc->peek_data(type, 8);
		if (is_type(type[4]) &&
			is_type(type[5]) &&
			is_type(type[6]) &&
			is_type(type[7])) {
			return true;
		}
		return false;
	}
	
	void print_info(void) {
		for (int i=0; i<level; i++) {
			tab[2*i + 1] = '-';
			tab[2*i + 2] = '-';
		}
		cout << tab << "type: " << type << " size: " << hex << 
			m.size << dec << endl;
		if (m.largesize) {
			cout << tab << "largesize: " << hex << m.largesize <<
				dec << endl;
		}
	}
	
	int parse_detail_info(char *type);
	
	box_member m;
	void *data;
	box *father;
	u32 start_offset;
	u32 end_offset;
	
protected:
 	file_desc *fdesc;
 	
private:
	int level;
	char type[5];
	char tab[32];
};


#define PRINT_TYPE_VAL_DEC(a)  do { \
	std::setw(2*sizeof(a)); \
	cout << tab << #a << ": " << (u32)LE(&(a)) << endl; \
} while (0)
#define PRINT_TYPE_VAL_HEX(a)  do { \
	setw(2*sizeof(a)); \
	cout << tab << #a << ": " << hex << (u32)LE(&(a)) << dec << endl; \
} while (0)

#define PRINT_TYPE_ARRAY_HEX(a, size)  do { \
	cout << tab << #a << ": "; \
	setw(2*sizeof(a[0])); \
	for (int i=0; i<size; i++) { \
		cout << hex << (u32)a[i] << " "; \
	} \
	cout << dec << endl; \
} while (0)
#define PRINT_TYPE_ARRAY_STR(a, size)  do { \
	cout << tab << #a << ": "; \
	for (int i=0; i<size; i++) { \
		cout << a[i] << " "; \
	} \
	cout << endl; \
} while (0)

struct ftyp_box {
	struct {
		u32 major_brand;
		u32 minor_version;
		u8 compatible_brands[12];
	} shape;
	void print_info(char *tab) {
		PRINT_TYPE_VAL_DEC(shape.major_brand);
		PRINT_TYPE_VAL_DEC(shape.minor_version);
		PRINT_TYPE_ARRAY_HEX(shape.compatible_brands, 12);
	}
	void parse_entry(file_desc *fdesc, char *tab) {}
};

struct mvhd_box {
	struct {
		u8 version;
		u32 creation_time;
		u32 modification_time;
		u32 time_scale;
		u32 duration;
		u32 rate;
		u16 volume;
		u8 matrix[36];
		u8 pre_defined[24];
		u32 next_track_id;
	} shape;
	void print_info(char *tab) {
		PRINT_TYPE_VAL_DEC(shape.version);
		PRINT_TYPE_VAL_DEC(shape.creation_time);
		PRINT_TYPE_VAL_DEC(shape.modification_time);
		PRINT_TYPE_VAL_DEC(shape.time_scale);
		PRINT_TYPE_VAL_DEC(shape.duration);
		PRINT_TYPE_VAL_DEC(shape.rate);
		PRINT_TYPE_VAL_DEC(shape.volume);
		PRINT_TYPE_ARRAY_HEX(shape.matrix, 36);
		PRINT_TYPE_ARRAY_HEX(shape.pre_defined, 24);
		PRINT_TYPE_VAL_DEC(shape.next_track_id);
	}
	void parse_entry(file_desc *fdesc, char *tab) {}
};

struct tkhd_box {
	struct {
		u8 version;
		u8 flags[3];
		u32 creation_time;
		u32 modification_time;
		u32 track_id;
		u8 reserved[4];
		u32 duration;
		u8 reserved2[8];
		u16 layer;
		u16 alternate_group;
		u16 volume;
		u16 reserved3;
		u8 matrix[36];
		u32 width;
		u32 height;
	} shape;
	void print_info(char *tab) {
		PRINT_TYPE_VAL_DEC(shape.version);
		PRINT_TYPE_ARRAY_HEX(shape.flags, 3);
		PRINT_TYPE_VAL_DEC(shape.creation_time);
		PRINT_TYPE_VAL_DEC(shape.modification_time);
		PRINT_TYPE_VAL_DEC(shape.track_id);
		PRINT_TYPE_VAL_DEC(shape.duration);
		PRINT_TYPE_VAL_DEC(shape.layer);
		PRINT_TYPE_VAL_DEC(shape.alternate_group);
		PRINT_TYPE_VAL_DEC(shape.volume);
		PRINT_TYPE_ARRAY_HEX(shape.matrix, 36);
		PRINT_TYPE_VAL_HEX(shape.width);
		PRINT_TYPE_VAL_HEX(shape.height);
	}
	void parse_entry(file_desc *fdesc, char *tab) {}
};

struct mdhd_box {
	struct {
		u8 version;
		u8 flags[3];
		u32 creation_time;
		u32 modification_time;
		u32 time_scale;
		u32 duration;
		u16 language;
		u16 pre_defined;
	} shape;
	void print_info(char *tab) {
		PRINT_TYPE_VAL_DEC(shape.version);
		PRINT_TYPE_ARRAY_HEX(shape.flags, 3);
		PRINT_TYPE_VAL_DEC(shape.creation_time);
		PRINT_TYPE_VAL_DEC(shape.modification_time);
		PRINT_TYPE_VAL_DEC(shape.time_scale);
		PRINT_TYPE_VAL_DEC(shape.duration);
		PRINT_TYPE_VAL_DEC(shape.language);
		PRINT_TYPE_VAL_DEC(shape.pre_defined);
	}
	void parse_entry(file_desc *fdesc, char *tab) {}
};

struct hdlr_box {
	struct {
		u8 version;
		u8 flags[3];
		u32 pre_defined;
		u8 handler_type[4];
		u8 reserved[12];
		u8 name[0];
	} shape;
	void print_info(char *tab) {
		PRINT_TYPE_VAL_DEC(shape.version);
		PRINT_TYPE_ARRAY_HEX(shape.flags, 3);
		PRINT_TYPE_VAL_DEC(shape.pre_defined);
		PRINT_TYPE_ARRAY_STR(shape.handler_type, 4);
		PRINT_TYPE_ARRAY_STR(shape.name, strlen((const char *)shape.name));
	}
	void parse_entry(file_desc *fdesc, char *tab) {}
};

struct chunk_info {
	u32 first_chunk;
	u32 samples_per_chunk;
	u32 sample_description_index;
};

struct stsc_samples_in_chunk {
	struct chunk_info shape;
	void print_info(char *tab) {
		PRINT_TYPE_VAL_DEC(shape.first_chunk);
		PRINT_TYPE_VAL_DEC(shape.samples_per_chunk);
		PRINT_TYPE_VAL_DEC(shape.sample_description_index);
	}
};

struct stsc_box {
	struct {
		u8 version;
		u8 flags[3];
		u32 entry_count;
	} shape;
	// struct stsc_samples_in_chunk
	void print_info(char *tab) {
		PRINT_TYPE_VAL_DEC(shape.version);
		PRINT_TYPE_ARRAY_HEX(shape.flags, 3);
		PRINT_TYPE_VAL_DEC(shape.entry_count);
	}
	void parse_entry(file_desc *fdesc, char *tab);
};

struct stco_box {
	struct {
		u8 version;
		u8 flags[3];
		u32 entry_count;
	} shape;
	// struct chunk_offset
	void print_info(char *tab) {
		PRINT_TYPE_VAL_DEC(shape.version);
		PRINT_TYPE_ARRAY_HEX(shape.flags, 3);
		PRINT_TYPE_VAL_DEC(shape.entry_count);
	}
	void parse_entry(file_desc *fdesc, char *tab);
};

struct stsz_box {
	struct {
		u8 version;
		u8 flags[3];
		u32 sample_size;
		u32 sample_count;
	} shape;
	// struct entry_size
	void print_info(char *tab) {
		PRINT_TYPE_VAL_DEC(shape.version);
		PRINT_TYPE_ARRAY_HEX(shape.flags, 3);
		PRINT_TYPE_VAL_DEC(shape.sample_size);
		PRINT_TYPE_VAL_DEC(shape.sample_count);
	}
	void parse_entry(file_desc *fdesc, char *tab);
};

struct time_sampe_info {
	u32 sample_count;
	u32 sample_delta;
};

struct stts_box {
	struct {
		u8 version;
		u8 flags[3];
		u32 entry_count;
	} shape;
	// struct time_sampe_info
	void print_info(char *tab) {
		PRINT_TYPE_VAL_DEC(shape.version);
		PRINT_TYPE_ARRAY_HEX(shape.flags, 3);
		PRINT_TYPE_VAL_DEC(shape.entry_count);
	}
	void parse_entry(file_desc *fdesc, char *tab);
};


#endif
