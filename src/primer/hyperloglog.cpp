#include "primer/hyperloglog.h"

namespace bustub {

template <typename KeyType>
HyperLogLog<KeyType>::HyperLogLog(int16_t n_bits) : cardinality_(0), registers_(1 << ((n_bits <= 0)?0:n_bits), 0), n_bits_(n_bits), mtx_() {}

template <typename KeyType>
auto HyperLogLog<KeyType>::ComputeBinary(const hash_t &hash) const -> std::bitset<BITSET_CAPACITY> {
  // convert hash to binary  
  std::bitset<BITSET_CAPACITY> binary{hash};
  return binary;
}

template <typename KeyType>
auto HyperLogLog<KeyType>::PositionOfLeftmostOne(const std::bitset<BITSET_CAPACITY> &bset) const -> uint64_t {
  for (int i = BITSET_CAPACITY - 1 - n_bits_;  i >= 0; i--) {
    if (bset[i]) {
      return BITSET_CAPACITY - n_bits_ - i;
    }
  }
  return BITSET_CAPACITY - n_bits_;
}

template <typename KeyType>
auto HyperLogLog<KeyType>::AddElem(KeyType val) -> void {
  /** @TODO(student) Implement this function! */
  mtx_.lock();
  hash_t hash = CalculateHash(val);
  std::bitset<BITSET_CAPACITY> binary = ComputeBinary(hash);
  uint64_t position = PositionOfLeftmostOne(binary);
  // Calculate the register index using the first n_bits_ of the hash
  size_t register_index = binary.to_ullong() >> (BITSET_CAPACITY - n_bits_);
  if (n_bits_ <= 0) {
    register_index = 0;
  }

  // Update the register if the new position is greater
  if (static_cast<long int>(position) > registers_[register_index]) {
    registers_[register_index] = position;
  }
  mtx_.unlock();
}

template <typename KeyType>
auto HyperLogLog<KeyType>::ComputeCardinality() -> void {
  /** @TODO(student) Implement this function! */
  double sum = 0.0;
  for (size_t i = 0; i < registers_.size(); i++) {
    sum += 1.0 / (1 << registers_[i]);
  }
  double estimate = CONSTANT * registers_.size()  * registers_.size() / sum;
  cardinality_ = static_cast<uint64_t>(std::floor(estimate));
}

template class HyperLogLog<int64_t>;
template class HyperLogLog<std::string>;

}  // namespace bustub
