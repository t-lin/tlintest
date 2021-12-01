#include <javad_gnss/helpers.hpp>

// C++ headers
#include <functional> // For std::bind

using std::mutex;
using std::unique_lock;
using std::vector;

/***************************************
 * Definitions for class ByteChannel
 ***************************************/
ByteChannel::ByteChannel() {}
ByteChannel::ByteChannel(size_t max) : maxSize_(max) {}
ByteChannel::~ByteChannel() {}

// Should be called when thread has lock
// TODO: See re-entrant locks
bool ByteChannel::exitGetWait_() {
  return !buf_.empty() || closed_;
}

// Should be called when thread has lock
bool ByteChannel::notFull_ () {
  return buf_.size() < maxSize_;
}

size_t ByteChannel::Len() {
  unique_lock<mutex> lock(bufMtx_); // Needed?
  return buf_.size();
}

size_t ByteChannel::Cap() {
  return maxSize_;
}

// Close the channel
void ByteChannel::Close() {
  // Lock to ensure those waiting are truly in the wait state
  //  - Explanation : https://www.modernescpp.com/index.php/c-core-guidelines-be-aware-of-the-traps-of-condition-variables
  unique_lock<mutex> lock(bufMtx_);
  closed_ = true;
  newData_.notify_all();
}

bool ByteChannel::IsClosed() {
  unique_lock<mutex> lock(bufMtx_);
  return closed_;
}

/* Returns
 *  - true if the item was successfully written
 *  - false if the item was not written
 *      - A return value of false may indicate that the channel is full
 *        and the wait flag is false, or that the channel is closed
 *      - The caller should follow-up by calling IsClosed() to check
 *        whether the channel is closed. Note that checking IsClosed()
 *        prior to Put() is meaningless in multi-producer scenarios
 *        since another thread may Close() in between the calls to
 *        IsClosed() and Put().
 */
bool ByteChannel::Put(const uint8_t& item, bool wait) {
  unique_lock<mutex> lock(bufMtx_);

  if (closed_) {
    //throw logic_error("ERROR: Attempting to put on a closed channel\n");
    return false;
  }

  if ( buf_.size() >= maxSize_ ) {
    if (wait) {
      newData_.wait(lock, std::bind(&ByteChannel::notFull_, this));
    } else {
      return false;
    }
  }

  buf_.push_back(item);
  lock.unlock();
  newData_.notify_one();
  return true;
}

bool ByteChannel::Put(const uint8_t* const items, const size_t n, bool wait) {
  unique_lock<mutex> lock(bufMtx_);

  if (closed_) {
    //throw logic_error("ERROR: Attempting to put on a closed channel\n");
    return false;
  }

  if ( buf_.size() >= maxSize_ ) {
    if (wait) {
      newData_.wait(lock, std::bind(&ByteChannel::notFull_, this));
    } else {
      return false;
    }
  }

  buf_.insert(buf_.end(), items, items + n);
  lock.unlock();
  newData_.notify_one();
  return true;
}

bool ByteChannel::Put(const vector<uint8_t>& items, bool wait) {
  return this->Put(items.data(), items.size(), wait);
}

/* Returns:
 *  - true if an item was successfully read
 *  - false if an item was not read
 *      - NOTE: If 'false' is returned, should check whether the channel
 *              is closed using the IsClosed() method
 */
bool ByteChannel::Get(uint8_t& item, bool wait) {
  unique_lock<mutex> lock(bufMtx_);
  if (wait) {
    newData_.wait(lock, std::bind(&ByteChannel::exitGetWait_, this));
  }

  if (buf_.empty()) {
    return false;
  }

  item = buf_.front();
  buf_.erase(buf_.begin());
  return true;
}

/* Reads at most 'n' elements and appends it to 'dst'
 * Returns the number of bytes fetched [0, n]
 *  - NOTE: If 0 is returned, should check whether the channel is closed
 *          using the IsClosed() method
 */
size_t ByteChannel::Get(vector<uint8_t>& dst, size_t n, bool wait) {
  unique_lock<mutex> lock(bufMtx_);
  if (wait) {
    newData_.wait(lock, std::bind(&ByteChannel::exitGetWait_, this));
  }

  if (buf_.empty()) {
    return 0;
  }

  if (buf_.size() < n) {
    n = buf_.size();
  }

  dst.insert(dst.end(), buf_.begin(), buf_.begin() + n);
  buf_.erase(buf_.begin(), buf_.begin() + n);
  return n;
}

/***************************************
 * Other helper funcs
 ***************************************/
