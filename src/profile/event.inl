
namespace profile
{
  inline Event::Event (const unsigned event_type,
                       const long long unsigned event)
    : m_event_type (event_type),
      m_event (event)
  {
  }

  inline unsigned Event::type () const
  {
    return m_event_type;
  }

  inline long long unsigned Event::event () const
  {
    return m_event;
  }
}
