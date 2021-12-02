#include <iostream>
#include <thread>

#include "helpers.hpp"

using namespace std;

void producer(int n, ByteChannel& chan) {
  for (int i = 0; i < n; i++) {
    chan.Put((unsigned char)(i % 255));

  }
  return;
}


int main() {
  int numItems = 200;
  ByteChannel byteChan(1);

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
