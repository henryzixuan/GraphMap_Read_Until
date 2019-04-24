/*
 * owler_experimental.cc
 *
 *  Created on: Feb 16, 2017
 *      Author: isovic
 */

#include "owler.h"
#include "minimizer_index/minimizer_index.h"

bool Owler::CheckOverlap_(std::shared_ptr<is::MinimizerIndex> index, const SingleSequence *read, const ProgramParameters *parameters, PairwiseOverlap& overlap) {
  int64_t tid = overlap.tid % index->get_num_sequences_forward();
  std::string rev_suffix = (overlap.tid >= index->get_num_sequences_forward()) ? (std::string("-rev")) : (std::string("-fwd"));

  if (overlap.num_seeds == 0) {
    std::string temp = FormatString("[qid = %ld, tid = %ld%s] overlap.num_seeds = 0\n", overlap.qid, tid, rev_suffix.c_str());
    overlap.reject_reason = temp;
    LOG_DEBUG_SPEC_NO_HEADER("\t%s", temp.c_str());
    return false;
  }

  if (overlap.num_seeds < parameters->min_num_seeds) {
    std::string temp = FormatString("[qid = %ld, tid = %ld%s] overlap.num_seeds < 4\n", overlap.qid, tid, rev_suffix.c_str());
    overlap.reject_reason = temp;
    LOG_DEBUG_SPEC_NO_HEADER("\t%s", temp.c_str());
    return false;
  }

//  if (overlap.num_sv > 0) {
//    return false;
//  }

  double perc_cov_bases_q = ((double) overlap.cov_bases_query) / ((double) overlap.query.dist());
  double perc_cov_bases_t = ((double) overlap.cov_bases_target) / ((double) overlap.target.dist());
  double perc_cov_bases = std::max(perc_cov_bases_q, perc_cov_bases_t);

   if (perc_cov_bases < parameters->min_percent_cov_bases) {
     return false;
  }

  double ratio = CalcRatio_(overlap);

  if ((1.0 - ratio) > parameters->error_rate) {
    std::string temp = FormatString("[qid = %ld, tid = %ld%s] (1.0 - ratio) = %f > %f\n", overlap.qid, tid, rev_suffix.c_str(), (1.0 - ratio), parameters->error_rate);
    overlap.reject_reason = temp;
    LOG_DEBUG_SPEC_NO_HEADER("\t%s", temp.c_str());
    return false;
  }

  int64_t read_len = read->get_sequence_length();
  int64_t target_len = index->get_reference_lengths()[overlap.tid];

  double max_overhang = (parameters->max_allowed_overhang >= 0) ? (parameters->max_allowed_overhang) : (std::max(read_len, target_len));
  double overhang_percent = parameters->overhang_percent;
  int64_t min_overlap_len = parameters->min_overlap_len;
  int64_t min_read_len = parameters->min_read_len;
  int64_t min_cov_bases = parameters->min_num_anchor_bases;
  int64_t margin_read = std::min(max_overhang, overhang_percent * read_len);
  int64_t margin_target = std::min(max_overhang, overhang_percent * target_len);

  if (overlap.cov_bases_query < min_cov_bases || overlap.cov_bases_target < min_cov_bases) {
    return false;
  }

  if (read_len < min_read_len || target_len < min_read_len) {
    return false;
  }

  if (overlap.query.dist() < min_overlap_len || overlap.target.dist() < min_overlap_len) {
    return false;
  }

  if (overlap.query.start > margin_read && overlap.target.start > margin_target) {
    std::string temp = FormatString("[qid = %ld, tid = %ld%s] margin start fail: overlap.query.start = %ld, margin_read = %ld, overlap.target.start = %ld, margin_target = %ld\n",
                                    overlap.qid, tid, rev_suffix.c_str(), overlap.query.start, margin_read, overlap.target.start, margin_target);
    overlap.reject_reason = temp;
    LOG_DEBUG_SPEC_NO_HEADER("\t%s", temp.c_str());
    return false;
  }

  if ((read_len - overlap.query.end) > margin_read && (target_len - overlap.target.end) > margin_target) {
    std::string temp = FormatString("[qid = %ld, tid = %ld%s] margin end fail: (read_len - overlap.query.end) = %ld, margin_read = %ld, (target_len - overlap.target.end) = %ld, margin_target = %ld\n",
                                    overlap.qid, tid, rev_suffix.c_str(), (read_len - overlap.query.end), margin_read, (target_len - overlap.target.end), margin_target);
    overlap.reject_reason = temp;
    LOG_DEBUG_SPEC_NO_HEADER("\t%s", temp.c_str());
    return false;
  }

  LOG_DEBUG_SPEC_NO_HEADER("[qid = %ld, tid = %ld%s] Accepted!\n\n", overlap.qid, tid, rev_suffix.c_str());

  return true;
}

int64_t Owler::CalcEditDist_(std::shared_ptr<is::MinimizerIndex> index, const SingleSequence *read, const PairwiseOverlap& overlap) {

  EdlibAlignTask task = EDLIB_TASK_DISTANCE;  // Calculate only edit distance.
  EdlibAlignMode edlib_mode = EDLIB_MODE_NW;
  char *ref = ((char *) index->get_data().data()) + index->get_reference_starting_pos()[overlap.tid];

  EdlibAlignResult result = edlibAlign((const char *) read->get_data() + overlap.query.start, overlap.query.dist() + 1,
                                       ref + overlap.target.start, overlap.target.dist() + 1,
                                       edlibNewAlignConfig(-1, edlib_mode, task));

  int64_t edit_dist = result.editDistance;

  edlibFreeAlignResult(result);

  return edit_dist;
}

double Owler::CalcRatio_(const PairwiseOverlap& overlap) {
  double dist_query = overlap.query.dist();
  double dist_target = overlap.target.dist();
  double ratio = std::min(dist_query, dist_target) / std::max(dist_query, dist_target);
  return ratio;
}

std::string Owler::GenerateDebugInfo_(std::shared_ptr<is::MinimizerIndex> index, const SingleSequence *read, const ProgramParameters *parameters, const PairwiseOverlap& overlap) {
  std::stringstream ss;

  double dist_query = overlap.query.dist();
  double dist_target = overlap.target.dist();
  double ratio = std::min(dist_query, dist_target) / std::max(dist_query, dist_target);

  int64_t edit_dist = 0;
//  edit_dist = CalcEditDist_(index, read, overlap);

  ss << "cov_bases_target = " << overlap.cov_bases_target << ", d_query = " << overlap.query.dist() << ", d_target = " << overlap.target.dist() << ", ratio = " << ratio << ", edit_dist = " << edit_dist << ", error_rate = " << (edit_dist / dist_target);

  return ss.str();
}










struct ClusterAndIndices {
  Range query;
  Range ref;
  int32_t num_anchors = 0;
  int32_t coverage = 0;
  std::vector<int> lcskpp_indices;
};

bool Owler::CheckDistanceTooBig_(const std::vector<uint128_t> &hits, int64_t index_last, int64_t index_current, float error_rate) {
  int64_t seed_length = 12;
  int64_t distance_query = (Owler::HitPosRead_(hits[index_current]) + seed_length) - Owler::HitPosRead_(hits[index_last]);
  int64_t distance_ref = (Owler::HitPosRef_(hits[index_current]) + seed_length) - Owler::HitPosRef_(hits[index_last]);
  float max_length = ((float) std::max(distance_query, distance_ref));
  float min_length = ((float) std::min(distance_query, distance_ref));
  if (max_length > 100 && ((min_length == 0 && max_length != 0) || (min_length > 0 && (max_length / min_length - 1.0f) > error_rate))) {
    return true;
  }

  return false;
}

int Owler::FilterAnchorBreakpoints_(const std::vector<int32_t> &lcskpp_indices, int64_t ref_hits_start, int64_t ref_hits_end, int64_t seed_length,
                                   int64_t min_cluster_length, float min_cluster_coverage, const std::vector<uint128_t> &hits,
                                   const ProgramParameters* parameters, std::vector<int32_t> &ret_filtered_lcskpp_indices,
                                   std::vector<int32_t> *ret_cluster_ids) {

  std::vector<ClusterAndIndices *> clusters;
  ClusterAndIndices *new_cluster = NULL;
  int64_t last_nonskipped_i = lcskpp_indices.size() + 1;
  for (int64_t i=(lcskpp_indices.size() - 1); i >= 0; i--) {
    /// Skip anchors which might be too erroneous.
    int64_t current_lcskp_index = lcskpp_indices.at(i) + ref_hits_start;
    if (CheckDistanceTooBig_(hits, current_lcskp_index, current_lcskp_index, parameters->error_rate / 2.0f) == true) {
      continue;
    }

    if (last_nonskipped_i > lcskpp_indices.size()) {

    } else {
      /// This is going to work, because last_nonskipped_i will be set the second iteration of the loop. The value of i starts counting from int64_t i=(lcskpp_indices.size() - 1).
      int64_t previous_lcskp_index = lcskpp_indices.at(last_nonskipped_i) + ref_hits_start;

      bool wrong_to_previous1 = CheckDistanceTooBig_(hits, previous_lcskp_index, current_lcskp_index, parameters->error_rate / 2.0f);
      bool wrong_to_previous2 = (new_cluster->lcskpp_indices.size() < 2) ? false :
                                (CheckDistanceTooBig_(hits, new_cluster->lcskpp_indices[new_cluster->lcskpp_indices.size()-2] + ref_hits_start, current_lcskp_index, parameters->error_rate / 2.0f));
      bool wrong_by_distance = false;

      if ((wrong_to_previous1 == true && wrong_to_previous2 == true) || (new_cluster->lcskpp_indices.size() > 1 && wrong_by_distance == true)) {
        /// In this case, the new point is a general outlier to the previous LCSk, because it doesn't fit nesither to the previous point, nor to the point before that.
        if (new_cluster != NULL) {
//          int64_t cov_bases_read = 0, cov_bases_ref = 0;
//          CalcCoveredBases(owler_data->seed_hits2, seed_length, new_cluster->lcskpp_indices, ref_hits_start, ref_hits_end, &cov_bases_read, &cov_bases_ref);
          int64_t cov_bases_read = new_cluster->lcskpp_indices.size() * seed_length; // overlap.cov_bases;
          int64_t cov_bases_ref =  new_cluster->lcskpp_indices.size() * seed_length; // overlap.cov_bases;
          new_cluster->coverage = std::max(cov_bases_read, cov_bases_ref);

          int64_t min_covered_bases = (new_cluster->query.end - new_cluster->query.start + 1) * min_cluster_coverage;

          if ((new_cluster->query.end - new_cluster->query.start + 1) >= min_cluster_length && new_cluster->coverage >= min_covered_bases) {
            clusters.push_back(new_cluster);
          } else {
            delete new_cluster;
          }
          new_cluster = NULL;
        }
      } else if (wrong_to_previous1 == true && wrong_to_previous2 == false) {
        /// In this case, the previous point was an outlier, because the new point fits better to the one before the previous one. Overwrite the previous entry in new_cluster.

        new_cluster->query.end = Owler::HitPosRead_(hits[current_lcskp_index]) + seed_length - 1;
        new_cluster->ref.end = Owler::HitPosRef_(hits[current_lcskp_index]) + seed_length - 1;

        /// This should not change, as we remove 12 bases and add 12 bases.
//          new_cluster->coverage -= local_score->get_registry_entries().covered_bases_queries[previous_lcskp_index];
//          new_cluster->coverage += local_score->get_registry_entries().covered_bases_queries[current_lcskp_index];
        new_cluster->lcskpp_indices[new_cluster->lcskpp_indices.size()-1] = current_lcskp_index - ref_hits_start;

        if (new_cluster->lcskpp_indices.size() == 1) {
          new_cluster->query.start = Owler::HitPosRead_(hits[current_lcskp_index]);
          new_cluster->ref.start = Owler::HitPosRef_(hits[current_lcskp_index]);
        }
        last_nonskipped_i = i;

        continue;
      }
    }

    if (new_cluster == NULL) {
      new_cluster = new ClusterAndIndices;
      new_cluster->query.start = Owler::HitPosRead_(hits[current_lcskp_index]);
      new_cluster->ref.start = Owler::HitPosRef_(hits[current_lcskp_index]);
    }

    new_cluster->query.end = Owler::HitPosRead_(hits[current_lcskp_index]) + seed_length - 1;
    new_cluster->ref.end = Owler::HitPosRef_(hits[current_lcskp_index]) + seed_length - 1;
    new_cluster->num_anchors += 1;
    new_cluster->lcskpp_indices.push_back(current_lcskp_index - ref_hits_start);

    last_nonskipped_i = i;
  }
  if (new_cluster != NULL) {
//    int64_t cov_bases_read = 0, cov_bases_ref = 0;
//    CalcCoveredBases(hits, seed_length, new_cluster->lcskpp_indices, ref_hits_start, ref_hits_end, &cov_bases_read, &cov_bases_ref);
    int64_t cov_bases_read = new_cluster->lcskpp_indices.size() * seed_length; // overlap.cov_bases;
    int64_t cov_bases_ref =  new_cluster->lcskpp_indices.size() * seed_length; // overlap.cov_bases;

    new_cluster->coverage = std::max(cov_bases_read, cov_bases_ref);

    int64_t min_covered_bases = (new_cluster->query.end - new_cluster->query.start + 1) * min_cluster_coverage;

    if ((new_cluster->query.end - new_cluster->query.start + 1) >= min_cluster_length && new_cluster->coverage >= min_covered_bases) {
      clusters.push_back(new_cluster);
    } else {
      delete new_cluster;
    }
    new_cluster = NULL;
  }

  if (ret_cluster_ids) {
    ret_cluster_ids->clear();
  }

  int ret_val = 0;
  /// Check if the leftover clusters are linear and that only outlier anchors are filtered. This is important for overlapping.
  for (int64_t i=1; i<clusters.size(); i++) {
    int64_t current_lcskp_index = clusters[i]->lcskpp_indices.front() + ref_hits_start;
    int64_t previous_lcskp_index = clusters[i-1]->lcskpp_indices.back() + ref_hits_start;

    bool wrong_to_previous1 = CheckDistanceTooBig_(hits, previous_lcskp_index, current_lcskp_index, parameters->error_rate);
    if (wrong_to_previous1 == true) {
      ret_val += 1;
    }
  }

  ret_filtered_lcskpp_indices.clear();

  for (int64_t i=0; i<clusters.size(); i++) {
    ret_filtered_lcskpp_indices.insert(ret_filtered_lcskpp_indices.end(), clusters[i]->lcskpp_indices.begin(), clusters[i]->lcskpp_indices.end());

    /// Create indices for debugging purposes (so we can differentiate clusters).
    if (ret_cluster_ids) {
      std::vector<int32_t> cluster_indices(clusters[i]->lcskpp_indices.size(), i);
      ret_cluster_ids->insert(ret_cluster_ids->end(), cluster_indices.begin(), cluster_indices.end());
    }

    if (clusters[i])
      delete clusters[i];
  }

  clusters.clear();

  return ret_val;
}

void Owler::FilterColinear_(std::shared_ptr<is::MinimizerIndex> index, const SingleSequence *read, const ProgramParameters *parameters,
                            const std::vector<uint128_t> &hits, int64_t begin_hit, int64_t end_hit, int64_t seed_len, const std::vector<int32_t> &raw_lcsk_indices,
                            std::vector<int32_t> &lcsk_indices, std::vector<int32_t> *cluster_ids, int32_t &num_sv) {

//  lcsk_indices = raw_lcsk_indices;
//  std::reverse(lcsk_indices.begin(), lcsk_indices.end());
//  num_sv = 0;
//  cluster_ids->clear();
//  cluster_ids->resize(lcsk_indices.size(), 0);

  num_sv = FilterAnchorBreakpoints_(raw_lcsk_indices, begin_hit, end_hit, seed_len,
                                    0.01f*read->get_sequence_length(), 0.01f, hits, parameters, lcsk_indices, cluster_ids);

//  if (lcsk_indices.size() == 0) {
//    return;
//  }
//
//  PairwiseOverlap temp_overlap(overlap.qid, overlap.tid);
//  temp_overlap = overlap;
//  int64_t back = begin_hit + lcsk_indices.front();
//  int64_t front = begin_hit + lcsk_indices.back();
//  temp_overlap.query.start = HitPosRead_(hits[back]);
//  temp_overlap.query.end = HitPosRead_(hits[front]) + 1; // +1 means that the end is not inclusive. // + seed_len;  TODO: The end does not cover the last seed entirely!
//  temp_overlap.target.start = HitPosRef_(hits[back]);
//  temp_overlap.target.end = HitPosRef_(hits[front]) + 1; // + seed_len;
//  temp_overlap.num_seeds = lcsk_indices.size();
//
//  int64_t edit_dist = CalcEditDist_(index, read, temp_overlap);
//  double error_rate = ((double) edit_dist) / ((double) temp_overlap.target.dist());
//  overlap.num_sv = 0;
//  if (error_rate > parameters->error_rate) {
//    overlap.num_sv = 1;
//  }



//  lcsk_indices = raw_lcsk_indices;




//  int64_t diag_epsilon = 50;
//
//  filtered_lcsk.clear();
//
//  std::vector<int32_t> chain;
//
//  for (int64_t start_i=0, end_i=0; end_i<lcsk_indices.size(); end_i++) {
////    int64_t hit_id = end_hit + lcsk_indices[end_i];
////    int64_t hit_id_next = end_hit + lcsk_indices[end_i + 1];
//    chain.push_back(lcsk_indices[end_i]);
//
//    if ((end_i + 1) == lcsk_indices.size() ||
//        std::abs(HitDiag_(hits[begin_hit + lcsk_indices[end_i + 1]]) - HitDiag_(hits[begin_hit + lcsk_indices[end_i]])) >= diag_epsilon) {
//
//      if (chain.size() > filtered_lcsk.size()) {
//        filtered_lcsk = chain;
//      }
//      chain.clear();
//    }
//  }

}



