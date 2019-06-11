#include <cmath>
#include <numeric>
#include <vector>

std::vector<double> f1 ()
{
  std::vector<double> v;
  for (auto i = 0; i < (1<<1); ++i)
  {
    v.push_back (rand ());
  }
  return v;
}

std::vector<double> f2 ()
{
  std::vector<double> v;
  for (auto j = 0; j < (1 << 1); ++j)
  {
    const auto v1 = f1 ();
    v.push_back (
      std::accumulate (v1.begin (), v1.end (), 0));
  }
  return v;
}

int main ()
{
  return static_cast<int> (f2 ().size ());
}
