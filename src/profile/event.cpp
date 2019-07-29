#include "profile/event.h"
#include "profile/exception.h"

#include <linux/perf_event.h>

#include <algorithm>
#include <tuple>
#include <vector>

namespace profile
{
  namespace
  {
    using EventLookup
      = std::pair<std::vector<Event>, std::vector<std::string>>;

    const EventLookup &event_lookup ()
    {
      static const auto lookup = []
      {
        EventLookup lookup;

#define ADD_EVENT(type, name)                           \
        lookup.first.emplace_back (Event (type, name)); \
        lookup.second.emplace_back (#name);

        ADD_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES);
        ADD_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS);
        ADD_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES);
        ADD_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES);
        ADD_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_INSTRUCTIONS);
        ADD_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES);
        ADD_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_BUS_CYCLES);
        ADD_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_FRONTEND);
        ADD_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_BACKEND);
        ADD_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_REF_CPU_CYCLES);
#undef ADD_EVENT

        return lookup;
      } ();

      return lookup;
    }
  }

  Event Event::from_string (const std::string &str)
  {
    const auto &lookup = event_lookup ();

    const auto icompare = [&] (const auto &name)
    {
      return std::equal (name.begin (), name.end (),
                         str.begin (), str.end (),
                         [] (char a, char b)
                         {
                           return tolower (a) == tolower (b);
                         });
    };

    const auto it = std::find_if (lookup.second.begin (),
                                  lookup.second.end (),
                                  icompare);

    ASSERT (it != lookup.second.end (), "No such event");

    return lookup.first[std::distance (lookup.second.begin (), it)];
  }

  const std::string &Event::str () const
  {
    const auto &lookup = event_lookup ();

    const auto it = std::find (lookup.first.begin (),
                               lookup.first.end (),
                               *this);

    ASSERT (it != lookup.first.end (), "No such event");

    return lookup.second[std::distance (lookup.first.begin (), it)];
  }

  const std::vector<Event> &Event::all_named ()
  {
    return event_lookup ().first;
  }

  bool operator == (const Event lhs, const Event rhs)
  {
    return std::forward_as_tuple (lhs.type (), lhs.event ()) ==
           std::forward_as_tuple (rhs.type (), rhs.event ());
  }

  bool operator != (const Event lhs, const Event rhs)
  {
    return !(lhs == rhs);
  }
}
