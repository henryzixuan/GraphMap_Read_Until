/*
 * owler_data.h
 *
 *  Created on: Jul 2, 2015
 *      Author: isovic
 */

#ifndef OWLER_DATA_H_
#define OWLER_DATA_H_

#include <memory>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <sstream>
#include <vector>

#include "minimizer_index/minimizer_index.h"
#include "containers/range.h"

class PairwiseOverlap {
 public:
//  PairwiseOverlap() : qid(0), tid(0), num_seeds(0), cov_bases(0), num_sv(0) { }
  PairwiseOverlap(int64_t _qid, int64_t _tid, int64_t _tid_fwd) : qid(_qid), tid(_tid), tid_fwd(_tid_fwd), num_seeds(0), num_hits(0), cov_bases_query(0), cov_bases_target(0), num_sv(0), lcsk_len(0) { }

  Range query, target;
  int64_t qid, tid, tid_fwd;
  int64_t num_seeds;                // Number of seed hits which survived all LCSk filters.
  int64_t num_hits;                 // Number of raw seed hits, without any sort of LCSk filtering.
  int64_t cov_bases_query;
  int64_t cov_bases_target;
  int32_t num_sv;

  std::vector<int32_t> lcsk_indices;
  std::vector<int32_t> cluster_ids;
  int64_t lcsk_len;

  std::string reject_reason;
};

class OwlerData {
 public:
  OwlerData() { };
  ~OwlerData() { };

  std::vector<uint128_t> hits;
  std::vector<PairwiseOverlap> overlaps;
  std::string unmapped_reason;
  std::string overlap_lines;
//  std::vector<std::string> out_lines;
};

#endif /* OWLER_DATA_H_ */
