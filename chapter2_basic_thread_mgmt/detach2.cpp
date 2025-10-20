#include <iostream>
#include <thread>
#include <unistd.h>

using namespace std;

void func1() {
  cout << "in func1" << endl;
  sleep(2);
  cout << "done func1" << endl;
}

void func2() {
  cout << "in func2" << endl;
  auto t = thread(func1);
  cout << "runninng thread with id: " << t.get_id() << endl;
  t.detach();
  cout << "done func2" << endl;
}

int main() {
  auto t = thread(func2);
  cout << "running thread with id: " << t.get_id() << endl;
  t.detach();
  sleep(3);
  return 0;
}
