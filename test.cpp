#include <iostream>
#include <thread>

#include <stdint.h>

#include "channel.hpp"

using namespace std;

void producer(int n, Channel<uint8_t>& chan) {
  for (int i = 0; i < n; i++) {
    chan.Put((uint8_t)(i % 255));

  }
  return;
}


int main() {
  int numItems = 200;
  Channel<uint8_t> byteChan(1);

  // Consumer in main thread
  thread prod(producer, numItems, std::ref(byteChan));

  unsigned char val;
  for (int i = 0; i < numItems; i++) {
    if (!byteChan.Get(val)) {
      cout << "Uh oh, Get() failed!" << endl;
      break;
    }

    cout << "Got " << (int)val << endl;
  }

  prod.join();
  cout << "Bye!" << endl;

  return 0;
}
