#pragma once

#include <vector>

#include "position.h"

struct DatasetPosition {
    Position position;
    double result;
};

DatasetPosition parse_epd_line(const std::string& line);
std::vector<DatasetPosition> load_epd_dataset(const std::string& path);
