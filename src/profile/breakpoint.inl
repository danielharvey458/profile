namespace profile::detail
{
  inline bool Breakpoint::is_enabled () const
  {
    return m_enabled;
  }

  inline std::intptr_t Breakpoint::address () const
  {
    return m_address;
  }
}
