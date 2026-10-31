#include "EvsManager.hh"

#include <iostream>
#include <stdexcept>

using namespace anlnext;

void require(bool condition, const char* message)
{
  if (!condition) { throw std::runtime_error(message); }
}

int main()
{
  try {
    EvsManager evs;
    evs.define("selected");
    evs.define("unused");
    evs.set("selected");
    evs.count();
    evs.count_completed();
    evs.count();
    require(evs.data().at("selected").counts == 2 &&
            evs.data().at("selected").counts_ok == 1, "initial counts are incorrect");

    evs.reset_all_counts();
    require(evs.data().size() == 2, "reset removed event selections");
    for (const auto& entry: evs.data()) {
      require(entry.second.counts == 0 && entry.second.counts_ok == 0,
              "reset did not clear all event selection counts");
    }
    require(evs.get("selected") && !evs.get("unused"), "reset changed event selection flags");

    evs.count();
    evs.count_completed();
    require(evs.data().at("selected").counts == 1 &&
            evs.data().at("selected").counts_ok == 1, "counting did not restart from zero");
    require(evs.data().at("unused").counts == 0 &&
            evs.data().at("unused").counts_ok == 0, "unset event selection was counted");
  }
  catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
