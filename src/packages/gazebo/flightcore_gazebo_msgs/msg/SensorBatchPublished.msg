# Gazebo producer-side declaration for one released observation batch.
#
# This message is published only after Gazebo has called publish() for every
# sensor selected by required_mask. It proves producer completion, not DDS
# delivery and not FlightCore consumption. Coordinator therefore never grants
# /clock directly from this message; generated FlightCore must first answer
# with ObservationReady carrying the same identity.
#
# Sensor mask bits are frozen by the lightweight lockstep protocol:
#   0x01 = IMU
#   0x02 = GPS
# Gazebo is the only component allowed to calculate required_mask. PRIME and
# every gps_rate_divider epoch require IMU|GPS; all other epochs require IMU.

# Active PRIME/session generation. Nonzero and constant for one Gazebo process.
uint64 session_id

# Command/plant epoch that produced this observation. PRIME uses step_id=0.
uint64 step_id

# Actuator command applied for step_id. PRIME uses command_id=0.
uint64 command_id

# Gazebo physics iteration represented by the sensor payload.
uint64 iteration

# Exact Gazebo simulation timestamp shared by all required sensor messages.
int64 sim_time_ns

# Sensors FlightCore must newly receive before it may acknowledge this batch.
uint8 required_mask

# Sensors for which Gazebo completed publish() in this batch.
uint8 published_mask

# False means Gazebo rejected or could not release the batch.
bool accepted

# Human-readable phase/error text; not used as an identity field.
string status
