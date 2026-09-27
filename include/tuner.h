#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "dataset.h"
#include "position.h"
#include "tunable_params.h"

int compute_qscore(Position& position);

double sigmoid(int q, double k);

double compute_error(std::vector<DatasetPosition>& dataset_position, double k);

double compute_error_sum(std::vector<DatasetPosition>& dataset, int begin, int end, double k);

double fit_k(std::vector<DatasetPosition>& dataset);

bool try_delta(TunableParam& param, int delta, std::vector<DatasetPosition>& dataset, double k,
               double& best_error);

void run_tuner(std::vector<DatasetPosition>& dataset, double k);

void print_param(std::ostream& out, const std::string& name, int value);

void print_param_array(std::ostream& out, const std::string& name, int* array, int size);
