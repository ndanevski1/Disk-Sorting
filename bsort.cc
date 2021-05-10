#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include "leveldb/db.h"
#include "leveldb/options.h"
#include "library.h"
#include "parse_schema.h"

// #include "json/json.h"
#include <jsoncpp/json/json.h>


using namespace std;

string get_key_attribute(string tuple, int sort_attribute_index) {
	stringstream ss(tuple);
	vector<string> values;

    while (ss.good()) {
        string substr;
        getline(ss, substr, ',');
        values.push_back(substr);
    }
	return values[sort_attribute_index];
}
int get_first_sort_attribute_length(Schema &schema) {
	int index = schema.sort_attrs[0];
	return schema.attrs[index].length;
}

string parse_key(string a) {
	size_t pos = a.find("_");
    string res = a.substr(0,pos);
    return res;
}

// TODO: FIX THE COMPARATOR
// Implementing a custom comparator;
// source: https://github.com/google/leveldb/blob/master/doc/index.md
// class TwoPartComparator : public leveldb::Comparator {
// 	public:
//   // Three-way comparison function:
//   //   if a < b: negative result
//   //   if a > b: positive result
//   //   else: zero result
//   		int Compare(const leveldb::Slice& a, const leveldb::Slice& b) const {
// 			string a1 = parse_key(a.ToString());
// 			string b1 = parse_key(b.ToString());
// 			return a1.compare(b1);
// 		}

//   // Ignore the following methods for now:
//   		const char* Name() const { return "TwoPartComparator"; }
// 		void FindShortestSeparator(std::string*, const leveldb::Slice&) const {}
// 		void FindShortSuccessor(std::string*) const {}
// };

int main(int argc, const char* argv[]) {

	// if (argc != 4) {
	// 	cout << "ERROR: invalid input parameters!" << endl;
	// 	cout << "Please enter <schema_file> <input_file> <out_index>" << endl;
	// 	exit(1);
	// }

	// Do work here

	leveldb::DB *db;
	leveldb::Options options;
	options.create_if_missing = true;
	// TODO: FIX THE COMPARATOR
	// TwoPartComparator cmp;
	// options.comparator = &cmp;
	leveldb::Status status = leveldb::DB::Open(options, "./leveldb_dir", &db);
	
	vector<string> sort_attrs_name = {"start_year", "student_number"};
	Schema schema = parse_schema("schema_example.json", sort_attrs_name);
	int sort_attr_len = get_first_sort_attribute_length(schema);


	string filename_in = "csv.csv";
	string filename_out = "csv_out_bsort.csv";	
	ifstream input(filename_in);
	ofstream output;
	output.open(filename_out);

	long counter = 0;
	for(string tuple; getline(input, tuple);) {
		string key_s = get_key_attribute(tuple, schema.sort_attrs[0]);
		counter++;
		key_s += "_";
		key_s += to_string(counter);

		leveldb::Slice key = key_s;
		leveldb::Slice value = tuple;

		db->Put(leveldb::WriteOptions(), key, value); 
	}


	leveldb::Iterator* it = db->NewIterator(leveldb::ReadOptions());
	for (it->SeekToFirst(); it->Valid(); it->Next()) {
		// leveldb::Slice key = it->key();
		leveldb::Slice value = it->value();
		// std::string key_str = key.ToString();
		string val_str = value.ToString();
		// cout << key_str << ": "  << val_str << endl;
		output << val_str << endl;
		// cout << val_str << endl;
	}
	assert(it->status().ok());  // Check for any errors found during the scan
	delete it;

	output.close();
	return 0;
}