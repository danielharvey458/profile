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

namespace nmsp
{
  void namespaced ()
  {
    for (auto i = 0; i < 1 << 10; ++i) {}
  }

  struct Foo
  {
    int bar ()
    {
      return 3;
    }
  };

  template<typename T>
  T template_function (const T &t)
  {
    return t;
  }
}

int main ()
{
  nmsp::namespaced ();
  const auto lambda_function_thing = [] (int, long) {return 1;};
  return nmsp::Foo ().bar ()
   + lambda_function_thing (1, 2)
   + nmsp::template_function (3.0);
}
