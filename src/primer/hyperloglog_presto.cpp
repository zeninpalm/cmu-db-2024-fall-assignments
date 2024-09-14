#include "primer/hyperloglog_presto.h"

namespace bustub {

template <typename KeyType>
HyperLogLogPresto<KeyType>::HyperLogLogPresto(int16_t n_leading_bits) : cardinality_(0), n_leading_bits_(n_leading_bits<=0?0:n_leading_bits), dense_bucket_(1<<(n_leading_bits<=0?0:n_leading_bits), 0) {}

template <typename KeyType>
auto HyperLogLogPresto<KeyType>::AddElem(KeyType val) -> void {
  /** @TODO(student) Implement this function! */
    hash_t hash = CalculateHash(val);
    std::bitset<64> bset(hash);
    
    size_t bucket_index = bset.to_ullong() >> (64 - n_leading_bits_);
    if (n_leading_bits_ == 0) {
      bucket_index = 0;
    }

    uint64_t value = CountOfRightmostContinuousZeros(bset);

    std::bitset<DENSE_BUCKET_SIZE> current = dense_bucket_[bucket_index];
    uint64_t current_value = current.to_ulong();

    if (value <= current_value) {
        return;  // No need to update
    }

    uint64_t max_value = (1 << DENSE_BUCKET_SIZE) - 1;
    if (value > max_value) {
        // Handle overflow
        uint64_t overflow_value = value >> DENSE_BUCKET_SIZE;
        if (overflow_value > 7) {  // 3 bits can represent up to 7
            overflow_value = 7;
        }
        overflow_bucket_[bucket_index] = std::bitset<OVERFLOW_BUCKET_SIZE>(overflow_value);
        dense_bucket_[bucket_index] = std::bitset<DENSE_BUCKET_SIZE>(value & max_value);
    } else {
        dense_bucket_[bucket_index] = std::bitset<DENSE_BUCKET_SIZE>(value);
    }

}

template <typename T>
auto HyperLogLogPresto<T>::ComputeCardinality() -> void {
  /** @TODO(student) Implement this function! */
    double sum = 0.0;
    size_t m = dense_bucket_.size();

    for (size_t i = 0; i < m; ++i) {
        uint64_t value = dense_bucket_[i].to_ullong();
        if (overflow_bucket_.find(i) != overflow_bucket_.end()) {
          uint64_t overflow_value = overflow_bucket_[i].to_ullong();
          std::bitset<TOTAL_BUCKET_SIZE> combined;
          combined |= (overflow_value << DENSE_BUCKET_SIZE);
          combined |= value;
          value = combined.to_ullong();
        }

        if (value >= 64) {
          sum += 1.0 / ((1ULL << 63) * 2.0);
        } else {
          sum += 1.0 / (1ULL << value);
        }
    }


    double alpha_m = CONSTANT * m * m;
    double estimate = alpha_m / sum;

    cardinality_ = static_cast<uint64_t>(std::floor(estimate));
}

template <typename KeyType>
auto HyperLogLogPresto<KeyType>::CountOfRightmostContinuousZeros(const std::bitset<64> &bset) const -> uint64_t {
  for (int i = 0;  i <= 64 - n_leading_bits_; i++) {
    if (bset[i]) {
      return i;
    }
  }
  return 64 - n_leading_bits_;
}

template class HyperLogLogPresto<int64_t>;
template class HyperLogLogPresto<std::string>;
}  // namespace bustub
