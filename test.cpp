#include <iostream>

using namespace std;

int main() {
  float a = 3.1415926535;
  uint32_t b = *(uint32_t*)&a;
  auto c = *reinterpret_cast<uint32_t*>(&a);
  cout << "b: " << b << "; c: " << c << endl;

  return 0;
}
