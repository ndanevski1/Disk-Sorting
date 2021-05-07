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
  int get_schema_length();
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
};

/**
 * Creates sorted runs of length `run_length` in
 * the `out_fp`.
 */
void mk_runs(FILE *in_fp, FILE *out_fp, long run_length, Schema &schema);

/**
 * The iterator helps you scan through a run.
 * you can add additional members as your wish
 */
class RunIterator {
  /**
   * Creates an interator using the `buf_size` to
   * scan through a run that starts at `start_pos`
   * with length `run_length`.
   */
  RunIterator(FILE *fp, long start_pos, long run_length, long buf_size,
              Schema *schema);

  /**
   * free memory
   */
  ~RunIterator();

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
 * Merge runs given by the `iterators`.
 * The number of `iterators` should be equal to the `num_runs`.
 * Write the merged runs to `out_fp` starting at position `start_pos`.
 * Cannot use more than `buf_size` of heap memory allocated to `buf`.
 */
void merge_runs(RunIterator* iterators[], int num_runs, FILE *out_fp,
                long start_pos, char *buf, long buf_size);

