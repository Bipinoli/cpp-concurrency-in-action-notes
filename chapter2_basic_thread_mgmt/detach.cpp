#include <unistd.h>
#include <iostream>
#include <thread>

using namespace std;

int main() {
  auto th = thread([&]() {
      cout << "running the thhread" << endl;
      sleep(3);
      cout << "finished running the thrrread" << endl;
  });
  th.detach();
  sleep(1);

  cout << "Finishing the main proccess" << endl;
  pthread_exit(nullptr); // wait for all threads including detached to finish
  return 0;
}
