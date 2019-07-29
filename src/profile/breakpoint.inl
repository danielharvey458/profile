namespace profile::detail
{
  bool Breakpoint::is_enabled () const
  {
    return m_enabled;
  }

  std::intptr_t Breakpoint::address () const
  {
    return m_address;
  }
}
