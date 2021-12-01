#pragma once
// C library headers
#include <stdlib.h>
#include <stdint.h>

// C++ library headers
#include <vector>
#include <mutex>
#include <condition_variable>

// Helper functions & data structures
// TODO: Eventually move to separate lib once it can be generalized

// Go-like channel for uint8_t
// Partial credit: https://st.xorian.net/blog/2012/08/go-style-channel-in-c/
// TODO: This can be templated for any type
class ByteChannel {
  private:
    /* NOTE: Using vector to guarantee contiguous memory for data.
     *       While the potential downside is the internal shifting of the
     *       buffer when reading/erasing from the front, the contiguous
     *       nature of vectors takes advantage of cache line locality,
     *       minimizing the shifting overhead as reads/erases get larger.
     */
    size_t maxSize_ = 65535; // Arbitrarily decided
    std::vector<uint8_t> buf_;
    std::mutex bufMtx_;
    std::condition_variable newData_;
    bool closed_ = false;

    bool exitGetWait_();

    bool notFull_ ();

  public:
    ByteChannel();
    ByteChannel(size_t max);
    ~ByteChannel();

    // Mimicking Go-style... capitalizing first letter of methods
    // Length (i.e. number of elements waiting in channel)
    size_t Len();

    // Maximum capacity of channel
    size_t Cap();

    // Close the channel
    void Close();

    bool IsClosed();

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
    bool Put(const uint8_t& item, bool wait = true);

    bool Put(const uint8_t* const items, const size_t n, bool wait = true);

    bool Put(const std::vector<uint8_t>& items, bool wait = true);

    /* Returns:
     *  - true if an item was successfully read
     *  - false if an item was not read
     *      - NOTE: If 'false' is returned, should check whether the channel
     *              is closed using the IsClosed() method
     */
    bool Get(uint8_t& item, bool wait = true);

    /* Reads at most 'n' elements and appends it to 'dst'
     * Returns the number of bytes fetched [0, n]
     *  - NOTE: If 0 is returned, should check whether the channel is closed
     *          using the IsClosed() method
     */
    size_t Get(std::vector<uint8_t>& dst, size_t n, bool wait = true);
};
