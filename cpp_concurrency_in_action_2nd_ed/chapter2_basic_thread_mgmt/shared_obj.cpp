#include <iostream>
#include <thread>
  
using namespace std;

struct Person {
  string name;
  int age;
};

int thread_func(Person& p) {
  p.name += '-';
  p.age += 1;
  return p.age;
}

int main() {
  Person p {.name = "bipin", .age = 28};
  auto th1 = thread(std::bind(thread_func, p));
  th1.join();
  cout << p.name << " " << p.age << endl;
  auto th2 = thread([&](){
      thread_func(p);
  });
  th2.join();
  cout << p.name << " " << p.age << endl;
  auto th3 = thread(bind(thread_func, ref(p)));
  th3.join();
  cout << p.name << " " << p.age << endl;
  return 0;
}
