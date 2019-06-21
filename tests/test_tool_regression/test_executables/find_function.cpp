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
