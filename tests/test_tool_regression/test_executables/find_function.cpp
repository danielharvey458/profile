namespace nmsp
{
  void namespaced ()
  {
    for (auto i = 0; i < 1 << 10; ++i) {}
  }
}

int main ()
{
  nmsp::namespaced ();
}
