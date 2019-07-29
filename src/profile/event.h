#ifndef PROFILE_EVENT_H
#define PROFILE_EVENT_H

#include <string>
#include <vector>

namespace profile
{
  /**
   * Event represents a measurable performance event, such as
   * a CPU cycle or cache reference
   */
  class Event
  {
    public:

      /**
       * Construct an event from its @p event_type and @p event.
       *
       * The values are defined by the events for the linux
       * perf_event_open call. See perf_event_open(2)
       */
      explicit Event (unsigned event_type, long long unsigned event);

      /**
       * Get the event type this event was constructed with
       */
      unsigned type () const;

      /**
       * Get the even this event was constructed with
       */
      long long unsigned event () const;

      /**
       * Get a string representation of the event
       */
      const std::string &str () const;

      /**
       * Construct an event from a string. See Event::all_named
       * to list all available events
       */
      static Event from_string (const std::string &str);

      /**
       * List all available events.
       */
      static const std::vector<Event> &all_named ();

    private:
      unsigned m_event_type;
      long long unsigned m_event;
  };

  /**
   * Compare event for equality
   */
  bool operator == (Event lhs, Event rhs);

  /**
   * Compare event for inequality
   */
  bool operator != (Event lhs, Event rhs);
}

#include "profile/event.inl"

#endif