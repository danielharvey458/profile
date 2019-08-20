/*
 * Copyright 2019 Daniel Harvey
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
