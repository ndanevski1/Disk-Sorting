#ifndef _LIBRARY_H
#define _LIBRARY_H

#include <cstdio>
#include <string>
#include <vector>
using namespace std;

/**
 * An attribute schema. You should probably modify
 * this to add your own fields.
 */

enum type{t_string, t_integer, t_float};

struct Attribute {
  string name;
  int length;
  type t;
  Attribute(string _name, int _length, type _t) : name(_name), length(_length), t(_t) {
  }
};

/**
 * A record schema contains an array of attribute
 * schema `attrs`, as well as an array of sort-by 
 * attributes (represented as the indices of the 
 * `attrs` array).
 */
struct Schema {
  vector<Attribute> attrs;
  vector<int> sort_attrs;
  Schema(vector<Attribute> _attrs, vector<int> _sort_attrs) : attrs(_attrs), sort_attrs(_sort_attrs) {
  }
  int get_serializing_length();
};

/**
 * A record can be defined as a struct with a pointer
 * to the schema and some data. 
 */
struct Record {
  Schema *schema;
  vector<string> data;
  Record() {
  }
  Record(Schema *_schema, vector<string> _data) : schema(_schema), data(_data) {
  }
  void print();
};
int get_file_size(FILE *fp);
vector<Record> get_records(char* buffer, int tuple_len, Schema &schema, int number_of_records);

/**
 * The iterator helps you scan through a run.
 * you can add additional members as your wish
 */
class RunIterator {
  /**
   * Creates an iterator which uses a buffer with
   * size `buf_size` to scan through a run that 
   * starts at file offset `start_pos` with 
   * length `run_length`.
   */
  private:
    Schema &schema;
    FILE *fp;
    long curr_pos;
    long run_length;
    vector<Record> cur_buffer;
    Record cur_record;
    int tuples_left; //from the entire run
    int buffer_index;
    int tuples_in_buf;

  public:
    RunIterator(FILE *_fp, long _start_pos, long _run_length, long buf_size,
    Schema *_schema) : schema(*_schema), fp(_fp), curr_pos(_start_pos), run_length(_run_length) {
        int tuple_len = schema.get_serializing_length();
        assert(buf_size >= tuple_len);
        tuples_left = min(run_length, get_file_size(_fp) - _start_pos) / tuple_len;
        tuples_in_buf = buf_size / tuple_len;
        buffer_index = 0;
        cur_buffer.reserve(tuples_in_buf);
    };
    Schema get_schema();
    /**
     * free memory
     */
    // ~RunIterator();   
    /**
     * reads the next record
     */
    Record* next();
    
    /**
     * return false if iterator reaches the end
     * of the run
     */
    bool has_next();
};

/**
 * Creates sorted runs of length `run_length` in
 * the `out_fp`.
 */
void mk_runs(FILE *in_fp, FILE *out_fp, long run_length, Schema &schema);

/**
 * Merge runs given by the `iterators`.
 * The number of `iterators` should be equal to the `num_runs`.
 * Write the merged runs to `out_fp` starting at position `start_pos`.
 * Cannot use more than `buf_size` of heap memory allocated to `buf`.
 */
void merge_runs(vector<RunIterator *> &iterators, FILE *out_fp,
                long start_pos, char *buf, long buf_size);

#endif