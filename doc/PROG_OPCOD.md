# mission PROG chunk OPCODE LIST
- AB (171) set message indexe to waypoint for navmap
- A5 (165) set objective to fly to waypoint return true when completed
- A6 (166) set objective to fly to way area return true when completed
- A2 (162) set objective to land plane at waypoint return true when completed
- A1 (161) set objective to takeoff from waypoint (simulate takeoff for ai) return true when completed return true when completed
- AA (170) set objective to follow target with id as the value of the index of the part in the mission file
- A8 (168) set objective to defend ally with id as the value  of the index of the part in the mission file return true when completed
- A7 (167) destroy target with id as the value id is the index of the part in the mission file return true when completed
- 92 (146) check if part ID is in same area.
- 94 (148) destroy / instant kill target
- BE (190) deactivate target / disable AI on target id
- 48 (72) if last state not true goto label
- 46 (70) if last state is true goto label
- 08 (8) define label
- 01 (1) exit prog

# not complety sure about the following

- 45 (69) check flag with id as flag number
- 10 (16) set label number



- 52 (82) set flag with id as flag number

From the strike.exe we should have the following:

- 161 : Take off 
- 162 : Landing  
- Fly to 
Precise Way
- 165 : Fly to 
Way Point
- 166 : Fly to 
Way Area
- 167 : Destroy
Target
- 168 Defend
Ally
- Defend
Point
- 170 Follow
Leader

# NOTE on TEAM CHUNK
so it's seems that the team is define in the mission filed as follow:
- first ushort (16 bit) is the leader. the value is the index of the part in the mission file (beware, not the actual id, but the position in the part array)