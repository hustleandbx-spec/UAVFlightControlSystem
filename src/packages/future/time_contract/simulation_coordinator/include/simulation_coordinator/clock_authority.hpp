#pragma once

#include <cstdint>

namespace simulation_coordinator
{

using GenerationId = std::uint64_t;
using StepId = std::uint64_t;
using SimTimeNs = std::int64_t;

enum class AuthorityState
{
  IDLE,
  READY,
  STEP_PENDING,
  ABORTED
};

class ClockAuthority
{
public:
  explicit ClockAuthority(SimTimeNs physics_dt_ns);

  bool reset();
  bool prepare_next_step();
  bool commit_pending_step();
  void abort();

  GenerationId generation() const;
  StepId committed_step() const;
  SimTimeNs committed_time_ns() const;

  StepId pending_step() const;
  SimTimeNs pending_time_ns() const;

  SimTimeNs physics_dt_ns() const;
  AuthorityState state() const;
  
private:
  GenerationId generation_{0};

  StepId committed_step_{0};
  SimTimeNs committed_time_ns_{0};

  StepId pending_step_{0};
  SimTimeNs pending_time_ns_{0};

  SimTimeNs physics_dt_ns_{0};
  AuthorityState state_{AuthorityState::IDLE};
};

}  // namespace simulation_coordinator