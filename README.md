# multimedia-tools

### es_split_frame.c 
    split es stream to small file on disk.
### flv_avc_parser.c
    parse flv file's avc info
### mp3_parse.c
    mp3 file header parse
### nal_parse.c
    add start code to es stream
### nal_size_to_start_code.c
    replace nal size by start code
### split_file.c
    cut file from head
### split_sps.c
    split 264 file by sps header
### ts_extractor.c
    split the special pid ts packet to a file
### ts_muxer.c
    mux a es to ts file
### ts_parse.c
    parse ts file and print discontinuity_indicator
### ts_pkt_192_to_188.c
    delete first 4 bytes, change to 188 bytes packet
### parshit.py
    using ffprobe to parse ts/ps file, and print pts-pos chart
### mp4_parser
    parse mp4 box and calculate index table

