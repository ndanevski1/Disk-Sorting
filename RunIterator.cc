#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <algorithm>
#include "library.h"


Record* RunIterator::next() {
    return &cur_record;
}

bool RunIterator::has_next() {

    // buffer_index == cur_buffer.size() -> get new buffer
    if(buffer_index == (int) cur_buffer.size()) {
        if(tuples_left == 0)
            return false;

        int read_tuples = min(tuples_left, tuples_in_buf);
        int tuple_len = schema.get_schema_length() + schema.attrs.size();
        
        char* buf_tupples = new char[read_tuples * tuple_len+1];
        fseek(fp, curr_pos, SEEK_SET);
        curr_pos += read_tuples * tuple_len;
        fread(buf_tupples, 1, read_tuples * tuple_len, fp);

        buf_tupples[read_tuples * tuple_len] = 0;
        // cout << "BUFF_TUPLES = " << buf_tupples << endl;
        
        // create the buffer
        cur_buffer = get_records(buf_tupples, tuple_len, schema, read_tuples);
        buffer_index = 0;
        tuples_left -= read_tuples;

        delete buf_tupples;
    }
    cur_record = cur_buffer[buffer_index];
    buffer_index++;
    
    return true;
}