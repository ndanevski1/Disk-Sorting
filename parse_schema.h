#include <iostream>
#include <fstream>
#include <cassert>

#include "library.h"
#include <jsoncpp/json/json.h>

Schema parse_schema(string schema_file, vector<string> sort_attrs_name);