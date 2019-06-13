#include <cmath>
#include <vector>


std::vector<double> vector (1 << 10);

void cache_missy ()
{
  for (auto i = 0u; i < vector.size (); ++i)
  {
    vector[rand () % vector.size ()] += 1;
  }
}

void cache_friendly ()
{
  for (auto i = 0u; i < vector.size (); ++i)
  {
    vector[i] += 1;
  }
}

int main ()
{
  cache_missy ();
  cache_friendly ();
  return vector[rand () % vector.size ()];
}
