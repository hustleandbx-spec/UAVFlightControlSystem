# FlightCore consumer-side confirmation for one immutable observation snapshot.
#
# Generated FlightCore publishes this message from asynchronous IMU/GPS receive
# callbacks, before the corresponding /clock is granted. The model step reads
# the already-cached sensor payload; it does not execute ROS subscription
# callbacks itself. Coordinator validates identity and masks, then combines
# this ACK with the previous model step_notify before granting the next clock.
#
# received_mask describes transport receipt only. Sensor message `valid` fields
# remain data-quality inputs to FlightCore and must not be confused with DDS
# delivery, otherwise one invalid GPS sample would deadlock the scheduler.

# Copied unchanged from SensorBatchPublished.
uint64 session_id

# Copied unchanged from SensorBatchPublished; PRIME uses zero.
uint64 step_id

# Copied unchanged from SensorBatchPublished; PRIME uses zero.
uint64 command_id

# Sensor sequence/physics iteration committed into the receive snapshot.
uint64 iteration

# Sensor timestamp; must equal the batch and required message timestamps.
int64 sim_time_ns

# Required sensor set declared by Gazebo for this iteration.
uint8 required_mask

# Sensor callbacks actually executed by generated FlightCore for this iteration.
uint8 received_mask

# True only when every required bit was received with matching identity.
bool accepted

# Human-readable phase/error text; not used as an identity field.
string status
