// C++ libs
#include <iostream>
#include <thread>

// C libs
#include <stdint.h>

// Project libs
#include "channel.hpp"

// Google test, duh
#include "gtest/gtest.h"

using namespace std;

void producer(int n, Channel<uint8_t>& chan) {
  for (int i = 0; i < n; i++) {
    chan.Put((uint8_t)(i % 255));

  }
  return;
}


bool TestChanSize1() {
  int numItems = 200;
  Channel<uint8_t> byteChan(1);

  // Consumer in main thread
  thread prod(producer, numItems, std::ref(byteChan));

  unsigned char val;
  for (int i = 0; i < numItems; i++) {
    if (!byteChan.Get(val)) {
      cout << "Uh oh, Get() failed!" << endl;
      return false;
    }

    cout << "Got " << (int)val << endl;
  }

  prod.join();
  cout << "Bye!" << endl;

  return true;
}

TEST(ChannelTest, ChanSize1) {
  EXPECT_TRUE(TestChanSize1());
}
